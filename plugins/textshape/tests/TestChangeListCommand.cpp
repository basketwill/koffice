#include "TestChangeListCommand.h"
#include "tests/MockShapes.h"
#include "../commands/ChangeListCommand.h"

#include <KListStyle.h>
#include <KListLevelProperties.h>
#include <KStyleManager.h>
#include <KTextDocument.h>

#include <TextTool.h>
#include <KCanvasBase.h>

#include <QTextDocument>
#include <QTextCursor>
#include <QTextList>


void TestChangeListCommand::addList()
{
    QTextDocument doc;
    KTextDocument(&doc).setStyleManager(new KStyleManager);
    TextTool *tool = new TextTool(new MockCanvas);
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4\n");

    QTextBlock block = doc.begin().next();
    cursor.setPosition(block.position());
    ChangeListCommand clc(cursor, KListStyle::DecimalItem);
    clc.setTool(tool);
    clc.redo();

    block = doc.begin();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QVERIFY(block.textList()); // this one we just changed
    QTextList *tl = block.textList();
    block = block.next();
    QVERIFY(block.textList() == 0);

    QTextListFormat format = tl->format();
    QCOMPARE(format.intProperty(QTextListFormat::ListStyle), (int) KListStyle::DecimalItem);

    cursor.setPosition(block.position());
    ChangeListCommand clc2(cursor, KListStyle::DiscItem);
    clc2.setTool(tool);
    clc2.redo();

    block = doc.begin();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QCOMPARE(block.textList(), tl);
    block = block.next();
    QVERIFY(block.textList());
    QVERIFY(block.textList() != tl);
}

void TestChangeListCommand::addListWithLevel2()
{
    QTextDocument doc;
    KTextDocument(&doc).setStyleManager(new KStyleManager);
    TextTool *tool = new TextTool(new MockCanvas);
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4\n");

    QTextBlock block = doc.begin().next();
    cursor.setPosition(block.position());

    KListStyle style;
    KListLevelProperties llp;
    llp.setLevel(2);
    llp.setDisplayLevel(2);
    llp.setStyle(KListStyle::DiscItem);
    style.setLevelProperties(llp);

    ChangeListCommand clc(cursor, &style, 2);
    clc.setTool(tool);
    clc.redo();

    block = doc.begin();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QVERIFY(block.textList()); // this one we just changed
    QTextList *tl = block.textList();
    block = block.next();
    QVERIFY(block.textList() == 0);

    QTextListFormat format = tl->format();
    QCOMPARE(format.intProperty(QTextListFormat::ListStyle), (int) KListStyle::DiscItem);
    QCOMPARE(format.intProperty(KListStyle::DisplayLevel), (int) 2);
    QCOMPARE(format.intProperty(KListStyle::Level), (int) 2);
}

void TestChangeListCommand::removeList()
{
    QTextDocument doc;
    KTextDocument(&doc).setStyleManager(new KStyleManager);
    TextTool *tool = new TextTool(new MockCanvas);
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4\n");
    KListStyle style;
    QTextBlock block = doc.begin().next();
    while (block.isValid()) {
        style.applyStyle(block);
        block = block.next();
    }

    block = doc.begin().next();
    QVERIFY(block.textList()); // init, we should not have to test KListStyle here ;)

    cursor.setPosition(block.position());
    ChangeListCommand clc(cursor, KListStyle::None);
    clc.setTool(tool);
    clc.redo();

    block = doc.begin();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QVERIFY(block.textList());
    block = block.next();
    QVERIFY(block.textList());
    block = block.next();
    QVERIFY(block.textList());

    cursor.setPosition(block.position());
    ChangeListCommand clc2(cursor, KListStyle::None);
    clc2.setTool(tool);
    clc2.redo();
    block = doc.begin();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QVERIFY(block.textList());
    block = block.next();
    QVERIFY(block.textList());
    block = block.next();
    QVERIFY(block.textList() == 0);
}

void TestChangeListCommand::joinList()
{
    QTextDocument doc;
    KTextDocument(&doc).setStyleManager(new KStyleManager);
    TextTool *tool = new TextTool(new MockCanvas);
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4\n");
    KListStyle style;
    KListLevelProperties llp;
    llp.setLevel(1);
    llp.setStyle(KListStyle::DiscItem);
    style.setLevelProperties(llp);
    QTextBlock block = doc.begin().next();
    style.applyStyle(block);
    block = block.next();
    block = block.next(); // skip parag2
    style.applyStyle(block);
    block = block.next();
    style.applyStyle(block);

    block = doc.begin().next();
    QTextList *tl = block.textList();
    QVERIFY(tl); // init, we should not have to test KListStyle here ;)
    block = block.next(); // parag2
    QVERIFY(block.textList() == 0);

    cursor.setPosition(block.position());
    ChangeListCommand clc(cursor, KListStyle::DiscItem);
    clc.setTool(tool);
    clc.redo();
    QCOMPARE(block.textList(), tl);
}

void TestChangeListCommand::joinList2()
{
    // test usecase of joining with the one before and the one after based on similar styles.
    QTextDocument doc;
    KTextDocument(&doc).setStyleManager(new KStyleManager);
    TextTool *tool = new TextTool(new MockCanvas);
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4");
    KListStyle style;
    KListLevelProperties llp1;
    llp1.setLevel(1);
    llp1.setStyle(KListStyle::DiscItem);
    style.setLevelProperties(llp1);
    QTextBlock block = doc.begin().next().next();
    style.applyStyle(block); // apply on parag2

    KListStyle style2;
    KListLevelProperties llp;
    llp.setLevel(1);
    llp.setStyle(KListStyle::DecimalItem);
    llp.setListItemSuffix(".");
    style2.setLevelProperties(llp);
    block = block.next().next(); // parag4
    style2.applyStyle(block);

    // now apply the default 'DiscItem' on 'parag1' expecting it to join with the list already set on 'parag2'
    block = doc.begin().next();
    cursor.setPosition(block.position());
    ChangeListCommand clc(cursor, KListStyle::DiscItem);
    clc.setTool(tool);
    clc.redo();
    QTextList *tl = block.textList();
    QVERIFY(tl);
    block = block.next();
    QCOMPARE(tl, block.textList());
    QCOMPARE(tl->format().intProperty(QTextListFormat::ListStyle), (int) KListStyle::DiscItem);

    // now apply the 'DecimalItem' on 'parag3' and expect it to join with the list already set on 'parag4'
    block = doc.findBlock(30);
    QCOMPARE(block.text(), QString("parag4"));
    QTextList *numberedList = block.textList();
    QVERIFY(numberedList);
    block = block.previous(); // parag3
    QVERIFY(block.textList() == 0);
    cursor.setPosition(block.position());
    ChangeListCommand clc2(cursor, KListStyle::DecimalItem);
    clc2.setTool(tool);
    clc2.redo();
    QVERIFY(block.textList());
    QVERIFY(block.textList() != tl);
    QVERIFY(block.textList() == numberedList);
    QCOMPARE(numberedList->format().intProperty(QTextListFormat::ListStyle), (int) KListStyle::DecimalItem);
}

void TestChangeListCommand::splitList()
{
    // assume I start with;
    // 1 paragA
    // 1.1 paragB
    // 1.2 paragC
    // now I change parag 'B' to '1.a'  then C should have 1.1 as a numbering. I.e. we should split an existing list.

    QTextDocument doc;
    KTextDocument(&doc).setStyleManager(new KStyleManager);
    TextTool *tool = new TextTool(new MockCanvas);
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparagA\nparagB\nparagC");
    QTextBlock block = doc.begin().next();
    KListStyle style;
    style.applyStyle(block); // apply on parag2

    KListStyle style2;
    KListLevelProperties llp = style2.levelProperties(2);
    style2.setLevelProperties(llp);
    block = block.next();
    style2.applyStyle(block);
    block = block.next();
    style2.applyStyle(block);

    QTextBlock paragA = doc.begin().next();
    QVERIFY(paragA.textList());
    QTextBlock paragB = paragA.next();
    QVERIFY(paragB.textList());
    QVERIFY(paragB.textList() != paragA.textList());
    QTextBlock paragC = paragB.next();
    QVERIFY(paragC.textList());
    QCOMPARE(paragC.textList(), paragB.textList());

    QTextList *tl = paragB.textList();
    cursor.setPosition(paragB.position());
    ChangeListCommand clc(cursor, KListStyle::AlphaLowerItem);
    clc.setTool(tool);
    clc.redo();

    QVERIFY(doc.begin().textList() == 0);
    QVERIFY(paragA.textList());
    QTextList *newTextList = paragB.textList();
    QVERIFY(newTextList == tl);
    QCOMPARE(paragC.textList(), tl);

    QCOMPARE(tl->format().intProperty(KListStyle::Level), 2);
    QCOMPARE(newTextList->format().intProperty(KListStyle::Level), 2);
}

QTEST_MAIN(TestChangeListCommand)

#include <TestChangeListCommand.moc>
