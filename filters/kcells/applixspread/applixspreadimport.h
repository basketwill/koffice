/* This file is part of the KDE project
   Copyright (C) 2001 Enno Bartels <ebartels@nwn.de>

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

#ifndef APPLIXSPREADIMPORT_H
#define APPLIXSPREADIMPORT_H

#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QTextStream>
#include <QtCore/QList>
#include <QtCore/QByteArray>

#include <KoFilter.h>
#include <KOdfStore.h>

struct t_mycolor;
struct t_rc;

class APPLIXSPREADImport : public KoFilter
{

    Q_OBJECT

public:
    APPLIXSPREADImport(QObject *parent, const QVariantList&);
    virtual ~APPLIXSPREADImport() {}

    virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);

protected:
    QString nextLine(QTextStream &);
    QChar   specCharfind(QChar , QChar);
    void    writePen(QString &, int, int, QString);
    QString writeColor(t_mycolor *);
    void    readTypefaceTable(QTextStream &, QStringList &);
    void    readColormap(QTextStream &, QList<t_mycolor*> &);
    void    readView(QTextStream &, QString, t_rc &);
    void    filterSHFGBG(QString, int *, int *, int *);
    void    transPenFormat(QString, int *, int *);
    int     readHeader(QTextStream &);
    int     translateColumnNumber(const QString&);
    QString convertFormula(const QString& input) const;

private:
    int m_stepsize;
    int m_instep;
    int m_progress;
    QString m_nextPendingLine;
};
#endif // APPLIXSPREADIMPORT_H
