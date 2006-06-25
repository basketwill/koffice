/* This file is part of the KDE project
   Copyright (C) 2006 Tomas Mecir <mecirt@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "manipulator_data.h"

#include <klocale.h>

#include "kspread_cell.h"
#include "kspread_sheet.h"

using namespace KSpread;

AbstractDataManipulator::AbstractDataManipulator ()
{
  m_format = false;
}

AbstractDataManipulator::~AbstractDataManipulator ()
{
  oldData.clear ();
}

bool AbstractDataManipulator::process (Element* element)
{
  QRect range = element->rect();
  for (int col = range.left(); col <= range.right(); ++col)
    for (int row = range.top(); row <= range.bottom(); ++row) {
      Value val;
      QString text;
      int colidx = col - range.left();
      int rowidx = row - range.top();
      bool parse = false;
      FormatType fmtType = No_format;
      if (m_reverse) {
        // reverse - use the stored value
        if (oldData.contains (colidx) && oldData[colidx].contains (rowidx)) {
          val = oldData[colidx][rowidx].val;
          text = oldData[colidx][rowidx].text;
          fmtType = oldData[colidx][rowidx].format;
          parse = (!text.isEmpty());   // parse if text not empty
        }
      } else {
        val = newValue (element, col, row, &parse, &fmtType);
        if (parse)
          text = val.asString();
      }

      // we have the data - set it !
      if (parse) {
        Cell *cell = m_sheet->nonDefaultCell (col, row);
        cell->setCellText (text);
      } else {
        Cell *cell = m_sheet->cellAt (col, row);
        if (!(val.isEmpty() && cell->isDefault()))
        // nothing if value and cell both empty
        {
          Cell *cell = m_sheet->nonDefaultCell (col, row);
          cell->setCellValue (val, fmtType, text);
        }
      }
    }
  return true;
}

Value AbstractDataManipulator::stored (int col, int row, bool *parse)
{
  Value val = oldData[col][row].val;
  QString text = oldData[col][row].text;
  *parse = false;
  if (!text.isEmpty()) {
    val = Value (text);
    *parse = true;
  }
  return val;
}

bool AbstractDataManipulator::preProcessing ()
{
  // not the first run - data already stored ...
  if (!m_firstrun) return true;

  Region::Iterator endOfList(cells().end());
  for (Region::Iterator it = cells().begin(); it != endOfList; ++it)
  {
    QRect range = (*it)->rect();
    for (int col = range.left(); col <= range.right(); ++col)
      for (int row = range.top(); row <= range.bottom(); ++row)
      {
        Cell* cell = m_sheet->cellAt(col, row);
        if (cell != m_sheet->defaultCell())  // non-default cell - remember it
        {
          ADMStorage st;

          int colidx = col - range.left();
          int rowidx = row - range.top();

          if (cell->isFormula())
            st.text = cell->text();
          st.val = m_sheet->value (col, row);
          st.format = cell->formatType();
          oldData[colidx][rowidx] = st;
        }
      }
  }
  return true;
}

AbstractDFManipulator::AbstractDFManipulator ()
{
  m_changeformat = true;
}

AbstractDFManipulator::~AbstractDFManipulator ()
{
  // delete all stored formats ...
  QMap<int, QMap<int, Format *> >::iterator it1;
  QMap<int, Format *>::iterator it2;
  for (it1 = formats.begin(); it1 != formats.end(); ++it1)
    for (it2 = (*it1).begin(); it2 != (*it1).end(); ++it2)
      delete (*it2);
  formats.clear ();
}

bool AbstractDFManipulator::process (Element* element)
{
  // let parent class process it first
  AbstractDataManipulator::process (element);

  // don't continue if we have to change formatting
  if (!m_changeformat) return true;

  QRect range = element->rect();
  for (int col = range.left(); col <= range.right(); ++col)
  {
    for (int row = range.top(); row <= range.bottom(); ++row) {
      Cell *cell = m_sheet->cellAt (col, row);
      int colidx = col - range.left();
      int rowidx = row - range.top();
      // only non-default cells will be formatted
      // TODO: is this really correct ?
      if (!cell->isDefault())
      {
        Format *f = 0;
        if (m_reverse)
          f = formats[colidx][rowidx];
        else
          f = newFormat (element, col, row);
        if (f) cell->format()->copy (*f);
        cell->setLayoutDirtyFlag();
        cell->setDisplayDirtyFlag();
      }
    }
  }
  return true;
}

bool AbstractDFManipulator::preProcessing ()
{
  // not the first run - data already stored ...
  if (!m_firstrun) return true;

  AbstractDataManipulator::preProcessing ();

  Region::Iterator endOfList(cells().end());
  for (Region::Iterator it = cells().begin(); it != endOfList; ++it)
  {
    QRect range = (*it)->rect();
    for (int col = range.left(); col <= range.right(); ++col)
      for (int row = range.top(); row <= range.bottom(); ++row)
      {
        Cell* cell = m_sheet->cellAt(col, row);
        if (cell != m_sheet->defaultCell())  // non-default cell - remember it
        {
          int colidx = col - range.left();
          int rowidx = row - range.top();
          Format *f = new Format (m_sheet, 0);
          f->copy (*cell->format());
          formats[colidx][rowidx] = f;
        }
      }
  }
  return true;
}

DataManipulator::DataManipulator ()
  : m_format (No_format),
  m_parsing (false)
{
  // default name for DataManipulator, can be changed using setName
  m_name = i18n ("Change Value");
}

DataManipulator::~DataManipulator ()
{
}

Value DataManipulator::newValue (Element *element, int col, int row,
    bool *parsing, FormatType *formatType)
{
  *parsing = m_parsing;
  if (m_format != No_format)
    *formatType = m_format;
  QRect range = element->rect();
  int colidx = col - range.left();
  int rowidx = row - range.top();
  return data.element (colidx, rowidx);
}

ArrayFormulaManipulator::ArrayFormulaManipulator ()
{
  m_name = i18n ("Set Array Formula");
}

ArrayFormulaManipulator::~ArrayFormulaManipulator ()
{
}

Value ArrayFormulaManipulator::newValue (Element *element, int col, int row,
                                 bool *parsing, FormatType *)
{
  *parsing = true;
  QRect range = element->rect();
  int colidx = col - range.left();
  int rowidx = row - range.top();

  // fill in the cells ... top-left one gets the formula, the rest gets =INDEX
  // TODO: also fill in information about cells being a part of a range for GUI
  if (colidx || rowidx) {
    return (cellRef + QString::number (rowidx+1) + ';' +
        QString::number (colidx+1) + ')');
  } else {
    Cell *cell = m_sheet->nonDefaultCell (col, row);
    cellRef = "=INDEX(" + cell->name() + ';';
    return m_text;
  }
}

FillManipulator::FillManipulator ()
{
  m_dir = Down;
  m_changeformat = true;
  m_name = i18n ("Fill Selection");
}

FillManipulator::~FillManipulator ()
{
}

Value FillManipulator::newValue (Element *element, int col, int row,
    bool *parse, FormatType *fmtType)
{
  QRect range = element->rect();
  int colidx = col - range.left();
  int rowidx = row - range.top();
  int rows = element->rect().bottom() - element->rect().top() + 1;
  int cols = element->rect().right() - element->rect().left() + 1;
  switch (m_dir) {
    case Up: rowidx = rows - 1; break;
    case Down: rowidx = 0; break;
    case Left: colidx = cols - 1; break;
    case Right: colidx = 0; break;
  };
  return stored (colidx, rowidx, parse);
}

Format *FillManipulator::newFormat (Element *element, int col, int row)
{
  QRect range = element->rect();
  int colidx = col - range.left();
  int rowidx = row - range.top();
  int rows = element->rect().bottom() - element->rect().top() + 1;
  int cols = element->rect().right() - element->rect().left() + 1;
  switch (m_dir) {
    case Up: rowidx = rows - 1; break;
    case Down: rowidx = 0; break;
    case Left: colidx = cols - 1; break;
    case Right: colidx = 0; break;
  };
  return formats[colidx][rowidx];
}

ProtectedCheck::ProtectedCheck ()
{
}

ProtectedCheck::~ProtectedCheck ()
{
}

bool ProtectedCheck::check ()
{
  if (!m_sheet->isProtected())
    return false;

  bool prot = false;
  Region::Iterator endOfList(cells().end());
  for (Region::Iterator it = cells().begin(); it != endOfList; ++it)
  {
    Region::Element *element = *it;
    QRect range = element->rect();

    for (int col = range.left(); col <= range.right(); ++col)
    {
      for (int row = range.top(); row <= range.bottom(); ++row)
      {
        Cell *cell = m_sheet->cellAt (col, row);
        if (!cell->format()->notProtected (col, row))
        {
          prot = true;
          break;
        }
      }
      if (prot) break;
    }
  }
  return prot;
}

