/* This file is part of the KDE project
   Copyright (C) 2003 Peter Simonsson <psn@linux.se>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KIVIOGLOBAL_H
#define KIVIOGLOBAL_H

#include <koUnit.h>
#include <koGlobal.h>
#include <koSize.h>

class QDomElement;
class QPixmap;

namespace Kivio
{
  /**
   * Save a page layout to a QDomElement
   */
  void savePageLayout(QDomElement& e, KoPageLayout layout);
  /**
   * Load a page layout from a QDomElement
   */
  KoPageLayout loadPageLayout(const QDomElement& e);
  /**
   * Convert from the old TkUnit to KoUnit
   */
  KoUnit::Unit convToKoUnit(int tkUnit);
  /**
   * Return an approperiate string for the orientation
   */
  QString orientationString(KoOrientation o);
  /**
   * Return the orientation based on the string
   */
  KoOrientation orientationFromString(const QString& s);
  void setFormatOrientation(KoPageLayout& layout);

  /**
   * Load a KoSize from a QDomElement
   */
  KoSize loadSize(const QDomElement& e, const QString& name, const KoSize& def);
  /**
   * Save a KoSize to a QDomElement
   */
  void saveSize(QDomElement& e, const QString& name, const KoSize& size);

  /**
   * Returns a pixmap with all arrowheads
   */
  QPixmap arrowHeadPixmap();
}

#endif
