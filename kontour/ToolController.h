/* -*- C++ -*-

  $Id$

  This file is part of Kontour.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)
  Copyright (C) 2001 Igor Janssen (rm@linux.ru.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef __ToolController_h__
#define __ToolController_h__

#include <qobject.h>
#include <qptrlist.h>
#include <Tool.h>

class KontourView;
class QEvent;

namespace Kontour {

class ToolController : public QObject
{
  Q_OBJECT
public:
  ToolController(KontourView *aView);
  ~ToolController();

  KontourView *view() const {return mView; }

  void initToolBar();
  void registerTool(Tool *tool);

  void delegateEvent(QEvent *e);

public slots:
  void selectTool(Tool *t);
  void selectTool(QString id);

private slots:
  void toolActivated();

private:
  KontourView *mView;

  QPtrList<Tool> tools;
  Tool *mActiveTool;
};
};

using namespace Kontour;

#endif
