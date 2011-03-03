/* This file is part of the KDE project
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
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

#ifndef KPRPRESENTERVIEWSLIDESINTERFACE
#define KPRPRESENTERVIEWSLIDESINTERFACE

#include "SCPresenterViewBaseInterface.h"

class KoPAPageBase;
class KoPAPageThumbnailModel;

class QListView;
class QModelIndex;

/**
 * SCPresenterViewSlidesInterface
 * This widget shows all slides in the presentation
 */
class SCPresenterViewSlidesInterface : public SCPresenterViewBaseInterface
{
    Q_OBJECT
public:
    explicit SCPresenterViewSlidesInterface(const QList<KoPAPageBase *> &pages, QWidget *parent = 0);

signals:
    void selectedPageChanged(int index, bool doubleClicked);

private slots:
    void itemClicked(const QModelIndex &index);
    void itemDoubleClicked(const QModelIndex &index);

private:
    KoPAPageThumbnailModel *m_thumbnailModel;
    QListView *m_listView;
};

#endif
