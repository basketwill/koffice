/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __kspread_gui_h__
#define __kspread_gui_h__

class KSpreadView;
class KSpreadEditWidget;
class KSpreadCanvas;
class KSpreadHBorder;
class KSpreadVBorder;
class KSpreadScripts;
class KSpreadTable;
class KSpreadDoc;
class KSpreadPaperLayout;
class KSpreadChildPicture;
class KSpreadChildFrame;
class KSpreadShell;
class KSpreadTabBar;
class KSpreadEditWidget;
class KSpreadCanvas;
class KSpreadHBorder;
class KSpreadVBorder;
class KSpreadChild;
class KSpreadCell;

class KoDocumentEntry;

class KColorAction;

class KAction;
class KSelectAction;
class KFontAction;
class KFontSizeAction;
class KToggleAction;

#include <qlist.h>
#include <qscrbar.h>
#include <qlabel.h>
#include <qbutton.h>
#include <qpoint.h>

#include <container.h>

#include <koDataTool.h>

/**
 */
class KSpreadView : public ContainerView
{
    friend KSpreadCanvas;

    Q_OBJECT
public:
    KSpreadView( QWidget *_parent, const char *_name, KSpreadDoc *_doc );
    ~KSpreadView();

    KSpreadCanvas* canvasWidget() { return m_pCanvas; }
    KSpreadHBorder* hBorderWidget() { return m_pHBorderWidget; }
    KSpreadVBorder* vBorderWidget() { return m_pVBorderWidget; }
    QScrollBar* horzScrollBar() { return m_pHorzScrollBar; }
    QScrollBar* vertScrollBar() { return m_pVertScrollBar; }
    KSpreadEditWidget* editWidget() { return m_pEditWidget; }
    QLabel* posWidget() { return m_pPosWidget; }

    KSpreadDoc* doc() { return m_pDoc; }

    void addTable( KSpreadTable *_t );
    void removeTable( KSpreadTable *_t );
    void removeAllTables();
    void setActiveTable( KSpreadTable *_t );

    KSpreadTable* activeTable() { return m_pTable; }
    KSpreadTabBar* tabBar() { return  m_pTabBar;}

    void openPopupMenu( const QPoint &_global );
    void popupRowMenu(const QPoint & _point ) ;
    void popupColumnMenu( const QPoint & _point);

    void showFormulaToolBar( bool show );

    /**
     * Used by @ref KSpreadEditWidget. Sets the text of the active cell.
     */
    void setText( const QString& _text );

    void enableUndo( bool _b );
    void enableRedo( bool _b );

    /**
     * Called by @ref KSpreadInsertHandler
     *
     * @param _geometry is the zoomed geometry of the new child.
     */
    void insertChart( const QRect& _geometry, KoDocumentEntry& _entry );
    /**
     * Called by @ref KSpreadInsertHandler
     *
     * @param _geometry is the geometry of the new child.
     */
    void insertChild( const QRect& _geometry, KoDocumentEntry& _entry );

    virtual bool printDlg();

    QWidget* canvas();
    void paintContent( QPainter& painter, const QRect& rect, bool transparent );

    /**
     * Enables/Disables all actions of the formula toolbar.
     */
    void enableFormulaToolBar( bool );

    /**
     * Fills the @ref KSpreadEditWidget with the current cells
     * content. This function is usually called after the
     * cursor moved.
     */
    void updateEditWidget();

public slots:
    /**
     * Action
     */
    void transformPart();
    /**
     * Menu Edit->Copy
     */
    void copySelection();
    /**
     * Menu Edit->Cut
     */
    void cutSelection();
    /**
     * Menu Edit->Patse
     */
    void paste();
    /**
     * Menu Edit->Special Paste
     */
    void specialPaste();

    /**
     * Menu Edit->Cell
     */
    void editCell();
    /**
     * Menu Edit->Undo
     */
    void undo();
    /**
     * Menu Edit->Redo
     */
    void redo();
    /**
     * Menu Edit->Page Layout
     */
    void paperLayoutDlg();
    /**
     * Menu Edit->Insert->Object
     */
    void insertObject();


    /**
     * Menu Scripts->Edit Global Scripts
     */
    void editGlobalScripts();
    /**
     * Menu Scripts->Edit Local Script
     */
    void editLocalScripts();
    /**
     * Menu Scripts->Reload Scripts
     */
    void reloadScripts();
    /**
     * Menu Scripts->Run Local Script
     */
    void runLocalScript();

    /**
     * Menu View->New View
     */
    void newView();
    /**
     * Menu View->Show Page Borders
     */
    void togglePageBorders();
    /**
     * Menu Data
     */
     void gotoCell();
    /**
     * Menu Data
     */
     void replace();
    /**
     * Menu Data
     */
      void sort();
    /**
     * Menu Data
     */
      void createAnchor();
    /**
     * Menu Data
     */
    void consolidate();

    /**
     * Menu Folder
     */
    void insertTable();
     /**
     * Menu Folder
     */
     void removeTable();
     /**
     * Menu Folder
     */
     void hideTable();
     /**
     * Menu Folder
     */
     void showTable();
    /**
     * Menu for help menu
     */
    void helpUsing();

    /**
     * ToolBar
     */
    // void print();
    /**
     * ToolBar
     */
    void insertChart();

    /**
     * ToolBar
     */
  // void zoomMinus();
    /**
     * ToolBar
     */
  // void zoomPlus();

    /**
     * ToolBar
     */
    void moneyFormat();

    /**
     * ToolBar
     */
    void alignLeft( bool b );
    /**
     * ToolBar
     */
    void alignRight( bool b );
    /**
     * ToolBar
     */
    void alignCenter( bool b );
    /**
     * ToolBar
     */
    void multiRow( bool b );

    /**
     * ToolBar
     */
    void precisionMinus();
    /**
     * ToolBar
     */
    void precisionPlus();

    /**
     * ToolBar
     */
    void percent();

    /**
     * ToolBar
     */
    void fontSelected( const QString &_font );
    /**
     * ToolBar
     */
    void fontSizeSelected( int size );
    /**
     * ToolBar
     */
    void bold( bool b );
    /**
     * ToolBar
     */
    void italic( bool b );

    /**
     * ToolBar
     */
    void deleteColumn();
    /**
     * ToolBar
     */
    void deleteRow();
    /**
     * ToolBar
     */
    void insertColumn();
    /**
     * ToolBar
     */
    void insertRow();
    /**
     *Toolbar
     */

    void formulaSelection( const QString &_math );

    void changeTextColor();
    void changeBackgroundColor();
    void sortInc();
    void sortDec();

    void layoutDlg();
    void funct();
    void formulaPower();
    void formulaSubscript();
    void formulaParentheses();
    void formulaAbsValue();
    void formulaBrackets();
    void formulaFraction();
    void formulaRoot();
    void formulaIntegral();
    void formulaMatrix();
    void formulaLeftSuper();
    void formulaLeftSub();
    void formulaSum();
    void formulaProduct();
    void borderBottom();
    void borderRight();
    void borderLeft();
    void borderTop();
    void borderOutline();
    void borderAll();
    void borderRemove();
    void changeBorderColor();

    /**
     * @ref #tabBar is connected to this slot.
     * When the user selects a new table using the @ref #tabBar this slot
     * is signaled.
     */
    void changeTable( const QString& _name );

protected slots:
    // C++
    /**
     * Popup menu
     */
    void slotActivateTool( int _id );
    /**
     * Popup menu
     */
    void slotCopy();
    /**
     * Popup menu
     */
    void slotCut();
    /**
     * Popup menu
     */
    void slotPaste();
    /**
     * Popup menu
     */
     void slotSpecialPaste();
    /**
     * Popup menu
     */
    void slotDelete();
    /**
     * Popup menu
     */
    void slotAjust();
    /**
     * Popup menu
     */
     void slotClear();
     /**
     * Popup menu
     */
     void slotInsert();
     /**
     * Popup menu
     */
     void slotRemove();
    /**
     * Scroll @ref #tabBar.
     */
    void slotScrollToFirstTable();
    /**
     * Scroll @ref #tabBar.
     */
    void slotScrollToLeftTable();
    /**
     * Scroll @ref #tabBar.
     */
    void slotScrollToRightTable();
    /**
     * Scroll @ref #tabBar.
     */
    void slotScrollToLastTable();

    void slotInsertRow();

    void slotRemoveRow();

    void slotInsertColumn();

    void slotRemoveColumn();

    void slotResizeColumn();
    void slotResizeRow();
    void slotAjustColumn() ;
    void slotAjustRow();

protected slots:
    void repaintPolygon( const QPointArray& );

    void slotChildSelected( PartChild* ch );
    void slotChildUnselected( PartChild* );

public slots:
    // Document signals
    void slotUnselect( KSpreadTable *_table, const QRect& _old );
    void slotUpdateView( KSpreadTable *_table );
    void slotUpdateView( KSpreadTable *_table, const QRect& );
    void slotUpdateCell( KSpreadTable *_table, KSpreadCell* _cell, int _col, int _row );
    void slotUpdateHBorder( KSpreadTable *_table );
    void slotUpdateVBorder( KSpreadTable *_table );
    void slotChangeSelection( KSpreadTable *_table, const QRect &_old, const QRect &_new );
    void slotAddTable( KSpreadTable *_table );
    void slotInsertChild( KSpreadChild *_child );
    void slotRemoveChild( KSpreadChild *_child );
    void slotUpdateChildGeometry( KSpreadChild *_child );

    virtual int leftBorder() const;
    virtual int rightBorder() const;
    virtual int topBorder() const;
    virtual int bottomBorder() const;

signals:
    void sig_selectionChanged( KSpreadTable* _table, const QRect& _selection );

protected:
    bool eventKeyPressed( QKeyEvent* _event );
	
    virtual void keyPressEvent ( QKeyEvent * _ev );
    virtual void resizeEvent( QResizeEvent *_ev );

    virtual QWMatrix matrix() const;

    /**
     * Activates the formula editor for the current cell.
     * This function is usually called if the user presses
     * a button in the formula toolbar.
     */
    void activateFormulaEditor();

private:
    // GUI stuff
    QButton* newIconButton( const char *_file, bool _kbutton = false, QWidget *_parent = 0L );

    QScrollBar *m_pHorzScrollBar;
    QScrollBar *m_pVertScrollBar;
    KSpreadCanvas *m_pCanvas;
    KSpreadVBorder *m_pVBorderWidget;
    KSpreadHBorder *m_pHBorderWidget;
    KSpreadEditWidget *m_pEditWidget;
    QWidget *m_pFrame;
    QFrame *m_pToolWidget;
    QButton *m_pTabBarFirst;
    QButton *m_pTabBarLeft;
    QButton *m_pTabBarRight;
    QButton *m_pTabBarLast;
    QButton *m_pOkButton;
    QButton *m_pCancelButton;
    KSpreadTabBar *m_pTabBar;
    QLabel *m_pPosWidget;

    KToggleAction* m_bold;
    KToggleAction* m_italic;
    KAction* m_percent;
    KAction* m_precplus;
    KAction* m_precminus;
    KAction* m_money;
    KToggleAction* m_alignLeft;
    KToggleAction* m_alignCenter;
    KToggleAction* m_alignRight;
    KAction* m_insertPart;
    KAction* m_transform;
    KAction* m_copy;
    KAction* m_paste;
    KAction* m_cut;
    KAction* m_specialPaste;
    KAction* m_editCell;
    KAction* m_undo;
    KAction* m_redo;
    KAction* m_paperLayout;
    KAction* m_insertTable;
    KAction* m_removeTable;
    KAction* m_editGlobalScripts;
    KAction* m_editLocalScripts;
    KAction* m_reloadScripts;
    KAction* m_newView;
    KAction* m_gotoCell;
    KAction* m_replace;
    KAction* m_sort;
    KAction* m_createAnchor;
    KAction* m_consolidate;
    KAction* m_help;
    KAction* m_insertChart;
    KToggleAction* m_multiRow;
    KFontAction* m_selectFont;
    KFontSizeAction* m_selectFontSize;
    KAction* m_deleteColumn;
    KAction* m_deleteRow;
    KAction* m_insertColumn;
    KAction* m_insertRow;
    KAction* m_formulaPower;
    KAction* m_formulaSubscript;
    KAction* m_formulaParantheses;
    KAction* m_formulaAbsValue;
    KAction* m_formulaBrackets;
    KAction* m_formulaFraction;
    KAction* m_formulaRoot;
    KAction* m_formulaIntegral;
    KAction* m_formulaMatrix;
    KAction* m_formulaLeftSuper;
    KAction* m_formulaLeftSub;
    KAction* m_formulaSum;
    KAction* m_formulaProduct;
    KSelectAction* m_formulaSelection;
    KAction* m_sortDec;
    KAction* m_sortInc;
    KColorAction* m_textColor;
    KColorAction* m_bgColor;
    KAction* m_function;
    KAction* m_cellLayout;
    KAction* m_hideTable;
    KAction* m_showTable;
    KAction* m_borderLeft;
    KAction* m_borderRight;
    KAction* m_borderTop;
    KAction* m_borderBottom;
    KAction* m_borderAll;
    KAction* m_borderOutline;
    KAction* m_borderRemove;
    KColorAction* m_borderColor;
    /**
     * Pointer to the last popup menu.
     * Since only one popup menu can be opened at once, its pointer is stored here.
     * Delete the old one before you store a pointer to anotheron here.
     * May be 0L.
     */
    QPopupMenu *m_pPopupMenu;
    int m_popupMenuFirstToolId;

    QPopupMenu *m_pPopupRow;
    QPopupMenu *m_pPopupColumn;



    /**
     * Tells whether the user modfied the current cell.
     * Some key events are passed to the @ref EditWindow. When this flag is set and you
     * want to leave the cell with the marker then you must first save the new text
     * in the cell before moving the marker.
     */
    bool m_bEditDirtyFlag;

    /**
     * The active KSpreadTable. This table has the input focus. It may be 0L, too.
     */
    KSpreadTable* m_pTable;

    KSpreadDoc *m_pDoc;

    /**
     * Flags that indicate whether we should display additional
     * GUI stuff like rulers and scrollbars.
     *
     * @see #showGUI
     */
    bool m_bShowGUI;

    /**
     * If @ref #updateEditWidget is called it changes some KToggleActions.
     * That causes them to emit a signal. If this lock is TRUE, then these
     * signals are ignored.
     */
    bool m_toolbarLock;

   struct ToolEntry
   {
     QString command;
     KoDataToolInfo info;
   };
   QList<ToolEntry> m_lstTools;

   static KSpreadScripts *m_pGlobalScriptsDialog;
};

#endif
