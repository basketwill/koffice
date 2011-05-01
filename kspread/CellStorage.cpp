/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright 2007, 2009 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
#include "CellStorage.h"
#include "CellStorage_p.h"

// KDE
#include <klocale.h>

// KOffice
#include <KoXmlWriter.h>

// KSpread
#include "KCBindingStorage.h"
#include "ConditionsStorage.h"
#include "Damages.h"
#include "DependencyManager.h"
#include "FormulaStorage.h"
#include "KCMap.h"
#include "ModelSupport.h"
#include "RecalcManager.h"
#include "RectStorage.h"
#include "RowRepeatStorage.h"
#include "KCSheet.h"
#include "StyleStorage.h"
#include "ValidityStorage.h"
#include "ValueStorage.h"

// commands
#include "commands/PointStorageUndoCommand.h"
#include "commands/RectStorageUndoCommand.h"
#include "commands/StyleStorageUndoCommand.h"

// database
#include "database/DatabaseStorage.h"
#include "database/DatabaseManager.h"

Q_DECLARE_METATYPE(QSharedPointer<QTextDocument>)

typedef RectStorage<QString> NamedAreaStorage;

class CellStorage::Private
{
public:
    Private(KCSheet* sheet)
            : sheet(sheet)
            , bindingStorage(new KCBindingStorage(sheet->map()))
            , commentStorage(new CommentStorage(sheet->map()))
            , conditionsStorage(new ConditionsStorage(sheet->map()))
            , databaseStorage(new DatabaseStorage(sheet->map()))
            , formulaStorage(new FormulaStorage())
            , fusionStorage(new FusionStorage(sheet->map()))
            , linkStorage(new LinkStorage())
            , matrixStorage(new MatrixStorage(sheet->map()))
            , namedAreaStorage(new NamedAreaStorage(sheet->map()))
            , styleStorage(new StyleStorage(sheet->map()))
            , userInputStorage(new UserInputStorage())
            , validityStorage(new ValidityStorage(sheet->map()))
            , valueStorage(new ValueStorage())
            , richTextStorage(new RichTextStorage())
            , rowRepeatStorage(new RowRepeatStorage())
            , undoData(0) {}

    Private(const Private& other, KCSheet* sheet)
            : sheet(sheet)
            , bindingStorage(new KCBindingStorage(*other.bindingStorage))
            , commentStorage(new CommentStorage(*other.commentStorage))
            , conditionsStorage(new ConditionsStorage(*other.conditionsStorage))
            , databaseStorage(new DatabaseStorage(*other.databaseStorage))
            , formulaStorage(new FormulaStorage(*other.formulaStorage))
            , fusionStorage(new FusionStorage(*other.fusionStorage))
            , linkStorage(new LinkStorage(*other.linkStorage))
            , matrixStorage(new MatrixStorage(*other.matrixStorage))
            , namedAreaStorage(new NamedAreaStorage(*other.namedAreaStorage))
            , styleStorage(new StyleStorage(*other.styleStorage))
            , userInputStorage(new UserInputStorage(*other.userInputStorage))
            , validityStorage(new ValidityStorage(*other.validityStorage))
            , valueStorage(new ValueStorage(*other.valueStorage))
            , richTextStorage(new RichTextStorage(*other.richTextStorage))
            , rowRepeatStorage(new RowRepeatStorage(*other.rowRepeatStorage))
            , undoData(0) {}

    ~Private() {
        delete bindingStorage;
        delete commentStorage;
        delete conditionsStorage;
        delete databaseStorage;
        delete formulaStorage;
        delete fusionStorage;
        delete linkStorage;
        delete matrixStorage;
        delete namedAreaStorage;
        delete styleStorage;
        delete userInputStorage;
        delete validityStorage;
        delete valueStorage;
        delete richTextStorage;
        delete rowRepeatStorage;
    }

    void createCommand(QUndoCommand *parent) const;

    KCSheet*                  sheet;
    KCBindingStorage*         bindingStorage;
    CommentStorage*         commentStorage;
    ConditionsStorage*      conditionsStorage;
    DatabaseStorage*        databaseStorage;
    FormulaStorage*         formulaStorage;
    FusionStorage*          fusionStorage;
    LinkStorage*            linkStorage;
    MatrixStorage*          matrixStorage;
    NamedAreaStorage*       namedAreaStorage;
    StyleStorage*           styleStorage;
    UserInputStorage*       userInputStorage;
    ValidityStorage*        validityStorage;
    ValueStorage*           valueStorage;
    RichTextStorage*        richTextStorage;
    RowRepeatStorage*       rowRepeatStorage;
    CellStorageUndoData*    undoData;
};

void CellStorage::Private::createCommand(QUndoCommand *parent) const
{
    if (!undoData->bindings.isEmpty()) {
        RectStorageUndoCommand<KCBinding> *const command
        = new RectStorageUndoCommand<KCBinding>(sheet->model(), SourceRangeRole, parent);
        command->add(undoData->bindings);
    }
    if (!undoData->comments.isEmpty()) {
        RectStorageUndoCommand<QString> *const command
        = new RectStorageUndoCommand<QString>(sheet->model(), CommentRole, parent);
        command->add(undoData->comments);
    }
    if (!undoData->conditions.isEmpty()) {
        RectStorageUndoCommand<Conditions> *const command
        = new RectStorageUndoCommand<Conditions>(sheet->model(), ConditionRole, parent);
        command->add(undoData->conditions);
    }
    if (!undoData->databases.isEmpty()) {
        RectStorageUndoCommand<Database> *const command
        = new RectStorageUndoCommand<Database>(sheet->model(), TargetRangeRole, parent);
        command->add(undoData->databases);
    }
    if (!undoData->formulas.isEmpty()) {
        PointStorageUndoCommand<Formula> *const command
        = new PointStorageUndoCommand<Formula>(sheet->model(), FormulaRole, parent);
        command->add(undoData->formulas);
    }
    if (!undoData->fusions.isEmpty()) {
        RectStorageUndoCommand<bool> *const command
        = new RectStorageUndoCommand<bool>(sheet->model(), FusionedRangeRole, parent);
        command->add(undoData->fusions);
    }
    if (!undoData->links.isEmpty()) {
        PointStorageUndoCommand<QString> *const command
        = new PointStorageUndoCommand<QString>(sheet->model(), LinkRole, parent);
        command->add(undoData->links);
    }
    if (!undoData->matrices.isEmpty()) {
        RectStorageUndoCommand<bool> *const command
        = new RectStorageUndoCommand<bool>(sheet->model(), LockedRangeRole, parent);
        command->add(undoData->matrices);
    }
    if (!undoData->namedAreas.isEmpty()) {
        RectStorageUndoCommand<QString> *const command
        = new RectStorageUndoCommand<QString>(sheet->model(), NamedAreaRole, parent);
        command->add(undoData->namedAreas);
    }
    if (!undoData->richTexts.isEmpty()) {
        PointStorageUndoCommand<QSharedPointer<QTextDocument> > *const command
        = new PointStorageUndoCommand<QSharedPointer<QTextDocument> >(sheet->model(), RichTextRole, parent);
        command->add(undoData->richTexts);
    }
    if (!undoData->styles.isEmpty()) {
        StyleStorageUndoCommand *const command
        = new StyleStorageUndoCommand(styleStorage, parent);
        command->add(undoData->styles);
    }
    if (!undoData->userInputs.isEmpty()) {
        PointStorageUndoCommand<QString> *const command
        = new PointStorageUndoCommand<QString>(sheet->model(), UserInputRole, parent);
        command->add(undoData->userInputs);
    }
    if (!undoData->validities.isEmpty()) {
        RectStorageUndoCommand<Validity> *const command
        = new RectStorageUndoCommand<Validity>(sheet->model(), ValidityRole, parent);
        command->add(undoData->validities);
    }
    if (!undoData->values.isEmpty()) {
        PointStorageUndoCommand<KCValue> *const command
        = new PointStorageUndoCommand<KCValue>(sheet->model(), ValueRole, parent);
        command->add(undoData->values);
    }
}


CellStorage::CellStorage(KCSheet* sheet)
        : QObject(sheet)
        , d(new Private(sheet))
{
}

CellStorage::CellStorage(const CellStorage& other)
        : QObject(other.d->sheet)
        , d(new Private(*other.d))
{
}

CellStorage::CellStorage(const CellStorage& other, KCSheet* sheet)
        : QObject(sheet)
        , d(new Private(*other.d, sheet))
{
}

CellStorage::~CellStorage()
{
    delete d;
}

KCSheet* CellStorage::sheet() const
{
    return d->sheet;
}

void CellStorage::take(int col, int row)
{
    Formula oldFormula;
    QString oldLink;
    QString oldUserInput;
    KCValue oldValue;
    QSharedPointer<QTextDocument> oldRichText;

    oldFormula = d->formulaStorage->take(col, row);
    oldLink = d->linkStorage->take(col, row);
    oldUserInput = d->userInputStorage->take(col, row);
    oldValue = d->valueStorage->take(col, row);
    oldRichText = d->richTextStorage->take(col, row);

    if (!d->sheet->map()->isLoading()) {
        // Trigger a recalculation of the consuming cells.
        CellDamage::Changes changes = CellDamage:: KCBinding | CellDamage::Formula | CellDamage::KCValue;
        d->sheet->map()->addDamage(new CellDamage(KCCell(d->sheet, col, row), changes));

        d->rowRepeatStorage->setRowRepeat(row, 1);
    }
    // also trigger a relayout of the first non-empty cell to the left of this cell
    int prevCol;
    KCValue v = d->valueStorage->prevInRow(col, row, &prevCol);
    if (!v.isEmpty())
        d->sheet->map()->addDamage(new CellDamage(KCCell(d->sheet, prevCol, row), CellDamage::Appearance));


    // recording undo?
    if (d->undoData) {
        d->undoData->formulas   << qMakePair(QPoint(col, row), oldFormula);
        d->undoData->links      << qMakePair(QPoint(col, row), oldLink);
        d->undoData->userInputs << qMakePair(QPoint(col, row), oldUserInput);
        d->undoData->values     << qMakePair(QPoint(col, row), oldValue);
        d->undoData->richTexts  << qMakePair(QPoint(col, row), oldRichText);
    }
}

KCBinding CellStorage::binding(int column, int row) const
{
    return d->bindingStorage->contains(QPoint(column, row));
}

void CellStorage::setBinding(const KCRegion& region, const KCBinding& binding)
{
    // recording undo?
    if (d->undoData)
        d->undoData->bindings << d->bindingStorage->undoData(region);

    d->bindingStorage->insert(region, binding);
}

void CellStorage::removeBinding(const KCRegion& region, const KCBinding& binding)
{
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings << d->bindingStorage->undoData(region);
    }
    d->bindingStorage->remove(region, binding);
}

QString CellStorage::comment(int column, int row) const
{
    return d->commentStorage->contains(QPoint(column, row));
}

void CellStorage::setComment(const KCRegion& region, const QString& comment)
{
    // recording undo?
    if (d->undoData)
        d->undoData->comments << d->commentStorage->undoData(region);

    d->commentStorage->insert(region, comment);
    if (!d->sheet->map()->isLoading()) {
        foreach (const QRect& r, region.rects()) {
            d->rowRepeatStorage->splitRowRepeat(r.top());
            d->rowRepeatStorage->splitRowRepeat(r.bottom()+1);
        }
    }
}

Conditions CellStorage::conditions(int column, int row) const
{
    return d->conditionsStorage->contains(QPoint(column, row));
}

void CellStorage::setConditions(const KCRegion& region, Conditions conditions)
{
    // recording undo?
    if (d->undoData)
        d->undoData->conditions << d->conditionsStorage->undoData(region);

    d->conditionsStorage->insert(region, conditions);
    if (!d->sheet->map()->isLoading()) {
        foreach (const QRect& r, region.rects()) {
            d->rowRepeatStorage->splitRowRepeat(r.top());
            d->rowRepeatStorage->splitRowRepeat(r.bottom()+1);
        }
    }
}

Database CellStorage::database(int column, int row) const
{
    QPair<QRectF, Database> pair = d->databaseStorage->containedPair(QPoint(column, row));
    if (pair.first.isEmpty())
        return Database();
    if (pair.second.isEmpty())
        return Database();
    // update the range, which might get changed
    Database database = pair.second;
    database.setRange(KCRegion(pair.first.toRect(), d->sheet));
    return database;
}

QList< QPair<QRectF, Database> > CellStorage::databases(const KCRegion& region) const
{
    return d->databaseStorage->intersectingPairs(region);
}

void CellStorage::setDatabase(const KCRegion& region, const Database& database)
{
    // recording undo?
    if (d->undoData)
        d->undoData->databases << d->databaseStorage->undoData(region);

    d->databaseStorage->insert(region, database);
}

Formula CellStorage::formula(int column, int row) const
{
    return d->formulaStorage->lookup(column, row, Formula::empty());
}

void CellStorage::setFormula(int column, int row, const Formula& formula)
{
    Formula old = Formula::empty();
    if (formula.expression().isEmpty())
        old = d->formulaStorage->take(column, row, Formula::empty());
    else
        old = d->formulaStorage->insert(column, row, formula);

    // formula changed?
    if (formula != old) {
        if (!d->sheet->map()->isLoading()) {
            // trigger an update of the dependencies and a recalculation
            d->sheet->map()->addDamage(new CellDamage(KCCell(d->sheet, column, row), CellDamage::Formula | CellDamage::KCValue));
            d->rowRepeatStorage->setRowRepeat(row, 1);
        }
        // recording undo?
        if (d->undoData) {
            d->undoData->formulas << qMakePair(QPoint(column, row), old);
            // Also store the old value, if there wasn't a formula before,
            // because the new value is calculated later by the damage
            // processing and is not recorded for undoing.
            if (old == Formula())
                d->undoData->values << qMakePair(QPoint(column, row), value(column, row));
        }
    }
}

QString CellStorage::link(int column, int row) const
{
    return d->linkStorage->lookup(column, row);
}

void CellStorage::setLink(int column, int row, const QString& link)
{
    QString old;
    if (link.isEmpty())
        old = d->linkStorage->take(column, row);
    else
        old = d->linkStorage->insert(column, row, link);

    // recording undo?
    if (d->undoData && link != old)
        d->undoData->links << qMakePair(QPoint(column, row), old);
    if (!d->sheet->map()->isLoading())
        d->rowRepeatStorage->setRowRepeat(row, 1);
}

QString CellStorage::namedArea(int column, int row) const
{
    QPair<QRectF, QString> pair = d->namedAreaStorage->containedPair(QPoint(column, row));
    if (pair.first.isEmpty())
        return QString();
    if (pair.second.isEmpty())
        return QString();
    return pair.second;
}

QList< QPair<QRectF, QString> > CellStorage::namedAreas(const KCRegion& region) const
{
    return d->namedAreaStorage->intersectingPairs(region);
}

void CellStorage::setNamedArea(const KCRegion& region, const QString& namedArea)
{
    // recording undo?
    if (d->undoData)
        d->undoData->namedAreas << d->namedAreaStorage->undoData(region);

    d->namedAreaStorage->insert(region, namedArea);
}

void CellStorage::emitInsertNamedArea(const KCRegion &region, const QString &namedArea)
{
    emit insertNamedArea(region, namedArea);
}

KCStyle CellStorage::style(int column, int row) const
{
    return d->styleStorage->contains(QPoint(column, row));
}

KCStyle CellStorage::style(const QRect& rect) const
{
    return d->styleStorage->contains(rect);
}

void CellStorage::setStyle(const KCRegion& region, const KCStyle& style)
{
    // recording undo?
    if (d->undoData)
        d->undoData->styles << d->styleStorage->undoData(region);

    d->styleStorage->insert(region, style);
    if (!d->sheet->map()->isLoading()) {
        foreach (const QRect& r, region.rects()) {
            d->rowRepeatStorage->splitRowRepeat(r.top());
            d->rowRepeatStorage->splitRowRepeat(r.bottom()+1);
        }
    }
}

void CellStorage::insertSubStyle(const QRect &rect, const SharedSubStyle &subStyle)
{
    d->styleStorage->insert(rect, subStyle);
    if (!d->sheet->map()->isLoading()) {
        d->rowRepeatStorage->splitRowRepeat(rect.top());
        d->rowRepeatStorage->splitRowRepeat(rect.bottom()+1);
    }
}

QString CellStorage::userInput(int column, int row) const
{
    return d->userInputStorage->lookup(column, row);
}

void CellStorage::setUserInput(int column, int row, const QString& userInput)
{
    QString old;
    if (userInput.isEmpty())
        old = d->userInputStorage->take(column, row);
    else
        old = d->userInputStorage->insert(column, row, userInput);

    // recording undo?
    if (d->undoData && userInput != old)
        d->undoData->userInputs << qMakePair(QPoint(column, row), old);
    if (!d->sheet->map()->isLoading())
        d->rowRepeatStorage->setRowRepeat(row, 1);
}

QSharedPointer<QTextDocument> CellStorage::richText(int column, int row) const
{
    return d->richTextStorage->lookup(column, row);
}

void CellStorage::setRichText(int column, int row, QSharedPointer<QTextDocument> text)
{
    QSharedPointer<QTextDocument> old;
    if (text.isNull())
        old = d->richTextStorage->take(column, row);
    else
        old = d->richTextStorage->insert(column, row, text);

    // recording undo?
    if (d->undoData && text != old)
        d->undoData->richTexts << qMakePair(QPoint(column, row), old);
}

Validity CellStorage::validity(int column, int row) const
{
    return d->validityStorage->contains(QPoint(column, row));
}

void CellStorage::setValidity(const KCRegion& region, Validity validity)
{
    // recording undo?
    if (d->undoData)
        d->undoData->validities << d->validityStorage->undoData(region);

    d->validityStorage->insert(region, validity);
    if (!d->sheet->map()->isLoading()) {
        foreach (const QRect& r, region.rects()) {
            d->rowRepeatStorage->splitRowRepeat(r.top());
            d->rowRepeatStorage->splitRowRepeat(r.bottom()+1);
        }
    }
}

KCValue CellStorage::value(int column, int row) const
{
    return d->valueStorage->lookup(column, row);
}

KCValue CellStorage::valueRegion(const KCRegion& region) const
{
    // create a subStorage with adjusted origin
    return KCValue(d->valueStorage->subStorage(region, false), region.boundingRect().size());
}

void CellStorage::setValue(int column, int row, const KCValue& value)
{
    // release any lock
    unlockCells(column, row);

    KCValue old;
    if (value.isEmpty())
        old = d->valueStorage->take(column, row);
    else
        old = d->valueStorage->insert(column, row, value);

    // value changed?
    if (value != old) {
        if (!d->sheet->map()->isLoading()) {
            // Always trigger a repainting and a binding update.
            CellDamage::Changes changes = CellDamage::Appearance | CellDamage::KCBinding;
            // Trigger a recalculation of the consuming cells, only if we are not
            // already in a recalculation process.
            if (!d->sheet->map()->recalcManager()->isActive())
                changes |= CellDamage::KCValue;
            d->sheet->map()->addDamage(new CellDamage(KCCell(d->sheet, column, row), changes));
            // Also trigger a relayouting of the first non-empty cell to the left of this one
            int prevCol;
            KCValue v = d->valueStorage->prevInRow(column, row, &prevCol);
            if (!v.isEmpty())
                d->sheet->map()->addDamage(new CellDamage(KCCell(d->sheet, prevCol, row), CellDamage::Appearance));
            d->rowRepeatStorage->setRowRepeat(row, 1);
        }
        // recording undo?
        if (d->undoData)
            d->undoData->values << qMakePair(QPoint(column, row), old);
    }
}

bool CellStorage::doesMergeCells(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->fusionStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return false;
    if (pair.second == false)
        return false;
    // master cell?
    if (pair.first.toRect().topLeft() != QPoint(column, row))
        return false;
    return true;
}

bool CellStorage::isPartOfMerged(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->fusionStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return false;
    if (pair.second == false)
        return false;
    // master cell?
    if (pair.first.toRect().topLeft() == QPoint(column, row))
        return false;
    return true;
}

void CellStorage::mergeCells(int column, int row, int numXCells, int numYCells)
{
    // Start by unmerging the cells that we merge right now
    const QPair<QRectF, bool> pair = d->fusionStorage->containedPair(QPoint(column, row));
    if (!pair.first.isNull())
        d->fusionStorage->insert(KCRegion(pair.first.toRect()), false);
    // Merge the cells
    if (numXCells != 0 || numYCells != 0)
        d->fusionStorage->insert(KCRegion(column, row, numXCells + 1, numYCells + 1), true);
    if (!d->sheet->map()->isLoading())
        d->rowRepeatStorage->setRowRepeat(row, 1);
}

KCCell CellStorage::masterCell(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->fusionStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return KCCell(d->sheet, column, row);
    if (pair.second == false)
        return KCCell(d->sheet, column, row);
    return KCCell(d->sheet, pair.first.toRect().topLeft());
}

int CellStorage::mergedXCells(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->fusionStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return 0;
    // Not the master cell?
    if (pair.first.topLeft() != QPoint(column, row))
        return 0;
    return pair.first.toRect().width() - 1;
}

int CellStorage::mergedYCells(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->fusionStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return 0;
    // Not the master cell?
    if (pair.first.topLeft() != QPoint(column, row))
        return 0;
    return pair.first.toRect().height() - 1;
}

QList<KCCell> CellStorage::masterCells(const KCRegion& region) const
{
    const QList<QPair<QRectF, bool> > pairs = d->fusionStorage->intersectingPairs(region);
    if (pairs.isEmpty())
        return QList<KCCell>();
    QList<KCCell> masterCells;
    for (int i = 0; i < pairs.count(); ++i) {
        if (pairs[i].first.isNull())
            continue;
        if (pairs[i].second == false)
            continue;
        masterCells.append(KCCell(d->sheet, pairs[i].first.toRect().topLeft()));
    }
    return masterCells;
}

bool CellStorage::locksCells(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->matrixStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return false;
    if (pair.second == false)
        return false;
    // master cell?
    if (pair.first.toRect().topLeft() != QPoint(column, row))
        return false;
    return true;
}

bool CellStorage::isLocked(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->matrixStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return false;
    if (pair.second == false)
        return false;
    // master cell?
    if (pair.first.toRect().topLeft() == QPoint(column, row))
        return false;
    return true;
}

bool CellStorage::hasLockedCells(const KCRegion& region) const
{
    typedef QPair<QRectF, bool> RectBoolPair;
    QList<QPair<QRectF, bool> > pairs = d->matrixStorage->intersectingPairs(region);
    foreach (const RectBoolPair& pair, pairs) {
        if (pair.first.isNull())
            continue;
        if (pair.second == false)
            continue;
        // more than just the master cell in the region?
        const QPoint topLeft = pair.first.toRect().topLeft();
        if (pair.first.width() >= 1) {
            if (region.contains(topLeft + QPoint(1, 0), d->sheet))
                return true;
        }
        if (pair.first.height() >= 1) {
            if (region.contains(topLeft + QPoint(0, 1), d->sheet))
                return true;
        }
    }
    return false;
}

void CellStorage::lockCells(const QRect& rect)
{
    // Start by unlocking the cells that we lock right now
    const QPair<QRectF, bool> pair = d->matrixStorage->containedPair(rect.topLeft());  // FIXME
    if (!pair.first.isNull())
        d->matrixStorage->insert(KCRegion(pair.first.toRect()), false);
    // Lock the cells
    if (rect.width() > 1 || rect.height() > 1)
        d->matrixStorage->insert(KCRegion(rect), true);
}

void CellStorage::unlockCells(int column, int row)
{
    const QPair<QRectF, bool> pair = d->matrixStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return;
    if (pair.second == false)
        return;
    if (pair.first.toRect().topLeft() != QPoint(column, row))
        return;
    const QRect rect = pair.first.toRect();
    d->matrixStorage->insert(KCRegion(rect), false);
    // clear the values
    for (int r = rect.top(); r <= rect.bottom(); ++r) {
        for (int c = rect.left(); c <= rect.right(); ++c) {
            if (r != rect.top() || c != rect.left())
                setValue(c, r, KCValue());
        }
    }
    // recording undo?
    if (d->undoData)
        d->undoData->matrices << pair;
}

QRect CellStorage::lockedCells(int column, int row) const
{
    const QPair<QRectF, bool> pair = d->matrixStorage->containedPair(QPoint(column, row));
    if (pair.first.isNull())
        return QRect(column, row, 1, 1);
    if (pair.second == false)
        return QRect(column, row, 1, 1);
    if (pair.first.toRect().topLeft() != QPoint(column, row))
        return QRect(column, row, 1, 1);
    return pair.first.toRect();
}

void CellStorage::insertColumns(int position, int number)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    // FIXME Stefan: Would it be better to directly alter the dependency tree?
    // TODO Stefan: Optimize: Avoid the double creation of the sub-storages, but don't process
    //              formulas, that will get out of bounds after the operation.
    const KCRegion invalidRegion(QRect(QPoint(position, 1), QPoint(KS_colMax, KS_rowMax)), d->sheet);
    PointStorage<Formula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger an update of the bindings and the named areas.
    d->sheet->map()->addDamage(new CellDamage(d->sheet, invalidRegion, CellDamage::KCBinding | CellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->insertColumns(position, number);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->insertColumns(position, number);
    QList< QPair<QRectF, Conditions> > conditions = d->conditionsStorage->insertColumns(position, number);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->insertColumns(position, number);
    QVector< QPair<QPoint, Formula> > formulas = d->formulaStorage->insertColumns(position, number);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->insertColumns(position, number);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->insertColumns(position, number);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->insertColumns(position, number);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->insertColumns(position, number);
    QList< QPair<QRectF, SharedSubStyle> > styles = d->styleStorage->insertColumns(position, number);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->insertColumns(position, number);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->insertColumns(position, number);
    QList< QPair<QRectF, Validity> > validities = d->validityStorage->insertColumns(position, number);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->insertColumns(position, number);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new CellDamage(d->sheet, providers, CellDamage::KCValue));
}

void CellStorage::removeColumns(int position, int number)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(QPoint(position, 1), QPoint(KS_colMax, KS_rowMax)), d->sheet);
    PointStorage<Formula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger an update of the bindings and the named areas.
    const KCRegion region(QRect(QPoint(position - 1, 1), QPoint(KS_colMax, KS_rowMax)), d->sheet);
    d->sheet->map()->addDamage(new CellDamage(d->sheet, region, CellDamage::KCBinding | CellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->removeColumns(position, number);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->removeColumns(position, number);
    QList< QPair<QRectF, Conditions> > conditions = d->conditionsStorage->removeColumns(position, number);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->removeColumns(position, number);
    QVector< QPair<QPoint, Formula> > formulas = d->formulaStorage->removeColumns(position, number);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->removeColumns(position, number);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->removeColumns(position, number);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->removeColumns(position, number);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->removeColumns(position, number);
    QList< QPair<QRectF, SharedSubStyle> > styles = d->styleStorage->removeColumns(position, number);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->removeColumns(position, number);
    QList< QPair<QRectF, Validity> > validities = d->validityStorage->removeColumns(position, number);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->removeColumns(position, number);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->removeColumns(position, number);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new CellDamage(d->sheet, providers, CellDamage::KCValue));
}

void CellStorage::insertRows(int position, int number)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(QPoint(1, position), QPoint(KS_colMax, KS_rowMax)), d->sheet);
    PointStorage<Formula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger an update of the bindings and the named areas.
    d->sheet->map()->addDamage(new CellDamage(d->sheet, invalidRegion, CellDamage::KCBinding | CellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->insertRows(position, number);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->insertRows(position, number);
    QList< QPair<QRectF, Conditions> > conditions = d->conditionsStorage->insertRows(position, number);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->insertRows(position, number);
    QVector< QPair<QPoint, Formula> > formulas = d->formulaStorage->insertRows(position, number);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->insertRows(position, number);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->insertRows(position, number);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->insertRows(position, number);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->insertRows(position, number);
    QList< QPair<QRectF, SharedSubStyle> > styles = d->styleStorage->insertRows(position, number);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->insertRows(position, number);
    QList< QPair<QRectF, Validity> > validities = d->validityStorage->insertRows(position, number);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->insertRows(position, number);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->insertRows(position, number);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new CellDamage(d->sheet, providers, CellDamage::KCValue));

    d->rowRepeatStorage->insertRows(position, number);
}

void CellStorage::removeRows(int position, int number)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(QPoint(1, position), QPoint(KS_colMax, KS_rowMax)), d->sheet);
    PointStorage<Formula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger an update of the bindings and the named areas.
    const KCRegion region(QRect(QPoint(1, position - 1), QPoint(KS_colMax, KS_rowMax)), d->sheet);
    d->sheet->map()->addDamage(new CellDamage(d->sheet, region, CellDamage::KCBinding | CellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->removeRows(position, number);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->removeRows(position, number);
    QList< QPair<QRectF, Conditions> > conditions = d->conditionsStorage->removeRows(position, number);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->removeRows(position, number);
    QVector< QPair<QPoint, Formula> > formulas = d->formulaStorage->removeRows(position, number);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->removeRows(position, number);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->removeRows(position, number);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->removeRows(position, number);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->removeRows(position, number);
    QList< QPair<QRectF, SharedSubStyle> > styles = d->styleStorage->removeRows(position, number);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->removeRows(position, number);
    QList< QPair<QRectF, Validity> > validities = d->validityStorage->removeRows(position, number);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->removeRows(position, number);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->removeRows(position, number);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new CellDamage(d->sheet, providers, CellDamage::KCValue));

    d->rowRepeatStorage->removeRows(position, number);
}

void CellStorage::removeShiftLeft(const QRect& rect)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(rect.topLeft(), QPoint(KS_colMax, rect.bottom())), d->sheet);
    PointStorage<Formula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger an update of the bindings and the named areas.
    const KCRegion region(QRect(rect.topLeft() - QPoint(1, 0), QPoint(KS_colMax, rect.bottom())), d->sheet);
    d->sheet->map()->addDamage(new CellDamage(d->sheet, region, CellDamage::KCBinding | CellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, Conditions> > conditions = d->conditionsStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->removeShiftLeft(rect);
    QVector< QPair<QPoint, Formula> > formulas = d->formulaStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->removeShiftLeft(rect);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, SharedSubStyle> > styles = d->styleStorage->removeShiftLeft(rect);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->removeShiftLeft(rect);
    QList< QPair<QRectF, Validity> > validities = d->validityStorage->removeShiftLeft(rect);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->removeShiftLeft(rect);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->removeShiftLeft(rect);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new CellDamage(d->sheet, providers, CellDamage::KCValue));

    d->rowRepeatStorage->removeShiftLeft(rect);
}

void CellStorage::insertShiftRight(const QRect& rect)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(rect.topLeft(), QPoint(KS_colMax, rect.bottom())), d->sheet);
    PointStorage<Formula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger an update of the bindings and the named areas.
    d->sheet->map()->addDamage(new CellDamage(d->sheet, invalidRegion, CellDamage::KCBinding | CellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->insertShiftRight(rect);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->insertShiftRight(rect);
    QList< QPair<QRectF, Conditions> > conditions = d->conditionsStorage->insertShiftRight(rect);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->insertShiftRight(rect);
    QVector< QPair<QPoint, Formula> > formulas = d->formulaStorage->insertShiftRight(rect);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->insertShiftRight(rect);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->insertShiftRight(rect);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->insertShiftRight(rect);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->insertShiftRight(rect);
    QList< QPair<QRectF, SharedSubStyle> > styles = d->styleStorage->insertShiftRight(rect);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->insertShiftRight(rect);
    QList< QPair<QRectF, Validity> > validities = d->validityStorage->insertShiftRight(rect);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->insertShiftRight(rect);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->insertShiftRight(rect);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new CellDamage(d->sheet, providers, CellDamage::KCValue));

    d->rowRepeatStorage->insertShiftRight(rect);
}

void CellStorage::removeShiftUp(const QRect& rect)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(rect.topLeft(), QPoint(rect.right(), KS_rowMax)), d->sheet);
    PointStorage<Formula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger an update of the bindings and the named areas.
    const KCRegion region(QRect(rect.topLeft() - QPoint(0, 1), QPoint(rect.right(), KS_rowMax)), d->sheet);
    d->sheet->map()->addDamage(new CellDamage(d->sheet, region, CellDamage::KCBinding | CellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->removeShiftUp(rect);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->removeShiftUp(rect);
    QList< QPair<QRectF, Conditions> > conditions = d->conditionsStorage->removeShiftUp(rect);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->removeShiftUp(rect);
    QVector< QPair<QPoint, Formula> > formulas = d->formulaStorage->removeShiftUp(rect);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->removeShiftUp(rect);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->removeShiftUp(rect);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->removeShiftUp(rect);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->removeShiftUp(rect);
    QList< QPair<QRectF, SharedSubStyle> > styles = d->styleStorage->removeShiftUp(rect);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->removeShiftUp(rect);
    QList< QPair<QRectF, Validity> > validities = d->validityStorage->removeShiftUp(rect);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->removeShiftUp(rect);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->removeShiftUp(rect);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new CellDamage(d->sheet, providers, CellDamage::KCValue));

    d->rowRepeatStorage->removeShiftUp(rect);
}

void CellStorage::insertShiftDown(const QRect& rect)
{
    // Trigger a dependency update of the cells, which have a formula. (old positions)
    const KCRegion invalidRegion(QRect(rect.topLeft(), QPoint(rect.right(), KS_rowMax)), d->sheet);
    PointStorage<Formula> subStorage = d->formulaStorage->subStorage(invalidRegion);
    KCCell cell;
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger an update of the bindings and the named areas.
    d->sheet->map()->addDamage(new CellDamage(d->sheet, invalidRegion, CellDamage::KCBinding | CellDamage::NamedArea));

    QList< QPair<QRectF, KCBinding> > bindings = d->bindingStorage->insertShiftDown(rect);
    QList< QPair<QRectF, QString> > comments = d->commentStorage->insertShiftDown(rect);
    QList< QPair<QRectF, Conditions> > conditions = d->conditionsStorage->insertShiftDown(rect);
    QList< QPair<QRectF, Database> > databases = d->databaseStorage->insertShiftDown(rect);
    QVector< QPair<QPoint, Formula> > formulas = d->formulaStorage->insertShiftDown(rect);
    QList< QPair<QRectF, bool> > fusions = d->fusionStorage->insertShiftDown(rect);
    QVector< QPair<QPoint, QString> > links = d->linkStorage->insertShiftDown(rect);
    QList< QPair<QRectF, bool> > matrices = d->matrixStorage->insertShiftDown(rect);
    QList< QPair<QRectF, QString> > namedAreas = d->namedAreaStorage->insertShiftDown(rect);
    QList< QPair<QRectF, SharedSubStyle> > styles = d->styleStorage->insertShiftDown(rect);
    QVector< QPair<QPoint, QString> > userInputs = d->userInputStorage->insertShiftDown(rect);
    QList< QPair<QRectF, Validity> > validities = d->validityStorage->insertShiftDown(rect);
    QVector< QPair<QPoint, KCValue> > values = d->valueStorage->insertShiftDown(rect);
    QVector< QPair<QPoint, QSharedPointer<QTextDocument> > > richTexts = d->richTextStorage->insertShiftDown(rect);
    // recording undo?
    if (d->undoData) {
        d->undoData->bindings   << bindings;
        d->undoData->comments   << comments;
        d->undoData->conditions << conditions;
        d->undoData->databases  << databases;
        d->undoData->formulas   << formulas;
        d->undoData->fusions    << fusions;
        d->undoData->links      << links;
        d->undoData->matrices   << matrices;
        d->undoData->namedAreas << namedAreas;
        d->undoData->styles     << styles;
        d->undoData->userInputs << userInputs;
        d->undoData->validities << validities;
        d->undoData->values     << values;
        d->undoData->richTexts  << richTexts;
    }

    // Trigger a dependency update of the cells, which have a formula. (new positions)
    subStorage = d->formulaStorage->subStorage(invalidRegion);
    for (int i = 0; i < subStorage.count(); ++i) {
        cell = KCCell(d->sheet, subStorage.col(i), subStorage.row(i));
        d->sheet->map()->addDamage(new CellDamage(cell, CellDamage::Formula));
    }
    // Trigger a recalculation only for the cells, that depend on values in the changed region.
    KCRegion providers = d->sheet->map()->dependencyManager()->reduceToProvidingRegion(invalidRegion);
    d->sheet->map()->addDamage(new CellDamage(d->sheet, providers, CellDamage::KCValue));

    d->rowRepeatStorage->insertShiftDown(rect);
}

KCCell CellStorage::firstInColumn(int col, Visiting visiting) const
{
    Q_UNUSED(visiting);

    int newRow = 0;
    int tmpRow = 0;
    d->formulaStorage->firstInColumn(col, &tmpRow);
    newRow = tmpRow;
    d->valueStorage->firstInColumn(col, &tmpRow);
    if (tmpRow)
        newRow = newRow ? qMin(newRow, tmpRow) : tmpRow;
    if (!newRow)
        return KCCell();
    return KCCell(d->sheet, col, newRow);
}

KCCell CellStorage::firstInRow(int row, Visiting visiting) const
{
    int newCol = 0;
    int tmpCol = 0;
    d->formulaStorage->firstInRow(row, &tmpCol);
    newCol = tmpCol;
    d->valueStorage->firstInRow(row, &tmpCol);
    if (tmpCol)
        newCol = newCol ? qMin(newCol, tmpCol) : tmpCol;
    if (visiting == VisitAll) {
        tmpCol = d->styleStorage->firstColumnIndexInRow(row);
        if (tmpCol)
            newCol = newCol ? qMin(newCol, tmpCol) : tmpCol;
    }
    if (!newCol)
        return KCCell();
    return KCCell(d->sheet, newCol, row);
}

KCCell CellStorage::lastInColumn(int col, Visiting visiting) const
{
    Q_UNUSED(visiting);
    int newRow = 0;
    int tmpRow = 0;
    d->formulaStorage->lastInColumn(col, &tmpRow);
    newRow = tmpRow;
    d->valueStorage->lastInColumn(col, &tmpRow);
    newRow = qMax(newRow, tmpRow);
    if (!newRow)
        return KCCell();
    return KCCell(d->sheet, col, newRow);
}

KCCell CellStorage::lastInRow(int row, Visiting visiting) const
{
    Q_UNUSED(visiting);
    int newCol = 0;
    int tmpCol = 0;
    d->formulaStorage->lastInRow(row, &tmpCol);
    newCol = tmpCol;
    d->valueStorage->lastInRow(row, &tmpCol);
    newCol = qMax(newCol, tmpCol);
    if (!newCol)
        return KCCell();
    return KCCell(d->sheet, newCol, row);
}

KCCell CellStorage::nextInColumn(int col, int row, Visiting visiting) const
{
    Q_UNUSED(visiting);
    int newRow = 0;
    int tmpRow = 0;
    d->formulaStorage->nextInColumn(col, row, &tmpRow);
    newRow = tmpRow;
    d->valueStorage->nextInColumn(col, row, &tmpRow);
    if (tmpRow)
        newRow = newRow ? qMin(newRow, tmpRow) : tmpRow;
    if (!newRow)
        return KCCell();
    return KCCell(d->sheet, col, newRow);
}

KCCell CellStorage::nextInRow(int col, int row, Visiting visiting) const
{
    int newCol = 0;
    int tmpCol = 0;
    d->formulaStorage->nextInRow(col, row, &tmpCol);
    newCol = tmpCol;
    d->valueStorage->nextInRow(col, row, &tmpCol);
    if (tmpCol)
        newCol = newCol ? qMin(newCol, tmpCol) : tmpCol;
    if (visiting == VisitAll) {
        tmpCol = d->styleStorage->nextColumnIndexInRow(col, row);
        if (tmpCol)
            newCol = newCol ? qMin(newCol, tmpCol) : tmpCol;
    }
    if (!newCol)
        return KCCell();
    return KCCell(d->sheet, newCol, row);
}

KCCell CellStorage::prevInColumn(int col, int row, Visiting visiting) const
{
    Q_UNUSED(visiting);
    int newRow = 0;
    int tmpRow = 0;
    d->formulaStorage->prevInColumn(col, row, &tmpRow);
    newRow = tmpRow;
    d->valueStorage->prevInColumn(col, row, &tmpRow);
    newRow = qMax(newRow, tmpRow);
    if (!newRow)
        return KCCell();
    return KCCell(d->sheet, col, newRow);
}

KCCell CellStorage::prevInRow(int col, int row, Visiting visiting) const
{
    Q_UNUSED(visiting);
    int newCol = 0;
    int tmpCol = 0;
    d->formulaStorage->prevInRow(col, row, &tmpCol);
    newCol = tmpCol;
    d->valueStorage->prevInRow(col, row, &tmpCol);
    newCol = qMax(newCol, tmpCol);
    if (!newCol)
        return KCCell();
    return KCCell(d->sheet, newCol, row);
}

int CellStorage::columns(bool includeStyles) const
{
    int max = 0;
    max = qMax(max, d->commentStorage->usedArea().right());
    max = qMax(max, d->conditionsStorage->usedArea().right());
    max = qMax(max, d->fusionStorage->usedArea().right());
    if (includeStyles) max = qMax(max, d->styleStorage->usedArea().right());
    max = qMax(max, d->validityStorage->usedArea().right());
    max = qMax(max, d->formulaStorage->columns());
    max = qMax(max, d->linkStorage->columns());
    max = qMax(max, d->valueStorage->columns());

    // don't include bindings cause the bindingStorage does only listen to all cells in the sheet.
    //max = qMax(max, d->bindingStorage->usedArea().right());

    return max;
}

int CellStorage::rows(bool includeStyles) const
{
    int max = 0;
    max = qMax(max, d->commentStorage->usedArea().bottom());
    max = qMax(max, d->conditionsStorage->usedArea().bottom());
    max = qMax(max, d->fusionStorage->usedArea().bottom());
    if (includeStyles) max = qMax(max, d->styleStorage->usedArea().bottom());
    max = qMax(max, d->validityStorage->usedArea().bottom());
    max = qMax(max, d->formulaStorage->rows());
    max = qMax(max, d->linkStorage->rows());
    max = qMax(max, d->valueStorage->rows());

    // don't include bindings cause the bindingStorage does only listen to all cells in the sheet.
    //max = qMax(max, d->bindingStorage->usedArea().bottom());

    return max;
}

CellStorage CellStorage::subStorage(const KCRegion& region) const
{
    CellStorage subStorage(d->sheet);
    *subStorage.d->formulaStorage = d->formulaStorage->subStorage(region);
    *subStorage.d->linkStorage = d->linkStorage->subStorage(region);
    *subStorage.d->valueStorage = d->valueStorage->subStorage(region);
    return subStorage;
}

const KCBindingStorage* CellStorage::bindingStorage() const
{
    return d->bindingStorage;
}

const CommentStorage* CellStorage::commentStorage() const
{
    return d->commentStorage;
}

const ConditionsStorage* CellStorage::conditionsStorage() const
{
    return d->conditionsStorage;
}

const FormulaStorage* CellStorage::formulaStorage() const
{
    return d->formulaStorage;
}

const FusionStorage* CellStorage::fusionStorage() const
{
    return d->fusionStorage;
}

const LinkStorage* CellStorage::linkStorage() const
{
    return d->linkStorage;
}

const StyleStorage* CellStorage::styleStorage() const
{
    return d->styleStorage;
}

const ValidityStorage* CellStorage::validityStorage() const
{
    return d->validityStorage;
}

const ValueStorage* CellStorage::valueStorage() const
{
    return d->valueStorage;
}

void CellStorage::startUndoRecording()
{
    // If undoData is not null, the recording wasn't stopped.
    // Should not happen, hence this assertion.
    Q_ASSERT(d->undoData == 0);
    d->undoData = new CellStorageUndoData();
}

void CellStorage::stopUndoRecording(QUndoCommand *parent)
{
    // If undoData is null, the recording wasn't started.
    // Should not happen, hence this assertion.
    Q_ASSERT(d->undoData != 0);
    // append sub-commands to the parent command
    d->createCommand(parent); // needs d->undoData
    for (int i = 0; i < d->undoData->namedAreas.count(); ++i) {
        emit namedAreaRemoved(d->undoData->namedAreas[i].second);
    }
    delete d->undoData;
    d->undoData = 0;
}

void CellStorage::loadConditions(const QList<QPair<QRegion, Conditions> >& conditions)
{
    d->conditionsStorage->load(conditions);
}

void CellStorage::loadStyles(const QList<QPair<QRegion, KCStyle> > &styles)
{
    d->styleStorage->load(styles);
}

void CellStorage::invalidateStyleCache()
{
    d->styleStorage->invalidateCache();
}

int CellStorage::rowRepeat(int row) const
{
    return d->rowRepeatStorage->rowRepeat(row);
}

int CellStorage::firstIdenticalRow(int row) const
{
    return d->rowRepeatStorage->firstIdenticalRow(row);
}

void CellStorage::setRowsRepeated(int row, int count)
{
    d->rowRepeatStorage->setRowRepeat(row, count);
}

#include "CellStorage.moc"
