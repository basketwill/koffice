/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
                 2001       Sven Leiber         <s.leiber@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef koautoformatdia_h
#define koautoformatdia_h

#include <kdialogbase.h>
#include <qlineedit.h>
#include <koAutoFormat.h>

class QPushButton;
class QCheckBox;
class QVBox;
class QHBox;
class KCharSelect;
class KListView;
class KLineEdit;
class QListBox;
class QSpinBox;
class KIntNumInput;
/**
 *
 */
class KoAutoFormatLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    KoAutoFormatLineEdit ( QWidget * parent, const char * name=0 );

protected:
    virtual void keyPressEvent ( QKeyEvent * );
 signals:
    void keyReturnPressed();
};

/******************************************************************/
/* Class: KoAutoFormatExceptionWidget                             */
/******************************************************************/

class KoAutoFormatExceptionWidget : public QWidget
{
    Q_OBJECT

public:
    KoAutoFormatExceptionWidget(QWidget *_parent, const QString &name,const QStringList &_list, bool _autoInclude, bool _abbreviation=false);

    void setListException( const QStringList &list);
    QStringList getListException(){return m_listException;}
    bool autoInclude();
    void setAutoInclude( bool b );
protected slots:
    void slotAddException();
    void slotRemoveException();
    void textChanged ( const QString & );
    void slotExceptionListSelected();
 private:
    QListBox *exceptionList;
    KoAutoFormatLineEdit *exceptionLine;
    QPushButton *pbAddException,*pbRemoveException;
    QCheckBox *cbAutoInclude;
    QStringList m_listException;
    bool m_bAbbreviation;
};

/******************************************************************/
/* Class: KoAutoFormatDia                                         */
/******************************************************************/

class KoAutoFormatDia : public KDialogBase
{
    Q_OBJECT

public:
    KoAutoFormatDia( QWidget *parent, const char *name, KoAutoFormat * autoFormat );
    void addEntryList(const QString &key, KoAutoFormatEntry &_autoEntry);
    void editEntryList(const QString &key,const QString &newFindString, KoAutoFormatEntry &_autoEntry);
protected:
    bool applyConfig();
    void setupTab1();
    void setupTab2();
    void setupTab3();
    void setupTab4();

    void refreshEntryList();

    void initTab1();
    void initTab2();
    void initTab3();
    void initTab4();

    QWidget *tab1;
    QWidget *tab2;
    QWidget *tab3;
    QWidget *tab4;

    QCheckBox *cbTypographicSimpleQuotes, *cbTypographicDoubleQuotes,*cbUpperCase, *cbUpperUpper, *cbDetectUrl,
        *cbBackgroundSpellCheck, *cbAdvancedAutoCorrection,*cbIgnoreDoubleSpace , *cbRemoveSpaceBeginEndLine, *cbUseBulletStyle, *cbAutoChangeFormat, *cbAutoReplaceNumber, *cbUseNumberStyle, *cbAutoSuperScript;
    QPushButton *pbDoubleQuote1, *pbDoubleQuote2, /**pbEdit,*/ *pbRemove, *pbAdd, *pbDoubleDefault,
                *pbSpecialChar1, *pbSpecialChar2, *pbBulletStyle, *pbDefaultBulletStyle;

    QPushButton *pbSimpleQuote1, *pbSimpleQuote2, *pbSimpleDefault;

    KCharSelect *charselect;
    KoAutoFormatLineEdit *m_find, *m_replace;
    KListView *m_pListView;

    QChar oSimpleBegin, oSimpleEnd;
    QChar oDoubleBegin, oDoubleEnd;

    QChar bulletStyle;
    KoAutoFormat m_autoFormat; // The copy we're working on
    KoAutoFormat * m_docAutoFormat; // Pointer to the real one (in KWDocument)

    KoAutoFormatExceptionWidget *abbreviation;
    KoAutoFormatExceptionWidget *twoUpperLetter;

protected slots:
    virtual void slotOk();
    void slotItemRenamed(QListViewItem * item, const QString & newText, int column);
    void slotRemoveEntry();
    void slotEditEntry();
    void slotfind( const QString & );
    void slotfind2( const QString & );
    void chooseSpecialChar1();
    void chooseSpecialChar2();
    void chooseDoubleQuote1();
    void chooseDoubleQuote2();
    void defaultDoubleQuote();

    void chooseSimpleQuote1();
    void chooseSimpleQuote2();
    void defaultSimpleQuote();

    void slotChangeStateSimple(bool);
    void slotChangeStateDouble(bool);

    void slotAddEntry();
    void chooseBulletStyle();
    void defaultBulletStyle();

    void slotResetConf();

};

class KoCompletionDia : public KDialogBase
{
    Q_OBJECT

public:
    KoCompletionDia( QWidget *parent, const char *name, KoAutoFormat * autoFormat );
protected slots:
    virtual void slotOk();
    void slotRemoveCompletionEntry();
    void slotCompletionWordSelected( const QString & );
    void slotSaveCompletionEntry();
    void slotResetConf();
    bool applyConfig();

protected:
    void setup();

    KoAutoFormat m_autoFormat; // The copy we're working on
    KoAutoFormat * m_docAutoFormat; // Pointer to the real one (in KWDocument)
    QStringList m_listCompletion; // The copy of the completion items - don't use m_autoFormat.getCompletion()!
    QCheckBox *cbAllowCompletion, *cbAppendSpace, *cbAddCompletionWord;
    QPushButton *pbRemoveCompletionEntry;
    QPushButton *pbSaveCompletionEntry;
    QListBox *m_lbListCompletion;
    KIntNumInput *m_minWordLength;
    KIntNumInput *m_maxNbWordCompletion;

};
#endif


