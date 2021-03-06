/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KSHAPEFACTORYBASE_H
#define KSHAPEFACTORYBASE_H

#include <QStringList>
#include <QString>
#include <QWidget>
#include <QList>

#include "flake_export.h"

#include <KXmlReader.h>

class KShape;
class KProperties;
class KShapeConfigWidgetBase;
class KShapeLoadingContext;
class KDataCenterBase;
class KResourceManager;
class KCanvasBase;

#define SHAPETEMPLATE_MIMETYPE "application/x-flake-shapetemplate"
#define SHAPEID_MIMETYPE "application/x-flake-shapeId"

/**
 * Contains a KProperties object that describes the settings of a
 * particular variant of a shape object, together with a name, a description
 * and an icon for use in the user interface.
 */
struct FLAKE_EXPORT KoShapeTemplate {
    KoShapeTemplate() {
        properties = 0;
    }
    QString id;         ///< The id of the shape
    QString templateId;         ///< The id of this particular template - only has to be unique with the shape
    QString name;       ///< The name to be shown for this template
    QString family;       ///< The family of the shape (possible values are: "funny","arrow")
    QString toolTip;    ///< The tooltip text for the template
    QString icon;       ///< Icon name
    /**
     * The properties which, when passed to the KShapeFactoryBase::createShape() method
     * result in the shape this template represents.
     */
    KProperties *properties;
};

/**
 * A factory for KShape objects.
 * The baseclass for all shape plugins. Each plugin that ships a KShape should also
 * ship a factory. That factory will extend this class and set variable data like
 * a toolTip and icon in the constructor of that extending class.
 *
 * An example usage would be:
@code
class MyShapeFactory : public KShapeFactoryBase {
public:
    MyShapeFactory(QObject *parent)
        : KShapeFactoryBase(parent, "MyShape", i18n("My Shape")) {
        setToolTip(i18n("A nice shape"));
    }
    ~MyShapeFactory() {}
    // more methods here
};
@endcode

 * After you created the factory you should create a plugin that can announce the factory to the
 * KShapeRegistry.  See the KoPluginLoader as well.
 */
class FLAKE_EXPORT KShapeFactoryBase : public QObject
{
    Q_OBJECT
public:

    /**
     * Create the new factory
     * @param parent the parent QObject for memory management usage.
     * @param id a string that will be used internally for referencing the
     *   shape. See KToolFactoryBase::activationShapeId()
     * @param name the user visible name of the shape this factory creates.
     */
    KShapeFactoryBase(QObject *parent, const QString &id, const QString &name);
    virtual ~KShapeFactoryBase();

    virtual KShapeConfigWidgetBase *createConfigWidget(KCanvasBase *canvas) {
        Q_UNUSED(canvas);
        return 0;
    }

    /**
     * return the id for the shape this factory creates.
     * @return the id for the shape this factory creates.
     */
    QString id() const;
    /**
     * Return all the templates this factory knows about.
     * Each template shows a different way to create a shape this factory is specialized in.
     */
    QList<KoShapeTemplate> templates() const;
    /**
     * return a translated tooltip Text for a selector of shapes
     * @return a translated tooltip Text
     */
    QString toolTip() const;
    /**
     * return the basename of the icon for a selector of shapes
     * @return the basename of the icon for a selector of shapes
     */
    QString icon() const;
    /**
     * return the user visible (and translated) name to be seen by the user.
     * @return the user visible (and translated) name to be seen by the user.
     */
    QString name() const;
    /**
     * return the non-visible name of the family the default shape belongs to.
     * @return the family name.
     */
    QString family() const;
    /// lower prio means the shape is more generic and will be checked later
    int loadingPriority() const;

    /**
     * The list of namespaces to the supported elements the factory supports.
     */
    QList<QPair<QString, QStringList> > odfElements() const;

    /// returns true if this shapeFactory is able to load the ODF type
    /// started at argument element. ('draw:line' / 'draw:frame' / etc)
    virtual bool supports(const KXmlElement &element, KShapeLoadingContext &context) const = 0;

    /**
     * The hidden boolean requests if the shape should be hidden in the
     * shape selector or shown with all its templates.
     * The default is false
     * @see setHidden()
     */
    bool isHidden() const;

    /**
     * This slot is called whenever there is a new document resource
     * manager that is created. The factory may reimplement this in
     * order to get existing resources or put factory specific resources in.
     * In case the factory creates new resources it is adviced to parent
     * them to the manager (which is a QObject) for memory management
     * purposes.
     * @param manager the new manager
     */
    virtual void newDocumentResourceManager(KResourceManager *manager);

    /**
     * This method should be implemented by factories to create a shape that the user
     * gets when doing a base insert. For example from a script.  The created shape
     * should have its values set to good defaults that the user can then adjust further if
     * needed.  Including the KShape:setShapeId(), with the Id from this factory
     * The default shape position is not relevant, it will be moved by the caller.
     * @param documentResources the resources manager that has all the document wide
     *      resources which can be used to create the object.
     * @return a new shape
     * @see createShape() newDocumentResourceManager()
     */
    virtual KShape *createDefaultShape(KResourceManager *documentResources = 0) const = 0;

    /**
     * This method should be implemented by factories to create a shape based on a set of
     * properties that are specifically made for this shape-type.
     * This method should also set this factories shapeId on the shape using KShape::setShapeId()
     * The default implementation just ignores 'params' and calls createDefaultShape()
     * @return a new shape
     * @param params the properties object is the same as added in the addTemplate() call
     * @param documentResources the resources manager that has all the document wide
     *      resources which can be used to create the object.
     * @see createDefaultShape() newDocumentResourceManager()
     * @see KoShapeTemplate::properties
     */
    virtual KShape *createShape(const KProperties *params, KResourceManager *documentResources = 0) const;

protected:

    /**
     * Add a template with the properties of a speficic type of shape this factory can generate
     * using the createShape() method.
     * @param params the new template this factory knows to produce
     */
    void addTemplate(const KoShapeTemplate &params);

    /**
     * Set the tooltip to be used for a selector of shapes
     * @param tooltip the tooltip
     */
    void setToolTip(const QString &tooltip);

    /**
     * Set an icon to be used in a selector of shapes
     * @param iconName the basename (without extension) of the icon
     * @see KIconLoader
     */
    void setIcon(const QString &iconName);

    /**
     * Set the family name of the default shape
     * @param family the family name of the default shape this factory creates.
     *   for example "funny", "arrows", "geometrics". Use "" for default
     */
    void setFamily(const QString &family);

    /**
     * Set the loading priority for this icon; higher priority means
     * the shape is more specific which means it will be earlier in
     * the queue to try loading a particular odf element.
     */
    void setLoadingPriority(int priority);

    /**
     * Set the namespace and element tags used for quick checking whether this shapefactory
     * is able to create a shape from xml identified by this element
     * name.
     *
     * @param nameSpace the ODF name space (like
     * urn:oasis:names:tc:opendocument:xmlns:text:1.0,
     * take it from KOdfXmlNS.h)
     * @param elementNames the name of the element itself, like "draw"
     *
     */
    void setOdfElementNames(const QString &nameSpace, const QStringList &elementNames);

    /**
     * Set the namespaces and according element tags used for quick checking whether this shapefactory
     * is able to create a shape from xml identified by this element
     * name.
     *
     * @param elementNamesList containing a list of namespace (like
     * urn:oasis:names:tc:opendocument:xmlns:text:1.0,
     * take it from KOdfXmlNS.h) to a list of elementName of the element itself, like "draw"
     */
    void setOdfElements(const QList<QPair<QString, QStringList> > &elementNamesList);

    /**
     * The hidden boolean requests if the shape should be hidden in the
     * shape selector or shown with all its templates.
     * The default is false
     * @see hidden()
     */
    void setHidden(bool hidden);

private:
    class Private;
    Private * const d;
};

#endif
