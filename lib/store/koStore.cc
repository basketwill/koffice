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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "koStore.h"

#include <qbuffer.h>
#include <kdebug.h>
#include <ktar.h>

KoStore::KoStore( const QString & _filename, Mode _mode )
{
  m_bIsOpen = false;
  m_mode = _mode;
  m_stream = 0L;

  kdDebug(30002) << "KoStore Constructor filename = " << _filename
		 << " mode = " << int(_mode) << endl;

  m_pTar = new KTarGz( _filename );

  m_bGood = m_pTar->open( _mode == Write ? IO_WriteOnly : IO_ReadOnly );
}

KoStore::~KoStore()
{
  m_pTar->close();
  delete m_pTar;
  if ( m_stream )
    delete m_stream;
}

QString KoStore::toExternalNaming( const QString & _internalNaming )
{
  // "root" is the main document, let's save it as "maindoc.xml"
  if (_internalNaming == "root")
    return "maindoc.xml";

  /*
  // tar:/0 is saved as part0.xml
  // tar:/0/1 is saved as part0/part1.xml
  // tar:/0/1/pictures/picture0.png is saved as part0/part1/pictures/picture0.png
  */
  if ( _internalNaming.left(5) == "tar:/" )
  {
    QString intern( _internalNaming.mid( 5 ) ); // remove protocol
    QString result( "" );
    int pos;
    while ( ( pos = intern.find( '/' ) ) != -1 ) {
      if ( QChar(intern.at(0)).isDigit() )
        result += "part";
      result += intern.left( pos + 1 ); // copy numbers (or "pictures") + "/"
      intern = intern.mid( pos + 1 ); // remove the dir we just processed
    }
    // now process the filename
    if ( QChar(intern.at(0)).isDigit() )
      result = result + "part" + intern + ".xml";
    else
      result += intern;

    return result;
  }

  return _internalNaming;
}

bool KoStore::open( const QString & _name )
{
  m_sName = toExternalNaming( _name );

  if ( m_bIsOpen )
  {
    kdWarning(30002) << "KoStore: File is already opened" << endl;
    return false;
  }

  if ( m_sName.length() > 512 )
  {
      kdError(30002) << "KoStore: Filename " << m_sName << " is too long" << endl;
    return false;
  }

  if ( m_mode == Write )
  {
    kdDebug(30002) << "KoStore: opening for writing '" << m_sName << "'" << endl;
    if ( m_strFiles.findIndex( m_sName ) != -1 ) // just check if it's there
    {
      kdWarning(30002) << "KoStore: Duplicate filename " << m_sName << endl;
      return false;
    }

    m_strFiles.append( m_sName );
    m_iSize = 0;
  }
  else if ( m_mode == Read )
  {
    kdDebug(30002) << "Opening for reading '" << m_sName << "'" << endl;

    const KTarEntry * entry = m_pTar->directory()->entry( m_sName );
    if ( entry == 0L )
    {
      kdWarning(30002) << "Unknown filename " << m_sName << endl;
      return false;
    }
    if ( entry->isDirectory() )
    {
      kdWarning(30002) << m_sName << " is a directory !" << endl;
      return false;
    }
    KTarFile * f = (KTarFile *) entry;
    m_byteArray = f->data();
    // warning, m_byteArray can be bigger than f->data().size() (if a previous file was bigger)
    // this is why we never use m_byteArray.size()
    m_iSize = f->data().size();
  }
  else
    assert( 0 );

  m_stream = new QBuffer( m_byteArray );
  m_stream->open( (m_mode == Write) ? IO_WriteOnly : IO_ReadOnly );
  m_bIsOpen = true;

  return true;
}

void KoStore::close()
{
  kdDebug(30002) << "KoStore: Closing" << endl;

  if ( !m_bIsOpen )
  {
    kdWarning(30002) << "KoStore: You must open before closing" << endl;
    return;
  }

  if ( m_mode == Write )
  {
    // write the whole bytearray at once into the tar file

    kdDebug(30002) << "Writing file " << m_sName << " into TAR archive. size "
		   << m_iSize << endl;
    m_pTar->writeFile( m_sName , "user", "group", m_iSize, m_byteArray.data() );
  }

  delete m_stream;
  m_stream = 0L;
  m_bIsOpen = false;
}

QByteArray KoStore::read( unsigned long int max )
{
  QByteArray data; // Data is a QArray<char>

  if ( !m_bIsOpen )
  {
    kdWarning(30002) << "KoStore: You must open before reading" << endl;
    data.resize( 0 );
    return data;
  }
  if ( m_mode != Read )
  {
    kdError(30002) << "KoStore: Can not read from store that is opened for writing" << endl;
    data.resize( 0 );
    return data;
  }

  if ( m_stream->atEnd() )
  {
    data.resize( 0 );
    return data;
  }

  if ( max > m_iSize - m_stream->at() )
    max = m_iSize - m_stream->at();
  if ( max == 0 )
  {
    data.resize( 0 );
    return data;
  }

  char *p = new char[ max ];
  m_stream->readBlock( p, max );

  data.setRawData( p, max );
  return data;
}

long KoStore::read( char *_buffer, unsigned long _len )
{
  if ( !m_bIsOpen )
  {
    kdError(30002) << "KoStore: You must open before reading" << endl;
    return -1;
  }
  if ( m_mode != Read )
  {
    kdError(30002) << "KoStore: Can not read from store that is opened for writing" << endl;
    return -1;
  }

  if ( m_stream->atEnd() )
    return 0;

  if ( _len > m_iSize - m_stream->at() )
    _len = m_iSize - m_stream->at();
  if ( _len == 0 )
    return 0;

  m_stream->readBlock( _buffer, _len );

  return _len;
}

long KoStore::size() const
{
  if ( !m_bIsOpen )
  {
    kdWarning(30002) << "KoStore: You must open before asking for a size" << endl;
    return -1;
  }
  if ( m_mode != Read )
  {
    kdWarning(30002) << "KoStore: Can not get size from store that is opened for writing" << endl;
    return -1;
  }
  return (long) m_iSize;
}

bool KoStore::write( const QByteArray& data )
{
  unsigned int len = data.size();
  if ( len == 0L ) return true; // nothing to do
  return write( data.data(), len ); // see below
}

bool KoStore::write( const char* _data, unsigned long _len )
{
  if ( _len == 0L ) return true;

  if ( !m_bIsOpen )
  {
    kdError(30002) << "KoStore: You must open before writing" << endl;
    return 0L;
  }
  if ( m_mode != Write  )
  {
    kdError(30002) << "KoStore: Can not write to store that is opened for reading" << endl;
    return 0L;
  }

  m_stream->writeBlock( _data, _len );
  m_iSize += _len;

  return true;
}

bool KoStore::at( int pos )
{
  return m_stream->at( pos );
}

int KoStore::at() const
{
  return m_stream->at();
}

bool KoStore::atEnd() const
{
  return m_stream->atEnd();
}
