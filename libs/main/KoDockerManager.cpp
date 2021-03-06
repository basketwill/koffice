/* This file is part of the KDE project
 *
 * Copyright (c) 2008 Casper Boemann <cbr@boemann.dk>
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoDockerManager.h"
#include "KDockFactoryBase.h"

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>

#include "KoToolDocker_p.h"

#include "KoView.h"
#include "KoMainWindow.h"

class ToolDockerFactory : public KDockFactoryBase
{
public:
    ToolDockerFactory(const QString &name, QObject *parent = 0)
        : KDockFactoryBase(parent, name)
    {
        setDefaultDockPosition(DockRight);
    }

    QDockWidget *createDockWidget() {
        KoToolDocker * dockWidget = new KoToolDocker();
        dockWidget->setObjectName(id());
        return dockWidget;
    }
};


class KoDockerManager::Private
{
public:
    Private() {}
    KoMainWindow* mainWindow;
    QMap<QString, KoToolDocker *> toolDockerMap;
    QMap<QString, bool> toolDockerVisibilityMap;
    QMap<QString, KoToolDocker *> activeToolDockerMap;
    QMap<QString, bool> toolDockerRaisedMap;
    void loadDocker(const QString& _name, bool visible);
    void removeDockers();
};

void KoDockerManager::Private::loadDocker(const QString &name, bool visible)
{
    ToolDockerFactory factory(name);
    KoToolDocker *td = qobject_cast<KoToolDocker*>(mainWindow->createDockWidget(&factory));
    Q_ASSERT(td);
    if (td == 0) return;
    toolDockerMap[name] = td;
    toolDockerVisibilityMap[name] = visible;
    toolDockerRaisedMap[name] = false;
    td->setVisible(false);
    td->setEnabled(false);
    td->toggleViewAction()->setVisible(false);
}

void KoDockerManager::Private::removeDockers()
{
    // First remove the previous active dockers from sight and docker menu
    QMapIterator<QString, KoToolDocker *> iter(activeToolDockerMap);
    while (iter.hasNext()) {
        iter.next();

        // Check if the dock is raised or not
        QList<QDockWidget*> tabedDocks = mainWindow->tabifiedDockWidgets(iter.value());
        bool isOnTop = true;
        int idx = mainWindow->children().indexOf(iter.value());
        foreach (QDockWidget* dock, tabedDocks) {
            if (mainWindow->children().indexOf(dock) > idx && dock->isVisible() && dock->isEnabled()) {
                isOnTop = false;
                break;
            }
        }
        toolDockerRaisedMap[iter.key()] = isOnTop;
        //kDebug() << iter.value() << " " << iter.value()->isVisible() << iter.key();
        iter.value()->toggleViewAction()->setVisible(false);
        toolDockerVisibilityMap[iter.key()] = iter.value()->isVisible();
        iter.value()->setVisible(false);
        iter.value()->setEnabled(false);
    }
    activeToolDockerMap.clear();
}

KoDockerManager::KoDockerManager(KoMainWindow *mainWindow)
    : QObject(mainWindow), d( new Private() )
{
    d->mainWindow = mainWindow;

    KConfigGroup cfg = KGlobal::config()->group("DockerManager");

    QStringList visibleList = cfg.readEntry("VisibleToolDockers", QStringList());

    QStringListIterator iter(visibleList);
    while (iter.hasNext()) {
        QString name = iter.next();
        //kDebug() << "name = " << name;
        d->loadDocker(name, true);
        //kDebug() << "visible = " << d->toolDockerVisibilityMap.value(name);
    }
    QStringList hiddenList = cfg.readEntry("HiddenToolDockers", QStringList());

    QStringListIterator j2(hiddenList);
    while (j2.hasNext()) {
        QString name = j2.next();
        d->loadDocker(name, false);
    }
}

KoDockerManager::~KoDockerManager()
{
    KConfigGroup cfg = KGlobal::config()->group("DockerManager");
    QStringList visibleList;
    QStringList hiddenList;
    QMapIterator<QString, KoToolDocker *> iter(d->toolDockerMap);
    while (iter.hasNext()) {
        iter.next();
        if (d->toolDockerVisibilityMap.value(iter.key())) {
            visibleList += iter.key();
        } else {
            hiddenList += iter.key();
        }
    }
    //kDebug() << "visibleList = " << visibleList;
    //kDebug() << "hiddenList = " << hiddenList;
    cfg.writeEntry("VisibleToolDockers", visibleList);
    cfg.writeEntry("HiddenToolDockers", hiddenList);
    cfg.sync();
    d->removeDockers();
    delete d;
}

void KoDockerManager::removeUnusedOptionWidgets()
{
    QMapIterator<QString, KoToolDocker *> iter(d->toolDockerMap);
    while (iter.hasNext()) {
        iter.next();
        if (! d->activeToolDockerMap.contains(iter.key())) {
            //kDebug(30004) << "removing" << iter.key() << ((void*) iter.value());
            iter.value()->setVisible(false);
            iter.value()->setEnabled(false);
            iter.value()->toggleViewAction()->setVisible(false);
        } else {
            iter.value()->setVisible(d->toolDockerVisibilityMap[iter.key()]);
            iter.value()->setEnabled(true);
            iter.value()->toggleViewAction()->setVisible(true);
        }
    }
}

void KoDockerManager::newOptionWidgets(const QMap<QString, QWidget *> &optionWidgetMap)
{
    d->removeDockers();

    // Now show new active dockers (maybe even create) and show in docker menu
    QMap<QString, QWidget*>::ConstIterator iter = optionWidgetMap.constBegin();
    for (;iter != optionWidgetMap.constEnd(); ++iter) {
        const QString name = iter.value()->objectName();
        if (name.isEmpty()) {
            kError(30004) << "tooldocker widget have no name " << iter.key();
            Q_ASSERT(!name.isEmpty());
            continue; // skip this docker in release build when assert don't crash
        }

        KoToolDocker *td = d->toolDockerMap.value(name);
        if (!td) {
            ToolDockerFactory factory(name);
            td = qobject_cast<KoToolDocker*>(d->mainWindow->createDockWidget(&factory));
            Q_ASSERT(td);
            d->toolDockerMap[name] = td;
            d->toolDockerVisibilityMap[name] =  true;
        }
        td->setEnabled(true);
        td->setWindowTitle(iter.key());
        td->newOptionWidget(iter.value());
        d->mainWindow->restoreDockWidget(td);
        //kDebug() << name << " " << d->toolDockerVisibilityMap[name];
        td->setVisible(d->toolDockerVisibilityMap[name]);
        //kDebug() << td->isVisible();
        td->toggleViewAction()->setVisible(true);
        d->activeToolDockerMap[name] = td;
        if (d->toolDockerRaisedMap.value(name)) {
            td->raise();
        }
    }
}

#include <KoDockerManager.moc>
