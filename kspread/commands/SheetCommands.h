/* This file is part of the KDE project
   Copyright 2004 Ariya Hidayat <ariya@kde.org>
   Copyright 2004 Laurent Montel <montel@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KSPREAD_SHEET_COMMANDS
#define KSPREAD_SHEET_COMMANDS

#include <QString>
#include <QUndoCommand>

#include "Sheet.h" // for Sheet::LayoutDirection

/**
 * The KSpread namespace.
 */
namespace KSpread
{
class Doc;

/**
 * Class RenameSheetCommand implements a command for renaming a sheet.
 *
 * \sa Sheet::setSheetName
 */

class RenameSheetCommand : public QUndoCommand
{
public:
  RenameSheetCommand( Sheet* sheet, const QString &name );

  virtual void redo();
  virtual void undo();
  virtual QString name() const;

protected:
  Sheet* sheet;
  QString oldName;
  QString newName;
};

class HideSheetCommand : public QUndoCommand
{
public:
  explicit HideSheetCommand( Sheet* sheet );

  virtual void redo();
  virtual void undo();
  virtual QString name() const;

protected:
  Doc* doc;
  QString sheetName;
};

class ShowSheetCommand : public QUndoCommand
{
public:
  explicit ShowSheetCommand( Sheet* sheet );

  virtual void redo();
  virtual void undo();
  virtual QString name() const;

protected:
  Doc* doc;
  QString sheetName;
};


class AddSheetCommand : public QUndoCommand
{
public:
  explicit AddSheetCommand( Sheet* sheet );

  virtual void redo();
  virtual void undo();
  virtual QString name() const;

protected:
    Sheet* sheet;
    Doc* doc;
};


class RemoveSheetCommand : public QUndoCommand
{
public:
  explicit RemoveSheetCommand( Sheet* sheet );

  virtual void redo();
  virtual void undo();
  virtual QString name() const;

protected:
    Sheet* sheet;
    Doc* doc;
};


/**
 * Class SheetPropertiesCommand implements a command for changing sheet properties.
 */

class SheetPropertiesCommand : public QUndoCommand
{
public:
  SheetPropertiesCommand( Doc* doc, Sheet* sheet );
  void setLayoutDirection( Sheet::LayoutDirection direction );
  void setAutoCalc( bool b );
  void setShowGrid( bool b );
  void setShowPageBorders( bool b );
  void setShowFormula( bool b );
  void setHideZero( bool b );
  void setShowFormulaIndicator( bool b );
  void setShowCommentIndicator( bool b );
  void setColumnAsNumber( bool b );
  void setLcMode( bool b );
  void setCapitalizeFirstLetter( bool b );

  virtual void redo();
  virtual void undo();
  virtual QString name() const;

protected:
  Sheet* sheet;
  Doc* doc;
  Sheet::LayoutDirection oldDirection, newDirection;
  bool oldAutoCalc, newAutoCalc;
  bool oldShowGrid, newShowGrid;
  bool oldShowPageBorders, newShowPageBorders;
  bool oldShowFormula, newShowFormula;
  bool oldHideZero, newHideZero;
  bool oldShowFormulaIndicator, newShowFormulaIndicator;
  bool oldShowCommentIndicator, newShowCommentIndicator;
  bool oldColumnAsNumber, newColumnAsNumber;
  bool oldLcMode, newLcMode;
  bool oldCapitalizeFirstLetter, newCapitalizeFirstLetter;
};

} // namespace KSpread

#endif // KSPREAD_SHEET_COMMANDS
