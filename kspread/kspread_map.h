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

#ifndef __kspread_map_h__
#define __kspread_map_h__

class KSpreadMap;
class KSpreadDoc;

class KoStore;

class DCOPObject;

#include <iostream.h>
#include <komlParser.h>

#include <qlist.h>
#include <qstring.h>
#include <qintdict.h>
#include <qobject.h>

#include "kspread_table.h"

/**
  A map is a simple container for all tables. Usually a complete map
  is saved in one file.
 */
class KSpreadMap : public QObject
{
public:
  /**
   * Created an empty map.
   */
  KSpreadMap( KSpreadDoc *_doc, const char* name = 0 );
  /**
   * This deletes all tables contained in this map.
   */
  virtual ~KSpreadMap();

  // C++
  virtual bool save( ostream& );
  virtual bool load( KOMLParser&, vector<KOMLAttrib>& );
  virtual bool loadChildren( KoStore* _store );

  bool saveChildren( KoStore* _store, const char *_path );
  /*
   * @return true if one of the direct children wants to
   *              be saved embedded. If there are no children or if
   *              every direct child saves itself into its own file
   *              then false is returned.
   *
   */
  bool hasToWriteMultipart();

  /**
   * @param _table becomes added to the map.
   */
  void addTable( KSpreadTable *_table );

  /**
   * @param _tables becomes removed from the map. This won't delete the table.
   */
  void removeTable( KSpreadTable *_table );

  /**
   * The table named @param _from is being moved to the table @param _to.
   * If @param _before is true @param _from is inserted before (after otherwise)   * @param _to.
   */
  void moveTable( const char* _from, const char* to, bool _before = true );

  KSpreadTable* findTable( const QString & _name );

  /**
   * Use the @ref #nextTable function to get all the other tables.
   * Attention: Function is not reentrant.
   *
   * @return a pointer to the first table in this map.
   */
  KSpreadTable* firstTable() { return m_lstTables.first();  }

  /**
   * Call @ref #firstTable first. This will set the list pointer to
   * the first table. Attention: Function is not reentrant.
   *
   * @return a pointer to the next table in this map.
   */
  KSpreadTable* nextTable() { return m_lstTables.next();  }

  QList<KSpreadTable>& tableList() { return m_lstTables; }

  /**
   * @return amount of tables in this map.
   */
  int count() { return m_lstTables.count(); }

  void update();

  /**
   * Needed for the printing Extension KOffice::Print
   */
  void draw( QPaintDevice* _dev, long int _width, long int _height,
	       float _scale );

  virtual DCOPObject* dcopObject();

  KSpreadDoc* doc();
    
private:
  /**
   * List of all tables in this map. The list has autodelete turned on.
   */
  QList<KSpreadTable> m_lstTables;

  /**
   * Pointer to the part which holds this map.
   */
  KSpreadDoc *m_pDoc;

  DCOPObject* m_dcop;
};

#endif
