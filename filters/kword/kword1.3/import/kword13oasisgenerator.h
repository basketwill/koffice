//

/* This file is part of the KDE project
   Copyright (C) 2004 Nicolas GOUTTE <goutte@kde.org>

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

#ifndef KWORD_1_3_OASIS_GENERATOR
#define KWORD_1_3_OASIS_GENERATOR

class QString;
class KWord13Document;

class KWord13OasisGenerator
{
public:
    KWord13OasisGenerator( void );
    ~KWord13OasisGenerator( void );
    
    bool generate ( const QString& fileName, KWord13Document& kwordDocument );
    
protected:
    QString escapeOOText(const QString& strText) const;
    QString escapeOOSpan(const QString& strText) const;
};

#endif // KWORD_1_3_OASIS_GENERATOR
