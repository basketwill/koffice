// $Header$

/*
   This file is part of the KDE project
   Copyright (C) 2001 Nicolas GOUTTE <nicog@snafu.de>

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

/*
   This file is based on the old file:
    /home/kde/koffice/filters/kword/ascii/asciiexport.cc

   The old file was copyrighted by
    Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
    Copyright (c) 2000 ID-PRO Deutschland GmbH. All rights reserved.
                       Contact: Wolf-Michael Bolle <Bolle@ID-PRO.de>

   The old file was licensed under the terms of the GNU Library General Public
   License version 2.
*/

/*
    Important changes

    04 Feb 2001  Nicolas GOUTTE <nicog@snafu.de>
        Re-write of HTML export
        (Replaces the old HTML export code completely)

*/

#include <qtextstream.h>
#include <qdom.h>

#include <kdebug.h>

#include <htmlexport.h>
#include <htmlexport.moc>

#include "ExportTagProcessing.h"
#include "ExportDialog.h"
#include "ExportFilterBase.h"
#include "ExportFilterSpartan.h"
#include "ExportFilterDirect.h"
#include "ExportFilterStyle.h"
#include "ExportFilterFullPower.h"

// Every tag has its own processing function. All of those functions
// have the same parameters since the functions are passed to
// ProcessSubtags throuch the TagProcessing class.  The top level
// function is ProcessDocTag and can be called with the node returned
// by QDomDocument::documentElement (). The tagData argument can be
// used to either pass variables down to the subtags or to allow
// subtags to return values. As a bare minimum the tag processing
// functions must handle the tag's attributes and the tag's subtags
// (which it can choose to ignore). Currently implemented is
// processing for the following tags and attributes:
//
// DOC
//   FRAMESETS
//     FRAMESET
//       PARAGRAPH
//          TEXT - Text Element
//          FORMATS
//            FORMAT id=1 pos= len=
//          LAYOUT
//            NAME value=


static void ProcessLayoutNameTag ( QDomNode myNode, void *tagData, QString &, ClassExportFilterBase* )
{
    LayoutData *layout = (LayoutData *) tagData;

    layout->styleName = "";
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ( "value", "QString", (void *) &layout->styleName ) );
    ProcessAttributes (myNode, attrProcessingList);

    if ( layout->styleName == "" )
    {
        layout->styleName = "Standard";
        kdError(30503) << "Bad layout name value!" << endl;
    }

    AllowNoSubtags (myNode);
}

static void ProcessLayoutFlowTag ( QDomNode myNode, void *tagData, QString &, ClassExportFilterBase* )
{
    LayoutData *layout = (LayoutData *) tagData;

    layout->alignment = "";
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ( "align", "QString", (void *) &layout->alignment ) );
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode);
}

static void ProcessCounterTag ( QDomNode myNode, void *tagData, QString &, ClassExportFilterBase* )
{
    CounterData *counter = (CounterData *) tagData;
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ( "numberingtype", "int", (void *) &counter->numbering ) );
    attrProcessingList.append ( AttrProcessing ( "type", "int", (void *) &counter->style ) );
    attrProcessingList.append ( AttrProcessing ( "depth", "int", (void *) &counter->depth ) );
    attrProcessingList.append ( AttrProcessing ( "start", "int", (void *) &counter->start ) );
    attrProcessingList.append ( AttrProcessing ( "lefttext", "QString", (void *) &counter->lefttext ) );
    attrProcessingList.append ( AttrProcessing ( "righttext", "QString", (void *) &counter->righttext ) );
    attrProcessingList.append ( AttrProcessing ( "bullet", "int", (void *) &counter->customCharacter ) );
    attrProcessingList.append ( AttrProcessing ( "bulletfont", "QString", (void *) &counter->customFont ) );
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode);
}


// FORMAT's subtags

static void ProcessItalicTag (QDomNode myNode, void* formatDataPtr , QString&, ClassExportFilterBase*)
{
    FormatData* formatData = (FormatData*) formatDataPtr;

    int value=0;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("value", "int", (void *)&value) );
    ProcessAttributes (myNode, attrProcessingList);

    formatData->italic=(value!=0);
}

static void ProcessUnderlineTag (QDomNode myNode, void* formatDataPtr , QString&, ClassExportFilterBase*)
{
    FormatData* formatData = (FormatData*) formatDataPtr;

    int value=0;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("value", "int", (void *)&value) );
    ProcessAttributes (myNode, attrProcessingList);

    formatData->underline=(value!=0);
}

static void ProcessStrikeOutTag (QDomNode myNode, void* formatDataPtr , QString&, ClassExportFilterBase*)
{
    FormatData* formatData = (FormatData*) formatDataPtr;

    int value=0;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("value", "int", (void *)&value) );
    ProcessAttributes (myNode, attrProcessingList);

    formatData->strikeout=(value!=0);
}

static void ProcessWeightTag (QDomNode myNode, void* formatDataPtr , QString&, ClassExportFilterBase*)
{
    FormatData* formatData = (FormatData*) formatDataPtr;

    int weight=50;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("value", "int", (void *)&weight) );
    ProcessAttributes (myNode, attrProcessingList);

    formatData->weight=weight;
}

static void ProcessSizeTag (QDomNode myNode, void* formatDataPtr , QString&, ClassExportFilterBase*)
{
    FormatData* formatData = (FormatData*) formatDataPtr;

    int size=-1;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("value", "int", (void *)&size) );
    ProcessAttributes (myNode, attrProcessingList);

    formatData->fontSize=size;
}

static void ProcessFontTag (QDomNode myNode, void* formatDataPtr , QString&, ClassExportFilterBase* )
{
    FormatData* formatData = (FormatData*) formatDataPtr;

    QString fontName;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("name", "QString", (void *)&fontName) );
    ProcessAttributes (myNode, attrProcessingList);

    formatData->fontName=fontName;
}

static void ProcessColorTag (QDomNode myNode, void* formatDataPtr , QString&, ClassExportFilterBase*)
{
    FormatData* formatData = (FormatData*) formatDataPtr;

    int red,green,blue;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("red"   , "int", (void *)&red   ) );
    attrProcessingList.append ( AttrProcessing ("green" , "int", (void *)&green ) );
    attrProcessingList.append ( AttrProcessing ("blue"  , "int", (void *)&blue  ) );
    ProcessAttributes (myNode, attrProcessingList);

    formatData->colour.setRgb(red, green, blue);
}

static void ProcessVertAlignTag (QDomNode myNode, void* formatDataPtr , QString&, ClassExportFilterBase*)
{
    FormatData* formatData = (FormatData*) formatDataPtr;

    int value=-1;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("value", "int", (void *)&value) );
    ProcessAttributes (myNode, attrProcessingList);

    formatData->verticalAlignment=value;
}

static void ProcessSingleFormatTag (QDomNode myNode, void *tagData, QString &, ClassExportFilterBase* exportFilter)
{
    FormatData *formatData = (FormatData*) tagData;

    int formatId (-1);

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ( "id",  "int", (void *) &formatId       ) );
    attrProcessingList.append ( AttrProcessing ( "pos", "int", (void *) &formatData->pos ) );
    attrProcessingList.append ( AttrProcessing ( "len", "int", (void *) &formatData->len ) );
    ProcessAttributes (myNode, attrProcessingList);

    if ( formatId != 1 )
    {
        kdError(30503) << "Unexpected FORMAT attribute id value " << formatId << "!" << endl;
    }

    if ( formatData->pos == -1 || formatData->len == -1 )
    {
        formatData->pos = 0;
        formatData->len = 0;

        kdError(30503) << "Missing formatting!" << endl;
    }

    if ( 6 == formatId )
    {// <FORMAT id=6> have no length but has one character in <TEXT>
        //TODO: verifiy that KWord 0.9 still does it!
        formatData->realLen=1;
    }
    else
    {
        formatData->realLen=formatData->len;
    }

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList.append ( TagProcessing ( "ITALIC",    ProcessItalicTag,   (void*) formatData ) );
    tagProcessingList.append ( TagProcessing ( "UNDERLINE", ProcessUnderlineTag,(void*) formatData ) );
    tagProcessingList.append ( TagProcessing ( "STRIKEOUT", ProcessStrikeOutTag,(void*) formatData ) );
    tagProcessingList.append ( TagProcessing ( "WEIGHT",    ProcessWeightTag,   (void*) formatData ) );
    tagProcessingList.append ( TagProcessing ( "SIZE",      ProcessSizeTag,     (void*) formatData ) );
    tagProcessingList.append ( TagProcessing ( "FONT",      ProcessFontTag,     (void*) formatData ) );
    tagProcessingList.append ( TagProcessing ( "COLOR",     ProcessColorTag,    (void*) formatData ) );
    tagProcessingList.append ( TagProcessing ( "VERTALIGN", ProcessVertAlignTag,(void*) formatData ) );

    QString strDummy;

    ProcessSubtags (myNode, tagProcessingList, strDummy, exportFilter);

}

static void ProcessIndentsTag (QDomNode myNode, void* tagData , QString&, ClassExportFilterBase*)
{
    LayoutData *layout = (LayoutData *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("first" , "double", (void *)&layout->indentFirst ) );
    attrProcessingList.append ( AttrProcessing ("left"  , "double", (void *)&layout->indentLeft  ) );
    attrProcessingList.append ( AttrProcessing ("right" , "double", (void *)&layout->indentRight ) );
    ProcessAttributes (myNode, attrProcessingList);
}

static void ProcessLayoutTag ( QDomNode myNode, void *tagData, QString &outputText, ClassExportFilterBase* exportFilter )
{
    LayoutData *layout = (LayoutData *) tagData;

    AllowNoAttributes (myNode);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList.append ( TagProcessing ( "NAME",        ProcessLayoutNameTag,     (void*) layout  ) );
    tagProcessingList.append ( TagProcessing ( "FOLLOWING",   NULL,                     NULL            ) );
    tagProcessingList.append ( TagProcessing ( "COUNTER",     ProcessCounterTag,        (void*) &layout->counter ) );
    tagProcessingList.append ( TagProcessing ( "FORMAT",      ProcessSingleFormatTag,   (void*) &layout->formatData ) );
    tagProcessingList.append ( TagProcessing ( "TABULATOR",   NULL,                     NULL            ) );
    tagProcessingList.append ( TagProcessing ( "FLOW",        ProcessLayoutFlowTag,     (void*) layout  ) );
    tagProcessingList.append ( TagProcessing ( "INDENTS",     ProcessIndentsTag,        (void*) layout  ) );
    tagProcessingList.append ( TagProcessing ( "OFFSETS",     NULL,                     NULL            ) );
    tagProcessingList.append ( TagProcessing ( "LINESPACING", NULL,                     NULL            ) );
    tagProcessingList.append ( TagProcessing ( "PAGEBREAKING",NULL,                     NULL            ) );
    ProcessSubtags (myNode, tagProcessingList, outputText, exportFilter);
}

static void ProcessFormatTag (QDomNode myNode, void *tagData, QString & outputText, ClassExportFilterBase* exportFilter)
{
    // To use in <FORMATS> elements

    ValueListFormatData *formatDataList = (ValueListFormatData *) tagData;

    FormatData formatData (-1,-1);

    ProcessSingleFormatTag(myNode,(void*) &formatData, outputText, exportFilter);

    formatDataList->append (formatData);
}


static void ProcessFormatsTag ( QDomNode myNode, void *tagData, QString &outputText, ClassExportFilterBase* exportFilter )
{
    ValueListFormatData *formatDataList = (ValueListFormatData *) tagData;

    AllowNoAttributes (myNode);

    (*formatDataList).clear ();
    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList.append ( TagProcessing ( "FORMAT", ProcessFormatTag, (void *) formatDataList ) );
    ProcessSubtags (myNode, tagProcessingList, outputText, exportFilter);
}


static void ProcessTextTag ( QDomNode myNode, void *tagData, QString &, ClassExportFilterBase*)
{
    QString *tagText = (QString *) tagData;

    QDomText myText ( myNode.firstChild ().toText () );

    if ( !myText.isNull () )
    {
        *tagText = myText.data ();
    }
    else
    {
        *tagText = "";
    }

    AllowNoAttributes (myNode);

    AllowNoSubtags (myNode);
}

static void ProcessParagraphTag ( QDomNode myNode, void *, QString   &outputText, ClassExportFilterBase* exportFilter)
{
    AllowNoAttributes (myNode);

    QString paraText;
    ValueListFormatData paraFormatDataList;
    LayoutData paraLayout;
    QValueList<TagProcessing> tagProcessingList;

    tagProcessingList.append ( TagProcessing ( "TEXT",    ProcessTextTag,       (void *) &paraText           ) );
    tagProcessingList.append ( TagProcessing ( "FORMATS", ProcessFormatsTag,    (void *) &paraFormatDataList ) );
    tagProcessingList.append ( TagProcessing ( "LAYOUT",  ProcessLayoutTag,     (void *) &paraLayout         ) );
    tagProcessingList.append ( TagProcessing ( "HARDBRK", NULL,                 NULL) ); // Not documented!
    ProcessSubtags (myNode, tagProcessingList, outputText, exportFilter);

    QString strParaText;

    exportFilter->ProcessParagraphData ( paraText, paraFormatDataList, strParaText);

    if (strParaText.isEmpty())
    {
        //An empty paragraph is not allowed in HTML, so add a non-breaking space!
        strParaText="&nbsp;";
    }

    // As KWord has only one depth of lists, we can process lists very simply.
    if ( paraLayout.counter.numbering == CounterData::NUM_LIST )
    {
        if (exportFilter->inList)
        {
            // We are in a list but does it have the right type?
            if ( paraLayout.counter.style!=exportFilter->typeList)
            {
                // No, then close the previous list
                if (exportFilter->orderedList)
                {
                    outputText+="</ol>\n";
                }
                else
                {
                    outputText+="</ul>\n";
                }
                exportFilter->inList=false; // We are not in a list anymore
            }
        }

        // Are we still in a list?
        if (!exportFilter->inList)
        {
            // We are not yet part of a list
            exportFilter->inList=true;
            outputText+=exportFilter->getStartOfListOpeningTag(paraLayout.counter.style,exportFilter->orderedList);
            exportFilter->typeList=paraLayout.counter.style;
        }
        // TODO: with Cascaded Style Sheet, we could add the exact counter type we want
        outputText += exportFilter->getParagraphElement("li",strParaText,paraLayout);
    }
    else
    {
        if (exportFilter->inList)
        {
            // The previous paragraphs were in a list, so we have to close it
            if (exportFilter->orderedList)
            {
                outputText+="</ol>\n";
            }
            else
            {
                outputText+="</ul>\n";
            }
            exportFilter->inList=false;
        }
        if ( paraLayout.counter.numbering == CounterData::NUM_CHAPTER )
        {
            QString strDepth=QString("h%1").arg(paraLayout.counter.depth + 1);
            outputText += exportFilter->getParagraphElement(strDepth,strParaText,paraLayout);
        }
        else
        {
            outputText += exportFilter->getParagraphElement("p",strParaText,paraLayout);
        }
    }
}


static void ProcessFramesetTag ( QDomNode myNode, void *, QString   &outputText, ClassExportFilterBase* exportFilter)
{
    int frameType=-1;
    int frameInfo=-1;

    QValueList<AttrProcessing> attrProcessingList;

    attrProcessingList.append ( AttrProcessing ( "frameType", "int", (void*) &frameType ) );
    attrProcessingList.append ( AttrProcessing ( "frameInfo", "int", (void*) &frameInfo) );
    attrProcessingList.append ( AttrProcessing ( "removable", "", NULL ) );
    attrProcessingList.append ( AttrProcessing ( "visible",   "", NULL ) );
    attrProcessingList.append ( AttrProcessing ( "name",      "", NULL ) );
    ProcessAttributes (myNode, attrProcessingList);

    if ((1==frameType) && (0==frameInfo))
    {   //Main text

        QValueList<TagProcessing> tagProcessingList;
        tagProcessingList.append ( TagProcessing ( "FRAME",     NULL,                NULL ) );
        tagProcessingList.append ( TagProcessing ( "PARAGRAPH", ProcessParagraphTag, NULL ) );
        ProcessSubtags (myNode, tagProcessingList, outputText, exportFilter);

        // Are we still in a list?
        if (exportFilter->inList)
        {
            // We are in a list, so close it!
            if (exportFilter->orderedList)
            {
                outputText+="</ol>\n";
            }
            else
            {
                outputText+="</ul>\n";
            }
            exportFilter->inList=false;
        }
    }
    //TODO: Treat the other types of frames (frameType)
}


static void ProcessFramesetsTag (QDomNode myNode, void *, QString   &outputText, ClassExportFilterBase* exportFilter)
{
    AllowNoAttributes (myNode);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList.append ( TagProcessing ( "FRAMESET", ProcessFramesetTag, NULL ) );
    ProcessSubtags (myNode, tagProcessingList, outputText, exportFilter);
}

static void ProcessDocTag (QDomNode myNode, void *,  QString &outputText, ClassExportFilterBase* exportFilter)
{
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ( "editor",        "", NULL ) );
    attrProcessingList.append ( AttrProcessing ( "mime",          "", NULL ) );
    attrProcessingList.append ( AttrProcessing ( "syntaxVersion", "", NULL ) );
    ProcessAttributes (myNode, attrProcessingList);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList.append ( TagProcessing ( "PAPER",       NULL,                NULL ) );
    tagProcessingList.append ( TagProcessing ( "ATTRIBUTES",  NULL,                NULL ) );
    tagProcessingList.append ( TagProcessing ( "FOOTNOTEMGR", NULL,                NULL ) );
    tagProcessingList.append ( TagProcessing ( "STYLES",      NULL,                NULL ) );
    tagProcessingList.append ( TagProcessing ( "PIXMAPS",     NULL,                NULL ) );
    tagProcessingList.append ( TagProcessing ( "SERIALL",     NULL,                NULL ) );
    tagProcessingList.append ( TagProcessing ( "FRAMESETS",   ProcessFramesetsTag, NULL ) );
    ProcessSubtags (myNode, tagProcessingList, outputText, exportFilter);
}

// ClassExportFilterBase
bool ClassExportFilterBase::filter(const QString  &filenameIn, const QString  &filenameOut)
// NOTE: this member function is still define here! (TODO: decide if it should be moved too!)
{

    KoStore koStoreIn (filenameIn, KoStore::Read);

    QByteArray byteArrayIn;

    QString strTitle;
    if ( koStoreIn.open ( "documentinfo.xml" ) )
    {
        byteArrayIn = koStoreIn.read ( koStoreIn.size () );
        koStoreIn.close ();

        QDomDocument qDomDocumentInfo;
        qDomDocumentInfo.setContent (byteArrayIn);

        // Search <title> element (be carefull: there are two of them!)
        QDomNodeList docNodeList = qDomDocumentInfo.elementsByTagName("title");

        // Now find the <title> element that is child of a <about> element
        for (uint i=0; i<docNodeList.count(); i++)
        {
            QDomNode node=docNodeList.item(i);
            kdDebug(30502) << " Parent name: " << node.parentNode().nodeName() << endl;
            if (node.parentNode().nodeName()=="about")
            {
                // We have the one we want!
                //  Therefore retrieve text of element (may be empty!)
                strTitle=node.toElement().text();
                kdDebug(30502) << "Found new title " << strTitle << endl;
            }
        }
    }
    else
    {
        // Note: we do not worry too much if we cannot open the document info!
        kdWarning(30502) << "Unable to open documentinfo.xml sub-file!" << endl;
    }

    if ( !koStoreIn.open ( "root" ) )
    {
        koStoreIn.close ();

        kdError(30503) << "Unable to open input file!" << endl;
        return false;
    }

    byteArrayIn = koStoreIn.read ( koStoreIn.size () );
    koStoreIn.close ();

    // let parse the buffer just read from the file
    qDomDocumentIn.setContent(byteArrayIn);

    QDomNode docNodeIn = qDomDocumentIn.documentElement ();

    //Now all is ready to write to a file
    QFile fileOut (filenameOut);

    if ( !fileOut.open (IO_WriteOnly) )
    {
        fileOut.close ();

        kdError(30503) << "Unable to open output file!" << endl;
        return false;
    }

    //Use a QTextStream, so we do not have a big buffer using plenty of memory.
    QTextStream streamOut(&fileOut);

    streamOut.setEncoding( QTextStream::UnicodeUTF8 ); // TODO: possibility of choosing other encodings

    // Make the file header

    if (isXML())
    {   //Write out the XML declaration
        streamOut << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    }
    // write <!DOCTYPE
    streamOut << getDocType() << endl;

    // No "lang" or "xml:lang" attribute for <html>, as we do not know in which language the document is!
    streamOut << "<html"<< getHtmlOpeningTagExtraAttributes() << ">" << endl;

    streamOut << "<head>" << endl;

    // Declare that we are using UTF-8
    streamOut << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\""<< (isXML()?" /":"") << ">" << endl;

    // Say who we are (with the CVS revision number) in case we have a bug in our filter output!
    QString strVersion("$Revision$");
    // Eliminate the dollar signs
    //  (We don't want that the version number changes if the HTML file is itself put in a CVS storage.)
    streamOut << "<meta name=\"Generator\" content=\"KWord HTML Export Filter Version ="
              << strVersion.mid(10).replace(QRegExp("\\$"),"") // Note: double escape character (one for C++, one for QRegExp!)
              << "\""<< (isXML()?" /":"") // X(HT)ML closes empty elements, HTML not!
              << ">" << endl;

    if (strTitle.isEmpty())
    {
        // We have still no title for the document,
        //  therefore make one from the file name
        strTitle=filenameOut;
        const int result=strTitle.findRev("/");
        if (result>=0)
        {
            strTitle=strTitle.mid(result+1);
        }
    }

    streamOut << "<title>"<< escapeText(strTitle) <<"</title>" << endl;  // <TITLE> is mandatory in HTML 4.01 !

    //TODO: transform documentinfo.xml into many <META> elements (at least the author!)

    streamOut << getStyleElement(); //Includes an end of line at the end if it is not empty.

    streamOut << "</head>" << endl;

    streamOut << "<body" << getBodyOpeningTagExtraAttributes() << ">" << endl;

    // Now that we have the header, we can do the real work!
    QString stringBufOut;
    ProcessDocTag (docNodeIn, NULL, stringBufOut, this);
    streamOut << stringBufOut;

    // Add the tail of the file
    streamOut << "</body>\n</html>\n";

    fileOut.close();
    return true;

}

//
// HTMLExport
//

HTMLExport::HTMLExport(KoFilter *parent, const char *name) :
                     KoFilter(parent, name) {
}

bool HTMLExport::filter(const QString  &filenameIn,
                        const QString  &filenameOut,
                        const QString  &from,
                        const QString  &to,
                        const QString  &)
{
    if ((from != "application/x-kword") || (to != "text/html"))
    {
        return false;
    }

#if 1
    // Some "security" to see if I have forgotten to run "make install"
    // (Can be deleted when the filter will be stable.)
    kdDebug(30503) << "htmlexport.cc " << __DATE__ " " __TIME__ << " " << "$Revision$" << endl;
#endif

    HtmlExportDialog* dialog = new HtmlExportDialog();

    if (!dialog)
    {
        kdError(30503) << "Dialog has not been created Aborting!" << endl;
        return false;
    }

    if (!dialog->exec())
    {
        kdError(30503) << "Dialog was aborted! Aborting filter!" << endl;
        return false;
    }

    ClassExportFilterBase* exportFilter=NULL;

    if (dialog->isXHtml())
    {
        // XHTML 1.0
        switch (dialog->getMode())
        {
        case 0:
            { // "Spartan" XHTML 1.0
                kdDebug(30503) << "Spartan XHTML option" << endl;
                exportFilter=new ClassExportFilterXHtmlSpartan;
                break;
            }
        case 1:
            { // "Direct" XHTML 1.0
                kdDebug(30503) << "Direct XHTML option" << endl;
                exportFilter=new ClassExportFilterXHtmlTransitional;
                break;
            }
        default:
        case 2:
            { // "Style" XHTML 1.0
                kdDebug(30503) << "Style XHTML option" << endl;
                exportFilter=new ClassExportFilterXHtmlStyle;
                break;
            }
        case 3:
            { // "Full Power" XHTML 1.0
                kdDebug(30503) << "Full Power XHTML option" << endl;
                exportFilter=new ClassExportFilterXHtmlFullPower;
                break;
            }
        }
    }
    else
    {
        // HTML 4.01
        switch (dialog->getMode())
        {
        case 0:
            { // "Spartan" HTML 4.01
                kdDebug(30503) << "Spartan HTML option" << endl;
                exportFilter=new ClassExportFilterHtmlSpartan;
                break;
            }
        case 1:
            { // "Direct" HTML 4.01
                kdDebug(30503) << "Direct HTML option" << endl;
                exportFilter=new ClassExportFilterHtmlTransitional;
                break;
            }
        default:
        case 2:
            { // "Style" HTML 4.01
                kdDebug(30503) << "Style HTML option" << endl;
                exportFilter=new ClassExportFilterHtmlStyle;
                break;
            }
        case 3:
            { // "Full Power" HTML 4.01
                kdDebug(30503) << "Full Power HTML option" << endl;
                exportFilter=new ClassExportFilterHtmlFullPower;
                break;
            }
       }
    }

    delete dialog;

    if (!exportFilter)
    {
        kdError(30503) << "No (X)HTML filter created! Aborting! (Memory problem?)" << endl;
        return false;
    }

    // Do the work!
    const bool result = exportFilter->filter(filenameIn,filenameOut);

    delete exportFilter;

    return result;
}
