// $Header$

/* This file is part of the KDE project
   Copyright (C) 2001 Nicolas GOUTTE <goutte@kde.org>

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

#include <qstringlist.h>
#include <qregexp.h>

#include <kdebug.h>

#include "ImportHelpers.h"

bool AbiPropsMap::setProperty(const QString& newName, const QString& newValue)
{
    replace(newName,AbiProps(newValue));
    return true;
}

// Treat the "props" attribute of AbiWord's tags and split it in separates names and values
void AbiPropsMap::splitAndAddAbiProps(const QString& strProps)
{
    if (strProps.isEmpty())
        return;
    // Split the properties (we do not want empty ones)
    QStringList list=QStringList::split(';',strProps,false);
    QString name,value;

    QStringList::ConstIterator it;
    for (it=list.begin();it!=list.end();it++)
    {
        const int result=(*it).find(':');
        if (result==-1)
        {
            name=(*it);
            value=QString::null;
            kdWarning(30506) << "Property without value: " << name << endl;
        }
        else
        {
            name=(*it).left(result);
            value=(*it).mid(result+1);
        }
        // kdDebug(30506) << "========== (Property :" << name.stripWhiteSpace()<< "=" << value.stripWhiteSpace() <<":)"<<endl;
        // Now set the property
        setProperty(name.stripWhiteSpace(),value.stripWhiteSpace());
    }
}

double ValueWithLengthUnit(const QString& _str)
{
    double result;
    // We search an unit (defined by a sequence of lower case characters)
    QRegExp unitExp("[a-z]+");
    const int pos=unitExp.search(_str);
    if (pos==-1)
    {
        bool flag=false;
        result=_str.toDouble(&flag);
        if (!flag)
            kdWarning(30506) << "Unknown value: " << _str << " (ValueWithLengthUnit)" << endl;
    }
    else
    {
        const double rawValue=_str.left(pos).toDouble();
        const QString strUnit=unitExp.cap();
        if (strUnit=="cm")
            result=CentimetresToPoints(rawValue);
        else if (strUnit=="in")
            result=InchesToPoints(rawValue);
        else if (strUnit=="mm")
            result=MillimetresToPoints(rawValue);
        else if (strUnit=="pt")
            result=rawValue;
        else if(strUnit=="pi")
            result=PicaToPoints(rawValue);
        else
        {
            kdWarning(30506) << "Value " << _str << " has non-supported unit: "
                << strUnit << " (ValueWithLengthUnit)" << endl;
            result=rawValue;
        }

        // kdDebug(30506) << "Value: " << _str << " Unit: " << strUnit << " Result: " << result << endl;
    }
    return result;
}
