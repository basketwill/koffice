/* This file is part of the KDE project
   Copyright 2009 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#include "KCColumnStyleCommand.h"

#include "Damages.h"
#include "kcells_limits.h"
#include "KCMap.h"
#include "RowColumnFormat.h"
#include "KCSheet.h"
#include "KCSheetPrint.h"

KCColumnStyleCommand::KCColumnStyleCommand(QUndoCommand *parent)
        : KCAbstractRegionCommand(parent)
        , m_width(0.0)
        , m_hidden(false)
        , m_pageBreak(false)
{
}

KCColumnStyleCommand::~KCColumnStyleCommand()
{
    qDeleteAll(m_columnFormats);
}

void KCColumnStyleCommand::setWidth(double width)
{
    m_width = width;
}

void KCColumnStyleCommand::setHidden(bool hidden)
{
    m_hidden = hidden;
}

void KCColumnStyleCommand::setPageBreak(bool pageBreak)
{
    m_pageBreak = pageBreak;
}

void KCColumnStyleCommand::setTemplate(const KCColumnFormat &columnFormat)
{
    m_width = columnFormat.width();
    m_hidden = columnFormat.isHidden();
    m_pageBreak = columnFormat.hasPageBreak();
}

bool KCColumnStyleCommand::mainProcessing()
{
    double deltaWidth = 0.0;
    const KCRegion::ConstIterator end(constEnd());
    for (KCRegion::ConstIterator it(constBegin()); it != end; ++it) {
        const QRect range = (*it)->rect();
        for (int column = range.left(); column <= range.right(); ++column) {
            // Save the old style.
            if (m_firstrun) {
                const KCColumnFormat *columnFormat = m_sheet->columnFormat(column);
                if (!columnFormat->isDefault() && !m_columnFormats.contains(column)) {
                    m_columnFormats.insert(column, new KCColumnFormat(*columnFormat));
                }
            }

            // Set the new style.
            deltaWidth -= m_sheet->columnFormat(column)->width();
            if (m_reverse) {
                if (m_columnFormats.contains(column)) {
                    m_sheet->insertColumnFormat(m_columnFormats.value(column));
                } else {
                    m_sheet->deleteColumnFormat(column);
                }
            } else {
                KCColumnFormat *columnFormat = m_sheet->nonDefaultColumnFormat(column);
                columnFormat->setWidth(m_width);
                columnFormat->setHidden(m_hidden);
                columnFormat->setPageBreak(m_pageBreak);
            }
            deltaWidth += m_sheet->columnFormat(column)->width();
        }
        // Possible visual cache invalidation due to dimension change; rebuild it.
        const KCRegion region(range.left(), 1, KS_colMax - range.right() + 1, KS_rowMax, m_sheet);
        m_sheet->map()->addDamage(new KCCellDamage(m_sheet, region, KCCellDamage::Appearance));
    }
    m_sheet->adjustDocumentWidth(deltaWidth);
    m_sheet->print()->updateHorizontalPageParameters(0);
    return true;
}
