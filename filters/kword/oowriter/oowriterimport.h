/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
   Copyright (C) 2003 David Faure <faure@kde.org>

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

#ifndef OoWriter_IMPORT_H__
#define OoWriter_IMPORT_H__

#include <koFilter.h>

#include <qdict.h>
#include <qdom.h>
#include <stylestack.h>

class KoStore;

class OoWriterImport : public KoFilter
{
    Q_OBJECT
public:
    OoWriterImport( KoFilter * parent, const char * name, const QStringList & );
    virtual ~OoWriterImport();

    virtual KoFilter::ConversionStatus convert( QCString const & from, QCString const & to );

private:
    void prepareDocument( QDomDocument& mainDocument, QDomElement& framesetsElem );
    void createPageDocument( QDomDocument& mainDocument, QDomElement& framesetsElem );
    QDomElement parseList( QDomDocument& doc, const QDomElement& list );
    QDomElement parseParagraph( QDomDocument& doc, const QDomElement& paragraph );
    void parseSpanOrSimilar( QDomDocument& doc, const QDomElement& parent, QDomElement& kwordParagraph, QDomElement& kwordFormats, QString& paragraphText, uint& pos);
    // Reads from m_styleStack, writes the text properties to parentElement
    void writeFormat( QDomDocument& doc, QDomElement& parentElement, int id, int pos, int length );
    // Reads from m_styleStack, writes the paragraph properties to layoutElement
    void writeLayout( QDomDocument& doc, QDomElement& layoutElement );
    void createInitialFrame( QDomElement& parentFramesetElem, int top, int bottom, bool headerFooter );
    void createStyles( QDomDocument &doc );
    void createDocumentInfo( QDomDocument &docinfo );
    void createDocumentContent( QDomDocument &doccontent, QDomElement& mainFramesetElement );
    KoFilter::ConversionStatus openFile();
    bool createStyleMap( const QDomDocument & styles );
    void insertStyles( const QDomElement& element );
    void fillStyleStack( const QDomElement& object );
    void addStyles( const QDomElement* style );
    void appendPicture(QDomDocument& doc, QDomElement& formats, const QDomElement& object, uint pos);
    void appendKWordVariable(QDomDocument& doc, QDomElement& formats, const QDomElement& object, uint pos,
        const QString& key, int type, QDomElement& child);

    
    QDomDocument    m_content;
    QDomDocument    m_meta;
    QDomDocument    m_settings;
    QDomDocument    m_stylesDoc;
    QDomElement     m_masterPage;

    QDict<QDomElement>   m_styles;
    StyleStack m_styleStack;
    
    uint m_pictureNumber; // Number of the picture (increment *before* use)
    KoStore* m_oostore; // Input KoStore
};

#endif

