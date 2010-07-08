/* This file is part of the KOffice project
   Copyright (C) 2010 Srihari Prasad G V <sri-hari@live.com>
   Copyright (C) 2010 Pramod S G <pramod.xyle@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the Library GNU General Public
   version 2 of the License, or (at your option) version 3 or,
   at the discretion of KDE e.V (which shall act as a proxy as in
   section 14 of the GPLv3), any later version..

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "htmlodfexport.h"

#include <qdom.h>
#include <qfontinfo.h>
#include <QFile>
#include <QString>
#include <QBuffer>
//Added by qt3to4:
#include <QByteArray>

#include <kdebug.h>
#include <kgenericfactory.h>

#include <KoFilterChain.h>
#include <KoOdfWriteStore.h>
#include <KoStoreDevice.h>
//#include <KoXmlWriter.h>

#include <document.h>
#include "pole.h"
#include <exportdialog.h>

//function prototypes of local functions
bool readStream(POLE::Storage& storage, const char* streampath, QBuffer& buffer);

typedef KGenericFactory<HTMLOdfExport> HTMLOdfExportFactory;
K_EXPORT_COMPONENT_FACTORY(libhtmlodf_export, HTMLOdfExportFactory("kofficefilters"))


HTMLOdfExport::HTMLOdfExport(QObject* parent, const QStringList&) :
KoFilter(parent), m_dialog(new ExportDialog())
{
}

HTMLOdfExport::~HTMLOdfExport()
{
}



KoFilter::ConversionStatus HTMLOdfExport::convert(const QByteArray &from, const QByteArray &to)
{
    // check for proper conversion
    if (to != "text/html"
            || from != "application/vnd.oasis.opendocument.text")
        return KoFilter::NotImplemented;

    if (m_dialog->exec() == QDialog::Rejected)
        return KoFilter::UserCancelled;

    kDebug(30513) << "######################## HTMLOdfExport::convert ########################";

    QString inputFile = m_chain->inputFile();
    QString outputFile = m_chain->outputFile();

    if (m_dialog->exec() == QDialog::Rejected)
        return KoFilter::UserCancelled;

    QBuffer buff3;

    LEInputStream wdocument_stream(&buff3);

    // Create output files
    KoStore *storeout;
    struct Finalizer {
    public:
        Finalizer(KoStore *store) : m_store(store), m_genStyles(0), m_document(0), m_contentWriter(0), m_bodyWriter(0) { }
        ~Finalizer() {
            delete m_store; delete m_genStyles; delete m_document; delete m_contentWriter; delete m_bodyWriter;
        }

        KoStore *m_store;
        KoGenStyles *m_genStyles;
        Document *m_document;
        KoXmlWriter* m_contentWriter;
        KoXmlWriter* m_bodyWriter;
    };

    storeout = KoStore::createStore(outputFile, KoStore::Write,
                                    "application/vnd.oasis.opendocument.text", KoStore::Zip);
    if (!storeout) {
        kWarning(30513) << "Unable to open output file!";
        return KoFilter::FileNotFound;
    }
    Finalizer finalizer(storeout);
    storeout->disallowNameExpansion();
    kDebug(30513) << "created storeout.";
    KoOdfWriteStore oasisStore(storeout);

    kDebug(30513) << "created oasisStore.";

    //create KoGenStyles for writing styles while we're parsing
    KoGenStyles* mainStyles = new KoGenStyles();
    finalizer.m_genStyles = mainStyles; // will delete this as it goes out of scope.

    //create a writer for meta.xml
    QBuffer buf;
    buf.open(QIODevice::WriteOnly);
    KoXmlWriter metaWriter(&buf);

    //create a writer for manifest.xml
    QBuffer manifestBuf;
    manifestBuf.open(QIODevice::WriteOnly);
    KoXmlWriter manifestWriter(&manifestBuf);

    //open contentWriter & bodyWriter *temp* writers
    //so we can write picture files while we parse
    QBuffer contentBuf;
    QBuffer bodyBuf;
    KoXmlWriter *contentWriter = new KoXmlWriter(&contentBuf);
    finalizer.m_contentWriter = contentWriter;
    KoXmlWriter *bodyWriter = new KoXmlWriter(&bodyBuf);
    finalizer.m_bodyWriter = bodyWriter;
    if (!bodyWriter || !contentWriter)
        return KoFilter::CreationError; //not sure if this is the right error to return

    kDebug(30513) << "created temp contentWriter and bodyWriter.";

    //open tags in bodyWriter
    bodyWriter->startElement("office:body");
    bodyWriter->startElement("office:text");

    //create our document object, writing to the temporary buffers

    kDebug(30513) << "finished parsing.";

    //save the office:automatic-styles & and fonts in content.xml
    mainStyles->saveOdfStyles(KoGenStyles::FontFaceDecls, contentWriter);
    mainStyles->saveOdfStyles(KoGenStyles::DocumentAutomaticStyles, contentWriter);

    //close tags in bodyWriter
    bodyWriter->endElement();//office:text
    bodyWriter->endElement();//office:body

    //now create real content/body writers & dump the information there
    KoXmlWriter* realContentWriter = oasisStore.contentWriter();
    realContentWriter->addCompleteElement(&contentBuf);
    KoXmlWriter* realBodyWriter = oasisStore.bodyWriter();
    realBodyWriter->addCompleteElement(&bodyBuf);

    //now close content & body writers
    if (!oasisStore.closeContentWriter()) {
        kWarning(30513) << "Error closing content.";
        return KoFilter::CreationError;
    }

    kDebug(30513) << "closed content & body writers.";

    //create the manifest file
    KoXmlWriter *realManifestWriter = oasisStore.manifestWriter("application/vnd.oasis.opendocument.text");
    //create the styles.xml file
    mainStyles->saveOdfStylesDotXml(storeout, realManifestWriter);
    realManifestWriter->addManifestEntry("content.xml", "text/xml");
    realManifestWriter->addCompleteElement(&manifestBuf);

    kDebug(30513) << "created manifest and styles.xml";

    //create meta.xml
    if (!storeout->open("meta.xml"))
        return KoFilter::CreationError;

    KoStoreDevice metaDev(storeout);
    KoXmlWriter *meta = KoOdfWriteStore::createOasisXmlWriter(&metaDev, "office:document-meta");
    meta->startElement("office:meta");
    meta->addCompleteElement(&buf);
    meta->endElement(); //office:meta
    meta->endElement(); //office:document-meta
    meta->endDocument();
    delete meta;
    if (!storeout->close())
        return KoFilter::CreationError;

    realManifestWriter->addManifestEntry("meta.xml", "text/xml");
    oasisStore.closeManifestWriter();

    kDebug(30513) << "######################## MSWordOdfImport::convert done ####################";
    return KoFilter::OK;
}

/*
 * Read the stream content into buffer.
 * @param storage; POLE storage
 * @param streampath; stream path into the POLE storage
 * @param buffer; buffer provided by the user
 */

#include <htmlodfexport.moc>
