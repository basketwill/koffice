/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KO_TEXT_RDF_CORE_H
#define KO_TEXT_RDF_CORE_H

#include "kodftext_export.h"

// this file can only be used by code that is built
// with soprano enabled.
#include <Soprano/Soprano>

class KOdfStore;
class KXmlWriter;

/**
 * @short Basic low level methods that are available to KOdfText objects
 *
 * Low level functionality such as streaming a Soprano::Model to and
 * from an ODF container is provided here so that both KoDocumentRdf
 * and other code in libs/kotext can share it.
 *
 * @author Ben Martin
 * @see KoDocumentRdf
 */
namespace KoTextRdfCore
{
/**
 * Load an Rdf linked list of statements. See saveList() for the
 * details. The return value of loadList() is the equivalent of
 * dataBNodeList in saveList().
 *
 * @see saveList()
 */
QList<Soprano::Statement> KODFTEXT_EXPORT loadList(Soprano::Model *model, Soprano::Node ListHeadSubject);

/**
 * Save an Rdf List of data nodes into the model. Rdf defines a
 * linked list format in the
 * http://www.w3.org/1999/02/22-rdf-syntax-ns URI namespace using
 * first/rest to link the current element with the "rest" of the
 * list. A scheme that will be familiar to many lisp programmers
 * car/cdr. Unfortunately dealing with such lists directly is
 * clumsy so this and loadList() let you store a list of data
 * nodes and these methods create all the boilerplate Rdf triples
 * to store/read a simple QList of nodes to Rdf properly. You
 * supply the list header node ListHeadSubject which is normally
 * the subject that you want the list associated with in Rdf. The
 * other nodes used in the internal structure of the Rdf list are
 * just random bnodes as shown below. If you have a previous,
 * existing list then this method will remove those nodes first so
 * that the Rdf model does not grow with disgarded list nodes over
 * time.
 *
 * The old list nodes are removed if they exist, and a new list is
 * created starting at ListHeadSubject, and linking all the nodes
 * in dataBNodeList using the supplied rdf context. Use the
 * loadList() method to get the list dataBNodeList back from the
 * model again.
 *
 * The result will be like:
@verbatim
ListHeadSubject 22-rdf-syntax-ns#first dataBNodeList[0]
ListHeadSubject 22-rdf-syntax-ns#rest  bnodeA
bnodeA          22-rdf-syntax-ns#first dataBNodeList[1]
bnodeA          22-rdf-syntax-ns#rest  bnodeB
...
bnodeZ          22-rdf-syntax-ns#first dataBNodeList[N]
bnodeZ          22-rdf-syntax-ns#rest  nil
@endverbatim
 *
 */
void KODFTEXT_EXPORT saveList(Soprano::Model *model, Soprano::Node ListHeadSubject,
        QList<Soprano::Node> &dataBNodeList, Soprano::Node context);

/**
 * Using model->removeStatements() will fail if the statement does not
 * exist in the model. This method is a bit sloppier in that it ignores
 * attempts to remove statements twice, or ones that no longer exist
 * in the model. This is handy for set based remove/add bulk updates
 * because you don't have to ensure that a statement is added only once
 * to the remove list.
 */
void KODFTEXT_EXPORT removeStatementsIfTheyExist(Soprano::Model *model,
        const QList<Soprano::Statement> &removeList);

/**
 * Given the Subj+Pred get the Object for the triple. If there are
 * more than one object, a random one from the possible candidates is
 * returned. This is mainly useful when you *know* there is only zero
 * or one object.
 */
Soprano::Node KODFTEXT_EXPORT getObject(Soprano::Model *model, Soprano::Node s, Soprano::Node p);

QString KODFTEXT_EXPORT getProperty(Soprano::Model *m,
                                  Soprano::Node subj,
                                  Soprano::Node pred,
                                  const QString &defval);
QString KODFTEXT_EXPORT optionalBindingAsString(Soprano::QueryResultIterator& it,
                                              const QString &bindingName,
                                              const QString &def = QString());
QByteArray KODFTEXT_EXPORT fileToByteArray(const QString &fileName);

}
#endif

