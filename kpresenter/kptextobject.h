/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef kptextobject_h
#define kptextobject_h

#include <kpobject.h>
#include "qrichtext_p.h"
#include <kotextview.h>


class KPresenterView;
class KPresenterDoc;
class QDomElement;
class TextCmd;
class KPGradient;

class KoTextView;
class KoTextObject;
class KPTextView;
class KCommand;
class KPrCanvas;
class KoPoint;
class KoVariable;
class KPrTextDrag;
class KPrTextDocument;
class DCOPObject;
class KPrPage;

/******************************************************************/
/* Class: KPTextObject                                            */
/******************************************************************/

class KPTextObject :  public QObject, public KP2DObject
{
    Q_OBJECT
public:
    KPTextObject( KPresenterDoc *doc );
    virtual ~KPTextObject();

    virtual DCOPObject* dcopObject();

    virtual void setSize( double _width, double _height );
    virtual void resizeBy( double _dx, double _dy );

    virtual void setDrawEditRect( bool b )
    { drawEditRect = b; }
    virtual void setDrawEmpty( bool b )
    { drawEmpty = b; }

    virtual ObjType getType() const
    { return OT_TEXT; }
    virtual QString getTypeString() const
    { return i18n("Text"); }

    virtual bool getDrawEditRect() const
    { return drawEditRect; }
    virtual bool getDrawEmpty() const
    { return drawEmpty; }
    virtual int getSubPresSteps() const;

    virtual QBrush getBrush() const;

    void setShadowParameter(int _distance,ShadowDirection _direction,QColor _color);

    virtual QDomDocumentFragment save( QDomDocument& doc, double offset );
    virtual double load(const QDomElement &element);

    virtual void paint( QPainter *_painter, KoZoomHandler*_zoomHandler,
			bool drawingShadow, bool drawContour );
    void paint( QPainter *_painter, KoZoomHandler*_zoomHandler, bool onlyChanged, QTextCursor* cursor, bool resetChanged, bool drawingShadow,bool drawContour );
    void paintEdited( QPainter *_painter, KoZoomHandler*_zoomHandler,
                      bool onlyChanged, QTextCursor* cursor, bool resetChanged );

    void drawCursor( QPainter *p, QTextCursor *cursor, bool cursorVisible, KPrCanvas* /*canvas*/ );

    virtual void extendObject2Contents( KPresenterView *view = 0 );

    void recalcPageNum( KPresenterDoc *doc, KPrPage *page );


    /** Return the contained text object */
    KoTextObject * textObject() const { return m_textobj; }

    KPrTextDocument *textDocument() const;

    KPresenterDoc* kPresenterDocument() const { return m_doc; }

    KPTextView * createKPTextView( KPrCanvas * );

    void removeHighlight ();
    void highlightPortion( Qt3::QTextParag * parag, int index, int length, KPrCanvas */*_canvas*/ );

    KCommand * pasteKPresenter( QTextCursor * cursor, const QCString & data, bool removeSelected );

    void saveParagraph( QDomDocument& doc,
			KoTextParag * parag,
			QDomElement &parentElem,
			int from /* default 0 */,
			int to /* default length()-2 */ );
    KoParagLayout loadParagLayout( QDomElement & parentElem);

    KoTextFormat loadFormat( QDomElement &n );

signals:
    void repaintChanged( KPTextObject* );

protected slots:
    void slotFormatChanged(const KoTextFormat &);

protected:
    virtual QDomElement saveKTextObject( QDomDocument& doc );
    QDomElement saveHelper(const QString &tmpText,KoTextFormat*lastFormat ,QDomDocument &doc);

    virtual void loadKTextObject( const QDomElement &e, int type );
    void drawText( QPainter* _painter, KoZoomHandler* zoomHandler, bool onlyChanged, QTextCursor* cursor, bool resetChanged );
    void drawParags( QPainter *p, KoZoomHandler* zoomHandler, const QColorGroup& cg, int from, int to );
    void saveParagLayout( const KoParagLayout& layout, QDomElement & parentElem );

protected slots:
    void slotNewCommand( KCommand *cmd );
    void slotAvailableHeightNeeded();
    void slotRepaintChanged();
private:
    KPTextObject &operator=( const KPTextObject & );

    static const QString &tagTEXTOBJ, &attrLineSpacing, &attrParagSpacing,
                           &attrMargin, &attrBulletType1, &attrBulletType2,
                           &attrBulletType3, &attrBulletType4, &attrBulletColor1,
                           &attrBulletColor2, &attrBulletColor3, &attrBulletColor4,
                           &attrObjType, &tagP, &attrAlign, &attrType,
                           &attrDepth, &tagTEXT, &attrFamily, &attrPointSize,
                           &attrBold, &attrItalic, & attrUnderline,& attrStrikeOut,&attrColor, &attrWhitespace, &attrTextBackColor, &attrVertAlign, &attrLinkName, &attrHrefName;

    /** The contained text object */
    KoTextObject *m_textobj;
    KPresenterDoc *m_doc;
    bool drawEditRect, drawEmpty;
    KoParagLayout m_paragLayout;
};


class KPTextView : public KoTextView
{
Q_OBJECT
public:
    KPTextView( KPTextObject * txtObj, KPrCanvas *_canvas );
    virtual ~KPTextView();

    virtual KoTextViewIface* dcopObject();

    KoTextView * textView() { return this; }
    KPTextObject * kpTextObject() { return m_kptextobj; }

    void keyPressEvent( QKeyEvent * );
    void keyReleaseEvent( QKeyEvent * );
    void mousePressEvent( QMouseEvent *, const QPoint &);

    void mouseMoveEvent( QMouseEvent *, const QPoint & );
    void mouseReleaseEvent( QMouseEvent *, const QPoint & );
    void mouseDoubleClickEvent( QMouseEvent *, const QPoint &);
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent *, const QPoint & );
    void dragLeaveEvent( QDragLeaveEvent * );
    void dropEvent( QDropEvent * );

    void clearSelection();
    void selectAll();
    virtual void drawCursor( bool b );

    const KoParagLayout & currentParagLayout() const { return m_paragLayout; }
    void showPopup( KPresenterView *view, const QPoint &point, QPtrList<KAction> &actionList );
    void insertVariable( int type, int subtype = 0 );
    void insertCustomVariable( const QString &name);
    void insertLink(const QString &_linkName, const QString & hrefName);
    void insertVariable( KoVariable *var);
    void terminate();

public slots:
    void cut();
    void copy();
    void paste();
    // Reimplemented from KoTextView
    virtual void updateUI( bool updateFormat, bool force = false );
    virtual void ensureCursorVisible();


protected slots:
    virtual void startDrag();

    KPrTextDrag * newDrag( QWidget * parent ) const ;

protected:
    // Reimplemented from KoTextView
    virtual void doAutoFormat( QTextCursor* cursor, KoTextParag *parag, int index, QChar ch );
    virtual void showFormat( KoTextFormat *format ) ;

    virtual void pgUpKeyPressed();
    virtual void pgDownKeyPressed();

    KPTextObject *m_kptextobj;
    KPrCanvas *m_canvas;
    KoParagLayout m_paragLayout;
    QPtrList<KAction> m_actionList; // for the kodatatools
};


#endif
