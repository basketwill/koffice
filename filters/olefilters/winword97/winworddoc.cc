/* This file is part of the KDE project
   Copyright (C) 1999 Werner Trobin <trobin@kde.org>

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

#include <winworddoc.h>
#include <qarray.h>
#define DISABLE_FLOATING true

WinWordDoc::WinWordDoc(
    QDomDocument &part,
    const myFile &mainStream,
    const myFile &table0Stream,
    const myFile &table1Stream,
    const myFile &dataStream) :
        MsWord(
            mainStream.data,
            table0Stream.data,
            table1Stream.data,
            dataStream.data),
        m_part(part)
{
    m_phase = INIT;
    m_success = TRUE;
    m_body = QString("");
    m_tableManager = 0;
    m_cellEdges.setAutoDelete(true);
}

WinWordDoc::~WinWordDoc()
{
}

const bool WinWordDoc::convert()
{
    // We do the conversion in two passes, to allow all the tables to be turned into framesets
    // after the main frameset with the text.

    if (m_phase == INIT)
    {
        m_body.append(
            "  <FRAMESET frameType=\"1\" frameInfo=\"0\" removeable=\"0\" visible=\"1\">\n"
            "   <FRAME left=\"");
        m_body.append(QString::number(s_hMargin));
        m_body.append("\" top=\"");
        m_body.append(QString::number(s_vMargin));
        m_body.append("\" right=\"");
        m_body.append(QString::number(s_width - s_hMargin));
        m_body.append("\" bottom=\"");
        m_body.append(QString::number(s_height - s_vMargin));
        m_body.append("\" runaround=\"1\" runaGapPT=\"2\" runaGapMM=\"1\" runaGapINCH=\"0.0393701\"  lWidth=\"1\" lRed=\"255\" lGreen=\"255\" lBlue=\"255\" lStyle=\"0\"  rWidth=\"1\" rRed=\"255\" rGreen=\"255\" rBlue=\"255\" rStyle=\"0\"  tWidth=\"1\" tRed=\"255\" tGreen=\"255\" tBlue=\"255\" tStyle=\"0\"  bWidth=\"1\" bRed=\"255\" bGreen=\"255\" bBlue=\"255\" bStyle=\"0\" bkRed=\"255\" bkGreen=\"255\" bkBlue=\"255\" bleftpt=\"0\" bleftmm=\"0\" bleftinch=\"0\" brightpt=\"0\" brightmm=\"0\" brightinch=\"0\" btoppt=\"0\" btopmm=\"0\" btopinch=\"0\" bbottompt=\"0\" bbottommm=\"0\" bbottominch=\"0");
        m_body.append("\" autoCreateNewFrame=\"1\" newFrameBehaviour=\"0\"/>\n");
        m_phase = TEXT_PASS;
        m_tableManager = 0;
        parse();
        m_body.append(
            "  </FRAMESET>\n");
        if (m_success)
        {
            m_phase = TABLE_PASS;
            m_tableManager = 0;
            parse();
        }
    }

    if (m_phase != DONE)
    {
        QString newstr;

        newstr = QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE DOC >\n"
            "<DOC author=\"Reginald Stadlbauer and Torben Weis\" email=\"reggie@kde.org and weis@kde.org\" editor=\"KWord\" mime=\"application/x-kword\">\n"
            " <PAPER format=\"1\" ptWidth=\"595\" ptHeight=\"841\" mmWidth =\"210\" mmHeight=\"297\" inchWidth =\"8.26772\" inchHeight=\"11.6929\" orientation=\"0\" columns=\"1\" ptColumnspc=\"2\" mmColumnspc=\"1\" inchColumnspc=\"0.0393701\" hType=\"0\" fType=\"0\" ptHeadBody=\"9\" ptFootBody=\"9\" mmHeadBody=\"3.5\" mmFootBody=\"3.5\" inchHeadBody=\"0.137795\" inchFootBody=\"0.137795\">\n"
            "  <PAPERBORDERS mmLeft=\"10\" mmTop=\"15\" mmRight=\"10\" mmBottom=\"15\" ptLeft=\"");
        newstr.append(QString::number(s_hMargin));
        newstr.append("\" ptTop=\"");
        newstr.append(QString::number(s_vMargin));
        newstr.append("\" ptRight=\"");
        newstr.append(QString::number(s_hMargin));
        newstr.append("\" ptBottom=\"");
        newstr.append(QString::number(s_vMargin));
        newstr.append("\" inchLeft=\"0.393701\" inchTop=\"0.590551\" inchRight=\"0.393701\" inchBottom=\"0.590551\"/>\n"
            " </PAPER>\n"
            " <ATTRIBUTES processing=\"0\" standardpage=\"1\" hasHeader=\"0\" hasFooter=\"0\" unit=\"mm\"/>\n"
            " <FOOTNOTEMGR>\n"
            "  <START value=\"1\"/>\n"
            "  <FORMAT superscript=\"1\" type=\"1\"/>\n"
            "  <FIRSTPARAG ref=\"(null)\"/>\n"
            " </FOOTNOTEMGR>\n"
            " <FRAMESETS>\n");
        if (!m_success)
        {
            newstr.append(
                "  <FRAMESET frameType=\"1\" frameInfo=\"0\" removeable=\"0\" visible=\"1\">\n"
                "   <FRAME left=\"");
            newstr.append(QString::number(s_hMargin));
            newstr.append("\" top=\"");
            newstr.append(QString::number(s_vMargin));
            newstr.append("\" right=\"");
            newstr.append(QString::number(s_width - s_hMargin));
            newstr.append("\" bottom=\"");
            newstr.append(QString::number(s_height - s_vMargin));
            newstr.append("\" runaround=\"1\" runaGapPT=\"2\" runaGapMM=\"1\" runaGapINCH=\"0.0393701\"  lWidth=\"1\" lRed=\"255\" lGreen=\"255\" lBlue=\"255\" lStyle=\"0\"  rWidth=\"1\" rRed=\"255\" rGreen=\"255\" rBlue=\"255\" rStyle=\"0\"  tWidth=\"1\" tRed=\"255\" tGreen=\"255\" tBlue=\"255\" tStyle=\"0\"  bWidth=\"1\" bRed=\"255\" bGreen=\"255\" bBlue=\"255\" bStyle=\"0\" bkRed=\"255\" bkGreen=\"255\" bkBlue=\"255\" bleftpt=\"0\" bleftmm=\"0\" bleftinch=\"0\" brightpt=\"0\" brightmm=\"0\" brightinch=\"0\" btoppt=\"0\" btopmm=\"0\" btopinch=\"0\" bbottompt=\"0\" bbottommm=\"0\" bbottominch=\"0");
            newstr.append("\" autoCreateNewFrame=\"1\" newFrameBehaviour=\"0\"/>\n"
                "   <PARAGRAPH>\n"
                "    <TEXT>This filter is still crappy and it obviously was not able to convert your document.</TEXT>\n"
                "   </PARAGRAPH>\n"
                "  </FRAMESET>\n");
        }
        newstr.append(m_body);
        newstr.append(
            "  </FRAMESETS>\n"
            "</DOC>\n");
        m_part.setContent(newstr);
        m_phase = DONE;
    }
    return m_success;
}

void WinWordDoc::encode(QString &text)
{
  // When encoding the stored form of text to its run-time form,
  // be sure to do the conversion for "&amp;" to "&" first to avoid
  // accidentally converting user text into one of the other escape
  // sequences.

  text.replace(QRegExp("&"), "&amp;");
  text.replace(QRegExp("<"), "&lt;");
  text.replace(QRegExp(">"), "&gt;");
}

void WinWordDoc::gotError(const QString &text)
{
    if (m_phase == TEXT_PASS)
    {
        QString xml_friendly = text;

        encode(xml_friendly);
        m_body.append("<PARAGRAPH>\n<TEXT>");
        m_body.append(xml_friendly);
        m_body.append("</TEXT>\n</PARAGRAPH>\n");
    }
    m_success = false;
}

void WinWordDoc::gotParagraph(const QString &text, PAP &/*style*/)
{
    if (m_phase == TEXT_PASS)
    {
        QString xml_friendly = text;

        encode(xml_friendly);
        m_body.append("<PARAGRAPH>\n<TEXT>");
        m_body.append(xml_friendly);
        m_body.append("</TEXT>\n</PARAGRAPH>\n");
    }
}

void WinWordDoc::gotHeadingParagraph(const QString &text, PAP &style)
{
    if (m_phase == TEXT_PASS)
    {
        QString xml_friendly = text;

        encode(xml_friendly);
        m_body.append("<PARAGRAPH>\n<TEXT>");
        m_body.append(xml_friendly);
        m_body.append("</TEXT>\n"
            " <LAYOUT>\n"
            "  <NAME value=\"Head ");
        m_body.append(QString::number((int)style.istd));
        m_body.append("\"/>\n  <COUNTER type=\"");
        m_body.append(numberingType(style.anld.nfc));
        m_body.append("\" depth=\"");
        m_body.append(QString::number(style.istd - 1));
        m_body.append("\" bullet=\"176\" start=\"1\" numberingtype=\"1\" lefttext=\"\" righttext=\"\" bulletfont=\"times\"/>\n"
            " </LAYOUT>\n"
            "</PARAGRAPH>\n");
    }
}

void WinWordDoc::gotListParagraph(const QString &text, PAP &style)
{
    static const char *listStyle[6] =
    {
        "Enumerated List",
        "Enumerated List",
        "Enumerated List",
        "Alphabetical List",
        "Alphabetical List",
        "Bullet List"
    };

    if (m_phase == TEXT_PASS)
    {
        QString xml_friendly = text;

        encode(xml_friendly);
        m_body.append("<PARAGRAPH>\n<TEXT>");
        m_body.append(xml_friendly);
        m_body.append("</TEXT>\n"
            " <LAYOUT>\n"
            "  <NAME value=\"");
        m_body.append(listStyle[style.anld.nfc]);
        m_body.append("\"/>\n"
            "  <FOLLOWING name=\"");
        m_body.append(listStyle[style.anld.nfc]);
        m_body.append("\"/>\n"
            "  <COUNTER type=\"");
        m_body.append(numberingType(style.anld.nfc));
        m_body.append("\" depth=\"");
        m_body.append(QString::number((int)style.ilvl));
        m_body.append("\" bullet=\"183\" start=\"");
        m_body.append(QString::number((int)style.anld.iStartAt));
        m_body.append("\" numberingtype=\"0\" lefttext=\"\" righttext=\"\" bulletfont=\"symbol\"/>\n"
            " </LAYOUT>\n"
            "</PARAGRAPH>\n");
    }
}

void WinWordDoc::gotTableBegin()
{
    // Create a unique group manager for each new table.

    m_tableManager++;
    m_tableRow = 0;
    if (m_phase == TEXT_PASS)
    {
        m_cellEdges.resize(m_tableManager);
        m_cellEdges.insert(m_tableManager - 1, new QArray<unsigned>);
        m_body.append("<PARAGRAPH>\n<TEXT>");
if (DISABLE_FLOATING)
{
        if (m_tableManager == 1)
            m_body.append("This filter is currently unable to position tables correctly."
                " All the tables are at the end of this document. Other tables can be found by looking for strings like"
                "\"Table 1 goes here\"");
        m_body.append("Table ");
        m_body.append(QString::number(m_tableManager));
        m_body.append(" goes here.</TEXT>\n</PARAGRAPH>\n");
}
else
{
        // This '0' will be replaced with the anchor character.
        m_body.append('0');
        m_body.append("</TEXT>\n<FORMATS>\n<FORMAT id=\"6\" pos=\"0\">\n");
        m_body.append("<ANCHOR type=\"grpMgr\" instance=\"grpmgr_");
        m_body.append(QString::number(m_tableManager));
        m_body.append("\"/>\n</FORMAT>\n</FORMATS>\n</PARAGRAPH>\n");
}
    }
}

void WinWordDoc::gotTableEnd()
{
}

//
// Add to/lookup a cell edge in the cache of cell edges for a given table.
//

int WinWordDoc::cacheCellEdge(
    unsigned tableManager,
    unsigned cellEdge)
{
    QArray<unsigned> *edges = m_cellEdges[tableManager - 1];
    unsigned i;
    unsigned *data;
    unsigned index;
    
    /*
     * Do we already know about this edge?
     */

    data = edges->data();
    index = edges->size();
    for (i = 0; i < index; i++)
    {
        if (data[i] == cellEdge)
            return i;
    }

    /*
     * Add the edge to the (sorted) array.
     */

    edges->resize(index + 1);
    data = edges->data();
    data[index] = cellEdge;
    for (i = index; i > 0; i--)
    {
        unsigned tmp;

        if (data[i - 1] > data[i])
        {
            tmp = data[i - 1];
            data[i - 1] = data[i];
            data[i] = tmp;
        }
        else
        {
            break;
        }
    }
    return i;
}

//
// Compute the Word notion of cell edge ordinate into a Kword one.
//

unsigned WinWordDoc::computeCellEdge(
    TAP &row,
    unsigned edge)
{
    unsigned rowWidth = row.rgdxaCenter[row.itcMac] - row.rgdxaCenter[0];
    unsigned cellEdge;

    // We want to preserve the proportion of row widths in the original document.
    // For now, we do so on the assumption that the table occupies the full width of
    // the page.

    cellEdge = row.rgdxaCenter[edge] - row.rgdxaCenter[0];
    cellEdge = (unsigned)((double)cellEdge * (s_width - s_hMargin - s_hMargin) / rowWidth);
    return cellEdge + s_hMargin;
}

void WinWordDoc::gotTableRow(const QString texts[], const PAP /*styles*/[], TAP &row)
{
    if (m_phase == TEXT_PASS)
    {
        // Add the left and right edge of each cell to our array.

        for (unsigned i = 0; i < row.itcMac; i++)
        {
            cacheCellEdge(m_tableManager, computeCellEdge(row, i));
            cacheCellEdge(m_tableManager, computeCellEdge(row, i + 1));
        }
    }
    if (m_phase == TABLE_PASS)
    {
        QString xml_friendly;

        // Create the XML for each cell in the row.

        for (unsigned i = 0; i < row.itcMac; i++)
        {
            QString cell;
            unsigned left;
            unsigned right;
            unsigned cellEdge;

            cell.append("<FRAMESET frameType=\"1\" frameInfo=\"0\" grpMgr=\"grpmgr_");
            cell.append(QString::number(m_tableManager));
            cell.append("\" row=\"");
            cell.append(QString::number(m_tableRow));
            cell.append("\" col=\"");
            left = cacheCellEdge(m_tableManager, computeCellEdge(row, i));
            cell.append(QString::number(left));
            cell.append("\" rows=\"");
            cell.append(QString::number(1));
            cell.append("\" cols=\"");

            // In cases where not all columns are present, ensure that the last
            // column spans the remainder of the table.

            if ((int)i < row.itcMac - 1)
            {
                right = cacheCellEdge(m_tableManager, computeCellEdge(row, i + 1));
            }
            else
            {
                right = m_cellEdges[m_tableManager - 1]->size() - 1;
            }
            cell.append(QString::number(right - left));
            cell.append("\" removeable=\"0\" visible=\"1\">\n"
                " <FRAME left=\"");
            cellEdge = m_cellEdges[m_tableManager - 1]->at(left);
            cell.append(QString::number(cellEdge));
            cell.append("\" right=\"");
            cellEdge = m_cellEdges[m_tableManager - 1]->at(right);
            cell.append(QString::number(cellEdge));
            cell.append("\" top=\"");
if (DISABLE_FLOATING)
            cell.append(QString::number(400 + m_tableManager * 10 + m_tableRow * 30));
else
            cell.append(QString::number(30 + m_tableRow * 30));
            cell.append("\" bottom=\"");
if (DISABLE_FLOATING)
            cell.append(QString::number(430 + m_tableManager * 10 + m_tableRow * 30));
else
            cell.append(QString::number(60 + m_tableRow * 30));
            cell.append(
                "\" runaround=\"1\" runaGapPT=\"2\" runaGapMM=\"1\" runaGapINCH=\"0.0393701\" "
                "lWidth=\"1\" lStyle=\"0\" " +
                colourType(row.rgtc[i].brcLeft.ico, "lRed", "lGreen", "lBlue") +
                "rWidth=\"1\" rStyle=\"0\" " +
                colourType(row.rgtc[i].brcRight.ico, "rRed", "rGreen", "rBlue") +
                "tWidth=\"1\" tStyle=\"0\" " +
                colourType(row.rgtc[i].brcTop.ico, "tRed", "tGreen", "tBlue") +
                "bWidth=\"1\" bStyle=\"0\" " +
                colourType(row.rgtc[i].brcBottom.ico, "bRed", "bGreen", "bBlue") +
                colourType(row.rgshd[i].icoBack, "bkRed", "bkGreen", "bkBlue", 8) +
                "bleftpt=\"0\" bleftmm=\"0\" bleftinch=\"0\" brightpt=\"0\" brightmm=\"0\" brightinch=\"0\" btoppt=\"0\" btopmm=\"0\" btopinch=\"0\" bbottompt=\"0\" bbottommm=\"0\" bbottominch=\"0");

            cell.append("\" autoCreateNewFrame=\"0\" newFrameBehaviour=\"1\"/>\n");
            cell.append("<PARAGRAPH>\n<TEXT>");
            xml_friendly = texts[i];
            encode(xml_friendly);
            cell.append(xml_friendly);
            cell.append("</TEXT>\n </PARAGRAPH>\n");
            cell.append("</FRAMESET>\n");
            m_body.append(cell);
        }
    }
    m_tableRow++;
}

QString WinWordDoc::colourType(
    unsigned colour,
    const char *red,
    const char *green,
    const char *blue,
    unsigned defaultColour) const
{
    // Word colours are:
    //     0 Auto
    //     1 Black
    //     2 Blue
    //     3 Cyan
    //     4 Green
    //     5 Magenta
    //     6 Red
    //     7 Yellow
    //     8 White
    //     9 DkBlue
    //     10 DkCyan
    //     11 DkGreen
    //     12 DkMagenta
    //     13 DkRed
    //     14 DkYellow
    //     15 DkGray
    //     16 LtGray
    //
    // I've translated these by approximately matching the Word name
    // to an X name as indicated. I'm sure there is a more rational way
    // to do this!

    static unsigned colourTypes[17] =
    {
        0x000000, // 0 returns the default colour.
        0x000000, // 1 black
        0x0000ff, // 2 blue
        0x00ffff, // 3 cyan
        0x00ff00, // 4 green
        0xff00ff, // 5 magenta
        0xff0000, // 6 red
        0xffff00, // 7 yellow
        0xffffff, // 8 white
        0x483d8b, // 9 DarkSlateBlue
        0x008b8b, // 10 cyan4
        0x006400, // 11 DarkGreen
        0x8b008b, // 12 magenta4
        0x8b0000, // 13 red4
        0x8b8b00, // 14 yellow4
        0x3d3d3d, // 15 grey24
        0xd3d3d3  // 16 LightGrey
    };

    QString result;

    if (colour > 16)
        colour = defaultColour;
    if (colour == 0)
        colour = defaultColour;
    result += red;
    result += "=\"";
    result += QString::number((colourTypes[colour] >> 16) & 0xff);
    result += "\" ";
    result += green;
    result += "=\"";
    result += QString::number((colourTypes[colour] >> 8) & 0xff);
    result += "\" ";
    result += blue;
    result += "=\"";
    result += QString::number((colourTypes[colour]) & 0xff);
    result += "\" ";
    return result;
}

char WinWordDoc::numberingType(unsigned nfc) const
{
    // Word number formats are:
    //
    // 0 Arabic numbering
    // 1 Upper case Roman
    // 2 Lower case Roman
    // 3 Upper case Letter
    // 4 Lower case letter
    // 5 Ordinal

    static unsigned numberingTypes[6] =
    {
        '1', '5', '4', '3', '2', '6'
    };

    return numberingTypes[nfc];
}

const QDomDocument * const WinWordDoc::part()
{
    if (m_phase == DONE)
        return &m_part;
    else
        return NULL;
}
