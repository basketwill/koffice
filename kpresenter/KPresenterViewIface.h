#ifndef KPRESENTER_DOC_IFACE_H
#define KPRESENTER_DOC_IFACE_H

#include <dcopobject.h>
#include <dcopref.h>

#include <qstring.h>

class KPresenterView;

class KPresenterViewIface : virtual public DCOPObject
{
    K_DCOP
public:
    KPresenterViewIface( KPresenterView *view_ );

k_dcop:
    virtual void printDlg();
    // edit menu
    virtual void editUndo();
    virtual void editRedo();
    virtual void editCut();
    virtual void editCopy();
    virtual void editPaste();
    virtual void editDelete();
    virtual void editSelectAll();
    virtual void editCopyPage();
    virtual void editDelPage();
    virtual void editFind();
    virtual void editFindReplace();
    virtual void editHeaderFooter();

    // view menu
    virtual void newView();

    // insert menu
    virtual void insertPage();
    virtual void insertPicture();
    virtual void insertClipart();

    // tools menu
    virtual void toolsMouse();
    virtual void toolsLine();
    virtual void toolsRectangle();
    virtual void toolsCircleOrEllipse();
    virtual void toolsPie();
    virtual void toolsText();
    virtual void toolsAutoform();
    virtual void toolsDiagramm();
    virtual void toolsTable();
    virtual void toolsFormula();
    virtual void toolsObject();

    // extra menu
    virtual void extraPenBrush();
    virtual void extraConfigPie();
    virtual void extraConfigRect();
    virtual void extraRaise();
    virtual void extraLower();
    virtual void extraRotate();
    virtual void extraShadow();
    virtual void extraBackground();
    virtual void extraLayout();
    virtual void extraOptions();
    virtual void extraLineBegin();
    virtual void extraLineEnd();
    virtual void extraWebPres();

    virtual void extraAlignObjLeft();
    virtual void extraAlignObjCenterH();
    virtual void extraAlignObjRight();
    virtual void extraAlignObjTop();
    virtual void extraAlignObjCenterV();
    virtual void extraAlignObjBottom();

    virtual void extraAlignObjs();

    // screen menu
    virtual void screenConfigPages();
    virtual void screenPresStructView();
    virtual void screenAssignEffect();
    virtual void screenStart();
    virtual void screenStop();
    virtual void screenPause();
    virtual void screenFirst();
    virtual void screenPrev();
    virtual void screenNext();
    virtual void screenLast();
    virtual void screenSkip();
    virtual void screenFullScreen();
    virtual void screenPenColor();
    virtual void screenPenWidth( const QString &w );

    // help menu
    virtual void helpContents();

    // text toolbar
    virtual void sizeSelected();
    virtual void fontSelected();
    virtual void textBold();
    virtual void textItalic();
    virtual void textUnderline();
    virtual void textColor();
    virtual void textAlignLeft();
    virtual void textAlignCenter();
    virtual void textAlignRight();
    virtual void mtextFont();
    virtual void textEnumList();
    virtual void textUnsortList();
    virtual void textNormalText();
    virtual void textDepthPlus();
    virtual void textDepthMinus();
    virtual void textSpacing();
    virtual void textContentsToHeight();
    virtual void textObjectToContents();

    // in presentation mode
    virtual int getCurrentPresPage();
    virtual int getCurrentPresStep();
    virtual int getPresStepsOfPage();
    virtual int getNumPresPages();
    virtual bool gotoPresPage( int pg );

    // in edit mode
    virtual int getCurrentPageNum();
    
private:
    KPresenterView *view;

};

#endif
