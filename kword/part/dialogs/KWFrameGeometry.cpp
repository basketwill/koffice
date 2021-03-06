/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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
#include "KWFrameGeometry.h"
#include "KWDocument.h"
#include "KWPage.h"
#include "frames/KWTextFrame.h"
#include "frames/KWTextFrameSet.h"

#include <KShape.h>
#include <KShapeKeepAspectRatioCommand.h>
#include <KShapeMoveCommand.h>
#include <KShapeSizeCommand.h>
#include <KShapeLockCommand.h>

#include <kdebug.h>

KWFrameGeometry::KWFrameGeometry(FrameConfigSharedState *state)
        : m_state(state),
        m_frame(0),
        m_topOfPage(0),
        m_blockSignals(false),
        m_originalGeometryLock(false)
{
    m_state->addUser();
    widget.setupUi(this);
    setUnit(m_state->document()->unit());
    widget.width->setMinimum(0.0);
    widget.height->setMinimum(0.0);
    widget.leftMargin->setMinimum(0.0);
    widget.rightMargin->setMinimum(0.0);
    widget.bottomMargin->setMinimum(0.0);
    widget.topMargin->setMinimum(0.0);

    widget.keepAspect->setKeepAspectRatio(m_state->keepAspectRatio());

    connect(widget.leftMargin, SIGNAL(valueChangedPt(qreal)), this, SLOT(syncMargins(qreal)));
    connect(widget.rightMargin, SIGNAL(valueChangedPt(qreal)), this, SLOT(syncMargins(qreal)));
    connect(widget.bottomMargin, SIGNAL(valueChangedPt(qreal)), this, SLOT(syncMargins(qreal)));
    connect(widget.topMargin, SIGNAL(valueChangedPt(qreal)), this, SLOT(syncMargins(qreal)));

    connect(widget.width, SIGNAL(valueChangedPt(qreal)), this, SLOT(widthChanged(qreal)));
    connect(widget.height, SIGNAL(valueChangedPt(qreal)), this, SLOT(heightChanged(qreal)));

    connect(m_state, SIGNAL(keepAspectRatioChanged(bool)), widget.keepAspect, SLOT(setKeepAspectRatio(bool)));
    connect(widget.keepAspect, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(updateAspectRatio(bool)));

    connect(widget.positionSelector, SIGNAL(positionSelected(KFlake::Position)),
            this, SLOT(setGeometryAlignment(KFlake::Position)));

}

KWFrameGeometry::~KWFrameGeometry()
{
    m_state->removeUser();
}

void KWFrameGeometry::open(KWFrame *frame)
{
    m_frame = frame;
    open(frame->shape());

    KWTextFrame *tfs = dynamic_cast<KWTextFrame*>(frame);
    if (tfs) {
        KInsets insets(tfs->insets());
        widget.marginsGB->setVisible(true);
        widget.leftMargin->changeValue(insets.left);
        widget.rightMargin->changeValue(insets.right);
        widget.topMargin->changeValue(insets.top);
        widget.bottomMargin->changeValue(insets.bottom);
    } else {
        widget.marginsGB->setVisible(false);
    }
}

void KWFrameGeometry::open(KShape *shape)
{
    KWPage page = m_state->document()->pageManager()->page(shape);
    m_topOfPage = page.offsetInDocument();
    m_originalPosition = shape->position();
    m_originalSize = shape->size();
    QPointF position = shape->absolutePosition(widget.positionSelector->position());

    widget.xPos->changeValue(position.x());
    widget.yPos->changeValue(position.y() - m_topOfPage);
    widget.width->changeValue(m_originalSize.width());
    widget.height->changeValue(m_originalSize.height());

    if (m_frame == 0) {
        // default values for new frames
        widget.leftMargin->changeValue(MM_TO_POINT(3));
        widget.rightMargin->changeValue(MM_TO_POINT(3));
        widget.topMargin->changeValue(MM_TO_POINT(3));
        widget.bottomMargin->changeValue(MM_TO_POINT(3));
    }

    connect(widget.protectSize, SIGNAL(stateChanged(int)),
            this, SLOT(protectSizeChanged(int)));

    m_originalGeometryLock = shape->isGeometryProtected();
    if (m_originalGeometryLock) {
        widget.protectSize->setCheckState(Qt::Checked);
        KWTextFrame *tf = dynamic_cast<KWTextFrame*>(shape->applicationData());
        if (tf && static_cast<KWTextFrameSet*>(tf->frameSet())->textFrameSetType() != KWord::OtherTextFrameSet)
            widget.protectSize->setEnabled(false); // auto-generated frame, can't edit
    }

    connect(widget.xPos, SIGNAL(valueChanged(double)), this, SLOT(updateShape()));
    connect(widget.yPos, SIGNAL(valueChanged(double)), this, SLOT(updateShape()));
    connect(widget.width, SIGNAL(valueChanged(double)), this, SLOT(updateShape()));
    connect(widget.height, SIGNAL(valueChanged(double)), this, SLOT(updateShape()));
}

void KWFrameGeometry::updateShape()
{
    if (m_blockSignals) return;
    KWFrame *frame = m_frame;
    if (frame == 0) {
        frame = m_state->frame();
        m_state->markFrameUsed();
    }
    Q_ASSERT(frame);
    frame->shape()->update();
    KShape *shape = frame->shape();
    QPointF currentPos(shape->absolutePosition(widget.positionSelector->position()));
    QPointF pos(widget.xPos->value(), widget.yPos->value() + m_topOfPage);
    QPointF moved = pos - currentPos;
    QPointF prev(moved);
    m_state->document()->clipToDocument(frame->shape(), moved);
    pos = currentPos + moved;

    frame->shape()->setAbsolutePosition(pos, widget.positionSelector->position());
    QSizeF size(widget.width->value(), widget.height->value());
    frame->shape()->setSize(size);

    KWTextFrame *tfs = dynamic_cast <KWTextFrame*>(frame);
    if (tfs) {
        KInsets insets(widget.topMargin->value(), widget.leftMargin->value(),
                widget.bottomMargin->value(), widget.rightMargin->value());
        tfs->setInsets(insets);
    }

    frame->shape()->update();
}

void KWFrameGeometry::protectSizeChanged(int protectSizeState)
{
    KWFrame *frame = m_frame;
    if (frame == 0) {
        frame = m_state->frame();
        m_state->markFrameUsed();
    }
    Q_ASSERT(frame);
    bool lock = (protectSizeState == Qt::Checked);
    frame->shape()->setGeometryProtected(lock);
    widget.xPos->setDisabled(lock);
    widget.yPos->setDisabled(lock);
    widget.width->setDisabled(lock);
    widget.height->setDisabled(lock);
    widget.keepAspect->setDisabled(lock);
}

void KWFrameGeometry::syncMargins(qreal value)
{
    if (widget.synchronize->isChecked()) {
        widget.leftMargin->changeValue(value);
        widget.topMargin->changeValue(value);
        widget.rightMargin->changeValue(value);
        widget.bottomMargin->changeValue(value);
    }
    updateShape();
}

void KWFrameGeometry::save()
{
    // no-op now
}

void KWFrameGeometry::cancel()
{
    KWFrame *frame = m_frame;
    if (frame == 0) {
        frame = m_state->frame();
        m_state->markFrameUsed();
    }
    Q_ASSERT(frame);
    frame->shape()->setPosition(m_originalPosition);
    frame->shape()->setSize(m_originalSize);
}

void KWFrameGeometry::widthChanged(qreal value)
{
    if (! m_state->keepAspectRatio())  return;
    if (m_blockSignals) return;
    m_blockSignals = true;
    widget.height->changeValue(m_originalSize.height() / m_originalSize.width() * value);
    m_blockSignals = false;
}

void KWFrameGeometry::heightChanged(qreal value)
{
    if (! m_state->keepAspectRatio())  return;
    if (m_blockSignals) return;
    m_blockSignals = true;
    widget.width->changeValue(m_originalSize.width() / m_originalSize.height() * value);
    m_blockSignals = false;
}

void KWFrameGeometry::setUnit(KUnit unit)
{
    widget.xPos->setUnit(unit);
    widget.yPos->setUnit(unit);
    widget.width->setUnit(unit);
    widget.height->setUnit(unit);
    widget.leftMargin->setUnit(unit);
    widget.topMargin->setUnit(unit);
    widget.rightMargin->setUnit(unit);
    widget.bottomMargin->setUnit(unit);
}

void KWFrameGeometry::setGeometryAlignment(KFlake::Position position)
{
    KWFrame *frame = m_frame;
    if (frame == 0) {
        frame = m_state->frame();
        m_state->markFrameUsed();
    }
    QPointF pos = frame->shape()->absolutePosition(position);
    m_blockSignals = true;
    widget.xPos->changeValue(pos.x());
    widget.yPos->changeValue(pos.y() - m_topOfPage);
    m_blockSignals = false;
}

void KWFrameGeometry::updateAspectRatio(bool keep)
{
    m_state->setKeepAspectRatio(keep);
    if (keep)
        widget.height->changeValue(m_originalSize.height() / m_originalSize.width() * widget.width->value());
}

QUndoCommand *KWFrameGeometry::createCommand(QUndoCommand *parent)
{
    if (m_frame == 0)
        return 0;
    KShape *shape = m_frame->shape();
    if (!qFuzzyCompare(m_originalSize.width(), shape->size().width())
            || !qFuzzyCompare(m_originalSize.height(), shape->size().height())) {
        new KShapeSizeCommand(QList<KShape*>() << shape, QList<QSizeF>() << m_originalSize,
            QList<QSizeF>() << shape->size(), parent);
    }
    if (!qFuzzyCompare(m_originalPosition.x(), shape->position().x())
            || !qFuzzyCompare(m_originalPosition.y(), shape->position().y())) {
        new KShapeMoveCommand(QList<KShape*>() << shape, QList<QPointF>() << m_originalPosition,
            QList<QPointF>() << shape->position(), parent);
    }
    if (m_originalGeometryLock != shape->isGeometryProtected()) {
        new KShapeLockCommand(QList<KShape*>() << shape, QList<bool>() << m_originalGeometryLock,
            QList<bool>() << !m_originalGeometryLock, parent);
    }
    return parent;
}
