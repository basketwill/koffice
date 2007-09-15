/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KWPositionSelector.h"

#include <QRadioButton>
#include <QGridLayout>
#include <QButtonGroup>
#include <QButtonGroup>
#include <QPainter>

#include <KDebug>

class KWPositionSelector::Private {
public:
    Private()
        : position(KoFlake::TopLeftCorner)
    {
        topLeft = createButton(KoFlake::TopLeftCorner);
        topLeft->setChecked(true);
        topRight = createButton(KoFlake::TopRightCorner);
        center = createButton(KoFlake::CenteredPositon);
        bottomRight = createButton(KoFlake::BottomRightCorner);
        bottomLeft = createButton(KoFlake::BottomLeftCorner);
    }

    QRadioButton *createButton(int id) {
        QRadioButton *b = new QRadioButton();
        buttonGroup.addButton(b, id);
        return b;
    }

    QRadioButton *topLeft, *topRight, *center, *bottomRight, *bottomLeft;
    QButtonGroup buttonGroup;
    KoFlake::Position position;
};

class RadioLayout : public QLayout {
public:
    RadioLayout(QWidget *parent)
        : QLayout(parent)
    {
    }

    void setGeometry (const QRect &geom) {
        QSize prefSize = calcSizes();

        const int columnWidth = qRound(geom.width() / ((double) maxCol + 1));
        const int rowHeight = qRound(geom.height() / ((double) maxRow + 1));
        foreach(Item item, items) {
            QPoint point( item.column * columnWidth, item.row * rowHeight );
            QRect rect(point + geom.topLeft(), prefSize);
            item.child->setGeometry(rect);
        }
    }

    QSize calcSizes() {
        QSize prefSize;
        maxRow = 0;
        maxCol = 0;
        foreach(Item item, items) {
            if(prefSize.isEmpty()) {
                QAbstractButton *but = dynamic_cast<QAbstractButton*> (item.child->widget());
                Q_ASSERT(but);
                prefSize = but->iconSize();
            }
            maxRow = qMax(maxRow, item.row);
            maxCol = qMax(maxRow, item.column);
        }
        maxCol++; maxRow++; // due to being zero-based.
        preferred = QSize(maxCol * prefSize.width() + (maxCol-1) * 5, maxRow * prefSize.height() + (maxRow-1) * 5);
        minimum = QSize(maxCol * prefSize.width(), maxRow * prefSize.height());
        return prefSize;
    }

    QLayoutItem *itemAt (int index) const {
        return items.at(index).child;
    }

    QLayoutItem *takeAt (int index) {
        Q_ASSERT(index < count());
        Item item = items.takeAt(index);
        return item.child;
    }

    int count () const {
        return items.count();
    }

    void addItem(QLayoutItem *) {
        Q_ASSERT(0);
    }

    QSize sizeHint() const {
        if(preferred.isEmpty())
            const_cast<RadioLayout*> (this)->calcSizes();
        return preferred;
    }

    QSize minimumSize() const {
        if(minimum.isEmpty())
            const_cast<RadioLayout*> (this)->calcSizes();
        return minimum;
    }

    void addWidget(QRadioButton *widget, int row, int column) {
        Item newItem;
        newItem.child = new QWidgetItem(widget);
        newItem.row = row;
        newItem.column = column;
        items.append(newItem);
        widget->setParent(parentWidget());
    }

private:
    struct Item {
        QLayoutItem *child;
        int column;
        int row;
    };
    QList<Item> items;
    QSize preferred, minimum;
    int maxCol, maxRow;
};

KWPositionSelector::KWPositionSelector(QWidget *parent)
    : QWidget(parent),
    d(new Private())
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    RadioLayout *lay = new RadioLayout(this);
    lay->addWidget(d->topLeft, 0, 0);
    lay->addWidget(d->topRight, 0, 2);
    lay->addWidget(d->center, 1, 1);
    lay->addWidget(d->bottomRight, 2, 2);
    lay->addWidget(d->bottomLeft, 2, 0);
    setLayout(lay);

    connect(&d->buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(positionChanged(int)));
}

KWPositionSelector::~KWPositionSelector() {
    delete d;
}

KoFlake::Position KWPositionSelector::position() const {
    return d->position;
}

void KWPositionSelector::setPosition(KoFlake::Position position) {
    d->position = position;
    switch(d->position) {
        case KoFlake::TopLeftCorner:
            d->topLeft->setChecked(true);
            break;
        case KoFlake::TopRightCorner:
            d->topRight->setChecked(true);
            break;
        case KoFlake::CenteredPositon:
            d->center->setChecked(true);
            break;
        case KoFlake::BottomLeftCorner:
            d->bottomLeft->setChecked(true);
            break;
        case KoFlake::BottomRightCorner:
            d->bottomRight->setChecked(true);
            break;
    }
}

void KWPositionSelector::positionChanged(int position) {
    d->position = static_cast<KoFlake::Position> (position);
    emit positionSelected(d->position);
}

void KWPositionSelector::paintEvent (QPaintEvent *) {
    QPainter painter( this );
    QPen pen(Qt::black);
    pen.setWidth(3);
    painter.setPen(pen);
    painter.drawRect(d->topLeft->width() / 2 - 2, d->topLeft->height() / 2, d->bottomRight->x(), d->bottomRight->y());
    painter.end();
}

