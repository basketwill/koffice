/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#include <koFilterManager.h>
#ifndef USE_QFD
#include <koFilterManager.moc>
#endif

KoFilterManager* KoFilterManager::s_pSelf = 0;

KoFilterManager* KoFilterManager::self()
{
    if( s_pSelf == 0 )
    {
        s_pSelf = new KoFilterManager;
        s_pSelf->prepare=false;
    }
    return s_pSelf;
}

const QString KoFilterManager::fileSelectorList( const Direction &direction, const char *_format,
                                           const char *_native_pattern,
                                           const char *_native_name,
                                           const bool allfiles ) const
{
    QString service;
    if ( direction == Import )
        service = "Export == '";
    else
        service = "Import == '";
    service += _format;
    service += "'";

    QValueList<KoFilterEntry> vec = KoFilterEntry::query( service );

    QString ret;

    if ( _native_pattern && _native_name )
    {
#ifndef USE_QFD
        ret += _native_pattern;
        ret += "|";
#endif
        ret += _native_name;
        ret += " (";
        ret += _native_pattern;
        ret += ")";
    }

    for( unsigned int i = 0; i < vec.count(); ++i )
    {
        KMimeType::Ptr t;
        QString mime;
        if ( direction == Import )
            mime = vec[i].import;
        else
            mime = vec[i].export_;

        t = KMimeType::mimeType( mime );
        // Did we get exact this mime type ?
        if ( t && mime == t->mimeType() )
        {
            QStringList patterns = t->patterns();
            const char* s;
            for(unsigned int j = 0;j < patterns.count();j++)
            {
                s = patterns[j];
                if ( !ret.isEmpty() )
                    ret += "\n";
#ifndef USE_QFD
                ret += s;
                ret += "|";
#endif
                if ( direction == Import )
                    ret += vec[i].importDescription;
                else
                    ret += vec[i].exportDescription;
                ret += " (";
                ret += s;
                ret += ")";
            }
        }
        else
        {
            if ( !ret.isEmpty() )
                ret += "\n";
#ifndef USE_QFD
            ret += "*.*|";
#endif
            if ( direction == Import )
                ret += vec[i].importDescription;
            else
                ret += vec[i].exportDescription;
            ret += " (*.*)";
        }
    }
    if( allfiles )
    {
        if ( !ret.isEmpty() )
            ret += "\n";
#ifndef USE_QFD
        ret += "*.*|";
#endif
        ret += i18n( "All files (*.*)" );
    }
    return ret;
}

#ifndef USE_QFD
const bool KoFilterManager::prepareDialog( KFileDialog *dialog,
                                 const Direction &direction,
                                 const char *_format,
                                 const char *_native_pattern,
                                 const char *_native_name,
                                 const bool allfiles ) {

    unsigned i, j;
    QString service;
    if ( direction == Import )
        service = "Export == '";
    else
        service = "Import == '";
    service += _format;
    service += "'";

    QValueList<KoFilterEntry> vec = KoFilterEntry::query( service );

    QString filters;

    if ( _native_pattern && _native_name )
    {
        filters += _native_pattern;
        filters += "|";
        filters += _native_name;
        filters += " (";
        filters += _native_pattern;
        filters += ")";
    }

    for( i = 0; i < vec.count(); ++i )
    {
        KMimeType::Ptr t;
        QString mime;
        if ( direction == Import )
            mime = vec[i].import;
        else
            mime = vec[i].export_;

        t = KMimeType::mimeType( mime );
        // Did we get exact this mime type ?
        if ( t && mime == t->mimeType() )
        {
            QStringList patterns = t->patterns();
            const char* s;
            for( j = 0; j < patterns.count(); ++j )
            {
                s = patterns[j];
                if ( !filters.isEmpty() )
                    filters += "\n";
                filters += s;
                filters += "|";
                if ( direction == Import )
                    filters += vec[i].importDescription;
                else
                    filters += vec[i].exportDescription;
                filters += " (";
                filters += s;
                filters += ")";
            }
        }
        else
        {
            if ( !filters.isEmpty() )
                filters += "\n";
            filters += "*.*|";
            if ( direction == Import )
                filters += vec[i].importDescription;
            else
                filters += vec[i].exportDescription;
            filters += " (*.*)";
        }
    }
    if( allfiles )
    {
        if ( !filters.isEmpty() )
            filters += "\n";
        filters += "*.*|";
        filters += i18n( "All files (*.*)" );
    }

    dialog->setFilter(filters);

    QValueList<KoFilterDialogEntry> vec1 = KoFilterDialogEntry::query( service );

    ps=new PreviewStack(0L, "preview stack", this);

    unsigned int id;                 // id for the next widget

    for(i=0, id=1; i<vec1.count(); ++i) {
        KMimeType::Ptr t;
        QString mime;
        if ( direction == Import )
            mime = vec1[i].import;
        else
            mime = vec1[i].export_;

        t = KMimeType::mimeType( mime );
        // Did we get exactly this mime type ?
        if ( t && mime == t->mimeType() )
        {
            KoFilterDialog *filterdia=vec1[i].createFilterDialog();
            ASSERT(filterdia);

            QStringList patterns = t->patterns();
            QString tmp;
            unsigned short k;
            for(j=0; j<patterns.count(); ++j) {
                tmp=patterns[j];
                k=0;

                while(tmp[tmp.length()-k]!=QChar('.')) {
                    ++k;
                }
                //kDebugInfo(30003, "Extension:");
                //kDebugInfo(30003, tmp.right(k));
                dialogMap.insert(tmp.right(k), id);
            }
            ps->addWidget(filterdia, id);
            originalDialogs.insert(id, filterdia);
            ++id;
        }
    }
    if(!dialogMap.isEmpty())
        dialog->setPreviewWidget(ps);
    return true;
}

void KoFilterManager::cleanUp() {

    if(!dialogMap.isEmpty() && ps!=0L && !ps->isHidden()) {
        int id=ps->id(ps->visibleWidget());
        if(id!=0) {
            KoFilterDialog *dia=originalDialogs.find(id).data();
            if(dia!=0L) {
                config=dia->state();
                kDebugInfo(30003, config);
            }
            else
                kDebugWarning(30003, "default dia - no config!");
        }
    }
}

const int KoFilterManager::findWidget(const QString &ext) const {

    QMap<QString, int>::Iterator it;
    it=dialogMap.find(ext);

    if(it!=dialogMap.end())
        return it.data();
    else
        return 0;  // default Widget
}
#endif

const QString KoFilterManager::import( const QString & file, const char *_native_format )
{
    KURL url( file );

    KMimeType::Ptr t = KMimeType::findByURL( url, 0, true );
    QCString mimeType;
    if ( t && t->mimeType()!="application/octet-stream" ) {
        kDebugInfo( 30003, "######### FOUND MimeType %s", t->mimeType().data() );
        mimeType = t->mimeType();
    }
    else {
        kDebugInfo( 30003, "####### No MimeType found. Setting text/plain" );
        mimeType = "text/plain";
    }

    if ( mimeType == _native_format )
    {
        return file;
    }

    QString constr = "Export == '";
    constr += _native_format;
    constr += "' and Import == '";
    constr += mimeType;
    constr += "'";

    QValueList<KoFilterEntry> vec = KoFilterEntry::query( constr );
    if ( vec.isEmpty() )
    {
        QString tmp = i18n("Could not import file of type\n%1").arg( t->mimeType() );
        QApplication::restoreOverrideCursor();
        KMessageBox::error( 0L, tmp, i18n("Missing import filter") );
        return "";
    }

    KTempFile tempFile; // create with default file prefix, extension and mode
    if (tempFile.status() != 0)
        return "";
    QString tempfname = tempFile.name();

    unsigned int i=0;
    bool ok=false;
    while(i<vec.count() && !ok) {
        KoFilter* filter = vec[i].createFilter();
        ASSERT( filter );
#ifndef USE_QFD
        ok=filter->filter( QCString(file), QCString(tempfname), QCString(mimeType), QCString(_native_format), config );
#else
        ok=filter->filter( QCString(file), QCString(tempfname), QCString(mimeType), QCString(_native_format) );
#endif
        delete filter;
        ++i;
    }
    return ok ? tempfname : QString("");
}

const QString KoFilterManager::prepareExport( const QString & file, const char *_native_format )
{
    exportFile=file;
    native_format=_native_format;

    KTempFile tempFile; // create with default file prefix, extension and mode
    if (tempFile.status() != 0)
        return file;
    tmpFile = tempFile.name();
    prepare=true;
    return tmpFile;
}

const bool KoFilterManager::export_() {

    prepare=false;

    KURL url( exportFile );

    KMimeType::Ptr t = KMimeType::findByURL( url, 0, url.isLocalFile() );
    QCString mimeType;
    if (t) {
        kDebugInfo( 30003, "######### FOUND MimeType %c", t->mimeType().ascii() );
        mimeType = t->mimeType();
    }
    else {
        kDebugInfo( 30003, "####### No MimeType found. findByURL returned 0. Setting text/plain" );
        mimeType = "text/plain";
    }

    if ( (strcmp( mimeType, native_format ) == 0) )
    {
        kDebugInfo( 30003, "strcmp( mimeType, _native_format ) == 0 !! Returning without conversion. " );
        assert( url.isLocalFile() );
        return false;
    }

    QString constr = "Export == '";
    constr += mimeType;
    constr += "' and Import == '";
    constr += native_format;
    constr += "'";

    QValueList<KoFilterEntry> vec = KoFilterEntry::query( constr );
    if ( vec.isEmpty() )
    {
        QString tmp = i18n("Could not export file of type\n%1").arg( t->mimeType() );
        QApplication::restoreOverrideCursor();
        KMessageBox::error( 0L, tmp, i18n("Missing export filter") );
        return false;
    }

    unsigned int i=0;
    bool ok=false;
    while(i<vec.count() && !ok) {
        KoFilter* filter = vec[i].createFilter();
        ASSERT( filter );
#ifndef USE_QFD
        ok=filter->filter( QCString(tmpFile), QCString(exportFile), QCString(native_format), QCString(mimeType), config );
#else
        ok=filter->filter( QCString(tmpFile), QCString(exportFile), QCString(native_format), QCString(mimeType) );
#endif
        delete filter;
        ++i;
    }
    // Done, remove temporary file
    unlink( tmpFile.ascii() );
    return true;
}

#ifndef USE_QFD
//////////////////////////////////////////////////////////////////////////////
// PreviewStack

PreviewStack::PreviewStack(QWidget *parent, const char *name,
                           KoFilterManager *m) : QWidgetStack(parent, name),
                           mgr(m), hidden(false) {
}

PreviewStack::~PreviewStack() {
}

void PreviewStack::showPreview(const KURL &url) {

    QString tmp=url.url();
    unsigned short k=0, id;
    unsigned int foo=tmp.length();

    // try to find the extension
    while(tmp[foo-k]!=QChar('.') && k<=foo) {
        ++k;
    }

    // did we find the extension?
    if(tmp[foo-k]!=QChar('.')) {
        if(!hidden) {
            hide();
            hidden=true;
        }
        return;
    }
    // do we have a dialog for that extension? (0==we don't have one)
    id=mgr->findWidget(tmp.right(k));
    if(id==0) {
        if(!hidden) {
            hide();
            hidden=true;
        }
        return;
    }
    else {
        raiseWidget(id);
        if(hidden) {
            show();
            hidden=false;
        }
    }
}
#endif
