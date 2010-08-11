/* This file is part of the KDE project
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "MsooXmlDrawingTableStyleReader.h"

#include <KoGenStyles.h>
#include <KoGenStyle.h>
#include <KoOdfGraphicStyles.h>

#define MSOOXML_CURRENT_NS "a"
#define MSOOXML_CURRENT_CLASS MsooXmlDrawingTableStyleReader

#include <MsooXmlReader_p.h>
#include <MsooXmlUtils.h>
#include <MsooXmlRelationships.h>
#include <MsooXmlImport.h>

#include <KoXmlWriter.h>

#define MSOOXMLDRAWINGTABLESTYLEREADER_CPP

using namespace MSOOXML;

QPen Border::pen() const
{
    return m_pen;
}

void Border::setPen(const QPen& pen)
{
    m_pen = pen;
}

void TableStyleProperties::addBorder(Border border, BorderSide side)
{
    m_borders.insert(side, border);
}

Border TableStyleProperties::borderForSide(BorderSide side) const
{
    return m_borders.value(side);
}

QString TableStyle::stringFromType(TableStyle::Type type)
{
    switch(type) {
        case FirstRow:
            return "firstRow";
            break;
        case LastCol:
            return "lastCol";
            break;
        case LastRow:
            return "lastRow";
            break;
        case NeCell:
            return "neCell";
            break;
        case NwCell:
            return "nwCell";
            break;
        case SeCell:
            return "seCell";
            break;
        case SwCell:
            return "swCell";
            break;
        case TblBg:
            return "tblBg";
            break;
        case WholeTbl:
            return "wholeTbl";
            break;
    }

    //Shut up the compiler about not returning from a non-void function
    //I know is impossible for it to figure out it will never happen
    Q_ASSERT(false);
    return QString();
}

TableStyle::Type TableStyle::typeFromString(const QString& string)
{
    if(string == "firstRow") {
        return FirstRow;
    }
    else if(string == "lastCol") {
        return LastCol;
    }
    else if(string == "lastRow") {
        return LastRow;
    }
    else if(string == "neCell") {
        return NeCell;
    }
    else if(string == "nwCell") {
        return NwCell;
    }
    else if(string == "seCell") {
        return SeCell;
    }
    else if(string == "swCell") {
        return SwCell;
    }
    else if(string == "tblBg") {
        return TblBg;
    }
    else if(string == "wholeTbl") {
        return WholeTbl;
    }

    //Shut up the compiler about not returning from a non-void function
    //I know is impossible for it to figure out it will never happen
    Q_ASSERT(false);
    return WholeTbl;
}

void TableStyle::setId(const QString& id)
{
    m_id = id;
}


QString TableStyle::id() const
{
    return m_id;
}

TableStyleProperties TableStyle::propertiesForType(Type type) const
{
    if(!m_properties.contains(type)) {
        if(!m_properties.contains(TableStyle::WholeTbl)) {
            //Return at least something
            return TableStyleProperties();
        }

        //Asume WholeTbl
        return m_properties.value(TableStyle::WholeTbl);
    }

    return m_properties.value(type);
}

void TableStyle::addProperties(TableStyleProperties properties, Type type)
{
    m_properties.insert(type, properties);
}

TableStyle TableStyleList::tableStyle(const QString& id) const
{
    return m_styles.value(id);
}

void TableStyleList::insertStyle(QString id, TableStyle style)
{
    m_styles.insert(id, style);
}

MsooXmlDrawingTableStyleReader::MsooXmlDrawingTableStyleReader(KoOdfWriters* writers)
: MsooXmlCommonReader(writers)
{
}

MsooXmlDrawingTableStyleReader::~MsooXmlDrawingTableStyleReader()
{
}

KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read(MsooXmlReaderContext* context)
{
    m_context = dynamic_cast<MsooXmlDrawingTableStyleContext*>(context);
    Q_ASSERT(m_context);

    readNext();
    if (!isStartDocument()) {
        return KoFilter::WrongFormat;
    }

    readNext();
    KoFilter::ConversionStatus result = read_tblStyleLst();
    Q_ASSERT(result == KoFilter::OK);

    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL tblStyleLst
KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read_tblStyleLst()
{
    READ_PROLOGUE

    while(!atEnd()) {
        if(isStartElement()) {
            TRY_READ_IF(tblStyle)
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL tblStyle
KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read_tblStyle()
{
    READ_PROLOGUE

    while(!atEnd()) {
        if(isStartElement()) {
//             TRY_READ_IF(band1H)
//             ELSE_TRY_READ_IF(band1V)
//             ELSE_TRY_READ_IF(band2H)
//             ELSE_TRY_READ_IF(band2V)
//             ELSE_TRY_READ_IF(extLst)
//             ELSE_TRY_READ_IF(firstCol)
//             ELSE_TRY_READ_IF(firstRow)
//             ELSE_TRY_READ_IF(lastCol)
//             ELSE_TRY_READ_IF(lastRow)
//             ELSE_TRY_READ_IF(neCell)
//             ELSE_TRY_READ_IF(swCell)
//             ELSE_TRY_READ_IF(seCell)
//             ELSE_TRY_READ_IF(swCell)
//             ELSE_TRY_READ_IF(tblBg)
            /*ELSE_*/TRY_READ_IF(wholeTbl)
//             ELSE_WRONG_FORMAT
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    QXmlStreamAttributes attrs(attributes());
    READ_ATTR_WITHOUT_NS(styleId)
    m_context->styleList.insertStyle(styleId, m_currentStyle);
    m_currentStyle = TableStyle();

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL wholeTbl
KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read_wholeTbl()
{
    READ_PROLOGUE

    while(!atEnd()) {
        if(isStartElement()) {
            TRY_READ_IF(tcStyle)
            ELSE_TRY_READ_IF(tcTxStyle)
            ELSE_WRONG_FORMAT
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    m_currentStyle.addProperties(m_currentStyleProperties, TableStyle::WholeTbl);
    m_currentStyleProperties = TableStyleProperties();

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL tcStyle
KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read_tcStyle()
{
    READ_PROLOGUE

    while(!atEnd()) {
        if(isStartElement()) {
//             TRY_READ_IF(cell3D)
//             ELSE_TRY_READ_IF(fill)
//             ELSE_TRY_READ_IF(fillRef)
            /*ELSE_*/TRY_READ_IF(tcBrd)
//             ELSE_WRONG_FORMAT
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL tcTxStyle
KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read_tcTxStyle()
{
    READ_PROLOGUE
    SKIP_EVERYTHING_AND_RETURN
}

#undef CURRENT_EL
#define CURRENT_EL tcBrd
KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read_tcBrd()
{
    READ_PROLOGUE

    while(!atEnd()) {
        if(isStartElement()) {
            TRY_READ_IF(bottom)
//             ELSE_TRY_READ_IF(extLst)
            ELSE_TRY_READ_IF(insideH)
            ELSE_TRY_READ_IF(insideV)
            ELSE_TRY_READ_IF(left)
            ELSE_TRY_READ_IF(right)
            ELSE_TRY_READ_IF(tl2br)
            ELSE_TRY_READ_IF(top)
            ELSE_TRY_READ_IF(tr2bl)
//             ELSE_WRONG_FORMAT
            m_currentPen = QPen();
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL bottom
KoFilter::ConversionStatus MSOOXML::MsooXmlDrawingTableStyleReader::read_bottom()
{
    READ_PROLOGUE

    while(!atEnd()) {
        if(isStartElement()) {
            TRY_READ_IF(ln)
//             ELSE_TRY_READ_IF(lnRef)
//             ELSE_WRONG_FORMAT
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    Border border;
    border.setPen(m_currentPen);
    m_currentStyleProperties.addBorder(border, TableStyleProperties::Bottom);

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL top
KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read_top()
{
    READ_PROLOGUE

    while(!atEnd()) {
        if(isStartElement()) {
            TRY_READ_IF(ln)
//             ELSE_TRY_READ_IF(lnRef)
//             ELSE_WRONG_FORMAT
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    Border border;
    border.setPen(m_currentPen);
    m_currentStyleProperties.addBorder(border, TableStyleProperties::Top);

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL left
KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read_left()
{
    READ_PROLOGUE

    while(!atEnd()) {
        if(isStartElement()) {
            TRY_READ_IF(ln)
//             ELSE_TRY_READ_IF(lnRef)
//             ELSE_WRONG_FORMAT
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    Border border;
    border.setPen(m_currentPen);
    m_currentStyleProperties.addBorder(border, TableStyleProperties::Left);

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL right
KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read_right()
{
    READ_PROLOGUE

    while(!atEnd()) {
        if(isStartElement()) {
            TRY_READ_IF(ln)
//             ELSE_TRY_READ_IF(lnRef)
//             ELSE_WRONG_FORMAT
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    Border border;
    border.setPen(m_currentPen);
    m_currentStyleProperties.addBorder(border, TableStyleProperties::Right);

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL insideH
KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read_insideH()
{
    READ_PROLOGUE

    while(!atEnd()) {
        if(isStartElement()) {
            TRY_READ_IF(ln)
//             ELSE_TRY_READ_IF(lnRef)
//             ELSE_WRONG_FORMAT
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    Border border;
    border.setPen(m_currentPen);
    m_currentStyleProperties.addBorder(border, TableStyleProperties::InsideH);

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL insideV
KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read_insideV()
{
    READ_PROLOGUE

    while(!atEnd()) {
        if(isStartElement()) {
            TRY_READ_IF(ln)
//             ELSE_TRY_READ_IF(lnRef)
//             ELSE_WRONG_FORMAT
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    Border border;
    border.setPen(m_currentPen);
    m_currentStyleProperties.addBorder(border, TableStyleProperties::InsideV);

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL tl2br
KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read_tl2br()
{
    READ_PROLOGUE

    while(!atEnd()) {
        if(isStartElement()) {
            TRY_READ_IF(ln)
//             ELSE_TRY_READ_IF(lnRef)
//             ELSE_WRONG_FORMAT
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    Border border;
    border.setPen(m_currentPen);
    m_currentStyleProperties.addBorder(border, TableStyleProperties::TopLeftToBottomRight);

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL tr2bl
KoFilter::ConversionStatus MsooXmlDrawingTableStyleReader::read_tr2bl()
{
    READ_PROLOGUE

    while(!atEnd()) {
        if(isStartElement()) {
            TRY_READ_IF(ln)
//             ELSE_TRY_READ_IF(lnRef)
//             ELSE_WRONG_FORMAT
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    Border border;
    border.setPen(m_currentPen);
    m_currentStyleProperties.addBorder(border, TableStyleProperties::TopRightToBottomLeft);

    READ_EPILOGUE
}

#define blipFill_NS "a"
#define SETUP_PARA_STYLE_IN_READ_P

#include <MsooXmlCommonReaderImpl.h>

#define DRAWINGML_NS "a"
#define DRAWINGML_PIC_NS "p" // DrawingML/Picture

#include <MsooXmlCommonReaderDrawingMLImpl.h>