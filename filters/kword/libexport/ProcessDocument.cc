// $Header$

/*
   This file is part of the KDE project
   Copyright (C) 2001 Nicolas GOUTTE <nicog@snafu.de>
   Copyright (c) 2001 IABG mbH. All rights reserved.
                      Contact: Wolf-Michael Bolle <Bolle@IABG.de>

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
                       Contact: Wolf-Michael Bolle <Wolf-Michael.Bolle@GMX.de>

   The old file was licensed under the terms of the GNU Library General Public
   License version 2.
*/

#include <qdom.h>
#include <qvaluelist.h>

#include <kdebug.h>

#include "KWEFStructures.h"
#include "TagProcessing.h"
#include "ProcessDocument.h"
#include "KWEFKWordLeader.h"


// == KOFFICE DOCUMENT INFOMRATION ==

// TODO: verify that all document info is read!

static void ProcessAboutTag ( QDomNode         myNode,
                              void            *tagData,
                              QString         &outputText,
                              KWEFKWordLeader *exportFilter )
{
    KWEFDocumentInfo *docInfo = (KWEFDocumentInfo *) tagData;

    AllowNoAttributes (myNode);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList.append ( TagProcessing ( "title",    ProcessTextTag, (void *) &(*docInfo).title    ) );
    tagProcessingList.append ( TagProcessing ( "abstract", ProcessTextTag, (void *) &(*docInfo).abstract ) );
    ProcessSubtags (myNode, tagProcessingList, outputText, exportFilter);
}


static void ProcessAuthorTag ( QDomNode         myNode,
                               void            *tagData,
                               QString         &outputText,
                               KWEFKWordLeader *exportFilter )
{
    KWEFDocumentInfo *docInfo = (KWEFDocumentInfo *) tagData;

    AllowNoAttributes (myNode);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList.append ( TagProcessing ( "full-name",   ProcessTextTag, (void *) &(*docInfo).fullName   ) );
    tagProcessingList.append ( TagProcessing ( "title",       ProcessTextTag, (void *) &(*docInfo).jobTitle   ) );
    tagProcessingList.append ( TagProcessing ( "company",     ProcessTextTag, (void *) &(*docInfo).company    ) );
    tagProcessingList.append ( TagProcessing ( "email",       ProcessTextTag, (void *) &(*docInfo).email      ) );
    tagProcessingList.append ( TagProcessing ( "telephone",   ProcessTextTag, (void *) &(*docInfo).telephone  ) );
    tagProcessingList.append ( TagProcessing ( "fax",         ProcessTextTag, (void *) &(*docInfo).fax        ) );
    tagProcessingList.append ( TagProcessing ( "country",     ProcessTextTag, (void *) &(*docInfo).country    ) );
    tagProcessingList.append ( TagProcessing ( "postal-code", ProcessTextTag, (void *) &(*docInfo).postalCode ) );
    tagProcessingList.append ( TagProcessing ( "city",        ProcessTextTag, (void *) &(*docInfo).city       ) );
    tagProcessingList.append ( TagProcessing ( "street",      ProcessTextTag, (void *) &(*docInfo).street     ) );
    ProcessSubtags (myNode, tagProcessingList, outputText, exportFilter);
}


void ProcessDocumentInfoTag ( QDomNode         myNode,
                              void            *,
                              QString         &outputText,
                              KWEFKWordLeader *exportFilter )
{
    AllowNoAttributes (myNode);

    KWEFDocumentInfo docInfo;

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList.append ( TagProcessing ( "log",    NULL,             NULL              ) );
    tagProcessingList.append ( TagProcessing ( "author", ProcessAuthorTag, (void *) &docInfo ) );
    tagProcessingList.append ( TagProcessing ( "about",  ProcessAboutTag,  (void *) &docInfo ) );
    ProcessSubtags (myNode, tagProcessingList, outputText, exportFilter);

    exportFilter->doFullDocumentInfo (docInfo);
}


// == KWORD ==

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
// TODO: make this list up-to-date
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


// --------------------------------------------------------------------------------


static void ProcessOneAttrTag ( QDomNode  myNode,
                                QString   attrName,
                                QString   attrType,
                                void     *attrData  )
{
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing (attrName, attrType, attrData);
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode);
}


static void ProcessColorAttrTag ( QDomNode myNode, void *tagData, QString &, KWEFKWordLeader * )
{
    QColor *attrValue = (QColor *) tagData;

    int red, green, blue;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "red",   "int", (void *) &red   );
    attrProcessingList << AttrProcessing ( "green", "int", (void *) &green );
    attrProcessingList << AttrProcessing ( "blue",  "int", (void *) &blue  );
    ProcessAttributes (myNode, attrProcessingList);

    attrValue->setRgb (red, green, blue);
}


static void ProcessBoolIntAttrTag ( QDomNode  myNode,
                                    QString   attrName,
                                    void     *attrData   )
{
    bool *boolValue = (bool *) attrData;

    int intValue = -1;
    ProcessOneAttrTag (myNode, attrName, "int", (void *) &intValue);

    switch ( intValue )
    {
        case 0:
            *boolValue = false;
            break;

        case 1:
            *boolValue = true;
            break;

        case -1:
            kdError(30508) << "Bad attributes in " << myNode.nodeName () << " tag!" << endl;
            break;

        default:
            kdError(30508) << "Unexpected " << myNode.nodeName () << " attribute "
                                            << attrName << " value " << intValue << "!" << endl;
    }
}


// --------------------------------------------------------------------------------


static void ProcessIntValueTag (QDomNode myNode, void *tagData, QString &, KWEFKWordLeader *)
{
    ProcessOneAttrTag (myNode, "value", "int", tagData);
}


static void ProcessBoolIntValueTag ( QDomNode myNode, void *tagData, QString &, KWEFKWordLeader * )
{
    ProcessBoolIntAttrTag (myNode, "value", tagData);
}


static void ProcessStringValueTag ( QDomNode myNode, void *tagData, QString &, KWEFKWordLeader * )
{
    ProcessOneAttrTag (myNode, "value", "QString", tagData);
}


static void ProcessStringAlignTag ( QDomNode myNode, void *tagData, QString &, KWEFKWordLeader * )
{
    ProcessOneAttrTag (myNode, "align", "QString", tagData);
}


static void ProcessStringNameTag (QDomNode myNode, void *tagData, QString &, KWEFKWordLeader *)
{
    ProcessOneAttrTag (myNode, "name", "QString", tagData);
}


// --------------------------------------------------------------------------------


void ProcessAnchorTag ( QDomNode       myNode,
                        void          *tagData,
                        QString       &,
                        KWEFKWordLeader *         )
{
    QString *instance = (QString *) tagData;

    QString type;
    *instance = QString::null;
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "type",     "QString", (void *) &type    )
                       << AttrProcessing ( "instance", "QString", (void *) instance );
    ProcessAttributes (myNode, attrProcessingList);

    if ( type != "frameset" )
    {
       kdError (30508) << "Unknown ANCHOR type " << type << "!" << endl;
    }

    if ( (*instance).isEmpty () )
    {
        kdError (30508) << "Bad ANCHOR instance name!" << endl;
    }

    AllowNoSubtags (myNode);
}


struct LinkData
{
   QString name;
   QString href;
};


static void ProcessLinkTag (QDomNode myNode, void *tagData, QString &, KWEFKWordLeader *)
{
    LinkData *linkData = (LinkData *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("linkName", "QString", (void *) &linkData->name) );
    attrProcessingList.append ( AttrProcessing ("hrefName", "QString", (void *) &linkData->href) );
    ProcessAttributes (myNode, attrProcessingList);

#if 0
    kdError (30508) << "DEBUG: ProcessLinkTag (): " << linkData->name
                    << " references to " << linkData->href << endl;
#endif
}


static void ProcessFormatTag (QDomNode myNode, void *tagData, QString &outputText, KWEFKWordLeader *exportFilter)
{
    ValueListFormatData *formatDataList = (ValueListFormatData *) tagData;

    int formatId  = -1;
    int formatPos = -1;
    int formatLen = -1;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "id",  "int", (void *) &formatId  );
    attrProcessingList << AttrProcessing ( "pos", "int", (void *) &formatPos );
    attrProcessingList << AttrProcessing ( "len", "int", (void *) &formatLen );
    ProcessAttributes (myNode, attrProcessingList);

    switch ( formatId )
    {
        case 1:   // regular texts
            if ( formatPos != -1 && formatLen != -1 )
            {
                QString  fontName;
                bool     italic    = false;
                bool     underline = false;
                bool     strikeout = false;
                int      weight    = 50;
                int      fontSize  = -1;
                QColor   fgColor;
                QColor   bgColor;
                int      verticalAlignment = 0;
                LinkData linkData;

                QValueList<TagProcessing> tagProcessingList;
                tagProcessingList << TagProcessing ( "CHARSET",             NULL,                   NULL                        )
                                  << TagProcessing ( "ITALIC",              ProcessBoolIntValueTag, (void *) &italic            )
                                  << TagProcessing ( "UNDERLINE",           ProcessBoolIntValueTag, (void *) &underline         )
                                  << TagProcessing ( "STRIKEOUT",           ProcessBoolIntValueTag, (void *) &strikeout         )
                                  << TagProcessing ( "WEIGHT",              ProcessIntValueTag,     (void *) &weight            )
                                  << TagProcessing ( "SIZE",                ProcessIntValueTag,     (void *) &fontSize          )
                                  << TagProcessing ( "FONT",                ProcessStringNameTag,   (void *) &fontName          )
                                  << TagProcessing ( "COLOR",               ProcessColorAttrTag,    (void *) &fgColor           )
                                  << TagProcessing ( "VERTALIGN",           ProcessIntValueTag,     (void *) &verticalAlignment )
                                  << TagProcessing ( "TEXTBACKGROUNDCOLOR", ProcessColorAttrTag,    (void *) &bgColor           )
                                  << TagProcessing ( "LINK",                ProcessLinkTag,         (void *) &linkData          );
                ProcessSubtags (myNode, tagProcessingList, outputText, exportFilter);

                (*formatDataList) << FormatData ( formatPos, formatLen,
                                                  TextFormatting ( fontName, italic, underline, strikeout,
                                                                   weight, fontSize, fgColor, bgColor,
                                                                   verticalAlignment, linkData.name, linkData.href, false ) );
            }
#if 0   // happens with empty lines
            else
            {
                kdError(30508) << "Missing formatting!" << endl;
            }
#endif

            break;

        case 6:   // anchors
            if ( formatPos != -1 && formatLen != -1 )
            {
               QString instance;

               QValueList<TagProcessing> tagProcessingList;
               // TODO: We can have all layout information as in regular texts
               //       They simply apply to the table frames
               //       FONT is just the first that we've come across so far
               tagProcessingList << TagProcessing ( "FONT",   NULL,             NULL               )
                                 << TagProcessing ( "ANCHOR", ProcessAnchorTag, (void *) &instance );
               ProcessSubtags (myNode, tagProcessingList, outputText, exportFilter);

#if 0
               kdError (30508) << "DEBUG: Adding frame anchor " << instance << endl;
#endif

               (*formatDataList) << FormatData ( formatPos, formatLen, FrameAnchor (instance) );
            }
            else
            {
               kdError (30508) << "Missing or bad anchor formatting!" << endl;
            }
            break;

        case -1:
            kdError (30508) << "FORMAT attribute id value not set!" << endl;
            AllowNoSubtags (myNode);
            break;

        default:
            kdError(30508) << "Unexpected FORMAT attribute id value " << formatId << "!" << endl;
            AllowNoSubtags (myNode);
    }
}


void ProcessFormatsTag ( QDomNode myNode, void *tagData, QString &outputText, KWEFKWordLeader *exportFilter )
{
    ValueListFormatData *formatDataList = (ValueListFormatData *) tagData;

    AllowNoAttributes (myNode);

    (*formatDataList).clear ();
    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "FORMAT", ProcessFormatTag, (void *) formatDataList );
    ProcessSubtags (myNode, tagProcessingList, outputText, exportFilter);
}


// --------------------------------------------------------------------------------


static void ProcessCounterTag ( QDomNode myNode, void *tagData, QString &, KWEFKWordLeader * )
{
    CounterData *counter = (CounterData *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "numberingtype", "int",     (void *) &counter->numbering       );
    attrProcessingList << AttrProcessing ( "type",          "int",     (void *) &counter->style           );
    attrProcessingList << AttrProcessing ( "depth",         "int",     (void *) &counter->depth           );
    attrProcessingList << AttrProcessing ( "start",         "int",     (void *) &counter->start           );
    attrProcessingList << AttrProcessing ( "lefttext",      "QString", (void *) &counter->lefttext        );
    attrProcessingList << AttrProcessing ( "righttext",     "QString", (void *) &counter->righttext       );
    attrProcessingList << AttrProcessing ( "bullet",        "int",     (void *) &counter->customCharacter );
    attrProcessingList << AttrProcessing ( "bulletfont",    "QString", (void *) &counter->customFont      );
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode);
}


static void ProcessLayoutTabulatorTag ( QDomNode myNode, void *tagData, QString &, KWEFKWordLeader *)
{
    // WARNING: This is exactly the format AbiWord needs for defining its tabulators
    // TODO: make this function independant of the AbiWord export filter

    QString *tabulator = (QString *) tagData;

    double ptPos;
    int    type;

    QValueList<AttrProcessing> attrProcessingList;

    attrProcessingList << AttrProcessing ( "ptpos", "double", (void *) &ptPos );
    attrProcessingList << AttrProcessing ( "type",  "int",    (void *) &type  );
    ProcessAttributes (myNode, attrProcessingList);

    if ( tabulator->isEmpty () )
    {
        *tabulator = QString::number (ptPos);
    }
    else
    {
        *tabulator += "," + QString::number (ptPos);
    }

    *tabulator += "pt";

    switch ( type )
    {
        case 0:  *tabulator += "/L0"; break;
        case 1:  *tabulator += "/C0"; break;
        case 2:  *tabulator += "/R0"; break;
        case 3:  *tabulator += "/D0"; break;
        default: *tabulator += "/L0";
    }

    AllowNoSubtags (myNode);
}


static void ProcessIndentsTag (QDomNode myNode, void *tagData , QString&, KWEFKWordLeader *)
{
    LayoutData *layout = (LayoutData *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ("first" , "double", (void *) &layout->indentFirst );
    attrProcessingList << AttrProcessing ("left"  , "double", (void *) &layout->indentLeft  );
    attrProcessingList << AttrProcessing ("right" , "double", (void *) &layout->indentRight );
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode);
}


static void ProcessLayoutOffsetTag ( QDomNode myNode, void *tagData, QString &, KWEFKWordLeader * )
{
    LayoutData *layout = (LayoutData *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ("after" ,  "double", (void *) &layout->marginBottom );
    attrProcessingList << AttrProcessing ("before" , "double", (void *) &layout->marginTop    );
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode);
}


static void ProcessLineBreakingTag ( QDomNode myNode, void *tagData, QString &, KWEFKWordLeader *)
{
    LayoutData *layout = (LayoutData *) tagData;

    QString strBefore, strAfter;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "linesTogether",       "",        NULL                );
    attrProcessingList << AttrProcessing ( "hardFrameBreak",      "QString", (void *) &strBefore );
    attrProcessingList << AttrProcessing ( "hardFrameBreakAfter", "QString", (void *) &strAfter  );
    ProcessAttributes (myNode, attrProcessingList);

    layout->pageBreakBefore = (strBefore == "true");
    layout->pageBreakAfter  = (strAfter  == "true");

    AllowNoSubtags (myNode);
}


void ProcessLayoutTag ( QDomNode myNode, void *tagData, QString &outputText, KWEFKWordLeader *exportFilter )
// Processes <LAYOUT> and <STYLE>
{
    LayoutData *layout = (LayoutData *) tagData;

    AllowNoAttributes (myNode);

    ValueListFormatData formatDataList;

    QString lineSpacing;
    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "NAME",         ProcessStringValueTag,       &layout->styleName           );
    tagProcessingList << TagProcessing ( "FOLLOWING",    NULL,                        NULL                         );
    tagProcessingList << TagProcessing ( "COUNTER",      ProcessCounterTag,           (void *) &layout->counter    );
    tagProcessingList << TagProcessing ( "FORMAT",       ProcessFormatTag,            (void *) &formatDataList     );
    tagProcessingList << TagProcessing ( "TABULATOR",    ProcessLayoutTabulatorTag,   (void *) &layout->tabulator  );
    tagProcessingList << TagProcessing ( "FLOW",         ProcessStringAlignTag,       &layout->alignment           );
    tagProcessingList << TagProcessing ( "INDENTS",      ProcessIndentsTag,           (void *) layout              );
    tagProcessingList << TagProcessing ( "OFFSETS",      ProcessLayoutOffsetTag,      (void *) layout              );
    tagProcessingList << TagProcessing ( "LINESPACING",  ProcessStringValueTag,       &lineSpacing                 );
    tagProcessingList << TagProcessing ( "PAGEBREAKING", ProcessLineBreakingTag,      (void *) layout              );
    tagProcessingList << TagProcessing ( "LEFTBORDER",   NULL,                        NULL                         );
    tagProcessingList << TagProcessing ( "RIGHTBORDER",  NULL,                        NULL                         );
    tagProcessingList << TagProcessing ( "TOPBORDER",    NULL,                        NULL                         );
    tagProcessingList << TagProcessing ( "BOTTOMBORDER", NULL,                        NULL                         );
    ProcessSubtags (myNode, tagProcessingList, outputText, exportFilter);


    if ( formatDataList.count () )
    {
        layout->formatData = *formatDataList.begin ();

        if ( formatDataList.count () > 1 )
        {
            kdError (30508) << "More than one FORMAT tag within LAYOUT!" << endl;
        }
    }
#if 0   // happens with empty lines
    else
    {
        kdError (30508) << "No FORMAT tag within LAYOUT!" << endl;
    }
#endif


    if ( layout->styleName.isEmpty () )
    {
        layout->styleName = "Standard";

        kdError(30508) << "Bad layout name value!" << endl;
    }

    if ( lineSpacing == "oneandhalf" )
    {
        layout->lineSpacingType = 15;
    }
    else if ( lineSpacing == "double" )
    {
        layout->lineSpacingType = 20;
    }
    else
    {
        const double size = lineSpacing.toDouble ();

        if ( size >= 1.0 )
        {
            // We have a valid size
            layout->lineSpacingType = 0; // set to custom
            layout->lineSpacing     = size;
        }
        else
        {
            layout->lineSpacingType = 10; // set to 1 line
        }
    }
}


// --------------------------------------------------------------------------------


void ProcessTextTag ( QDomNode myNode, void *tagData, QString &, KWEFKWordLeader * )
{
    QString *tagText = (QString *) tagData;
    
    QDomText myText ( myNode.firstChild ().toText () );

    if ( !myText.isNull () )
    {
        *tagText = myText.data ();
    }
    else
    {
        *tagText = QString::null;
    }

    AllowNoAttributes (myNode);

    AllowNoSubtags (myNode);
}
