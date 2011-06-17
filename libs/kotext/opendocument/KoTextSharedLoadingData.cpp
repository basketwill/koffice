/* This file is part of the KDE project
 * Copyright (C) 2004-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007, 2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "KoTextSharedLoadingData.h"

#include <QString>
#include <QHash>

#include <kdebug.h>

#include <KXmlReader.h>
#include <KOdfXmlNS.h>
#include <KOdfStylesReader.h>
#include <KOdfLoadingContext.h>
#include <KShapeLoadingContext.h>

#include "styles/KStyleManager.h"
#include "styles/KParagraphStyle.h"
#include "styles/KCharacterStyle.h"
#include "styles/KListStyle.h"
#include "styles/KListLevelProperties.h"
#include "styles/KoTableStyle.h"
#include "styles/KTableColumnStyle.h"
#include "styles/KTableRowStyle.h"
#include "styles/KTableCellStyle.h"
#include "styles/KSectionStyle.h"

class KoTextSharedLoadingData::Private
{
public:
    Private()
    : applicationDefaultStyle(0)
    {}
    ~Private() {
        qDeleteAll(paragraphStylesToDelete);
        qDeleteAll(characterStylesToDelete);
        qDeleteAll(listStylesToDelete);
        qDeleteAll(tableStylesToDelete);
        qDeleteAll(tableCellStylesToDelete);
        qDeleteAll(tableColumnStylesToDelete);
        qDeleteAll(tableRowStylesToDelete);
        qDeleteAll(sectionStylesToDelete);
    }

    // It is possible that automatic-styles in content.xml and styles.xml have the same name
    // within the same family. Therefore we have to keep them separate. The office:styles are
    // added to the autostyles so that only one lookup is needed to get the style. This is
    // about 30% faster than having a special data structure for office:styles.
    QHash<QString, KParagraphStyle *> paragraphContentDotXmlStyles;
    QHash<QString, KCharacterStyle *> characterContentDotXmlStyles;
    QHash<QString, KListStyle *> listContentDotXmlStyles;
    QHash<QString, KoTableStyle *> tableContentDotXmlStyles;
    QHash<QString, KTableColumnStyle *> tableColumnContentDotXmlStyles;
    QHash<QString, KTableRowStyle *> tableRowContentDotXmlStyles;
    QHash<QString, KTableCellStyle *> tableCellContentDotXmlStyles;
    QHash<QString, KSectionStyle *> sectionContentDotXmlStyles;
    QHash<QString, KParagraphStyle *> paragraphStylesDotXmlStyles;
    QHash<QString, KCharacterStyle *> characterStylesDotXmlStyles;
    QHash<QString, KListStyle *> listStylesDotXmlStyles;
    QHash<QString, KoTableStyle *> tableStylesDotXmlStyles;
    QHash<QString, KTableColumnStyle *> tableColumnStylesDotXmlStyles;
    QHash<QString, KTableRowStyle *> tableRowStylesDotXmlStyles;
    QHash<QString, KTableCellStyle *> tableCellStylesDotXmlStyles;
    QHash<QString, KSectionStyle *> sectionStylesDotXmlStyles;

    QList<KParagraphStyle *> paragraphStylesToDelete;
    QList<KCharacterStyle *> characterStylesToDelete;
    QList<KListStyle *> listStylesToDelete;
    QList<KoTableStyle *> tableStylesToDelete;
    QList<KTableCellStyle *> tableCellStylesToDelete;
    QList<KTableColumnStyle *> tableColumnStylesToDelete;
    QList<KTableRowStyle *> tableRowStylesToDelete;
    QList<KSectionStyle *> sectionStylesToDelete;
    QHash<QString, KParagraphStyle*> namedParagraphStyles;

    KParagraphStyle *applicationDefaultStyle;
};

KoTextSharedLoadingData::KoTextSharedLoadingData()
        : d(new Private())
{
}

KoTextSharedLoadingData::~KoTextSharedLoadingData()
{
    delete d;
}

void KoTextSharedLoadingData::addDefaultParagraphStyle(KShapeLoadingContext &context, const KXmlElement *styleElem, const KXmlElement *appDefault, KStyleManager *styleManager)
{
    // first fill with defaultstyles.xml
    const KXmlElement *appDef = context.odfLoadingContext().defaultStylesReader().defaultStyle("paragraph");
    if (appDef) {
        KCharacterStyle *style = styleManager->defaultParagraphStyle()->characterStyle();
        KOdfStyleStack defaultStyleStack;
        defaultStyleStack.push(*appDef);
        defaultStyleStack.setTypeProperties("text");
        style->loadOdfProperties(defaultStyleStack);
    }

    if (styleManager && styleElem) {
        styleManager->defaultParagraphStyle()->loadOdf(styleElem, context);
    }
    else if (styleManager && appDefault) {
        styleManager->defaultParagraphStyle()->loadOdf(appDefault, context);
    }
    if (styleManager)
        d->applicationDefaultStyle = styleManager->defaultParagraphStyle();
}

void KoTextSharedLoadingData::loadOdfStyles(KShapeLoadingContext &shapeContext, KStyleManager *styleManager)
{
    KOdfLoadingContext &context = shapeContext.odfLoadingContext();

    // only add styles of office:styles to the style manager
    addCharacterStyles(shapeContext, context.stylesReader().customStyles("text").values(), ContentDotXml | StylesDotXml, styleManager);
    addCharacterStyles(shapeContext, context.stylesReader().autoStyles("text", true).values(), StylesDotXml);
    addCharacterStyles(shapeContext, context.stylesReader().autoStyles("text").values(), ContentDotXml);

    addListStyles(shapeContext, context.stylesReader().autoStyles("list").values(), ContentDotXml);
    addListStyles(shapeContext, context.stylesReader().autoStyles("list", true).values(), StylesDotXml);
    addListStyles(shapeContext, context.stylesReader().customStyles("list").values(), ContentDotXml | StylesDotXml, styleManager);

    addDefaultParagraphStyle(shapeContext, context.stylesReader().defaultStyle("paragraph"), context.defaultStylesReader().defaultStyle("paragraph"), styleManager);

    // adding all the styles in order of dependency; automatic styles can have a parent in the named styles, so load the named styles first.

    // add office:styles from styles.xml to paragraphContentDotXmlStyles, paragraphStylesDotXmlStyles and styleManager
    // now all styles referencable from the body in content.xml is in paragraphContentDotXmlStyles
    addParagraphStyles(shapeContext, context.stylesReader().customStyles("paragraph").values(), ContentDotXml | StylesDotXml, styleManager);
    // add office:automatic-styles in styles.xml to paragraphStylesDotXmlStyles
    addParagraphStyles(shapeContext, context.stylesReader().autoStyles("paragraph", true).values(), StylesDotXml);
    // add office:automatic-styles in content.xml to paragraphContentDotXmlStyles
    addParagraphStyles(shapeContext, context.stylesReader().autoStyles("paragraph").values(), ContentDotXml);

    addTableStyles(context, context.stylesReader().autoStyles("table").values(), ContentDotXml);
    addTableStyles(context, context.stylesReader().autoStyles("table", true).values(), StylesDotXml);
    addTableStyles(context, context.stylesReader().customStyles("table").values(), ContentDotXml | StylesDotXml, styleManager);

    addTableColumnStyles(context, context.stylesReader().autoStyles("table-column").values(), ContentDotXml);
    addTableColumnStyles(context, context.stylesReader().autoStyles("table-column", true).values(), StylesDotXml);
    addTableColumnStyles(context, context.stylesReader().customStyles("table-column").values(), ContentDotXml | StylesDotXml, styleManager);

    addTableRowStyles(context, context.stylesReader().autoStyles("table-row").values(), ContentDotXml);
    addTableRowStyles(context, context.stylesReader().autoStyles("table-row", true).values(), StylesDotXml);
    addTableRowStyles(context, context.stylesReader().customStyles("table-row").values(), ContentDotXml | StylesDotXml, styleManager);

    addTableCellStyles(context, context.stylesReader().autoStyles("table-cell").values(), ContentDotXml);
    addTableCellStyles(context, context.stylesReader().autoStyles("table-cell", true).values(), StylesDotXml);
    addTableCellStyles(context, context.stylesReader().customStyles("table-cell").values(), ContentDotXml | StylesDotXml, styleManager);

    addSectionStyles(context, context.stylesReader().autoStyles("section").values(), ContentDotXml);
    addSectionStyles(context, context.stylesReader().autoStyles("section", true).values(), StylesDotXml);
    addSectionStyles(context, context.stylesReader().customStyles("section").values(), ContentDotXml | StylesDotXml, styleManager);

    addOutlineStyle(shapeContext, styleManager);

    kDebug(32500) << "content.xml: paragraph styles" << d->paragraphContentDotXmlStyles.count() << "character styles" << d->characterContentDotXmlStyles.count();
    kDebug(32500) << "styles.xml:  paragraph styles" << d->paragraphStylesDotXmlStyles.count() << "character styles" << d->characterStylesDotXmlStyles.count();
}

void KoTextSharedLoadingData::addParagraphStyles(KShapeLoadingContext &context, QList<KXmlElement*> styleElements,
        int styleTypes, KStyleManager *styleManager)
{
    QList<QPair<QString, KParagraphStyle *> > paragraphStyles(loadParagraphStyles(context, styleElements, styleTypes, styleManager));

    QList<QPair<QString, KParagraphStyle *> >::iterator it(paragraphStyles.begin());
    for (; it != paragraphStyles.end(); ++it) {
        KParagraphStyle *style = it->second;
        Q_ASSERT(style);
        if (d->applicationDefaultStyle && style->parentStyle() == 0) {
            Q_ASSERT(d->applicationDefaultStyle != style);
            style->setParentStyle(d->applicationDefaultStyle);
        }
        if (styleTypes & ContentDotXml) {
            d->paragraphContentDotXmlStyles.insert(it->first, style);
        }
        if (styleTypes & StylesDotXml) {
            d->paragraphStylesDotXmlStyles.insert(it->first, style);
        }
    }
}

QList<QPair<QString, KParagraphStyle *> > KoTextSharedLoadingData::loadParagraphStyles(KShapeLoadingContext &context, QList<KXmlElement*> styleElements,
        int styleTypes, KStyleManager *styleManager)
{
    QList<QPair<QString, KParagraphStyle *> > paragraphStyles;
    QHash<KParagraphStyle*,QString> nextStyles;
    QHash<KParagraphStyle*,QString> parentStyles;

    foreach(KXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KOdfXmlNS::style, "name");
        KParagraphStyle *parastyle = 0;
        if (styleManager) {
            QString displayName = styleElem->attributeNS(KOdfXmlNS::style, "display-name", name);
            parastyle = styleManager->paragraphStyle(displayName);
        }
        if (parastyle == 0) {
            parastyle = new KParagraphStyle();
            parastyle->loadOdf(styleElem, context);
            QString listStyleName = styleElem->attributeNS(KOdfXmlNS::style, "list-style-name");
            KListStyle *list = listStyle(listStyleName, styleTypes & StylesDotXml);
            if (list) {
                KListStyle *newListStyle = new KListStyle(parastyle);
                newListStyle->copyProperties(list);
                parastyle->setListStyle(newListStyle);
            }
            if (styleManager)
                styleManager->add(parastyle);
            else
                d->paragraphStylesToDelete.append(parastyle);
        }
        paragraphStyles.append(QPair<QString, KParagraphStyle *>(name, parastyle));
        d->namedParagraphStyles.insert(name, parastyle);

        if (styleElem->hasAttributeNS(KOdfXmlNS::style, "next-style-name"))
            nextStyles.insert(parastyle, styleElem->attributeNS(KOdfXmlNS::style, "next-style-name"));
        if (styleElem->hasAttributeNS(KOdfXmlNS::style, "parent-style-name"))
            parentStyles.insert(parastyle, styleElem->attributeNS(KOdfXmlNS::style, "parent-style-name"));

        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
    }

    // second pass; resolve all the 'next-style's and parent-style's.
    foreach (KParagraphStyle *style, nextStyles.keys()) {
        KParagraphStyle *next = d->namedParagraphStyles.value(nextStyles.value(style));
        if (next && next->styleId() >= 0)
            style->setNextStyle(next->styleId());
    }
    foreach (KParagraphStyle *style, parentStyles.keys()) {
        KParagraphStyle *parent = d->namedParagraphStyles.value(parentStyles.value(style));
        if (parent)
            style->setParentStyle(parent);
    }

    return paragraphStyles;
}

void KoTextSharedLoadingData::addCharacterStyles(KShapeLoadingContext &context, QList<KXmlElement*> styleElements,
        int styleTypes, KStyleManager *styleManager)
{
    QList<OdfCharStyle> characterStyles(loadCharacterStyles(context, styleElements, styleManager));

    foreach (const OdfCharStyle &odfStyle, characterStyles) {
        if (styleTypes & ContentDotXml) {
            d->characterContentDotXmlStyles.insert(odfStyle.odfName, odfStyle.style);
        }
        if (styleTypes & StylesDotXml) {
            d->characterStylesDotXmlStyles.insert(odfStyle.odfName, odfStyle.style);
        }

        if (styleManager) {
            styleManager->add(odfStyle.style);
        } else {
            if (!odfStyle.parentStyle.isEmpty()) { // an auto style with a parent.
                // lets find the parent and set the styleId of that one on the auto-style too.
                // this will have the effect that whereever the autostyle is applied, it will
                // cause the parent style-id to be applied. So we don't loose this info.
                KCharacterStyle *parent = characterStyle(odfStyle.parentStyle, false);
                if (!parent)
                    parent = characterStyle(odfStyle.parentStyle, true); // try harder
                if (parent)
                    odfStyle.style->setStyleId(parent->styleId());
            }
            d->characterStylesToDelete.append(odfStyle.style);
        }
    }
}

QList<KoTextSharedLoadingData::OdfCharStyle> KoTextSharedLoadingData::loadCharacterStyles(KShapeLoadingContext &shapeContext, QList<KXmlElement*> styleElements, KStyleManager *styleManager)
{
    QList<OdfCharStyle> characterStyles;
    KOdfLoadingContext &context = shapeContext.odfLoadingContext();

    foreach(KXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KOdfXmlNS::style, "name");
        QString displayName = styleElem->attributeNS(KOdfXmlNS::style, "display-name", name);
        kDebug(32500) << "styleName =" << name << "styleDisplayName =" << displayName;

        KCharacterStyle *characterStyle = 0;
        if (styleManager)
            characterStyle = styleManager->characterStyle(displayName);
        if (characterStyle == 0) {
            context.styleStack().save();
            context.styleStack().push(*styleElem);

            context.styleStack().setTypeProperties("text");

            characterStyle = new KCharacterStyle();
            characterStyle->setName(displayName);
            characterStyle->loadOdf(shapeContext);
            context.styleStack().restore();
        }

        OdfCharStyle answer;
        answer.odfName = name;
        answer.parentStyle = styleElem->attributeNS(KOdfXmlNS::style, "parent-style-name");
        answer.style = characterStyle;
        characterStyles.append(answer);
    }
    return characterStyles;
}

void KoTextSharedLoadingData::addListStyles(KShapeLoadingContext &context, QList<KXmlElement*> styleElements,
                                            int styleTypes, KStyleManager *styleManager)
{
    QList<QPair<QString, KListStyle *> > listStyles(loadListStyles(context, styleElements, styleManager));

    QList<QPair<QString, KListStyle *> >::iterator it(listStyles.begin());
    for (; it != listStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->listContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->listStylesDotXmlStyles.insert(it->first, it->second);
        }
        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memory
        if (styleManager) {
            styleManager->add(it->second);
        } else {
            d->listStylesToDelete.append(it->second);
        }
    }
}

QList<QPair<QString, KListStyle *> > KoTextSharedLoadingData::loadListStyles(KShapeLoadingContext &context, QList<KXmlElement*> styleElements, KStyleManager *styleManager)
{
    QList<QPair<QString, KListStyle *> > listStyles;

    foreach (KXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KOdfXmlNS::style, "name");
        QString displayName = styleElem->attributeNS(KOdfXmlNS::style, "display-name", name);
        KListStyle *liststyle = 0;
        if (styleManager)
            liststyle = styleManager->listStyle(displayName);
        if (liststyle == 0) {
            liststyle = new KListStyle();
            liststyle->setName(displayName);
            liststyle->loadOdf(context, *styleElem);
        }
        listStyles.append(QPair<QString, KListStyle *>(name, liststyle));
    }
    return listStyles;
}

void KoTextSharedLoadingData::addTableStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements,
                                            int styleTypes, KStyleManager *styleManager)
{
    QList<QPair<QString, KoTableStyle *> > tableStyles(loadTableStyles(context, styleElements));

    QList<QPair<QString, KoTableStyle *> >::iterator it(tableStyles.begin());
    for (; it != tableStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->tableContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->tableStylesDotXmlStyles.insert(it->first, it->second);
        }
        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if (styleManager) {
            styleManager->add(it->second);
        } else {
            d->tableStylesToDelete.append(it->second);
        }
    }
}

QList<QPair<QString, KoTableStyle *> > KoTextSharedLoadingData::loadTableStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements)
{
    QList<QPair<QString, KoTableStyle *> > tableStyles;

    foreach(KXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KOdfXmlNS::style, "name", QString());
        // nah don't think this is it: context.fillStyleStack(*styleElem, KOdfXmlNS::style, "style-name", "table");
        KoTableStyle *tablestyle = new KoTableStyle();
        tablestyle->loadOdf(styleElem, context);
        tableStyles.append(QPair<QString, KoTableStyle *>(name, tablestyle));
    }
    return tableStyles;
}

void KoTextSharedLoadingData::addTableColumnStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements,
                                            int styleTypes, KStyleManager *styleManager)
{
    QList<QPair<QString, KTableColumnStyle *> > tableColumnStyles(loadTableColumnStyles(context, styleElements));

    QList<QPair<QString, KTableColumnStyle *> >::iterator it(tableColumnStyles.begin());
    for (; it != tableColumnStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->tableColumnContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->tableColumnStylesDotXmlStyles.insert(it->first, it->second);
        }
        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if (styleManager) {
            styleManager->add(it->second);
        } else {
            d->tableColumnStylesToDelete.append(it->second);
        }
    }
}

QList<QPair<QString, KTableColumnStyle *> > KoTextSharedLoadingData::loadTableColumnStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements)
{
    QList<QPair<QString, KTableColumnStyle *> > tableColumnStyles;

    foreach(KXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KOdfXmlNS::style, "name", QString());
        // nah don't think this is it: context.fillStyleStack(*styleElem, KOdfXmlNS::style, "style-name", "table");
        KTableColumnStyle *tablecolumnstyle = new KTableColumnStyle();
        tablecolumnstyle->loadOdf(styleElem, context);
        tableColumnStyles.append(QPair<QString, KTableColumnStyle *>(name, tablecolumnstyle));
    }
    return tableColumnStyles;
}

void KoTextSharedLoadingData::addTableRowStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements,
                                            int styleTypes, KStyleManager *styleManager)
{
    QList<QPair<QString, KTableRowStyle *> > tableRowStyles(loadTableRowStyles(context, styleElements));

    QList<QPair<QString, KTableRowStyle *> >::iterator it(tableRowStyles.begin());
    for (; it != tableRowStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->tableRowContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->tableRowStylesDotXmlStyles.insert(it->first, it->second);
        }
        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if (styleManager) {
            styleManager->add(it->second);
        } else {
            d->tableRowStylesToDelete.append(it->second);
        }
    }
}

QList<QPair<QString, KTableRowStyle *> > KoTextSharedLoadingData::loadTableRowStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements)
{
    QList<QPair<QString, KTableRowStyle *> > tableRowStyles;

    foreach(KXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KOdfXmlNS::style, "name", QString());
        // nah don't think this is it: context.fillStyleStack(*styleElem, KOdfXmlNS::style, "style-name", "table");
        KTableRowStyle *tablerowstyle = new KTableRowStyle();
        tablerowstyle->loadOdf(styleElem, context);
        tableRowStyles.append(QPair<QString, KTableRowStyle *>(name, tablerowstyle));
    }
    return tableRowStyles;
}

void KoTextSharedLoadingData::addTableCellStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements,
                                            int styleTypes, KStyleManager *styleManager)
{
    QList<QPair<QString, KTableCellStyle *> > tableCellStyles(loadTableCellStyles(context, styleElements));

    QList<QPair<QString, KTableCellStyle *> >::iterator it(tableCellStyles.begin());
    for (; it != tableCellStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->tableCellContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->tableCellStylesDotXmlStyles.insert(it->first, it->second);
        }
        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if (styleManager) {
            styleManager->add(it->second);
        } else {
            d->tableCellStylesToDelete.append(it->second);
        }
    }
}

QList<QPair<QString, KTableCellStyle *> > KoTextSharedLoadingData::loadTableCellStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements)
{
    QList<QPair<QString, KTableCellStyle *> > tableCellStyles;

    foreach(KXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KOdfXmlNS::style, "name", QString());
        // nah don't think this is it: context.fillStyleStack(*styleElem, KOdfXmlNS::style, "style-name", "table");
        KTableCellStyle *tablecellstyle = new KTableCellStyle();
        tablecellstyle->loadOdf(styleElem, context);
        tableCellStyles.append(QPair<QString, KTableCellStyle *>(name, tablecellstyle));
    }
    return tableCellStyles;
}

void KoTextSharedLoadingData::addSectionStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements,
                                            int styleTypes, KStyleManager *styleManager)
{
    QList<QPair<QString, KSectionStyle *> > sectionStyles(loadSectionStyles(context, styleElements));

    QList<QPair<QString, KSectionStyle *> >::iterator it(sectionStyles.begin());
    for (; it != sectionStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->sectionContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->sectionStylesDotXmlStyles.insert(it->first, it->second);
        }
        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if (styleManager) {
            styleManager->add(it->second);
        } else {
            d->sectionStylesToDelete.append(it->second);
        }
    }
}

QList<QPair<QString, KSectionStyle *> > KoTextSharedLoadingData::loadSectionStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements)
{
    QList<QPair<QString, KSectionStyle *> > sectionStyles;

    foreach(KXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KOdfXmlNS::style, "name", QString());
        // nah don't think this is it: context.fillStyleStack(*styleElem, KOdfXmlNS::style, "style-name", "table");
        KSectionStyle *sectionstyle = new KSectionStyle();
        sectionstyle->loadOdf(styleElem, context);
        sectionStyles.append(QPair<QString, KSectionStyle *>(name, sectionstyle));
    }
    return sectionStyles;
}

void KoTextSharedLoadingData::addOutlineStyle(KShapeLoadingContext &context, KStyleManager *styleManager)
{
    // outline-styles used e.g. for headers
    KXmlElement outlineStyleElem = KoXml::namedItemNS(context.odfLoadingContext().stylesReader().officeStyle(), KOdfXmlNS::text, "outline-style");
    if (styleManager && outlineStyleElem.isElement()) {
        KListStyle *outlineStyle = new KListStyle();
        outlineStyle->loadOdf(context, outlineStyleElem);
        styleManager->setOutlineStyle(outlineStyle); // style manager owns it now. he will take care of deleting it.
    }
}

KParagraphStyle *KoTextSharedLoadingData::paragraphStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->paragraphStylesDotXmlStyles.value(name) : d->paragraphContentDotXmlStyles.value(name);
}

KCharacterStyle *KoTextSharedLoadingData::characterStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->characterStylesDotXmlStyles.value(name) : d->characterContentDotXmlStyles.value(name);
}

QList<KCharacterStyle*> KoTextSharedLoadingData::characterStyles(bool stylesDotXml) const
{
    return stylesDotXml ? d->characterStylesDotXmlStyles.values() : d->characterContentDotXmlStyles.values();
}

KListStyle *KoTextSharedLoadingData::listStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->listStylesDotXmlStyles.value(name) : d->listContentDotXmlStyles.value(name);
}

KoTableStyle *KoTextSharedLoadingData::tableStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->tableStylesDotXmlStyles.value(name) : d->tableContentDotXmlStyles.value(name);
}

KTableColumnStyle *KoTextSharedLoadingData::tableColumnStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->tableColumnStylesDotXmlStyles.value(name) : d->tableColumnContentDotXmlStyles.value(name);
}

KTableRowStyle *KoTextSharedLoadingData::tableRowStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->tableRowStylesDotXmlStyles.value(name) : d->tableRowContentDotXmlStyles.value(name);
}

KTableCellStyle *KoTextSharedLoadingData::tableCellStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->tableCellStylesDotXmlStyles.value(name) : d->tableCellContentDotXmlStyles.value(name);
}

KSectionStyle *KoTextSharedLoadingData::sectionStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->sectionStylesDotXmlStyles.value(name) : d->sectionContentDotXmlStyles.value(name);
}

void KoTextSharedLoadingData::shapeInserted(KShape *shape, const KXmlElement &element, KShapeLoadingContext &/*context*/)
{
    Q_UNUSED(shape);
    Q_UNUSED(element);
}
