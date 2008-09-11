/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "KoListStyle.h"
#include "KoListLevelProperties.h"
#include "KoTextBlockData.h"
#include "KoParagraphStyle.h"

#include <KoStyleStack.h>
#include <KoOdfStylesReader.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoGenStyle.h>
#include <kdebug.h>
#include <QTextCursor>
#include <QBuffer>

class KoListStyle::Private
{
public:
    Private() : styleId(0) { }

    QTextList *textList(int level, const QTextDocument *doc) {
        if (! textLists.contains(level))
            return 0;
        QMap<const QTextDocument*, QPointer<QTextList> > map = textLists[level];
        if (! map.contains(doc))
            return 0;
        QPointer<QTextList> pointer = map[doc];
        if (pointer.isNull())
            return 0;
        return pointer;
    }

    void setTextList(int level, const QTextDocument *doc, QTextList *list) {
        QMap<const QTextDocument*, QPointer<QTextList> > map = textLists[level];
        map.insert(doc, QPointer<QTextList>(list));
        textLists.insert(level, map);
    }

    QString name;
    int styleId;
    QMap<int, KoListLevelProperties> levels;
    QMap<int, QMap<const QTextDocument*, QPointer<QTextList> > > textLists;
};

KoListStyle::KoListStyle(QObject *parent)
        : QObject(parent), d(new Private())
{
}

KoListStyle::~KoListStyle()
{
    delete d;
}

bool KoListStyle::operator==(const KoListStyle &other) const
{
    foreach(int level, d->levels.keys()) {
        if (! other.hasLevelProperties(level))
            return false;
        if (!(other.levelProperties(level) == d->levels[level]))
            return false;
    }
    foreach(int level, other.d->levels.keys()) {
        if (! hasLevelProperties(level))
            return false;
    }
    return true;
}

bool KoListStyle::operator!=(const KoListStyle &other) const
{
    return !KoListStyle::operator==(other);
}

void KoListStyle::copyProperties(KoListStyle *other)
{
    d->styleId = other->d->styleId;
    d->levels = other->d->levels;
    d->textLists = other->d->textLists;
    setName(other->name());
}

KoListStyle *KoListStyle::clone(QObject *parent)
{
    KoListStyle *newStyle = new KoListStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}

QString KoListStyle::name() const
{
    return d->name;
}

void KoListStyle::setName(const QString &name)
{
    if (d->name == name)
        return;
    d->name = name;
    emit nameChanged(d->name);
}

int KoListStyle::styleId() const
{
    return d->styleId;
}

void KoListStyle::setStyleId(int id)
{
    d->styleId = id;
    foreach(int level, d->levels.keys()) {
        d->levels[level].setStyleId(id);
    }
}

KoListLevelProperties KoListStyle::levelProperties(int level) const
{
    if (d->levels.contains(level))
        return d->levels.value(level);
    if (d->levels.count()) {
        KoListLevelProperties llp = d->levels.begin().value();
        llp.setLevel(level);
        return llp;
    }
    KoListLevelProperties llp;
    if (d->styleId)
        llp.setStyleId(d->styleId);
    llp.setLevel(level);
    llp.setStyle(KoListStyle::DecimalItem);
    llp.setListItemSuffix(".");
    return llp;
}

QTextListFormat KoListStyle::listFormat(int level)
{
    KoListLevelProperties llp = levelProperties(level);
    QTextListFormat format;
    llp.applyStyle(format);
    return format;
}

void KoListStyle::setLevelProperties(const KoListLevelProperties &properties)
{
    d->levels.insert(properties.level(), properties);

    // find all QTextList objects and apply the changed style on them.
    if (! d->textLists.contains(properties.level()))
        return;
    QMap<const QTextDocument*, QPointer<QTextList> > map = d->textLists.value(properties.level());
    foreach(QPointer<QTextList> list, map) {
        if (list.isNull()) continue;
        QTextListFormat format = list->format();
        properties.applyStyle(format);
        list->setFormat(format);

        QTextBlock tb = list->item(0);
        if (tb.isValid()) { // invalidate the counter part
            KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(tb.userData());
            if (userData)
                userData->setCounterWidth(-1.0);
        }
    }
}

bool KoListStyle::hasLevelProperties(int level) const
{
    return d->levels.contains(level);
}

void KoListStyle::removeLevelProperties(int level)
{
    d->levels.remove(level);
}

void KoListStyle::applyStyle(const QTextBlock &block, int level)
{
    if (level == 0) { // illegal level; fetch the first proper level we have
        if (d->levels.count())
            level = d->levels.keys().first();
        else // just go for default, then
            level = 1;
    }

    const bool contains = hasLevelProperties(level);

    QTextList *textList = d->textList(level, block.document());
    if (textList && block.textList() && block.textList() != textList) // remove old one
        block.textList()->remove(block);
    if (block.textList() == 0 && textList) { // add if new
        textList->add(block);
    }
    if (block.textList() && textList == 0) {
        textList = block.textList(); // missed it ?
        d->setTextList(level, block.document(), textList);
    }

    QTextListFormat format;
    if (block.textList())
        format = block.textList()->format();

    KoListLevelProperties llp = this->levelProperties(level);
    if (d->styleId)
        llp.setStyleId(d->styleId);
    llp.applyStyle(format);

    if (textList) {
        textList->setFormat(format);
        QTextBlock tb = textList->item(0);
        if (tb.isValid()) { // invalidate the counter part
            KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(tb.userData());
            if (userData)
                userData->setCounterWidth(-1.0);
        }
    } else { // does not exist yet, this is the first parag that uses it :)
        QTextCursor cursor(block);
        textList = cursor.createList(format);
        d->setTextList(level, block.document(), textList);
    }

    if (contains)
        d->levels.insert(level, llp);

    QTextCursor cursor(block);
    QTextBlockFormat blockFormat = cursor.blockFormat();
    if (d->styleId) {
        blockFormat.setProperty(KoParagraphStyle::ListStyleId, d->styleId);
    } else {
        blockFormat.clearProperty(KoParagraphStyle::ListStyleId);
    }
    cursor.setBlockFormat(blockFormat);
}

// static
KoListStyle* KoListStyle::fromTextList(QTextList *list)
{
    KoListStyle *answer = new KoListStyle();
    KoListLevelProperties llp = KoListLevelProperties::fromTextList(list);
    answer->setLevelProperties(llp);
    answer->d->setTextList(llp.level(), list->document(), list);
    return answer;
}

void KoListStyle::loadOdf(KoOdfLoadingContext& context, const KoXmlElement& style)
{
    d->name = style.attributeNS(KoXmlNS::style, "display-name", QString());
    // if no style:display-name is given us the style:name
    if (d->name.isEmpty()) {
        d->name = style.attributeNS(KoXmlNS::style, "name", QString());
    }

    KoXmlElement styleElem;
    forEachElement(styleElem, style) {
        KoListLevelProperties properties;
        properties.loadOdf(context, styleElem);
        if (d->styleId)
            properties.setStyleId(d->styleId);
        setLevelProperties(properties);
    }
}

void KoListStyle::saveOdf(KoGenStyle &style)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    QMapIterator<int, KoListLevelProperties> it(d->levels);
    while (it.hasNext()) {
        it.next();
        it.value().saveOdf(&elementWriter);
    }
    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    style.addChildElement("text-list-level-style-content", elementContents);
}
