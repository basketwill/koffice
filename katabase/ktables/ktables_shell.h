/***************************************************************************
                          ktables.h  -  description                              
                             -------------------                                         
    begin                : Mi� J�l  7 17:04:49 CEST 1999
                                           
    copyright            : (C) 1999 by �rn E. Hansen                         
    email                : hanseno@mail.bip.net                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KTABLES_H
#define KTABLES_H
 

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


// include files for KDE 
#include <koMainWindow.h>
#include <klocale.h>
#include <kglobal.h>
#include <kaccel.h>

// forward declaration of the Ktables classes
class KtablesDoc;
class KtablesView;
class KtablesServer;
class TableSelect;
class QueryDialog;

/**
  * The base class for Ktables application windows. It sets up the main
  * window and reads the config file as well as providing a menubar, toolbar
  * and statusbar. An instance of KtablesView creates your center view, which is connected
  * to the window's Doc object.
  * KtablesApp reimplements the methods that KTMainWindow provides for main window handling and supports
  * full session management as well as keyboard accelerator configuration by using KAccel.
	* @see KTMainWindow
	* @see KApplication
	* @see KConfig
	* @see KAccel
	*
	* @author Source Framework Automatically Generated by KDevelop, (c) The KDevelop Team.
	* @version KDevelop version 0.4 code generation
  */
class KtablesApp : public KoMainWindow
{
  Q_OBJECT

  friend class KtablesView;

public:
  KtablesApp();
  ~KtablesApp();
  virtual void cleanUp();

	void setDocument(KtablesDoc *);	
	virtual bool newDocument();
	virtual bool openDocument(const char *, const char *);
	virtual bool saveDocument(const char *, const char *);
	virtual bool closeDocument();
	virtual bool closeAllDocuments();
	virtual void releaseDocument();

	virtual void createFileMenu(OPMenuBar *);
	
protected:
  virtual KOffice::Document_ptr document();
  virtual KOffice::View_ptr view();

  virtual void helpAbout();

	void setCaption(const QString&);
  void initToolBar();
  void initStatusBar();

 public slots:
  void slotFileNewWindow();
  void slotFileNew();
  void slotFileOpen();
	void slotFileOpenRecent(int id_);
  void slotFileSave();
  void slotFileSaveAs();
  void slotFileClose();
  void slotFilePrint();
  void slotFileQuit();
  void slotEditCut();
  void slotEditCopy();
  void slotEditPaste();
  void slotViewToolBar();
  void slotViewStatusBar();
  void slotStatusMsg(const char *text);
  void slotStatusHelpMsg(const char *text);

protected:
	KAccel      *key_accel;
  OPMenu      *m_pEditMenu;
  OPMenu      *m_pViewMenu;
  OPMenu      *m_pQueryMenu;
  KtablesView *m_pView;
  KtablesDoc  *m_pDoc;

};
 
#endif // KTABLES_H

















































