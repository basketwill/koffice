/***************************************************************************
                          karbonaiparserbase.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "karbonaiparserbase.h"

#include <core/vcolor.h>
#include <core/vlayer.h>
#include <core/vgroup.h>
#include "aicolor.h"

// generic
KarbonAIParserBase::KarbonAIParserBase() : m_pot(POT_Other), m_ptt (PTT_Output)
/* , m_strokeColor(), m_fillColor() */ {

  // A4, 70 dpi
  m_bbox.llx = 0;
  m_bbox.lly = 0;
  m_bbox.urx = 612;
  m_bbox.ury = 792;

/*  m_lineWidth = 0;
  m_flatness = 0;
  m_lineCaps = 0;
  m_lineJoin = 0;
  m_miterLimit = 10; */
  m_windingOrder = 0;

  m_fm = FM_NonZero;

  m_curKarbonPath = new VPath();

  m_document = new VDocument();
  m_layer = NULL;
  m_combination = NULL;
}

// generic
KarbonAIParserBase::~KarbonAIParserBase(){
  delete m_curKarbonPath;

  delete m_document;
}

// generic
void KarbonAIParserBase::parsingStarted(){
//  qDebug ( getHeader().latin1() );
}

// generic
void KarbonAIParserBase::parsingFinished(){
//  qDebug ( getFooter().latin1() );
}

// generic
QString KarbonAIParserBase::getParamList(Parameters& params){
  QString data("");

  Parameter *param;

  if (params.count() > 0)
  {

    for ( param=params.first(); param != 0; param=params.next() ) {
      data += " " + param->first + "=\"" + param->second + "\"";
    }
  }

  return data;
}

// generic
void KarbonAIParserBase::gotStartTag (const char *tagName, Parameters& params){
  qDebug ("<%s%s>", tagName, getParamList (params).latin1() );
}

// generic
void KarbonAIParserBase::gotEndTag (const char *tagName){
  qDebug ("</%s>", tagName );
}

// generic
void KarbonAIParserBase::gotSimpleTag (const char *tagName, Parameters& params){
  qDebug ("<%s%s/>", tagName, getParamList (params).latin1() );
}

// generic
void KarbonAIParserBase::gotPathElement (PathElement &element){
  switch (element.petype)
  {
    case PET_MoveTo :
      m_curKarbonPath->moveTo (KoPoint (element.pevalue.pointdata.x,element.pevalue.pointdata.y));
      break;
    case PET_LineTo :
      m_curKarbonPath->lineTo (KoPoint (element.pevalue.pointdata.x,element.pevalue.pointdata.y));
      break;
    case PET_CurveTo :
      m_curKarbonPath->curveTo (KoPoint (element.pevalue.bezierdata.x1,element.pevalue.bezierdata.y1),
                             KoPoint (element.pevalue.bezierdata.x2,element.pevalue.bezierdata.y2),
                             KoPoint (element.pevalue.bezierdata.x3,element.pevalue.bezierdata.y3));
      break;
    case PET_CurveToOmitC1 :
      m_curKarbonPath->curve1To (KoPoint (element.pevalue.bezierdata.x2,element.pevalue.bezierdata.y2),
                             KoPoint (element.pevalue.bezierdata.x3,element.pevalue.bezierdata.y3));
      break;
    case PET_CurveToOmitC2 :
      m_curKarbonPath->curve2To (KoPoint (element.pevalue.bezierdata.x1,element.pevalue.bezierdata.y1),
                             KoPoint (element.pevalue.bezierdata.x3,element.pevalue.bezierdata.y3));
      break;
  }
}

// generic
void KarbonAIParserBase::gotFillPath (bool closed, bool reset, FillMode fm){
//  qDebug ("found fill path");
  if (!reset) qDebug ("retain filled path");

  if (closed) m_curKarbonPath->close();

  if (!reset)
    m_pot = POT_Filled;
  else
  {
    doOutputCurrentPath2 (POT_Filled);
    m_pot = POT_Other;
  }
}

// generic
void KarbonAIParserBase::gotIgnorePath (bool closed, bool reset){
//  qDebug ("found ignore path");

  if (closed) m_curKarbonPath->close();

  if (! reset)
    m_pot = POT_Other;
  else
  {
    doOutputCurrentPath2 (POT_Ignore);
    m_pot = POT_Other;
  }
}

// generic
void KarbonAIParserBase::gotStrokePath (bool closed) {
//  qDebug ("found stroke path");

  if (closed) m_curKarbonPath->close();

  PathOutputType pot = POT_Stroked;
  if (m_pot != POT_Other)
  {
    pot = POT_FilledStroked;
  }

  doOutputCurrentPath2 (pot);

  m_pot = POT_Other;
}

// generic
void KarbonAIParserBase::gotClipPath (bool closed){

  doOutputCurrentPath2 (POT_Clip);
}

// generic
void KarbonAIParserBase::gotFillColor (AIColor &color){
//  double r, g, b;
//  color.toRGB (r,g,b);
//  qDebug ("set fillcolor to %f %f %f",r,g,b);
//  m_fillColor = color;

  VColor karbonColor = toKarbonColor (color);
  m_fill.setColor (karbonColor);
}

// generic
void KarbonAIParserBase::gotStrokeColor (AIColor &color){
//  double r, g, b;
//  color.toRGB (r,g,b);
//  qDebug ("set strokecolor to %f %f %f",r,g,b);
//  m_strokeColor = color;

  VColor karbonColor = toKarbonColor (color);
  m_stroke.setColor (karbonColor);
}

// generic
void KarbonAIParserBase::gotBoundingBox (int llx, int lly, int urx, int ury){
  m_bbox.llx = llx;
  m_bbox.lly = lly;
  m_bbox.urx = urx;
  m_bbox.ury = ury;
}

void KarbonAIParserBase::gotLineWidth (double val){
//  m_lineWidth = val;
  m_stroke.setLineWidth (val);
}

void KarbonAIParserBase::gotFlatness (double val)
{
//  m_flatness = val;
//  m_stroke.setFlatness (val);
}

void KarbonAIParserBase::gotLineCaps (int val)
{
//  m_lineCaps = val;
  VLineCap lineCap = cap_butt;

  switch (val)
  {
	  case 0 : lineCap = cap_butt; break;
	  case 1 : lineCap = cap_round; break;
	  case 2 : lineCap = cap_square; break;
  }

  m_stroke.setLineCap (lineCap);
}

void KarbonAIParserBase::gotLineJoin (int val)
{
//  m_lineJoin = val;

  VLineJoin lineJoin = join_miter;

  switch (val)
  {
	  case 0 : lineJoin = join_miter; break;
	  case 1 : lineJoin = join_round; break;
	  case 2 : lineJoin = join_bevel; break;
  }

  m_stroke.setLineJoin (lineJoin);

}

void KarbonAIParserBase::gotMiterLimit (double val)
{
//  m_miterLimit = val;
  m_stroke.setMiterLimit (val);
}

void KarbonAIParserBase::gotWindingOrder (int val)
{
  m_windingOrder = val;
}

void KarbonAIParserBase::gotBeginGroup (bool clipping)
{
//  qDebug ("start begin group");
  VGroup *group = new VGroup();
  m_groupStack.push (group);
//  qDebug ("end begin group");

}

void KarbonAIParserBase::gotEndGroup (bool clipping)
{
//  qDebug ("start end group");

  VGroup *group = m_groupStack.pop();

  if (m_groupStack.isEmpty())
  {
    m_layer->appendObject (group);
  }
  else
  {
    m_groupStack.top()->insertObject (group);
  }

//  qDebug ("end end group");
}

void KarbonAIParserBase::gotBeginCombination () {
  m_ptt = PTT_Combine;
}

void KarbonAIParserBase::gotEndCombination () {
//  qDebug ( "got end combination" );

  m_ptt = PTT_Output;

  if (m_combination != NULL)
  {
    m_curKarbonPath = m_combination;
    doOutputCurrentPath2 (POT_Leave);
  }

  m_combination = NULL;
}


const VColor KarbonAIParserBase::toKarbonColor (const AIColor &color)
{
  AIColor temp (color);
  VColor value;

  double v1, v2, v3, v4;
  temp.toCMYK (v1, v2, v3, v4);

  float cv1 = v1;
  float cv2 = v2;
  float cv3 = v3;
  float cv4 = v4;

  value.setColorSpace (VColor::cmyk);
  value.setValues (&cv1, &cv2, &cv3, &cv4);

  return value;
}

void KarbonAIParserBase::doOutputCurrentPath2(PathOutputType type)
{
  if (!m_layer)
  {
    m_layer = new VLayer();
    m_document->insertLayer (m_layer);
  }

  if (type != POT_Leave)
  {
    if ((type != POT_Filled) && (type != POT_Stroked) && (type != POT_FilledStroked)) return;
    if ((type == POT_Filled) || (type == POT_FilledStroked))
    {
/*      VFill fill;
      fill.setColor (toKarbonColor (m_fillColor));
      m_curKarbonPath->setFill(fill); */
      m_curKarbonPath->setFill(m_fill);
    }

    if ((type == POT_Stroked) || (type == POT_FilledStroked))
    {
/*      VStroke stroke;
      stroke.setColor (toKarbonColor (m_strokeColor));
      m_curKarbonPath->setStroke (stroke); */
      m_curKarbonPath->setStroke (m_stroke);
    }
  }

  if (m_ptt == PTT_Combine)
  {
//    m_pot |= type;
    if (m_combination == NULL)
      m_combination = m_curKarbonPath;
    else
      m_combination->combine (*m_curKarbonPath);

    m_curKarbonPath = new VPath();

    return;
  }

  if (m_groupStack.isEmpty())
  {
    m_layer->appendObject(m_curKarbonPath);
  }
  else
  {
    m_groupStack.top()->insertObject(m_curKarbonPath);
  }

  m_curKarbonPath = new VPath();
}

bool KarbonAIParserBase::parse (QIODevice& fin, QDomDocument &doc)
{

  bool res = AIParserBase::parse (fin);

//  qDebug ("document is %s",doc.toString().latin1());
  if (res)
  {
      qDebug ("before save document");
      m_document->save (doc);
      qDebug ("after save document");
  }
  else
  {
    QDomDocument tempDoc;
    doc = tempDoc;
  }

  return res;
}
