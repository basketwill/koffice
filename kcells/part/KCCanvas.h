/* This file is part of the KDE project
   Copyright 2006-2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 1999-2002,2004 Laurent Montel <montel@kde.org>
   Copyright 2002-2005 Ariya Hidayat <ariya@kde.org>
   Copyright 1999-2001,2003 David Faure <faure@kde.org>
   Copyright 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2000-2001 Werner Trobin <trobin@kde.org>
   Copyright 2002 Harri Porten <porten@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 1999-2000 Torben Weis <weis@kde.org>
   Copyright 2000 Wilco Greven <greven@kde.org>
   Copyright 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   Copyright 1999 Reginald Stadlbauer <reggie@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KC_CANVAS
#define KC_CANVAS

#include "kcells_export.h"

#include "Global.h"

#include <KCanvasBase.h>

#include <QList>
#include <QWidget>

// Width of row header and height of column headers.  These are not
// part of the styles.
// FIXME: Rename to ROWHEADER_WIDTH and COLHEADER_HEIGHT?
#define YBORDER_WIDTH  35
#define XBORDER_HEIGHT 20

class KPointerEvent;
class KoZoomHandler;

class KCColumnHeader;
class KCDoc;
class KCSheet;
class KCRowHeader;
class Selection;
class KCView;


/**
 * The scrollable area showing the cells.
 */
class KCELLS_EXPORT KCCanvas : public QWidget, public KCanvasBase
{
    friend class KCColumnHeader;
    friend class KCRowHeader;
    friend class KCView;
    friend class KCCellTool;

    Q_OBJECT

public:
    explicit KCCanvas(KCView* view);
    ~KCCanvas();

    KCView* view() const;

    /// reimplemented method from KCanvasBase
    virtual QWidget* canvasWidget() {
        return this;
    }
    /// reimplemented method from KCanvasBase
    virtual const QWidget* canvasWidget() const {
        return this;
    }
    /// reimplemented method from KCanvasBase
    virtual void gridSize(qreal* horizontal, qreal* vertical) const;
    /// reimplemented method from KCanvasBase
    virtual bool snapToGrid() const;
    /// reimplemented method from KCanvasBase
    virtual void addCommand(QUndoCommand* command);
    /// reimplemented method from KCanvasBase
    virtual KShapeManager* shapeManager() const;
    /// reimplemented method from KCanvasBase
    virtual void updateCanvas(const QRectF& rc);
    /// reimplemented method from KCanvasBase
    virtual KToolProxy* toolProxy() const;
    /// reimplemented method from KCanvasBase
    virtual KUnit unit() const;
    /// reimplemented method from KCanvasBase
    virtual void updateInputMethodInfo();
    /// reimplemented method from KCanvasBase
    virtual const KViewConverter* viewConverter() const;


    QPointF offset() const {
        return m_offset;
    }

    /**
     * @return a pointer to the active sheet
     */
    KCSheet* activeSheet() const;
    virtual Selection* selection() const;
    KCDoc *doc() const {
        return m_doc;
    }

    /**
     * @return the width of the columns before the current screen
     */
    qreal xOffset() const {
        return m_offset.x();
    }

    /**
     * @return the height of the rows before the current screen
     */
    qreal yOffset() const {
        return m_offset.y();
    }

    /**
     * Validates the selected cell.
     */
    void validateSelection();
    /**
     * Calculates the region in view coordinates occupied by a range of cells on
     * the currently active sheet. Respects the scrolling offset and the layout
     * direction
     *
     * \param cellRange The range of cells on the current sheet.
     */
    QRectF cellCoordinatesToView(const QRect &cellRange) const;

    KoZoomHandler* zoomHandler() const;
    bool isViewLoading() const;
    void enableAutoScroll();
    void disableAutoScroll();
    void showContextMenu(const QPoint& globalPos);
    KCColumnHeader* columnHeader() const;
    KCRowHeader* rowHeader() const;

public slots:
    void setDocumentOffset(const QPoint &offset);
    void setDocumentSize(const QSizeF &size);

signals:
    void documentSizeChanged(const QSize&);

protected:
    virtual bool event(QEvent *e);
    virtual void paintEvent(QPaintEvent *ev);
    virtual void mousePressEvent(QMouseEvent *ev);
    virtual void mouseReleaseEvent(QMouseEvent *ev);
    virtual void mouseMoveEvent(QMouseEvent *ev);
    virtual void mouseDoubleClickEvent(QMouseEvent*);
    virtual void focusInEvent(QFocusEvent *ev);
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);
    /// reimplemented method from superclass
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
    /// reimplemented method from superclass
    virtual void inputMethodEvent(QInputMethodEvent *event);
    /// reimplemented method from superclass
    virtual void tabletEvent(QTabletEvent *e);

private:
    void setVertScrollBarPos(qreal pos);
    void setHorizScrollBarPos(qreal pos);

    bool eventFilter(QObject *o, QEvent *e);
    void keyPressed(QKeyEvent *ev);
    /**
     * Determines the cell at @p point and shows its tooltip.
     * @param point the position for which a tooltip is requested
     */
    void showToolTip(const QPoint& point);

    /**
     * Returns the range of cells which appear in the specified area of the KCCanvas widget
     * For example, viewToCellCoordinates( QRect(0,0,width(),height()) ) returns a range containing all visible cells
     *
     * @param area The area (in pixels) on the KCCanvas widget
     */
    QRect viewToCellCoordinates(const QRectF& area) const;

private:
    Q_DISABLE_COPY(KCCanvas)

    KCView *m_view;
    KCDoc *m_doc;
    // Non-visible range top-left from current screen
    // Example: If the first visible column is 'E', then offset stores
    // the width of the invisible columns 'A' to 'D'.
    // Example: If the first visible row is '5', then offset stores
    // the height of the invisible rows '1' to '4'.
    QPointF m_offset;

    // flake
    KShapeManager *m_shapeManager;
    KToolProxy *m_toolProxy;
};

#endif // KC_CANVAS
