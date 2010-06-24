/*
 * This file is part of Office 2007 Filters for KOffice
 * Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
 * Copyright (C) 2003 David Faure <faure@kde.org>
 * Copyright (C) 2002, 2003, 2004 Nicolas GOUTTE <goutte@kde.org>
 * Copyright (C) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include "PptxImport.h"
#include "PptxXmlDocumentReader.h"
#include "PptxXmlSlideReader.h"

#include <MsooXmlUtils.h>
#include <MsooXmlSchemas.h>
#include <MsooXmlContentTypes.h>
#include <MsooXmlDocPropertiesReader.h>

#include <QColor>
#include <QFile>
#include <QFont>
#include <QPen>
#include <QRegExp>
#include <QImage>

#include <kde_file.h> // for WARNING
#include <kdeversion.h>
#include <KDebug>
#include <KZip>
#include <KGenericFactory>
#include <KMessageBox>

#include <KoOdfWriteStore.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoDocumentInfo.h>
#include <KoDocument.h>
#include <KoFilterChain.h>
#include <KoUnit.h>
#include <KoPageLayout.h>
#include <KoXmlWriter.h>

#include <memory>

typedef KGenericFactory<PptxImport> PptxImportFactory;
K_EXPORT_COMPONENT_FACTORY(libpptximport, PptxImportFactory("kofficefilters"))

enum PptxDocumentType {
    PptxDocumentPresentation,
    PptxDocumentTemplate,
    PptxDocumentSlideShow
};

class PptxImport::Private
{
public:
    Private() : type(PptxDocumentPresentation), macrosEnabled(false) {
    }

    const char* mainDocumentContentType() const
    {
        if (type == PptxDocumentSlideShow)
            return MSOOXML::ContentTypes::presentationSlideShow;
        if (type == PptxDocumentTemplate)
            return MSOOXML::ContentTypes::presentationTemplate;
        return MSOOXML::ContentTypes::presentationDocument;
    }

    PptxDocumentType type;
    bool macrosEnabled;
};

PptxImport::PptxImport(QObject* parent, const QStringList &)
        : MSOOXML::MsooXmlImport(QLatin1String("presentation"), parent), d(new Private)
{
}

PptxImport::~PptxImport()
{
    delete d;
}

bool PptxImport::acceptsSourceMimeType(const QByteArray& mime) const
{
    kDebug() << "Entering PPTX Import filter: from " << mime;
    if (mime == "application/vnd.openxmlformats-officedocument.presentationml.presentation") {
        d->type = PptxDocumentPresentation;
        d->macrosEnabled = false;
    }
    else if (mime == "application/vnd.openxmlformats-officedocument.presentationml.template") {
        d->type = PptxDocumentTemplate;
        d->macrosEnabled = false;
    }
    else if (mime == "application/vnd.openxmlformats-officedocument.presentationml.slideshow") {
        d->type = PptxDocumentSlideShow;
        d->macrosEnabled = false;
    }
    else if (mime == "application/vnd.ms-powerpoint.presentation.macroEnabled.12") {
        d->type = PptxDocumentPresentation;
        d->macrosEnabled = true;
    }
    else if (mime == "application/vnd.ms-powerpoint.template.macroEnabled.12") {
        d->type = PptxDocumentTemplate;
        d->macrosEnabled = true;
    }
    else if (mime == "application/vnd.ms-powerpoint.slideshow.macroEnabled.12") {
        d->type = PptxDocumentSlideShow;
        d->macrosEnabled = true;
    }
    else
        return false;
    return true;
}

bool PptxImport::acceptsDestinationMimeType(const QByteArray& mime) const
{
    kDebug() << "Entering PPTX Import filter: to " << mime;
    return mime == "application/vnd.oasis.opendocument.presentation";
}

KoFilter::ConversionStatus PptxImport::parseParts(KoOdfWriters *writers,
        MSOOXML::MsooXmlRelationships *relationships, QString& errorMessage)
{
    // more here...
    // 0. Document properties
    {
        MSOOXML::MsooXmlDocPropertiesReader docPropsReader(writers);
        RETURN_IF_ERROR( loadAndParseDocumentIfExists(
            MSOOXML::ContentTypes::coreProps, &docPropsReader, writers, errorMessage) )
    }

    // 1. temporary styles
//! @todo create styles in PptxXmlDocumentReader (PPTX defines styles in presentation.xml)

    writers->mainStyles->insertRawOdfStyles(
        KoGenStyles::DocumentStyles,
        "    <!-- COPIED -->"
        "\n    <draw:marker draw:name=\"Arrow\" svg:viewBox=\"0 0 20 30\" svg:d=\"m10 0-10 30h20z\"/>"
        "\n    <style:default-style style:family=\"graphic\">"
        "\n      <style:paragraph-properties style:text-autospace=\"ideograph-alpha\" style:punctuation-wrap=\"simple\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\" style:font-independent-line-spacing=\"false\">"
        "\n        <style:tab-stops/>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties style:use-window-font-color=\"true\" fo:font-family=\"'Thorndale AMT'\" style:font-family-generic=\"roman\" style:font-pitch=\"variable\" fo:font-size=\"24pt\" fo:language=\"en\" fo:country=\"US\" style:font-family-asian=\"Arial\" style:font-family-generic-asian=\"system\" style:font-pitch-asian=\"variable\" style:font-size-asian=\"24pt\" style:language-asian=\"zxx\" style:country-asian=\"none\" style:font-family-complex=\"Tahoma\" style:font-family-generic-complex=\"system\" style:font-pitch-complex=\"variable\" style:font-size-complex=\"24pt\" style:language-complex=\"zxx\" style:country-complex=\"none\"/>"
        "\n    </style:default-style>"
        "\n    <style:style style:name=\"standard\" style:family=\"graphic\">"
        "\n      <style:graphic-properties draw:stroke=\"solid\" svg:stroke-width=\"0cm\" svg:stroke-color=\"#000000\" draw:marker-start-width=\"0.3cm\" draw:marker-start-center=\"false\" draw:marker-end-width=\"0.3cm\" draw:marker-end-center=\"false\" draw:fill=\"solid\" draw:fill-color=\"#99ccff\" fo:padding-top=\"0.125cm\" fo:padding-bottom=\"0.125cm\" fo:padding-left=\"0.25cm\" fo:padding-right=\"0.25cm\" draw:shadow=\"hidden\" draw:shadow-offset-x=\"0.3cm\" draw:shadow-offset-y=\"0.3cm\" draw:shadow-color=\"#808080\">"
        "\n        <text:list-style style:name=\"standard\">"
        "\n          <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"•\">"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"2\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"1.27cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"3\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"2.54cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"4\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"3.81cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"5\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"6\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"7\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"8\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"9\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"10\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n        </text:list-style>"
        "\n      </style:graphic-properties>"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"start\" text:enable-numbering=\"false\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\" style:font-independent-line-spacing=\"true\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"0cm\"/>"
        "\n          <style:tab-stop style:position=\"2.54cm\"/>"
        "\n          <style:tab-stop style:position=\"5.08cm\"/>"
        "\n          <style:tab-stop style:position=\"7.62cm\"/>"
        "\n          <style:tab-stop style:position=\"10.16cm\"/>"
        "\n          <style:tab-stop style:position=\"12.7cm\"/>"
        "\n          <style:tab-stop style:position=\"15.24cm\"/>"
        "\n          <style:tab-stop style:position=\"17.78cm\"/>"
        "\n          <style:tab-stop style:position=\"20.32cm\"/>"
        "\n          <style:tab-stop style:position=\"22.86cm\"/>"
        "\n          <style:tab-stop style:position=\"25.4cm\"/>"
        "\n          <style:tab-stop style:position=\"27.94cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-outline=\"false\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"18pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-family-asian=\"Arial\" style:font-family-generic-asian=\"system\" style:font-pitch-asian=\"variable\" style:font-size-asian=\"18pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-family-complex=\"Tahoma\" style:font-family-generic-complex=\"system\" style:font-pitch-complex=\"variable\" style:font-size-complex=\"18pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:text-emphasize=\"none\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"objectwitharrow\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:stroke=\"solid\" svg:stroke-width=\"0.15cm\" svg:stroke-color=\"#000000\" draw:marker-start=\"Arrow\" draw:marker-start-width=\"0.7cm\" draw:marker-start-center=\"true\" draw:marker-end-width=\"0.3cm\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"objectwithshadow\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:shadow=\"visible\" draw:shadow-offset-x=\"0.3cm\" draw:shadow-offset-y=\"0.3cm\" draw:shadow-color=\"#808080\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"objectwithoutfill\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:fill=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"text\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"textbody\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\"/>"
        "\n      <style:text-properties fo:font-size=\"16pt\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"textbodyjustfied\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\"/>"
        "\n      <style:paragraph-properties fo:text-align=\"justify\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"textbodyindent\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\">"
        "\n        <text:list-style style:name=\"textbodyindent\">"
        "\n          <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"0.6cm\" text:min-label-width=\"-0.6cm\" text:min-label-distance=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"2\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"0.6cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"3\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"1.2cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"4\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"1.8cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"5\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"2.4cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"6\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"3cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"7\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"3.6cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"8\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"4.2cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"9\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"4.8cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"10\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"5.4cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n        </text:list-style>"
        "\n      </style:graphic-properties>"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" fo:text-indent=\"0.6cm\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"title\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\"/>"
        "\n      <style:text-properties fo:font-size=\"44pt\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"title1\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"solid\" draw:fill-color=\"#008080\" draw:shadow=\"visible\" draw:shadow-offset-x=\"0.2cm\" draw:shadow-offset-y=\"0.2cm\" draw:shadow-color=\"#808080\"/>"
        "\n      <style:paragraph-properties fo:text-align=\"center\"/>"
        "\n      <style:text-properties fo:font-size=\"24pt\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"title2\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties svg:stroke-width=\"0.05cm\" draw:fill-color=\"#ffcc99\" draw:shadow=\"visible\" draw:shadow-offset-x=\"0.2cm\" draw:shadow-offset-y=\"0.2cm\" draw:shadow-color=\"#808080\">"
        "\n        <text:list-style style:name=\"title2\">"
        "\n          <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"●\">"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"2\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"0.6cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"3\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"1.2cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"4\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"1.8cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"5\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"2.4cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"6\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"3cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"7\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"3.6cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"8\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"4.2cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"9\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"4.8cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"10\" text:bullet-char=\"●\">"
        "\n            <style:list-level-properties text:space-before=\"5.4cm\" text:min-label-width=\"0.6cm\"/>"
        "\n            <style:text-properties fo:font-family=\"StarSymbol\" style:use-window-font-color=\"true\" fo:font-size=\"45%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n        </text:list-style>"
        "\n      </style:graphic-properties>"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0.2cm\" fo:margin-top=\"0.1cm\" fo:margin-bottom=\"0.1cm\" fo:text-align=\"center\" fo:text-indent=\"0cm\"/>"
        "\n      <style:text-properties fo:font-size=\"36pt\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"headline\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\"/>"
        "\n      <style:paragraph-properties fo:margin-top=\"0.42cm\" fo:margin-bottom=\"0.21cm\"/>"
        "\n      <style:text-properties fo:font-size=\"24pt\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"headline1\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\"/>"
        "\n      <style:paragraph-properties fo:margin-top=\"0.42cm\" fo:margin-bottom=\"0.21cm\"/>"
        "\n      <style:text-properties fo:font-size=\"18pt\" fo:font-weight=\"bold\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"headline2\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\"/>"
        "\n      <style:paragraph-properties fo:margin-top=\"0.42cm\" fo:margin-bottom=\"0.21cm\"/>"
        "\n      <style:text-properties fo:font-size=\"0pt\" fo:font-style=\"italic\" fo:font-weight=\"bold\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"measure\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:stroke=\"solid\" draw:marker-start=\"Arrow\" draw:marker-start-width=\"0.2cm\" draw:marker-end=\"Arrow\" draw:marker-end-width=\"0.2cm\" draw:fill=\"none\" draw:show-unit=\"true\"/>"
        "\n      <style:text-properties fo:font-size=\"12pt\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-title\" style:family=\"presentation\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\" draw:textarea-vertical-align=\"middle\">"
        "\n        <text:list-style style:name=\"Default-title\">"
        "\n          <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"•\">"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n        </text:list-style>"
        "\n      </style:graphic-properties>"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"center\" text:enable-numbering=\"false\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"0cm\"/>"
        "\n          <style:tab-stop style:position=\"2.54cm\"/>"
        "\n          <style:tab-stop style:position=\"5.08cm\"/>"
        "\n          <style:tab-stop style:position=\"7.62cm\"/>"
        "\n          <style:tab-stop style:position=\"10.16cm\"/>"
        "\n          <style:tab-stop style:position=\"12.7cm\"/>"
        "\n          <style:tab-stop style:position=\"15.24cm\"/>"
        "\n          <style:tab-stop style:position=\"17.78cm\"/>"
        "\n          <style:tab-stop style:position=\"20.32cm\"/>"
        "\n          <style:tab-stop style:position=\"22.86cm\"/>"
        "\n          <style:tab-stop style:position=\"25.4cm\"/>"
        "\n          <style:tab-stop style:position=\"27.94cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-outline=\"false\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"44pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-family-asian=\"Arial\" style:font-family-generic-asian=\"system\" style:font-pitch-asian=\"variable\" style:font-size-asian=\"44pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-family-complex=\"Tahoma\" style:font-family-generic-complex=\"system\" style:font-pitch-complex=\"variable\" style:font-size-complex=\"44pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:text-emphasize=\"none\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-subtitle\" style:family=\"presentation\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\" draw:textarea-vertical-align=\"middle\">"
        "\n        <text:list-style style:name=\"Default-subtitle\">"
        "\n          <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"•\">"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"2\" text:bullet-char=\"–\">"
        "\n            <style:list-level-properties text:space-before=\"1.27cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"3\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"2.54cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"4\" text:bullet-char=\"–\">"
        "\n            <style:list-level-properties text:space-before=\"3.81cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"5\" text:bullet-char=\"»\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"6\" text:bullet-char=\"»\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"7\" text:bullet-char=\"»\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"8\" text:bullet-char=\"»\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"9\" text:bullet-char=\"»\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n        </text:list-style>"
        "\n      </style:graphic-properties>"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0.282cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"center\" text:enable-numbering=\"false\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"0cm\"/>"
        "\n          <style:tab-stop style:position=\"2.54cm\"/>"
        "\n          <style:tab-stop style:position=\"5.08cm\"/>"
        "\n          <style:tab-stop style:position=\"7.62cm\"/>"
        "\n          <style:tab-stop style:position=\"10.16cm\"/>"
        "\n          <style:tab-stop style:position=\"12.7cm\"/>"
        "\n          <style:tab-stop style:position=\"15.24cm\"/>"
        "\n          <style:tab-stop style:position=\"17.78cm\"/>"
        "\n          <style:tab-stop style:position=\"20.32cm\"/>"
        "\n          <style:tab-stop style:position=\"22.86cm\"/>"
        "\n          <style:tab-stop style:position=\"25.4cm\"/>"
        "\n          <style:tab-stop style:position=\"27.94cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-outline=\"false\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"32pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-family-asian=\"Arial\" style:font-family-generic-asian=\"system\" style:font-pitch-asian=\"variable\" style:font-size-asian=\"32pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-family-complex=\"Tahoma\" style:font-family-generic-complex=\"system\" style:font-pitch-complex=\"variable\" style:font-size-complex=\"32pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:text-emphasize=\"none\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-background\" style:family=\"presentation\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"solid\" draw:fill-color=\"#ffffff\" draw:textarea-horizontal-align=\"center\" draw:textarea-vertical-align=\"middle\" draw:shadow=\"hidden\"/>"
        "\n      <style:paragraph-properties fo:text-align=\"center\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-backgroundobjects\" style:family=\"presentation\">"
        "\n      <style:graphic-properties draw:shadow=\"hidden\" draw:shadow-offset-x=\"0.3cm\" draw:shadow-offset-y=\"0.3cm\" draw:shadow-color=\"#808080\">"
        "\n        <text:list-style style:name=\"Default-backgroundobjects\">"
        "\n          <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"•\">"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"2\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"1.27cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"3\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"2.54cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"4\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"3.81cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"5\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"6\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"7\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"8\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"9\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"10\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n        </text:list-style>"
        "\n      </style:graphic-properties>"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"start\" text:enable-numbering=\"false\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"0cm\"/>"
        "\n          <style:tab-stop style:position=\"2.54cm\"/>"
        "\n          <style:tab-stop style:position=\"5.08cm\"/>"
        "\n          <style:tab-stop style:position=\"7.62cm\"/>"
        "\n          <style:tab-stop style:position=\"10.16cm\"/>"
        "\n          <style:tab-stop style:position=\"12.7cm\"/>"
        "\n          <style:tab-stop style:position=\"15.24cm\"/>"
        "\n          <style:tab-stop style:position=\"17.78cm\"/>"
        "\n          <style:tab-stop style:position=\"20.32cm\"/>"
        "\n          <style:tab-stop style:position=\"22.86cm\"/>"
        "\n          <style:tab-stop style:position=\"25.4cm\"/>"
        "\n          <style:tab-stop style:position=\"27.94cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"18pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-size-asian=\"18pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-size-complex=\"18pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-notes\" style:family=\"presentation\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\">"
        "\n        <text:list-style style:name=\"Default-notes\">"
        "\n          <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"•\">"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"2\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"1.27cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"3\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"2.54cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"4\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"3.81cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"5\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"6\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"7\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"8\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"9\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"10\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n        </text:list-style>"
        "\n      </style:graphic-properties>"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0.158cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"start\" text:enable-numbering=\"false\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"0cm\"/>"
        "\n          <style:tab-stop style:position=\"2.54cm\"/>"
        "\n          <style:tab-stop style:position=\"5.08cm\"/>"
        "\n          <style:tab-stop style:position=\"7.62cm\"/>"
        "\n          <style:tab-stop style:position=\"10.16cm\"/>"
        "\n          <style:tab-stop style:position=\"12.7cm\"/>"
        "\n          <style:tab-stop style:position=\"15.24cm\"/>"
        "\n          <style:tab-stop style:position=\"17.78cm\"/>"
        "\n          <style:tab-stop style:position=\"20.32cm\"/>"
        "\n          <style:tab-stop style:position=\"22.86cm\"/>"
        "\n          <style:tab-stop style:position=\"25.4cm\"/>"
        "\n          <style:tab-stop style:position=\"27.94cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-outline=\"false\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"12pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-family-asian=\"Arial\" style:font-family-generic-asian=\"system\" style:font-pitch-asian=\"variable\" style:font-size-asian=\"12pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-family-complex=\"Tahoma\" style:font-family-generic-complex=\"system\" style:font-pitch-complex=\"variable\" style:font-size-complex=\"12pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:text-emphasize=\"none\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-outline1\" style:family=\"presentation\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\">"
        "\n        <text:list-style style:name=\"Default-outline1\">"
        "\n          <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:min-label-width=\"0.952cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"2\" text:bullet-char=\"–\">"
        "\n            <style:list-level-properties text:space-before=\"1.27cm\" text:min-label-width=\"0.793cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"3\" text:bullet-char=\"•\">"
        "\n            <style:list-level-properties text:space-before=\"2.54cm\" text:min-label-width=\"0.635cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"4\" text:bullet-char=\"–\">"
        "\n            <style:list-level-properties text:space-before=\"3.81cm\" text:min-label-width=\"0.635cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"5\" text:bullet-char=\"»\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"6\" text:bullet-char=\"»\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"7\" text:bullet-char=\"»\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n          <text:list-level-style-bullet text:level=\"8\" text:bullet-char=\"»\">"
        "\n            <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n            <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n          </text:list-level-style-bullet>"
        "\n        </text:list-style>"
        "\n      </style:graphic-properties>"
        "\n      <style:paragraph-properties fo:margin-left=\"0.952cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0.282cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"start\" text:enable-numbering=\"true\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"1.587cm\"/>"
        "\n          <style:tab-stop style:position=\"4.127cm\"/>"
        "\n          <style:tab-stop style:position=\"6.667cm\"/>"
        "\n          <style:tab-stop style:position=\"9.207cm\"/>"
        "\n          <style:tab-stop style:position=\"11.747cm\"/>"
        "\n          <style:tab-stop style:position=\"14.287cm\"/>"
        "\n          <style:tab-stop style:position=\"16.827cm\"/>"
        "\n          <style:tab-stop style:position=\"19.367cm\"/>"
        "\n          <style:tab-stop style:position=\"21.907cm\"/>"
        "\n          <style:tab-stop style:position=\"24.447cm\"/>"
        "\n          <style:tab-stop style:position=\"26.987cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-outline=\"false\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"32pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-family-asian=\"Arial\" style:font-family-generic-asian=\"system\" style:font-pitch-asian=\"variable\" style:font-size-asian=\"32pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-family-complex=\"Tahoma\" style:font-family-generic-complex=\"system\" style:font-pitch-complex=\"variable\" style:font-size-complex=\"32pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:text-emphasize=\"none\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-outline2\" style:family=\"presentation\" style:parent-style-name=\"Default-outline1\">"
        "\n      <style:paragraph-properties fo:margin-left=\"2.063cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0.246cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"start\" text:enable-numbering=\"true\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"0.476cm\"/>"
        "\n          <style:tab-stop style:position=\"3.016cm\"/>"
        "\n          <style:tab-stop style:position=\"5.556cm\"/>"
        "\n          <style:tab-stop style:position=\"8.096cm\"/>"
        "\n          <style:tab-stop style:position=\"10.636cm\"/>"
        "\n          <style:tab-stop style:position=\"13.176cm\"/>"
        "\n          <style:tab-stop style:position=\"15.716cm\"/>"
        "\n          <style:tab-stop style:position=\"18.256cm\"/>"
        "\n          <style:tab-stop style:position=\"20.796cm\"/>"
        "\n          <style:tab-stop style:position=\"23.336cm\"/>"
        "\n          <style:tab-stop style:position=\"25.876cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"28pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-size-asian=\"28pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-size-complex=\"28pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-outline3\" style:family=\"presentation\" style:parent-style-name=\"Default-outline2\">"
        "\n      <style:paragraph-properties fo:margin-left=\"3.175cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0.211cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"start\" text:enable-numbering=\"true\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"1.905cm\"/>"
        "\n          <style:tab-stop style:position=\"4.445cm\"/>"
        "\n          <style:tab-stop style:position=\"6.985cm\"/>"
        "\n          <style:tab-stop style:position=\"9.525cm\"/>"
        "\n          <style:tab-stop style:position=\"12.065cm\"/>"
        "\n          <style:tab-stop style:position=\"14.605cm\"/>"
        "\n          <style:tab-stop style:position=\"17.145cm\"/>"
        "\n          <style:tab-stop style:position=\"19.685cm\"/>"
        "\n          <style:tab-stop style:position=\"22.225cm\"/>"
        "\n          <style:tab-stop style:position=\"24.765cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"24pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-size-asian=\"24pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-size-complex=\"24pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-outline4\" style:family=\"presentation\" style:parent-style-name=\"Default-outline3\">"
        "\n      <style:paragraph-properties fo:margin-left=\"4.445cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0.176cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"start\" text:enable-numbering=\"true\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"0.635cm\"/>"
        "\n          <style:tab-stop style:position=\"3.175cm\"/>"
        "\n          <style:tab-stop style:position=\"5.715cm\"/>"
        "\n          <style:tab-stop style:position=\"8.255cm\"/>"
        "\n          <style:tab-stop style:position=\"10.795cm\"/>"
        "\n          <style:tab-stop style:position=\"13.335cm\"/>"
        "\n          <style:tab-stop style:position=\"15.875cm\"/>"
        "\n          <style:tab-stop style:position=\"18.415cm\"/>"
        "\n          <style:tab-stop style:position=\"20.955cm\"/>"
        "\n          <style:tab-stop style:position=\"23.495cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"20pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-size-asian=\"20pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-size-complex=\"20pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-outline5\" style:family=\"presentation\" style:parent-style-name=\"Default-outline4\">"
        "\n      <style:paragraph-properties fo:margin-left=\"5.715cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0.176cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"start\" text:enable-numbering=\"true\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"1.905cm\"/>"
        "\n          <style:tab-stop style:position=\"4.445cm\"/>"
        "\n          <style:tab-stop style:position=\"6.985cm\"/>"
        "\n          <style:tab-stop style:position=\"9.525cm\"/>"
        "\n          <style:tab-stop style:position=\"12.065cm\"/>"
        "\n          <style:tab-stop style:position=\"14.605cm\"/>"
        "\n          <style:tab-stop style:position=\"17.145cm\"/>"
        "\n          <style:tab-stop style:position=\"19.685cm\"/>"
        "\n          <style:tab-stop style:position=\"22.225cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"20pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-size-asian=\"20pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-size-complex=\"20pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-outline6\" style:family=\"presentation\" style:parent-style-name=\"Default-outline5\">"
        "\n      <style:paragraph-properties fo:margin-left=\"5.715cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0.176cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"start\" text:enable-numbering=\"true\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"1.905cm\"/>"
        "\n          <style:tab-stop style:position=\"4.445cm\"/>"
        "\n          <style:tab-stop style:position=\"6.985cm\"/>"
        "\n          <style:tab-stop style:position=\"9.525cm\"/>"
        "\n          <style:tab-stop style:position=\"12.065cm\"/>"
        "\n          <style:tab-stop style:position=\"14.605cm\"/>"
        "\n          <style:tab-stop style:position=\"17.145cm\"/>"
        "\n          <style:tab-stop style:position=\"19.685cm\"/>"
        "\n          <style:tab-stop style:position=\"22.225cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"20pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-size-asian=\"20pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-size-complex=\"20pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-outline7\" style:family=\"presentation\" style:parent-style-name=\"Default-outline6\">"
        "\n      <style:paragraph-properties fo:margin-left=\"5.715cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0.176cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"start\" text:enable-numbering=\"true\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"1.905cm\"/>"
        "\n          <style:tab-stop style:position=\"4.445cm\"/>"
        "\n          <style:tab-stop style:position=\"6.985cm\"/>"
        "\n          <style:tab-stop style:position=\"9.525cm\"/>"
        "\n          <style:tab-stop style:position=\"12.065cm\"/>"
        "\n          <style:tab-stop style:position=\"14.605cm\"/>"
        "\n          <style:tab-stop style:position=\"17.145cm\"/>"
        "\n          <style:tab-stop style:position=\"19.685cm\"/>"
        "\n          <style:tab-stop style:position=\"22.225cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"20pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-size-asian=\"20pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-size-complex=\"20pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-outline8\" style:family=\"presentation\" style:parent-style-name=\"Default-outline7\">"
        "\n      <style:paragraph-properties fo:margin-left=\"5.715cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0.176cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"start\" text:enable-numbering=\"true\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"1.905cm\"/>"
        "\n          <style:tab-stop style:position=\"4.445cm\"/>"
        "\n          <style:tab-stop style:position=\"6.985cm\"/>"
        "\n          <style:tab-stop style:position=\"9.525cm\"/>"
        "\n          <style:tab-stop style:position=\"12.065cm\"/>"
        "\n          <style:tab-stop style:position=\"14.605cm\"/>"
        "\n          <style:tab-stop style:position=\"17.145cm\"/>"
        "\n          <style:tab-stop style:position=\"19.685cm\"/>"
        "\n          <style:tab-stop style:position=\"22.225cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"20pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-size-asian=\"20pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-size-complex=\"20pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Default-outline9\" style:family=\"presentation\" style:parent-style-name=\"Default-outline8\">"
        "\n      <style:paragraph-properties fo:margin-left=\"5.715cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0.176cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"100%\" fo:text-align=\"start\" text:enable-numbering=\"true\" fo:text-indent=\"0cm\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\">"
        "\n        <style:tab-stops>"
        "\n          <style:tab-stop style:position=\"1.905cm\"/>"
        "\n          <style:tab-stop style:position=\"4.445cm\"/>"
        "\n          <style:tab-stop style:position=\"6.985cm\"/>"
        "\n          <style:tab-stop style:position=\"9.525cm\"/>"
        "\n          <style:tab-stop style:position=\"12.065cm\"/>"
        "\n          <style:tab-stop style:position=\"14.605cm\"/>"
        "\n          <style:tab-stop style:position=\"17.145cm\"/>"
        "\n          <style:tab-stop style:position=\"19.685cm\"/>"
        "\n          <style:tab-stop style:position=\"22.225cm\"/>"
        "\n        </style:tab-stops>"
        "\n      </style:paragraph-properties>"
        "\n      <style:text-properties fo:color=\"#000000\" style:text-line-through-style=\"none\" style:text-position=\"0% 100%\" fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:font-size=\"20pt\" fo:font-style=\"normal\" fo:text-shadow=\"none\" style:text-underline-style=\"none\" fo:font-weight=\"normal\" style:font-size-asian=\"20pt\" style:font-style-asian=\"normal\" style:font-weight-asian=\"normal\" style:font-size-complex=\"20pt\" style:font-style-complex=\"normal\" style:font-weight-complex=\"normal\" style:font-relief=\"none\"/>"
        "\n    </style:style>"
/* 2010-06-04 no longer hardcoded
        "\n    <style:presentation-page-layout style:name=\"AL0T26\">"
        "\n      <presentation:placeholder presentation:object=\"handout\" svg:x=\"2.057cm\" svg:y=\"1.743cm\" svg:width=\"6.103cm\" svg:height=\"-0.233cm\"/>"
        "\n      <presentation:placeholder presentation:object=\"handout\" svg:x=\"10.96cm\" svg:y=\"1.743cm\" svg:width=\"6.103cm\" svg:height=\"-0.233cm\"/>"
        "\n      <presentation:placeholder presentation:object=\"handout\" svg:x=\"19.863cm\" svg:y=\"1.743cm\" svg:width=\"6.103cm\" svg:height=\"-0.233cm\"/>"
        "\n      <presentation:placeholder presentation:object=\"handout\" svg:x=\"2.057cm\" svg:y=\"3.612cm\" svg:width=\"6.103cm\" svg:height=\"-0.233cm\"/>"
        "\n      <presentation:placeholder presentation:object=\"handout\" svg:x=\"10.96cm\" svg:y=\"3.612cm\" svg:width=\"6.103cm\" svg:height=\"-0.233cm\"/>"
        "\n      <presentation:placeholder presentation:object=\"handout\" svg:x=\"19.863cm\" svg:y=\"3.612cm\" svg:width=\"6.103cm\" svg:height=\"-0.233cm\"/>"
        "\n      <presentation:placeholder presentation:object=\"handout\" svg:x=\"2.057cm\" svg:y=\"5.481cm\" svg:width=\"6.103cm\" svg:height=\"-0.233cm\"/>"
        "\n      <presentation:placeholder presentation:object=\"handout\" svg:x=\"10.96cm\" svg:y=\"5.481cm\" svg:width=\"6.103cm\" svg:height=\"-0.233cm\"/>"
        "\n      <presentation:placeholder presentation:object=\"handout\" svg:x=\"19.863cm\" svg:y=\"5.481cm\" svg:width=\"6.103cm\" svg:height=\"-0.233cm\"/>"
        "\n    </style:presentation-page-layout>"
*/
        "\n    <!-- /COPIED -->"
    );

    writers->mainStyles->insertRawOdfStyles(
        KoGenStyles::MasterStyles,
        "    <!-- COPIED -->"
        "\n    <draw:layer-set>"
        "\n      <draw:layer draw:name=\"layout\"/>"
        "\n      <draw:layer draw:name=\"background\"/>"
        "\n      <draw:layer draw:name=\"backgroundobjects\"/>"
        "\n      <draw:layer draw:name=\"controls\"/>"
        "\n      <draw:layer draw:name=\"measurelines\"/>"
        "\n    </draw:layer-set>"
/*
        "\n    <style:master-page style:name=\"Default\" style:page-layout-name=\"PMpredef1\" draw:style-name=\"dppredef1\">"
        "\n      <draw:frame presentation:style-name=\"Default-title\" draw:layer=\"backgroundobjects\" svg:width=\"22.86cm\" svg:height=\"3.176cm\" svg:x=\"1.27cm\" svg:y=\"0.762cm\" presentation:class=\"title\" presentation:placeholder=\"true\">"
        "\n        <draw:text-box/>"
        "\n      </draw:frame>"
        "\n      <draw:frame presentation:style-name=\"Default-outline1\" draw:layer=\"backgroundobjects\" svg:width=\"22.86cm\" svg:height=\"12.573cm\" svg:x=\"1.27cm\" svg:y=\"4.445cm\" presentation:class=\"outline\" presentation:placeholder=\"true\">"
        "\n        <draw:text-box/>"
        "\n      </draw:frame>"
        "\n      <draw:frame presentation:style-name=\"pr1\" draw:text-style-name=\"Ppredef4\" draw:layer=\"backgroundobjects\" svg:width=\"5.927cm\" svg:height=\"1.015cm\" svg:x=\"1.269cm\" svg:y=\"17.657cm\" presentation:class=\"date-time\">"
        "\n        <draw:text-box>"
        "\n          <text:p text:style-name=\"Ppredef3\">"
        "\n            <text:span text:style-name=\"Tpredef1\">"
        "\n              <presentation:date-time/>"
        "\n            </text:span>"
        "\n          </text:p>"
        "\n        </draw:text-box>"
        "\n      </draw:frame>"
        "\n      <draw:frame presentation:style-name=\"pr2\" draw:text-style-name=\"Ppredef5\" draw:layer=\"backgroundobjects\" svg:width=\"8.044cm\" svg:height=\"1.277cm\" svg:x=\"8.678cm\" svg:y=\"17.526cm\" presentation:class=\"footer\">"
        "\n        <draw:text-box>"
        "\n          <text:p/>"
        "\n        </draw:text-box>"
        "\n      </draw:frame>"
        "\n      <draw:frame presentation:style-name=\"prpredef3\" draw:text-style-name=\"Ppredef7\" draw:layer=\"backgroundobjects\" svg:width=\"5.927cm\" svg:height=\"1.015cm\" svg:x=\"18.202cm\" svg:y=\"17.657cm\" presentation:class=\"page-number\">"
        "\n        <draw:text-box>"
        "\n          <text:p text:style-name=\"Ppredef6\">"
        "\n            <text:span text:style-name=\"Tpredef1\">"
        "\n              <text:page-number>1</text:page-number>"
        "\n            </text:span>"
        "\n          </text:p>"
        "\n        </draw:text-box>"
        "\n      </draw:frame>"
        "\n    </style:master-page>"*/
        "\n    <!-- /COPIED -->"
    );

    writers->mainStyles->insertRawOdfStyles(
        KoGenStyles::DocumentAutomaticStyles,
        "    <!-- COPIED -->"
/* 2010-06-04 no longer hardcoded
        "\n    <style:style style:name=\"pr1\" style:family=\"presentation\" style:parent-style-name=\"Default-title\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\" draw:fill-color=\"#ffffff\" draw:textarea-horizontal-align=\"justify\" draw:textarea-vertical-align=\"middle\" draw:auto-grow-height=\"true\" draw:auto-grow-width=\"false\" fo:min-height=\"2.922cm\" fo:min-width=\"0cm\" fo:padding-top=\"0.127cm\" fo:padding-bottom=\"0.127cm\" fo:padding-left=\"0.254cm\" fo:padding-right=\"0.254cm\" fo:wrap-option=\"no-wrap\" draw:shadow=\"hidden\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"pr2\" style:family=\"presentation\" style:parent-style-name=\"Default-outline1\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\" draw:fill-color=\"#ffffff\" draw:textarea-horizontal-align=\"justify\" draw:textarea-vertical-align=\"top\" draw:auto-grow-height=\"true\" draw:auto-grow-width=\"false\" fo:min-height=\"12.319cm\" fo:min-width=\"0cm\" fo:padding-top=\"0.127cm\" fo:padding-bottom=\"0.127cm\" fo:padding-left=\"0.254cm\" fo:padding-right=\"0.254cm\" fo:wrap-option=\"no-wrap\" draw:shadow=\"hidden\"/>"
        "\n    </style:style>"
        "\n    <!-- (used for notes) style:style style:name=\"prpredef3\" style:family=\"presentation\" style:parent-style-name=\"Default-notes\">"
        "\n      <style:graphic-properties draw:fill-color=\"#ffffff\" fo:min-height=\"11.43cm\"/>"
        "\n    </style:style -->"
        "\n    <style:style style:name=\"Ppredef1\" style:family=\"paragraph\">"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" text:enable-numbering=\"false\" fo:text-indent=\"0cm\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Ppredef2\" style:family=\"paragraph\">"
        "\n      <style:paragraph-properties style:writing-mode=\"lr-tb\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Ppredef3\" style:family=\"paragraph\">"
        "\n      <style:paragraph-properties fo:margin-left=\"0.952cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0.282cm\" fo:margin-bottom=\"0cm\" text:enable-numbering=\"true\" fo:text-indent=\"-0.952cm\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Ppredef4\" style:family=\"paragraph\">"
        "\n      <style:paragraph-properties fo:margin-left=\"2.063cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0.246cm\" fo:margin-bottom=\"0cm\" text:enable-numbering=\"true\" fo:text-indent=\"-0.793cm\"/>"
        "\n    </style:style>"
*/
        "\n    <text:list-style style:name=\"Lpredef1\">"
        "\n      <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"•\">"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n    </text:list-style>"
        "\n    <text:list-style style:name=\"Lpredef2\">"
        "\n      <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:min-label-width=\"0.952cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"2\" text:bullet-char=\"–\">"
        "\n        <style:list-level-properties text:space-before=\"1.27cm\" text:min-label-width=\"0.793cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"3\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"2.54cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"4\" text:bullet-char=\"–\">"
        "\n        <style:list-level-properties text:space-before=\"3.81cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"5\" text:bullet-char=\"»\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"6\" text:bullet-char=\"»\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"7\" text:bullet-char=\"»\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"8\" text:bullet-char=\"»\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n    </text:list-style>"
        "\n    <text:list-style style:name=\"Lpredef3\">"
        "\n      <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:min-label-width=\"0.952cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#b7dee8\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"2\" text:bullet-char=\"–\">"
        "\n        <style:list-level-properties text:space-before=\"1.27cm\" text:min-label-width=\"0.793cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"3\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"2.54cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"4\" text:bullet-char=\"–\">"
        "\n        <style:list-level-properties text:space-before=\"3.81cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"5\" text:bullet-char=\"»\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"6\" text:bullet-char=\"»\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"7\" text:bullet-char=\"»\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"8\" text:bullet-char=\"»\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n    </text:list-style>"
        "\n    <text:list-style style:name=\"Lpredef4\">"
        "\n      <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:min-label-width=\"0.952cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#d99694\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"2\" text:bullet-char=\"–\">"
        "\n        <style:list-level-properties text:space-before=\"1.27cm\" text:min-label-width=\"0.793cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"3\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"2.54cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"4\" text:bullet-char=\"–\">"
        "\n        <style:list-level-properties text:space-before=\"3.81cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"5\" text:bullet-char=\"»\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"6\" text:bullet-char=\"»\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"7\" text:bullet-char=\"»\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"8\" text:bullet-char=\"»\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\" text:min-label-width=\"0.635cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Arial\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n    </text:list-style>"
        "\n    <!-- /COPIED -->"
    );

    writers->mainStyles->insertRawOdfStyles(
        KoGenStyles::StylesXmlAutomaticStyles,
        "    <!-- COPIED -->"
/*        
        "\n    <style:page-layout style:name=\"PMpredef1\">"
        "\n      <style:page-layout-properties fo:margin-top=\"0cm\" fo:margin-bottom=\"0cm\" fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" fo:page-width=\"25.4cm\" fo:page-height=\"19.05cm\" style:print-orientation=\"landscape\"/>"
        "\n    </style:page-layout>"
        "\n    <style:style style:name=\"dppredef1\" style:family=\"drawing-page\">"
        "\n      <style:drawing-page-properties draw:background-size=\"border\" draw:fill=\"solid\" draw:fill-color=\"#ffffff\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"dppredef2\" style:family=\"drawing-page\">"
        "\n      <style:drawing-page-properties presentation:display-header=\"true\" presentation:display-footer=\"true\" presentation:display-page-number=\"false\" presentation:display-date-time=\"true\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"grpredef1\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\" draw:fill-color=\"#ffffff\" draw:auto-grow-height=\"false\" fo:min-height=\"1.27cm\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"grpredef2\" style:family=\"graphic\" style:parent-style-name=\"standard\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\" draw:fill-color=\"#ffffff\" draw:textarea-vertical-align=\"bottom\" draw:auto-grow-height=\"false\" fo:min-height=\"1.27cm\"/>"
        "\n    </style:style>"*/
/* 2010-06-04 no longer hardcoded
        "\n    <style:style style:name=\"pr1\" style:family=\"presentation\" style:parent-style-name=\"Default-backgroundobjects\" style:list-style-name=\"Lpredef2\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\" draw:fill-color=\"#ffffff\" draw:textarea-horizontal-align=\"justify\" draw:textarea-vertical-align=\"middle\" draw:auto-grow-height=\"false\" draw:auto-grow-width=\"false\" fo:min-height=\"1.016cm\" fo:min-width=\"0cm\" fo:padding-top=\"0.13cm\" fo:padding-bottom=\"0.13cm\" fo:padding-left=\"0.25cm\" fo:padding-right=\"0.25cm\" fo:wrap-option=\"no-wrap\" draw:shadow=\"hidden\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"pr2\" style:family=\"presentation\" style:parent-style-name=\"Default-backgroundobjects\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\" draw:fill-color=\"#ffffff\" draw:textarea-horizontal-align=\"justify\" draw:textarea-vertical-align=\"middle\" draw:auto-grow-height=\"false\" draw:auto-grow-width=\"false\" fo:min-height=\"1.278cm\" fo:min-width=\"0cm\" fo:padding-top=\"0.13cm\" fo:padding-bottom=\"0.13cm\" fo:padding-left=\"0.25cm\" fo:padding-right=\"0.25cm\" fo:wrap-option=\"no-wrap\" draw:shadow=\"hidden\"/>"
        "\n    </style:style>"
*/
/*        "\n    <style:style style:name=\"prpredef3\" style:family=\"presentation\" style:parent-style-name=\"Default-backgroundobjects\" style:list-style-name=\"Lpredef2\">"
        "\n      <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\" draw:fill-color=\"#ffffff\" draw:textarea-horizontal-align=\"justify\" draw:textarea-vertical-align=\"middle\" draw:auto-grow-height=\"false\" draw:auto-grow-width=\"false\" fo:min-height=\"1.016cm\" fo:min-width=\"0cm\" fo:padding-top=\"0.13cm\" fo:padding-bottom=\"0.13cm\" fo:padding-left=\"0.25cm\" fo:padding-right=\"0.25cm\" fo:wrap-option=\"no-wrap\" draw:shadow=\"hidden\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Ppredef1\" style:family=\"paragraph\">"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" fo:text-indent=\"0cm\"/>"
        "\n      <style:text-properties fo:font-size=\"14pt\" style:font-size-asian=\"14pt\" style:font-size-complex=\"14pt\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Ppredef2\" style:family=\"paragraph\">"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" fo:text-align=\"end\" fo:text-indent=\"0cm\"/>"
        "\n      <style:text-properties fo:font-size=\"14pt\" style:font-size-asian=\"14pt\" style:font-size-complex=\"14pt\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Ppredef3\" style:family=\"paragraph\">"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" text:enable-numbering=\"false\" fo:text-indent=\"0cm\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Ppredef4\" style:family=\"paragraph\">"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" text:enable-numbering=\"false\" fo:text-indent=\"0cm\" style:writing-mode=\"lr-tb\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Ppredef5\" style:family=\"paragraph\">"
        "\n      <style:paragraph-properties style:writing-mode=\"lr-tb\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Ppredef6\" style:family=\"paragraph\">"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" fo:text-align=\"end\" text:enable-numbering=\"false\" fo:text-indent=\"0cm\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Ppredef7\" style:family=\"paragraph\">"
        "\n      <style:paragraph-properties fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" fo:text-align=\"end\" text:enable-numbering=\"false\" fo:text-indent=\"0cm\" style:writing-mode=\"lr-tb\"/>"
        "\n    </style:style>"
        "\n    <style:style style:name=\"Tpredef1\" style:family=\"text\">"
        "\n      <style:text-properties fo:color=\"#898989\" fo:font-size=\"12pt\" fo:language=\"pl\" fo:country=\"PL\" style:font-size-asian=\"12pt\" style:font-size-complex=\"12pt\"/>"
        "\n    </style:style>"
        "\n    <text:list-style style:name=\"Lpredef1\">"
        "\n      <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"•\">"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"2\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"1.27cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"3\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"2.54cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"4\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"3.81cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"5\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"6\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"7\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"8\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"9\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"10\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n    </text:list-style>"
        "\n    <text:list-style style:name=\"Lpredef2\">"
        "\n      <text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"•\">"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#898989\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"2\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"1.27cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"3\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"2.54cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"4\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"3.81cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"5\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"6\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"7\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"8\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"9\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n      <text:list-level-style-bullet text:level=\"10\" text:bullet-char=\"•\">"
        "\n        <style:list-level-properties text:space-before=\"5.08cm\"/>"
        "\n        <style:text-properties fo:font-family=\"Calibri\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\" fo:color=\"#000000\" fo:font-size=\"100%\"/>"
        "\n      </text:list-level-style-bullet>"
        "\n    </text:list-style>"*/
        "\n    <!-- /COPIED -->"
    );
    // 2. parse themes
    QMap<QString, MSOOXML::DrawingMLTheme*> themes;
    MSOOXML::Utils::ContainerDeleter< QMap<QString, MSOOXML::DrawingMLTheme*> > themesDeleter(themes);
    RETURN_IF_ERROR( parseThemes(themes, writers, errorMessage) )
#if 0 // moved to PptxXmlDocumentReader::read_sldMasterId()
    // 3. parse master slides
#ifdef __GNUC__
#warning TODO use MsooXmlRelationships; parse all used master slides; now one hardcoded master name is used
#else
#pragma WARNING( use MsooXmlRelationships; parse all used master slides; now one hardcoded master name is used )
#endif
    /* Algorithm:
     slides/_rels/slide1.xml.rels contains Target="../slideLayouts/slideLayout2.xml";
     slideLayouts/_rels/slideLayout2.xml.rels contains Target="../slideMasters/slideMaster1.xml"
     (the same for slide2)
    */
#define HARDCODED_SLIDEMASTER_PATH "ppt/slideMasters"
#define HARDCODED_SLIDEMASTER_FILE "slideMaster1.xml"
    PptxSlideProperties masterSlideProperties;
//! @todo support more than one slide master
    {
        PptxXmlSlideReaderContext context(
            *this,
            QLatin1String(HARDCODED_SLIDEMASTER_PATH), QLatin1String(HARDCODED_SLIDEMASTER_FILE),
            0, themes,
            PptxXmlSlideReader::SlideMaster,
            masterSlideProperties,
            *relationships
        );
        PptxXmlSlideReader slideMasterReader(writers);
        KoFilter::ConversionStatus status = loadAndParseDocument(
                                                &slideMasterReader,
                                                QLatin1String(HARDCODED_SLIDEMASTER_PATH "/" HARDCODED_SLIDEMASTER_FILE),
                                                errorMessage, &context
                                            );
        if (status != KoFilter::OK) {
            kDebug() << slideMasterReader.errorString();
            return status;
        }
    }
#endif //0
    QList<QByteArray> partNames = this->partNames(d->mainDocumentContentType());
    if (partNames.count() != 1) {
        errorMessage = i18n("Unable to find part for type %1", d->mainDocumentContentType());
        return KoFilter::WrongFormat;
    }
    // 4. parse document
    const QString documentPathAndFile(partNames.first());
    QString documentPath, documentFile;
    MSOOXML::Utils::splitPathAndFile(documentPathAndFile, &documentPath, &documentFile);
    kDebug() << documentPathAndFile << documentPath << documentFile;
    {
        PptxXmlDocumentReaderContext context(
            *this, themes,
            documentPath, documentFile,
            *relationships);
        PptxXmlDocumentReader documentReader(writers);
        RETURN_IF_ERROR( loadAndParseDocument(
            d->mainDocumentContentType(), &documentReader, writers, errorMessage, &context) )
    }
    // more here...
    return KoFilter::OK;
}

#include "PptxImport.moc"
