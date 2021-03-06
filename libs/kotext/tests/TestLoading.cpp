/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
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

#include "TestLoading.h"

#include <QtGui/QTextDocument>
#include <QtGui/QApplication>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtScript>
#include <QtTest/QtTest>

#include <KDE/KDebug>
#include <KDE/KTextEdit>
#include <KStyleManager.h>
#include <KOdfStylesReader.h>
#include <KOdfStore.h>
#include <KTextLoader.h>
#include <KXmlReader.h>
#include <KOdfStoreReader.h>
#include <KOdfWriteStore.h>
#include <KTemporaryFile>
#include <KOdfStorageDevice.h>
#include <KXmlWriter.h>
#include <KTextShapeData.h>
#include <KShapeLoadingContext.h>
#include <KOdfLoadingContext.h>
#include <KShapeSavingContext.h>
#include <KOdfGenericStyles.h>
#include <KOdfXmlNS.h>
#include <kcomponentdata.h>
#include <KTextDebug_p.h>
#include <KListStyle.h>
#include <KTableStyle.h>
#include <KTableCellStyle.h>
#include <KTextDocumentLayout.h>
#include <KCharacterStyle.h>
#include <KParagraphStyle.h>
#include <KOdfText.h>
#include <KOdfEmbeddedDocumentSaver.h>
#include <KInlineTextObjectManager.h>
#include <KTextSharedLoadingData.h>
#include <KTextSharedSavingData.h>
#include <KTextDocument.h>
#include <kstandarddirs.h>

#include <KOdfGenericChanges.h>
#include <KChangeTracker.h>

typedef KOdfText::Tab KoTextTab;
// because in a QtScript, I don't seem to be able to use a namespaced type

static void showDocument(QTextDocument *document)
{
    KTextEdit *textEdit = new KTextEdit;
    textEdit->setDocument(document);
    textEdit->show();
    qApp->exec();
    delete textEdit;
}

// Functions that help compare two QTextDocuments
bool TestLoading::compareFragments(const QTextFragment &actualFragment, const QTextFragment &expectedFragment)
{
    if (actualFragment.text() != expectedFragment.text()) {
        qDebug() << "compareFragments: text not equal " << actualFragment.text() << expectedFragment.text();
        return false;
    }
    if (actualFragment.position() != expectedFragment.position()) {
        qDebug() << "compareFragments: position not equal " << actualFragment.position() << expectedFragment.position();
        return false;
    }

    QTextCharFormat actualFormat = actualFragment.charFormat();
    QTextImageFormat actualImageFormat = actualFormat.toImageFormat();

    QTextCharFormat expectedFormat = expectedFragment.charFormat();
    QTextImageFormat expectedImageFormat = expectedFormat.toImageFormat();

    if (actualImageFormat.isValid()) {
        if (!expectedImageFormat.isValid()) {
            qDebug() << "compareFragments: actualDocument has unexpected image at " << actualFragment.text();
            return false;
        }
        return true; // FIXME: Compare the image formats
    } else {
        if (expectedImageFormat.isValid()) {
            qDebug() << "compareFragment: expecting image in actualDocument at " << actualFragment.text();
            return false;
        }
    }

    // this should really be actualFormat.properties() == expectedFormat.properties()
    bool equal = actualFormat.font() == expectedFormat.font()
                 && actualFormat.property(KCharacterStyle::FontCharset).toString()
                 == expectedFormat.property(KCharacterStyle::FontCharset).toString()
                 && actualFormat.font().styleHint() == expectedFormat.font().styleHint() // test this explicity since font == ignores it
                 && actualFormat.foreground() == expectedFormat.foreground()
                 && actualFormat.background() == expectedFormat.background()
                 && actualFormat.fontCapitalization() == expectedFormat.fontCapitalization()
                 && actualFormat.underlineColor() == expectedFormat.underlineColor()
                 && actualFormat.textOutline() == expectedFormat.textOutline()
                 && actualFormat.property(KCharacterStyle::UnderlineStyle).toInt()
                 == expectedFormat.property(KCharacterStyle::UnderlineStyle).toInt()
                 && actualFormat.property(KCharacterStyle::UnderlineMode).toInt()
                 == expectedFormat.property(KCharacterStyle::UnderlineMode).toInt()
                 && actualFormat.property(KCharacterStyle::UnderlineWeight).toInt()
                 == expectedFormat.property(KCharacterStyle::UnderlineWeight).toInt()
                 && actualFormat.property(KCharacterStyle::UnderlineWidth).toDouble()
                 == expectedFormat.property(KCharacterStyle::UnderlineWidth).toDouble()
                 && qvariant_cast<QColor>(actualFormat.property(KCharacterStyle::StrikeOutColor))
                 == qvariant_cast<QColor>(expectedFormat.property(KCharacterStyle::StrikeOutColor))
                 && actualFormat.property(KCharacterStyle::StrikeOutStyle).toInt()
                 == expectedFormat.property(KCharacterStyle::StrikeOutStyle).toInt()
                 && actualFormat.property(KCharacterStyle::StrikeOutType).toInt()
                 == expectedFormat.property(KCharacterStyle::StrikeOutType).toInt()
                 && actualFormat.property(KCharacterStyle::StrikeOutMode).toInt()
                 == expectedFormat.property(KCharacterStyle::StrikeOutMode).toInt()
                 && actualFormat.property(KCharacterStyle::StrikeOutWeight).toInt()
                 == expectedFormat.property(KCharacterStyle::StrikeOutWeight).toInt()
                 && actualFormat.property(KCharacterStyle::StrikeOutWidth).toDouble()
                 == expectedFormat.property(KCharacterStyle::StrikeOutWidth).toDouble()
                 && actualFormat.property(KCharacterStyle::StrikeOutText).toString()
                 == expectedFormat.property(KCharacterStyle::StrikeOutText).toString()
                 && actualFormat.property(KCharacterStyle::Country).toString()
                 == expectedFormat.property(KCharacterStyle::Country).toString()
                 && actualFormat.property(KCharacterStyle::Language).toString()
                 == expectedFormat.property(KCharacterStyle::Language).toString()
                 && actualFormat.verticalAlignment() == expectedFormat.verticalAlignment(); // FIXME: Compare other properties

    if (!equal) {
        qDebug() << "compareFragments: properties mismatch at " << actualFragment.text() << endl
        << "actual:  " << KTextDebug::textAttributes(actualFormat) << endl
        << "expected:" << KTextDebug::textAttributes(expectedFormat);
    }

    return equal;
}

bool TestLoading::compareTabProperties(QVariant actualTabs, QVariant expectedTabs)
{
    QList<QVariant> actualTabList = qvariant_cast<QList<QVariant> >(actualTabs);
    QList<QVariant> expectedTabList = qvariant_cast<QList<QVariant> >(expectedTabs);
    if (actualTabList.count() != expectedTabList.count())
        return false;
    for (int i = 0; i < actualTabList.count(); i++) {
        KOdfText::Tab actualTab = actualTabList[i].value<KOdfText::Tab>();
        KOdfText::Tab expectedTab = expectedTabList[i].value<KOdfText::Tab>();
        if (actualTab.position != expectedTab.position
                || actualTab.type != expectedTab.type
                || actualTab.delimiter != expectedTab.delimiter
                || actualTab.leaderType != expectedTab.leaderType
                || actualTab.leaderStyle != expectedTab.leaderStyle
                || actualTab.leaderColor != expectedTab.leaderColor
                || actualTab.leaderWeight != expectedTab.leaderWeight
                || actualTab.leaderWidth != expectedTab.leaderWidth
                || actualTab.leaderText != expectedTab.leaderText
           ) {
            return false;
        }
    }
    return true;
}

bool TestLoading::compareBlockFormats(const QTextBlockFormat &actualFormat, const QTextBlockFormat &expectedFormat)
{
    if (actualFormat.background() != expectedFormat.background()
            || actualFormat.alignment() != expectedFormat.alignment()
            || actualFormat.indent() != expectedFormat.indent()
            || actualFormat.textIndent() != expectedFormat.textIndent()
            || actualFormat.foreground() != expectedFormat.foreground()) {
        return false;
    }
    // check custom properties
    const QMap<int, QVariant> actualProperty = actualFormat.properties();
    const QMap<int, QVariant> expectedProperty = expectedFormat.properties();
    QList<int> allPropertyIds = actualProperty.keys();
    allPropertyIds << expectedProperty.keys();
    bool match = true;
    foreach(int id, allPropertyIds) {
        QString key, value;
        switch (id) {
        case KParagraphStyle::UnnumberedListItem:
        case KParagraphStyle::IsListHeader:
            if (actualProperty[id].toInt() != static_cast<int>(expectedProperty[id].toBool()))
                match = false;
            break;
        case KParagraphStyle::AutoTextIndent:
        case KParagraphStyle::DropCaps:
        case KParagraphStyle::DropCapsLines:
        case KParagraphStyle::DropCapsLength:
        case KParagraphStyle::ListLevel:
            if (actualProperty[id].toInt() != expectedProperty[id].toInt())
                match = false;
            break;
        case KParagraphStyle::LeftBorderWidth:
        case KParagraphStyle::TopBorderWidth:
        case KParagraphStyle::RightBorderWidth:
        case KParagraphStyle::BottomBorderWidth:
        case KParagraphStyle::LeftInnerBorderWidth:
        case KParagraphStyle::TopInnerBorderWidth:
        case KParagraphStyle::RightInnerBorderWidth:
        case KParagraphStyle::BottomInnerBorderWidth:
        case KParagraphStyle::LeftBorderSpacing:
        case KParagraphStyle::TopBorderSpacing:
        case KParagraphStyle::RightBorderSpacing:
        case KParagraphStyle::BottomBorderSpacing:
        case KParagraphStyle::TabStopDistance:
        case KParagraphStyle::DropCapsDistance:
        case QTextFormat::BlockLeftMargin:
        case QTextFormat::BlockRightMargin:
        case QTextFormat::BlockTopMargin:
        case QTextFormat::BlockBottomMargin:
            if (abs(actualProperty[id].toDouble() - expectedProperty[id].toDouble()) > 1e-10)
                // just checking if it's equal results in floating point errors
                match = false;
            break;
        case KParagraphStyle::TabPositions:
            if (!compareTabProperties(actualProperty[id], expectedProperty[id]))
                match = false;
            break;
        }
        if (!match) {
            qDebug() << "Actual:  " << KTextDebug::paraAttributes(actualFormat);
            qDebug() << "Expected:" << KTextDebug::paraAttributes(expectedFormat);
            qDebug() << "At index: QTextFormat::UserProperty + " << id - QTextFormat::UserProperty;
            return false;
        }
    }
    return match;
}

bool TestLoading::compareListFormats(const QTextListFormat &actualFormat, const QTextListFormat &expectedFormat)
{
    QMap<int, QVariant> actualProperties = actualFormat.properties();
    actualProperties.remove(KListStyle::StyleId);
    actualProperties.remove(KListStyle::MinimumDistance);
    actualProperties.remove(KListStyle::ListId);
    actualProperties.remove(QTextFormat::ObjectIndex);

    QMap<int, QVariant> expectedProperties = expectedFormat.properties();
    expectedProperties.remove(KListStyle::StyleId);
    expectedProperties.remove(KListStyle::MinimumDistance);
    expectedProperties.remove(KListStyle::ListId);
    expectedProperties.remove(QTextFormat::ObjectIndex);


    struct Enum {
        static QString resolve(int key) {
            QMetaObject meta;
            if (key >= QTextFormat::UserProperty)
                meta = KListStyle::staticMetaObject;
            else
                meta = QTextFormat::staticMetaObject;
            QMetaEnum me = meta.enumerator(meta.indexOfEnumerator("Property"));
            QString answer(me.valueToKey(key));
            if (answer.isEmpty())
                answer = QString::number(key);
            return answer;
        }
    };

    for (QMap<int, QVariant>::const_iterator prop = expectedProperties.constBegin(); prop != expectedProperties.constEnd(); ++prop) {
	int key = prop.key();
        if (actualProperties.contains(key)) {
            if (actualProperties.value(key) != prop.value()) {
                qDebug() << "properties don't match;" << Enum::resolve(key)
                    << actualProperties.value(key) << prop.value();
            }
        } else {
            qDebug() << "Expected but missing property;" << Enum::resolve(key);
        }
    }
    for (QMap<int, QVariant>::const_iterator prop = actualProperties.constBegin(); prop != actualProperties.constEnd(); ++prop) {
	int key = prop.key();
        if (!expectedProperties.contains(key)) {
            qDebug() << "found extra property;" << Enum::resolve(key);
        }
    }

    return actualProperties == expectedProperties;
}

bool TestLoading::compareBlocks(const QTextBlock &actualBlock, const QTextBlock &expectedBlock)
{
    QTextList *actualList = actualBlock.textList();
    QTextList *expectedList = expectedBlock.textList();

    if (actualList) {
        if (!expectedList) {
            qDebug() << "compareBlocks: Unexpected list in actualDocument at " << actualBlock.text() << expectedBlock.text();
            return false;
        }
        if (!compareListFormats(actualList->format(), expectedList->format())
                || (actualList->itemNumber(actualBlock) != expectedList->itemNumber(expectedBlock))) {
            qDebug() << "compareBlocks: list properties mismatch at " << actualBlock.text() << endl
            << "actual:  " << KTextDebug::listAttributes(actualList->format())
            << actualList->itemNumber(actualBlock) << endl
            << "expected:" << KTextDebug::listAttributes(expectedList->format())
            << expectedList->itemNumber(expectedBlock);
            return false;
        }
    } else {
        if (expectedList) {
            qDebug() << "compareBlock: Expecting  list in actualDocument at " << actualBlock.text();
            return false;
        }
    }
    // this should really be actualBlock.blockFormat().properties() == expectedBlock.blockFormat().properties()
    QTextBlockFormat actualFormat = actualBlock.blockFormat();
    QTextBlockFormat expectedFormat = expectedBlock.blockFormat();
    if (!compareBlockFormats(actualFormat, expectedFormat)) {
        qDebug() << "compareBlock: block properties mismatch at " << actualBlock.text();
        return false;
    }

    QTextBlock::Iterator actualIterator = actualBlock.begin();
    QTextBlock::Iterator expectedIterator = expectedBlock.begin();
    for (; !actualIterator.atEnd() && !expectedIterator.atEnd(); ++actualIterator, ++expectedIterator) {
        QTextFragment actualFragment = actualIterator.fragment();
        QTextFragment expectedFragment = expectedIterator.fragment();
        if (actualFragment.isValid()) {
            if (!expectedFragment.isValid()) {
                qDebug() << "compareBlock: Unexpected fragment in actualDocument at " << actualFragment.text();
                return false;
            }

            if (!compareFragments(actualFragment, expectedFragment))
                return false;

        } else {
            if (expectedFragment.isValid()) {
                qDebug() << "compareBlock: Expecting fragment in actualDocument at " << actualFragment.text();
                return false;
            }
        }
    }

    bool equal = actualIterator.atEnd() == expectedIterator.atEnd();
    if (!equal) {
        if (actualIterator.atEnd())
            qDebug() << "compareBlock: expected more text after block length:"
                << actualBlock.length() << ", expected:" << expectedBlock.length()
                << "at position:" << actualBlock.position();
        else
            qDebug() << "compareBlock: more text than expected in block length:"
                << actualBlock.length() << ", expected:" << expectedBlock.length()
                << "at position:" << actualBlock.position();
    }

    return equal;
}

bool TestLoading::compareTableCellFormats(QTextTableCellFormat &actualFormat, QTextTableCellFormat &expectedFormat)
{
    if (actualFormat.background() != expectedFormat.background()
            || actualFormat.leftPadding() != expectedFormat.leftPadding()
            || actualFormat.rightPadding() != expectedFormat.rightPadding()
            || actualFormat.topPadding() != expectedFormat.topPadding()
            || actualFormat.bottomPadding() != expectedFormat.bottomPadding()) {
        qDebug() << "Background or Padding mismatch";
        qDebug() << "Actual Background: " << actualFormat.background();
        qDebug() << "Expected background: " << expectedFormat.background();
        qDebug() << "Actual Left Padding: " << actualFormat.leftPadding();
        qDebug() << "Expected Left Padding: " << expectedFormat.leftPadding();
        qDebug() << "Actual Right Padding: " << actualFormat.rightPadding();
        qDebug() << "Expected Right Padding: " << expectedFormat.rightPadding();
        qDebug() << "Actual Top Padding: " << actualFormat.topPadding();
        qDebug() << "Expected Top Padding: " << expectedFormat.topPadding();
        qDebug() << "Actual Bottom Padding: " << actualFormat.bottomPadding();
        qDebug() << "Expected Bottom Padding: " << expectedFormat.bottomPadding();
        return false;
    }

    // check custom properties
    const QMap<int, QVariant> actualProperty = actualFormat.properties();
    const QMap<int, QVariant> expectedProperty = expectedFormat.properties();
    QList<int> allPropertyIds = actualProperty.keys();
    allPropertyIds << expectedProperty.keys();
    bool match = true;
    foreach(int id, allPropertyIds) {
        QString key, value;
        switch (id) {
        // double properties
        case KTableCellStyle::LeftBorderSpacing:
        case KTableCellStyle::RightBorderSpacing:
        case KTableCellStyle::TopBorderSpacing:
        case KTableCellStyle::BottomBorderSpacing:
            if (abs(actualProperty[id].toDouble() - expectedProperty[id].toDouble()) > 1e-10) {
                qDebug() << "Cell Border Spacing Mismatch";
                qDebug() << "Expected Spacing: " << expectedProperty[id].toDouble();
                qDebug() << "Actual Spacing: " << actualProperty[id].toDouble();
                match = false;
            }
            break;
        // string properties
        case KTableCellStyle::MasterPageName:
            if (actualProperty[id].toString() != expectedProperty[id].toString())
                match = false;
            break;
        // pen properties
        case KTableCellStyle::LeftBorderOuterPen:
        case KTableCellStyle::LeftBorderInnerPen:
        case KTableCellStyle::RightBorderOuterPen:
        case KTableCellStyle::RightBorderInnerPen:
        case KTableCellStyle::TopBorderOuterPen:
        case KTableCellStyle::TopBorderInnerPen:
        case KTableCellStyle::BottomBorderOuterPen:
        case KTableCellStyle::BottomBorderInnerPen: {
            QPen actualPen = qvariant_cast<QPen>(actualProperty[id]);
            QPen expectedPen = qvariant_cast<QPen>(expectedProperty[id]);
            if (actualPen != expectedPen) {
                qDebug() << "Cell Border Pen MisMatch";
                qDebug() << "Actual Pen:" << actualPen;
                qDebug() << "Expected Pen:" << expectedPen;
                match = false;
            }
            break;
        }
        // brush properties
        case KTableCellStyle::CellBackgroundBrush: {
            QBrush actualBrush = qvariant_cast<QBrush>(actualProperty[id]);
            QBrush expectedBrush = qvariant_cast<QBrush>(expectedProperty[id]);
            if (actualBrush != expectedBrush)
                match = false;
            break;
        }
        }
        if (!match) {
            qDebug() << "Actual property:   " << KTextDebug::tableCellAttributes(actualFormat);
            qDebug() << "Expected property: " << KTextDebug::tableCellAttributes(expectedFormat);
            qDebug() << "At index: QTextTableFormat::UserProperty + " << id + 7001 - QTextFormat::UserProperty;
            return false;
        }
    }
    return match;
}

bool TestLoading::compareTableCells(QTextTableCell &actualCell, QTextTableCell &expectedCell)
{
    //compare row and column spans
    if (actualCell.rowSpan() != expectedCell.rowSpan()) {
        qDebug() << "compareTableCells: table cell row span mismatch";
        qDebug() << "Actual Row Span : " << actualCell.rowSpan();
        qDebug() << "Expected Row Span : " << expectedCell.rowSpan();
        return false;
    }

    if (actualCell.columnSpan() != expectedCell.columnSpan()) {
        qDebug() << "comparetableCells: table cell column span mismatch";
        qDebug() << "Actual Column Span : " << actualCell.columnSpan();
        qDebug() << "Expected Column Span : " << expectedCell.columnSpan();
        return false;
    }

    // compare cell formats
    QTextTableCellFormat actualFormat = actualCell.format().toTableCellFormat();
    QTextTableCellFormat expectedFormat = expectedCell.format().toTableCellFormat();

    if (!compareTableCellFormats(actualFormat, expectedFormat)) {
        qDebug() << "compareTableCells: table cell properties mismatch at " << actualCell.firstCursorPosition().block().text();
        return false;
    }

    // compare cell content
    QTextFrame::iterator actualIterator = actualCell.begin();
    QTextFrame::iterator expectedIterator = expectedCell.begin();

    for (; !actualIterator.atEnd() && !expectedIterator.atEnd(); ++actualIterator, ++expectedIterator) {
        QTextFrame *actualChildFrame = actualIterator.currentFrame();
        QTextBlock actualTextBlock = actualIterator.currentBlock();

        if (actualChildFrame) {
            QTextFrame *expectedChildFrame = expectedIterator.currentFrame();
            if (!expectedChildFrame) {
                qDebug() << "compareTableCells: Unexpected frame at " << actualTextBlock.text();
                return false;
            }
            QTextTable *actualTable = qobject_cast<QTextTable *>(actualChildFrame);
            QTextTable *expectedTable = qobject_cast<QTextTable *>(expectedChildFrame);
            if (actualTable) {
                if (!expectedTable) {
                    qDebug() << "compareTableCells: Unexpected table at " << actualTextBlock.text();
                    return false;
                }
                if (!compareTables(actualTable, expectedTable))
                    return false;
            } else {
                if (expectedTable) {
                    qDebug() << "compareTableCells: Expecting table at " << actualTextBlock.text();
                    return false;
                }
                if (!compareFrames(actualChildFrame, expectedChildFrame))
                    return false;
            }
        } else if (actualTextBlock.isValid()) {
            QTextBlock expectedTextBlock = expectedIterator.currentBlock();
            if (!expectedTextBlock.isValid()) {
                qDebug() << "compareTableCells: Unexpected text block at " << actualTextBlock.text();
                return false;
            }
            if (!compareBlocks(actualTextBlock, expectedTextBlock))
                return false;
        } else {
            qDebug() << "compareTableCells: neither frame nor block! - internal error!";
            return false;
        }
    }
    return actualIterator.atEnd() == expectedIterator.atEnd();
}

bool TestLoading::compareTableFormats(QTextTableFormat &actualFormat, QTextTableFormat &expectedFormat)
{
    if (actualFormat.background() != expectedFormat.background()) {
        qDebug() << "compareTableFormats: Background mismatch";
        qDebug() << "Expected Background: " << expectedFormat.background();
        qDebug() << "Actual Background: " << actualFormat.background();
        return false;
    }

    if (actualFormat.alignment() != expectedFormat.alignment()) {
        qDebug() << "compareTableFormats: Alignment mismatch";
        qDebug() << "Expected Alignment: " << expectedFormat.alignment();
        qDebug() << "Actual Alignment: " << actualFormat.alignment();
        return false;
    }

    if (actualFormat.width() != expectedFormat.width()) {
        qDebug() << "compareTableFormats: Width mismatch";
        qDebug() << "Expected Width: " << expectedFormat.width();
        qDebug() << "Actual Width: " << actualFormat.width();
        return false;
    }

    // check custom properties
    const QMap<int, QVariant> actualProperty = actualFormat.properties();
    const QMap<int, QVariant> expectedProperty = expectedFormat.properties();
    QList<int> allPropertyIds = actualProperty.keys();
    allPropertyIds << expectedProperty.keys();
    bool match = true;
    foreach(int id, allPropertyIds) {
        QString key, value;
        switch (id) {
        // bool properties
        case KTableStyle::BreakBefore:
        case KTableStyle::BreakAfter:
        case KTableStyle::KeepWithNext:
        case KTableStyle::MayBreakBetweenRows:
        case KTableStyle::CollapsingBorders:
            if (actualProperty[id].toBool() != expectedProperty[id].toBool())
                match = false;
            break;
        // double properties
        case QTextFormat::FrameLeftMargin:
        case QTextFormat::FrameRightMargin:
        case QTextFormat::FrameTopMargin:
        case QTextFormat::FrameBottomMargin:
            if (abs(actualProperty[id].toDouble() - expectedProperty[id].toDouble()) > 1e-10)
                match = false;
            break;
        // string properties
        case KTableStyle::MasterPageName:
            if (actualProperty[id].toString() != expectedProperty[id].toString())
                match = false;
            break;
        }
        if (!match) {
            qDebug() << "Actual property:   " << KTextDebug::tableAttributes(actualFormat);
            qDebug() << "Expected property: " << KTextDebug::tableAttributes(expectedFormat);
            qDebug() << "At index: QTextTableFormat::UserProperty + " << id - QTextTableFormat::UserProperty;
            return false;
        }
    }
    return match;
}

bool TestLoading::compareTables(QTextTable *actualTable, QTextTable *expectedTable)
{
    QTextTableFormat actualFormat = actualTable->format();
    QTextTableFormat expectedFormat = expectedTable->format();

    if (!compareTableFormats(actualFormat, expectedFormat)) {
        qDebug() << "compareTables: table properties mismatch at " << actualTable->cellAt(0, 0).firstCursorPosition().block().text();
        return false;
    }

    if (actualTable->rows() != expectedTable->rows()) {
        qDebug() << "compareTables: Rows mismatch";
        qDebug() << "Actual Rows : " << actualTable->rows();
        qDebug() << "Expected Rows : " << expectedTable->rows();
        return false;
    }

    if (actualTable->columns() != expectedTable->columns()) {
        qDebug() << "compareTables: Columns mismatch";
        qDebug() << "Actual Columns : " << actualTable->columns();
        qDebug() << "Expected Columns : " << expectedTable->columns();
        return false;
    }

    for (int row = 0; row < expectedTable->rows(); ++row) {
        for (int col = 0; col < expectedTable->columns(); ++col) {
            QTextTableCell actualCell = actualTable->cellAt(row, col);
            QTextTableCell expectedCell = expectedTable->cellAt(row, col);
            if (!compareTableCells(actualCell, expectedCell)) {
                return false;
            }
        }
    }
    return true;
}

bool TestLoading::compareFrames(QTextFrame *actualFrame, QTextFrame *expectedFrame)
{
    QTextFrame::iterator actualIterator = actualFrame->begin();
    QTextFrame::iterator expectedIterator = expectedFrame->begin();

    for (; !actualIterator.atEnd() && !expectedIterator.atEnd(); ++actualIterator, ++expectedIterator) {
        QTextFrame *actualChildFrame = actualIterator.currentFrame();
        QTextBlock actualTextBlock = actualIterator.currentBlock();

        if (actualChildFrame) {
            QTextFrame *expectedChildFrame = expectedIterator.currentFrame();
            if (!expectedChildFrame) {
                qDebug() << "compareFrames: Unexpected frame at " << actualTextBlock.text();
                return false;
            }
            QTextTable *actualTable = qobject_cast<QTextTable *>(actualChildFrame);
            QTextTable *expectedTable = qobject_cast<QTextTable *>(expectedChildFrame);
            if (actualTable) {
                if (!expectedTable) {
                    qDebug() << "compareFrames: Unexpected table at " << actualTextBlock.text();
                    return false;
                }
                if (!compareTables(actualTable, expectedTable))
                    return false;
            } else {
                if (expectedTable) {
                    qDebug() << "compareFrames: Expecting table at " << actualTextBlock.text();
                    return false;
                }
                if (!compareFrames(actualChildFrame, expectedChildFrame))
                    return false;
            }
        } else if (actualTextBlock.isValid()) {
            QTextBlock expectedTextBlock = expectedIterator.currentBlock();
            if (!expectedTextBlock.isValid()) {
                qDebug() << "compareFrames: Unexpected text block at " << actualTextBlock.text();
                return false;
            }
            if (!compareBlocks(actualTextBlock, expectedTextBlock))
                return false;
        } else {
            qDebug() << "compareFrames: neither frame nor block! - internal error!";
            return false;
        }
    }
    return actualIterator.atEnd() == expectedIterator.atEnd();
}

bool TestLoading::compareDocuments(QTextDocument *actualDocument, QTextDocument *expectedDocument)
{
    QTextFrame *actualFrame = actualDocument->rootFrame();
    QTextFrame *expectedFrame = expectedDocument->rootFrame();
    return compareFrames(actualFrame, expectedFrame);
}

// helper to evaluate script on an engine
static QScriptValue evaluate(QScriptEngine *engine, const QString &script)
{
    QString contents;
    QFile file(script);

    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        contents = stream.readAll();
        file.close();
    }

    QScriptValue r = engine->evaluate(contents);
    if (engine->hasUncaughtException()) {
        QStringList backtrace = engine->uncaughtExceptionBacktrace();
        qDebug("    %s\n%s\n", qPrintable(r.toString()),
               qPrintable(backtrace.join("\n")));
    }
    return r;
}


// add a include function for the scripts
static QScriptValue includeFunction(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return engine->nullValue();

    return evaluate(engine, QString(FILES_DATA_DIR) + context->argument(0).toString());
}

// FIXME: Remove this once the generator is fixed
Q_DECLARE_METATYPE(QTextFormat)
Q_DECLARE_METATYPE(QTextFormat *)

static QScriptValue setFormatProperty(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 3) {
        qWarning() << "too few arguments to setFormatProperty(format, id, value)";
        return engine->nullValue();
    }

    QTextFormat *format = qscriptvalue_cast<QTextFormat *>(context->argument(0));
    int id = context->argument(1).toInt32();
    QScriptValue arg = context->argument(2);
    QVariant value;
    QList<QVariant> qvlist;
    QList<KOdfText::Tab> tabList;
    if (arg.isNumber()) {
        // ### hack to detect if the number is of type int
        if ((qsreal)arg.toInt32() == arg.toNumber())
            value = arg.toInt32();
        else
            value = arg.toNumber();
        format->setProperty(id, value);
    } else if (arg.isArray()) {
        switch (id) {
        case KParagraphStyle::TabPositions:
            qvlist.clear();
            tabList.clear();
            qScriptValueToSequence(arg, tabList);
            foreach(const KOdfText::Tab &tab, tabList) {
                QVariant v;
                v.setValue(tab);
                qvlist.append(v);
            }
            format->setProperty(id, qvlist);
            break;
        default:
            value = arg.toVariant();
            format->setProperty(id, value);
            break;
        }
        //FIXME: Ain't able to convert KOdfText::Tab->QVariant>QScriptValue
        //       in QtScript and back to QScriptValue->QVariant->KOdfText::Tab
        //       in C++. If one can, there's no need for a switch-case here.
    } else {
        value = arg.toVariant();
        format->setProperty(id, value);
    }

    return QScriptValue();
}

Q_DECLARE_METATYPE(QTextCharFormat)
Q_DECLARE_METATYPE(QTextCharFormat *)

static QScriptValue copyFormatProperties(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 1)
        return engine->nullValue();

    QTextFormat *dest = qscriptvalue_cast<QTextFormat *>(context->argument(0));
    QTextFormat *src = qscriptvalue_cast<QTextFormat *>(context->argument(1));
    if (dest && src) {
        QMap<int, QVariant> properties = src->properties();
	for (QMap<int, QVariant>::const_iterator prop = properties.constBegin(); prop != properties.constEnd(); ++prop) {
            dest->setProperty(prop.key(), prop.value());
        }
    }

    return QScriptValue();
}


// May the testing begin
TestLoading::TestLoading()
{
    componentData = new KComponentData("TestLoading");
    componentData->dirs()->addResourceType("styles", "data", "kword/styles/");
}

TestLoading::~TestLoading()
{
    delete componentData;
}

static QScriptValue importExtension(QScriptContext *context, QScriptEngine *engine)
{
    return engine->importExtension(context->argument(0).toString());
}

QScriptValue KoTextTabToQScriptValue(QScriptEngine *engine, const KoTextTab &tab)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("position", QScriptValue(engine, tab.position)); // qreal
    obj.setProperty("type", QScriptValue(engine, tab.type)); // enum
    obj.setProperty("delimiter", QScriptValue(engine, tab.delimiter)); // QChar
    obj.setProperty("leaderType", QScriptValue(engine, tab.leaderType)); // enum
    obj.setProperty("leaderStyle", QScriptValue(engine, tab.leaderStyle)); // enum
    obj.setProperty("leaderWeight", QScriptValue(engine, tab.leaderWeight)); // enum
    obj.setProperty("leaderWidth", QScriptValue(engine, tab.leaderWidth)); // qreal
    if (tab.leaderColor.isValid())
        obj.setProperty("leaderColor", QScriptValue(engine, tab.leaderColor.name())); // QColor
    else
        obj.setProperty("leaderColor", QScriptValue(engine, "invalid")); // QColor
    obj.setProperty("leaderText", QScriptValue(engine, tab.leaderText)); // QString
    return obj;
}

void QScriptValueToKoTextTab(const QScriptValue &obj, KoTextTab &tab)
{
    tab.position = obj.property("position").toNumber();
    tab.type = (QTextOption::TabType) obj.property("type").toInt32();
    tab.delimiter = obj.property("delimiter").toString()[0];
    tab.leaderType = (KCharacterStyle::LineType) obj.property("leaderType").toInt32();
    tab.leaderStyle = (KCharacterStyle::LineStyle) obj.property("leaderStyle").toInt32();
    tab.leaderWeight = (KCharacterStyle::LineWeight) obj.property("leaderWeight").toInt32();
    tab.leaderWidth = obj.property("leaderWidth").toNumber();
    if (obj.property("leaderColor").toString() != "invalid")
        tab.leaderColor = QColor(obj.property("leaderColor").toString());
    tab.leaderText = obj.property("leaderText").toString();
}

QScriptValue constructKoTextTab(QScriptContext *, QScriptEngine *engine)
{
    return engine->toScriptValue(KoTextTab());
}
Q_DECLARE_METATYPE(QList<KOdfText::Tab>)

// initTestCase/cleanupTestCase are called beginning and end of test suite
void TestLoading::initTestCase()
{
    QString pluginPath = qgetenv("QSCRIPT_PLUGIN_PATH");
    if (pluginPath.isEmpty())
        qWarning() << "QSCRIPT_PLUGIN_PATH not set. Set it to the path of QScript plugins";

    QStringList paths = qApp->libraryPaths();
    paths <<  pluginPath;
    qApp->setLibraryPaths(paths);

    engine = new QScriptEngine();

    QScriptValue rc = engine->importExtension("qt.core");
    if (rc.isError()) {
        kWarning() << "Failed to find Qt bindings, aborting";
        QSKIP("Failed to find Qt bindings (qtscript generator), aborting", SkipAll);
    }
    rc = engine->importExtension("qt.gui");
    if (rc.isError()) {
        kWarning() << "Failed to load QtGui bindings, aborting";
        abort();
    }

    QScriptValue globalObject = engine->globalObject();
    globalObject.setProperty("qApp", engine->newQObject(qApp));
    QScriptValue qscript = engine->newObject();
    qscript.setProperty("importExtension", engine->newFunction(importExtension));
    globalObject.property("qt").setProperty("script", qscript);

    globalObject.setProperty("include", engine->newFunction(includeFunction));
    globalObject.setProperty("setFormatProperty", engine->newFunction(setFormatProperty));
    globalObject.setProperty("copyFormatProperties", engine->newFunction(copyFormatProperties));

    globalObject.setProperty("KoTextTab", engine->newFunction(constructKoTextTab));
    qScriptRegisterMetaType(engine, KoTextTabToQScriptValue, QScriptValueToKoTextTab);
    qScriptRegisterSequenceMetaType< QList<KOdfText::Tab> > (engine);
}

void TestLoading::cleanupTestCase()
{
    delete engine;
    engine = 0;
}

// init/cleanup are called beginning and end of every test case
void TestLoading::init()
{
    // FIXME: the line below exists because I haven't manage get includeFunction to work
    evaluate(engine, QString(FILES_DATA_DIR) + "common.qs");
}

void TestLoading::cleanup()
{
}

void TestLoading::addData(LoadSave loadSave)
{
    QTest::newRow("whitespace") << "TextContents/Paragraph/whitespace";
    QTest::newRow("basicContents") << "TextContents/Paragraph/basicContents";
    QTest::newRow("boldAndItalic") << "TextContents/TextFormatting/boldAndItalic";

    QTest::newRow("bulletedList") << "TextContents/Lists/bulletedList";
    QTest::newRow("continueNumbering") << "TextContents/Lists/continueNumbering";
    QTest::newRow("embeddedBulletedList") << "TextContents/Lists/embeddedBulletedList";
    QTest::newRow("listHeader") << "TextContents/Lists/listHeader";
    QTest::newRow("multilevelList") << "TextContents/Lists/multilevelList";
    QTest::newRow("multipleParagraphs") << "TextContents/Lists/multipleParagraphs";
    QTest::newRow("numberedList") << "TextContents/Lists/numberedList";
    QTest::newRow("numberedParagraph") << "TextContents/Lists/numberedParagraph";
    QTest::newRow("startValue") << "TextContents/Lists/startValue";


    QTest::newRow("attributedText") << "TextContents/Paragraph/attributedText";
    QTest::newRow("nestedSpan") << "TextContents/Paragraph/nestedSpan";

    QTest::newRow("fontSize1") << "TextContents/TextFormatting/fontSize";

    QTest::newRow("fontColors") << "TextContents/TextFormatting/fontColors";

    if (loadSave == TestLoadingData) {
        QTest::newRow("simpleTable") << "Tables/simpleTable";
        QTest::newRow("spanningCells") << "Tables/spanningCells";
        QTest::newRow("tableWidth") << "Tables/tableWidth";
        QTest::newRow("tableAlignment") << "Tables/tableAlignment";
        QTest::newRow("tableTopAndBottomMargin") << "Tables/tableTopAndBottomMargin";
        QTest::newRow("tableLeftAndRightMargin") << "Tables/tableLeftAndRightMargin";
        QTest::newRow("tableMargins") << "Tables/tableMargins";
        QTest::newRow("breakBeforeAndBreakAfter") << "Tables/breakBeforeAndBreakAfter";
        QTest::newRow("mayBreakBetweenRows") << "Tables/mayBreakBetweenRows";
        QTest::newRow("tableBackground") << "Tables/tableBackground";
        QTest::newRow("borderModelProperty") << "Tables/borderModelProperty";
        QTest::newRow("keepWithNext") << "Tables/keepWithNext";
        QTest::newRow("cellBackground") << "Tables/cellBackground";
        QTest::newRow("cellPadding") << "Tables/cellPadding";
        QTest::newRow("cellBorder") << "Tables/cellBorder";
        QTest::newRow("cellBorderStyle") << "Tables/cellBorderStyle";
        QTest::newRow("cellBorderSpacing") << "Tables/cellBorderSpacing";
    }

    // TODO: Write tests for these.
    //QTest::newRow("borderModelProperty") << "FormattingProperties/TableFormattingProperties/borderModelProperty";
    //QTest::newRow("breakBeforeAndBreakAfter") << "FormattingProperties/TableFormattingProperties/breakBeforeAndBreakAfter";
    //QTest::newRow("display") << "FormattingProperties/TableFormattingProperties/display";
    //QTest::newRow("mayBreakBetweenRows") << "FormattingProperties/TableFormattingProperties/mayBreakBetweenRows";
    //QTest::newRow("pageNumber") << "FormattingProperties/TableFormattingProperties/pageNumber";
    //QTest::newRow("tableAlignment") << "FormattingProperties/TableFormattingProperties/tableAlignment";
    //QTest::newRow("tableBackgroundAndBackgroundImage") << "FormattingProperties/TableFormattingProperties/tableBackgroundAndBackgroundImage";
    //QTest::newRow("tableMargins") << "FormattingProperties/TableFormattingProperties/tableMargins";
    //QTest::newRow("tableShadow") << "FormattingProperties/TableFormattingProperties/tableShadow";
    //QTest::newRow("writingMode") << "FormattingProperties/TableFormattingProperties/writingMode";

    QTest::newRow("color") << "FormattingProperties/TextFormattingProperties/color";

    QTest::newRow("country") << "FormattingProperties/TextFormattingProperties/country";
    QTest::newRow("fontCharset") << "FormattingProperties/TextFormattingProperties/fontCharacterSet";
    QTest::newRow("fontFamily") << "FormattingProperties/TextFormattingProperties/fontFamily";
    QTest::newRow("fontFamilyGeneric") << "FormattingProperties/TextFormattingProperties/fontFamilyGeneric";
    QTest::newRow("fontName") << "FormattingProperties/TextFormattingProperties/fontName";
    QTest::newRow("fontPitch") << "FormattingProperties/TextFormattingProperties/fontPitch";
    QTest::newRow("fontSize2") << "FormattingProperties/TextFormattingProperties/fontSize";
    QTest::newRow("fontStyle") << "FormattingProperties/TextFormattingProperties/fontStyle";
    QTest::newRow("fontWeight") << "FormattingProperties/TextFormattingProperties/fontWeight";
    QTest::newRow("fontVariant") << "FormattingProperties/TextFormattingProperties/fontVariant";
    QTest::newRow("language") << "FormattingProperties/TextFormattingProperties/language";
    QTest::newRow("letterKerning") << "FormattingProperties/TextFormattingProperties/letterKerning";
    QTest::newRow("lineThroughColor") << "FormattingProperties/TextFormattingProperties/lineThroughColor";
    QTest::newRow("lineThroughMode") << "FormattingProperties/TextFormattingProperties/lineThroughMode";
    QTest::newRow("lineThroughStyle") << "FormattingProperties/TextFormattingProperties/lineThroughStyle";
    QTest::newRow("lineThroughType") << "FormattingProperties/TextFormattingProperties/lineThroughType";
    QTest::newRow("lineThroughWidth") << "FormattingProperties/TextFormattingProperties/lineThroughWidth";
    QTest::newRow("lineThroughText") << "FormattingProperties/TextFormattingProperties/lineThroughText";
    QTest::newRow("relativeFontSize") << "FormattingProperties/TextFormattingProperties/relativeFontSize";
    QTest::newRow("textBackgroundColor") << "FormattingProperties/TextFormattingProperties/textBackgroundColor";
    QTest::newRow("textOutline") << "FormattingProperties/TextFormattingProperties/textOutline";
    QTest::newRow("textTranformations") << "FormattingProperties/TextFormattingProperties/textTransformations";
    QTest::newRow("underlineColor") << "FormattingProperties/TextFormattingProperties/underlineColor";
    QTest::newRow("underlineMode") << "FormattingProperties/TextFormattingProperties/underlineMode";
    QTest::newRow("underlineType") << "FormattingProperties/TextFormattingProperties/underlineType";
    QTest::newRow("underlineStyle") << "FormattingProperties/TextFormattingProperties/underlineStyle";
    QTest::newRow("underlineWidth") << "FormattingProperties/TextFormattingProperties/underlineWidth";

    QTest::newRow("autoTextIndent") << "FormattingProperties/ParagraphFormattingProperties/automaticTextIndent";
    QTest::newRow("border") << "FormattingProperties/ParagraphFormattingProperties/border";
    QTest::newRow("borderLineWidth") << "FormattingProperties/ParagraphFormattingProperties/borderLineWidth";
    QTest::newRow("dropCapsDistance") << "FormattingProperties/ParagraphFormattingProperties/dropCapsDistance";
    QTest::newRow("dropCapsLength") << "FormattingProperties/ParagraphFormattingProperties/dropCapsLength";
    QTest::newRow("dropCapsLines") << "FormattingProperties/ParagraphFormattingProperties/dropCapsLines";
    QTest::newRow("lineSpacing") << "FormattingProperties/ParagraphFormattingProperties/lineSpacing";
    QTest::newRow("margin") << "FormattingProperties/ParagraphFormattingProperties/margin";
    QTest::newRow("marginLeftRight") << "FormattingProperties/ParagraphFormattingProperties/marginLeftRight";
    QTest::newRow("marginTopBottom") << "FormattingProperties/ParagraphFormattingProperties/marginTopBottom";
    QTest::newRow("paragraphBackground") << "FormattingProperties/ParagraphFormattingProperties/paragraphBackgroundColor";
    QTest::newRow("textAlign") << "FormattingProperties/ParagraphFormattingProperties/textAlign";
    QTest::newRow("textIndent") << "FormattingProperties/ParagraphFormattingProperties/textIndent";
    QTest::newRow("tabDelimiterChar") << "FormattingProperties/ParagraphFormattingProperties/tabDelimiterChar";
    QTest::newRow("tabLeaderColor") << "FormattingProperties/ParagraphFormattingProperties/tabLeaderColor";
    QTest::newRow("tabLeaderStyle") << "FormattingProperties/ParagraphFormattingProperties/tabLeaderStyle";
    QTest::newRow("tabLeaderText") << "FormattingProperties/ParagraphFormattingProperties/tabLeaderText";
    QTest::newRow("tabLeaderType") << "FormattingProperties/ParagraphFormattingProperties/tabLeaderType";
    QTest::newRow("tabLeaderWidth") << "FormattingProperties/ParagraphFormattingProperties/tabLeaderWidth";
    QTest::newRow("tabPosition") << "FormattingProperties/ParagraphFormattingProperties/tabPosition";
    QTest::newRow("tabStopDistance") << "FormattingProperties/ParagraphFormattingProperties/tabStopDistance";
    QTest::newRow("tabType") << "FormattingProperties/ParagraphFormattingProperties/tabType";

    QTest::newRow("bookmark") << "ParagraphElements/bookmark";
    QTest::newRow("note") << "ParagraphElements/note";
}

QTextDocument *TestLoading::documentFromScript(const QString &script)
{
    return qobject_cast<QTextDocument *>(evaluate(engine, script).toQObject());
}

QTextDocument *TestLoading::documentFromOdt(const QString &odt)
{
    if (!QFile(odt).exists()) {
        qFatal("%s does not exist", qPrintable(odt));
        return 0;
    }
    return KOdfText::loadOpenDocument(odt);
}

QString TestLoading::documentToOdt(QTextDocument *document)
{
    QString odt("test.odt");
    if (QFile::exists(odt))
        QFile::remove(odt);
    QFile f(odt);
    f.open(QFile::WriteOnly);
    f.close();

    KOdfStore *store = KOdfStore::createStore(odt, KOdfStore::Write, "application/vnd.oasis.opendocument.text", KOdfStore::Zip);
    KOdfWriteStore odfWriteStore(store);
    KXmlWriter *manifestWriter = odfWriteStore.manifestWriter("application/vnd.oasis.opendocument.text");
    manifestWriter->addManifestEntry("content.xml", "text/xml");
    if (!store->open("content.xml"))
        return QString();

    KOdfStorageDevice contentDev(store);
    KXmlWriter* contentWriter = KOdfWriteStore::createOasisXmlWriter(&contentDev, "office:document-content");

    // for office:body
    KTemporaryFile contentTmpFile;
    if (!contentTmpFile.open())
        qFatal("Error opening temporary file!");
    KXmlWriter xmlWriter(&contentTmpFile, 1);

    KOdfGenericStyles mainStyles;
    KStyleManager *styleMan = KTextDocument(document).styleManager();
    Q_UNUSED(styleMan);
    KOdfEmbeddedDocumentSaver embeddedSaver;

    KOdfGenericChanges changes;
    KShapeSavingContext context(xmlWriter, mainStyles, embeddedSaver);

    KSharedSavingData *sharedData = context.sharedData(KODFTEXT_SHARED_SAVING_ID);
    KTextSharedSavingData *textSharedData = 0;
    if (sharedData) {
        textSharedData = dynamic_cast<KTextSharedSavingData *>(sharedData);
    }

    kDebug(32500) << "sharedData" << sharedData << "textSharedData" << textSharedData;

    if (!textSharedData) {
        textSharedData = new KTextSharedSavingData();
        textSharedData->setGenChanges(changes);
        if (!sharedData) {
            context.addSharedData(KODFTEXT_SHARED_SAVING_ID, textSharedData);
        } else {
            kWarning(32500) << "A different type of sharedData was found under the" << KODFTEXT_SHARED_SAVING_ID;
            Q_ASSERT(false);
        }
    }

    KTextShapeData *textShapeData = new KTextShapeData;
    textShapeData->setDocument(document, false /* ownership */);
    if (qobject_cast<KTextDocumentLayout *>(document->documentLayout()) == 0) {
        // Setup layout and managers just like kotext
        KTextDocumentLayout *layout = new KTextDocumentLayout(textShapeData->document());
        textShapeData->document()->setDocumentLayout(layout);
        layout->setInlineTextObjectManager(new KInlineTextObjectManager(layout)); // required while saving
        KStyleManager *styleManager = new KStyleManager;
        KTextDocument(textShapeData->document()).setStyleManager(styleManager);
    }
    KChangeTracker* changeTracker = new KChangeTracker;
    KTextDocument(textShapeData->document()).setChangeTracker(changeTracker);

    textShapeData->saveOdf(context, 0);

    contentTmpFile.close();

    mainStyles.saveOdfStyles(KOdfGenericStyles::DocumentAutomaticStyles, contentWriter);

    contentWriter->startElement("office:body");
    contentWriter->startElement("office:text");

//    changes.saveOdfChanges(contentWriter, false);

    contentWriter->addCompleteElement(&contentTmpFile);

    contentWriter->endElement(); //office text
    contentWriter->endElement(); //office body

    contentWriter->endElement(); // root element
    contentWriter->endDocument();
    delete contentWriter;


    if (!store->close())
        qWarning() << "Failed to close the store";

    mainStyles.saveOdfStylesDotXml(store, manifestWriter);

    odfWriteStore.closeManifestWriter();


    delete store;
    delete textShapeData;

    return odt;
}

void TestLoading::testLoading_data()
{
    QTest::addColumn<QString>("testcase");

    addData(TestLoadingData);
}

void TestLoading::testLoading()
{
    QFETCH(QString, testcase);
    QString testName = testcase;
    testcase.prepend(FILES_DATA_DIR);

    QTextDocument *actualDocument = documentFromOdt(testcase + ".odt");
    QVERIFY(actualDocument != 0);

    QTextDocument *expectedDocument = documentFromScript(testcase + ".qs");
    QVERIFY(expectedDocument != 0);

    bool documentsEqual = compareDocuments(actualDocument, expectedDocument);

    Q_UNUSED(showDocument);
//    showDocument(actualDocument);
//    showDocument(expectedDocument);
    if (!documentsEqual) {
        testName.replace('/', '_');
        QFile file1("failed-loading-" + testName + "-actual");
        if (file1.open(QFile::WriteOnly)) {
            QTextStream out(&file1);
            KTextDebug::dumpDocument(actualDocument, out);
            file1.close();
        }
        QFile file2("failed-loading-" + testName + "-expected");
        if (file2.open(QFile::WriteOnly)) {
            QTextStream out(&file2);
            KTextDebug::dumpDocument(expectedDocument, out);
            file2.close();
        }
    }
    if (testcase.indexOf("Tables/") > 0)
        QEXPECT_FAIL("", "Tables not supported yet", Abort);
    delete actualDocument;
    delete expectedDocument;
    QVERIFY(documentsEqual);
}

void TestLoading::testSaving_data()
{
    QTest::addColumn<QString>("testcase");

    addData(TestSavingData);
}

void TestLoading::testSaving()
{
    QFETCH(QString, testcase);
    QString testName = testcase;
    testcase.prepend(FILES_DATA_DIR);

    QTextDocument *actualDocument = documentFromOdt(testcase + ".odt");
    QVERIFY(actualDocument != 0);
    QString fileName = documentToOdt(actualDocument);
    QVERIFY(!fileName.isEmpty());
    QTextDocument *savedDocument = documentFromOdt(fileName);

    QTextDocument *expectedDocument = documentFromScript(testcase + ".qs");
    QVERIFY(expectedDocument != 0);

    bool documentsEqual = compareDocuments(savedDocument, expectedDocument);

    if (!documentsEqual) {
        testName.replace('/', '_');
        QFile file1("failed-saving-" + testName + "-actual");
        if (file1.open(QFile::WriteOnly)) {
            QTextStream out(&file1);
            KTextDebug::dumpDocument(savedDocument, out);
            file1.close();
        }
        QFile file2("failed-saving-" + testName + "-expected");
        if (file2.open(QFile::WriteOnly)) {
            QTextStream out(&file2);
            KTextDebug::dumpDocument(expectedDocument, out);
            file2.close();
        }
    }
    delete actualDocument;
    delete expectedDocument;
    delete savedDocument;
    if (testcase.indexOf("Tables/") > 0)
        QEXPECT_FAIL("", "Tables not supported yet", Abort);
    QVERIFY(documentsEqual);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    return QTest::qExec(new TestLoading, argc, argv);
}

#include <TestLoading.moc>
