/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
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
#ifndef KINLINETEXTOBJECTMANAGER_H
#define KINLINETEXTOBJECTMANAGER_H

#include "KInlineObject.h"
#include "KoBookmarkManager.h"
#include "KVariableManager.h"
#include "kodftext_export.h"

// Qt + kde
#include <QHash>
#include <QTextCharFormat>

class KCanvasBase;
class KTextLocator;
class QAction;

/**
 * A container to register all the inlineTextObjects with.
 * Inserting an inline-object in a QTextDocument should be done via this manager which will
 * insert a placeholder in the text and if you add the KInlineTextObjectManager to the
 * KTextDocumentLayout for that specific textDocument, your inline text object will get painted
 * properly.
 */
class KODFTEXT_EXPORT KInlineTextObjectManager : public QObject
{
    Q_OBJECT
// TODO, when to delete the inlineObject s
public:
    /// Constructor
    explicit KInlineTextObjectManager(QObject *parent = 0);

    /**
     * Retrieve a formerly added inline object based on the format.
     * @param format the textCharFormat
     */
    KInlineObject *inlineTextObject(const QTextCharFormat &format) const;
    /**
     * Retrieve a formerly added inline object based on the cursor position.
     * @param cursor the cursor which position is used. The anchor is ignored.
     */
    KInlineObject *inlineTextObject(const QTextCursor &cursor) const;

    /**
     * Retrieve a formerly added inline object based on the KInlineObject::id() of the object.
     * @param id the id assigned to the inline text object when it was added.
     */
    KInlineObject *inlineTextObject(int id) const;

    QList<KInlineObject*> inlineTextObjects() const;

    /**
     * Insert a new inline object into the manager as well as the document.
     * This method will cause a placeholder to be inserted into the text at cursor position,
     *  possibly replacing a selection.  The object will then be used as an inline
     * character and painted at the specified location in the text.
     * @param cursor the cursor which indicated the document and the position in that document
     *      where the inline object will be inserted.
     * @param object the inline object to insert.
     * @param charFormat specifies char format which will be used to insert inline object
     */
    void insertInlineObject(QTextCursor &cursor, KInlineObject *object, QTextCharFormat charFormat = QTextCharFormat());

    /**
     * Remove an inline object from this manager (as well as the document).
     * This method will also remove the placeholder for the inline object.
     * @param cursor the cursor which indicated the document and the position in that document
     *      where the inline object will be deleted
     * @return returns true if the inline object in the cursor position has been successfully
     *      deleted
     */
    bool removeInlineObject(QTextCursor &cursor);

    /// remove an inline object from this manager.
    void removeInlineObject(KInlineObject *object);

    /**
     * Set a property that may have changed which will be forwarded to all registered textObjects.
     * If the key has changed then all registered InlineObject instances that have stated to want
     * updates will get called with the change.
     * The property will be stored to allow it to be retrieved via the intProperty() and friends.
     * @see KInlineObject::propertyChangeListener()
     */
    void setProperty(KInlineObject::Property key, const QVariant &value);
    /// retrieve a propery
    QVariant property(KInlineObject::Property key) const;
    /// retrieve an int property
    int intProperty(KInlineObject::Property key) const;
    /// retrieve a bool property
    bool boolProperty(KInlineObject::Property key) const;
    /// retrieve a string property
    QString stringProperty(KInlineObject::Property key) const;
    /// remove a property from the store.
    void removeProperty(KInlineObject::Property key);

    /**
     * Return the variableManager.
     */
    const KVariableManager *variableManager() const;
    /**
     * Return the variableManager.
     */
    KVariableManager *variableManager();
    /**
     * Return the bookmarkManager.
     */
    KoBookmarkManager *bookmarkManager();

    /**
     * Create a list of actions that can be used to plug into a menu, for example.
     * This method internally uses KInlineObjectRegistry::createInsertVariableActions() but extends
     * the list with all registered variable-names.
     * Each of thse actions, when executed, will insert the relevant variable in the current text-position.
     * The actions assume that the text tool is selected, if thats not the case then they will silently fail.
     * @param host the canvas for which these actions are created.  Note that the actions will get these
     *  actions as a parent (for memory management purposes) as well.
     * @see KVariableManager
     */
    QList<QAction*> createInsertVariableActions(KCanvasBase *host) const;

    QList<KTextLocator*> textLocators() const;

public slots:
    void documentInformationUpdated(const QString &info, const QString &data);

signals:
    /**
     * Emitted whenever a propery is set and it turns out to be changed.
     */
    void propertyChanged(int, const QVariant &variant);

private:
    enum Properties {
        InlineInstanceId = 577297549 // If you change this, don't forget to change KCharacterStyle.h
    };

    QHash<int, KInlineObject*> m_objects;
    QList<KInlineObject*> m_listeners; // holds objects also in m_objects, but which want propertyChanges
    int m_lastObjectId;
    QHash<int, QVariant> m_properties;

    KVariableManager m_variableManager;
    KoBookmarkManager m_bookmarkManager;
};

Q_DECLARE_METATYPE(KInlineTextObjectManager*)
#endif
