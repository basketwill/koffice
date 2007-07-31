/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>
   Copyright (C) 2004 Nicolas GOUTTE <goutte@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#include <csvimport.h>

#include <QFile>
#include <QRegExp>
//Added by qt3to4:
#include <QByteArray>
#include <Q3MemArray>

#include <kapplication.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <KoFilterChain.h>
#include <KoFilterManager.h>

#include <kspread/Cell.h>
#include <kspread/Doc.h>
#include <kspread/Global.h>
#include <kspread/Map.h>
#include <kspread/RowColumnFormat.h>
#include <kspread/Sheet.h>
#include <kspread/Style.h>
#include <kspread/Value.h>
#include <kspread/ValueConverter.h>

#include <csvdialog.h>

using namespace KSpread;

// hehe >:->

/*
 To generate a test CSV file:

 perl -e '$i=0;while($i<30000) { print rand().",".rand()."\n"; $i++ }' > file.csv
*/

typedef KGenericFactory<CSVFilter> CSVImportFactory;
K_EXPORT_COMPONENT_FACTORY( libcsvimport, CSVImportFactory( "kofficefilters" ) )

CSVFilter::CSVFilter(QObject* parent, const QStringList&) :
                     KoFilter(parent) {
}

KoFilter::ConversionStatus CSVFilter::convert( const QByteArray& from, const QByteArray& to )
{
    QString file( m_chain->inputFile() );
    KoDocument* document = m_chain->outputDocument();

    if ( !document )
        return KoFilter::StupidError;

    if ( !qobject_cast<const KSpread::Doc *>( document ) )
    {
      kWarning(30501) << "document isn't a KSpread::Doc but a " << document->metaObject()->className() << endl;
        return KoFilter::NotImplemented;
    }
    if(from!="text/csv" && from!="text/plain" || to!="application/x-kspread")
    {
        kWarning(30501) << "Invalid mimetypes " << from << " " << to << endl;
        return KoFilter::NotImplemented;
    }

    Doc *ksdoc = static_cast<Doc *>( document ); // type checked above

    if(ksdoc->mimeType()!="application/x-kspread")
    {
        kWarning(30501) << "Invalid document mimetype " << ksdoc->mimeType() << endl;
        return KoFilter::NotImplemented;
    }

    QFile in(file);
    if(!in.open(QIODevice::ReadOnly)) {
        KMessageBox::sorry( 0L, i18n("CSV filter cannot open input file - please report.") );
        in.close();
        return KoFilter::FileNotFound;
    }

    QString csv_delimiter;
    // ###### FIXME: disabled for now
    //if (!config.isNull())
    //    csv_delimiter = config[0];

    QByteArray inputFile( in.readAll() );
    in.close();

    CSVDialog *dialog = new CSVDialog(0L, inputFile, csv_delimiter );
    if (!m_chain->manager()->getBatchMode() && !dialog->exec())
	  return KoFilter::UserCancelled;
    inputFile.resize( 0 ); // Release memory (input file content)

    ElapsedTime t( "Filling data into document" );

    Sheet *sheet=ksdoc->map()->addNewSheet();

    int numRows = dialog->getRows();
    int numCols = dialog->getCols();

    if (numRows == 0)
      ++numRows;

    int step = 100 / numRows * numCols;
    int value = 0;

    emit sigProgress(value);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    int i;
    double init = sheet->nonDefaultColumnFormat( 1 )->width();
    Q3MemArray<double> widths( numCols );
    for ( i = 0; i < numCols; ++i )
      widths[i] = init;

    Cell cell( sheet, 1, 1 );
    QFontMetrics fm( cell.style().font() );

    for ( int row = 0; row < numRows; ++row )
    {
        for (int col = 0; col < numCols; ++col)
        {
            value += step;
            emit sigProgress(value);
            const QString text( dialog->getText( row, col ) );

            // ### FIXME: how to calculate the width of numbers (as they might not be in the right format)
            const double len = fm.width( text );
            if ( len > widths[col] )
              widths[col] = len;

            switch (dialog->getHeader(col))
            {
             case CSVDialog::TEXT:
             {
               //see CSVDialog::accept(), Tomas introduced the Generic format between KOffice 1.3 and 1.4
               //the Insert->External Data-> ... dialog uses the generic format for everything (see mentioned method)
               //I will use this approach only for the TEXT format in the CSV import filter... (raphael)
               //### FIXME: long term solution is to allow to select Generic format ("autodetect") in the dialog and make it the default

               cell = Cell( sheet, col + 1, row + 1 );
               cell.parseUserInput( text );

               /* old code
              cell = Cell( sheet, col + 1, row + 1, s );
              cell.setValue( Value( text ) );
               */
              break;
             // ### TODO: put the code for the different numbers together (at least partially)
             }
             case CSVDialog::NUMBER:
                {
                    bool ok = false;
                    double d = ksdoc->locale()->readNumber( text, &ok );
                    // If not, try with the '.' as decimal separator
                    if ( !ok )
                        d = text.toDouble( &ok );
                    if ( !ok )
                    {
                        cell = Cell( sheet, col + 1, row + 1 );
                        cell.setValue( Value( text ) );
                    }
                    else
                    {
                        cell = Cell( sheet,  col + 1, row + 1 );
                        cell.setValue(Value(d));
                    }
                    cell.setUserInput(text);
                    break;
                }
             case CSVDialog::COMMANUMBER:
                {
                    bool ok = false;
                    QString tmp ( text );
                    tmp.remove ( QRegExp( "[^0-9,Ee+-]" ) ); // Keep only 0 to 9, comma, E, e, plus, minus
                    tmp.replace ( ',', '.' );
                    kDebug(30501) <<"Comma:" << text <<" =>" << tmp;
                    const double d = tmp.toDouble( &ok );
                    if ( !ok )
                    {
                        cell = Cell( sheet, col + 1, row + 1 );
                        cell.setValue( Value( text ) );
                    }
                    else
                    {
                        cell = Cell( sheet, col + 1, row + 1 );
                        cell.setValue(Value(d));
                    }
                    cell.setUserInput(tmp);
                    break;
                }
             case CSVDialog::POINTNUMBER:
                {
                    bool ok = false;
                    QString tmp ( text );
                    tmp.remove ( QRegExp( "[^0-9\\.EeD+-]" ) ); // Keep only 0 to 9, dot, E, e, D, plus, minus
                    tmp.replace ( 'D', 'E' ); // double from FORTRAN use D instead of E
                    kDebug(30501) <<"Point:" << text <<" =>" << tmp;
                    const double d = tmp.toDouble( &ok );
                    if ( !ok )
                    {
                        cell = Cell( sheet, col + 1, row + 1 );
                        cell.setValue( Value( text ) );
                    }
                    else
                    {
                        cell = Cell( sheet, col + 1, row + 1 );
                        cell.setValue(Value(d));
                    }
                    cell.setUserInput(tmp);
                    break;
                }
             case CSVDialog::DATE:
             {
              cell = Cell( sheet, col + 1, row + 1 );
              cell.setUserInput(text);
              cell.setValue(ksdoc->converter()->asDate(Value(text)));
              break;
             }
             case CSVDialog::CURRENCY:
             {
              cell = Cell( sheet, col + 1, row + 1 );
              cell.setUserInput(text);
              Value value(text);
              value.setFormat(Value::fmt_Money);
              cell.setValue(value);
              break;
             }
            }
        }
    }

    emit sigProgress( 98 );

    ElapsedTime t2( "Resizing columns" );
    for ( i = 0; i < numCols; ++i )
    {
      ColumnFormat * c  = sheet->nonDefaultColumnFormat( i + 1 );
      c->setWidth( widths[i] );
    }

    emit sigProgress( 100 );
    QApplication::restoreOverrideCursor();
    delete dialog;

    return KoFilter::OK;
}

#include <csvimport.moc>
