/* This file is part of the KDE libraries
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <koFilter.h>

#include <qfile.h>

#include <ktempfile.h>
#include <kdebug.h>
#include <koFilterManager.h>


KoFilter::KoFilter() : QObject( 0, 0 ), m_chain( 0 )
{
}

KoFilter::~KoFilter()
{
}


KoEmbeddingFilter::~KoEmbeddingFilter()
{
}

int KoEmbeddingFilter::lruPartIndex() const
{
    return m_partStack.top()->m_lruPartIndex;
}

KoEmbeddingFilter::KoEmbeddingFilter() : KoFilter()
{
    m_partStack.push( new PartState() );
}

int KoEmbeddingFilter::embedPart( const QCString& from, QCString& to,
                                  KoFilter::ConversionStatus& status, const QString& key )
{
    ++( m_partStack.top()->m_lruPartIndex );

    KTempFile tempIn;
    tempIn.setAutoDelete( true );
    savePartContents( tempIn.file() );
    tempIn.file()->close();

    KoFilterManager *manager = new KoFilterManager( tempIn.name(), from, m_chain );
    status = manager->exp0rt( QString::null, to );
    delete manager;

    // Add the part to the current "stack frame", using the number as key
    // if the key string is empty
    PartReference ref( lruPartIndex(), to );
    m_partStack.top()->m_partReferences.insert( key.isEmpty() ? QString::number( lruPartIndex() ) : key, ref );

    return lruPartIndex();
}

void KoEmbeddingFilter::startInternalEmbedding( const QString& key, const QCString& mimeType )
{
    filterChainEnterDirectory( QString::number( ++( m_partStack.top()->m_lruPartIndex ) ) );
    PartReference ref( lruPartIndex(), mimeType );
    m_partStack.top()->m_partReferences.insert( key, ref );
    m_partStack.push( new PartState() );
}

void KoEmbeddingFilter::endInternalEmbedding()
{
    if ( m_partStack.count() == 1 ) {
        kdError( 30500 ) << "You're trying to endInternalEmbedding more often than you started it" << endl;
        return;
    }
    delete m_partStack.pop();
    filterChainLeaveDirectory();
}

int KoEmbeddingFilter::internalPartReference( const QString& key )
{
    QMapConstIterator<QString, PartReference> it = m_partStack.top()->m_partReferences.find( key );
    if ( it == m_partStack.top()->m_partReferences.end() )
        return -1;
    return it.data().m_index;
}

QCString KoEmbeddingFilter::internalPartMimeType( const QString& key )
{
    QMapConstIterator<QString, PartReference> it = m_partStack.top()->m_partReferences.find( key );
    if ( it == m_partStack.top()->m_partReferences.end() )
        return QCString();
    return it.data().m_mimeType;
}

KoEmbeddingFilter::PartReference::PartReference( int index, const QCString& mimeType ) :
    m_index( index ), m_mimeType( mimeType )
{
}

bool KoEmbeddingFilter::PartReference::isValid()
{
    return m_index != 1 && !m_mimeType.isEmpty();
}

KoEmbeddingFilter::PartState::PartState() : m_lruPartIndex( 0 )
{
}

void KoEmbeddingFilter::savePartContents( QIODevice* )
{
}

void KoEmbeddingFilter::filterChainEnterDirectory( const QString& directory ) const
{
    m_chain->enterDirectory( directory );
}

void KoEmbeddingFilter::filterChainLeaveDirectory() const
{
    m_chain->leaveDirectory();
}

#include <koFilter.moc>
