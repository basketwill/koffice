/* This file is part of the KDE project
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

#ifndef SCPAGELAYOUTDOCKER_H
#define SCPAGELAYOUTDOCKER_H

#include <QDockWidget>
#include <QMap>

class QListWidget;
class QListWidgetItem;
class SCPageLayout;
class SCView;

class SCPageLayoutDocker : public QDockWidget
{
    Q_OBJECT
public:
    explicit SCPageLayoutDocker(QWidget* parent = 0, Qt::WindowFlags flags = 0);

    void setView(SCView* view);

public slots:
    void slotActivePageChanged();
    void slotItemPressed(QListWidgetItem * item);
    void slotCurrentItemChanged(QListWidgetItem * item, QListWidgetItem * previous);

private:
    QListWidgetItem * addLayout(SCPageLayout * layout);
    void applyLayout(QListWidgetItem * item);
    SCView* m_view;
    QListWidget * m_layoutsView;
    QMap<SCPageLayout *, QListWidgetItem *> m_layout2item;
    // store the last item which was active so we can detect that 
    // the already selected item was clicked again.
    QListWidgetItem * m_previousItem;
};

#endif /* SCPAGELAYOUTDOCKER_H */
