#include "kformula_view.h"
#include "kformula_doc.h"
#include "kformula_main.h"

#include <op_app.h>
#include <utils.h>
#include <part_frame_impl.h>

#include <qpainter.h>
#include <qstring.h>
#include <qmsgbox.h>
#include <qkeycode.h>

/**********************************************************
 *
 * KFormulaView
 *
 **********************************************************/

KFormulaView::KFormulaView( QWidget *_parent, const char *_name) :
    QWidget( _parent, _name ), View_impl(), KFormula::View_skel()
{
    setWidget( this );
  
    setBackgroundColor( white );
  
    Control_impl::setFocusPolicy( OPControls::Control::ClickFocus );
 
    m_pDoc = 0L;

    setGeometry( 5000, 5000, 100, 100 );
}

KFormulaView::~KFormulaView()
{
    cleanUp();
}

void KFormulaView::cleanUp()
{
    if ( m_bIsClean )
	return;
  
    m_pDoc->removeView( this );
  
    m_rMenuBar = 0L;
    m_vMenuBarFactory = 0L;

    m_rToolBarFormula = 0L;
    m_rToolBarFont = 0L;
    m_rToolBarType = 0L;
    m_vToolBarFactory = 0L;

    View_impl::cleanUp();
}
  
void KFormulaView::setDocument( KFormulaDocument *_doc )
{
    if ( m_pDoc )
	m_pDoc->removeView( this );

    View_impl::setDocument( _doc );
  
    m_pDoc = _doc;

    m_pDoc->addView( this );

    QObject::connect( m_pDoc, SIGNAL( sig_modified() ), 
		      this, SLOT( slotModified() ) );

    QObject::connect( m_pDoc, SIGNAL( sig_changeType(int) ), 
		      this, SLOT( slotTypeChanged(int) ) );
}

void KFormulaView::paintEvent( QPaintEvent *_ev )
{
    m_pDoc->paintEvent(_ev, this);
}
void KFormulaView::mousePressEvent( QMouseEvent *_ev )
{
    m_pDoc->mousePressEvent(_ev,this);
}

void KFormulaView::resizeEvent( QResizeEvent *_ev )
{
}

void KFormulaView::keyPressEvent( QKeyEvent *k )
{
    m_pDoc->keyPressEvent(k);
 
}



CORBA::Long KFormulaView::addToolButton (ToolBar_ref toolbar,
					 const char* pictname,
					 const char* tooltip,
					 const char* func) 
{
    QString tmp = KApplication::kde_datadir().copy();
    tmp += "/kformula/pics/";
    tmp += pictname;
    
    QString pix = loadPixmap( tmp );
   
    CORBA::Long id = toolbar->insertButton( CORBA::string_dup( pix ), 
					    CORBA::string_dup(tooltip),
					    this,
					    func);
    return id;
}

void KFormulaView::createGUI()
{ 
    m_vMenuBarFactory = m_vPartShell->menuBarFactory();
    if ( !CORBA::is_nil( m_vMenuBarFactory ) ) {
	// Menubar
	m_rMenuBar = m_vMenuBarFactory->createMenuBar( this );
	
	// View
	m_idMenuView = m_rMenuBar->insertMenu( i18n( "&View" ) );
	
	m_idMenuView_NewView = m_rMenuBar->insertItem( i18n( "&New View" ),
						       m_idMenuView,
						       this, "newView" );
    }
    
    m_vToolBarFactory = m_vPartShell->toolBarFactory();
    if ( !CORBA::is_nil( m_vToolBarFactory ) ) {
	
	m_rToolBarFormula = m_vToolBarFactory->createToolBar( this, i18n( "Formula" ) );
	
	m_idButtonFormula_0 = addToolButton(m_rToolBarFormula, "mini-xy.xpm", 
					    i18n( "Add/change to simple text" ),
					    "addText" );

	m_idButtonFormula_1 = addToolButton(m_rToolBarFormula, "mini-root.xpm", 
					    i18n( "Add/change to root" ), "addRoot" );

	m_idButtonFormula_2 = addToolButton(m_rToolBarFormula, "mini-frac.xpm",
					    i18n( "Add/change fract line" ), "addFrac");

	m_idButtonFormula_3 = addToolButton(m_rToolBarFormula, "mini-vspace.xpm",
					    i18n( "Add/change vertical space" ), "addVertSpace");
	
	
	m_idButtonFormula_4 = addToolButton(m_rToolBarFormula, "mini-bra.xpm",
					    i18n( "Add/change a bracket block" ), "addBracket" );

	m_idButtonFormula_5 = addToolButton(m_rToolBarFormula, "mini-integral.xpm",
					    i18n( "Add/change an integral" ), "addIntegral" );

	m_idButtonFormula_6 = addToolButton(m_rToolBarFormula, "mini-symbols.xpm",
					    i18n( "Add/change a block with symbols" ), "addSymbol" );
    }

    if ( !CORBA::is_nil( m_vToolBarFactory ) )
	{
	    m_rToolBarFont = m_vToolBarFactory->createToolBar( this, i18n( "Font" ) );

	    m_idButtonFont_0 = addToolButton(m_rToolBarFont, "reduce.xpm",
					     i18n( "Reduce the font of active block" ), "reduce");

	    m_idButtonFont_1 = addToolButton(m_rToolBarFont, "enlarge.xpm",
					     i18n( "Enlarge the font of active block" ), "enlarge");

	    m_idButtonFont_2 = addToolButton(m_rToolBarFont, "reduceall.xpm",
					     i18n( "Reduce the font of active block & children" ), "reduceRecur");
      
	    m_idButtonFont_3 = addToolButton(m_rToolBarFont, "enlargeall.xpm",
					     i18n( "Enlarge the font of active block & children" ), "enlargeRecur" );

	    m_idButtonFont_4 = addToolButton(m_rToolBarFont, "enlargenext.xpm",
					     i18n( "Reduce the font of active block, children & next blocks" ), "enlargeAll");

	    m_idButtonFont_5 = addToolButton(m_rToolBarFont, "greek.xpm",
					     i18n( "Set greek font" ), "setGreek" );
	    m_rToolBarFont->setToggle( m_idButtonFont_5, true );
	}
  
    if ( !CORBA::is_nil( m_vToolBarFactory ) )
	{
	    m_rToolBarType = m_vToolBarFactory->createToolBar( this, i18n( "Type" ) );
	  
	    m_idButtonType_0 = addToolButton(m_rToolBarType, "kformula1-0.xpm", 
					     i18n( "Add exponent" ), "addCh1" );

	    m_idButtonType_1 = addToolButton(m_rToolBarType, "kformula2-0.xpm", 
					     i18n( "Add index" ), "addCh2" );

	    m_idButtonType_2 = addToolButton(m_rToolBarType, "kformula2-1.xpm", 
					     i18n( "Add root index" ), "addCh2" );

	    m_idButtonType_3 = addToolButton(m_rToolBarType, "kformula2-2.xpm", 
					     i18n( "Add high limit" ), "addCh2" );

	    m_idButtonType_4 = addToolButton(m_rToolBarType, "kformula3-2.xpm", 
					     i18n( "Add low limit" ), "addCh3" );

	    // TODO check, why two exponents button exist!
	    m_idButtonType_5 = addToolButton(m_rToolBarType, "kformula2-3.xpm", 
					     i18n( "Add exponent" ), "addCh2" );

	    // TODO insert Line Edit control
	}  
}


void KFormulaView::slotTypeChanged(int type)
{   
   m_rToolBarType->setItemEnabled(m_idButtonType_0,(type==0));          
   m_rToolBarType->setItemEnabled(m_idButtonType_1,(type==0));          
   m_rToolBarType->setItemEnabled(m_idButtonType_2,(type==1));          
   m_rToolBarType->setItemEnabled(m_idButtonType_3,(type==2));          
   m_rToolBarType->setItemEnabled(m_idButtonType_4,(type==2));          
   m_rToolBarType->setItemEnabled(m_idButtonType_5,(type==3));          
debug("Type Changed");
    update();

}


void KFormulaView::slotModified()
{
    update();
}


void KFormulaView::newView()
{
    assert( (m_pDoc != 0L) );

    KFormulaShell* shell = new KFormulaShell;
    shell->enableMenuBar();
    shell->PartShell_impl::enableStatusBar();
    shell->enableToolBars();
    shell->show();
    shell->setDocument( m_pDoc );
  
    CORBA::release( shell );
}

void KFormulaView::addText()
{
    debug("adding Text");
    m_pDoc->addTextElement();
}

void KFormulaView::addRoot()
{ 
    debug("adding Root");
    m_pDoc->addRootElement();
}

void KFormulaView::addFraction()
{
    debug("##adding Fraction");
}

void KFormulaView::addVertSpace()
{
    debug("##adding VerticalSpace");
}

void KFormulaView::addBracket()
{
    debug("##adding parenthesis");
}

void KFormulaView::addIntegral()
{
    debug("##adding Integral");
}

void KFormulaView::addSymbol()
{
    debug("##adding Symbol");
}

void KFormulaView::reduce()
{
    debug("reduce");
    m_pDoc->activeElement()->scaleNumericFont(FN_REDUCE | FN_ELEMENT);
    update();
}

void KFormulaView::enlarge()
{
    debug("enlarge");
    m_pDoc->activeElement()->scaleNumericFont(FN_ENLARGE | FN_ELEMENT);
    update();
}

void KFormulaView::reduceRecur()
{
    m_pDoc->activeElement()->scaleNumericFont(FN_REDUCE | FN_ALL);
    update();
}

void KFormulaView::enlargeRecur()
{
    m_pDoc->activeElement()->scaleNumericFont(FN_ENLARGE | FN_ALL);
    update();
}

void KFormulaView::enlargeAll()
{
}

void KFormulaView::setGreek()
{
}

void KFormulaView::addCh1()
{
    debug("addCh1");
//    m_pDoc->addRootElement();

}

void KFormulaView::addCh2()
{
    debug("addCh2");
//    m_pDoc->addCh2();

}

void KFormulaView::addCh3()
{
    debug("addCh3");
}

#include "kformula_view.moc"






