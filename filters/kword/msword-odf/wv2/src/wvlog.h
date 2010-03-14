/* This file is part of the wvWare 2 project
   Copyright (C) 2003 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111-1307, USA.
*/

#ifndef WVLOG_H
#define WVLOG_H

#include <iostream>
#include <kdebug.h>

#define wvlog kDebug()

KDECORE_EXPORT QDebug operator<<(QDebug s, const std::string &o);
KDECORE_EXPORT QDebug operator<<(QDebug s, std::basic_ostream<char>& (*o)( std::basic_ostream<char>& ));

#endif // WVLOG_H
