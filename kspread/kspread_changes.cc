/* This file is part of the KDE project
   Copyright (C) 2002 Norbert Andres, nandres@web.de

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

#include "kspread_cell.h"
#include "kspread_changes.h"
#include "kspread_map.h"
#include "kspread_sheet.h"

#include <kconfig.h>
#include <kmdcodec.h>
#include <qdom.h>

KSpreadChanges::KSpreadChanges( KSpreadMap * map )
  : m_counter( 0 ),
    m_map( map )
{
  m_dependancyList.setAutoDelete( false );
  m_changeRecords.setAutoDelete( true );
  m_authors.setAutoDelete( true );

  KConfig * emailCfg = new KConfig( "emaildefaults", true );
  emailCfg->setGroup( "Defaults" );
  m_name = emailCfg->readEntry( "FullName" );
}

KSpreadChanges::~KSpreadChanges()
{
}

void KSpreadChanges::setProtected( QCString const & hash )
{
  m_strPassword = hash;
}

void KSpreadChanges::saveXml( QDomDocument & doc, QDomElement & map )
{
  if ( m_changeRecords.first() == 0 )
    return;

  QDomElement records = doc.createElement( "tracked-changes" );

  if ( !m_strPassword.isNull() )
  {
    if ( m_strPassword.size() > 0 )
    {
      QCString str = KCodecs::base64Encode( m_strPassword ); 
      records.setAttribute( "protected", QString( str.data() ) );
    }
    else
      records.setAttribute( "protected", "" );      
  }

  saveAuthors( doc, records );
  saveChanges( doc, records );

  map.appendChild( records );
}

void KSpreadChanges::saveAuthors( QDomDocument & doc, QDomElement & changes )
{
  if ( m_authors.first() == 0 )
    return;

  QDomElement authors = doc.createElement( "authors" );
  QPtrListIterator<AuthorInfo> it( m_authors );
  for ( ; it.current(); ++it )
  {
    QDomElement author = doc.createElement( "author" );
    author.setAttribute( "id",   QString::number( it.current()->id() ) );
    author.setAttribute( "name", it.current()->name() );
    authors.appendChild( author );
  }
  changes.appendChild( authors );
}

void KSpreadChanges::saveChanges( QDomDocument & doc, QDomElement & changes )
{
  QDomElement records = doc.createElement( "changes" );
  QPtrListIterator<ChangeRecord> it( m_changeRecords );
  for ( ; it.current(); ++it )
  {
    QDomElement record = doc.createElement( "change" );

    it.current()->saveXml( doc, record );

    records.appendChild( record );
  }
  changes.appendChild( records );
}

bool KSpreadChanges::loadXml( QDomElement const & changes )
{
  if ( changes.hasAttribute( "protected" ) )
  {
    QString passwd = changes.attribute( "protected" );
    
    if ( passwd.length() > 0 )
    {
      QCString str( passwd.latin1() );
      m_strPassword = KCodecs::base64Decode( str );        
    }
    else
      m_strPassword = QCString( "" );
  }

  QDomNode n = changes.firstChild();
  while( !n.isNull() )
  {
    QDomElement e = n.toElement();
    if ( !e.isNull() && e.tagName() == "changes" )
    {
      if ( !loadChanges( e ) )
        return false;
    }
    else
    if ( !e.isNull() && e.tagName() == "authors" )
    {
      if ( !loadAuthors( e ) )
        return false;      
    }
    n = n.nextSibling();
  }

  return true;
}

bool KSpreadChanges::loadAuthors( QDomElement const & authors )
{
  QDomNode n = authors.firstChild();
  while( !n.isNull() )
  {
    QDomElement e = n.toElement();
    if ( !e.isNull() && e.tagName() == "author" )
    {
      int id;
      bool ok = false;
      if ( e.hasAttribute( "id" ) )
        id = e.attribute( "id" ).toInt( &ok );

      if ( ok &&  e.hasAttribute( "name" ) )
      {          
        AuthorInfo * info = new AuthorInfo( id, e.attribute( "name" ) );
        m_authors.append( info );
      }
      else 
        return false;
    }
    n = n.nextSibling();
  }

  return true;
}

bool KSpreadChanges::loadChanges( QDomElement const & changes )
{
  QDomNode n = changes.firstChild();
  while( !n.isNull() )
  {
    QDomElement e = n.toElement();

    ChangeRecord * record = new ChangeRecord();

    if ( record->loadXml( e ) )
      m_changeRecords.append( record );
    else
      delete record;

    n = n.nextSibling();
  }

  return true;
}

void KSpreadChanges::addChange( KSpreadSheet * table, KSpreadCell * cell, QPoint const & point,
                                QString const & oldFormat, QString const & oldValue )
{
  ++m_counter;
  CellChange * change  = new CellChange();
  change->authorID     = addAuthor();
  change->formatString = oldFormat;
  change->oldValue     = oldValue;
  change->cell         = cell;
  QPoint cellRef( cell->column(), cell->row() );

  ChangeRecord * record = new ChangeRecord( m_counter, ChangeRecord::PENDING, ChangeRecord::CELL,
                                            table, cellRef, change );
  m_changeRecords.append( record );

  // find records we depend on
  ChangeRecord * r = 0;

  for ( r = m_dependancyList.last(); r; r = m_dependancyList.prev() )
  {
    if ( r->isDependant( table, cellRef ) )
    {
      r->addDependant( record, cellRef );
      return;
    }
  }

  // nothing, so we add ourself to the end
  m_dependancyList.append( record );
}

int KSpreadChanges::addAuthor()
{
  int id = m_authors.count();
  QPtrListIterator<AuthorInfo> it( m_authors );
  for( ; it.current(); ++it )
  {
    if ( it.current()->name() == m_name )
      return it.current()->id();
  }

  AuthorInfo * info  = new AuthorInfo( id, m_name );

  m_authors.append( info );

  return id;
}

/*
 * class ChangeRecord
 */

KSpreadChanges::ChangeRecord::ChangeRecord()
{
  m_dependants.setAutoDelete( false );
}

KSpreadChanges::ChangeRecord::ChangeRecord( int id, State state, ChangeType type, KSpreadSheet const * table, 
                                            QPoint const & cellRef, Change * change )
  : m_id( id ), m_state( state ), m_type( type ), m_table ( table ),
    m_cell( cellRef ), m_change( change )
{
  m_dependants.setAutoDelete( false );
}

KSpreadChanges::ChangeRecord::~ChangeRecord()
{
  delete m_change;
}

void KSpreadChanges::ChangeRecord::saveXml( QDomDocument & doc, QDomElement & changes ) const
{
  QDomElement change = doc.createElement( "record" );
  change.setAttribute( "y",     QString::number( m_cell.y() ) );
  change.setAttribute( "x",     QString::number( m_cell.x() ) );
  change.setAttribute( "id",    QString::number( m_id ) );
  change.setAttribute( "state", QString::number( (int) m_state ) );
  change.setAttribute( "type",  QString::number( (int) m_type ) );
  change.setAttribute( "table", m_table->tableName() );

  QPtrListIterator<ChangeRecord> it( m_dependants );
  for ( ; it.current(); ++it )
  {
    QDomElement dep = doc.createElement( "dependant" );
    dep.setAttribute( "id", it.current()->id() );
    change.appendChild( dep );
  }  

  m_change->saveXml( doc, change );

  changes.appendChild( change );
}

bool KSpreadChanges::ChangeRecord::loadXml( QDomElement & changes )
{
}

bool KSpreadChanges::ChangeRecord::isDependant( KSpreadSheet const * const table, QPoint const & cell ) const
{
  if ( table != m_table )
    return false;

  if ( cell.x() != 0 && cell.x() == m_cell.x() )
    return true;

  if ( cell.y() != 0 && cell.y() == m_cell.y() )
    return true;

  return false;
}

void KSpreadChanges::ChangeRecord::addDependant( ChangeRecord * record, QPoint const & cellRef )
{
  QPtrListIterator<ChangeRecord> it( m_dependants );
  for ( ; it.current(); ++it )
  {
    if ( it.current()->isDependant( record->table(), cellRef ) )
    {
      it.current()->addDependant( record, cellRef );
      return;
    }
  }
  m_dependants.append( record );
}

/*
 * class Change
 */

KSpreadChanges::Change::~Change()
{
  delete comment;
}

/*
 * class CellChange
 */

KSpreadChanges::CellChange::~CellChange()
{
  delete comment;
  comment = 0;
}

bool KSpreadChanges::CellChange::loadXml( QDomElement const & change, 
                                          KSpreadSheet const * const table,
                                          QPoint const & cellRef )
{
  bool ok = false;
  if ( change.hasAttribute( "author" ) )
  {
    authorID = change.attribute( "author" ).toInt( &ok );
    if ( !ok ) 
      return false;
  }

  if ( change.hasAttribute( "time" ) )
  {
    uint n = (uint) change.attribute( "time" ).toInt( &ok );

    if ( !ok )
      return false;

    timestamp.setTime_t( n );
  }

  if ( change.hasAttribute( "comment" ) )
    comment = new QString( change.attribute( "comment" ) );

  if ( change.hasAttribute( "format" ) )
    formatString = change.attribute( "format" );
  else return false;

  if ( change.hasAttribute( "oldValue" ) )
    oldValue = change.attribute( "oldValue" );
  else return false;

  return true;
}

void KSpreadChanges::CellChange::saveXml( QDomDocument & doc, QDomElement & change ) const
{
    int        authorID;
    QDateTime  timestamp;
    QString *  comment;
    QString       formatString;
    QString       oldValue;
    KSpreadCell * cell;

  QDomElement cellChange = doc.createElement( "cell" );
  cellChange.setAttribute( "author", QString::number( authorID ) );
  cellChange.setAttribute( "time",   QString::number( (int) timestamp.toTime_t() ) );
  if ( comment && !comment->isNull() )
    cellChange.setAttribute( "comment", *comment );
  cellChange.setAttribute( "format", formatString );
  cellChange.setAttribute( "oldValue", oldValue );  

  change.appendChild( cellChange );
}




