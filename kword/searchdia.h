/******************************************************************/ 
/* KWord - (c) by Reginald Stadlbauer and Torben Weis 1997-1998   */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer, Torben Weis                       */
/* E-Mail: reggie@kde.org, weis@kde.org                           */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Search Dialog (header)                                 */
/******************************************************************/

#ifndef search_h
#define search_h

#include <stdlib.h>

#include <qtabdialog.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcolor.h>
#include <qstrlist.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qevent.h>

#include <kapp.h>
#include <kcolorbtn.h>
#include <kbuttonbox.h>

#include "format.h"

class KWordDocument;
class KWPage;
class KWordView;

/******************************************************************/
/* Class: KWSearchDia                                             */
/******************************************************************/

class KWSearchDia : public QTabDialog
{
  Q_OBJECT

public:
  struct KWSearchEntry
  {
    KWSearchEntry() {
      checkFamily = checkColor = checkSize = checkBold = checkItalic = checkUnderline = checkVertAlign = false;
      expr = "";
      family = "times";
      color = black;
      size = 12;
      bold = italic = underline = false;
      vertAlign = KWFormat::VA_NORMAL;
      caseSensitive = false;
      regexp = false;
      reverse = false;
      wholeWords = false;
      wildcard = false;
    }

    QString expr;
    bool checkFamily,checkColor,checkSize,checkBold,checkItalic,checkUnderline,checkVertAlign;
    QString family;
    QColor color;
    int size;
    bool bold,italic,underline;
    KWFormat::VertAlign vertAlign;
    bool caseSensitive,regexp,reverse,wholeWords,wildcard;
  };

  KWSearchDia(QWidget *parent,const char *name,KWordDocument *_doc,KWPage *_page,KWordView *_view,
	      KWSearchEntry *_searchEntry,KWSearchEntry *_replaceEntry,QStrList _fontlist);

protected:
  void setupTab1();
  void setupTab2();
  void closeEvent(QCloseEvent *e) { emit cancelButtonPressed(); }

  QWidget *tab1;
  QGridLayout *grid1,*sGrid;
  QGroupBox *gSearch;
  QCheckBox *cRegExp,*cFamily,*cSize,*cColor,*cBold,*cItalic,*cUnderline,*cVertAlign,*cmBold,*cmItalic,*cmUnderline,*cCase,*cWholeWords,*cRev;
  QComboBox *cmFamily,*cmSize,*cmVertAlign;
  KColorButton *bColor;
  QLabel *lSearch;
  KButtonBox *bbSearch;
  QPushButton *bSearchFirst,*bSearchNext,*bSearchAll;
  QLineEdit *eSearch;

  QWidget *tab2;
  QGridLayout *grid2,*rGrid;
  QGroupBox *gReplace;
  QCheckBox *rcFamily,*rcSize,*rcColor,*rcBold,*rcItalic,*rcUnderline,*rcVertAlign,*rcmBold,*rcmItalic,*rcmUnderline,*cAsk,*cWildcard;
  QComboBox *rcmFamily,*rcmSize,*rcmVertAlign;
  KColorButton *rbColor;
  QLabel *lReplace;
  KButtonBox *bbReplace;
  QPushButton *bReplaceFirst,*bReplaceNext,*bReplaceAll;
  QLineEdit *eReplace;

  KWordDocument *doc;
  KWPage *page;
  KWordView *view;
  KWSearchEntry *searchEntry,*replaceEntry;
  QStrList fontlist;

protected slots:
  void searchFirst();
  void searchNext();
  void slotCheckFamily();
  void slotCheckColor();
  void slotCheckSize();
  void slotCheckBold();
  void slotCheckItalic();
  void slotCheckUnderline();
  void slotCheckVertAlign();
  void slotFamily(const char*);
  void slotSize(const char*);
  void slotColor(const QColor&);
  void slotBold();
  void slotItalic();
  void slotUnderline();
  void slotVertAlign(int);
  void replaceFirst();
  void replaceNext();
  void replaceAll();
  void rslotCheckFamily();
  void rslotCheckColor();
  void rslotCheckSize();
  void rslotCheckBold();
  void rslotCheckItalic();
  void rslotCheckUnderline();
  void rslotCheckVertAlign();
  void rslotFamily(const char*);
  void rslotSize(const char*);
  void rslotColor(const QColor&);
  void rslotBold();
  void rslotItalic();
  void rslotUnderline();
  void rslotVertAlign(int);
  void saveSettings();
  void slotRegExp();

};

#endif


