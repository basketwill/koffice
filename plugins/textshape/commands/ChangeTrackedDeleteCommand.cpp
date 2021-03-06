/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.*/

#include "ChangeTrackedDeleteCommand.h"
#include "TextPasteCommand.h"
#include "ListItemNumberingCommand.h"
#include "ChangeListCommand.h"
#include <KoTextEditor.h>
#include <TextTool.h>
#include <klocale.h>
#include <KChangeTracker.h>
#include <KChangeTrackerElement.h>
#include <KTextDocument.h>
#include <KTextDocumentLayout.h>
#include <KInlineTextObjectManager.h>
#include <KAction>
#include <KTextAnchor.h>
#include <KDeleteChangeMarker.h>
#include <KCanvasBase.h>
#include <KShapeController.h>
#include <KoList.h>
#include <KParagraphStyle.h>
#include <KTextOdfSaveHelper.h>
#include <KTextDrag.h>
#include <KOdf.h>
#include <KDocumentRdfBase.h>
#include <QTextDocumentFragment>
#include <QUndoCommand>

#include <KDebug>
//#include <iostream>
#include <QDebug>
#include <QWeakPointer>

//A convenience function to get a ListIdType from a format

static KListStyle::ListIdType ListId(const QTextListFormat &format)
{
    KListStyle::ListIdType listId;

    if (sizeof(KListStyle::ListIdType) == sizeof(uint))
        listId = format.property(KListStyle::ListId).toUInt();
    else
        listId = format.property(KListStyle::ListId).toULongLong();

    return listId;
}

using namespace std;
ChangeTrackedDeleteCommand::ChangeTrackedDeleteCommand(DeleteMode mode, TextTool *tool, QUndoCommand *parent) :
    TextCommandBase (parent),
    m_tool(tool),
    m_first(true),
    m_undone(false),
    m_canMerge(true),
    m_mode(mode),
    m_removedElements()
{
      setText(i18n("Deletion"));
}

void ChangeTrackedDeleteCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);

    QTextDocument *document = m_tool->m_textEditor.data()->document();
    KTextDocument(document).changeTracker()->elementById(m_addedChangeElement)->setValid(false);
    foreach (int changeId, m_removedElements) {
      KTextDocument(document).changeTracker()->elementById(changeId)->setValid(true);
    }
    updateListChanges();
    m_undone = true;
}

void ChangeTrackedDeleteCommand::redo()
{
    m_undone = false;
    if (!m_first) {
        TextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
        QTextDocument *document = m_tool->m_textEditor.data()->document();
        KTextDocument(document).changeTracker()->elementById(m_addedChangeElement)->setValid(true);
        foreach (int changeId, m_removedElements) {
          KTextDocument(document).changeTracker()->elementById(changeId)->setValid(false);
        }
    } else {
        m_first = false;
        m_tool->m_textEditor.data()->beginEditBlock();
        if(m_mode == PreviousChar)
            deletePreviousChar();
        else
            deleteChar();
        m_tool->m_textEditor.data()->endEditBlock();
    }
}

void ChangeTrackedDeleteCommand::deleteChar()
{
    QTextCursor *caret = m_tool->m_textEditor.data()->cursor();

    if (caret->atEnd() && !caret->hasSelection())
        return;

    if (!caret->hasSelection())
        caret->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

    deleteSelection(*caret);
}

void ChangeTrackedDeleteCommand::deletePreviousChar()
{
    QTextCursor *caret = m_tool->m_textEditor.data()->cursor();

    if (caret->atStart() && !caret->hasSelection())
        return;

    if (!caret->hasSelection() && caret->block().textList() && (caret->position() == caret->block().position())) {
        handleListItemDelete(*caret);
        return;
    }

    if (!caret->hasSelection()) 
        caret->movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

    deleteSelection(*caret);
}

void ChangeTrackedDeleteCommand::handleListItemDelete(QTextCursor &selection)
{
    m_canMerge = false;
    QTextDocument *document = selection.document();

    bool numberedListItem = false;
    if (!selection.blockFormat().boolProperty(KParagraphStyle::UnnumberedListItem))
         numberedListItem = true;      
 
    // Mark the complete list-item
    QTextBlock block = document->findBlock(selection.position());
    selection.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, (block.length() - 1));

    // Copy the marked item
    int from = selection.anchor();
    int to = selection.position();
    KTextOdfSaveHelper saveHelper(m_tool->m_textShapeData, from, to);
    KTextDrag drag;

    if (KDocumentRdfBase *rdf = KDocumentRdfBase::fromResourceManager(m_tool->canvas())) {
        saveHelper.setRdfModel(rdf->model());
    }
    drag.setOdf(KOdf::mimeType(KOdf::TextDocument), saveHelper);
    QTextDocumentFragment fragment = selection.selection();
    drag.setData("text/html", fragment.toHtml("utf-8").toUtf8());
    drag.setData("text/plain", fragment.toPlainText().toUtf8());
    drag.addToClipboard();

    // Delete the marked section
    selection.setPosition(selection.anchor() -1);
    selection.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, block.length());
    deleteSelection(selection);
    // Insert a new Block and paste the copied contents
    selection.insertBlock();
    // Mark it as inserted content
    QTextCharFormat format = selection.charFormat();
    m_tool->m_textEditor.data()->registerTrackedChange(selection, KOdfGenericChange::InsertChange, i18n("Key Press"), format, format, false);
    //Paste the selected text
    TextPasteCommand *pasteCommand = new TextPasteCommand(QClipboard::Clipboard, m_tool, this);
    pasteCommand->redo();

    // Convert it into a un-numbered list-item or a paragraph
    if (numberedListItem) {
        ListItemNumberingCommand *changeNumberingCommand = new ListItemNumberingCommand(selection.block(), false, this);
        changeNumberingCommand->redo();
    } else {
        ChangeListCommand *changeListCommand = new ChangeListCommand(selection, KListStyle::None, 0, 
                                                                     ChangeListCommand::ModifyExistingList | ChangeListCommand::MergeWithAdjacentList, 
                                                                     this);
        changeListCommand->redo();
    }
    selection.setPosition(selection.block().position());
}

void ChangeTrackedDeleteCommand::deleteSelection(QTextCursor &selection)
{
    QTextDocument *document = m_tool->m_textEditor.data()->document();
    KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());

    QTextCursor checker = QTextCursor(selection);
    KDeleteChangeMarker *deleteChangemarker = 0;
    KDeleteChangeMarker *testMarker;

    bool backwards = (checker.anchor() > checker.position());
    int selectionBegin = qMin(checker.anchor(), checker.position());
    int selectionEnd = qMax(checker.anchor(), checker.position());
    int changeId;

    QList<KShape *> shapesInSelection;

    checker.setPosition(selectionBegin);

    while ((checker.position() < selectionEnd) && (!checker.atEnd())) {
        QChar charAtPos = document->characterAt(checker.position());
        checker.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        if (layout->inlineTextObjectManager()->inlineTextObject(checker) && charAtPos == QChar::ObjectReplacementCharacter) {
            testMarker = dynamic_cast<KDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(checker));
            if (testMarker) {
                QTextDocumentFragment inter = KTextDocument(document).changeTracker()->elementById(testMarker->changeId())->deleteData();
                if (!KTextDocument(document).changeTracker()->displayChanges()) {
                    KChangeTracker::insertDeleteFragment(checker, testMarker);
                    selectionEnd = selectionEnd + KChangeTracker::fragmentLength(inter);
                }
                checker.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, KChangeTracker::fragmentLength(inter));
                KTextDocument(document).changeTracker()->elementById(testMarker->changeId())->setValid(false);
                m_removedElements.push_back(testMarker->changeId());
           } else {
                KTextAnchor *anchor = dynamic_cast<KTextAnchor *>(layout->inlineTextObjectManager()->inlineTextObject(checker));
                if (anchor)
                    shapesInSelection.push_back(anchor->shape());
           } 
        }
        checker.setPosition(checker.position());
    }

    checker.setPosition(selectionBegin);

    if (!KTextDocument(document).changeTracker()->displayChanges()) {
        QChar charAtPos = document->characterAt(checker.position() - 1);
        if (layout->inlineTextObjectManager()->inlineTextObject(checker) && charAtPos == QChar::ObjectReplacementCharacter) {
            testMarker = dynamic_cast<KDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(checker));
            if (testMarker) {
                KChangeTracker::insertDeleteFragment(checker, testMarker);
                QTextDocumentFragment prevFragment = KTextDocument(checker.document()).changeTracker()->elementById(testMarker->changeId())->deleteData();
                selectionBegin += KChangeTracker::fragmentLength(prevFragment);
                selectionEnd += KChangeTracker::fragmentLength(prevFragment);
            }
        }
    }

    if (KTextDocument(document).changeTracker()->containsInlineChanges(checker.charFormat())) {
        int changeId = checker.charFormat().property(KCharacterStyle::ChangeTrackerId).toInt();
        if (KTextDocument(document).changeTracker()->elementById(changeId)->changeType() == KOdfGenericChange::DeleteChange) {
            QTextDocumentFragment prefix =  KTextDocument(document).changeTracker()->elementById(changeId)->deleteData();
            selectionBegin -= (KChangeTracker::fragmentLength(prefix) + 1 );
            KTextDocument(document).changeTracker()->elementById(changeId)->setValid(false);
            m_removedElements.push_back(changeId);
        }
    }

    checker.setPosition(selectionEnd);
    if (!checker.atEnd()) {
        QChar charAtPos = document->characterAt(checker.position());
        checker.movePosition(QTextCursor::NextCharacter);
        if (layout->inlineTextObjectManager()->inlineTextObject(checker) && charAtPos == QChar::ObjectReplacementCharacter) {
            testMarker = dynamic_cast<KDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(checker));
            if (testMarker) {
                QTextDocumentFragment sufix =  KTextDocument(document).changeTracker()->elementById(testMarker->changeId())->deleteData();
                if (!KTextDocument(document).changeTracker()->displayChanges())
                    KChangeTracker::insertDeleteFragment(checker, testMarker);
                selectionEnd += KChangeTracker::fragmentLength(sufix) + 1;
                KTextDocument(document).changeTracker()->elementById(testMarker->changeId())->setValid(false);
                m_removedElements.push_back(testMarker->changeId());
            }
        }
    }

    selection.setPosition(selectionBegin);
    selection.setPosition(selectionEnd, QTextCursor::KeepAnchor);
    QTextDocumentFragment deletedFragment;
    changeId = KTextDocument(document).changeTracker()->deleteChangeId(i18n("Deletion"), deletedFragment, 0);
    KChangeTrackerElement *element = KTextDocument(document).changeTracker()->elementById(changeId);
    deleteChangemarker = new KDeleteChangeMarker(KTextDocument(document).changeTracker());
    deleteChangemarker->setChangeId(changeId);
    element->setDeleteChangeMarker(deleteChangemarker);

    QTextCharFormat charFormat;
    charFormat.setProperty(KCharacterStyle::ChangeTrackerId, changeId);
    selection.mergeCharFormat(charFormat);

    deletedFragment = KChangeTracker::generateDeleteFragment(selection, deleteChangemarker);
    element->setDeleteData(deletedFragment);

    //Store the position and length. Will be used in updateListChanges()
    m_position = (selection.anchor() < selection.position()) ? selection.anchor():selection.position();
    m_length = qAbs(selection.anchor() - selection.position());

    updateListIds(selection);
    layout->inlineTextObjectManager()->insertInlineObject(selection, deleteChangemarker);

    m_addedChangeElement = changeId;
    
    //Insert the deleted data again after the marker with the charformat set to the change-id
    if (KTextDocument(document).changeTracker()->displayChanges()) {
        int startPosition = selection.position();
        KChangeTracker::insertDeleteFragment(selection, deleteChangemarker);
        QTextCursor tempCursor(selection);
        tempCursor.setPosition(startPosition);
        tempCursor.setPosition(selection.position(), QTextCursor::KeepAnchor);
        //tempCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, fragmentLength(deletedFragment));
        updateListIds(tempCursor);
        if (backwards)
            selection.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, KChangeTracker::fragmentLength(deletedFragment) + 1);
    } else {
        if (backwards)
            selection.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor,1);

        foreach (KShape *shape, shapesInSelection) {
            QUndoCommand *shapeDeleteCommand = m_tool->canvas()->shapeController()->removeShape(shape, this);
            shapeDeleteCommand->redo();
            m_canMerge = false;
        }
    }
}

int ChangeTrackedDeleteCommand::id() const
{
    return 98765;
}

bool ChangeTrackedDeleteCommand::mergeWith( const QUndoCommand *command)
{
    class UndoTextCommand : public QUndoCommand
    {
    public:
        UndoTextCommand(QTextDocument *document, QUndoCommand *parent = 0)
        : QUndoCommand(i18n("Text"), parent),
        m_document(document)
        {}

        void undo() {
            QTextDocument *doc = m_document.data();
            if (doc)
                doc->undo(KTextDocument(doc).textEditor()->cursor());
        }

        void redo() {
            QTextDocument *doc = m_document.data();
            if (doc)
                doc->redo(KTextDocument(doc).textEditor()->cursor());
        }

        QWeakPointer<QTextDocument> m_document;
    };

    if (command->id() != id())
        return false;

    ChangeTrackedDeleteCommand *other = const_cast<ChangeTrackedDeleteCommand *>(static_cast<const ChangeTrackedDeleteCommand *>(command));

    if (other->m_canMerge == false)
        return false;

    if (other->m_removedElements.contains(m_addedChangeElement)) {
        removeChangeElement(m_addedChangeElement);
        other->m_removedElements.removeAll(m_addedChangeElement);
        m_addedChangeElement = other->m_addedChangeElement;

        m_removedElements += other->m_removedElements;
        other->m_removedElements.clear();

        m_newListIds = other->m_newListIds;

        m_position = other->m_position;
        m_length = other->m_length;

        for(int i=0; i < command->childCount(); i++)
            new UndoTextCommand(m_tool->m_textEditor.data()->document(), this);

        return true;
    }
    return false;
}

void ChangeTrackedDeleteCommand::updateListIds(QTextCursor &cursor)
{
    m_newListIds.clear();
    QTextDocument *document = m_tool->m_textEditor.data()->document();
    QTextCursor tempCursor(document);
    QTextBlock startBlock = document->findBlock(cursor.anchor());
    QTextBlock endBlock = document->findBlock(cursor.position());
    QTextList *currentList;

    for (QTextBlock currentBlock = startBlock; currentBlock != endBlock.next(); currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        currentList = tempCursor.currentList();
        if (currentList) {
            KListStyle::ListIdType listId = ListId(currentList->format());
            m_newListIds.push_back(listId);
        }
    }
}
void ChangeTrackedDeleteCommand::updateListChanges()
{
    QTextDocument *document = m_tool->m_textEditor.data()->document();
    QTextCursor tempCursor(document);
    QTextBlock startBlock = document->findBlock(m_position);
    QTextBlock endBlock = document->findBlock(m_position + m_length);
    QTextList *currentList;
    int newListIdsCounter = 0;

    for (QTextBlock currentBlock = startBlock; currentBlock != endBlock.next(); currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        currentList = tempCursor.currentList();
        if (currentList) {
            KListStyle::ListIdType listId = m_newListIds[newListIdsCounter];
            if (!KTextDocument(document).list(currentBlock)) {
                KoList *list = KTextDocument(document).list(listId);
                if (list)
                    list->updateStoredList(currentBlock);
            }
            newListIdsCounter++;
        }
    }
}

ChangeTrackedDeleteCommand::~ChangeTrackedDeleteCommand()
{
    if (m_undone) {
        removeChangeElement(m_addedChangeElement);
    } else {
        foreach (int changeId, m_removedElements) {
           removeChangeElement(changeId);
        }
    }
}

void ChangeTrackedDeleteCommand::removeChangeElement(int changeId)
{
    QTextDocument *document = m_tool->m_textEditor.data()->document();
    KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(document->documentLayout());
    KChangeTrackerElement *element = KTextDocument(document).changeTracker()->elementById(changeId);
    KDeleteChangeMarker *marker = element->deleteChangeMarker();
    layout->inlineTextObjectManager()->removeInlineObject(marker);
    KTextDocument(document).changeTracker()->removeById(changeId);
}
