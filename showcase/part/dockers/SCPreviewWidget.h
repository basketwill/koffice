/* This file is part of the KDE project
   Copyright (C) 2008 Sven Langkamp <sven.langkamp@gmail.com>

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

#ifndef SCPREVIEWWIDGET_H
#define SCPREVIEWWIDGET_H

#include "showcase_export.h"

#include <QWidget>
#include <QTimeLine>

class SCPage;
class SCPageEffect;
class SCPageEffectRunner;

/**
 * A widget for page effect preview. It shows a transition from a black page to the current page
 */
class SHOWCASE_EXPORT SCPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    SCPreviewWidget(QWidget* parent = 0);
    ~SCPreviewWidget();

    /**
     * Set a page effect
     *
     * @param pageEffect The effect which will be previewed.
     * @param page The current page used in the preview. If 0 the preview will be x
     * @param prevpage The page coming before @p page
     */
    void setPageEffect(SCPageEffect* pageEffect, SCPage* page, SCPage* prevpage);

    /**
     * Run the current page effect. Does nothing if no page effect was set.
     */
    void runPreview();

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);
    void mousePressEvent(QMouseEvent* event);

protected slots:
    void animate();

private:
    void updatePixmaps();

    QTimeLine m_timeLine;

    SCPageEffect* m_pageEffect;
    SCPageEffectRunner* m_pageEffectRunner;
    SCPage* m_page;
    SCPage* m_prevpage;

    QPixmap m_oldPage;
    QPixmap m_newPage;
};

#endif /* SCPREVIEWWIDGET_H */
