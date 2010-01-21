/* libppt - library to read PowerPoint presentation
   Copyright (C) 2005 Yolla Indria <yolla.indria@gmail.com>
   Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
   Contact: Amit Aggarwal <amitcs06@gmail.com>

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
 * Boston, MA 02110-1301, USA
*/

#ifndef LIBPPT_SLIDE
#define LIBPPT_SLIDE

#include <QString>


namespace Libppt
{

class Presentation;
class GroupObject;
class TextObject;

class Slide
{
public:
    Slide(Presentation* presentation);
    ~Slide();
    void clear();
    QString title() const;
    void setTitle(const QString& title);
    GroupObject* rootObject();
    void setRootObject(GroupObject *);
    TextObject* textObject(unsigned placeId);

    double pageWidth() const;
    void setPageWidth(double pageWidth) ;
    double pageHeight() const;
    void setPageHeight(double pageHeight) ;
    unsigned int slideId();
    void setSlideId(unsigned int id);
    void setHeaderFooterFlags(int flags);
    int headerFooterFlags();
    void setStyleName(const QString &name);
    QString styleName() const;
    void setDateTimeFormatId(int formatId);
    int dateTimeFormatId();
    //Notes
    void  setNotesHeaderFooterFlags(int notesFlags);
    int   notesHeaderFooterFlags();
    void  setNotesDateTimeFormatId(int formatId);
    int   notesDateTimeFormatId();
    void  setUseFooterName(const QString &name);
    QString useFooterName();
    void  setUseHeaderName(const QString &name);
    QString useHeaderName();

private:
    // no copy or assign
    Slide(const Slide&);
    Slide& operator=(const Slide&);

    class Private;
    Private* d;
};

}

#endif /* LIBPPT_SLIDE */
