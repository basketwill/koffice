/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Jean Nicolas Artaud <jean.nicolas.artaud@kogmbh.com>
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
 * Boston, MA 02110-1301, USA.
 */

#include "ToCGenerator.h"

#include <KParagraphStyle.h>
#include <KTextDocumentLayout.h>
#include <KTextShapeData.h>
#include <KTextPage.h>
#include <KShape.h>
#include <KTextDocument.h>
#include <KTextBlockData.h>
#include <KStyleManager.h>

#include <QTextFrame>
#include <QTimer>
#include <KDebug>

ToCGenerator::ToCGenerator(QTextFrame *tocFrame)
    : QObject(tocFrame),
    m_state(NeverGeneratedState),
    m_ToCFrame(tocFrame)
{
    Q_ASSERT(tocFrame);
/*
    // do a generate right now to have a ToC with placeholder numbers.
    QTimer::singleShot(0, this, SLOT(documentLayoutFinished()));

    // disabled for now as this requires us to update the list items in 'update' too
*/
}

void ToCGenerator::documentLayoutFinished()
{
    switch (m_state) {
    case DirtyState:
    case NeverGeneratedState:
        generate();
        m_state = WithoutPageNumbersState;
        break;
    case WithoutPageNumbersState:
        update();
        m_state = GeneratedState;
    case GeneratedState:
        break;
    };
}

void ToCGenerator::generate()
{
    // Add a frame to the current layout
    QTextFrameFormat tocFormat = m_ToCFrame->frameFormat();
    QTextDocument *doc = m_ToCFrame->document();
    KTextDocument koDocument(doc);
    KStyleManager *styleManager = koDocument.styleManager();
    QList<KParagraphStyle *> paragStyle;

    QTextCursor cursor = m_ToCFrame->lastCursorPosition();
    cursor.setPosition(m_ToCFrame->firstPosition(), QTextCursor::KeepAnchor);
    cursor.beginEditBlock();
    // Add the title
    cursor.insertText("Table of Contents"); // TODO i18n
    KParagraphStyle *titleStyle = styleManager->paragraphStyle("Contents Heading"); // TODO don't hardcode this!
    if (titleStyle) {
        QTextBlock block = cursor.block();
        titleStyle->applyStyle(block);
    }
    cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());

    // looks for blocks to add in the ToC
    QTextBlock block = doc->begin();
    while (block.isValid()) {
        int outlineLevel = block.blockFormat().intProperty(KParagraphStyle::OutlineLevel);

        if (outlineLevel > 0) {
            cursor.insertBlock();
            KParagraphStyle *currentStyle = styleManager->paragraphStyle("Contents "+QString::number(outlineLevel));
            if (currentStyle == 0) {
                currentStyle = new KParagraphStyle();
                currentStyle->setName("Contents " + QString::number(outlineLevel));
                currentStyle->setParent(styleManager->paragraphStyle("Standard"));

                currentStyle->setLeftMargin(8 * (outlineLevel-1));

                QList<KOdfText::Tab> tabList;
                struct KOdfText::Tab aTab;
                aTab.type = QTextOption::RightTab;
                aTab.leaderText = '.';
                aTab.position = 490 - outlineLevel * 8;
                tabList.append(aTab);
                currentStyle->setTabPositions(tabList);

                styleManager->add(currentStyle);
            }
            QTextBlock tocBlock = cursor.block();
            currentStyle->applyStyle(tocBlock);

            KTextBlockData *bd = dynamic_cast<KTextBlockData *>(block.userData());
            if (bd && bd->hasCounterData()) {
                // TODO instead of using plain text we likely want to use a text list
                // which makes all paragraphs be properly aligned
                cursor.insertText(bd->counterText());
                cursor.insertText(QLatin1String(" "));
            }
            // Wrong page number, it will be corrected in the update
            // note that the fact that I use 4 chars is reused in the update method!
            cursor.insertText(block.text() + "\t0000");
            m_originalBlocksInToc << block;
        }
        block = block.next();
    }
    cursor.endEditBlock();
}

void ToCGenerator::update()
{
    // update the text for the TOC entries with the proper page numbers
    QTextDocument *doc = m_ToCFrame->document();
    KTextDocument koDocument(doc);
    KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(doc->documentLayout());

    QTextCursor cursor = m_ToCFrame->firstCursorPosition();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, 2);// past our header
    foreach (const QTextBlock &block, m_originalBlocksInToc) {
        KShape *shape = layout->shapeForPosition(block.position());
        cursor.movePosition(QTextCursor::NextBlock);
        Q_ASSERT(shape);
        if (shape == 0)
            continue;
        KTextShapeData *shapeData = qobject_cast<KTextShapeData *>(shape->userData());
        Q_ASSERT(shapeData);
        if (shapeData == 0 || shapeData->page() == 0)
            continue;
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 4);
        Q_ASSERT(cursor.position() < m_ToCFrame->lastPosition());
        cursor.insertText(QString::number(shapeData->page()->pageNumber()));
        cursor.movePosition(QTextCursor::NextBlock);
    }
    cursor.endEditBlock();
    m_originalBlocksInToc.clear();
}

#include <ToCGenerator.moc>
