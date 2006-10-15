/***************************************************************************
 * guimanager.h
 * This file is part of the KDE project
 * copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
 * copyright (C) 2006 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "guimanager.h"
#include "manager.h"
#include "action.h"
#include "guiclient.h"
#include "model.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileInfo>
#include <QDir>

#include <kapplication.h>
//#include <kdeversion.h>
#include <kconfig.h>
#include <klocale.h>
#include <kicon.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kfiledialog.h>
#include <kmenu.h>

#include <ktar.h>
#include <kio/netaccess.h>
#include <knewstuff/provider.h>
#include <knewstuff/engine.h>
#include <knewstuff/downloaddialog.h>
#include <knewstuff/knewstuffsecure.h>

using namespace Kross;

/******************************************************************************
 * GUIManagerView
 */

namespace Kross {

    /// \internal class that inherits \a KNewStuffSecure to implement the GHNS-functionality.
    class GUIManagerNewStuff : public KNewStuffSecure
    {
        public:
            GUIManagerNewStuff(GUIManagerView* view, const QString& type, QWidget *parentWidget = 0)
                : KNewStuffSecure(type, parentWidget)
                , m_view(view) {}
            virtual ~GUIManagerNewStuff() {}
        private:
            GUIManagerView* m_view;
            virtual void installResource() { m_view->installPackage( m_tarName ); }
    };

    /// \internal d-pointer class.
    class GUIManagerView::Private
    {
        public:
            GUIManagerModule* module;
            QItemSelectionModel* selectionmodel;
            GUIManagerNewStuff* newstuff;
            bool modified;

            Private(GUIManagerModule* m) : module(m), newstuff(0), modified(false) {}
    };

}

GUIManagerView::GUIManagerView(GUIManagerModule* module, QWidget* parent)
    : QTreeView(parent)
    , d(new Private(module))
{
    setAlternatingRowColors(true);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setItemsExpandable(false);
    header()->hide();

    ActionCollectionModel* model = new ActionCollectionModel(this, Kross::Manager::self().actionCollection());
    setModel(model);

    connect(model, SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)),
            this, SLOT(slotDataChanged(const QModelIndex&,const QModelIndex&)));

    d->selectionmodel = new QItemSelectionModel(model, this);
    connect(d->selectionmodel, SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(slotSelectionChanged()));
    setSelectionModel(d->selectionmodel);
}

GUIManagerView::~GUIManagerView()
{
    delete d;
}

bool GUIManagerView::isModified() const
{
    return d->modified;
}

bool GUIManagerView::installPackage(const QString& scriptpackagefile)
{
    KTar archive( scriptpackagefile );
    if(! archive.open(QIODevice::ReadOnly)) {
        KMessageBox::sorry(0, i18n("Could not read the package \"%1\".", scriptpackagefile));
        return false;
    }

    const KArchiveDirectory* archivedir = archive.directory();
    const KArchiveEntry* entry = archivedir->entry("install.rc");
    if(! entry || ! entry->isFile()) {
        KMessageBox::sorry(0, i18n("The package \"%1\" does not contain a valid install.rc file.", scriptpackagefile));
        return false;
    }

    QString xml = static_cast< const KArchiveFile* >(entry)->data();
    QDomDocument domdoc;
    if(! domdoc.setContent(xml)) {
        KMessageBox::sorry(0, i18n("Failed to parse the install.rc file at package \"%1\".", scriptpackagefile));
        return false;
    }

    QString destination = KGlobal::dirs()->saveLocation("appdata", "scripts/", true);
    if(destination.isNull()) {
        KMessageBox::sorry(0, i18n("Failed to determinate location where the package \"%1\" should be installed to.", scriptpackagefile));
        return false;
    }

    QString packagename = QFileInfo(scriptpackagefile).baseName();
    destination += packagename; // add the packagename to the name of the destination-directory.

    QDir packagepath(destination);
    if( packagepath.exists() ) {
        if( KMessageBox::warningContinueCancel(0,
            i18n("A script package with the name \"%1\" already exists. Replace this package?", packagename),
            i18n("Replace")) != KMessageBox::Continue )
                return false;
        if(! KIO::NetAccess::del(destination, 0) ) {
            KMessageBox::sorry(0, i18n("Could not uninstall this script package. You may not have sufficient permissions to delete the folder \"%1\".", destination));
            return false;
        }
    }

    krossdebug( QString("Copy script-package to destination directory: %1").arg(destination) );
    archivedir->copyTo(destination, true);

    QDomNodeList nodelist = domdoc.elementsByTagName("ScriptAction");
    int nodelistcount = nodelist.count();
    for(int i = 0; i < nodelistcount; ++i) {
        QDomElement element = nodelist.item(i).toElement();

        Action* action = new Action(Manager::self().actionCollection(), element, packagepath);
        connect(action, SIGNAL( failed(const QString&, const QString&) ), this, SLOT( executionFailed(const QString&, const QString&) ));
        connect(action, SIGNAL( success() ), this, SLOT( executionSuccessful() ));
        connect(action, SIGNAL( activated(Kross::Action*) ), SIGNAL( executionStarted(Kross::Action*)));
        Manager::self().actionMenu()->addAction(action);
    }

    d->modified = true;
    return true;
}

bool GUIManagerView::uninstallPackage(Action* action)
{
    const QString name = action->objectName();

    KUrl url = action->getFile();
    if(! url.isValid() || ! url.isLocalFile()) {
        KMessageBox::sorry(0, i18n("Could not uninstall the script package \"%1\" since the script is not installed.").arg(action->objectName()));
        return false;
    }

    QDir dir = QFileInfo( url.path() ).dir();
    const QString scriptpackagepath = dir.absolutePath();
    krossdebug( QString("Uninstall script-package with destination directory: %1").arg(scriptpackagepath) );

    if(! KIO::NetAccess::del(scriptpackagepath, 0) ) {
        KMessageBox::sorry(0, i18n("Could not uninstall the script package \"%1\". You may not have sufficient permissions to delete the folder \"%1\".").arg(action->objectName()).arg(scriptpackagepath));
        return false;
    }

    Manager::self().actionMenu()->removeAction(action);
    delete action; action = 0; // removes the action from d->actions as well

    d->modified = true;
    return true;
}

void GUIManagerView::slotSelectionChanged()
{
    /*TODO
    bool isselected = d->selectionmodel->hasSelection();
    d->collection->action("runscript")->setEnabled(isselected);
    d->collection->action("stopscript")->setEnabled(isselected);
    d->collection->action("uninstallscript")->setEnabled(isselected);
    */
}

void GUIManagerView::slotDataChanged(const QModelIndex&, const QModelIndex&)
{
    d->modified = true;
}

void GUIManagerView::slotRun()
{
    foreach(QModelIndex index, d->selectionmodel->selectedIndexes())
        if(index.isValid())
            static_cast< Action* >(index.internalPointer())->trigger();
}

void GUIManagerView::slotStop()
{
    foreach(QModelIndex index, d->selectionmodel->selectedIndexes())
        if(index.isValid())
            static_cast< Action* >(index.internalPointer())->finalize();
}

bool GUIManagerView::slotInstall()
{
    KFileDialog* filedialog = new KFileDialog(
        KUrl("kfiledialog:///KrossInstallPackage"), // startdir
        "*.tar.gz *.tgz *.bz2", // filter
        0, // custom widget
        0 // parent
    );
    filedialog->setCaption(i18n("Install Script Package"));
    return filedialog->exec() ? installPackage(filedialog->selectedUrl().path()) : false;
}

void GUIManagerView::slotUninstall()
{
    foreach(QModelIndex index, d->selectionmodel->selectedIndexes())
        if(index.isValid())
            if(! uninstallPackage( static_cast< Action* >(index.internalPointer()) ))
                break;
}

void GUIManagerView::slotNewScripts()
{
    const QString appname = KApplication::kApplication()->objectName();
    const QString type = QString("%1/script").arg(appname);
    krossdebug( QString("GUIManagerView::slotNewScripts %1").arg(type) );
    if(! d->newstuff) {
        d->newstuff = new GUIManagerNewStuff(this, type);
        connect(d->newstuff, SIGNAL(installFinished()), this, SLOT(slotNewScriptsInstallFinished()));
    }
    KNS::Engine *engine = new KNS::Engine(d->newstuff, type, this);
    KNS::DownloadDialog *d = new KNS::DownloadDialog(engine, this);
    d->setCategory(type);
    KNS::ProviderLoader *p = new KNS::ProviderLoader(this);
    QObject::connect(p, SIGNAL(providersLoaded(Provider::List*)), d, SLOT(slotProviders(Provider::List*)));
    p->load(type, QString("http://download.kde.org/khotnewstuff/%1scripts-providers.xml").arg(appname));
    d->exec();
}

void GUIManagerView::slotNewScriptsInstallFinished()
{
    // Delete KNewStuff's configuration entries. These entries reflect what has
    // already been installed. As we cannot yet keep them in sync after uninstalling
    // scripts, we deactivate the check marks entirely.
    KGlobal::config()->deleteGroup("KNewStuffStatus");
}

/******************************************************************************
 * GUIManagerModule
 */

namespace Kross {

    /// \internal d-pointer class.
    class GUIManagerModule::Private
    {
        public:
    };

}

GUIManagerModule::GUIManagerModule()
    : QObject()
    , d(new Private())
{
}

GUIManagerModule::~GUIManagerModule()
{
    delete d;
}

void GUIManagerModule::showManagerDialog()
{
    KDialog* dialog = new KDialog();
    dialog->setCaption( i18n("Script Manager") );
    dialog->setButtons( KDialog::Ok | KDialog::Cancel );

    QWidget* mainwidget = dialog->mainWidget();
    QHBoxLayout* mainlayout = new QHBoxLayout();
    mainlayout->setMargin(0);
    mainwidget->setLayout(mainlayout);

    GUIManagerView* view = new GUIManagerView(this, mainwidget);
    mainlayout->addWidget(view);

    QWidget* btnwidget = new QWidget(mainwidget);
    QVBoxLayout* btnlayout = new QVBoxLayout();
    btnlayout->setMargin(0);
    btnwidget->setLayout(btnlayout);
    mainlayout->addWidget(btnwidget);

    KPushButton* runbtn = new KPushButton(KIcon("player_play"), i18n("Run"), btnwidget);
    runbtn->setToolTip( i18n("Execute the selected script.") );
    btnlayout->addWidget(runbtn);
    connect(runbtn, SIGNAL(clicked()), view, SLOT(slotRun()) );

    KPushButton* stopbtn = new KPushButton(KIcon("player_stop"), i18n("Stop"), btnwidget);
    stopbtn->setToolTip( i18n("Stop execution of the selected script.") );
    btnlayout->addWidget(stopbtn);
    connect(stopbtn, SIGNAL(clicked()), view, SLOT(slotStop()) );

    KPushButton* installbtn = new KPushButton(KIcon("fileimport"), i18n("Install"), btnwidget);
    installbtn->setToolTip( i18n("Install a script-package.") );
    btnlayout->addWidget(installbtn);
    connect(installbtn, SIGNAL(clicked()), view, SLOT(slotInstall()) );

    KPushButton* uninstallbtn = new KPushButton(KIcon("fileclose"), i18n("Uninstall"), btnwidget);
    uninstallbtn->setToolTip( i18n("Uninstall the selected script-package.") );
    btnlayout->addWidget(uninstallbtn);
    connect(uninstallbtn, SIGNAL(clicked()), view, SLOT(slotUninstall()) );

    KPushButton* newstuffbtn = new KPushButton(KIcon("knewstuff"), i18n("Get New Scripts"), btnwidget);
    newstuffbtn->setToolTip( i18n("Get new scripts from the internet.") );
    btnlayout->addWidget(newstuffbtn);
    connect(newstuffbtn, SIGNAL(clicked()), view, SLOT(slotNewScripts()) );

    //i18n("About") i18n("Configure") ...

    btnlayout->addStretch(1);
    dialog->resize( QSize(460, 340).expandedTo( dialog->minimumSizeHint() ) );

    int result = dialog->exec();
    if ( view->isModified() ) {
        if( result == QDialog::Accepted /*&& dialog->result() == KDialog::Ok*/ ) {
            // save new config
            Manager::self().writeConfig();
        }
        else {
            // restore old config
            Manager::self().readConfig();
        }
        QMetaObject::invokeMethod(&Manager::self(), "configChanged");
    }
    dialog->delayedDestruct();
}

#include "guimanager.moc"
