/* This file is part of the KDE project
   Copyright (C) 2002-2003 Laurent Montel <montel@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KWBOOKMARK_H
#define KWBOOKMARK_H

class KoTextParag;
class KWFrameSet;

class KWBookMark
{
public:
    KWBookMark(const QString &name);
    KWBookMark(const QString &name, KoTextParag *startParag, KoTextParag *endParag, KWFrameSet *frameSet, int start, int end);
    ~KWBookMark();
    QString bookMarkName() const { return m_name; }
    void setBookMarkName( const QString &name ) { m_name = name; }
    KWFrameSet * frameSet() const { return m_frameSet; }
    void setFrameSet(KWFrameSet * frame) { m_frameSet = frame; }

    KoTextParag *startParag() const { return m_startParag; }
    void setStartParag( KoTextParag *parag ) { m_startParag = parag; }

    KoTextParag *endParag() const { return m_endParag; }
    void setEndParag( KoTextParag *parag ) { m_endParag = parag; }

    void setBookmarkStartIndex( int pos ) { m_startIndex = pos; }
    int bookmarkStartIndex() const { return m_startIndex; }

    void setBookmarkEndIndex( int end ) { m_endIndex = end; }
    int bookmarkEndIndex() const { return m_endIndex; }

private:
    QString m_name;
    KoTextParag *m_startParag;
    KoTextParag *m_endParag;
    KWFrameSet *m_frameSet;
    int m_startIndex;
    int m_endIndex;
};

#endif /* KWBOOKMARK_H */

