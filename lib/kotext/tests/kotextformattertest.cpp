//KoTextFormatter test (also for profiling purposes), GPL v2, David Faure <faure@kde.org>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include "kotextformatter.h"
#include "kotextformat.h"
#include "kotextdocument.h"
#include "kozoomhandler.h"
#include "koparagcounter.h"

class KoTextFormatterTest
{
public:
    KoTextFormatterTest();
    ~KoTextFormatterTest() {
        delete zh;
        delete doc;
    }

    void speedTest();
    void counterAndBigChar();
    void oneLineTest();
    void noHeightTest();
    void noWidthEverTest();

private:
    KoZoomHandler* zh;
    KoTextDocument* doc;
};

// To test various cases with variable document widths: a custom KoTextFlow
class TextFlow : public KoTextFlow
{
public:
    TextFlow( int availableWidth, int availableHeight )
        : m_availableWidth( availableWidth ), m_availableHeight( availableHeight )
        {}
    virtual int availableHeight() const {
        return m_availableHeight;
    }
    virtual void adjustMargins( int yp, int h, int reqMinWidth, int& leftMargin, int& rightMargin, int& pageWidth, KoTextParag* ) {
        Q_UNUSED(yp);
        Q_UNUSED(h);
        Q_UNUSED(reqMinWidth);
        Q_UNUSED(leftMargin);
        Q_UNUSED(rightMargin);
        pageWidth = m_availableWidth;
    }
private:
    int m_availableWidth;
    int m_availableHeight;
};

KoTextFormatterTest::KoTextFormatterTest()
{
    zh = new KoZoomHandler;
    QFont defaultFont( "helvetica", 12 );
    KoTextFormatCollection* fc = new KoTextFormatCollection( defaultFont, Qt::black, "en_US", false /*no hyphenation*/, 1.0 );
    KoTextFormatter* formatter = new KoTextFormatter;
    // fc and formatter are owned by the doc
    doc = new KoTextDocument( zh, fc, formatter );
}

void KoTextFormatterTest::speedTest()
{
    kdDebug() << k_funcinfo << endl;
    doc->clear(true);
    KoTextParag* parag = doc->firstParag();
    parag->append( "They burst into flames when it is time for them to die, and then they are reborn from the ashes" );

    // Format it 50 times
    for ( uint i = 0 ; i < 50 ; ++i )
    {
      parag->invalidate(0);
      parag->format();
    }
    doc->clear(false);
}

void KoTextFormatterTest::noHeightTest()
{
    kdDebug() << k_funcinfo << endl;
    // We test the case of going past maxY - by setting the available height to 0
    // Expected result: the formatter 'aborts', i.e. no line-breaking, but still
    // goes over each character to set them all correctly; and usually KWord
    // would create a new page and reformat the paragraph
    doc->setFlow( new TextFlow( 250, 0 ) ); // 250 is just enough for one char
    doc->clear(true);
    KoTextParag* parag = doc->firstParag();
    parag->append( "abcdefghi" );
    parag->format();
    assert( parag->lines() == 2 ); // one break, then we keep going
    doc->clear(false);
    doc->setFlow( new KoTextFlow ); // default
}

void KoTextFormatterTest::noWidthEverTest()
{
    kdDebug() << k_funcinfo << endl;
    // We test the case of formatting where there is no width (e.g. narrow
    // passage, or after last page).
    // Expected result: the formatter goes down until it finds more width
    // (TODO a test case where this happens)
    // If it doesn't find any (and hits maxY), then it 'aborts', i.e. no line-breaking,
    // but still goes over each character to set them all correctly; and usually KWord
    // would create a new page and reformat the paragraph
    doc->setFlow( new TextFlow( 0, 2000 ) );
    doc->clear(true);
    KoTextParag* parag = doc->firstParag();
    parag->append( "abcdefghi" );
    parag->format();
    // The resulting paragraph is NOT marked as formatted. See kotextformatter.cc -r1.79
    assert( !parag->isValid() );
    doc->clear(false);
    doc->setFlow( new KoTextFlow ); // default
}

void KoTextFormatterTest::oneLineTest()
{
    kdDebug() << k_funcinfo << endl;
    // Normal case, only one line
    // Expected: the parag is as wide as the doc
    doc->setFlow( new TextFlow( 2000, 200 ) );
    doc->clear(true);
    KoTextParag* parag = doc->firstParag();
    parag->append( "abcdefghi" );
    parag->format();
    assert( parag->lines() == 1 );
    assert( parag->isValid() );
    assert( parag->rect().width() == 2000 );
    assert( parag->widthUsed() < 2000 );
    doc->clear(false);
    doc->setFlow( new KoTextFlow ); // default
}

void KoTextFormatterTest::counterAndBigChar()
{
    kdDebug() << k_funcinfo << endl;
    // Only one line, with a counter and a big char.
    // Bug #82609: the new height led to "formatting again" which restarted without taking the counter into account
    // Expected: the char starts after the counter
    doc->setFlow( new TextFlow( 2000, 200 ) );
    doc->clear(true);
    KoTextParag* parag = doc->firstParag();
    parag->append( "aB" );
    KoTextFormat f( *parag->at( 0 )->format() );
    f.setPointSize( 48 );
    parag->setFormat( 1, 1, doc->formatCollection()->format( &f ), true );
    KoParagCounter counter;
    counter.setNumbering( KoParagCounter::NUM_LIST );
    counter.setStyle( KoParagCounter::STYLE_NUM );
    parag->setCounter( &counter );
    parag->format();
    parag->printRTDebug( 1 );
    assert( parag->lines() == 1 );
    assert( parag->isValid() );
    assert( parag->rect().width() == 2000 );
    assert( parag->widthUsed() < 2000 );
    assert( parag->at(0)->x > 0 );
    doc->clear(false);
    doc->setFlow( new KoTextFlow ); // default
}

int main (int argc, char ** argv)
{
    KApplication app(argc, argv, "KoTextFormatter test");

    // Don't let locale settings lead to different hyphenation output
    KGlobal::locale()->setLanguage( QString::fromLatin1( "en_US" ) );
    KGlobal::locale()->setCountry( QString::fromLatin1( "C" ) );

    KoTextFormatterTest test;
    //test.speedTest();
    test.oneLineTest();
    test.counterAndBigChar();
    test.noHeightTest();
    test.noWidthEverTest();

    return 0;
}
