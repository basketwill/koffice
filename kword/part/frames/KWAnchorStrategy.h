/* This file is part of the KDE project
 * Copyright (C) 2007, 2009, 2010 Thomas Zander <zander@kde.org>
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

#ifndef KWANCHORSTRATEGY_H
#define KWANCHORSTRATEGY_H

#include <KoTextDocumentLayout.h>
#include <KoTextAnchor.h>

class KWAnchorStrategy;
class KWFrame;

/**
 * Class for text layout of anchored frames.
 * In KWTextDocumentLayout whenever we find an inlineObject of type KoTextAnchor an instance
 * of this class is created.
 * The layout process will query the state of the anchored data after each line is layouted by
 * calling checkState() which will return false as long as there is not enough layout information
 * to properly position the anchored frame.
 */
class KWAnchorStrategy
{
public:
    /**
     * Constructor.
     * @param anchor the anchor and affiliated shape this strategy will be operating on.
     */
    KWAnchorStrategy(KoTextAnchor *anchor);
    ~KWAnchorStrategy();

    /**
     * This method is the main work code.
     * It will check if enough information on layout info is generated by the layout state
     * and if there is it will move the anchor according to the anchor()s properties.
     * The layout state is reverted to an earlier paragraph if needed to account for the newly
     * placed shape.
     * @param state the state of the layout.
     * @param startOfBlock the position in the document of the first character in a block.
     * @part startOfBlockText the position in the document of the first non-anchor character in a block
     * @return will return true if the layout state has been changed.
     * @see isFinished()
     */
    bool checkState(KoTextDocumentLayout::LayoutState *state, int startOfBlock, int startOfBlockText);

    /**
     * @return if the anchor is placed properly and no more changes are required.
     * In other words; return true if the usage of this anchor is expired (in the current run)
     */
    bool isFinished();

    /**
     * @return the anchored shape that we will reposition.
     */
    KoShape *anchoredShape() const;

    /**
     * Return the text anchor this strategy works for.
     */
    KoTextAnchor *anchor() {
        return m_anchor;
    }

private:
    void calculateKnowledgePoint();

    KoTextAnchor *const m_anchor;
    int m_knowledgePoint; // the cursor position at which the layout process has gathered enough info to do our work
    bool m_finished;
    qreal m_currentLineY;
    int m_pass;
    int m_lastknownPosInDoc;

    QPointF m_lastOffset;
    KoTextAnchor::AnchorVertical m_lastVerticalAnchorAlignment;
    KoTextAnchor::AnchorHorizontal m_lastHorizontalAnchorAlignment;
};

#endif
