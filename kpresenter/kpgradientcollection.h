/******************************************************************/
/* KPresenter - (c) by Reginald Stadlbauer 1998                   */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* needs c++ library Qt (http://www.troll.no)                     */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* written for KDE (http://www.kde.org)                           */
/* KPresenter is under GNU GPL                                    */
/******************************************************************/
/* Module: gradient collection (header)                           */
/******************************************************************/

#ifndef kpgradientcollection_h
#define kpgradientcollection_h

#include <qobject.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qlist.h>
#include <qcolor.h>

#include "kpgradient.h"
#include "global.h"

/******************************************************************/
/* Class: KPGradientCollection                                    */
/******************************************************************/

class KPGradientCollection : public QObject
{
  Q_OBJECT

public:
  KPGradientCollection()
    { gradientList.setAutoDelete(true); }

  ~KPGradientCollection()
    { gradientList.clear(); }

  virtual QPixmap* getGradient(QColor _color1,QColor _color2,BCType _bcType,QSize _size,bool addref = true);

  virtual void removeRef(QColor _color1,QColor _color2,BCType _bcType,QSize _size);

protected:
  virtual int inGradientList(QColor _color1,QColor _color2,BCType _bcType,QSize _size);

  QList<KPGradient> gradientList;

};

#endif
