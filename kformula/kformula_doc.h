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

#ifndef KFORMULA_DOC_H
#define KFORMULA_DOC_H

class FormulaCursor;
class KFormulaContainer;
class KFormulaDoc;


#include <qlist.h>
#include <qpainter.h>

#include <kcommand.h>
#include <koDocument.h>

#include "formuladef.h"
#include "kformula_view.h"


/**
 * The part's document. Forwards most of the requests.
 */
class KFormulaDoc : public KoDocument
{
    Q_OBJECT
    
public:

    KFormulaDoc(QWidget *parentWidget = 0,
                const char *widgetName = 0,
                QObject* parent = 0,
                const char* name = 0,
                bool singleViewMode = false);
    ~KFormulaDoc();

    virtual void paintContent(QPainter& painter,
                              const QRect& rect, bool transparent = FALSE);

    virtual bool initDoc();

    virtual bool loadXML(QIODevice *, const QDomDocument& doc);
    virtual QDomDocument saveXML();

    KFormulaContainer* getFormula() { return formula; }
    
    void print(QPrinter *thePrt);

public slots:

    void enlarge();
    void reduce();
    void enlargeRecur();
    void reduceRecur();
    void enlargeAll();
    void reduceAll();
    void pro();
    void dele();

protected slots:

    void addBracket();
    void commandExecuted();
    void documentRestored();
    
protected:
    
    virtual QString configFile() const;
    virtual KoView* createViewInstance(QWidget* parent, const char* name);

private:

    /**
     * Our undo stack.
     */
    KCommandHistory history;
    
    /**
     * The place where all formula related work is done.
     */
    KFormulaContainer* formula;
};

#endif
