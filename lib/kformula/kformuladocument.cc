/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qptrlist.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "contextstyle.h"
#include "kformulacontainer.h"
#include "kformuladocument.h"
#include "symboltable.h"

KFORMULA_NAMESPACE_BEGIN

struct Document::Document_Impl {

    Document_Impl( KConfig* c )
            : leftBracketChar( LeftRoundBracket ), rightBracketChar( RightRoundBracket ),
              formula(0), firstTime( true ),
              actionsCreated( false ), config( c )
    {
        formulae.setAutoDelete( false );
        //kdDebug( DEBUGID ) << "Document::Document_Impl " << formulae.count() << endl;
    }


    ~Document_Impl()
    {
        if (ownHistory) {
            delete history;
        }
    }

    // We know our actions, maybe a client is interessted...

    KAction* addThinSpaceAction;
    KAction* addMediumSpaceAction;
    KAction* addThickSpaceAction;
    KAction* addQuadSpaceAction;
    KAction* addBracketAction;
    KAction* addSBracketAction;
    KAction* addCBracketAction;
    KAction* addAbsAction;
    KAction* addFractionAction;
    KAction* addRootAction;
    KAction* addSumAction;
    KAction* addProductAction;
    KAction* addIntegralAction;
    KAction* addMatrixAction;
    KAction* addOneByTwoMatrixAction;
    KAction* addUpperLeftAction;
    KAction* addLowerLeftAction;
    KAction* addUpperRightAction;
    KAction* addLowerRightAction;
    KAction* addGenericUpperAction;
    KAction* addGenericLowerAction;
    KAction* removeEnclosingAction;
    KAction* makeGreekAction;
    KAction* insertSymbolAction;

    KAction* appendColumnAction;
    KAction* insertColumnAction;
    KAction* removeColumnAction;
    KAction* appendRowAction;
    KAction* insertRowAction;
    KAction* removeRowAction;

    KToggleAction* syntaxHighlightingAction;
    KSelectAction* leftBracket;
    KSelectAction* rightBracket;
    KSelectAction* symbolNamesAction;

    SymbolType leftBracketChar;
    SymbolType rightBracketChar;
    QString selectedName;

    /**
     * The active formula.
     */
    Container* formula;

    /**
     * Our undo stack. We don't own it. The stack belongs to
     * our parent and might contain not formula related commands
     * as well.
     */
    KCommandHistory* history;

    /**
     * Tells whether we are responsible to remove our history.
     */
    bool ownHistory;

    /**
     * The documents context style. This is the place where all
     * the user configurable informations are stored.
     */
    ContextStyle contextStyle;

    /**
     * All formulae that belong to this document.
     */
    QPtrList<Container> formulae;

    /**
     * Lazy initialization. Read the symbol table only if
     * we create a container.
     */
    bool firstTime;

    bool actionsCreated;

    /**
     * The applications config object. We need to remember this so that
     * we don't depend on global variables. (We don't know who uses us.)
     */
    KConfig* config;
};


double Document::getXResolution() const { return impl->contextStyle.zoomedResolutionX(); }
double Document::getYResolution() const { return impl->contextStyle.zoomedResolutionY(); }

//void Document::setResolution(double zX, double zY) { impl->contextStyle.setResolution(zX, zY); }

KCommandHistory* Document::getHistory() const { return impl->history; }
const SymbolTable& Document::getSymbolTable() const { return impl->contextStyle.symbolTable(); }

KAction* Document::getAddThinSpaceAction()     { return impl->addThinSpaceAction; }
KAction* Document::getAddMediumSpaceAction()   { return impl->addMediumSpaceAction; }
KAction* Document::getAddThickSpaceAction()    { return impl->addThickSpaceAction; }
KAction* Document::getAddQuadSpaceAction()     { return impl->addQuadSpaceAction; }
KAction* Document::getAddBracketAction()       { return impl->addBracketAction; }
KAction* Document::getAddSBracketAction()      { return impl->addSBracketAction;}
KAction* Document::getAddCBracketAction()      { return impl->addCBracketAction;}
KAction* Document::getAddAbsAction()           { return impl->addAbsAction;}
KAction* Document::getAddFractionAction()      { return impl->addFractionAction; }
KAction* Document::getAddRootAction()          { return impl->addRootAction; }
KAction* Document::getAddSumAction()           { return impl->addSumAction; }
KAction* Document::getAddProductAction()       { return impl->addProductAction; }
KAction* Document::getAddIntegralAction()      { return impl->addIntegralAction; }
KAction* Document::getAddMatrixAction()        { return impl->addMatrixAction; }
KAction* Document::getAddOneByTwoMatrixAction(){ return impl->addOneByTwoMatrixAction; }
KAction* Document::getAddUpperLeftAction()     { return impl->addUpperLeftAction; }
KAction* Document::getAddLowerLeftAction()     { return impl->addLowerLeftAction; }
KAction* Document::getAddUpperRightAction()    { return impl->addUpperRightAction; }
KAction* Document::getAddLowerRightAction()    { return impl->addLowerRightAction; }
KAction* Document::getAddGenericUpperAction()  { return impl->addGenericUpperAction; }
KAction* Document::getAddGenericLowerAction()  { return impl->addGenericLowerAction; }
KAction* Document::getRemoveEnclosingAction()  { return impl->removeEnclosingAction; }
KAction* Document::getMakeGreekAction()        { return impl->makeGreekAction; }
KAction* Document::getInsertSymbolAction()     { return impl->insertSymbolAction; }

KAction* Document::getAppendColumnAction()     { return impl->appendColumnAction; }
KAction* Document::getInsertColumnAction()     { return impl->insertColumnAction; }
KAction* Document::getRemoveColumnAction()     { return impl->removeColumnAction; }
KAction* Document::getAppendRowAction()        { return impl->appendRowAction; }
KAction* Document::getInsertRowAction()        { return impl->insertRowAction; }
KAction* Document::getRemoveRowAction()        { return impl->removeRowAction; }

KSelectAction* Document::getLeftBracketAction()  { return impl->leftBracket; }
KSelectAction* Document::getRightBracketAction() { return impl->rightBracket; }
KSelectAction* Document::getSymbolNamesAction()  { return impl->symbolNamesAction; }
KToggleAction* Document::getSyntaxHighlightingAction() { return impl->syntaxHighlightingAction; }

Container* Document::formula() const { return impl->formula; }

SymbolType Document::leftBracketChar() const  { return impl->leftBracketChar; }
SymbolType Document::rightBracketChar() const { return impl->rightBracketChar; }


Document::Document( KConfig* config,
                    KActionCollection* collection,
                    KCommandHistory* his )
{
    impl = new Document_Impl( config );

    KGlobal::dirs()->addResourceType("toolbar", KStandardDirs::kde_default("data") + "kformula/pics/");
    createActions(collection);
    //kdDebug( DEBUGID ) << "Document::Document" << endl;
    impl->contextStyle.readConfig( impl->config );
    impl->syntaxHighlightingAction->setChecked( impl->contextStyle.syntaxHighlighting() );

    if (his == 0) {
        impl->history = new KCommandHistory(collection);
        impl->ownHistory = true;
    }
    else {
        impl->history = his;
        impl->ownHistory = false;
    }
}


Document::Document( KConfig* config, KCommandHistory* his )
{
    impl = new Document_Impl( config );
    impl->contextStyle.readConfig( impl->config );
    if (his == 0) {
        impl->history = new KCommandHistory;
        impl->ownHistory = true;
    }
    else {
        impl->history = his;
        impl->ownHistory = false;
    }
}


Document::~Document()
{
    delete impl;
}


void Document::lazyInit()
{
    if ( impl->firstTime ) {
        impl->firstTime = false;
        impl->contextStyle.init();

        if ( impl->actionsCreated ) {
            QStringList names = impl->contextStyle.symbolTable().allNames();
            impl->symbolNamesAction->setItems(names);
            impl->selectedName = names[0];
        }
    }
}

ContextStyle& Document::getContextStyle( bool edit )
{
    lazyInit();
    impl->contextStyle.setEdit( edit );
    return impl->contextStyle;
}

void Document::setZoomAndResolution( int zoom, int dpiX, int dpiY )
{
    lazyInit();
    impl->contextStyle.setZoomAndResolution( zoom, dpiX, dpiY );
}

void Document::newZoomAndResolution( bool updateViews, bool /*forPrint*/ )
{
    if ( updateViews ) {
        recalc();
    }
}

void Document::setZoom( double zoomX, double zoomY, bool updateViews, bool forPrint )
{
    if ( getContextStyle( !forPrint ).setZoom( zoomX, zoomY, updateViews, forPrint ) && updateViews ) {
        recalc();
    }
}

Container* Document::createFormula()
{
    lazyInit();
    Container* f = new Container(this);
    impl->formulae.append(f);
    return f;
}


void Document::activate(Container* f)
{
    impl->formula = f;
}


void Document::formulaDies(Container* f)
{
    if (f == impl->formula) {
        impl->formula = 0;
    }
    impl->formulae.remove(f);
}


void Document::createActions(KActionCollection* collection)
{
    impl->addThinSpaceAction = new KAction( i18n( "Add thin space" ),
                                    0,
                                    this, SLOT( addThinSpace() ),
                                    collection, "formula_addthinspace") ;
    impl->addMediumSpaceAction = new KAction( i18n( "Add medium space" ),
                                    0,
                                    this, SLOT( addMediumSpace() ),
                                    collection, "formula_addmediumspace" );
    impl->addThickSpaceAction = new KAction( i18n( "Add thick space" ),
                                    0,
                                    this, SLOT( addThickSpace() ),
                                    collection, "formula_addthickspace" );
    impl->addQuadSpaceAction = new KAction( i18n( "Add quad space" ),
                                    0,
                                    this, SLOT( addQuadSpace() ),
                                    collection, "formula_addquadspace" );

    impl->addIntegralAction = new KAction(i18n("Add integral"),
                                    "int",
                                    CTRL + Key_6,
                                    this, SLOT(addIntegral()),
                                    collection, "formula_addintegral");
    impl->addSumAction      = new KAction(i18n("Add sum"),
                                    "sum",
                                    CTRL + Key_7,
                                    this, SLOT(addSum()),
                                    collection, "formula_addsum");
    impl->addProductAction  = new KAction(i18n("Add product"),
                                    "prod",
                                    CTRL + Key_4,
                                    this, SLOT(addProduct()),
                                    collection, "formula_addproduct");
    impl->addRootAction     = new KAction(i18n("Add root"),
                                    "sqrt",
                                    CTRL + Key_2,
                                    this, SLOT(addRoot()),
                                    collection, "formula_addroot");
    impl->addFractionAction = new KAction(i18n("Add fraction"),
                                    "frac",
                                    CTRL + Key_3,
                                    this, SLOT(addFraction()),
                                    collection, "formula_addfrac");
    impl->addBracketAction  = new KAction(i18n("Add bracket"),
                                    "paren",
                                    0,
                                    this, SLOT(addDefaultBracket()),
                                    collection,"formula_addbra");
    impl->addSBracketAction = new KAction(i18n("Add square bracket"),
                                    "brackets",
                                    0,
                                    this, SLOT(addSquareBracket()),
                                    collection,"formula_addsqrbra");
    impl->addCBracketAction = new KAction(i18n("Add curly bracket"),
                                    "brace",
                                    0,
                                    this, SLOT(addCurlyBracket()),
                                    collection,"formula_addcurbra");
    impl->addAbsAction      = new KAction(i18n("Add abs"),
                                    "abs",
                                    0,
                                    this, SLOT(addLineBracket()),
                                    collection,"formula_addabsbra");

    impl->addMatrixAction   = new KAction(i18n("Add matrix"),
                                    "matrix",
                                    CTRL + Key_8,
                                    this, SLOT(addMatrix()),
                                    collection, "formula_addmatrix");

    impl->addOneByTwoMatrixAction   = new KAction(i18n("Add 1x2 matrix"),
                                    "onetwomatrix",
                                    0,
                                    this, SLOT(addOneByTwoMatrix()),
                                    collection, "formula_add_one_by_two_matrix");


    impl->addUpperLeftAction  = new KAction(i18n("Add upper left index"),
                                      "lsup",
                                      0,
                                      this, SLOT(addUpperLeftIndex()),
                                      collection, "formula_addupperleft");
    impl->addLowerLeftAction  = new KAction(i18n("Add lower left index"),
                                      "lsub",
                                      0,
                                      this, SLOT(addLowerLeftIndex()),
                                      collection, "formula_addlowerleft");
    impl->addUpperRightAction = new KAction(i18n("Add upper right index"),
                                      "rsup",
                                      0,
                                      this, SLOT(addUpperRightIndex()),
                                      collection, "formula_addupperright");
    impl->addLowerRightAction = new KAction(i18n("Add lower right index"),
                                      "rsub",
                                      0,
                                      this, SLOT(addLowerRightIndex()),
                                      collection, "formula_addlowerright");

    impl->addGenericUpperAction = new KAction(i18n("Add upper index"),
                                      CTRL + Key_U,
                                      this, SLOT(addGenericUpperIndex()),
                                      collection, "formula_addupperindex");
    impl->addGenericLowerAction = new KAction(i18n("Add lower index"),
                                      CTRL + Key_L,
                                      this, SLOT(addGenericLowerIndex()),
                                      collection, "formula_addlowerindex");

    impl->removeEnclosingAction = new KAction(i18n("Remove enclosing element"),
                                        CTRL + Key_R,
                                        this, SLOT(removeEnclosing()),
                                        collection, "formula_removeenclosing");

    impl->makeGreekAction = new KAction(i18n("Convert to Greek"),
                                  CTRL + Key_G,
                                  this, SLOT(makeGreek()),
                                  collection, "formula_makegreek");

    impl->appendColumnAction = new KAction( i18n( "Append Column" ),
                                            "inscol",
                                            0,
                                            this, SLOT( appendColumn() ),
                                            collection, "formula_appendcolumn" );
    impl->insertColumnAction = new KAction( i18n( "Insert Column" ),
                                            "inscol",
                                            0,
                                            this, SLOT( insertColumn() ),
                                            collection, "formula_insertcolumn" );
    impl->removeColumnAction = new KAction( i18n( "Remove Column" ),
                                            "remcol",
                                            0,
                                            this, SLOT( removeColumn() ),
                                            collection, "formula_removecolumn" );
    impl->appendRowAction = new KAction( i18n( "Append Row" ),
                                         "insrow",
                                         0,
                                         this, SLOT( appendRow() ),
                                         collection, "formula_appendrow" );
    impl->insertRowAction = new KAction( i18n( "Insert Row" ),
                                         "insrow",
                                         0,
                                         this, SLOT( insertRow() ),
                                         collection, "formula_insertrow" );
    impl->removeRowAction = new KAction( i18n( "Remove Row" ),
                                         "remrow",
                                         0,
                                         this, SLOT( removeRow() ),
                                         collection, "formula_removerow" );

    impl->syntaxHighlightingAction = new KToggleAction(i18n("Syntax highlighting"),
                                                 0,
                                                 this, SLOT(toggleSyntaxHighlighting()),
                                                 collection, "formula_syntaxhighlighting");

    QStringList delimiter;
    delimiter.append(QString("("));
    delimiter.append(QString("["));
    delimiter.append(QString("{"));
    delimiter.append(QString("<"));
    delimiter.append(QString("/"));
    delimiter.append(QString("\\"));
    delimiter.append(QString("|"));
    delimiter.append(QString(" "));
    delimiter.append(QString(")"));
    delimiter.append(QString("]"));
    delimiter.append(QString("}"));
    delimiter.append(QString(">"));
    impl->leftBracket = new KSelectAction(i18n("Left delimiter"),
                                    0, this, SLOT(delimiterLeft()),
                                    collection, "formula_typeleft");
    impl->leftBracket->setItems(delimiter);
    //leftBracket->setCurrentItem(0);

    delimiter.clear();
    delimiter.append(QString(")"));
    delimiter.append(QString("]"));
    delimiter.append(QString("}"));
    delimiter.append(QString(">"));
    delimiter.append(QString("/"));
    delimiter.append(QString("\\"));
    delimiter.append(QString("|"));
    delimiter.append(QString(" "));
    delimiter.append(QString("("));
    delimiter.append(QString("["));
    delimiter.append(QString("{"));
    delimiter.append(QString("<"));
    impl->rightBracket = new KSelectAction(i18n("Right delimiter"),
                                     0, this, SLOT(delimiterRight()),
                                     collection, "formula_typeright");
    impl->rightBracket->setItems(delimiter);
    //rightBracket->setCurrentItem(0);

    impl->insertSymbolAction = new KAction(i18n("Insert symbol"),
                                        CTRL + Key_I,
                                        this, SLOT(insertSymbol()),
                                        collection, "formula_insertsymbol");
    impl->symbolNamesAction = new KSelectAction(i18n("Symbol names"),
                                          0, this, SLOT(symbolNames()),
                                          collection, "formula_symbolnames");

    impl->actionsCreated = true;
}


void Document::paste()
{
    if (hasFormula()) {
        formula()->paste();
    }
}

void Document::copy()
{
    if (hasFormula()) {
        formula()->copy();
    }
}

void Document::cut()
{
    if (hasFormula()) {
        formula()->cut();
    }
}

void Document::addThinSpace()
{
    if (hasFormula()) {
        SpaceRequest r( THIN );
        formula()->performRequest( &r );
    }
}
void Document::addMediumSpace()
{
    if (hasFormula()) {
        SpaceRequest r( MEDIUM );
        formula()->performRequest( &r );
    }
}
void Document::addThickSpace()
{
    if (hasFormula()) {
        SpaceRequest r( THICK );
        formula()->performRequest( &r );
    }
}
void Document::addQuadSpace()
{
    if (hasFormula()) {
        SpaceRequest r( QUAD );
        formula()->performRequest( &r );
    }
}

void Document::addDefaultBracket()
{
    if (hasFormula()) {
        BracketRequest r( impl->leftBracketChar, impl->rightBracketChar );
        formula()->performRequest( &r );
    }
}

void Document::addBracket( SymbolType left, SymbolType right )
{
    if (hasFormula()) {
        BracketRequest r( left, right );
        formula()->performRequest( &r );
    }
}

void Document::addParenthesis()
{
    if (hasFormula()) {
        BracketRequest r( LeftRoundBracket, RightRoundBracket );
        formula()->performRequest( &r );
    }
}

void Document::addSquareBracket()
{
    if (hasFormula()) {
        BracketRequest r( LeftSquareBracket, RightSquareBracket );
        formula()->performRequest( &r );
    }
}

void Document::addCurlyBracket()
{
    if (hasFormula()) {
        BracketRequest r( LeftCurlyBracket, RightCurlyBracket );
        formula()->performRequest( &r );
    }
}

void Document::addLineBracket()
{
    if (hasFormula()) {
        BracketRequest r( LeftLineBracket, RightLineBracket );
        formula()->performRequest( &r );
    }
}

void Document::addFraction()
{
    if (hasFormula()) {
        Request r( req_addFraction );
        formula()->performRequest( &r );
    }
}

void Document::addRoot()
{
    if (hasFormula()) {
        Request r( req_addRoot );
        formula()->performRequest( &r );
    }
}

void Document::addIntegral()
{
    if (hasFormula()) {
        SymbolRequest r( Integral );
        formula()->performRequest( &r );
    }
}

void Document::addProduct()
{
    if (hasFormula()) {
        SymbolRequest r( Product );
        formula()->performRequest( &r );
    }
}

void Document::addSum()
{
    if (hasFormula()) {
        SymbolRequest r( Sum );
        formula()->performRequest( &r );
    }
}

void Document::addMatrix( uint rows, uint columns )
{
    if (hasFormula()) {
        MatrixRequest r( rows, columns );
        formula()->performRequest( &r );
    }
}

void Document::addOneByTwoMatrix()
{
    if (hasFormula()) {
        Request r( req_addOneByTwoMatrix );
        formula()->performRequest( &r );
    }
}

void Document::addNameSequence()
{
    if (hasFormula()) {
        Request r( req_addNameSequence );
        formula()->performRequest( &r );
    }
}

void Document::addLowerLeftIndex()
{
    if (hasFormula()) {
        IndexRequest r( lowerLeftPos );
        formula()->performRequest( &r );
    }
}

void Document::addUpperLeftIndex()
{
    if (hasFormula()) {
        IndexRequest r( upperLeftPos );
        formula()->performRequest( &r );
    }
}

void Document::addLowerRightIndex()
{
    if (hasFormula()) {
        IndexRequest r( lowerRightPos );
        formula()->performRequest( &r );
    }
}

void Document::addUpperRightIndex()
{
    if (hasFormula()) {
        IndexRequest r( upperRightPos );
        formula()->performRequest( &r );
    }
}

void Document::addGenericLowerIndex()
{
    if (hasFormula()) {
        IndexRequest r( lowerMiddlePos );
        formula()->performRequest( &r );
    }
}

void Document::addGenericUpperIndex()
{
    if (hasFormula()) {
        IndexRequest r( upperMiddlePos );
        formula()->performRequest( &r );
    }
}

void Document::removeEnclosing()
{
    if (hasFormula()) {
        DirectedRemove r( req_removeEnclosing, beforeCursor );
        formula()->performRequest( &r );
    }
}

void Document::makeGreek()
{
    if (hasFormula()) {
        Request r( req_makeGreek );
        formula()->performRequest( &r );
    }
}

void Document::insertSymbol()
{
    if ( hasFormula() && impl->contextStyle.symbolTable().contains( impl->selectedName ) ) {
        QChar ch = impl->contextStyle.symbolTable().unicode( impl->selectedName );
        if ( ch != QChar::null ) {
            TextCharRequest r( ch, true );
            formula()->performRequest( &r );
        }
        else {
            TextRequest r( impl->selectedName );
            formula()->performRequest( &r );
        }
    }
}

void Document::insertSymbol( QString name )
{
    if ( hasFormula() && impl->contextStyle.symbolTable().contains( name ) ) {
        QChar ch = impl->contextStyle.symbolTable().unicode( name );
        if ( ch != QChar::null ) {
            TextCharRequest r( ch, true );
            formula()->performRequest( &r );
            return;
        }
    }
    TextRequest r( name );
    formula()->performRequest( &r );
}

void Document::appendColumn()
{
    if ( hasFormula() ) {
        Request r( req_appendColumn );
        formula()->performRequest( &r );
    }
}

void Document::insertColumn()
{
    if ( hasFormula() ) {
        Request r( req_insertColumn );
        formula()->performRequest( &r );
    }
}

void Document::removeColumn()
{
    if ( hasFormula() ) {
        Request r( req_removeColumn );
        formula()->performRequest( &r );
    }
}

void Document::appendRow()
{
    if ( hasFormula() ) {
        Request r( req_appendRow );
        formula()->performRequest( &r );
    }
}

void Document::insertRow()
{
    if ( hasFormula() ) {
        Request r( req_insertRow );
        formula()->performRequest( &r );
    }
}

void Document::removeRow()
{
    if ( hasFormula() ) {
        Request r( req_removeRow );
        formula()->performRequest( &r );
    }
}

void Document::toggleSyntaxHighlighting()
{
    //kdDebug( DEBUGID ) << "Document::toggleSyntaxHighlighting" << endl;
    impl->contextStyle.setSyntaxHighlighting( impl->syntaxHighlightingAction->isChecked() );
    // Only to notify all views. We don't expect to get new values.
    // Here is a really bad bug.
    recalc();
}

void Document::delimiterLeft()
{
    QString left = impl->leftBracket->currentText();
    switch ( left.at(0).latin1() ) {
    case '[':
    case ']':
    case '{':
    case '}':
    case '<':
    case '>':
    case '(':
    case ')':
    case '/':
    case '\\':
        impl->leftBracketChar = static_cast<SymbolType>( left.at(0).latin1() );
        break;
    case '|':
        impl->leftBracketChar = LeftLineBracket;
        break;
    case ' ':
        impl->leftBracketChar = EmptyBracket;
        break;
    }
}

void Document::delimiterRight()
{
    QString right = impl->rightBracket->currentText();
    switch ( right.at(0).latin1() ) {
    case '[':
    case ']':
    case '{':
    case '}':
    case '<':
    case '>':
    case '(':
    case ')':
    case '/':
    case '\\':
        impl->rightBracketChar = static_cast<SymbolType>( right.at(0).latin1() );
        break;
    case '|':
        impl->rightBracketChar = RightLineBracket;
        break;
    case ' ':
        impl->rightBracketChar = EmptyBracket;
        break;
    }
}

void Document::symbolNames()
{
    impl->selectedName = impl->symbolNamesAction->currentText();
}

void Document::undo()
{
    getHistory()->undo();
}

void Document::redo()
{
    getHistory()->redo();
}

bool Document::hasFormula()
{
    return ( formula() != 0 ) && ( formula()->activeCursor() != 0 );
}

void Document::recalc()
{
    //kdDebug( DEBUGID ) << "Document::recalc " << impl->formulae.count() << endl;
    for ( Container* f = impl->formulae.first(); f != 0; f=impl->formulae.next() ) {
        f->recalc();
    }
}

void Document::updateConfig()
{
    impl->syntaxHighlightingAction->setChecked( impl->contextStyle.syntaxHighlighting() );
    recalc();
}

KFORMULA_NAMESPACE_END

using namespace KFormula;
#include "kformuladocument.moc"
