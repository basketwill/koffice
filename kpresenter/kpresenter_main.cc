/******************************************************************/
/* KPresenter - (c) by Reginald Stadlbauer 1998                   */
/* Version: 0.1.0                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* needs c++ library Qt (http://www.troll.no)                     */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* written for KDE (http://www.kde.org)                           */
/* KPresenter is under GNU GPL                                    */
/******************************************************************/
/* Module: KPresenter Application                                 */
/******************************************************************/

#include "kpresenter_main.h"
#include "kpresenter_main.moc"
#include "kpresenter_doc.h"
#include <koFactory.h>
#include <koDocument.h>
#include <opAutoLoader.h>
#include <kimgio.h>
#include <kstddirs.h>

KOFFICE_DOCUMENT_FACTORY( KPresenterDoc, KPresenterFactory, KPresenter::DocumentFactory_skel )
typedef OPAutoLoader<KPresenterFactory> KPresenterAutoLoader;

/******************************************************************/
/* class KPresenterApp - KPresenter Application                   */
/******************************************************************/

/*====================== constrcutor =============================*/
KPresenterApp::KPresenterApp( int &argc, char** argv )
    : KoApplication( argc, argv, "kpresenter" )
{
}

/*====================== destrcutor ==============================*/
KPresenterApp::~KPresenterApp()
{
}

/*======================== main ==================================*/
int main( int argc, char **argv )
{
    KPresenterAutoLoader loader( "IDL:KPresenter/DocumentFactory:1.0", "KPresenter" );

    KPresenterApp app( argc, argv );
    KGlobal::dirs()->addResourceType("kpresenter_template", 
		     KStandardDirs::kde_default("data") + "kpresenter/templates/");
    KGlobal::dirs()->addResourceType("autoforms", 
				     KStandardDirs::kde_default("data") + "kpresenter/autoforms/");
    KGlobal::dirs()->addResourceType("toolbar", 
				     KStandardDirs::kde_default("data") + "kpresenter/toolbar/");

    kimgioRegister();

    app.exec();

    return 0;
}

