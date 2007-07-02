/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef KPRESENTER_ABOUTDATA
#define KPRESENTER_ABOUTDATA

#include <kaboutdata.h>
#include <klocale.h>
#include <kofficeversion.h>

static const char* description=I18N_NOOP("KOffice Presentation Tool");
static const char* version=KOFFICE_VERSION_STRING;

KAboutData * newKPresenterAboutData()
{
    KAboutData * aboutData=new KAboutData( "kpresenter", 0, ki18n("KPresenter"),
                                           version, ki18n(description), KAboutData::License_LGPL,
                                           ki18n("(c) 1998-2007, The KPresenter Team"), KLocalizedString(),
                                           "http://www.koffice.org/kpresenter/");
    aboutData->addAuthor(ki18n("Laurent Montel"), ki18n("current maintainer"), "montel@kde.org");
    aboutData->addAuthor(ki18n("Werner Trobin"), KLocalizedString(), "trobin@kde.org");
    aboutData->addAuthor(ki18n("David Faure"), KLocalizedString(), "faure@kde.org");
    aboutData->addAuthor(ki18n("Toshitaka Fujioka"), KLocalizedString(), "fujioka@kde.org");
    aboutData->addAuthor(ki18n("Lukáš Tinkl"), KLocalizedString(), "lukas@kde.org");
    aboutData->addAuthor(ki18n("Thorsten Zachmann"), KLocalizedString(), "t.zachmann@zagge.de");
    aboutData->addAuthor(ki18n("Ariya Hidayat"), KLocalizedString(), "ariya@kde.org");
    aboutData->addAuthor(ki18n("Percy Leonhardt"), KLocalizedString(), "percy@eris23.de");
    aboutData->addAuthor(ki18n("Thomas Zander"));
    aboutData->addAuthor(ki18n("Reginald Stadlbauer"),ki18n("original author"), "reggie@kde.org");
    return aboutData;
}

#endif
