/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
 
   Original work :
   kis_resourceserver.h - part of KImageShop
 
   Copyright (c) 1999 Matthias Elter <elter@kde.org>

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

#ifndef KARBONRESOURCESERVER_H
#define KARBONRESOURCESERVER_H

#include <qptrlist.h>
#include <qstring.h>

#include "vpattern.h"
#include "vgradient.h"
#include "vcliparttool.h"

class VGradientListItem;
class VGradient;
class VObject;

class KarbonResourceServer
{
public:
	KarbonResourceServer();
	virtual ~KarbonResourceServer();

	int patternCount() { return m_patterns.count(); }
	QPtrList<KoIconItem> patterns() { return m_patterns; }

	int gradientCount() { return m_gradients->count(); }
	QPtrList<VGradientListItem>* gradients() { return m_gradients; }
	VGradientListItem* addGradient( VGradient* gradient );
	void removeGradient( VGradientListItem* gradient );

	int clipartCount() { return m_cliparts->count(); }
	QPtrList<VClipartIconItem>* cliparts() { return m_cliparts; }
	VClipartIconItem* addClipart( VObject* clipart, double width, double height );
	void removeClipart( VClipartIconItem* clipartIcon );

protected:
	const VPattern* loadPattern( const QString& filename );

	void loadGradient( const QString& filename );
	void saveGradient( VGradient* gradient, const QString& filename );
	
	void loadClipart( const QString& filename );
	void saveClipart( VObject* object, double width, double height, const QString& filename );
 
private:
	QPtrList<KoIconItem>         m_patterns;
	QPtrList<VGradientListItem>* m_gradients;
	QPtrList<VClipartIconItem>*  m_cliparts;
};

#endif
