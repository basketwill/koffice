/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 1998,1999 Torben Weis <weis@kde.org>
   Copyright 1999-2007 The KCells Team <koffice-devel@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Local
#include "KCSheet.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QList>
#include <QMap>
#include <QMimeData>
#include <QStack>
#include <QTextStream>
#include <QImage>

#include <kdebug.h>
#include <kcodecs.h>
#include <kfind.h>
#include <kfinddialog.h>
#include <kreplace.h>
#include <kreplacedialog.h>
#include <kurl.h>

#include <KoDocumentInfo.h>
#include <KOdfLoadingContext.h>
#include <KOdfSettings.h>
#include <KOdfStylesReader.h>

#include <KShape.h>
#include <KResourceManager.h>
#include <KShapeLoadingContext.h>
#include <KShapeManager.h>
#include <KShapeRegistry.h>
#include <KShapeSavingContext.h>
#include <KOdfStyleStack.h>
#include <KUnit.h>
#include <KOdfXmlNS.h>
#include <KXmlWriter.h>
#include <KOdfStore.h>
#include <KOdfText.h>
#include <KStyleManager.h>
#include <KTextSharedLoadingData.h>
#include <KParagraphStyle.h>
#include <KoUpdater.h>
#include <KoProgressUpdater.h>

#include "KCCellStorage.h"
#include "KCCluster.h"
#include "KCCondition.h"
#include "Damages.h"
#include "KCDependencyManager.h"
#include "KCDocBase.h"
#include "KCFormulaStorage.h"
#include "Global.h"
#include "KCHeaderFooter.h"
#include "KCLoadingInfo.h"
#include "KCLocalization.h"
#include "KCMap.h"
#include "KCNamedAreaManager.h"
#include "KCOdfLoadingContext.h"
#include "KCOdfSavingContext.h"
#include "KCPrintSettings.h"
#include "KCRecalcManager.h"
#include "RowColumnFormat.h"
#include "KCShapeApplicationData.h"
#include "KCSheetPrint.h"
#include "KCRectStorage.h"
#include "KCSheetModel.h"
#include "KCStyle.h"
#include "KCStyleManager.h"
#include "KCStyleStorage.h"
#include "Util.h"
#include "KCValidity.h"
#include "KCValueConverter.h"
#include "KCValueStorage.h"


template<typename T> class IntervalMap
{
public:
    IntervalMap() {}
    // from and to are inclusive, assumes no overlapping ranges
    // even though no checks are done
    void insert(int from, int to, const T& data) {
        m_data.insert(to, qMakePair(from, data));
    }
    T get(int idx) const {
        typename QMap<int, QPair<int, T> >::ConstIterator it = m_data.lowerBound(idx);
        if (it != m_data.end() && it.value().first <= idx) {
            return it.value().second;
        }
        return T();
    }
private:
    QMap<int, QPair<int, T> > m_data;
};

static QString createObjectName(const QString &sheetName)
{
    QString objectName;
    for (int i = 0; i < sheetName.count(); ++i) {
        if (sheetName[i].isLetterOrNumber() || sheetName[i] == '_')
            objectName.append(sheetName[i]);
        else
            objectName.append('_');
    }
    return objectName;
}


class KCSheet::Private
{
public:
    KCMap* workbook;
    KCSheetModel *model;

    QString name;

    Qt::LayoutDirection layoutDirection;

    // true if sheet is hidden
    bool hide;

    bool showGrid;
    bool showFormula;
    bool showFormulaIndicator;
    bool showCommentIndicator;
    bool autoCalc;
    bool lcMode;
    bool showColumnNumber;
    bool hideZero;
    bool firstLetterUpper;

    // clusters to hold objects
    KCCellStorage* cellStorage;
    KCRowCluster rows;
    KCColumnCluster columns;
    QList<KShape*> shapes;

    // hold the print object
    KCSheetPrint* print;

    // Indicates whether the sheet should paint the page breaks.
    // Doing so costs some time, so by default it should be turned off.
    bool showPageBorders;

    // Max range of canvas in x and y direction.
    //  Depends on KS_colMax/KS_rowMax and the width/height of all columns/rows
    QSizeF documentSize;

    QImage backgroundImage;
    KCSheet::BackgroundImageProperties backgroundProperties;
};


KCSheet::KCSheet(KCMap* map, const QString &sheetName)
        : KShapeUserData(map)
        , KShapeControllerBase()
        , d(new Private)
{
    d->workbook = map;
    if (map->doc()) {
        resourceManager()->setUndoStack(map->doc()->undoStack());
        QVariant variant;
        variant.setValue<void*>(map->doc()->sheetAccessModel());
        resourceManager()->setResource(75751149, variant); // duplicated in kchart.
    }
    d->model = new KCSheetModel(this);

    d->layoutDirection = QApplication::layoutDirection();

    d->name = sheetName;

    // Set a valid object name, so that we can offer scripting.
    setObjectName(createObjectName(d->name));

    d->cellStorage = new KCCellStorage(this);
    d->rows.setAutoDelete(true);
    d->columns.setAutoDelete(true);

    d->documentSize = QSizeF(KS_colMax * d->workbook->defaultColumnFormat()->width(),
                             KS_rowMax * d->workbook->defaultRowFormat()->height());

    d->hide = false;
    d->showGrid = true;
    d->showFormula = false;
    d->showFormulaIndicator = false;
    d->showCommentIndicator = true;
    d->showPageBorders = false;

    d->lcMode = false;
    d->showColumnNumber = false;
    d->hideZero = false;
    d->firstLetterUpper = false;
    d->autoCalc = true;
    d->print = new KCSheetPrint(this);

    // document size changes always trigger a visible size change
    connect(this, SIGNAL(documentSizeChanged(const QSizeF&)), SIGNAL(visibleSizeChanged()));
    // KCCellStorage connections
    connect(d->cellStorage, SIGNAL(insertNamedArea(const KCRegion&, const QString&)),
            d->workbook->namedAreaManager(), SLOT(insert(const KCRegion&, const QString&)));
    connect(d->cellStorage, SIGNAL(namedAreaRemoved(const QString&)),
            d->workbook->namedAreaManager(), SLOT(remove(const QString&)));
}

KCSheet::KCSheet(const KCSheet &other)
        : KShapeUserData(other.d->workbook)
        , KShapeControllerBase()
        , KCProtectableObject(other)
        , d(new Private)
{
    d->workbook = other.d->workbook;
    d->model = new KCSheetModel(this);

    // create a unique name
    int i = 1;
    do
        d->name = other.d->name + QString("_%1").arg(i++);
    while (d->workbook->findSheet(d->name));

    // Set a valid object name, so that we can offer scripting.
    setObjectName(createObjectName(d->name));

    d->layoutDirection = other.d->layoutDirection;
    d->hide = other.d->hide;
    d->showGrid = other.d->showGrid;
    d->showFormula = other.d->showFormula;
    d->showFormulaIndicator = other.d->showFormulaIndicator;
    d->showCommentIndicator = other.d->showCommentIndicator;
    d->autoCalc = other.d->autoCalc;
    d->lcMode = other.d->lcMode;
    d->showColumnNumber = other.d->showColumnNumber;
    d->hideZero = other.d->hideZero;
    d->firstLetterUpper = other.d->firstLetterUpper;

    d->cellStorage = new KCCellStorage(*other.d->cellStorage, this);
    d->rows = other.d->rows;
    d->columns = other.d->columns;

    // flake
#if 0 // KCELLS_WIP_COPY_SHEET_(SHAPES)
    //FIXME This does not work as copySettings does not work. Also createDefaultShapeAndInit without the correct settings can not work
    //I think this should use saveOdf and loadOdf for copying
    KShape* shape;
    const QList<KShape*> shapes = other.d->shapes;
    for (int i = 0; i < shapes.count(); ++i) {
        shape = KShapeRegistry::instance()->value(shapes[i]->shapeId())->createDefaultShapeAndInit(0);
        shape->copySettings(shapes[i]);
        addShape(shape);
    }
#endif // KCELLS_WIP_COPY_SHEET_(SHAPES)

    d->print = new KCSheetPrint(this); // FIXME = new KCSheetPrint(*other.d->print);

    d->showPageBorders = other.d->showPageBorders;
    d->documentSize = other.d->documentSize;
}

KCSheet::~KCSheet()
{
    //Disable automatic recalculation of dependancies on this sheet to prevent crashes
    //in certain situations:
    //
    //For example, suppose a cell in SheetB depends upon a cell in SheetA.  If the cell in SheetB is emptied
    //after SheetA has already been deleted, the program would try to remove dependancies from the cell in SheetA
    //causing a crash.
    setAutoCalculationEnabled(false);

    delete d->print;
    delete d->cellStorage;
    qDeleteAll(d->shapes);
    delete d;
}

QAbstractItemModel* KCSheet::model() const
{
    return d->model;
}

QString KCSheet::sheetName() const
{
    return d->name;
}

KCMap* KCSheet::map() const
{
    return d->workbook;
}

KCDocBase* KCSheet::doc() const
{
    return d->workbook->doc();
}

void KCSheet::addShape(KShape* shape)
{
    if (!shape)
        return;
    d->shapes.append(shape);
    shape->setApplicationData(new KCShapeApplicationData());
    emit shapeAdded(this, shape);
}

void KCSheet::removeShape(KShape* shape)
{
    if (!shape)
        return;
    d->shapes.removeAll(shape);
    emit shapeRemoved(this, shape);
}

void KCSheet::deleteShapes()
{
    qDeleteAll(d->shapes);
    d->shapes.clear();
}

KResourceManager* KCSheet::resourceManager() const
{
    return map()->resourceManager();
}

QList<KShape*> KCSheet::shapes() const
{
    return d->shapes;
}

Qt::LayoutDirection KCSheet::layoutDirection() const
{
    return d->layoutDirection;
}

void KCSheet::setLayoutDirection(Qt::LayoutDirection dir)
{
    d->layoutDirection = dir;
}

bool KCSheet::isHidden() const
{
    return d->hide;
}

void KCSheet::setHidden(bool hidden)
{
    d->hide = hidden;
}

bool KCSheet::getShowGrid() const
{
    return d->showGrid;
}

void KCSheet::setShowGrid(bool _showGrid)
{
    d->showGrid = _showGrid;
}

bool KCSheet::getShowFormula() const
{
    return d->showFormula;
}

void KCSheet::setShowFormula(bool _showFormula)
{
    d->showFormula = _showFormula;
}

bool KCSheet::getShowFormulaIndicator() const
{
    return d->showFormulaIndicator;
}

void KCSheet::setShowFormulaIndicator(bool _showFormulaIndicator)
{
    d->showFormulaIndicator = _showFormulaIndicator;
}

bool KCSheet::getShowCommentIndicator() const
{
    return d->showCommentIndicator;
}

void KCSheet::setShowCommentIndicator(bool _indic)
{
    d->showCommentIndicator = _indic;
}

bool KCSheet::getLcMode() const
{
    return d->lcMode;
}

void KCSheet::setLcMode(bool _lcMode)
{
    d->lcMode = _lcMode;
}

bool KCSheet::isAutoCalculationEnabled() const
{
    return d->autoCalc;
}

void KCSheet::setAutoCalculationEnabled(bool enable)
{
    //Avoid possible recalculation of dependancies if the auto calc setting hasn't changed
    if (d->autoCalc == enable)
        return;

    d->autoCalc = enable;
    //If enabling automatic calculation, make sure that the dependencies are up-to-date
    if (enable == true) {
        map()->dependencyManager()->addSheet(this);
        map()->recalcManager()->recalcSheet(this);
    } else {
        map()->dependencyManager()->removeSheet(this);
    }
}

bool KCSheet::getShowColumnNumber() const
{
    return d->showColumnNumber;
}

void KCSheet::setShowColumnNumber(bool _showColumnNumber)
{
    d->showColumnNumber = _showColumnNumber;
}

bool KCSheet::getHideZero() const
{
    return d->hideZero;
}

void KCSheet::setHideZero(bool _hideZero)
{
    d->hideZero = _hideZero;
}

bool KCSheet::getFirstLetterUpper() const
{
    return d->firstLetterUpper;
}

void KCSheet::setFirstLetterUpper(bool _firstUpper)
{
    d->firstLetterUpper = _firstUpper;
}

bool KCSheet::isShowPageBorders() const
{
    return d->showPageBorders;
}

const KCColumnFormat* KCSheet::columnFormat(int _column) const
{
    const KCColumnFormat *p = d->columns.lookup(_column);
    if (p != 0)
        return p;

    return map()->defaultColumnFormat();
}

const KCRowFormat* KCSheet::rowFormat(int _row) const
{
    const KCRowFormat *p = d->rows.lookup(_row);
    if (p != 0)
        return p;

    return map()->defaultRowFormat();
}

KCCellStorage* KCSheet::cellStorage() const
{
    return d->cellStorage;
}

const CommentStorage* KCSheet::commentStorage() const
{
    return d->cellStorage->commentStorage();
}

const KCConditionsStorage* KCSheet::conditionsStorage() const
{
    return d->cellStorage->conditionsStorage();
}

const KCFormulaStorage* KCSheet::formulaStorage() const
{
    return d->cellStorage->formulaStorage();
}

const FusionStorage* KCSheet::fusionStorage() const
{
    return d->cellStorage->fusionStorage();
}

const LinkStorage* KCSheet::linkStorage() const
{
    return d->cellStorage->linkStorage();
}

const KCStyleStorage* KCSheet::styleStorage() const
{
    return d->cellStorage->styleStorage();
}

const KCValidityStorage* KCSheet::validityStorage() const
{
    return d->cellStorage->validityStorage();
}

const KCValueStorage* KCSheet::valueStorage() const
{
    return d->cellStorage->valueStorage();
}

KCSheetPrint* KCSheet::print() const
{
    return d->print;
}

KCPrintSettings* KCSheet::printSettings() const
{
    return d->print->settings();
}

void KCSheet::setPrintSettings(const KCPrintSettings &settings)
{
    d->print->setSettings(settings);
    // Repaint, if page borders are shown and this is the active sheet.
    if (isShowPageBorders()) {
        // Just repaint everything visible; no need to invalidate the visual cache.
        map()->addDamage(new KCSheetDamage(this, KCSheetDamage::ContentChanged));
    }
}

KCHeaderFooter *KCSheet::headerFooter() const
{
    return d->print->headerFooter();
}

QSizeF KCSheet::documentSize() const
{
    return d->documentSize;
}

void KCSheet::adjustDocumentWidth(double deltaWidth)
{
    d->documentSize.rwidth() += deltaWidth;
    emit documentSizeChanged(d->documentSize);
}

void KCSheet::adjustDocumentHeight(double deltaHeight)
{
    d->documentSize.rheight() += deltaHeight;
    emit documentSizeChanged(d->documentSize);
}

int KCSheet::leftColumn(double _xpos, double &_left) const
{
    _left = 0.0;
    int col = 1;
    double x = columnFormat(col)->visibleWidth();
    while (x < _xpos && col < KS_colMax) {
        _left += columnFormat(col)->visibleWidth();
        x += columnFormat(++col)->visibleWidth();
    }
    return col;
}

int KCSheet::rightColumn(double _xpos) const
{
    int col = 1;
    double x = columnFormat(col)->visibleWidth();
    while (x <= _xpos && col < KS_colMax)
        x += columnFormat(++col)->visibleWidth();
    return col;
}

int KCSheet::topRow(double _ypos, double & _top) const
{
    _top = 0.0;
    int row = 1;
    double y = rowFormat(row)->visibleHeight();
    while (y < _ypos && row < KS_rowMax) {
        _top += rowFormat(row)->visibleHeight();
        y += rowFormat(++row)->visibleHeight();
    }
    return row;
}

int KCSheet::bottomRow(double _ypos) const
{
    int row = 1;
    double y = rowFormat(row)->visibleHeight();
    while (y <= _ypos && row < KS_rowMax)
        y += rowFormat(++row)->visibleHeight();
    return row;
}

QRectF KCSheet::cellCoordinatesToDocument(const QRect& cellRange) const
{
    // TODO Stefan: Rewrite to save some iterations over the columns/rows.
    QRectF rect;
    rect.setLeft(columnPosition(cellRange.left()));
    rect.setRight(columnPosition(cellRange.right()) + columnFormat(cellRange.right())->width());
    rect.setTop(rowPosition(cellRange.top()));
    rect.setBottom(rowPosition(cellRange.bottom()) + rowFormat(cellRange.bottom())->height());
    return rect;
}

QRect KCSheet::documentToCellCoordinates(const QRectF &area) const
{
    double width = 0.0;
    int left = 0;
    while (width <= area.left())
        width += columnFormat(++left)->visibleWidth();
    int right = left;
    while (width < area.right())
        width += columnFormat(++right)->visibleWidth();
    int top = 0;
    double height = 0.0;
    while (height <= area.top())
        height += rowFormat(++top)->visibleHeight();
    int bottom = top;
    while (height < area.bottom())
        height += rowFormat(++bottom)->visibleHeight();
    return QRect(left, top, right - left + 1, bottom - top + 1);
}

double KCSheet::columnPosition(int _col) const
{
    const int max = qMin(_col, KS_colMax);
    double x = 0.0;
    for (int col = 1; col < max; ++col)
        x += columnFormat(col)->visibleWidth();
    return x;
}


double KCSheet::rowPosition(int _row) const
{
    const int max = qMin(_row, KS_rowMax);
    double y = 0.0;
    for (int row = 1; row < max; ++row)
        y += rowFormat(row)->visibleHeight();
    return y;
}


KCRowFormat* KCSheet::firstRow() const
{
    return d->rows.first();
}

KCColumnFormat* KCSheet::firstCol() const
{
    return d->columns.first();
}

KCColumnFormat* KCSheet::nonDefaultColumnFormat(int _column, bool force_creation)
{
    Q_ASSERT(_column >= 1 && _column <= KS_colMax);
    KCColumnFormat *p = d->columns.lookup(_column);
    if (p != 0 || !force_creation)
        return p;

    p = new KCColumnFormat(*map()->defaultColumnFormat());
    p->setSheet(this);
    p->setColumn(_column);

    d->columns.insertElement(p, _column);

    return p;
}

KCRowFormat* KCSheet::nonDefaultRowFormat(int _row, bool force_creation)
{
    Q_ASSERT(_row >= 1 && _row <= KS_rowMax);
    KCRowFormat *p = d->rows.lookup(_row);
    if (p != 0 || !force_creation)
        return p;

    p = new KCRowFormat(*map()->defaultRowFormat());
    p->setSheet(this);
    p->setRow(_row);

    d->rows.insertElement(p, _row);

    return p;
}

void KCSheet::changeCellTabName(QString const & old_name, QString const & new_name)
{
    for (int c = 0; c < formulaStorage()->count(); ++c) {
        if (formulaStorage()->data(c).expression().contains(old_name)) {
            int nb = formulaStorage()->data(c).expression().count(old_name + '!');
            QString tmp = old_name + '!';
            int len = tmp.length();
            tmp = formulaStorage()->data(c).expression();

            for (int i = 0; i < nb; ++i) {
                int pos = tmp.indexOf(old_name + '!');
                tmp.replace(pos, len, new_name + '!');
            }
            KCCell cell(this, formulaStorage()->col(c), formulaStorage()->row(c));
            KCFormula formula(this, cell);
            formula.setExpression(tmp);
            cell.setFormula(formula);
            cell.makeFormula();
        }
    }
}

void KCSheet::insertShiftRight(const QRect& rect)
{
    foreach(KCSheet* sheet, map()->sheetList()) {
        for (int i = rect.top(); i <= rect.bottom(); ++i) {
            sheet->changeNameCellRef(QPoint(rect.left(), i), false,
                                     KCSheet::ColumnInsert, sheetName(),
                                     rect.right() - rect.left() + 1);
        }
    }
}

void KCSheet::insertShiftDown(const QRect& rect)
{
    foreach(KCSheet* sheet, map()->sheetList()) {
        for (int i = rect.left(); i <= rect.right(); ++i) {
            sheet->changeNameCellRef(QPoint(i, rect.top()), false,
                                     KCSheet::RowInsert, sheetName(),
                                     rect.bottom() - rect.top() + 1);
        }
    }
}

void KCSheet::removeShiftUp(const QRect& rect)
{
    foreach(KCSheet* sheet, map()->sheetList()) {
        for (int i = rect.left(); i <= rect.right(); ++i) {
            sheet->changeNameCellRef(QPoint(i, rect.top()), false,
                                     KCSheet::RowRemove, sheetName(),
                                     rect.bottom() - rect.top() + 1);
        }
    }
}

void KCSheet::removeShiftLeft(const QRect& rect)
{
    foreach(KCSheet* sheet, map()->sheetList()) {
        for (int i = rect.top(); i <= rect.bottom(); ++i) {
            sheet->changeNameCellRef(QPoint(rect.left(), i), false,
                                     KCSheet::ColumnRemove, sheetName(),
                                     rect.right() - rect.left() + 1);
        }
    }
}

void KCSheet::insertColumns(int col, int number)
{
    double deltaWidth = 0.0;
    for (int i = 0; i < number; i++) {
        deltaWidth -= columnFormat(KS_colMax)->width();
        d->columns.insertColumn(col);
        deltaWidth += columnFormat(col + i)->width();
    }
    // Adjust document width (plus widths of new columns; minus widths of removed columns).
    adjustDocumentWidth(deltaWidth);

    foreach(KCSheet* sheet, map()->sheetList()) {
        sheet->changeNameCellRef(QPoint(col, 1), true,
                                 KCSheet::ColumnInsert, sheetName(),
                                 number);
    }
    //update print settings
    d->print->insertColumn(col, number);
}

void KCSheet::insertRows(int row, int number)
{
    double deltaHeight = 0.0;
    for (int i = 0; i < number; i++) {
        deltaHeight -= rowFormat(KS_rowMax)->height();
        d->rows.insertRow(row);
        deltaHeight += rowFormat(row)->height();
    }
    // Adjust document height (plus heights of new rows; minus heights of removed rows).
    adjustDocumentHeight(deltaHeight);

    foreach(KCSheet* sheet, map()->sheetList()) {
        sheet->changeNameCellRef(QPoint(1, row), true,
                                 KCSheet::RowInsert, sheetName(),
                                 number);
    }
    //update print settings
    d->print->insertRow(row, number);
}

void KCSheet::removeColumns(int col, int number)
{
    double deltaWidth = 0.0;
    for (int i = 0; i < number; ++i) {
        deltaWidth -= columnFormat(col)->width();
        d->columns.removeColumn(col);
        deltaWidth += columnFormat(KS_colMax)->width();
    }
    // Adjust document width (plus widths of new columns; minus widths of removed columns).
    adjustDocumentWidth(deltaWidth);

    foreach(KCSheet* sheet, map()->sheetList()) {
        sheet->changeNameCellRef(QPoint(col, 1), true,
                                 KCSheet::ColumnRemove, sheetName(),
                                 number);
    }
    //update print settings
    d->print->removeColumn(col, number);
}

void KCSheet::removeRows(int row, int number)
{
    double deltaHeight = 0.0;
    for (int i = 0; i < number; i++) {
        deltaHeight -= rowFormat(row)->height();
        d->rows.removeRow(row);
        deltaHeight += rowFormat(KS_rowMax)->height();
    }
    // Adjust document height (plus heights of new rows; minus heights of removed rows).
    adjustDocumentHeight(deltaHeight);

    foreach(KCSheet* sheet, map()->sheetList()) {
        sheet->changeNameCellRef(QPoint(1, row), true,
                                 KCSheet::RowRemove, sheetName(),
                                 number);
    }

    //update print settings
    d->print->removeRow(row, number);
}

QString KCSheet::changeNameCellRefHelper(const QPoint& pos, bool fullRowOrColumn, ChangeRef ref,
                                       int nbCol, const QPoint& point, bool isColumnFixed,
                                       bool isRowFixed, bool isRangeStart, bool isRangeEnd)
{
    QString newPoint;
    int col = point.x();
    int row = point.y();
    // update column
    if (isColumnFixed)
        newPoint.append('$');
    if (ref == ColumnInsert &&
            col + nbCol <= KS_colMax &&
            col >= pos.x() &&    // Column after the new one : +1
            (fullRowOrColumn || row == pos.y())) {  // All rows or just one
        newPoint += KCCell::columnName(col + nbCol);
    } else if (ref == ColumnRemove &&
               (col > pos.x() || (isRangeEnd && col == pos.x())) &&    // Column after the deleted one : -1
               (fullRowOrColumn || row == pos.y())) {  // All rows or just one
        newPoint += KCCell::columnName(col - nbCol);
    } else
        newPoint += KCCell::columnName(col);

    // Update row
    if (isRowFixed)
        newPoint.append('$');
    if (ref == RowInsert &&
            row + nbCol <= KS_rowMax &&
            row >= pos.y() &&   // Row after the new one : +1
            (fullRowOrColumn || col == pos.x())) {  // All columns or just one
        newPoint += QString::number(row + nbCol);
    } else if (ref == RowRemove &&
               (row > pos.y() || (isRangeEnd && row == pos.y())) &&   // Row after the deleted one : -1
               (fullRowOrColumn || col == pos.x())) {  // All columns or just one
        newPoint += QString::number(row - nbCol);
    } else
        newPoint += QString::number(row);

    if ((!isRangeStart && !isRangeEnd) && 
	    ((ref == ColumnRemove
            && col == pos.x() // Column is the deleted one : error
            && (fullRowOrColumn || row == pos.y())) ||
            (ref == RowRemove
             && row == pos.y() // Row is the deleted one : error
             && (fullRowOrColumn || col == pos.x())) ||
            (ref == ColumnInsert
             && col + nbCol > KS_colMax
             && col >= pos.x()     // Column after the new one : +1
             && (fullRowOrColumn || row == pos.y())) ||
            (ref == RowInsert
             && row + nbCol > KS_rowMax
             && row >= pos.y() // Row after the new one : +1
             && (fullRowOrColumn || col == pos.x())))) {
        newPoint = '#' + i18n("Dependency") + '!';
    }
    return newPoint;
}

void KCSheet::changeNameCellRef(const QPoint& pos, bool fullRowOrColumn, ChangeRef ref,
                              const QString& tabname, int nbCol)
{
    for (int c = 0; c < formulaStorage()->count(); ++c) {
        QString newText('=');
        const Tokens tokens = formulaStorage()->data(c).tokens();
        for (int t = 0; t < tokens.count(); ++t) {
            const KCToken token = tokens[t];
            switch (token.type()) {
            case KCToken::KCCell:
            case KCToken::Range: {
                if (map()->namedAreaManager()->contains(token.text())) {
                    newText.append(token.text()); // simply keep the area name
                    break;
                }
                const KCRegion region(token.text(), map());
                if (!region.isValid() || !region.isContiguous()) {
                    newText.append(token.text());
                    break;
                }
                if (!region.firstSheet() && tabname != sheetName()) {
                    // nothing to do here
                    newText.append(token.text());
                    break;
                }
                // actually only one element in here, but we need extended access to the element
                KCRegion::ConstIterator end(region.constEnd());
                for (KCRegion::ConstIterator it(region.constBegin()); it != end; ++it) {
                    KCRegion::Element* element = (*it);
                    if (element->type() == KCRegion::Element::Point) {
                        if (element->sheet())
                            newText.append(element->sheet()->sheetName() + '!');
                        QString newPoint = changeNameCellRefHelper(pos, fullRowOrColumn, ref,
                                           nbCol,
                                           element->rect().topLeft(),
                                           element->isColumnFixed(),
                                           element->isRowFixed(), false, false);
                        newText.append(newPoint);
                    } else { // (element->type() == KCRegion::Element::Range)
                        if (element->sheet())
                            newText.append(element->sheet()->sheetName() + '!');
                        QString firstPoint;
                        QString secondPoint;
                        firstPoint = changeNameCellRefHelper(pos, fullRowOrColumn, ref,
                                                           nbCol, element->rect().topLeft(),
                                                           element->isColumnFixed(),
                                                           element->isRowFixed(), true, false);
                        newText.append(firstPoint + ':');
                        secondPoint = changeNameCellRefHelper(pos, fullRowOrColumn, ref,
                                                           nbCol, element->rect().bottomRight(),
                                                           element->isColumnFixed(),
                                                           element->isRowFixed(), false, true);
                        newText.append(secondPoint);
			//If the range is "backwards", we have created a range dependency that can't be resolved.
			if ( secondPoint < firstPoint ) {	
				KCCell cell(this, formulaStorage()->col(c), formulaStorage()->row(c));
				KCFormula formula(this, cell);
				formula.setExpression("=#Dependency!");
				cell.setFormula(formula);
				return;
			}
                    }
                }
                break;
            }
            default: {
                newText.append(token.text());
                break;
            }
            }
        }

        KCCell cell(this, formulaStorage()->col(c), formulaStorage()->row(c));
        KCFormula formula(this, cell);
        formula.setExpression(newText);
        cell.setFormula(formula);
    }
}

// helper function for KCSheet::areaIsEmpty
bool KCSheet::cellIsEmpty(const KCCell& cell, TestType _type)
{
    if (!cell.isPartOfMerged()) {
        switch (_type) {
        case Text :
            if (!cell.userInput().isEmpty())
                return false;
            break;
        case KCValidity:
            if (!cell.validity().isEmpty())
                return false;
            break;
        case Comment:
            if (!cell.comment().isEmpty())
                return false;
            break;
        case ConditionalCellAttribute:
            if (cell.conditions().conditionList().count() > 0)
                return false;
            break;
        }
    }
    return true;
}

// TODO: convert into a manipulator, similar to the Dilation one
bool KCSheet::areaIsEmpty(const KCRegion& region, TestType _type)
{
    KCRegion::ConstIterator endOfList = region.constEnd();
    for (KCRegion::ConstIterator it = region.constBegin(); it != endOfList; ++it) {
        QRect range = (*it)->rect();
        // Complete rows selected ?
        if ((*it)->isRow()) {
            for (int row = range.top(); row <= range.bottom(); ++row) {
                KCCell cell = d->cellStorage->firstInRow(row);
                while (!cell.isNull()) {
                    if (!cellIsEmpty(cell, _type))
                        return false;
                    cell = d->cellStorage->nextInRow(cell.column(), row);
                }
            }
        }
        // Complete columns selected ?
        else if ((*it)->isColumn()) {
            for (int col = range.left(); col <= range.right(); ++col) {
                KCCell cell = d->cellStorage->firstInColumn(col);
                while (!cell.isNull()) {
                    if (!cellIsEmpty(cell, _type))
                        return false;
                    cell = d->cellStorage->nextInColumn(col, cell.row());
                }
            }
        } else {
            KCCell cell;
            int right  = range.right();
            int bottom = range.bottom();
            for (int x = range.left(); x <= right; ++x)
                for (int y = range.top(); y <= bottom; ++y) {
                    cell = KCCell(this, x, y);
                    if (!cellIsEmpty(cell, _type))
                        return false;
                }
        }
    }
    return true;
}

QDomElement KCSheet::saveXML(QDomDocument& dd)
{
    QDomElement sheet = dd.createElement("table");

    // backward compatibility
    QString sheetName;
    for (int i = 0; i < d->name.count(); ++i) {
        if (d->name[i].isLetterOrNumber() || d->name[i] == ' ' || d->name[i] == '.')
            sheetName.append(d->name[i]);
        else
            sheetName.append('_');
    }
    sheet.setAttribute("name", sheetName);

    //Laurent: for oasis format I think that we must use style:direction...
    sheet.setAttribute("layoutDirection", (layoutDirection() == Qt::RightToLeft) ? "rtl" : "ltr");
    sheet.setAttribute("columnnumber", (int)getShowColumnNumber());
    sheet.setAttribute("borders", (int)isShowPageBorders());
    sheet.setAttribute("hide", (int)isHidden());
    sheet.setAttribute("hidezero", (int)getHideZero());
    sheet.setAttribute("firstletterupper", (int)getFirstLetterUpper());
    sheet.setAttribute("grid", (int)getShowGrid());
    sheet.setAttribute("printGrid", (int)print()->settings()->printGrid());
    sheet.setAttribute("printCommentIndicator", (int)print()->settings()->printCommentIndicator());
    sheet.setAttribute("printFormulaIndicator", (int)print()->settings()->printFormulaIndicator());
    sheet.setAttribute("showFormula", (int)getShowFormula());
    sheet.setAttribute("showFormulaIndicator", (int)getShowFormulaIndicator());
    sheet.setAttribute("showCommentIndicator", (int)getShowCommentIndicator());
    sheet.setAttribute("lcmode", (int)getLcMode());
    sheet.setAttribute("autoCalc", (int)isAutoCalculationEnabled());
    sheet.setAttribute("borders1.2", 1);
    QByteArray pwd;
    password(pwd);
    if (!pwd.isNull()) {
        if (pwd.size() > 0) {
            QByteArray str = KCodecs::base64Encode(pwd);
            sheet.setAttribute("protected", QString(str.data()));
        } else
            sheet.setAttribute("protected", "");
    }

    // paper parameters
    QDomElement paper = dd.createElement("paper");
    paper.setAttribute("format", printSettings()->paperFormatString());
    paper.setAttribute("orientation", printSettings()->orientationString());
    sheet.appendChild(paper);

    QDomElement borders = dd.createElement("borders");
    KOdfPageLayoutData pageLayout = print()->settings()->pageLayout();
    borders.setAttribute("left", pageLayout.leftMargin);
    borders.setAttribute("top", pageLayout.topMargin);
    borders.setAttribute("right", pageLayout.rightMargin);
    borders.setAttribute("bottom", pageLayout.bottomMargin);
    paper.appendChild(borders);

    QDomElement head = dd.createElement("head");
    paper.appendChild(head);
    if (!print()->headerFooter()->headLeft().isEmpty()) {
        QDomElement left = dd.createElement("left");
        head.appendChild(left);
        left.appendChild(dd.createTextNode(print()->headerFooter()->headLeft()));
    }
    if (!print()->headerFooter()->headMid().isEmpty()) {
        QDomElement center = dd.createElement("center");
        head.appendChild(center);
        center.appendChild(dd.createTextNode(print()->headerFooter()->headMid()));
    }
    if (!print()->headerFooter()->headRight().isEmpty()) {
        QDomElement right = dd.createElement("right");
        head.appendChild(right);
        right.appendChild(dd.createTextNode(print()->headerFooter()->headRight()));
    }
    QDomElement foot = dd.createElement("foot");
    paper.appendChild(foot);
    if (!print()->headerFooter()->footLeft().isEmpty()) {
        QDomElement left = dd.createElement("left");
        foot.appendChild(left);
        left.appendChild(dd.createTextNode(print()->headerFooter()->footLeft()));
    }
    if (!print()->headerFooter()->footMid().isEmpty()) {
        QDomElement center = dd.createElement("center");
        foot.appendChild(center);
        center.appendChild(dd.createTextNode(print()->headerFooter()->footMid()));
    }
    if (!print()->headerFooter()->footRight().isEmpty()) {
        QDomElement right = dd.createElement("right");
        foot.appendChild(right);
        right.appendChild(dd.createTextNode(print()->headerFooter()->footRight()));
    }

    // print range
    QDomElement printrange = dd.createElement("printrange-rect");
    QRect _printRange = printSettings()->printRegion().lastRange();
    int left = _printRange.left();
    int right = _printRange.right();
    int top = _printRange.top();
    int bottom = _printRange.bottom();
    //If whole rows are selected, then we store zeros, as KS_colMax may change in future
    if (left == 1 && right == KS_colMax) {
        left = 0;
        right = 0;
    }
    //If whole columns are selected, then we store zeros, as KS_rowMax may change in future
    if (top == 1 && bottom == KS_rowMax) {
        top = 0;
        bottom = 0;
    }
    printrange.setAttribute("left-rect", left);
    printrange.setAttribute("right-rect", right);
    printrange.setAttribute("bottom-rect", bottom);
    printrange.setAttribute("top-rect", top);
    sheet.appendChild(printrange);

    // Print repeat columns
    QDomElement printRepeatColumns = dd.createElement("printrepeatcolumns");
    printRepeatColumns.setAttribute("left", printSettings()->repeatedColumns().first);
    printRepeatColumns.setAttribute("right", printSettings()->repeatedColumns().second);
    sheet.appendChild(printRepeatColumns);

    // Print repeat rows
    QDomElement printRepeatRows = dd.createElement("printrepeatrows");
    printRepeatRows.setAttribute("top", printSettings()->repeatedRows().first);
    printRepeatRows.setAttribute("bottom", printSettings()->repeatedRows().second);
    sheet.appendChild(printRepeatRows);

    //Save print zoom
    sheet.setAttribute("printZoom", printSettings()->zoom());

    //Save page limits
    const QSize pageLimits = printSettings()->pageLimits();
    sheet.setAttribute("printPageLimitX", pageLimits.width());
    sheet.setAttribute("printPageLimitY", pageLimits.height());

    // Save all cells.
    const QRect usedArea = this->usedArea();
    for (int row = 1; row <= usedArea.height(); ++row) {
        KCCell cell = d->cellStorage->firstInRow(row);
        while (!cell.isNull()) {
            QDomElement e = cell.save(dd);
            if (!e.isNull())
                sheet.appendChild(e);
            cell = d->cellStorage->nextInRow(cell.column(), row);
        }
    }

    // Save all KCRowFormat objects.
    KCRowFormat* rowFormat = firstRow();
    int styleIndex = styleStorage()->nextRowStyleIndex(0);
    while (rowFormat || styleIndex) {
        if (rowFormat && (!styleIndex || rowFormat->row() <= styleIndex)) {
            QDomElement e = rowFormat->save(dd);
            if (e.isNull())
                return QDomElement();
            sheet.appendChild(e);
            if (rowFormat->row() == styleIndex)
                styleIndex = styleStorage()->nextRowStyleIndex(styleIndex);
            rowFormat = rowFormat->next();
        } else if (styleIndex) {
            KCRowFormat rowFormat(*map()->defaultRowFormat());
            rowFormat.setSheet(this);
            rowFormat.setRow(styleIndex);
            QDomElement e = rowFormat.save(dd);
            if (e.isNull())
                return QDomElement();
            sheet.appendChild(e);
            styleIndex = styleStorage()->nextRowStyleIndex(styleIndex);
        }
    }

    // Save all KCColumnFormat objects.
    KCColumnFormat* columnFormat = firstCol();
    styleIndex = styleStorage()->nextColumnStyleIndex(0);
    while (columnFormat || styleIndex) {
        if (columnFormat && (!styleIndex || columnFormat->column() <= styleIndex)) {
            QDomElement e = columnFormat->save(dd);
            if (e.isNull())
                return QDomElement();
            sheet.appendChild(e);
            if (columnFormat->column() == styleIndex)
                styleIndex = styleStorage()->nextColumnStyleIndex(styleIndex);
            columnFormat = columnFormat->next();
        } else if (styleIndex) {
            KCColumnFormat columnFormat(*map()->defaultColumnFormat());
            columnFormat.setSheet(this);
            columnFormat.setColumn(styleIndex);
            QDomElement e = columnFormat.save(dd);
            if (e.isNull())
                return QDomElement();
            sheet.appendChild(e);
            styleIndex = styleStorage()->nextColumnStyleIndex(styleIndex);
        }
    }
#if 0 // KCELLS_KOPART_EMBEDDING
    foreach(EmbeddedObject* object, doc()->embeddedObjects()) {
        if (object->sheet() == this) {
            QDomElement e = object->save(dd);

            if (e.isNull())
                return QDomElement();
            sheet.appendChild(e);
        }
    }
#endif // KCELLS_KOPART_EMBEDDING
    return sheet;
}

bool KCSheet::isLoading()
{
    return map()->isLoading();
}

void KCSheet::checkContentDirection(QString const & name)
{
    /* set sheet's direction to RTL if sheet name is an RTL string */
    if ((name.isRightToLeft()))
        setLayoutDirection(Qt::RightToLeft);
    else
        setLayoutDirection(Qt::LeftToRight);
}

bool KCSheet::loadSheetStyleFormat(KXmlElement *style)
{
    QString hleft, hmiddle, hright;
    QString fleft, fmiddle, fright;
    KXmlNode header = KoXml::namedItemNS(*style, KOdfXmlNS::style, "header");

    if (!header.isNull()) {
        kDebug(36003) << "Header exists";
        KXmlNode part = KoXml::namedItemNS(header, KOdfXmlNS::style, "region-left");
        if (!part.isNull()) {
            hleft = getPart(part);
            kDebug(36003) << "Header left:" << hleft;
        } else
            kDebug(36003) << "KCStyle:region:left doesn't exist!";
        part = KoXml::namedItemNS(header, KOdfXmlNS::style, "region-center");
        if (!part.isNull()) {
            hmiddle = getPart(part);
            kDebug(36003) << "Header middle:" << hmiddle;
        }
        part = KoXml::namedItemNS(header, KOdfXmlNS::style, "region-right");
        if (!part.isNull()) {
            hright = getPart(part);
            kDebug(36003) << "Header right:" << hright;
        }
        //If Header doesn't have region tag add it to Left
        hleft.append(getPart(header));
    }
    //TODO implement it under kcells
    KXmlNode headerleft = KoXml::namedItemNS(*style, KOdfXmlNS::style, "header-left");
    if (!headerleft.isNull()) {
        KXmlElement e = headerleft.toElement();
        if (e.hasAttributeNS(KOdfXmlNS::style, "display"))
            kDebug(36003) << "header.hasAttribute( style:display ) :" << e.hasAttributeNS(KOdfXmlNS::style, "display");
        else
            kDebug(36003) << "header left doesn't has attribute  style:display";
    }
    //TODO implement it under kcells
    KXmlNode footerleft = KoXml::namedItemNS(*style, KOdfXmlNS::style, "footer-left");
    if (!footerleft.isNull()) {
        KXmlElement e = footerleft.toElement();
        if (e.hasAttributeNS(KOdfXmlNS::style, "display"))
            kDebug(36003) << "footer.hasAttribute( style:display ) :" << e.hasAttributeNS(KOdfXmlNS::style, "display");
        else
            kDebug(36003) << "footer left doesn't has attribute  style:display";
    }

    KXmlNode footer = KoXml::namedItemNS(*style, KOdfXmlNS::style, "footer");

    if (!footer.isNull()) {
        KXmlNode part = KoXml::namedItemNS(footer, KOdfXmlNS::style, "region-left");
        if (!part.isNull()) {
            fleft = getPart(part);
            kDebug(36003) << "Footer left:" << fleft;
        }
        part = KoXml::namedItemNS(footer, KOdfXmlNS::style, "region-center");
        if (!part.isNull()) {
            fmiddle = getPart(part);
            kDebug(36003) << "Footer middle:" << fmiddle;
        }
        part = KoXml::namedItemNS(footer, KOdfXmlNS::style, "region-right");
        if (!part.isNull()) {
            fright = getPart(part);
            kDebug(36003) << "Footer right:" << fright;
        }
        //If Footer doesn't have region tag add it to Left
        fleft.append(getPart(footer));
    }

    print()->headerFooter()->setHeadFootLine(hleft, hmiddle, hright,
            fleft, fmiddle, fright);
    return true;
}

void KCSheet::replaceMacro(QString & text, const QString & old, const QString & newS)
{
    int n = text.indexOf(old);
    if (n != -1)
        text = text.replace(n, old.length(), newS);
}

QString KCSheet::getPart(const KXmlNode & part)
{
    QString result;
    KXmlElement e = KoXml::namedItemNS(part, KOdfXmlNS::text, "p");
    while (!e.isNull()) {
        QString text = e.text();

        KXmlElement macro = KoXml::namedItemNS(e, KOdfXmlNS::text, "time");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<time>");

        macro = KoXml::namedItemNS(e, KOdfXmlNS::text, "date");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<date>");

        macro = KoXml::namedItemNS(e, KOdfXmlNS::text, "page-number");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<page>");

        macro = KoXml::namedItemNS(e, KOdfXmlNS::text, "page-count");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<pages>");

        macro = KoXml::namedItemNS(e, KOdfXmlNS::text, "sheet-name");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<sheet>");

        macro = KoXml::namedItemNS(e, KOdfXmlNS::text, "title");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<name>");

        macro = KoXml::namedItemNS(e, KOdfXmlNS::text, "file-name");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<file>");

        //add support for multi line into kcells
        if (!result.isEmpty())
            result += '\n';
        result += text;
        e = e.nextSibling().toElement();
    }

    return result;
}

void KCSheet::loadColumnNodes(const KXmlElement& parent,
                            int& indexCol,
                            int& maxColumn,
                            KOdfLoadingContext& odfContext,
                            QHash<QString, QRegion>& columnStyleRegions,
                            IntervalMap<QString>& columnStyles
                            )
{
    KXmlNode node = parent.firstChild();
    while (!node.isNull()) {
        KXmlElement elem = node.toElement();
        if (!elem.isNull() && elem.namespaceURI() == KOdfXmlNS::table) {
            if (elem.localName() == "table-column") {
                loadColumnFormat(elem, odfContext.stylesReader(), indexCol, columnStyleRegions, columnStyles);
                maxColumn = qMax(maxColumn, indexCol - 1);
            } else if (elem.localName() == "table-column-group") {
                loadColumnNodes(elem, indexCol, maxColumn, odfContext, columnStyleRegions, columnStyles);
            }
        }
        node = node.nextSibling();
    }
}

void KCSheet::loadRowNodes(const KXmlElement& parent,
                            int& rowIndex,
                            int& maxColumn,
                            KCOdfLoadingContext& tableContext,
                            QHash<QString, QRegion>& rowStyleRegions,
                            QHash<QString, QRegion>& cellStyleRegions,
                            const IntervalMap<QString>& columnStyles,
                            const Styles& autoStyles
                            )
{
    KXmlNode node = parent.firstChild();
    while (!node.isNull()) {
        KXmlElement elem = node.toElement();
        if (!elem.isNull() && elem.namespaceURI() == KOdfXmlNS::table) {
            if (elem.localName() == "table-row") {
                int columnMaximal = loadRowFormat(elem, rowIndex, tableContext,
                                                        rowStyleRegions, cellStyleRegions,
                                                        columnStyles, autoStyles);
                // allow the row to define more columns then defined via table-column
                maxColumn = qMax(maxColumn, columnMaximal);
            } else if (elem.localName() == "table-row-group") {
                loadRowNodes(elem, rowIndex, maxColumn, tableContext, rowStyleRegions, cellStyleRegions, columnStyles, autoStyles);
            }
        }
        node = node.nextSibling();
    }
}


bool KCSheet::loadOdf(const KXmlElement& sheetElement,
                    KCOdfLoadingContext& tableContext,
                    const Styles& autoStyles,
                    const QHash<QString, KCConditions>& conditionalStyles)
{

    QPointer<KoUpdater> updater;
    if (doc() && doc()->progressUpdater()) {
        updater = doc()->progressUpdater()->startSubtask(1,
                                                     "KCells::KCSheet::loadOdf");
        updater->setProgress(0);
    }

    KOdfLoadingContext& odfContext = tableContext.odfContext;
    if (sheetElement.hasAttributeNS(KOdfXmlNS::table, "style-name")) {
        QString stylename = sheetElement.attributeNS(KOdfXmlNS::table, "style-name", QString());
        //kDebug(36003)<<" style of table :"<<stylename;
        const KXmlElement *style = odfContext.stylesReader().findStyle(stylename, "table");
        Q_ASSERT(style);
        //kDebug(36003)<<" style :"<<style;
        if (style) {
            KXmlElement properties(KoXml::namedItemNS(*style, KOdfXmlNS::style, "table-properties"));
            if (!properties.isNull()) {
                if (properties.hasAttributeNS(KOdfXmlNS::table, "display")) {
                    bool visible = (properties.attributeNS(KOdfXmlNS::table, "display", QString()) == "true" ? true : false);
                    setHidden(!visible);
                }
            }
            if (style->hasAttributeNS(KOdfXmlNS::style, "master-page-name")) {
                QString masterPageStyleName = style->attributeNS(KOdfXmlNS::style, "master-page-name", QString());
                //kDebug()<<"style->attribute( style:master-page-name ) :"<<masterPageStyleName;
                KXmlElement *masterStyle = odfContext.stylesReader().masterPages()[masterPageStyleName];
                //kDebug()<<"stylesReader.styles()[masterPageStyleName] :"<<masterStyle;
                if (masterStyle) {
                    loadSheetStyleFormat(masterStyle);
                    if (masterStyle->hasAttributeNS(KOdfXmlNS::style, "page-layout-name")) {
                        QString masterPageLayoutStyleName = masterStyle->attributeNS(KOdfXmlNS::style, "page-layout-name", QString());
                        //kDebug(36003)<<"masterPageLayoutStyleName :"<<masterPageLayoutStyleName;
                        const KXmlElement *masterLayoutStyle = odfContext.stylesReader().findStyle(masterPageLayoutStyleName);
                        if (masterLayoutStyle) {
                            //kDebug(36003)<<"masterLayoutStyle :"<<masterLayoutStyle;
                            KOdfStyleStack styleStack;
                            styleStack.setTypeProperties("page-layout");
                            styleStack.push(*masterLayoutStyle);
                            loadOdfMasterLayoutPage(styleStack);
                        }
                    }
                }
            }

            if (style->hasChildNodes() ) {
                KXmlElement element;
                forEachElement(element, properties) {
                    if (element.nodeName() == "style:background-image") {
                        QString imagePath = element.attributeNS(KOdfXmlNS::xlink, "href");
                        KOdfStore* store = tableContext.odfContext.store();
                        if (store->hasFile(imagePath)) {
                            QByteArray data;
                            store->extractFile(imagePath, data);
                            QImage image = QImage::fromData(data);

                            if( image.isNull() ) {
                                continue;
                            }

                            setBackgroundImage(image);

                            BackgroundImageProperties bgProperties;
                            if( element.hasAttribute("draw:opacity") ) {
                                QString opacity = element.attribute("draw:opacity", "");
                                if( opacity.endsWith('%') ) {
                                    opacity = opacity.left(opacity.size() - 2);
                                }
                                bool ok;
                                float opacityFloat = opacity.toFloat( &ok );
                                if( ok ) {
                                    bgProperties.opacity = opacityFloat;
                                }
                            }
                            //TODO
                            //if( element.hasAttribute("style:filterName") ) {
                            //}
                            if( element.hasAttribute("style:position") ) {
                                const QString positionAttribute = element.attribute("style:position","");
                                const QStringList positionList = positionAttribute.split(' ', QString::SkipEmptyParts);
                                if( positionList.size() == 1) {
                                    const QString position = positionList.at(0);
                                    if( position == "left" ) {
                                        bgProperties.horizontalPosition = BackgroundImageProperties::Left;
                                    }
                                    if( position == "center" ) {
                                        //NOTE the standard is too vague to know what center alone means, we assume that it means both centered
                                        bgProperties.horizontalPosition = BackgroundImageProperties::HorizontalCenter;
                                        bgProperties.verticalPosition = BackgroundImageProperties::VerticalCenter;
                                    }
                                    if( position == "right" ) {
                                        bgProperties.horizontalPosition = BackgroundImageProperties::Right;
                                    }
                                    if( position == "top" ) {
                                        bgProperties.verticalPosition = BackgroundImageProperties::Top;
                                    }
                                    if( position == "bottom" ) {
                                        bgProperties.verticalPosition = BackgroundImageProperties::Bottom;
                                    }
                                }
                                else if (positionList.size() == 2) {
                                    const QString verticalPosition = positionList.at(0);
                                    const QString horizontalPosition = positionList.at(1);
                                    if( horizontalPosition == "left" ) {
                                        bgProperties.horizontalPosition = BackgroundImageProperties::Left;
                                    }
                                    if( horizontalPosition == "center" ) {
                                        bgProperties.horizontalPosition = BackgroundImageProperties::HorizontalCenter;
                                    }
                                    if( horizontalPosition == "right" ) {
                                        bgProperties.horizontalPosition = BackgroundImageProperties::Right;
                                    }
                                    if( verticalPosition == "top" ) {
                                        bgProperties.verticalPosition = BackgroundImageProperties::Top;
                                    }
                                    if( verticalPosition == "center" ) {
                                        bgProperties.verticalPosition = BackgroundImageProperties::VerticalCenter;
                                    }
                                    if( verticalPosition == "bottom" ) {
                                        bgProperties.verticalPosition = BackgroundImageProperties::Bottom;
                                    }
                                }
                            }
                            if( element.hasAttribute("style:repeat") ) {
                                const QString repeat = element.attribute("style:repeat");
                                if( repeat == "no-repeat" ) {
                                    bgProperties.repeat = BackgroundImageProperties::NoRepeat;
                                }
                                if( repeat == "repeat" ) {
                                    bgProperties.repeat = BackgroundImageProperties::Repeat;
                                }
                                if( repeat == "stretch" ) {
                                    bgProperties.repeat = BackgroundImageProperties::Stretch;
                                }
                            }
                            setBackgroundImageProperties(bgProperties);
                        }
                    }
                }

            }
        }
    }

    // KCCell style regions
    QHash<QString, QRegion> cellStyleRegions;
    // KCCell style regions (row defaults)
    QHash<QString, QRegion> rowStyleRegions;
    // KCCell style regions (column defaults)
    QHash<QString, QRegion> columnStyleRegions;
    IntervalMap<QString> columnStyles;

    int rowIndex = 1;
    int indexCol = 1;
    int maxColumn = 1;
    KXmlNode rowNode = sheetElement.firstChild();
    // Some spreadsheet programs may support more rows than
    // KCells so limit the number of repeated rows.
    // FIXME POSSIBLE DATA LOSS!

    // First load all style information for rows, columns and cells
    while (!rowNode.isNull() && rowIndex <= KS_rowMax) {
        //kDebug(36003) << " rowIndex :" << rowIndex << " indexCol :" << indexCol;
        KXmlElement rowElement = rowNode.toElement();
        if (!rowElement.isNull()) {
            // slightly faster
            KoXml::load(rowElement);

            //kDebug(36003) << " KCSheet::loadOdf rowElement.tagName() :" << rowElement.localName();
            if (rowElement.namespaceURI() == KOdfXmlNS::table) {
                if (rowElement.localName() == "table-header-columns") {
                    // NOTE Handle header cols as ordinary ones
                    //      as long as they're not supported.
                    loadColumnNodes(rowElement, indexCol, maxColumn, odfContext, columnStyleRegions, columnStyles);
                } else if (rowElement.localName() == "table-column-group") {
                    loadColumnNodes(rowElement, indexCol, maxColumn, odfContext, columnStyleRegions, columnStyles);
                } else if (rowElement.localName() == "table-column" && indexCol <= KS_colMax) {
                    //kDebug(36003) << " table-column found : index column before" << indexCol;
                    loadColumnFormat(rowElement, odfContext.stylesReader(), indexCol, columnStyleRegions, columnStyles);
                    //kDebug(36003) << " table-column found : index column after" << indexCol;
                    maxColumn = qMax(maxColumn, indexCol - 1);
                } else if (rowElement.localName() == "table-header-rows") {
                    // NOTE Handle header rows as ordinary ones
                    //      as long as they're not supported.
                    loadRowNodes(rowElement, rowIndex, maxColumn, tableContext, rowStyleRegions, cellStyleRegions, columnStyles, autoStyles);
                } else if (rowElement.localName() == "table-row-group") {
                    loadRowNodes(rowElement, rowIndex, maxColumn, tableContext, rowStyleRegions, cellStyleRegions, columnStyles, autoStyles);
                } else if (rowElement.localName() == "table-row") {
                    //kDebug(36003) << " table-row found :index row before" << rowIndex;
                    int columnMaximal = loadRowFormat(rowElement, rowIndex, tableContext,
                                  rowStyleRegions, cellStyleRegions, columnStyles, autoStyles);
                    // allow the row to define more columns then defined via table-column
                    maxColumn = qMax(maxColumn, columnMaximal);
                    //kDebug(36003) << " table-row found :index row after" << rowIndex;
                } else if (rowElement.localName() == "shapes") {
                    // OpenDocument v1.1, 8.3.4 Shapes:
                    // The <table:shapes> element contains all graphic shapes
                    // with an anchor on the table this element is a child of.
                    KShapeLoadingContext* shapeLoadingContext = tableContext.shapeContext;
                    KXmlElement element;
                    forEachElement(element, rowElement) {
                        if (element.namespaceURI() != KOdfXmlNS::draw)
                            continue;
                        loadOdfObject(element, *shapeLoadingContext);
                    }
                }
            }

            // don't need it anymore
            KoXml::unload(rowElement);
        }

        rowNode = rowNode.nextSibling();

        int count = map()->increaseLoadedRowsCounter();
        if (updater && count >= 0) updater->setProgress(count);
    }

    QList<QPair<QRegion, KCStyle> > styleRegions;
    QList<QPair<QRegion, KCConditions> > conditionRegions;
    // insert the styles into the storage (column defaults)
    kDebug(36003) << "Inserting column default cell styles ...";
    loadOdfInsertStyles(autoStyles, columnStyleRegions, conditionalStyles,
                        QRect(1, 1, maxColumn, rowIndex - 1), styleRegions, conditionRegions);
    // insert the styles into the storage (row defaults)
    kDebug(36003) << "Inserting row default cell styles ...";
    loadOdfInsertStyles(autoStyles, rowStyleRegions, conditionalStyles,
                        QRect(1, 1, maxColumn, rowIndex - 1), styleRegions, conditionRegions);
    // insert the styles into the storage
    kDebug(36003) << "Inserting cell styles ...";
    loadOdfInsertStyles(autoStyles, cellStyleRegions, conditionalStyles,
                        QRect(1, 1, maxColumn, rowIndex - 1), styleRegions, conditionRegions);

    cellStorage()->loadStyles(styleRegions);
    cellStorage()->loadConditions(conditionRegions);

    if (sheetElement.hasAttributeNS(KOdfXmlNS::table, "print-ranges")) {
        // e.g.: Sheet4.A1:Sheet4.E28
        QString range = sheetElement.attributeNS(KOdfXmlNS::table, "print-ranges", QString());
        KCRegion region(KCRegion::loadOdf(range));
        if (!region.firstSheet() || sheetName() == region.firstSheet()->sheetName())
            printSettings()->setPrintRegion(region);
    }

    if (sheetElement.attributeNS(KOdfXmlNS::table, "protected", QString()) == "true") {
        loadOdfProtection(sheetElement);
    }
    return true;
}

void KCSheet::loadOdfObject(const KXmlElement& element, KShapeLoadingContext& shapeContext)
{
    KShape* shape = KShapeRegistry::instance()->createShapeFromOdf(element, shapeContext);
    if (!shape)
        return;
    addShape(shape);
    dynamic_cast<KCShapeApplicationData*>(shape->applicationData())->setAnchoredToCell(false);
}

void KCSheet::loadOdfMasterLayoutPage(KOdfStyleStack &styleStack)
{
    KOdfPageLayoutData pageLayout;

    if (styleStack.hasProperty(KOdfXmlNS::fo, "page-width")) {
        pageLayout.width = KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "page-width"));
    }
    if (styleStack.hasProperty(KOdfXmlNS::fo, "page-height")) {
        pageLayout.height = KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "page-height"));
    }
    if (styleStack.hasProperty(KOdfXmlNS::fo, "margin-top")) {
        pageLayout.topMargin = KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "margin-top"));
    }
    if (styleStack.hasProperty(KOdfXmlNS::fo, "margin-bottom")) {
        pageLayout.bottomMargin = KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "margin-bottom"));
    }
    if (styleStack.hasProperty(KOdfXmlNS::fo, "margin-left")) {
        pageLayout.leftMargin = KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "margin-left"));
    }
    if (styleStack.hasProperty(KOdfXmlNS::fo, "margin-right")) {
        pageLayout.rightMargin = KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "margin-right"));
    }
    if (styleStack.hasProperty(KOdfXmlNS::style, "writing-mode")) {
        kDebug(36003) << "styleStack.hasAttribute( style:writing-mode ) :" << styleStack.hasProperty(KOdfXmlNS::style, "writing-mode");
        const QString writingMode = styleStack.property(KOdfXmlNS::style, "writing-mode");
        if (writingMode == "lr-tb") {
            setLayoutDirection(Qt::LeftToRight);
        } else if (writingMode == "rl-tb") {
            setLayoutDirection(Qt::RightToLeft);
        } else {
            // Set the layout direction to the direction of the sheet name.
            checkContentDirection(sheetName());
        }
        //TODO
        //<value>lr-tb</value>
        //<value>rl-tb</value>
        //<value>tb-rl</value>
        //<value>tb-lr</value>
        //<value>lr</value>
        //<value>rl</value>
        //<value>tb</value>
        //<value>page</value>

    } else {
        // Set the layout direction to the direction of the sheet name.
        checkContentDirection(sheetName());
    }
    if (styleStack.hasProperty(KOdfXmlNS::style, "print-orientation")) {
        pageLayout.orientation = (styleStack.property(KOdfXmlNS::style, "print-orientation") == "landscape")
                                 ? KOdfPageFormat::Landscape : KOdfPageFormat::Portrait;
    }
    if (styleStack.hasProperty(KOdfXmlNS::style, "num-format")) {
        //not implemented into kcells
        //These attributes specify the numbering style to use.
        //If a numbering style is not specified, the numbering style is inherited from
        //the page style. See section 6.7.8 for information on these attributes
        kDebug(36003) << " num-format :" << styleStack.property(KOdfXmlNS::style, "num-format");

    }
    if (styleStack.hasProperty(KOdfXmlNS::fo, "background-color")) {
        //TODO
        kDebug(36003) << " fo:background-color :" << styleStack.property(KOdfXmlNS::fo, "background-color");
    }
    if (styleStack.hasProperty(KOdfXmlNS::style, "print")) {
        //todo parsing
        QString str = styleStack.property(KOdfXmlNS::style, "print");
        kDebug(36003) << " style:print :" << str;

        if (str.contains("headers")) {
            //TODO implement it into kcells
        }
        if (str.contains("grid")) {
            print()->settings()->setPrintGrid(true);
        }
        if (str.contains("annotations")) {
            //TODO it's not implemented
        }
        if (str.contains("objects")) {
            //TODO it's not implemented
        }
        if (str.contains("charts")) {
            //TODO it's not implemented
        }
        if (str.contains("drawings")) {
            //TODO it's not implemented
        }
        if (str.contains("formulas")) {
            d->showFormula = true;
        }
        if (str.contains("zero-values")) {
            //TODO it's not implemented
        }
    }
    if (styleStack.hasProperty(KOdfXmlNS::style, "table-centering")) {
        QString str = styleStack.property(KOdfXmlNS::style, "table-centering");
        //TODO not implemented into kcells
        kDebug(36003) << " styleStack.attribute( style:table-centering ) :" << str;
#if 0
        if (str == "horizontal") {
        } else if (str == "vertical") {
        } else if (str == "both") {
        } else if (str == "none") {
        } else
            kDebug(36003) << " table-centering unknown :" << str;
#endif
    }
    print()->settings()->setPageLayout(pageLayout);
}


bool KCSheet::loadColumnFormat(const KXmlElement& column,
                             const KOdfStylesReader& stylesReader, int & indexCol,
                             QHash<QString, QRegion>& columnStyleRegions, IntervalMap<QString>& columnStyles)
{
//   kDebug(36003)<<"bool KCSheet::loadColumnFormat(const KXmlElement& column, const KOdfStylesReader& stylesReader, unsigned int & indexCol ) index Col :"<<indexCol;

    bool isNonDefaultColumn = false;

    int number = 1;
    if (column.hasAttributeNS(KOdfXmlNS::table, "number-columns-repeated")) {
        bool ok = true;
        int n = column.attributeNS(KOdfXmlNS::table, "number-columns-repeated", QString()).toInt(&ok);
        if (ok)
            // Some spreadsheet programs may support more rows than KCells so
            // limit the number of repeated rows.
            // FIXME POSSIBLE DATA LOSS!
            number = qMin(n, KS_colMax - indexCol + 1);
        //kDebug(36003) << "Repeated:" << number;
    }

    if (column.hasAttributeNS(KOdfXmlNS::table, "default-cell-style-name")) {
        const QString styleName = column.attributeNS(KOdfXmlNS::table, "default-cell-style-name", QString());
        if (!styleName.isEmpty()) {
            columnStyleRegions[styleName] += QRect(indexCol, 1, number, KS_rowMax);
            columnStyles.insert(indexCol, indexCol+number-1, styleName);
        }
    }

    enum { Visible, Collapsed, Filtered } visibility = Visible;
    if (column.hasAttributeNS(KOdfXmlNS::table, "visibility")) {
        const QString string = column.attributeNS(KOdfXmlNS::table, "visibility", "visible");
        if (string == "collapse")
            visibility = Collapsed;
        else if (string == "filter")
            visibility = Filtered;
        isNonDefaultColumn = true;
    }

    KOdfStyleStack styleStack;
    if (column.hasAttributeNS(KOdfXmlNS::table, "style-name")) {
        QString str = column.attributeNS(KOdfXmlNS::table, "style-name", QString());
        const KXmlElement *style = stylesReader.findStyle(str, "table-column");
        if (style) {
            styleStack.push(*style);
            isNonDefaultColumn = true;
        }
    }
    styleStack.setTypeProperties("table-column"); //style for column

    double width = -1.0;
    if (styleStack.hasProperty(KOdfXmlNS::style, "column-width")) {
        width = KUnit::parseValue(styleStack.property(KOdfXmlNS::style, "column-width") , -1.0);
        //kDebug(36003) << " style:column-width : width :" << width;
        isNonDefaultColumn = true;
    }

    bool insertPageBreak = false;
    if (styleStack.hasProperty(KOdfXmlNS::fo, "break-before")) {
        QString str = styleStack.property(KOdfXmlNS::fo, "break-before");
        if (str == "page") {
            insertPageBreak = true;
        } else {
            // kDebug(36003) << " str :" << str;
        }
        isNonDefaultColumn = true;
    } else if (styleStack.hasProperty(KOdfXmlNS::fo, "break-after")) {
        // TODO
    }

    // If it's a default column, we can return here.
    // This saves the iteration, which can be caused by column cell default styles,
    // but which are not inserted here.
    if (!isNonDefaultColumn) {
        indexCol += number;
        return true;
    }

    for (int i = 0; i < number; ++i) {
        //kDebug(36003) << " insert new column: pos :" << indexCol << " width :" << width << " hidden ?" << visibility;

        const KCColumnFormat* columnFormat;
        if (isNonDefaultColumn) {
            KCColumnFormat* cf = nonDefaultColumnFormat(indexCol);
            columnFormat = cf;

            if (width != -1.0)   //safe
                cf->setWidth(width);
            if (insertPageBreak) {
                cf->setPageBreak(true);
            }
            if (visibility == Collapsed)
                cf->setHidden(true);
            else if (visibility == Filtered)
                cf->setFiltered(true);

            cf->setPageBreak(insertPageBreak);
        } else {
            columnFormat = this->columnFormat(indexCol);
        }
        ++indexCol;
    }
//     kDebug(36003)<<" after index column !!!!!!!!!!!!!!!!!! :"<<indexCol;
    return true;
}

void KCSheet::loadOdfInsertStyles(const Styles& autoStyles,
                                const QHash<QString, QRegion>& styleRegions,
                                const QHash<QString, KCConditions>& conditionalStyles,
                                const QRect& usedArea,
                                QList<QPair<QRegion, KCStyle> >& outStyleRegions,
                                QList<QPair<QRegion, KCConditions> >& outConditionalStyles)
{
    const QList<QString> styleNames = styleRegions.keys();
    for (int i = 0; i < styleNames.count(); ++i) {
        if (!autoStyles.contains(styleNames[i]) && !map()->styleManager()->style(styleNames[i])) {
            kWarning(36003) << "\t" << styleNames[i] << " not used";
            continue;
        }
        const bool hasConditions = conditionalStyles.contains(styleNames[i]);
        const QRegion styleRegion = styleRegions[styleNames[i]] & QRegion(usedArea);
        if (hasConditions)
            outConditionalStyles.append(qMakePair(styleRegion, conditionalStyles[styleNames[i]]));
        if (autoStyles.contains(styleNames[i])) {
            //kDebug(36003) << "\tautomatic:" << styleNames[i] << " at" << styleRegion.rectCount() << "rects";
            KCStyle style;
            style.setDefault(); // "overwrite" existing style
            style.merge(autoStyles[styleNames[i]]);
            outStyleRegions.append(qMakePair(styleRegion, style));
        } else {
            const KCCustomStyle* namedStyle = map()->styleManager()->style(styleNames[i]);
            //kDebug(36003) << "\tcustom:" << namedStyle->name() << " at" << styleRegion.rectCount() << "rects";
            KCStyle style;
            style.setDefault(); // "overwrite" existing style
            style.merge(*namedStyle);
            outStyleRegions.append(qMakePair(styleRegion, style));
        }
    }
}

int KCSheet::loadRowFormat(const KXmlElement& row, int &rowIndex,
                          KCOdfLoadingContext& tableContext,
                          QHash<QString, QRegion>& rowStyleRegions,
                          QHash<QString, QRegion>& cellStyleRegions,
                          const IntervalMap<QString>& columnStyles,
                          const Styles& autoStyles)
{
    static const QString sStyleName             = QString::fromLatin1("style-name");
    static const QString sNumberRowsRepeated    = QString::fromLatin1("number-rows-repeated");
    static const QString sDefaultCellStyleName  = QString::fromLatin1("default-cell-style-name");
    static const QString sVisibility            = QString::fromLatin1("visibility");
    static const QString sVisible               = QString::fromLatin1("visible");
    static const QString sCollapse              = QString::fromLatin1("collapse");
    static const QString sFilter                = QString::fromLatin1("filter");
    static const QString sPage                  = QString::fromLatin1("page");
    static const QString sTableCell             = QString::fromLatin1("table-cell");
    static const QString sCoveredTableCell      = QString::fromLatin1("covered-table-cell");
    static const QString sNumberColumnsRepeated = QString::fromLatin1("number-columns-repeated");

//    kDebug(36003)<<"KCSheet::loadRowFormat( const KXmlElement& row, int &rowIndex,const KOdfStylesReader& stylesReader, bool isLast )***********";
    KOdfLoadingContext& odfContext = tableContext.odfContext;
    bool isNonDefaultRow = false;

    KOdfStyleStack styleStack;
    if (row.hasAttributeNS(KOdfXmlNS::table, sStyleName)) {
        QString str = row.attributeNS(KOdfXmlNS::table, sStyleName, QString());
        const KXmlElement *style = odfContext.stylesReader().findStyle(str, "table-row");
        if (style) {
            styleStack.push(*style);
            isNonDefaultRow = true;
        }
    }
    styleStack.setTypeProperties("table-row");

    int number = 1;
    if (row.hasAttributeNS(KOdfXmlNS::table, sNumberRowsRepeated)) {
        bool ok = true;
        int n = row.attributeNS(KOdfXmlNS::table, sNumberRowsRepeated, QString()).toInt(&ok);
        if (ok)
            // Some spreadsheet programs may support more rows than KCells so
            // limit the number of repeated rows.
            // FIXME POSSIBLE DATA LOSS!
            number = qMin(n, KS_rowMax - rowIndex + 1);
    }

    QString rowCellStyleName;
    if (row.hasAttributeNS(KOdfXmlNS::table, sDefaultCellStyleName)) {
        rowCellStyleName = row.attributeNS(KOdfXmlNS::table, sDefaultCellStyleName, QString());
        if (!rowCellStyleName.isEmpty()) {
            rowStyleRegions[rowCellStyleName] += QRect(1, rowIndex, KS_colMax, number);
        }
    }

    double height = -1.0;
    if (styleStack.hasProperty(KOdfXmlNS::style, "row-height")) {
        height = KUnit::parseValue(styleStack.property(KOdfXmlNS::style, "row-height") , -1.0);
        //    kDebug(36003)<<" properties style:row-height : height :"<<height;
        isNonDefaultRow = true;
    }

    enum { Visible, Collapsed, Filtered } visibility = Visible;
    if (row.hasAttributeNS(KOdfXmlNS::table, sVisibility)) {
        const QString string = row.attributeNS(KOdfXmlNS::table, sVisibility, sVisible);
        if (string == sCollapse)
            visibility = Collapsed;
        else if (string == sFilter)
            visibility = Filtered;
        isNonDefaultRow = true;
    }

    bool insertPageBreak = false;
    if (styleStack.hasProperty(KOdfXmlNS::fo, "break-before")) {
        QString str = styleStack.property(KOdfXmlNS::fo, "break-before");
        if (str == sPage) {
            insertPageBreak = true;
        }
        //  else
        //      kDebug(36003)<<" str :"<<str;
        isNonDefaultRow = true;
    } else if (styleStack.hasProperty(KOdfXmlNS::fo, "break-after")) {
        // TODO
    }

//     kDebug(36003)<<" create non defaultrow format :"<<rowIndex<<" repeate :"<<number<<" height :"<<height;
    if (isNonDefaultRow) {
        for (int r = 0; r < number; ++r) {
            KCRowFormat* rowFormat = nonDefaultRowFormat(rowIndex + r);
            if (height != -1.0)
                rowFormat->setHeight(height);
            if (insertPageBreak) {
                rowFormat->setPageBreak(true);
            }
            if (visibility == Collapsed)
                rowFormat->setHidden(true);
            else if (visibility == Filtered)
                rowFormat->setFiltered(true);

            rowFormat->setPageBreak(insertPageBreak);
        }
    }

    int columnIndex = 1;
    int columnMaximal = 0;
    const int endRow = qMin(rowIndex + number - 1, KS_rowMax);

    KXmlElement cellElement;
    forEachElement(cellElement, row) {
        if (cellElement.namespaceURI() != KOdfXmlNS::table)
            continue;
        if (cellElement.localName() != sTableCell && cellElement.localName() != sCoveredTableCell)
            continue;


        bool ok = false;
        const int n = cellElement.attributeNS(KOdfXmlNS::table, sNumberColumnsRepeated, QString()).toInt(&ok);
        // Some spreadsheet programs may support more columns than
        // KCells so limit the number of repeated columns.
        // FIXME POSSIBLE DATA LOSS!
        const int numberColumns = ok ? qMin(n, KS_colMax - columnIndex + 1) : 1;
        columnMaximal = qMax(numberColumns, columnMaximal);

        // Styles are inserted at the end of the loading process, so check the XML directly here.
        const QString styleName = cellElement.attributeNS(KOdfXmlNS::table , sStyleName, QString());
        if (!styleName.isEmpty())
            cellStyleRegions[styleName] += QRect(columnIndex, rowIndex, numberColumns, number);

        // figure out exact cell style for loading of cell content
        QString cellStyleName = styleName;
        if (cellStyleName.isEmpty())
            cellStyleName = rowCellStyleName;
        if (cellStyleName.isEmpty())
            cellStyleName = columnStyles.get(columnIndex);

        KCCell cell(this, columnIndex, rowIndex);
        cell.loadOdf(cellElement, tableContext, autoStyles, cellStyleName);

        if (!cell.comment().isEmpty())
            cellStorage()->setComment(KCRegion(columnIndex, rowIndex, numberColumns, number, this), cell.comment());
        if (!cell.conditions().isEmpty())
            cellStorage()->setConditions(KCRegion(columnIndex, rowIndex, numberColumns, number, this), cell.conditions());
        if (!cell.validity().isEmpty())
            cellStorage()->setValidity(KCRegion(columnIndex, rowIndex, numberColumns, number, this), cell.validity());

        if (!cell.hasDefaultContent()) {
            // Row-wise filling of PointStorages is faster than column-wise filling.
            QSharedPointer<QTextDocument> richText = cell.richText();
            for (int r = rowIndex; r <= endRow; ++r) {
                for (int c = 0; c < numberColumns; ++c) {
                    KCCell target(this, columnIndex + c, r);
                    target.setFormula(cell.formula());
                    target.setUserInput(cell.userInput());
                    target.setRichText(richText);
                    target.setValue(cell.value());
                    if (cell.doesMergeCells()) {
                        target.mergeCells(columnIndex + c, r, cell.mergedXCells(), cell.mergedYCells());
                    }
                }
            }
        }
        columnIndex += numberColumns;
    }

    cellStorage()->setRowsRepeated(rowIndex, number);

    rowIndex += number;
    return columnMaximal;
}

QRect KCSheet::usedArea(bool onlyContent) const
{
    int maxCols = d->cellStorage->columns(!onlyContent);
    int maxRows = d->cellStorage->rows(!onlyContent);

    if (!onlyContent) {
        const KCRowFormat * row = firstRow();
        while (row) {
            if (row->row() > maxRows)
                maxRows = row->row();

            row = row->next();
        }

        const KCColumnFormat* col = firstCol();
        while (col) {
            if (col->column() > maxCols)
                maxCols = col->column();

            col = col->next();
        }
    }

    // flake
    QRectF shapesBoundingRect;
    for (int i = 0; i < d->shapes.count(); ++i)
        shapesBoundingRect |= d->shapes[i]->boundingRect();
    const QRect shapesCellRange = documentToCellCoordinates(shapesBoundingRect);
    maxCols = qMax(maxCols, shapesCellRange.right());
    maxRows = qMax(maxRows, shapesCellRange.bottom());

    return QRect(1, 1, maxCols, maxRows);
}

bool KCSheet::compareRows(int row1, int row2, int& maxCols, KCOdfSavingContext& tableContext) const
{
    if (*rowFormat(row1) != *rowFormat(row2)) {
//         kDebug(36003) <<"\t Formats of" << row1 <<" and" << row2 <<" are different";
        return false;
    }
    if (tableContext.rowHasCellAnchoredShapes(this, row1) != tableContext.rowHasCellAnchoredShapes(this, row2)) {
        return false;
    }
    KCCell cell1 = cellStorage()->firstInRow(row1);
    KCCell cell2 = cellStorage()->firstInRow(row2);
    if (cell1.isNull() != cell2.isNull())
        return false;
    while (!cell1.isNull()) {
        if (cell1.column() != cell2.column())
            return false;
        if (cell1.column() > maxCols)
            break;
        if (!cell1.compareData(cell2)) {
//             kDebug(36003) <<"\t KCCell at column" << col <<" in row" << row2 <<" differs from the one in row" << row1;
            return false;
        }
        cell1 = cellStorage()->nextInRow(cell1.column(), cell1.row());
        cell2 = cellStorage()->nextInRow(cell2.column(), cell2.row());
        if (cell1.isNull() != cell2.isNull())
            return false;
    }
    return true;
}

void KCSheet::saveOdfHeaderFooter(KXmlWriter &xmlWriter) const
{
    QString headerLeft = print()->headerFooter()->headLeft();
    QString headerCenter = print()->headerFooter()->headMid();
    QString headerRight = print()->headerFooter()->headRight();

    QString footerLeft = print()->headerFooter()->footLeft();
    QString footerCenter = print()->headerFooter()->footMid();
    QString footerRight = print()->headerFooter()->footRight();

    xmlWriter.startElement("style:header");
    if ((!headerLeft.isEmpty())
            || (!headerCenter.isEmpty())
            || (!headerRight.isEmpty())) {
        xmlWriter.startElement("style:region-left");
        xmlWriter.startElement("text:p");
        convertPart(headerLeft, xmlWriter);
        xmlWriter.endElement();
        xmlWriter.endElement();

        xmlWriter.startElement("style:region-center");
        xmlWriter.startElement("text:p");
        convertPart(headerCenter, xmlWriter);
        xmlWriter.endElement();
        xmlWriter.endElement();

        xmlWriter.startElement("style:region-right");
        xmlWriter.startElement("text:p");
        convertPart(headerRight, xmlWriter);
        xmlWriter.endElement();
        xmlWriter.endElement();
    } else {
        xmlWriter.startElement("text:p");

        xmlWriter.startElement("text:sheet-name");
        xmlWriter.addTextNode("???");
        xmlWriter.endElement();

        xmlWriter.endElement();
    }
    xmlWriter.endElement();


    xmlWriter.startElement("style:footer");
    if ((!footerLeft.isEmpty())
            || (!footerCenter.isEmpty())
            || (!footerRight.isEmpty())) {
        xmlWriter.startElement("style:region-left");
        xmlWriter.startElement("text:p");
        convertPart(footerLeft, xmlWriter);
        xmlWriter.endElement();
        xmlWriter.endElement(); //style:region-left

        xmlWriter.startElement("style:region-center");
        xmlWriter.startElement("text:p");
        convertPart(footerCenter, xmlWriter);
        xmlWriter.endElement();
        xmlWriter.endElement();

        xmlWriter.startElement("style:region-right");
        xmlWriter.startElement("text:p");
        convertPart(footerRight, xmlWriter);
        xmlWriter.endElement();
        xmlWriter.endElement();
    } else {
        xmlWriter.startElement("text:p");

        xmlWriter.startElement("text:sheet-name");
        xmlWriter.addTextNode("Page ");   // ???
        xmlWriter.endElement();

        xmlWriter.startElement("text:page-number");
        xmlWriter.addTextNode("1");   // ???
        xmlWriter.endElement();

        xmlWriter.endElement();
    }
    xmlWriter.endElement();


}

void KCSheet::addText(const QString & text, KXmlWriter & writer) const
{
    if (!text.isEmpty())
        writer.addTextNode(text);
}

void KCSheet::convertPart(const QString & part, KXmlWriter & xmlWriter) const
{
    QString text;
    QString var;

    bool inVar = false;
    uint i = 0;
    uint l = part.length();
    while (i < l) {
        if (inVar || part[i] == '<') {
            inVar = true;
            var += part[i];
            if (part[i] == '>') {
                inVar = false;
                if (var == "<page>") {
                    addText(text, xmlWriter);
                    xmlWriter.startElement("text:page-number");
                    xmlWriter.addTextNode("1");
                    xmlWriter.endElement();
                } else if (var == "<pages>") {
                    addText(text, xmlWriter);
                    xmlWriter.startElement("text:page-count");
                    xmlWriter.addTextNode("99");   //TODO I think that it can be different from 99
                    xmlWriter.endElement();
                } else if (var == "<date>") {
                    addText(text, xmlWriter);
                    //text:p><text:date style:data-style-name="N2" text:date-value="2005-10-02">02/10/2005</text:date>, <text:time>10:20:12</text:time></text:p> "add style" => create new style
#if 0 //FIXME
                    KXmlElement t = dd.createElement("text:date");
                    t.setAttribute("text:date-value", "0-00-00");
                    // todo: "style:data-style-name", "N2"
                    t.appendChild(dd.createTextNode(QDate::currentDate().toString()));
                    parent.appendChild(t);
#endif
                } else if (var == "<time>") {
                    addText(text, xmlWriter);

                    xmlWriter.startElement("text:time");
                    xmlWriter.addTextNode(QTime::currentTime().toString());
                    xmlWriter.endElement();
                } else if (var == "<file>") { // filepath + name
                    addText(text, xmlWriter);
                    xmlWriter.startElement("text:file-name");
                    xmlWriter.addAttribute("text:display", "full");
                    xmlWriter.addTextNode("???");
                    xmlWriter.endElement();
                } else if (var == "<name>") { // filename
                    addText(text, xmlWriter);

                    xmlWriter.startElement("text:title");
                    xmlWriter.addTextNode("???");
                    xmlWriter.endElement();
                } else if (var == "<author>") {
                    KCDocBase* sdoc = doc();
                    KoDocumentInfo* docInfo = sdoc->documentInfo();

                    text += docInfo->authorInfo("creator");
                    addText(text, xmlWriter);
                } else if (var == "<email>") {
                    KCDocBase* sdoc = doc();
                    KoDocumentInfo* docInfo = sdoc->documentInfo();

                    text += docInfo->authorInfo("email");
                    addText(text, xmlWriter);

                } else if (var == "<org>") {
                    KCDocBase* sdoc = doc();
                    KoDocumentInfo* docInfo    = sdoc->documentInfo();

                    text += docInfo->authorInfo("company");
                    addText(text, xmlWriter);

                } else if (var == "<sheet>") {
                    addText(text, xmlWriter);

                    xmlWriter.startElement("text:sheet-name");
                    xmlWriter.addTextNode("???");
                    xmlWriter.endElement();
                } else {
                    // no known variable:
                    text += var;
                    addText(text, xmlWriter);
                }

                text = "";
                var  = "";
            }
        } else {
            text += part[i];
        }
        ++i;
    }
    if (!text.isEmpty() || !var.isEmpty()) {
        //we don't have var at the end =>store it
        addText(text + var, xmlWriter);
    }
    kDebug(36003) << " text end :" << text << " var :" << var;
}

void KCSheet::saveOdfBackgroundImage(KXmlWriter& xmlWriter) const
{
    const BackgroundImageProperties& properties = backgroundImageProperties();
    xmlWriter.startElement("style:backgroundImage");

    //xmlWriter.addAttribute("xlink:href", fileName);
    xmlWriter.addAttribute("xlink:type", "simple");
    xmlWriter.addAttribute("xlink:show", "embed");
    xmlWriter.addAttribute("xlink:actuate", "onLoad");

    QString opacity = QString("%1%").arg(properties.opacity);
    xmlWriter.addAttribute("draw:opacity", opacity);

    QString position;
    if(properties.horizontalPosition == BackgroundImageProperties::Left) {
        position += "left";
    }
    else if(properties.horizontalPosition == BackgroundImageProperties::HorizontalCenter) {
        position += "center";
    }
    else if(properties.horizontalPosition == BackgroundImageProperties::Right) {
        position += "right";
    }

    position += ' ';

    if(properties.verticalPosition == BackgroundImageProperties::Top) {
        position += "top";
    }
    else if(properties.verticalPosition == BackgroundImageProperties::VerticalCenter) {
        position += "center";
    }
    else if(properties.verticalPosition == BackgroundImageProperties::Bottom) {
        position += "right";
    }
    xmlWriter.addAttribute("style:position", position);

    QString repeat;
    if(properties.repeat == BackgroundImageProperties::NoRepeat) {
        repeat = "no-repeat";
    }
    else if(properties.repeat == BackgroundImageProperties::Repeat) {
        repeat = "repeat";
    }
    else if(properties.repeat == BackgroundImageProperties::Stretch) {
        repeat = "stretch";
    }
    xmlWriter.addAttribute("style:repeat", repeat);

    xmlWriter.endElement();
}


void KCSheet::loadOdfSettings(const KOdfSettings::NamedMap &settings)
{
    // Find the entry in the map that applies to this sheet (by name)
    KOdfSettings::Items items = settings.entry(sheetName());
    if (items.isNull())
        return;
    setHideZero(!items.parseConfigItemBool("ShowZeroValues"));
    setShowGrid(items.parseConfigItemBool("ShowGrid"));
    setFirstLetterUpper(items.parseConfigItemBool("FirstLetterUpper"));

    int cursorX = qMin(KS_colMax, qMax(1, items.parseConfigItemInt("CursorPositionX") + 1));
    int cursorY = qMin(KS_rowMax, qMax(1, items.parseConfigItemInt("CursorPositionY") + 1));
    map()->loadingInfo()->setCursorPosition(this, QPoint(cursorX, cursorY));

    double offsetX = items.parseConfigItemDouble("xOffset");
    double offsetY = items.parseConfigItemDouble("yOffset");
    map()->loadingInfo()->setScrollingOffset(this, QPointF(offsetX, offsetY));

    setShowFormulaIndicator(items.parseConfigItemBool("ShowFormulaIndicator"));
    setShowCommentIndicator(items.parseConfigItemBool("ShowCommentIndicator"));
    setShowPageBorders(items.parseConfigItemBool("ShowPageBorders"));
    setLcMode(items.parseConfigItemBool("lcmode"));
    setAutoCalculationEnabled(items.parseConfigItemBool("autoCalc"));
    setShowColumnNumber(items.parseConfigItemBool("ShowColumnNumber"));
}

void KCSheet::saveOdfSettings(KXmlWriter &settingsWriter) const
{
    //not into each page into oo spec
    settingsWriter.addConfigItem("ShowZeroValues", !getHideZero());
    settingsWriter.addConfigItem("ShowGrid", getShowGrid());
    //not define into oo spec
    settingsWriter.addConfigItem("FirstLetterUpper", getFirstLetterUpper());
    settingsWriter.addConfigItem("ShowFormulaIndicator", getShowFormulaIndicator());
    settingsWriter.addConfigItem("ShowCommentIndicator", getShowCommentIndicator());
    settingsWriter.addConfigItem("ShowPageBorders", isShowPageBorders());
    settingsWriter.addConfigItem("lcmode", getLcMode());
    settingsWriter.addConfigItem("autoCalc", isAutoCalculationEnabled());
    settingsWriter.addConfigItem("ShowColumnNumber", getShowColumnNumber());
}

bool KCSheet::saveOdf(KCOdfSavingContext& tableContext)
{
    KXmlWriter & xmlWriter = tableContext.shapeContext.xmlWriter();
    KOdfGenericStyles & mainStyles = tableContext.shapeContext.mainStyles();
    xmlWriter.startElement("table:table");
    xmlWriter.addAttribute("table:name", sheetName());
    xmlWriter.addAttribute("table:style-name", saveOdfSheetStyleName(mainStyles));
    QByteArray pwd;
    password(pwd);
    if (!pwd.isNull()) {
        xmlWriter.addAttribute("table:protected", "true");
        QByteArray str = KCodecs::base64Encode(pwd);
        // FIXME Stefan: see OpenDocument spec, ch. 17.3 Encryption
        xmlWriter.addAttribute("table:protection-key", QString(str));
    }
    QRect _printRange = printSettings()->printRegion().lastRange();
    if (_printRange != (QRect(QPoint(1, 1), QPoint(KS_colMax, KS_rowMax)))) {
        const KCRegion region(_printRange, this);
        if (region.isValid()) {
            kDebug(36003) << region;
            xmlWriter.addAttribute("table:print-ranges", region.saveOdf());
        }
    }

    // flake
    // Create a dict of cell anchored shapes with the cell as key.
    foreach(KShape* shape, d->shapes) {
        if (dynamic_cast<KCShapeApplicationData*>(shape->applicationData())->isAnchoredToCell()) {
            double dummy;
            const QPointF position = shape->position();
            const int col = leftColumn(position.x(), dummy);
            const int row = topRow(position.y(), dummy);
            tableContext.insertCellAnchoredShape(this, row, col, shape);
        }
    }

    const QRect usedArea = this->usedArea();
    saveOdfColRowCell(xmlWriter, mainStyles, usedArea.width(), usedArea.height(), tableContext);

    // flake
    // Save the remaining shapes, those that are anchored in the page.
    if (!d->shapes.isEmpty()) {
        xmlWriter.startElement("table:shapes");
        foreach(KShape* shape, d->shapes) {
            if (dynamic_cast<KCShapeApplicationData*>(shape->applicationData())->isAnchoredToCell())
                continue;
            shape->saveOdf(tableContext.shapeContext);
        }
        xmlWriter.endElement();
    }

    xmlWriter.endElement();
    return true;
}

QString KCSheet::saveOdfSheetStyleName(KOdfGenericStyles &mainStyles)
{
    KOdfGenericStyle pageStyle(KOdfGenericStyle::TableAutoStyle, "table"/*FIXME I don't know if name is sheet*/);

    KOdfGenericStyle pageMaster(KOdfGenericStyle::MasterPageStyle);
    const QString pageLayoutName = printSettings()->saveOdfPageLayout(mainStyles,
                                   getShowFormula(),
                                   !getHideZero());
    pageMaster.addAttribute("style:page-layout-name", pageLayoutName);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    saveOdfHeaderFooter(elementWriter);

    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    pageMaster.addChildElement("headerfooter", elementContents);
    pageStyle.addAttribute("style:master-page-name", mainStyles.insert(pageMaster, "Standard"));

    pageStyle.addProperty("table:display", !isHidden());

    if( !backgroundImage().isNull() ) {
        QBuffer bgBuffer;
        bgBuffer.open(QIODevice::WriteOnly);
        KXmlWriter bgWriter(&bgBuffer); //TODO pass identation level
        saveOdfBackgroundImage(bgWriter);

        const QString bgContent = QString::fromUtf8(bgBuffer.buffer(), bgBuffer.size());
        pageMaster.addChildElement("backgroundImage", bgContent);
    }

    return mainStyles.insert(pageStyle, "ta");
}


void KCSheet::saveOdfColRowCell(KXmlWriter& xmlWriter, KOdfGenericStyles &mainStyles,
                              int maxCols, int maxRows, KCOdfSavingContext& tableContext)
{
    kDebug(36003) << "KCSheet::saveOdfColRowCell:" << d->name;

    // calculate the column/row default cell styles
    int maxMaxRows = maxRows; // includes the max row a column default style occupies
    // also extends the maximum column/row to include column/row styles
    styleStorage()->saveOdfCreateDefaultStyles(maxCols, maxMaxRows, tableContext);
    if (tableContext.rowDefaultStyles.count() != 0)
        maxRows = qMax(maxRows, (--tableContext.rowDefaultStyles.constEnd()).key());
    // OpenDocument needs at least one cell per sheet.
    maxCols = qMin(KS_colMax, qMax(1, maxCols));
    maxRows = qMin(KS_rowMax, qMax(1, maxRows));
    maxMaxRows = maxMaxRows;
    kDebug(36003) << "\t KCSheet dimension:" << maxCols << " x" << maxRows;

    // saving the columns
    //
    int i = 1;
    while (i <= maxCols) {
        const KCColumnFormat* column = columnFormat(i);
//         kDebug(36003) << "KCSheet::saveOdfColRowCell: first col loop:"
//                       << "i:" << i
//                       << "column:" << (column ? column->column() : 0)
//                       << "default:" << (column ? column->isDefault() : false);

        //style default layout for column
        const KCStyle style = tableContext.columnDefaultStyles.value(i);

        int j = i;
        int count = 1;

        while (j <= maxCols) {
            const KCColumnFormat* nextColumn = d->columns.next(j);
            const int nextColumnIndex = nextColumn ? nextColumn->column() : 0;
            const QMap<int, KCStyle>::iterator nextColumnDefaultStyle = tableContext.columnDefaultStyles.upperBound(j);
            const int nextStyleColumnIndex = nextColumnDefaultStyle == tableContext.columnDefaultStyles.end()
                                             ? 0 : nextColumnDefaultStyle.key();
            // j becomes the index of the adjacent column
            ++j;

//           kDebug(36003) <<"KCSheet::saveOdfColRowCell: second col loop:"
//                         << "j:" << j
//                         << "next column:" << (nextColumn ? nextColumn->column() : 0)
//                         << "next styled column:" << nextStyleColumnIndex;

            // no next or not the adjacent column?
            if ((!nextColumn && !nextStyleColumnIndex) ||
                    (nextColumnIndex != j && nextStyleColumnIndex != j)) {
                // if the origin column was a default column,
                if (column->isDefault() && style.isDefault()) {
                    // we count the default columns
                    if (!nextColumn && !nextStyleColumnIndex)
                        count = maxCols - i + 1;
                    else if (nextColumn && (!nextStyleColumnIndex || nextColumn->column() <= nextStyleColumnIndex))
                        count = nextColumn->column() - i;
                    else
                        count = nextStyleColumnIndex - i;
                }
                // otherwise we just stop here to process the adjacent
                // column in the next iteration of the outer loop
                break;
            }

            // stop, if the next column differs from the current one
            if ((nextColumn && (*column != *nextColumn)) || (!nextColumn && !column->isDefault()))
                break;
            if (style != tableContext.columnDefaultStyles.value(j))
                break;
            ++count;
        }

        xmlWriter.startElement("table:table-column");
        if (!column->isDefault()) {
            KOdfGenericStyle currentColumnStyle(KOdfGenericStyle::TableColumnAutoStyle, "table-column");
            currentColumnStyle.addPropertyPt("style:column-width", column->width());
            if (column->hasPageBreak()) {
                currentColumnStyle.addProperty("fo:break-before", "page");
            }
            xmlWriter.addAttribute("table:style-name", mainStyles.insert(currentColumnStyle, "co"));
        }
        if (!column->isDefault() || !style.isDefault()) {
            if (!style.isDefault()) {
                KOdfGenericStyle currentDefaultCellStyle; // the type is determined in saveOdfStyle
                const QString name = style.saveOdf(currentDefaultCellStyle, mainStyles,
                                                   map()->styleManager());
                xmlWriter.addAttribute("table:default-cell-style-name", name);
            }

            if (column->isHidden())
                xmlWriter.addAttribute("table:visibility", "collapse");
            else if (column->isFiltered())
                xmlWriter.addAttribute("table:visibility", "filter");
        }
        if (count > 1)
            xmlWriter.addAttribute("table:number-columns-repeated", count);
        xmlWriter.endElement();

        kDebug(36003) << "KCSheet::saveOdfColRowCell: column" << i
        << "repeated" << count - 1 << "time(s)";

        i += count;
    }

    // saving the rows and the cells
    // we have to loop through all rows of the used area
    for (i = 1; i <= maxRows; ++i) {
        const KCRowFormat* row = rowFormat(i);

        // default cell style for row
        const KCStyle style = tableContext.rowDefaultStyles.value(i);

        xmlWriter.startElement("table:table-row");

        if (!row->isDefault()) {
            KOdfGenericStyle currentRowStyle(KOdfGenericStyle::TableRowAutoStyle, "table-row");
            currentRowStyle.addPropertyPt("style:row-height", row->height());
            if (row->hasPageBreak()) {
                currentRowStyle.addProperty("fo:break-before", "page");
            }
            xmlWriter.addAttribute("table:style-name", mainStyles.insert(currentRowStyle, "ro"));
        }

        int repeated = cellStorage()->rowRepeat(i);
        // empty row?
        if (!d->cellStorage->firstInRow(i) && !tableContext.rowHasCellAnchoredShapes(this, i)) { // row is empty
//             kDebug(36003) <<"KCSheet::saveOdfColRowCell: first row loop:"
//                           << " i: " << i
//                           << " row: " << row->row();
            int j = i + repeated;

            // search for
            //   next non-empty row
            // or
            //   next row with different KCFormat
            while (j <= maxRows && !d->cellStorage->firstInRow(j) && !tableContext.rowHasCellAnchoredShapes(this, j)) {
                const KCRowFormat* nextRow = rowFormat(j);
//               kDebug(36003) <<"KCSheet::saveOdfColRowCell: second row loop:"
//                         << " j: " << j
//                         << " row: " << nextRow->row();

                // if the reference row has the default row format
                if (row->isDefault() && style.isDefault()) {
                    // if the next is not default, stop here
                    if (!nextRow->isDefault() || !tableContext.rowDefaultStyles.value(j).isDefault())
                        break;
                    // otherwise, jump to the next
                    j += cellStorage()->rowRepeat(j);
                    continue;
                }

                // stop, if the next row differs from the current one
                if ((nextRow && *row != *nextRow) || (!nextRow && !row->isDefault()))
                    break;
                if (style != tableContext.rowDefaultStyles.value(j))
                    break;
                // otherwise, process the next
                j += cellStorage()->rowRepeat(j);
            }
            repeated = j - i;

            if (repeated > 1)
                xmlWriter.addAttribute("table:number-rows-repeated", repeated);
            if (!style.isDefault()) {
                KOdfGenericStyle currentDefaultCellStyle; // the type is determined in saveOdfCellStyle
                const QString name = style.saveOdf(currentDefaultCellStyle, mainStyles,
                                                   map()->styleManager());
                xmlWriter.addAttribute("table:default-cell-style-name", name);
            }
            if (row->isHidden())   // never true for the default row
                xmlWriter.addAttribute("table:visibility", "collapse");
            else if (row->isFiltered()) // never true for the default row
                xmlWriter.addAttribute("table:visibility", "filter");

            // NOTE Stefan: Even if paragraph 8.1 states, that rows may be empty, the
            //              RelaxNG schema does not allow that.
            xmlWriter.startElement("table:table-cell");
            // Fill the row with empty cells, if there's a row default cell style.
            if (!style.isDefault())
                xmlWriter.addAttribute("table:number-columns-repeated", QString::number(maxCols));
            // Fill the row with empty cells up to the last column with a default cell style.
            else if (!tableContext.columnDefaultStyles.isEmpty()) {
                const int col = (--tableContext.columnDefaultStyles.constEnd()).key();
                xmlWriter.addAttribute("table:number-columns-repeated", QString::number(col));
            }
            xmlWriter.endElement();

            kDebug(36003) << "KCSheet::saveOdfColRowCell: empty row" << i
            << "repeated" << repeated << "time(s)";

            // copy the index for the next row to process
            i = j - 1; /*it's already incremented in the for loop*/
        } else { // row is not empty
            if (!style.isDefault()) {
                KOdfGenericStyle currentDefaultCellStyle; // the type is determined in saveOdfCellStyle
                const QString name = style.saveOdf(currentDefaultCellStyle, mainStyles,
                                                   map()->styleManager());
                xmlWriter.addAttribute("table:default-cell-style-name", name);
            }
            if (row->isHidden())   // never true for the default row
                xmlWriter.addAttribute("table:visibility", "collapse");
            else if (row->isFiltered()) // never true for the default row
                xmlWriter.addAttribute("table:visibility", "filter");

            int j = i + repeated;
            while (j <= maxRows && compareRows(i, j, maxCols, tableContext)) {
                j += cellStorage()->rowRepeat(j);
            }
            repeated = j - i;
            if (repeated > 1) {
                kDebug(36003) << "KCSheet::saveOdfColRowCell: NON-empty row" << i
                << "repeated" << repeated << "times";

                xmlWriter.addAttribute("table:number-rows-repeated", repeated);
            }

            saveOdfCells(xmlWriter, mainStyles, i, maxCols, tableContext);

            // copy the index for the next row to process
            i = j - 1; /*it's already incremented in the for loop*/
        }
        xmlWriter.endElement();
    }

    // Fill in rows with empty cells, if there's a column default cell style.
    if (!tableContext.columnDefaultStyles.isEmpty()) {
        if (maxMaxRows > maxRows) {
            xmlWriter.startElement("table:table-row");
            if (maxMaxRows > maxRows + 1)
                xmlWriter.addAttribute("table:number-rows-repeated", maxMaxRows - maxRows);
            xmlWriter.startElement("table:table-cell");
            const int col = qMin(maxCols, (--tableContext.columnDefaultStyles.constEnd()).key());
            xmlWriter.addAttribute("table:number-columns-repeated", QString::number(col));
            xmlWriter.endElement();
            xmlWriter.endElement();
        }
    }
}

void KCSheet::saveOdfCells(KXmlWriter& xmlWriter, KOdfGenericStyles &mainStyles, int row, int maxCols,
                         KCOdfSavingContext& tableContext)
{
    int i = 1;
    KCCell cell(this, i, row);
    KCCell nextCell = d->cellStorage->nextInRow(i, row);
    // handle situations where the row contains shapes and nothing else
    if (cell.isDefault() && nextCell.isNull()) {
        int nextShape = tableContext.nextAnchoredShape(this, row, i);
        if (nextShape)
            nextCell = KCCell(this, nextShape, row);
    }
    // while
    //   the current cell is not a default one
    // or
    //   we have a further cell in this row
    while (!cell.isDefault() || tableContext.cellHasAnchoredShapes(this, cell.row(), cell.column()) || !nextCell.isNull()) {
//         kDebug(36003) <<"KCSheet::saveOdfCells:"
//                       << " i: " << i
//                       << " column: " << cell.column() << endl;

        int repeated = 1;
        int column = i;
        cell.saveOdf(xmlWriter, mainStyles, row, column, repeated, tableContext);
        i += repeated;
        // stop if we reached the end column
        if (i > maxCols || nextCell.isNull())
            break;

        cell = KCCell(this, i, row);
        // if we have a shape anchored to an empty cell, ensure that the cell gets also processed
        int nextShape = tableContext.nextAnchoredShape(this, row, column);
        if (nextShape && ((nextShape < i) || cell.isDefault())) {
            cell = KCCell(this, nextShape, row);
            i = nextShape;
        }

        nextCell = d->cellStorage->nextInRow(i, row);
    }

    // Fill the row with empty cells, if there's a row default cell style.
    if (tableContext.rowDefaultStyles.contains(row)) {
        if (maxCols >= i) {
            xmlWriter.startElement("table:table-cell");
            if (maxCols > i)
                xmlWriter.addAttribute("table:number-columns-repeated", QString::number(maxCols - i + 1));
            xmlWriter.endElement();
        }
    }
    // Fill the row with empty cells up to the last column with a default cell style.
    else if (!tableContext.columnDefaultStyles.isEmpty()) {
        const int col = (--tableContext.columnDefaultStyles.constEnd()).key();
        if (col >= i) {
            xmlWriter.startElement("table:table-cell");
            if (col > i)
                xmlWriter.addAttribute("table:number-columns-repeated", QString::number(col - i + 1));
            xmlWriter.endElement();
        }
    }
}

bool KCSheet::loadXML(const KXmlElement& sheet)
{
    bool ok = false;
    QString sname = sheetName();
    if (!map()->loadingInfo()->loadTemplate()) {
        sname = sheet.attribute("name");
        if (sname.isEmpty()) {
            doc()->setErrorMessage(i18n("Invalid document. Sheet name is empty."));
            return false;
        }
    }

    bool detectDirection = true;
    QString layoutDir = sheet.attribute("layoutDirection");
    if (!layoutDir.isEmpty()) {
        if (layoutDir == "rtl") {
            detectDirection = false;
            setLayoutDirection(Qt::RightToLeft);
        } else if (layoutDir == "ltr") {
            detectDirection = false;
            setLayoutDirection(Qt::LeftToRight);
        } else
            kDebug() << " Direction not implemented :" << layoutDir;
    }
    if (detectDirection)
        checkContentDirection(sname);

    /* older versions of KCells allowed all sorts of characters that
    the parser won't actually understand.  Replace these with '_'
    Also, the initial character cannot be a space.
    */
    while (sname[0] == ' ') {
        sname.remove(0, 1);
    }
    for (int i = 0; i < sname.length(); i++) {
        if (!(sname[i].isLetterOrNumber() ||
                sname[i] == ' ' || sname[i] == '.' || sname[i] == '_')) {
            sname[i] = '_';
        }
    }

    // validate sheet name, if it differs from the current one
    if (sname != sheetName()) {
        /* make sure there are no name collisions with the altered name */
        QString testName = sname;
        QString baseName = sname;
        int nameSuffix = 0;

        /* so we don't panic over finding ourself in the following test*/
        sname.clear();
        while (map()->findSheet(testName) != 0) {
            nameSuffix++;
            testName = baseName + '_' + QString::number(nameSuffix);
        }
        sname = testName;

        kDebug(36001) << "KCSheet::loadXML: table name =" << sname;
        setObjectName(sname.toUtf8());
        setSheetName(sname, true);
    }

//     (dynamic_cast<SheetIface*>(dcopObject()))->sheetNameHasChanged();

    if (sheet.hasAttribute("grid")) {
        setShowGrid((int)sheet.attribute("grid").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    if (sheet.hasAttribute("printGrid")) {
        print()->settings()->setPrintGrid((bool)sheet.attribute("printGrid").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    if (sheet.hasAttribute("printCommentIndicator")) {
        print()->settings()->setPrintCommentIndicator((bool)sheet.attribute("printCommentIndicator").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    if (sheet.hasAttribute("printFormulaIndicator")) {
        print()->settings()->setPrintFormulaIndicator((bool)sheet.attribute("printFormulaIndicator").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    if (sheet.hasAttribute("hide")) {
        setHidden((bool)sheet.attribute("hide").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    if (sheet.hasAttribute("showFormula")) {
        setShowFormula((bool)sheet.attribute("showFormula").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    //Compatibility with KCells 1.1.x
    if (sheet.hasAttribute("formular")) {
        setShowFormula((bool)sheet.attribute("formular").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    if (sheet.hasAttribute("showFormulaIndicator")) {
        setShowFormulaIndicator((bool)sheet.attribute("showFormulaIndicator").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    if (sheet.hasAttribute("showCommentIndicator")) {
        setShowCommentIndicator((bool)sheet.attribute("showCommentIndicator").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    if (sheet.hasAttribute("borders")) {
        setShowPageBorders((bool)sheet.attribute("borders").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    if (sheet.hasAttribute("lcmode")) {
        setLcMode((bool)sheet.attribute("lcmode").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    if (sheet.hasAttribute("autoCalc")) {
        setAutoCalculationEnabled((bool)sheet.attribute("autoCalc").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    if (sheet.hasAttribute("columnnumber")) {
        setShowColumnNumber((bool)sheet.attribute("columnnumber").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    if (sheet.hasAttribute("hidezero")) {
        setHideZero((bool)sheet.attribute("hidezero").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }
    if (sheet.hasAttribute("firstletterupper")) {
        setFirstLetterUpper((bool)sheet.attribute("firstletterupper").toInt(&ok));
        // we just ignore 'ok' - if it didn't work, go on
    }

    // Load the paper layout
    KXmlElement paper = sheet.namedItem("paper").toElement();
    if (!paper.isNull()) {
        KOdfPageLayoutData pageLayout;
        pageLayout.format = KOdfPageFormat::formatFromString(paper.attribute("format"));
        pageLayout.orientation = (paper.attribute("orientation")  == "Portrait")
                                 ? KOdfPageFormat::Portrait : KOdfPageFormat::Landscape;

        // <borders>
        KXmlElement borders = paper.namedItem("borders").toElement();
        if (!borders.isNull()) {
            pageLayout.leftMargin   = MM_TO_POINT(borders.attribute("left").toFloat());
            pageLayout.rightMargin  = MM_TO_POINT(borders.attribute("right").toFloat());
            pageLayout.topMargin    = MM_TO_POINT(borders.attribute("top").toFloat());
            pageLayout.bottomMargin = MM_TO_POINT(borders.attribute("bottom").toFloat());
        }
        print()->settings()->setPageLayout(pageLayout);

        QString hleft, hright, hcenter;
        QString fleft, fright, fcenter;
        // <head>
        KXmlElement head = paper.namedItem("head").toElement();
        if (!head.isNull()) {
            KXmlElement left = head.namedItem("left").toElement();
            if (!left.isNull())
                hleft = left.text();
            KXmlElement center = head.namedItem("center").toElement();
            if (!center.isNull())
                hcenter = center.text();
            KXmlElement right = head.namedItem("right").toElement();
            if (!right.isNull())
                hright = right.text();
        }
        // <foot>
        KXmlElement foot = paper.namedItem("foot").toElement();
        if (!foot.isNull()) {
            KXmlElement left = foot.namedItem("left").toElement();
            if (!left.isNull())
                fleft = left.text();
            KXmlElement center = foot.namedItem("center").toElement();
            if (!center.isNull())
                fcenter = center.text();
            KXmlElement right = foot.namedItem("right").toElement();
            if (!right.isNull())
                fright = right.text();
        }
        print()->headerFooter()->setHeadFootLine(hleft, hcenter, hright, fleft, fcenter, fright);
    }

    // load print range
    KXmlElement printrange = sheet.namedItem("printrange-rect").toElement();
    if (!printrange.isNull()) {
        int left = printrange.attribute("left-rect").toInt();
        int right = printrange.attribute("right-rect").toInt();
        int bottom = printrange.attribute("bottom-rect").toInt();
        int top = printrange.attribute("top-rect").toInt();
        if (left == 0) { //whole row(s) selected
            left = 1;
            right = KS_colMax;
        }
        if (top == 0) { //whole column(s) selected
            top = 1;
            bottom = KS_rowMax;
        }
        const KCRegion region(QRect(QPoint(left, top), QPoint(right, bottom)), this);
        printSettings()->setPrintRegion(region);
    }

    // load print zoom
    if (sheet.hasAttribute("printZoom")) {
        double zoom = sheet.attribute("printZoom").toDouble(&ok);
        if (ok) {
            printSettings()->setZoom(zoom);
        }
    }

    // load page limits
    if (sheet.hasAttribute("printPageLimitX")) {
        int pageLimit = sheet.attribute("printPageLimitX").toInt(&ok);
        if (ok) {
            printSettings()->setPageLimits(QSize(pageLimit, 0));
        }
    }

    // load page limits
    if (sheet.hasAttribute("printPageLimitY")) {
        int pageLimit = sheet.attribute("printPageLimitY").toInt(&ok);
        if (ok) {
            const int horizontalLimit = printSettings()->pageLimits().width();
            printSettings()->setPageLimits(QSize(horizontalLimit, pageLimit));
        }
    }

    // Load the cells
    KXmlNode n = sheet.firstChild();
    while (!n.isNull()) {
        KXmlElement e = n.toElement();
        if (!e.isNull()) {
            QString tagName = e.tagName();
            if (tagName == "cell")
                KCCell(this, 1, 1).load(e, 0, 0); // col, row will get overridden in all cases
            else if (tagName == "row") {
                KCRowFormat *rl = new KCRowFormat();
                rl->setSheet(this);
                if (rl->load(e))
                    insertRowFormat(rl);
                else
                    delete rl;
            } else if (tagName == "column") {
                KCColumnFormat *cl = new KCColumnFormat();
                cl->setSheet(this);
                if (cl->load(e))
                    insertColumnFormat(cl);
                else
                    delete cl;
            }
#if 0 // KCELLS_KOPART_EMBEDDING
            else if (tagName == "object") {
                EmbeddedKOfficeObject *ch = new EmbeddedKOfficeObject(doc(), this);
                if (ch->load(e))
                    insertObject(ch);
                else {
                    ch->embeddedObject()->setDeleted(true);
                    delete ch;
                }
            } else if (tagName == "chart") {
                EmbeddedChart *ch = new EmbeddedChart(doc(), this);
                if (ch->load(e))
                    insertObject(ch);
                else {
                    ch->embeddedObject()->setDeleted(true);
                    delete ch;
                }
            }
#endif // KCELLS_KOPART_EMBEDDING
        }
        n = n.nextSibling();
    }

    // load print repeat columns
    KXmlElement printrepeatcolumns = sheet.namedItem("printrepeatcolumns").toElement();
    if (!printrepeatcolumns.isNull()) {
        int left = printrepeatcolumns.attribute("left").toInt();
        int right = printrepeatcolumns.attribute("right").toInt();
        printSettings()->setRepeatedColumns(qMakePair(left, right));
    }

    // load print repeat rows
    KXmlElement printrepeatrows = sheet.namedItem("printrepeatrows").toElement();
    if (!printrepeatrows.isNull()) {
        int top = printrepeatrows.attribute("top").toInt();
        int bottom = printrepeatrows.attribute("bottom").toInt();
        printSettings()->setRepeatedRows(qMakePair(top, bottom));
    }

    if (!sheet.hasAttribute("borders1.2")) {
        convertObscuringBorders();
    }

    loadXmlProtection(sheet);

    return true;
}


bool KCSheet::loadChildren(KOdfStore* _store)
{
    Q_UNUSED(_store);
#if 0 // KCELLS_KOPART_EMBEDDING
    foreach(EmbeddedObject* object, doc()->embeddedObjects()) {
        if (object->sheet() == this && (object->getType() == OBJECT_KOFFICE_PART || object->getType() == OBJECT_CHART)) {
            kDebug() << "KCellsSheet::loadChildren";
            if (!dynamic_cast<EmbeddedKOfficeObject*>(object)->embeddedObject()->loadDocument(_store))
                return false;
        }
    }
#endif // KCELLS_KOPART_EMBEDDING
    return true;
}


void KCSheet::setShowPageBorders(bool b)
{
    if (b == d->showPageBorders)
        return;

    d->showPageBorders = b;
    // Just repaint everything visible; no need to invalidate the visual cache.
    if (!map()->isLoading()) {
        map()->addDamage(new KCSheetDamage(this, KCSheetDamage::ContentChanged));
    }
}

QImage KCSheet::backgroundImage() const
{
    return d->backgroundImage;
}

void KCSheet::setBackgroundImage(const QImage& image)
{
    d->backgroundImage = image;
}

KCSheet::BackgroundImageProperties KCSheet::backgroundImageProperties() const
{
    return d->backgroundProperties;
}

void KCSheet::setBackgroundImageProperties(const KCSheet::BackgroundImageProperties& properties)
{
    d->backgroundProperties = properties;
}

void KCSheet::insertColumnFormat(KCColumnFormat *l)
{
    d->columns.insertElement(l, l->column());
    if (!map()->isLoading()) {
        map()->addDamage(new KCSheetDamage(this, KCSheetDamage::ColumnsChanged));
    }
}

void KCSheet::insertRowFormat(KCRowFormat *l)
{
    d->rows.insertElement(l, l->row());
    if (!map()->isLoading()) {
        map()->addDamage(new KCSheetDamage(this, KCSheetDamage::RowsChanged));
    }
}

void KCSheet::deleteColumnFormat(int column)
{
    d->columns.removeElement(column);
    if (!map()->isLoading()) {
        map()->addDamage(new KCSheetDamage(this, KCSheetDamage::ColumnsChanged));
    }
}

void KCSheet::deleteRowFormat(int row)
{
    d->rows.removeElement(row);
    if (!map()->isLoading()) {
        map()->addDamage(new KCSheetDamage(this, KCSheetDamage::RowsChanged));
    }
}

void KCSheet::showStatusMessage(const QString &message, int timeout)
{
    emit statusMessage(message, timeout);
}

bool KCSheet::saveChildren(KOdfStore* _store, const QString &_path)
{
    Q_UNUSED(_store);
    Q_UNUSED(_path);
#if 0 // KCELLS_KOPART_EMBEDDING
    int i = 0;
    foreach(EmbeddedObject* object, doc()->embeddedObjects()) {
        if (object->sheet() == this && (object->getType() == OBJECT_KOFFICE_PART || object->getType() == OBJECT_CHART)) {
            QString path = QString("%1/%2").arg(_path).arg(i++);
            if (!dynamic_cast<EmbeddedKOfficeObject*>(object)->embeddedObject()->document()->saveToStore(_store, path))
                return false;
        }
    }
#endif // KCELLS_KOPART_EMBEDDING
    return true;
}

void KCSheet::hideSheet(bool _hide)
{
    setHidden(_hide);
    if (_hide)
        map()->addDamage(new KCSheetDamage(this, KCSheetDamage::Hidden));
    else
        map()->addDamage(new KCSheetDamage(this, KCSheetDamage::Shown));
}

bool KCSheet::setSheetName(const QString& name, bool init)
{
    Q_UNUSED(init);
    if (map()->findSheet(name))
        return false;

    if (isProtected())
        return false;

    if (d->name == name)
        return true;

    QString old_name = d->name;
    d->name = name;

    // FIXME: Why is the change of a sheet's name not supposed to be propagated here?
    // If it is not, we have to manually do so in the loading process, e.g. for the
    // KCSheetAccessModel in the document's data center map.
    //if (init)
    //    return true;

    foreach(KCSheet* sheet, map()->sheetList()) {
        sheet->changeCellTabName(old_name, name);
    }

    map()->addDamage(new KCSheetDamage(this, KCSheetDamage::Name));

    setObjectName(name.toUtf8());
//     (dynamic_cast<SheetIface*>(dcopObject()))->sheetNameHasChanged();

    return true;
}


void KCSheet::updateLocale()
{
    for (int c = 0; c < valueStorage()->count(); ++c) {
        KCCell cell(this, valueStorage()->col(c), valueStorage()->row(c));
        QString text = cell.userInput();
        cell.parseUserInput(text);
    }
    // Affects the displayed value; rebuild the visual cache.
    const KCRegion region(1, 1, KS_colMax, KS_rowMax, this);
    map()->addDamage(new KCCellDamage(this, region, KCCellDamage::Appearance));
}

void KCSheet::convertObscuringBorders()
{
    // FIXME Stefan: Verify that this is not needed anymore.
#if 0
    /* a word of explanation here:
       beginning with KCells 1.2 (actually, cvs of Mar 28, 2002), border information
       is stored differently.  Previously, for a cell obscuring a region, the entire
       region's border's data would be stored in the obscuring cell.  This caused
       some data loss in certain situations.  After that date, each cell stores
       its own border data, and prints it even if it is an obscured cell (as long
       as that border isn't across an obscuring border).
       Anyway, this function is used when loading a file that was stored with the
       old way of borders.  All new files have the sheet attribute "borders1.2" so
       if that isn't in the file, all the border data will be converted here.
       It's a bit of a hack but I can't think of a better way and it's not *that*
       bad of a hack.:-)
    */
    KCCell c = d->cellStorage->firstCell();
    QPen topPen, bottomPen, leftPen, rightPen;
    for (; c; c = c->nextCell()) {
        if (c->extraXCells() > 0 || c->extraYCells() > 0) {
            const KCStyle* style = this->style(c->column(), c->row());
            topPen = style->topBorderPen();
            leftPen = style->leftBorderPen();
            rightPen = style->rightBorderPen();
            bottomPen = style->bottomBorderPen();

            c->format()->setTopBorderStyle(Qt::NoPen);
            c->format()->setLeftBorderStyle(Qt::NoPen);
            c->format()->setRightBorderStyle(Qt::NoPen);
            c->format()->setBottomBorderStyle(Qt::NoPen);

            for (int x = c->column(); x < c->column() + c->extraXCells(); x++) {
                KCCell(this, x, c->row())->setTopBorderPen(topPen);
                KCCell(this, x, c->row() + c->extraYCells())->
                setBottomBorderPen(bottomPen);
            }
            for (int y = c->row(); y < c->row() + c->extraYCells(); y++) {
                KCCell(this, c->column(), y)->setLeftBorderPen(leftPen);
                KCCell(this, c->column() + c->extraXCells(), y)->
                setRightBorderPen(rightPen);
            }
        }
    }
#endif
}

/**********************
 * Printout Functions *
 **********************/

#ifndef NDEBUG
void KCSheet::printDebug()
{
    int iMaxColumn = d->cellStorage->columns();
    int iMaxRow = d->cellStorage->rows();

    kDebug(36001) << "KCCell | Content | KCValue  [UserInput]";
    KCCell cell;
    for (int currentrow = 1 ; currentrow <= iMaxRow ; ++currentrow) {
        for (int currentcolumn = 1 ; currentcolumn <= iMaxColumn ; currentcolumn++) {
            cell = KCCell(this, currentcolumn, currentrow);
            if (!cell.isEmpty()) {
                QString cellDescr = KCCell::name(currentcolumn, currentrow).rightJustified(4);
                //QString cellDescr = "KCCell ";
                //cellDescr += QString::number(currentrow).rightJustified(3,'0') + ',';
                //cellDescr += QString::number(currentcolumn).rightJustified(3,'0') + ' ';
                cellDescr += " | ";
                QString valueType;
                QTextStream stream(&valueType);
                stream << cell.value().type();
                cellDescr += valueType.rightJustified(7);
                cellDescr += " | ";
                cellDescr += map()->converter()->asString(cell.value()).asString().rightJustified(5);
                cellDescr += QString("  [%1]").arg(cell.userInput());
                kDebug(36001) << cellDescr;
            }
        }
    }
}
#endif

#include "KCSheet.moc"
