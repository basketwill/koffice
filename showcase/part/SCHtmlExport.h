/* This file is part of the KDE project
   Copyright (C) 2009-2010 Benjamin Port <port.benjamin@gmail.com>
   Copyright (C) 2009 Yannick Motta <yannick.motta@gmail.com>

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

#ifndef SCHTMLEXPORT_H
#define SCHTMLEXPORT_H

#include <KUrl>
#include <QStringList>

class SCView;
class KoPAPage;
class KJob;

class SCHtmlExport : public QObject
{
    Q_OBJECT
public:
    struct Parameter {
        Parameter() {}

        Parameter(KUrl styleUrl, SCView *kprView, QList<KoPAPage*> slides, KUrl destination,
                  QString author, QString title, QStringList slidesNames, bool openBrowser)
                      : styleUrl(styleUrl)
                      , kprView(kprView)
                      , slides(slides)
                      , destination(destination)
                      , author(author)
                      , title(title)
                      , slidesNames(slidesNames)
                      , openBrowser(openBrowser)
        {
        }

        KUrl styleUrl;
        SCView *kprView;
        QList<KoPAPage*> slides;
        KUrl destination;
        QString author;
        QString title;
        QStringList slidesNames;
        bool openBrowser;
    };

    SCHtmlExport();
    ~SCHtmlExport();
    void exportHtml(const Parameter &parameters);

    /**
     * Generates a preview of 1 frame into a tempoary directory
     * @param parameters Presentation data (only 1 slide should be provided in "slides" filed)
     * @param previewUrl  URL of output html
     */
    KUrl exportPreview(const Parameter &parameters);

protected:
    void extractStyle();
    void generateHtml();
    void generateToc();
    void exportImageToTmpDir();
    void writeHtmlFileToTmpDir(const QString &fileName, const QString &htmlBody);
    void copyFromTmpToDest();

private slots:
    void moveResult(KJob *job);

private:
    QString m_tmpDirPath;
    Parameter m_parameters;
};

#endif /* SCHTMLEXPORT_H */
