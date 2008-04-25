/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOPADOCUMENT_H
#define KOPADOCUMENT_H

#include <QObject>

#include <KoOdf.h>
#include <KoDocument.h>
#include <KoShapeControllerBase.h>
#include "KoPageApp.h"
#include "kopageapp_export.h"

class KoShapeSavingContext;
class KoPAPage;
class KoPAPageBase;
class KoPAMasterPage;
class KoPALoadingContext;
class KoPASavingContext;

class KoInlineTextObjectManager;

/// Document class that stores KoPAPage and KoPAMasterPage objects
class KOPAGEAPP_EXPORT KoPADocument : public KoDocument, public KoShapeControllerBase
{
    Q_OBJECT
public:

    explicit KoPADocument( QWidget* parentWidget, QObject* parent, bool singleViewMode = false );
    virtual ~KoPADocument();

    void paintContent( QPainter &painter, const QRect &rect);

    bool loadXML( QIODevice *, const KoXmlDocument & doc );
    bool loadOdf( KoOdfReadStore & odfStore );

    bool saveOdf( SavingContext & documentContext );

    /**
     * Load master pages
     *
     * @param masterStyles
     * @param context
     */
    QList<KoPAPageBase *> loadOdfMasterPages( const QHash<QString, KoXmlElement*> masterStyles, KoPALoadingContext & context );

    /**
     * Load pages
     *
     * @param body
     * @param context
     */
    QList<KoPAPageBase *> loadOdfPages( const KoXmlElement & body, KoPALoadingContext & context );

    /// reimplemented
    virtual bool completeLoading( KoStore* store );

    /// reimplemented
    virtual bool completeSaving( KoStore* store );


    /**
     * Save pages
     *
     * This is used by saveOasis and for copy and paste of pages.
     *
     * For all pages that are specified also the master slide has to be specified.
     */
    bool saveOasisPages( KoPASavingContext & paContext, QList<KoPAPageBase *> &pages, QList<KoPAPageBase *> &masterPages );

    /**
     * Get page by index.
     *
     * @param index of the page
     * @param masterPage if true return a masterPage, if false a normal page
     */
    KoPAPageBase* pageByIndex( int index, bool masterPage ) const;

    /// reimplemnted
    virtual int pageCount() const;

    /**
     * Get the index of the page
     *
     * @param page The page you want to get the index for
     *
     * @return The index of the page or -1 if the page is not found
     */
    int pageIndex( KoPAPageBase * page ) const;

    /**
     * Get page by navigation
     *
     * @param currentPage the current page
     * @param pageNavigation how to navigate from the current page
     *
     * @return the page which is reached by pageNavigation
     */
    KoPAPageBase* pageByNavigation( KoPAPageBase * currentPage, KoPageApp::PageNavigation pageNavigation ) const;

    /**
     * Insert page to the document at index
     *
     * The function checks if it is a normal or a master page and puts it in 
     * the correct list.
     *
     * @param page to insert to document
     * @param index where the page will be inserted.
     */
    void insertPage( KoPAPageBase* page, int index );

    /**
     * Insert @p page to the document after page @p before
     *
     * The function checks if it is a normal or a master page and puts it in 
     * the correct list.
     *
     * @param page to insert to document
     * @param after the page which the inserted page should come after. Set after to 0 to add at the beginning
     */
    void insertPage( KoPAPageBase* page, KoPAPageBase* after );

    /**
     * Take @page from the page
     *
     * @param page taken from the document
     * @return the position of the page was taken from the document, or -1 if the page was not found
     */
    int takePage( KoPAPageBase *page );

    void addShape( KoShape *shape );
    void removeShape( KoShape* shape );

    QList<KoPAPageBase*> pages( bool masterPages = false ) const;

    /**
     * Get a new page for inserting into the document
     *
     * The page is created with new.
     *
     * Reimplement when you need a derivered class in your kopageapplication 
     */
    virtual KoPAPage * newPage( KoPAMasterPage * masterPage = 0 );

    /**
     * Get a new master page for inserting into the document
     *
     * The page is created with new.
     *
     * Reimplement when you need a derivered class in your kopageapplication 
     */
    virtual KoPAMasterPage * newMasterPage();

    /**
     * Get the type of the document
     */
    virtual KoOdf::DocumentType documentType() const = 0;


    /// return the inlineTextObjectManager for this document.
    KoInlineTextObjectManager *inlineTextObjectManager() const;

    void setRulersVisible(bool visible);
    bool rulersVisible() const;

    QMap<QString, KoDataCenter *>  dataCenterMap(  );

    /**
     * Get the page on which the shape is located
     *
     * @param shape The shape for which the page should be found
     * @return The page on which the shape is located
     */
    KoPAPageBase * pageByShape( KoShape * shape ) const;

    void updateViews(KoPAPageBase *page = 0);

signals:
    void shapeAdded(KoShape* shape);
    void shapeRemoved(KoShape* shape);
    void pageAdded(KoPAPageBase* page);
    void pageRemoved(KoPAPageBase* page);

protected:
    virtual KoView *createViewInstance( QWidget *parent ) = 0;
    virtual const char *odfTagName( bool withNamespace ) = 0;

    /**
     * This function is called by at the end of addShape. This is used 
     * e.g. for doing work on the application which is in the KoShapeAppData.
     *
     * The default impementation does nothing
     */
    virtual void postAddShape( KoPAPageBase * page, KoShape * shape );

    /**
     * This function is called by at the end of removeShape. This is used 
     * e.g. for doing work on the application which is in the KoShapeAppData.
     *
     * The default impementation does nothing
     */
    virtual void postRemoveShape( KoPAPageBase * page, KoShape * shape );

    /**
     * @brief Enables/Disables the given actions in all views
     *
     * The actions are of Type KoPAAction
     *
     * @param actions which should be enabled/disabled
     * @param enable new state of the actions
     */
    void setActionEnabled( int actions, bool enable );

    /// Load the configuration
    void loadConfig();
    /// Save the configuration
    void saveConfig();

private:
    class Private;
    Private * const d;
};

#endif /* KOPADOCUMENT_H */
