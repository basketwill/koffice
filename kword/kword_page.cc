/******************************************************************/
/* KWord - (c) by Reginald Stadlbauer 1997-1998                   */
/* based on Torben Weis' KWord                                    */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Page                                                   */
/******************************************************************/

#include "kword_doc.h"
#include "kword_page.h"
#include "kword_view.h"

/******************************************************************/
/* Class: KWPage                                                  */
/******************************************************************/

/*================================================================*/
KWPage::KWPage(QWidget *parent,KWordDocument_impl *_doc,KWordGUI *_gui)
  : QWidget(parent,""), buffer(width(),height())
{ 
  setBackgroundColor(white);
  buffer.fill(white);
  doc = _doc;
  gui = _gui;

  xOffset = 0;
  yOffset = 0;

  markerIsVisible = true;
  paint_directly = false;
  has_to_copy = false;
    
  calcVisiblePages();

  QPainter painter;
  painter.begin(&buffer);
  fc = new KWFormatContext(doc);
  fc->init(doc->getFirstParag(),painter);
  fc->cursorGotoLine(0,painter);
  painter.end();
  drawBuffer();

  setCursor(ibeamCursor);
}

// these methods are implemented here, because it didn't compile for
// some reason when they were in kword_page.h
unsigned int KWPage::ptLeftBorder() { return doc->getPTLeftBorder(); }
unsigned int KWPage::ptRightBorder() { return doc->getPTRightBorder(); }
unsigned int KWPage::ptTopBorder() { return doc->getPTTopBorder(); }
unsigned int KWPage::ptBottomBorder() { return doc->getPTBottomBorder(); }
unsigned int KWPage::ptPaperWidth() { return doc->getPTPaperWidth(); }
unsigned int KWPage::ptPaperHeight() { return doc->getPTPaperHeight(); }
unsigned int KWPage::ptColumnWidth() { return doc->getPTColumnWidth(); }
unsigned int KWPage::ptColumnSpacing() { return doc->getPTColumnSpacing(); }

/*================================================================*/
void KWPage::paintEvent(QPaintEvent* e)
{
  QPainter painter;
  if (paint_directly)
    painter.begin(this);
  else
    {
      if (has_to_copy) copyBuffer();
      painter.begin(&buffer);
    }
  painter.setClipRect(e->rect());

  painter.eraseRect(e->rect().x() - xOffset,e->rect().y() - yOffset,
		    e->rect().width(),e->rect().height());

  // Draw the page borders
  for (unsigned int i = firstVisiblePage - 1;i < lastVisiblePage;i++)
    {
      painter.setPen(lightGray);
      painter.drawRect(-xOffset + ZOOM(ptLeftBorder()) - 1,
		       -yOffset + i * ZOOM(ptPaperHeight()) + ZOOM(ptTopBorder()) - 1,
		       ZOOM(ptPaperWidth()) - ZOOM(ptLeftBorder()) - ZOOM(ptRightBorder()) + 2,
		       ZOOM(ptPaperHeight()) - ZOOM(ptTopBorder()) - ZOOM(ptBottomBorder()) + 2);

      for (unsigned int j = 1;j < doc->getColumns();j++)
	{
	  int x = -xOffset + ZOOM(ptLeftBorder() + j * ptColumnWidth() + (j - 1) * ptColumnSpacing()) + 1;
	  painter.drawLine(x,-yOffset + ZOOM(i * ptPaperHeight() + ptTopBorder()),
			   x, -yOffset + ZOOM( (i + 1) * ptPaperHeight() - ptBottomBorder()));
	    
	  x = -xOffset + ZOOM(ptLeftBorder() + j * ptColumnWidth() + j * ptColumnSpacing()) - 1;
	  painter.drawLine(x,-yOffset + ZOOM(i * ptPaperHeight() + ptTopBorder()),
			   x, -yOffset + ZOOM((i + 1) * ptPaperHeight() - ptBottomBorder()));
	}

      if (i + 1 < lastVisiblePage)
	{
	  painter.setPen(red);
	  painter.drawLine(0,(i + 1) * ZOOM(ptPaperHeight()) - yOffset,
			   width(),(i + 1) * ZOOM(ptPaperHeight()) - yOffset);
	}
    }

    
  // Any text on the display ?
  KWParag *p = doc->findFirstParagOfPage(firstVisiblePage);
  if (p)
    {
      KWFormatContext *paintfc = new KWFormatContext(doc);
      paintfc->init(p,painter);

      bool bend = false;
      while (!bend)
	{
	  doc->printLine(*paintfc,painter,xOffset,yOffset);
	  bend = !paintfc->makeNextLineLayout(painter);
	  if (paintfc->getPage() > lastVisiblePage)
	    bend = true; 
	}
	
      delete paintfc;
    }
  
  doc->drawMarker(*fc,&painter,xOffset,yOffset);
    
  painter.end();

  if (!paint_directly) drawBuffer();
}

/*================================================================*/
void KWPage::keyPressEvent(QKeyEvent *e)
{
  XKeyboardControl kbdc;
  XKeyboardState kbds;
  bool repeat = true;

  // HACK
  XGetKeyboardControl(kapp->getDisplay(),&kbds);
  repeat = kbds.global_auto_repeat;
  kbdc.auto_repeat_mode = false;
  XChangeKeyboardControl(kapp->getDisplay(),KBAutoRepeatMode,&kbdc);
  
  QPainter painter;

  painter.begin(this);
  doc->drawMarker(*fc,&painter,xOffset,yOffset);
  painter.end();

  markerIsVisible = false;
    
  painter.begin(&buffer);
  bool draw_buffer = false;

  switch(e->key())
    {
    case Key_Home:
      fc->cursorGotoLineStart(painter);
      break;
    case Key_End:
      fc->cursorGotoLineEnd(painter);
      break;
    case Key_Right:
      fc->cursorGotoRight(painter);
      break;
    case Key_Left:
      fc->cursorGotoLeft(painter);
      break;
    case Key_Up:
      fc->cursorGotoUp(painter);
      break;
    case Key_Down:
      fc->cursorGotoDown(painter);
      break;
    case Key_Shift: case Key_Control: case Key_Alt: case Key_Meta:
      // these keys do nothing at the moment
      break;
    default:
      {
	if (has_to_copy) copyBuffer();

	draw_buffer = true;
	char tmpString[2] = {0,0};
	tmpString[0] = (char)e->ascii();
	unsigned int tmpTextPos = fc->getTextPos();
	fc->getParag()->insertText(fc->getTextPos(),tmpString); 
	fc->makeLineLayout(painter);
	KWFormatContext paintfc(doc);
	paintfc = *fc;
	bool bend = false;
	while (!bend)
	  {
	    painter.fillRect(paintfc.getPTLeft() - xOffset,
			     paintfc.getPTY() - yOffset,
			     paintfc.getPTWidth(),
			     paintfc.getPTAscender() + paintfc.getPTDescender(),
			     QBrush(white));
	    doc->printLine(paintfc,painter,xOffset,yOffset);
	    bend = !paintfc.makeNextLineLayout(painter);
	    if (paintfc.getPage() > lastVisiblePage)
	      bend = true; 
	  }
	
	if (tmpTextPos + 1 <= fc->getLineEndPos())
	  fc->cursorGotoPos(tmpTextPos + 1,painter);
	else 
	  fc->cursorGotoNextLine(painter);

	doc->updateAllViews(gui->getView());
      }  break;
    }
  painter.end();
  if (draw_buffer) drawBuffer();

  scrollToCursor(*fc);
  
  painter.begin(this);
  doc->drawMarker(*fc,&painter,xOffset,yOffset);
  markerIsVisible = true;
    
  painter.end();

  // HACK
  kbdc.auto_repeat_mode = repeat;
  XChangeKeyboardControl(kapp->getDisplay(),KBAutoRepeatMode,&kbdc);
}

/*================================================================*/
void KWPage::resizeEvent(QResizeEvent *)
{
  buffer.resize(width(),height());
  calcVisiblePages();
}

/*================================================================*/
void KWPage::scrollToCursor(KWFormatContext &_fc)
{
  int cy = isCursorYVisible(_fc);
  int cx = isCursorXVisible(_fc);    

  if (cx == 0 && cy == 0)
    return;
  
  int oy = yOffset, ox = xOffset;
  if (cy < 0)
    {
      oy = _fc.getPTY();
      if (oy < 0) oy = 0;
    }
  else if (cy > 0)
    oy = _fc.getPTY() - height() + _fc.getLineHeight() + 10;
  
  if (cx < 0)
    {
      ox = _fc.getPTPos() - width() / 3;
      if (ox < 0) ox = 0;
    }
  else if (cx > 0)
    ox = _fc.getPTPos() - width() * 2 / 3;
  
  scrollToOffset(ox,oy,_fc);
}

/*================================================================*/
void KWPage::scrollToOffset(int _x,int _y,KWFormatContext &_fc)
{
  QPainter painter;
  painter.begin(this);
  doc->drawMarker(_fc,&painter,xOffset,yOffset);
  painter.end();
    
  gui->getHorzScrollBar()->setValue(_x);
  gui->getVertScrollBar()->setValue(_y);
  
  painter.begin(this);
  doc->drawMarker(_fc,&painter,xOffset,yOffset);
  painter.end();
}

/*================================================================*/
void KWPage::scroll(int dx,int dy)
{ 
  unsigned int ox = xOffset,oy = yOffset;

  xOffset -= dx;
  yOffset -= dy;
  calcVisiblePages(); 

  int x1,y1,x2,y2,w = width(),h = height();

  if (dx > 0) 
    {
      x1 = 0;
      x2 = dx;
      w -= dx;
    } 
  else 
    {
      x1 = -dx;
      x2 = 0;
      w += dx;
    }

  if (dy > 0) 
    {
      y1 = 0;
      y2 = dy;
      h -= dy;
    } 
  else 
    {
      y1 = -dy;
      y2 = 0;
      h += dy;
    }

  bitBlt(this,x2,y2,this,x1,y1,w,h);

  paint_directly = true;

  if (yOffset > oy)
    {
      dy = abs(dy);
      repaint(0,height() - abs(dy),width(),abs(dy),true);
    }
  else
    {
      dy = abs(dy);
      repaint(0,0,width(),abs(dy),true);
    }

  if (xOffset > ox)
    {
      dx = abs(dx);
      repaint(width() - abs(dx),0,abs(dx),height(),true);
    }
  else
    {
      dx = abs(dx);
      repaint(0,0,abs(dx),height(),true);
    }

  paint_directly = false;
  has_to_copy = true;
}

/*================================================================*/
int KWPage::isCursorYVisible(KWFormatContext &_fc)
{
  if ((int)_fc.getPTY() - (int)yOffset < 0)
    return -1;

  if (_fc.getPTY() - (unsigned int)yOffset + _fc.getLineHeight() > (unsigned int)height())
    return 1;

  return 0;
}

/*================================================================*/
int KWPage::isCursorXVisible(KWFormatContext &_fc)
{
  if ((int)_fc.getPTPos() - (int)xOffset < 0)
    return -1;

  if (_fc.getPTPos() - (unsigned int)xOffset + 2 > (unsigned int)width())
    return 1;

  return 0;
}

/*================================================================*/
void KWPage::calcVisiblePages()
{
  firstVisiblePage = 1 + (unsigned int)floor((float)yOffset / (float)ZOOM(ptPaperHeight()));
  lastVisiblePage = (unsigned int)ceil((float)(yOffset + height()) /
				       (float)ZOOM(ptPaperHeight()));
}

/*================================================================*/
void KWPage::drawBuffer()
{
  QPainter painter;
  painter.begin(this);
  painter.drawPixmap(0,0,buffer);
  painter.end();
}

/*================================================================*/
void KWPage::copyBuffer()
{
  bitBlt(&buffer,0,0,this,0,0,width(),height());
  has_to_copy = false;
}
