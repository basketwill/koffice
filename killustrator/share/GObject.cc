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

#include <qdom.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <time.h>
#include <utility>
#include <GObject.h>

#include <GPolyline.h>
#include <GPolygon.h>
#include <GOval.h>
#include <GText.h>
#include <GGroup.h>
#include <GClipart.h>
#include <GBezier.h>
#include <Gradient.h>
#include <GPixmap.h>
#include <GCurve.h>

#include <kdebug.h>

using namespace std;

GObject::OutlineInfo GObject::defaultOutlineInfo;
GObject::FillInfo GObject::defaultFillInfo;

map<string, GObject*> GObject::prototypes;

void GObject::setDefaultOutlineInfo (const OutlineInfo& oi) {
  if (oi.mask & OutlineInfo::Color)
    defaultOutlineInfo.color = oi.color;
  if (oi.mask & OutlineInfo::Style)
    defaultOutlineInfo.style = oi.style;
  if (oi.mask & OutlineInfo::Width)
    defaultOutlineInfo.width = oi.width;
}

void GObject::setDefaultFillInfo (const FillInfo& fi) {
  if (fi.mask & FillInfo::Color)
    defaultFillInfo.color = fi.color;
  if (fi.mask & FillInfo::FillStyle)
    defaultFillInfo.fstyle = fi.fstyle;
  if (fi.mask & FillInfo::Pattern)
    defaultFillInfo.pattern = fi.pattern;
  if (fi.mask & FillInfo::GradientInfo)
    defaultFillInfo.gradient = fi.gradient;
}

GObject::OutlineInfo GObject::getDefaultOutlineInfo () {
  return defaultOutlineInfo;
}

GObject::FillInfo GObject::getDefaultFillInfo () {
  return defaultFillInfo;
}

GObject::GObject () {
  sflag = false;
  layer = 0L;
  inWork = false;
  wrapper = 0L;

  outlineInfo = defaultOutlineInfo;
  outlineInfo.mask = OutlineInfo::Color | OutlineInfo::Style |
    OutlineInfo::Width | OutlineInfo::Custom;
  outlineInfo.roundness = 0;
  outlineInfo.shape = OutlineInfo::DefaultShape;
  outlineInfo.startArrowId = outlineInfo.endArrowId = 0;

  fillInfo = defaultFillInfo;
  fillInfo.mask = FillInfo::Color | FillInfo::FillStyle;

  rcount = 1;
}

GObject::GObject (const QDomElement &element) {

    kdDebug() << "entering GObject() - element: " << element.tagName() << endl;
    layer = 0L;
    inWork = false;
    wrapper = 0L;

    outlineInfo.mask = 0;
    fillInfo.mask = 0;
    id = (const char*)element.attribute("id");
    refid = (const char*)element.attribute("ref");
    kdDebug() << "id: " << id << endl;
    kdDebug() << "refId: " << refid << endl;

    outlineInfo.color = QColor(element.attribute("strokecolor"));
    outlineInfo.mask |= OutlineInfo::Color;

    outlineInfo.style = (QT_PRFX::PenStyle) element.attribute("strokestyle").toInt();
    outlineInfo.mask |= OutlineInfo::Style;

    outlineInfo.width = element.attribute("linewidth").toFloat();
    outlineInfo.mask |= OutlineInfo::Width;

    fillInfo.fstyle = (FillInfo::Style) element.attribute("fillstyle").toInt();
    fillInfo.mask |= FillInfo::FillStyle;

    fillInfo.color = QColor(element.attribute("fillcolor"));
    fillInfo.mask |= FillInfo::Color;

    fillInfo.pattern = (QT_PRFX::BrushStyle) element.attribute("fillpattern").toInt();
    fillInfo.mask |= FillInfo::Pattern;

    fillInfo.gradient.setColor1 (QColor(element.attribute("gradcolor1")));
    fillInfo.mask |= FillInfo::GradientInfo;

    fillInfo.gradient.setColor2 (QColor(element.attribute("gradcolor2")));
    fillInfo.mask |= FillInfo::GradientInfo;

    fillInfo.gradient.setStyle ((Gradient::Style) element.attribute("gradstyle").toInt());
    fillInfo.mask |= FillInfo::GradientInfo;

    tMatrix=KIllustrator::toMatrix(element.namedItem("matrix").toElement());
    iMatrix = tMatrix.invert ();
    tmpMatrix = tMatrix;
    kdDebug() << "leaving GObject()" << endl;
}

GObject::GObject (const GObject& obj) : QObject()
{
  sflag = false;
  outlineInfo = obj.outlineInfo;
  fillInfo = obj.fillInfo;
  tMatrix = obj.tMatrix;
  tmpMatrix = tMatrix;
  iMatrix = obj.iMatrix;
  layer = obj.layer;
  inWork = false;
  wrapper = 0L;

  rcount = 1;
}

GObject::~GObject () {
}

void GObject::ref () {
  rcount++;
}

void GObject::unref () {
  if (--rcount == 0)
    delete this;
}

void GObject::updateRegion (bool recalcBBox) {
  Rect newbox = redrawBox ();
  if (recalcBBox) {
    Rect oldbox = newbox;
    calcBoundingBox ();
    newbox = redrawBox ().unite (oldbox);
  }

  if (isSelected ())
    // the object is selected, so enlarge the update region in order
    // to redraw the handle
    newbox.enlarge (8);
  else
    // a workaround for some problems
    newbox.enlarge (2);

  emit changed (newbox);
}

void GObject::transform (const QWMatrix& m, bool update) {
  tMatrix = tMatrix * m;
  iMatrix = tMatrix.invert ();
  initTmpMatrix ();
  gShape.setInvalid ();
  if (update)
    updateRegion ();
}

void GObject::initTmpMatrix () {
  tmpMatrix = tMatrix;
}

void GObject::ttransform (const QWMatrix& m, bool update) {
  tmpMatrix = tmpMatrix * m;
  if (update)
    updateRegion ();
}

void GObject::setOutlineInfo (const GObject::OutlineInfo& info) {
  if (info.mask & OutlineInfo::Color)
    outlineInfo.color = info.color;
  if (info.mask & OutlineInfo::Style)
    outlineInfo.style = info.style;
  if (info.mask & OutlineInfo::Width)
    outlineInfo.width = info.width;
  if (info.mask & OutlineInfo::Custom) {
    outlineInfo.roundness = info.roundness;
    outlineInfo.shape = info.shape;
    outlineInfo.startArrowId = info.startArrowId;
    outlineInfo.endArrowId = info.endArrowId;
  }
  updateRegion (false);
  emit propertiesChanged (Prop_Outline, info.mask);
}

GObject::OutlineInfo GObject::getOutlineInfo () const {
  return outlineInfo;
}

void GObject::setOutlineShape (OutlineInfo::Shape s) {
  outlineInfo.shape = s;
  updateRegion ();
  emit propertiesChanged (Prop_Outline, OutlineInfo::Custom);
}

void GObject::setOutlineColor (const QColor& color) {
  outlineInfo.color = color;
  updateRegion (false);
  emit propertiesChanged (Prop_Outline, OutlineInfo::Color);
}

void GObject::setOutlineStyle (QT_PRFX::PenStyle style) {
  outlineInfo.style = style;
  updateRegion (false);
  emit propertiesChanged (Prop_Outline, OutlineInfo::Style);
}

void GObject::setOutlineWidth (float width) {
  outlineInfo.width = width;
  updateRegion (false);
  emit propertiesChanged (Prop_Outline, OutlineInfo::Width);
}

const QColor& GObject::getOutlineColor () const {
  return outlineInfo.color;
}

QT_PRFX::PenStyle GObject::getOutlineStyle () const {
  return outlineInfo.style;
}

float GObject::getOutlineWidth () const {
  return outlineInfo.width;
}

void GObject::setFillInfo (const GObject::FillInfo& info) {
  if (info.mask & FillInfo::Color)
    fillInfo.color = info.color;
  if (info.mask & FillInfo::FillStyle)
    fillInfo.fstyle = info.fstyle;
  if (info.mask & FillInfo::Pattern)
    fillInfo.pattern = info.pattern;
  if (info.mask & FillInfo::GradientInfo)
    fillInfo.gradient = info.gradient;
  gShape.setInvalid ();
  updateRegion (false);
  emit propertiesChanged (Prop_Fill, info.mask);
}

GObject::FillInfo GObject::getFillInfo () const {
  return fillInfo;
}

void GObject::setFillColor (const QColor& color) {
  fillInfo.color = color;
  updateRegion (false);
  emit propertiesChanged (Prop_Fill, FillInfo::Color);
}

const QColor& GObject::getFillColor () const {
  return fillInfo.color;
}

void GObject::setFillPattern (QT_PRFX::BrushStyle b) {
  fillInfo.pattern = b;
  updateRegion (false);
  emit propertiesChanged (Prop_Fill, FillInfo::Pattern);
}

void GObject::setFillGradient (const Gradient& g) {
  fillInfo.gradient = g;
  gShape.setInvalid ();
  updateRegion (false);
  emit propertiesChanged (Prop_Fill, FillInfo::GradientInfo);
}

void GObject::setFillStyle (GObject::FillInfo::Style s) {
  fillInfo.fstyle = s;
  gShape.setInvalid ();
  updateRegion (false);
  emit propertiesChanged (Prop_Fill, FillInfo::FillStyle);
}

GObject::FillInfo::Style GObject::getFillStyle () const {
  return fillInfo.fstyle;
}

const Gradient& GObject::getFillGradient () const {
  return fillInfo.gradient;
}

QT_PRFX::BrushStyle GObject::getFillPattern () const {
  return fillInfo.pattern;
}

void GObject::select (bool flag) {
  sflag = flag;
}

bool GObject::contains (const Coord& p) {
  return box.contains (p);
}

bool GObject::intersects (const Rect& r) {
  return r.intersects (box);
}

void GObject::setLayer (GLayer* l) {
  layer = l;
  if (l == 0L)
    emit deleted ();
}

void GObject::updateBoundingBox (const Rect& r) {
  box = r.normalize ();
}

void GObject::updateBoundingBox (const Coord& p1, const Coord& p2) {
  Rect r (p1, p2);
  updateBoundingBox (r);
}

GOState* GObject::saveState () {
  GOState* state = new GOState;
  GObject::initState (state);
  return state;
}

void GObject::initState (GOState* state) {
  state->matrix = tMatrix;
  state->fInfo = fillInfo;
  state->oInfo = outlineInfo;
}

void GObject::restoreState (GOState* state) {
  tMatrix = state->matrix;
  iMatrix = tMatrix.invert ();
  tmpMatrix = tMatrix;
  setFillInfo (state->fInfo);
  setOutlineInfo (state->oInfo);

  updateRegion ();
}

void GObject::calcUntransformedBoundingBox (const Coord& tleft,
					    const Coord& tright,
					    const Coord& bright,
					    const Coord& bleft) {
  Coord p[4];
  Rect r;

  p[0] = tleft.transform (tmpMatrix);
  p[1] = tright.transform (tmpMatrix);
  p[2] = bleft.transform (tmpMatrix);
  p[3] = bright.transform (tmpMatrix);

  r.left (p[0].x ());
  r.top (p[0].y ());
  r.right (p[0].x ());
  r.bottom (p[0].y ());

  for (unsigned int i = 1; i < 4; i++) {
    r.left (QMIN(p[i].x (), r.left ()));
    r.top (QMIN(p[i].y (), r.top ()));
    r.right (QMAX(p[i].x (), r.right ()));
    r.bottom (QMAX(p[i].y (), r.bottom ()));
  }
  updateBoundingBox (r);
}

void GObject::initBrush (QBrush& brush) {
  switch (fillInfo.fstyle) {
  case GObject::FillInfo::NoFill:
    brush.setStyle (NoBrush);
    break;
  case GObject::FillInfo::SolidFill:
    brush.setColor (fillInfo.color);
    brush.setStyle (SolidPattern);
    break;
  case GObject::FillInfo::PatternFill:
    brush.setColor (fillInfo.color);
    brush.setStyle (fillInfo.pattern);
    break;
  default:
    brush.setStyle (NoBrush);
    break;
  }
}

void GObject::initPen (QPen& pen) {
  pen.setColor (inWork ? black : outlineInfo.color);
  pen.setWidth ((uint) outlineInfo.width);
  pen.setStyle (inWork ? SolidLine : outlineInfo.style);
}

QDomElement GObject::writeToXml (QDomDocument &document) {

    QDomElement element=document.createElement("gobject");
    if (hasId ())
	element.setAttribute ("id", (const char *) id);
    if(hasRefId())
	element.setAttribute("ref", getRefId());
    element.setAttribute ("strokecolor", outlineInfo.color.name());
    element.setAttribute ("strokestyle", (int) outlineInfo.style);
    element.setAttribute ("linewidth", outlineInfo.width);
    element.setAttribute ("fillstyle", (int) fillInfo.fstyle);
    switch (fillInfo.fstyle) {
    case FillInfo::SolidFill:
	element.setAttribute ("fillcolor", fillInfo.color.name());
	break;
    case FillInfo::PatternFill:
	element.setAttribute ("fillcolor", fillInfo.color.name());
	element.setAttribute ("fillpattern", (int) fillInfo.pattern);
	break;
    case FillInfo::GradientFill:
	element.setAttribute ("gradcolor1", fillInfo.gradient.getColor1().name());
	element.setAttribute ("gradcolor2", fillInfo.gradient.getColor2().name());
	element.setAttribute ("gradstyle", (int) fillInfo.gradient.getStyle());
	break;
    case FillInfo::NoFill:
    default:
	// nothing more
	break;
    }
    element.appendChild(KIllustrator::createMatrixElement("matrix", tMatrix, document));
    return element;
}

void GObject::printInfo () {
    cout << className () << " bbox = [" << boundingBox () << "]" << endl;
}

void GObject::invalidateClipRegion  () {
  if (gradientFill ())
    gShape.setInvalid ();
}

const char* GObject::getId () {
  if (! hasId ())
    id.sprintf ("id%ld", (long) time ((time_t) 0L));
  return (const char *) id;
}

void GObject::registerPrototype (const char *className, GObject* proto) {
  prototypes[className] = proto;
}

GObject* GObject::lookupPrototype (const char *className) {
  GObject* result = 0L;
  map<string, GObject*>::iterator it = prototypes.find (className);
  if (it != prototypes.end ())
    result = it->second;
  return result;
}

void GObject::setWrapper (SWrapper *wobj) {
  wobj->setObject (this);
}

QDomElement KIllustrator::createMatrixElement(const QString &tag, const QWMatrix &matrix, QDomDocument &document) {

    QDomElement m=document.createElement(tag);
    m.setAttribute("m11", matrix.m11());
    m.setAttribute("m12", matrix.m12());
    m.setAttribute("m21", matrix.m21());
    m.setAttribute("m22", matrix.m22());
    m.setAttribute("dx", matrix.dx());
    m.setAttribute("dy", matrix.dy());
    return m;
}

QWMatrix KIllustrator::toMatrix(const QDomElement &matrix) {

    double m11=matrix.attribute("m11").toDouble();
    double m12=matrix.attribute("m12").toDouble();
    double m21=matrix.attribute("m21").toDouble();
    double m22=matrix.attribute("m22").toDouble();
    double dx=matrix.attribute("dx").toDouble();
    double dy=matrix.attribute("dy").toDouble();
    return QWMatrix(m11, m12, m21, m22, dx, dy);
}

GObject *KIllustrator::objectFactory(const QDomElement &element) {

    if (element.tagName () == "polyline")
	return new GPolyline (element);
    else if (element.tagName () == "ellipse")
	return new GOval (element);
    else if (element.tagName () == "bezier")
	return new GBezier (element);
    else if (element.tagName () == "rectangle")
	return new GPolygon (element, GPolygon::PK_Rectangle);
    else if (element.tagName () == "polygon")
	return new GPolygon (element);
    else if (element.tagName () == "clipart")
	return new GClipart (element);
    else if (element.tagName () == "pixmap")
	return new GPixmap (element);
    else if (element.tagName () == "curve")
	return new GCurve (element);
    else if (element.tagName() == "text")
	return new GText(element);
    else if (element.tagName() == "group")
	return new GGroup (element);
    else {
	GObject *obj;
	GObject *proto = GObject::lookupPrototype (element.tagName());
	if (proto != 0L) {
	    obj = proto->clone (element);
	}
	else
	    kdDebug() << "invalid object type: " << element.tagName() << endl;
	return obj;
    }
}

#include <GObject.moc>
