
#ifndef TESTFORMULACURSOR_H
#define TESTFORMULACURSOR_H

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

#include "kformuladefs.h"

class KoCommandHistory;

KFORMULA_NAMESPACE_BEGIN

class BracketElement;
class FormulaElement;
class FormulaCursor;
class IndexElement;
class Container;
class Document;
class TextElement;


class TestFormulaCursor : public CppUnit::TestCase {
public:
    //TestFormulaCursor(std::string name) : CppUnit::TestCase(name) {}

    static Test* suite();

    void setUp();
    void tearDown();

private:

    void testTextInsertion();
    void testRemoval();
    void testRemoveIndexElementByMainChild();
    void testRemoveBracketElementByMainChild();
    void testRemoveBracketElementByNonEmptyMainChild();
    void testActiveIndexElement();
    void testReplaceByEmptyContent();
    void testReplaceByContent();
    void testReplaceSelectionByElement();
    void testCursorSaving();

    KoCommandHistory* history;
    Document* document;
    Container* container;
    FormulaElement* rootElement;
    FormulaCursor* cursor;

    BracketElement* element1;
    TextElement* element2;
    IndexElement* element3;
    TextElement* element4;
    TextElement* element5;
};

KFORMULA_NAMESPACE_END

#endif // TESTFORMULACURSOR_H
