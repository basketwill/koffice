/* This file is part of the KDE project
 * Copyright (C) 2005 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KWOdfLoader.h"
#include "KWOdfSharedLoadingData.h"
#include "KWDocument.h"
#include "KWPage.h"
#include "KWPageManager.h"
#include "frames/KWTextFrameSet.h"
#include "frames/KWTextFrame.h"

// koffice
#include <KOdfStylesReader.h>
#include <KOdfSettings.h>
#include <KOdfStoreReader.h>
#include <KXmlReader.h>
#include <KOdfXmlNS.h>
#include <KShapeRegistry.h>
#include <KShapeFactoryBase.h>
#include <KTextShapeData.h>
#include <KShapeLoadingContext.h>
#include <KStyleManager.h>
#include <KOdfLoadingContext.h>
#include <KoUpdater.h>
#include <KoProgressUpdater.h>
#include <KParagraphStyle.h>

// KDE + Qt includes
#include <QTextCursor>
#include <KDebug>

#include <KDocumentRdfBase.h>

KWOdfLoader::KWOdfLoader(KWDocument *document)
        : QObject(document),
        m_document(document)
{
    connect(this, SIGNAL(progressUpdate(int)), m_document, SIGNAL(sigProgress(int)));
}

KWOdfLoader::~KWOdfLoader()
{
}

KWDocument *KWOdfLoader::document() const
{
    return m_document;
}

//1.6: KWDocument::loadOasis
bool KWOdfLoader::load(KOdfStoreReader &odfStore)
{
    //kDebug(32001) << "========================> KWOdfLoader::load START";

    QPointer<KoUpdater> updater;
    if (m_document->progressUpdater()) {
        updater = m_document->progressUpdater()->startSubtask(1,
                                                           "KWOdfLoader::load");
        updater->setProgress(0);
    }

    KXmlElement content = odfStore.contentDoc().documentElement();
    KXmlElement realBody(KoXml::namedItemNS(content, KOdfXmlNS::office, "body"));
    if (realBody.isNull()) {
        kError(32001) << "No office:body found!" << endl;
        m_document->setErrorMessage(i18n("Invalid OpenDocument file. No office:body tag found."));
        return false;
    }

    KXmlElement body = KoXml::namedItemNS(realBody, KOdfXmlNS::office, "text");
    if (body.isNull()) {
        kError(32001) << "No office:text found!" << endl;
        KXmlElement childElem;
        QString localName;
        forEachElement(childElem, realBody)
            localName = childElem.localName();
        if (localName.isEmpty())
            m_document->setErrorMessage(i18n("Invalid OpenDocument file. No tag found inside office:body."));
        else
            m_document->setErrorMessage(i18n("This is not a word processing document, but %1. Please try opening it with the appropriate application.", KoDocument::tagNameToDocumentType(localName)));
        return false;
    }

    if (updater) updater->setProgress(20);

    // TODO check versions and mimetypes etc.

    bool hasMainText = false;
    KXmlElement childElem;
    forEachElement(childElem, body) {
        if (childElem.namespaceURI() == KOdfXmlNS::text
                && childElem.localName() != "page-sequence"
                && childElem.localName() != "user-field-decls"
                && childElem.localName() != "tracked-changes") {
            hasMainText = true;
            break;
        }
        if (childElem.namespaceURI() == KOdfXmlNS::table
                && childElem.localName() == "table") {
            hasMainText = true;
            break;
        }
    }

    KOdfLoadingContext odfContext(odfStore.styles(), odfStore.store(), m_document->componentData());
    KShapeLoadingContext sc(odfContext, m_document->resourceManager());

    // Load all styles before the corresponding paragraphs try to use them!
    KWOdfSharedLoadingData *sharedData = new KWOdfSharedLoadingData(this);
    sc.addSharedData(KODFTEXT_SHARED_LOADING_ID, sharedData);
    KStyleManager *styleManager = m_document->resourceManager()->resource(KOdfText::StyleManager).value<KStyleManager*>();
    Q_ASSERT(styleManager);
    sharedData->loadOdfStyles(sc, styleManager);

    if (updater) updater->setProgress(40);

    KOdfLoadingContext context(odfStore.styles(), odfStore.store(), m_document->componentData());

    loadMasterPageStyles(context, hasMainText);

    // add page background frame set
    KWFrameSet *pageBackgroundFrameSet = new KWFrameSet(KWord::BackgroundFrameSet);
    m_document->addFrameSet(pageBackgroundFrameSet);

#if 0 //1.6:
    KWOasisLoader oasisLoader(this);
    // <text:page-sequence> oasis extension for DTP (2003-10-27 post by Daniel)
    m_processingType = (!KoXml::namedItemNS(body, KOdfXmlNS::text, "page-sequence").isNull()) ? DTP : WP;
    m_hasTOC = false;
    m_tabStop = MM_TO_POINT(15);
    const KXmlElement *defaultParagStyle = styles.defaultStyle("paragraph");
    if (defaultParagStyle) {
        KOdfStyleStack stack;
        stack.push(*defaultParagStyle);
        stack.setTypeProperties("paragraph");
        QString tabStopVal = stack.property(KOdfXmlNS::style, "tab-stop-distance");
        if (!tabStopVal.isEmpty()) m_tabStop = KUnit::parseValue(tabStopVal);
    }
    m_initialEditing = 0;
    // TODO MAILMERGE
    // Variable settings
    // By default display real variable value
    if (!isReadWrite())
        m_varColl->variableSetting()->setDisplayFieldCode(false);
#endif

    // Load all styles before the corresponding paragraphs try to use them!
#if 0 //1.6:
    if (m_frameStyleColl->loadOasisStyles(context) == 0) {
        // no styles loaded -> load default styles
        loadDefaultFrameStyleTemplates();
    }
    if (m_tableStyleColl->loadOasisStyles(context, *m_styleColl, *m_frameStyleColl) == 0) {
        // no styles loaded -> load default styles
        loadDefaultTableStyleTemplates();
    }
    static_cast<KWVariableSettings *>(m_varColl->variableSetting())->loadNoteConfiguration(styles.officeStyle());
    loadDefaultTableTemplates();
//#else
    /*
    // We always needs at least one valid default paragraph style
    KParagraphStyle *defaultParagraphStyle = m_document->styleManager()->defaultParagraphStyle();
    //const KXmlElement *defaultParagraphStyle = context.stylesReader().defaultStyle("paragraph");
    //if(! defaultParagraphStyle) {
    KParagraphStyle *parastyle = new KParagraphStyle();
    parastyle->setName("Standard");
    m_document->styleManager()->add(parastyle);
    context.styleStack().setTypeProperties("paragraph"); // load all style attributes from "style:paragraph-properties"
    parastyle->loadOasis(context.styleStack()); // load the KParagraphStyle from the stylestack
    KCharacterStyle *charstyle = parastyle->characterStyle();
    context.styleStack().setTypeProperties("text"); // load all style attributes from "style:text-properties"
    charstyle->loadOasis(context.styleStack()); // load the KCharacterStyle from the stylestack
    //}
    */
#endif

    // load text:page-sequence
    KXmlElement pageSequence = KoXml::namedItemNS(body, KOdfXmlNS::text, "page-sequence");
    if (! pageSequence.isNull()) {
        KWPageManager *pageManager = m_document->pageManager();
        KXmlElement page;
        forEachElement(page, pageSequence) {
            if (page.namespaceURI() == KOdfXmlNS::text && page.localName() == "page") {
                QString master = page.attributeNS(KOdfXmlNS::text, "master-page-name", QString());
                pageManager->appendPage(pageManager->pageStyle(master));
            }
        }
    }

    if (updater) updater->setProgress(60);

    KTextShapeData textShapeData;
    if (hasMainText) {
        KWTextFrameSet *mainFs = new KWTextFrameSet(m_document, KWord::MainTextFrameSet);
        mainFs->setAllowLayout(false);
        mainFs->setPageStyle(m_document->pageManager()->pageStyle("Standard"));
        m_document->addFrameSet(mainFs);
        textShapeData.setDocument(mainFs->document(), false);
    }

    if (updater) updater->setProgress(80);

    // Let the TextShape handle loading the body element.
    textShapeData.loadOdf(body, sc, m_document->documentRdfBase());

    // Grab weak references to all the Rdf stuff that was loaded
    if (KDocumentRdfBase *rdf = m_document->documentRdfBase()) {
        rdf->updateInlineRdfStatements(textShapeData.document());
    }

    if (updater) updater->setProgress(90);

    loadSettings(odfStore.settingsDoc());

    if (updater) updater->setProgress(100);
    return true;
}

void KWOdfLoader::loadSettings(const KXmlDocument &settingsDoc)
{
    if (settingsDoc.isNull())
        return;

    kDebug(32001) << "KWOdfLoader::loadSettings";
    KOdfSettings settings(settingsDoc);
    KOdfSettings::Items viewSettings = settings.itemSet("view-settings");
    if (!viewSettings.isNull()) {
        QString docUnit(viewSettings.parseConfigItemString("unit"));
        if (!docUnit.isEmpty())
            m_document->setUnit(KUnit::unit(docUnit));
    }

    //1.6: KWOasisLoader::loadOasisIgnoreList
    KOdfSettings::Items configurationSettings = settings.itemSet("configuration-settings");
    if (!configurationSettings.isNull()) {
        const QString ignorelist = configurationSettings.parseConfigItemString("SpellCheckerIgnoreList");
        kDebug(32001) << "Ignorelist:" << ignorelist;
        //1.6: m_document->setSpellCheckIgnoreList(QStringList::split(',', ignorelist));
    }
    //1.6: m_document->variableCollection()->variableSetting()->loadOasis(settings);
}

void KWOdfLoader::loadMasterPageStyles(KOdfLoadingContext &context, bool hasMainText)
{
    //kDebug(32001) << " !!!!!!!!!!!!!! loadMasterPageStyles called !!!!!!!!!!!!!!";
    //kDebug(32001) << "Number of items :" << context.stylesReader().masterPages().size();

    //TODO probably we should introduce more logic to handle the "standard" even
    //in faulty documents. See also bugreport #129585 as example.
    const KOdfStylesReader &styles = context.stylesReader();
    QHashIterator<QString, KXmlElement *> it(styles.masterPages());
    while (it.hasNext()) {
        it.next();
        Q_ASSERT(! it.key().isEmpty());
        KWPageStyle masterPage = m_document->pageManager()->pageStyle(it.key());
        bool alreadyExists = masterPage.isValid();
        if (!alreadyExists)
            masterPage = KWPageStyle(it.key());
        const KXmlElement *masterNode = it.value();
        const KXmlElement *masterPageStyle = masterNode ? styles.findStyle(masterNode->attributeNS(KOdfXmlNS::style, "page-layout-name", QString())) : 0;
        if (masterPageStyle) {
            masterPage.loadOdf(context, *masterNode, *masterPageStyle, m_document->resourceManager());
            loadHeaderFooter(context, masterPage, *masterNode, LoadHeader);
            loadHeaderFooter(context, masterPage, *masterNode, LoadFooter);
        }
        masterPage.setHasMainTextFrame(hasMainText);
        if (!alreadyExists)
            m_document->pageManager()->addPageStyle(masterPage);
    }
}

// helper function to create a KWTextFrameSet for a header/footer.
void KWOdfLoader::loadHeaderFooterFrame(KOdfLoadingContext &context, const KWPageStyle &pageStyle, const KXmlElement &elem, KWord::TextFrameSetType fsType)
{
    KWTextFrameSet *fs = new KWTextFrameSet(m_document, fsType);
    fs->setPageStyle(pageStyle);
    fs->setAllowLayout(false);
    m_document->addFrameSet(fs);

    kDebug(32001) << "KWOdfLoader::loadHeaderFooterFrame localName=" << elem.localName() << " type=" << fs->name();

    // use auto-styles from styles.xml, not those from content.xml
    context.setUseStylesAutoStyles(true);

    KShapeLoadingContext ctxt(context, m_document->resourceManager());
    KWOdfSharedLoadingData *sharedData = new KWOdfSharedLoadingData(this);
    KStyleManager *styleManager = m_document->resourceManager()->resource(KOdfText::StyleManager).value<KStyleManager*>();
    Q_ASSERT(styleManager);
    sharedData->loadOdfStyles(ctxt, styleManager);
    ctxt.addSharedData(KODFTEXT_SHARED_LOADING_ID, sharedData);

    KTextLoader loader(ctxt);
    QTextCursor cursor(fs->document());
    loader.loadBody(elem, cursor);

    // restore use of auto-styles from content.xml, not those from styles.xml
    context.setUseStylesAutoStyles(false);
}

//1.6: KWOasisLoader::loadOasisHeaderFooter
void KWOdfLoader::loadHeaderFooter(KOdfLoadingContext &context, KWPageStyle &pageStyle, const KXmlElement &masterPage, HFLoadType headerFooter)
{
    // The actual content of the header/footer.
    KXmlElement elem = KoXml::namedItemNS(masterPage, KOdfXmlNS::style, headerFooter == LoadHeader ? "header" : "footer");
    // The two additional elements <style:header-left> and <style:footer-left> specifies if defined that even and odd pages
    // should be displayed different. If they are missing, the conent of odd and even (aka left and right) pages are the same.
    KXmlElement leftElem = KoXml::namedItemNS(masterPage, KOdfXmlNS::style, headerFooter == LoadHeader ? "header-left" : "footer-left");
    // Used in KWPageStyle to determine if, and what kind of header/footer to use.
    KWord::HeaderFooterType hfType = elem.isNull() ? KWord::HFTypeNone : leftElem.isNull() ? KWord::HFTypeUniform : KWord::HFTypeEvenOdd;

    if (! leftElem.isNull()) {   // header-left and footer-left
        loadHeaderFooterFrame(context, pageStyle, leftElem, headerFooter == LoadHeader ? KWord::EvenPagesHeaderTextFrameSet : KWord::EvenPagesFooterTextFrameSet);
    }

    if (! elem.isNull()) {   // header and footer
        loadHeaderFooterFrame(context, pageStyle, elem, headerFooter == LoadHeader ? KWord::OddPagesHeaderTextFrameSet : KWord::OddPagesFooterTextFrameSet);
    }

    if (headerFooter == LoadHeader) {
        pageStyle.setHeaderPolicy(hfType);
    } else {
        pageStyle.setFooterPolicy(hfType);
    }
}

