/* This file is part of the KDE project
 * Copyright (C) 2006, 2009-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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

#include "KStyleManager.h"
#include "KStyleManager_p.h"
#include "KParagraphStyle.h"
#include "KCharacterStyle.h"
#include "KListStyle.h"
#include "KListLevelProperties.h"
#include "KTableStyle.h"
#include "KTableColumnStyle.h"
#include "KTableRowStyle.h"
#include "KTableCellStyle.h"
#include "KSectionStyle.h"
#include "ChangeFollower_p.h"
#include "KTextDocument.h"

#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>

#include <QTimer>
#include <QUrl>
#include <kdebug.h>
#include <klocale.h>

// static
int KStyleManagerPrivate::s_stylesNumber = 100;

KStyleManagerPrivate::KStyleManagerPrivate()
    : updateTriggered(false),
    defaultParagraphStyle(0),
    defaultListStyle(0),
    outlineStyle(0)
{
}

KStyleManagerPrivate::~KStyleManagerPrivate()
{
    qDeleteAll(automaticListStyles);
}

void KStyleManagerPrivate::refreshUnsetStoreFor(int key)
{
    QMap<int, QVariant> keys;
    KParagraphStyle *parag = paragStyles.value(key);
    if (parag) {
        QTextBlockFormat bf;
        parag->applyStyle(bf);
        keys = bf.properties();
    } else {
        KCharacterStyle *charStyle = charStyles.value(key);
        if (charStyle) {
            QTextCharFormat cf;
            charStyle->applyStyle(cf);
            // find all relevant char styles downwards (to root).
	    for (QHash<int, KParagraphStyle*>::const_iterator it = paragStyles.constBegin(); it != paragStyles.constEnd(); ++it) {
                KParagraphStyle *ps = it.value();
                if (ps->characterStyle() == charStyle) { // he uses us, lets apply all parents too
                    KParagraphStyle *parent = ps->parentStyle();
                    while (parent) {
                        parent->characterStyle()->applyStyle(cf);
                        parent = parent->parentStyle();
                    }
                }
            }
            keys = cf.properties();
        }
    }
    unsetStore.insert(key, keys);
}

void KStyleManagerPrivate::requestUpdateForChildren(KParagraphStyle *style)
{
    const int charId = style->characterStyle()->styleId();
    if (!updateQueue.contains(charId))
        updateQueue.insert(charId, unsetStore.value(charId));

    for (QHash<int, KParagraphStyle*>::const_iterator it = paragStyles.constBegin(); it != paragStyles.constEnd(); ++it) {
        KParagraphStyle *ps = it.value();
        if (ps->parentStyle() == style)
            requestUpdateForChildren(ps);
    }
}

void KStyleManagerPrivate::requestFireUpdate(KStyleManager *q)
{
    if (updateTriggered)
        return;
    QTimer::singleShot(0, q, SLOT(updateAlteredStyles()));
    updateTriggered = true;
}

void KStyleManagerPrivate::updateAlteredStyles()
{
    foreach (ChangeFollower *cf, documentUpdaterProxies) {
        cf->processUpdates(updateQueue);
    }
    for (QMap<int, QMap<int, QVariant> >::const_iterator it = updateQueue.constBegin(); it != updateQueue.constEnd(); ++it) {
        refreshUnsetStoreFor(it.key());
    }
    updateQueue.clear();
    updateTriggered = false;
}

// ---------------------------------------------------

KStyleManager::KStyleManager(QObject *parent)
        : QObject(parent), d(new KStyleManagerPrivate())
{
    d->defaultParagraphStyle = new KParagraphStyle(this);
    d->defaultParagraphStyle->setName("[No Paragraph Style]");
    add(d->defaultParagraphStyle);
    KCharacterStyle *charStyle = d->defaultParagraphStyle->characterStyle();
    charStyle->setFontPointSize(12); // hardcoded defaults. use defaultstyles.xml to overide
    charStyle->setFontFamily(QLatin1String("Sans Serif"));
    charStyle->setForeground(QBrush(Qt::black));

    d->defaultListStyle = new KListStyle(this);
    KListLevelProperties llp;
    llp.setLevel(1);
    llp.setStartValue(1);
    llp.setStyle(KListStyle::DecimalItem);
    llp.setListItemSuffix(".");
    d->defaultListStyle->setLevelProperties(llp);
}

KStyleManager::~KStyleManager()
{
    delete d;
}

void KStyleManager::saveOdf(KOdfGenericStyles& mainStyles)
{
    // saveOdfDefaultStyles
    KOdfGenericStyle defStyle(KOdfGenericStyle::ParagraphStyle, "paragraph");
    defStyle.setDefaultStyle(true);
    d->defaultParagraphStyle->saveOdf(defStyle, mainStyles);
    mainStyles.insert(defStyle);

    // don't save character styles that are already saved as part of a paragraph style
    QSet<KCharacterStyle*> characterParagraphStyles;
    QHash<KParagraphStyle*, QString> savedNames;
    foreach(KParagraphStyle *paragraphStyle, d->paragStyles) {
        if (paragraphStyle == d->defaultParagraphStyle)
            continue;
        QString name(QUrl::toPercentEncoding(QString(paragraphStyle->name()).replace(' ', '_')));
        name.replace('%', '_');
        if (name.isEmpty()) {
            name = 'P';
        }

        KOdfGenericStyle style(KOdfGenericStyle::ParagraphStyle, "paragraph");
        paragraphStyle->saveOdf(style, mainStyles);
        QString newName = mainStyles.insert(style, name, KOdfGenericStyles::DontAddNumberToName);
        savedNames.insert(paragraphStyle, newName);
        characterParagraphStyles.insert(paragraphStyle->characterStyle());
    }

    foreach(KParagraphStyle *p, d->paragStyles) {
        if (p->nextStyle() > 0 && savedNames.contains(p) && paragraphStyle(p->nextStyle())) {
            KParagraphStyle *next = paragraphStyle(p->nextStyle());
            if (next == p) // this is the default
                continue;
            mainStyles.insertStyleRelation(savedNames.value(p), savedNames.value(next), "style:next-style-name");
        }
    }

    foreach(KCharacterStyle *characterStyle, d->charStyles) {
        if (characterStyle == d->defaultParagraphStyle->characterStyle() || characterParagraphStyles.contains(characterStyle))
            continue;

        QString name(QUrl::toPercentEncoding(QString(characterStyle->name()).replace(' ', '_')));
        name.replace('%', '_');
        if (name.isEmpty()) {
            name = 'T';
        }

        KOdfGenericStyle style(KOdfGenericStyle::ParagraphStyle, "text");
        characterStyle->saveOdf(style);
        mainStyles.insert(style, name, KOdfGenericStyles::DontAddNumberToName);
    }

    foreach(KListStyle *listStyle, d->listStyles) {
        if (listStyle == d->defaultListStyle)
            continue;
        QString name(QUrl::toPercentEncoding(QString(listStyle->name()).replace(' ', '_')));
        name.replace('%', '_');
        if (name.isEmpty())
            name = 'L';

        KOdfGenericStyle style(KOdfGenericStyle::ListStyle);
        listStyle->saveOdf(style);
        mainStyles.insert(style, name, KOdfGenericStyles::DontAddNumberToName);
    }

    foreach(KTableStyle *tableStyle, d->tableStyles) {
        QString name(QUrl::toPercentEncoding(QString(tableStyle->name()).replace(' ', '_')));
        name.replace('%', '_');
        if (name.isEmpty())
            name = "table";

        KOdfGenericStyle style(KOdfGenericStyle::TableStyle);
        tableStyle->saveOdf(style);
        mainStyles.insert(style, name, KOdfGenericStyles::DontAddNumberToName);
    }

    foreach(KTableColumnStyle *tableColumnStyle, d->tableColumnStyles) {
        QString name(QUrl::toPercentEncoding(QString(tableColumnStyle->name()).replace(' ', '_')));
        name.replace('%', '_');
        if (name.isEmpty())
            name = "Tc";

        KOdfGenericStyle style(KOdfGenericStyle::TableColumnStyle);
        tableColumnStyle->saveOdf(style);
        mainStyles.insert(style, name, KOdfGenericStyles::DontAddNumberToName);
    }

    foreach(KTableRowStyle *tableRowStyle, d->tableRowStyles) {
        QString name(QUrl::toPercentEncoding(QString(tableRowStyle->name()).replace(' ', '_')));
        name.replace('%', '_');
        if (name.isEmpty())
            name = "Tr";

        KOdfGenericStyle style(KOdfGenericStyle::TableRowStyle);
        tableRowStyle->saveOdf(style);
        mainStyles.insert(style, name, KOdfGenericStyles::DontAddNumberToName);
    }

    foreach(KTableCellStyle *tableCellStyle, d->tableCellStyles) {
        QString name(QUrl::toPercentEncoding(QString(tableCellStyle->name()).replace(' ', '_')));
        name.replace('%', '_');
        if (name.isEmpty())
            name = "Tc";

        KOdfGenericStyle style(KOdfGenericStyle::TableCellStyle);
        tableCellStyle->saveOdf(style);
        mainStyles.insert(style, name, KOdfGenericStyles::DontAddNumberToName);
    }

    foreach(KSectionStyle *sectionStyle, d->sectionStyles) {
        QString name(QUrl::toPercentEncoding(QString(sectionStyle->name()).replace(' ', '_')));
        name.replace('%', '_');
        if (name.isEmpty())
            name = 'S'; 

        KOdfGenericStyle style(KOdfGenericStyle::SectionStyle);
        sectionStyle->saveOdf(style);
        mainStyles.insert(style, name, KOdfGenericStyles::DontAddNumberToName);
    }
}

void KStyleManager::add(KCharacterStyle *style)
{
    if (d->charStyles.key(style, -1) != -1)
        return;
    style->setParent(this);
    style->setStyleId(d->s_stylesNumber);
    d->charStyles.insert(d->s_stylesNumber, style);
    d->refreshUnsetStoreFor(d->s_stylesNumber++);

    emit styleAdded(style);
}

void KStyleManager::add(KParagraphStyle *style)
{
    if (d->paragStyles.key(style, -1) != -1)
        return;
    style->setParent(this);
    style->setStyleId(d->s_stylesNumber);
    d->paragStyles.insert(d->s_stylesNumber++, style);

    // Make sure the style inherits from the defaultParagraphStyle
    KParagraphStyle *root = style;
    while (root->parentStyle()) root = root->parentStyle();
    if (root != d->defaultParagraphStyle && root->parentStyle() == 0)
        root->setParentStyle(d->defaultParagraphStyle);

    // add all parent styles too if not already present.
    root = style;
    while (root->parentStyle()) {
        root = root->parentStyle();
        if (root->styleId() == 0)
            add(root);
    }

    d->refreshUnsetStoreFor(style->styleId());

    if (style->characterStyle()) {
        add(style->characterStyle());
        if (style->characterStyle()->name().isEmpty())
            style->characterStyle()->setName(style->name());
    }
    if (style->listStyle() && style->listStyle()->styleId() == 0)
        add(style->listStyle());

    emit styleAdded(style);
}

void KStyleManager::add(KListStyle *style)
{
    if (d->listStyles.key(style, -1) != -1)
        return;
    style->setParent(this);
    style->setStyleId(d->s_stylesNumber);
    d->listStyles.insert(d->s_stylesNumber++, style);
    emit styleAdded(style);
}

void KStyleManager::addAutomaticListStyle(KListStyle *style)
{
    if (d->automaticListStyles.key(style, -1) != -1)
        return;
    style->setStyleId(d->s_stylesNumber);
    d->automaticListStyles.insert(d->s_stylesNumber++, style);
}

void KStyleManager::add(KTableStyle *style)
{
    if (d->tableStyles.key(style, -1) != -1)
        return;
    style->setParent(this);
    style->setStyleId(d->s_stylesNumber);
    d->tableStyles.insert(d->s_stylesNumber++, style);
    emit styleAdded(style);
}

void KStyleManager::add(KTableColumnStyle *style)
{
    if (d->tableColumnStyles.key(style, -1) != -1)
        return;
    style->setStyleId(d->s_stylesNumber);
    d->tableColumnStyles.insert(d->s_stylesNumber++, style);
    emit styleAdded(style);
}

void KStyleManager::add(KTableRowStyle *style)
{
    if (d->tableRowStyles.key(style, -1) != -1)
        return;
    style->setStyleId(d->s_stylesNumber);
    d->tableRowStyles.insert(d->s_stylesNumber++, style);
    emit styleAdded(style);
}

void KStyleManager::add(KTableCellStyle *style)
{
    if (d->tableCellStyles.key(style, -1) != -1)
        return;
    style->setParent(this);
    style->setStyleId(d->s_stylesNumber);
    d->tableCellStyles.insert(d->s_stylesNumber++, style);
    emit styleAdded(style);
}

void KStyleManager::add(KSectionStyle *style)
{
    if (d->sectionStyles.key(style, -1) != -1)
        return;
    style->setParent(this);
    style->setStyleId(d->s_stylesNumber);
    d->sectionStyles.insert(d->s_stylesNumber++, style);
    emit styleAdded(style);
}

void KStyleManager::remove(KCharacterStyle *style)
{
    if (d->charStyles.remove(style->styleId()))
        emit styleRemoved(style);
}

void KStyleManager::remove(KParagraphStyle *style)
{
    if (d->paragStyles.remove(style->styleId()))
        emit styleRemoved(style);
}

void KStyleManager::remove(KListStyle *style)
{
    if (d->listStyles.remove(style->styleId()))
        emit styleRemoved(style);
}

void KStyleManager::remove(KTableStyle *style)
{
    if (d->tableStyles.remove(style->styleId()))
        emit styleRemoved(style);
}

void KStyleManager::remove(KTableColumnStyle *style)
{
    if (d->tableColumnStyles.remove(style->styleId()))
        emit styleRemoved(style);
}

void KStyleManager::remove(KTableRowStyle *style)
{
    if (d->tableRowStyles.remove(style->styleId()))
        emit styleRemoved(style);
}

void KStyleManager::remove(KTableCellStyle *style)
{
    if (d->tableCellStyles.remove(style->styleId()))
        emit styleRemoved(style);
}

void KStyleManager::remove(KSectionStyle *style)
{
    if (d->sectionStyles.remove(style->styleId()))
        emit styleRemoved(style);
}

void KStyleManager::remove(ChangeFollower *cf)
{
    d->documentUpdaterProxies.removeAll(cf);
}

void KStyleManager::alteredStyle(const KParagraphStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (!d->updateQueue.contains(id)) {
        d->updateQueue.insert(id, d->unsetStore.value(id));
    }
    d->requestFireUpdate(this);

    // check if anyone that uses 'style' as a parent needs to be flagged as changed as well.
    foreach (KParagraphStyle *ps, d->paragStyles) {
        if (ps->parentStyle() == style)
            alteredStyle(ps);
    }
}

void KStyleManager::alteredStyle(const KCharacterStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }

    // we also implicitly have inheritence in char-styles.
    for (QHash<int, KParagraphStyle*>::const_iterator it = d->paragStyles.constBegin(); it != d->paragStyles.constEnd(); ++it) {
        KParagraphStyle *ps = it.value();
        if (ps->characterStyle() == style)
            d->requestUpdateForChildren(ps);
    }
    d->requestFireUpdate(this);
}

void KStyleManager::alteredStyle(const KListStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (!d->updateQueue.contains(id))
        d->updateQueue.insert(id, d->unsetStore.value(id));
    d->requestFireUpdate(this);
}

void KStyleManager::alteredStyle(const KTableStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (!d->updateQueue.contains(id))
        d->updateQueue.insert(id, d->unsetStore.value(id));
    d->requestFireUpdate(this);
}

void KStyleManager::alteredStyle(const KTableColumnStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (!d->updateQueue.contains(id))
        d->updateQueue.insert(id, d->unsetStore.value(id));
    d->requestFireUpdate(this);
}

void KStyleManager::alteredStyle(const KTableRowStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (!d->updateQueue.contains(id))
        d->updateQueue.insert(id, d->unsetStore.value(id));
    d->requestFireUpdate(this);
}

void KStyleManager::alteredStyle(const KTableCellStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (!d->updateQueue.contains(id))
        d->updateQueue.insert(id, d->unsetStore.value(id));
    d->requestFireUpdate(this);
}

void KStyleManager::alteredStyle(const KSectionStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (!d->updateQueue.contains(id))
        d->updateQueue.insert(id, d->unsetStore.value(id));
    d->requestFireUpdate(this);
}

void KStyleManager::add(QTextDocument *document)
{
    foreach(ChangeFollower *cf, d->documentUpdaterProxies) {
        if (cf->document() == document) {
            return; // already present.
        }
    }
    ChangeFollower *cf = new ChangeFollower(document, this);
    d->documentUpdaterProxies.append(cf);
}

void KStyleManager::remove(QTextDocument *document)
{
    foreach(ChangeFollower *cf, d->documentUpdaterProxies) {
        if (cf->document() == document) {
            d->documentUpdaterProxies.removeAll(cf);
            return;
        }
    }
}

KCharacterStyle *KStyleManager::characterStyle(int id) const
{
    return d->charStyles.value(id);
}

KParagraphStyle *KStyleManager::paragraphStyle(int id) const
{
    return d->paragStyles.value(id);
}

KListStyle *KStyleManager::listStyle(int id) const
{
    return d->listStyles.value(id);
}

KListStyle *KStyleManager::listStyle(int id, bool *automatic) const
{
    if (KListStyle *style = listStyle(id)) {
        *automatic = false;
        return style;
    }

    KListStyle *style = d->automaticListStyles.value(id);

    if (style) {
        *automatic = true;
    }
    else {
        // *automatic is unchanged
    }
    return style;
}

KTableStyle *KStyleManager::tableStyle(int id) const
{
    return d->tableStyles.value(id);
}

KTableColumnStyle *KStyleManager::tableColumnStyle(int id) const
{
    return d->tableColumnStyles.value(id);
}

KTableRowStyle *KStyleManager::tableRowStyle(int id) const
{
    return d->tableRowStyles.value(id);
}

KTableCellStyle *KStyleManager::tableCellStyle(int id) const
{
    return d->tableCellStyles.value(id);
}

KSectionStyle *KStyleManager::sectionStyle(int id) const
{
    return d->sectionStyles.value(id);
}

KCharacterStyle *KStyleManager::characterStyle(const QString &name) const
{
    foreach(KCharacterStyle *style, d->charStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KParagraphStyle *KStyleManager::paragraphStyle(const QString &name) const
{
    foreach(KParagraphStyle *style, d->paragStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KListStyle *KStyleManager::listStyle(const QString &name) const
{
    foreach(KListStyle *style, d->listStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KTableStyle *KStyleManager::tableStyle(const QString &name) const
{
    foreach(KTableStyle *style, d->tableStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KTableColumnStyle *KStyleManager::tableColumnStyle(const QString &name) const
{
    foreach(KTableColumnStyle *style, d->tableColumnStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KTableRowStyle *KStyleManager::tableRowStyle(const QString &name) const
{
    foreach(KTableRowStyle *style, d->tableRowStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KTableCellStyle *KStyleManager::tableCellStyle(const QString &name) const
{
    foreach(KTableCellStyle *style, d->tableCellStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KSectionStyle *KStyleManager::sectionStyle(const QString &name) const
{
    foreach(KSectionStyle *style, d->sectionStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KParagraphStyle *KStyleManager::defaultParagraphStyle() const
{
    return d->defaultParagraphStyle;
}

KListStyle *KStyleManager::defaultListStyle() const
{
    return d->defaultListStyle;
}

void KStyleManager::setOutlineStyle(KListStyle* listStyle)
{
    if (d->outlineStyle && d->outlineStyle->parent() == this)
        delete d->outlineStyle;
    listStyle->setParent(this);
    d->outlineStyle = listStyle;
}

KListStyle *KStyleManager::outlineStyle() const
{
    return d->outlineStyle;
}

QList<KCharacterStyle*> KStyleManager::characterStyles() const
{
    return d->charStyles.values();
}

QList<KParagraphStyle*> KStyleManager::paragraphStyles() const
{
    return d->paragStyles.values();
}

QList<KListStyle*> KStyleManager::listStyles() const
{
    return d->listStyles.values();
}

QList<KTableStyle*> KStyleManager::tableStyles() const
{
    return d->tableStyles.values();
}

QList<KTableColumnStyle*> KStyleManager::tableColumnStyles() const
{
    return d->tableColumnStyles.values();
}

QList<KTableRowStyle*> KStyleManager::tableRowStyles() const
{
    return d->tableRowStyles.values();
}

QList<KTableCellStyle*> KStyleManager::tableCellStyles() const
{
    return d->tableCellStyles.values();
}

QList<KSectionStyle*> KStyleManager::sectionStyles() const
{
    return d->sectionStyles.values();
}

KStyleManagerPrivate *KStyleManager::priv()
{
    return d;
}

#include <KStyleManager.moc>
