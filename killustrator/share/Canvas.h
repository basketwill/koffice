/* -*- C++ -*-

  $Id$

  This file is part of KIllustrator.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)

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

#ifndef Canvas_h_
#define Canvas_h_

#include <vector>

#include <iostream.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qfont.h>
#include <qarray.h>
#include <qdict.h>

#include "Painter.h"

class GDocument;
class GObject;
class ToolController;
class QwViewport;

class Canvas : public QWidget {
  Q_OBJECT
public:
  Canvas (GDocument* doc, float res, QwViewport* vp, QWidget* parent = 0, 
	  const char* name = 0);
  ~Canvas ();

  void setZoomFactor (float factor);
  float getZoomFactor () const;

  void setHorizHelplines (const vector<float>& lines);
  void setVertHelplines (const vector<float>& lines);
  const vector<float>& getHorizHelplines () const;
  const vector<float>& getVertHelplines () const;
  void alignToHelplines (bool flag);
  bool alignToHelplines ();
  void showHelplines (bool flag);
  bool showHelplines ();

  void showGrid (bool flag);
  bool showGrid () const { return gridIsOn; }

  void snapToGrid (bool flag);
  bool snapToGrid () const { return gridSnapIsOn; }

  void setGridDistance (float hdist, float vdist);
  float getHorizGridDistance () const { return hGridDistance; }
  float getVertGridDistance () const { return vGridDistance; }

  void snapPositionToGrid (float& x, float& y);

  void setToolController (ToolController *tc);
  
  void setDocument (GDocument* doc);
  GDocument* getDocument ();
  
  void printDocument ();

  void zoomIn (int x, int y);
  void zoomOut ();

  void showBasePoints (bool flag = true);
  void setOutlineMode (bool flag);

  float scaleFactor () const;

  static void initZoomFactors (QArray<float>& factors);

protected:
  void saveGridProperties ();
  void readGridProperties ();

  void mousePressEvent (QMouseEvent *e);
  void mouseReleaseEvent (QMouseEvent *e);
  void mouseMoveEvent (QMouseEvent *e);
  void keyPressEvent (QKeyEvent *e);
  void paintEvent (QPaintEvent *e);
  void moveEvent (QMoveEvent *e);

signals:
  void sizeChanged ();
  void gridStatusChanged ();
  void rightButtonAtObjectClicked (int x, int y, GObject* obj);
  void rightButtonAtSelectionClicked (int x, int y);
  void rightButtonClicked (int x, int y);

  void visibleAreaChanged (int x, int y);
  void zoomFactorChanged (float zoom);

  void mousePositionChanged (int x, int y);

public slots:
  void updateView ();
  void updateRegion (const Rect& r);
  void ensureVisibility (bool flag);
  void calculateSize ();
  void updateGridInfos ();

private slots:
  void retryUpdateRegion ();

private:
  void propagateMouseEvent (QMouseEvent *e);
  void propagateKeyEvent (QKeyEvent *e);
  void drawGrid (Painter& p);
  void drawHelplines (Painter& p);
  void redrawView (bool repaintFlag = true);

  QwViewport* viewport;
  QPixmap* pixmap;
  int width, height;
  float resolution;
  float zoomFactor;
  GDocument* document;
  ToolController* toolController;
  bool gridIsOn;
  bool gridSnapIsOn;
  bool dragging, ensureVisibilityFlag;
  float hGridDistance, vGridDistance;
  bool drawBasePoints;
  bool outlineMode;
  int pendingRedraws;
  Rect regionForUpdate, region;
  vector<float> horizHelplines, vertHelplines;
  bool helplinesAreOn, helplinesSnapIsOn;

  static QArray<float> zoomFactors;
};

#endif
