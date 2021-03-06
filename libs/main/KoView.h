/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>

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
#ifndef KO_VIEW_H
#define KO_VIEW_H

#include <QtGui/QWidget>
#include <kparts/part.h>
#include "komain_export.h"

class KoDocument;
class KoMainWindow;
class KoPrintJob;
class KoViewPrivate;
class KoZoomController;

// KDE classes
class KStatusBar;
class KXmlGuiWindow;
class KAction;

// Qt classes
class QToolBar;

/**
 * This class is used to display a @ref KoDocument.
 *
 * Multiple views can be attached to one document at a time.
 */
class KOMAIN_EXPORT KoView : public QWidget, public KParts::PartBase
{
    Q_OBJECT

public:
    /**
     * Creates a new view for the document. Usually you don't create views yourself
     * since the KOffice components come with their own view classes which inherit
     * KoView.
     *
     * The standard way to retrieve a KoView is to call @ref KoDocument::createView.
     *
     * @param document is the document which should be displayed in this view. This pointer
     *                 must not be zero.
     * @param parent   parent widget for this view.
     */
    explicit KoView(KoDocument *document, QWidget *parent = 0);
    /**
     * Destroys the view and unregisters at the document.
     */
    virtual ~KoView();

    /**
     *  Retrieves the document object of this view.
     */
    KoDocument *koDocument() const;

    /**
     * Tells this view that its document has got deleted (called internally)
     */
    void setDocumentDeleted();
    /**
     * @return true if the document has already got deleted.
     * This can be useful for the view destructor to know if it can
     * access the document or not.
     */
    bool documentDeleted() const;

    virtual void setPartManager(KParts::PartManager *manager);
    virtual KParts::PartManager *partManager() const;

    /**
     * Returns the action described action object. In fact only the "name" attribute
     * of @p element is of interest here. The method searches in the
     * KActionCollection of this view.
     *
     * Please notice that KoView indirectly inherits KXMLGUIClient.
     *
     * @see KXMLGUIClient
     * @see KXMLGUIClient::actionCollection
     * @see KoDocument::action
     */
    virtual QAction *action(const QDomElement &element) const;

    /**
     * Returns the action with the given name. The method searches in the
     * KActionCollection of this view.
     *
     * Please notice that KoView indirectly inherits KXMLGUIClient.
     *
     * @see KXMLGUIClient
     * @see KXMLGUIClient::actionCollection
     * @see KoDocument::action
     */
    virtual QAction *action(const char* name) const;

    /**
     *  Retrieves the document that is hit. This can be an embedded document.
     *
     *  The default implementation asks @ref KoDocument::hitTest. This
     *  will iterate over all child documents to detect a hit.
     *
     *  If your koffice component has multiple pages, like for example KCells, then the hittest
     *  may not succeed for a child that is not on the visible page. In those
     *  cases you need to reimplement this method.
     */
    virtual KoDocument *hitTest(const QPoint &pos);

    /**
     * Retrieves the left border width that is displayed around the content if
     * the view is active.
     *
     * In a spread sheet this border is for example used to display the
     * rows, while a top border is used to display the names of the cells
     * and a right and bottom border is used to display scrollbars. If the view
     * becomes inactive, then this stuff is not displayed anymore.
     *
     * The default border is 0.
     */
    virtual int leftBorder() const;
    /**
     * @see #leftBorder
     */
    virtual int rightBorder() const;
    /**
     * @see #leftBorder
     */
    virtual int topBorder() const;
    /**
     * @see #leftBorder
     */
    virtual int bottomBorder() const;

    /**
     * Overload this function if the content will be displayed
     * on some child widget instead of the view directly.
     *
     * By default this function returns a pointer to the view.
     */
    virtual QWidget *canvas() const;

    /**
     * Overload this function if the content will be displayed
     * with an offset relative to the upper left corner
     * of the canvas widget.
     *
     * By default this function returns 0.
     */
    virtual int canvasXOffset() const;

    /**
     * Overload this function if the content will be displayed
     * with an offset relative to the upper left corner
     * of the canvas widget.
     *
     * By default this function returns 0.
     */
    virtual int canvasYOffset() const;

    /**
     * Sets up so that autoScroll signals are emitted when the mouse pointer is outside the view
     */
    void enableAutoScroll();

    /**
     * Stops the emitting of autoScroll signals
     */
    void disableAutoScroll();

    /**
     * calls KoDocument::paintEverything()
     */
    virtual void paintEverything(QPainter &painter, const QRect &rect);

    /**
     * In order to print the document represented by this view a new print job should
     * be constructed that is capable of doing the printing.
     * The default implementation returns 0, which silently cancels printing.
     */
    virtual KoPrintJob * createPrintJob();

    /**
     * In order to export the document represented by this view a new print job should
     * be constructed that is capable of doing the printing.
     * The default implementation call createPrintJob.
     */
    virtual KoPrintJob * createPdfPrintJob();

    /**
     * @return the KoMainWindow in which this view is currently.
     * WARNING: this could be 0, if the main window isn't a koffice main window.
     * (e.g. it can be any KParts application).
     */
    KoMainWindow * shell() const;

    /**
     * @return the KXmlGuiWindow in which this view is currently.
     * This one should never return 0, in a KDE app.
     */
    KXmlGuiWindow* mainWindow() const;

    /**
     * @return the statusbar of the KoMainWindow in which this view is currently.
     * WARNING: this could be 0, if the main window isn't a koffice main window.
     * (e.g. it can be any KParts application).
     */
    KStatusBar * statusBar() const;

    /**
     * This adds a widget to the statusbar for this view.
     * If you use this method instead of using statusBar() directly,
     * KoView will take care of removing the items when the view GUI is deactivated
     * and readding them when it is reactivated.
     * The parameters are the same as QStatusBar::addWidget().
     *
     * Note that you can't use KStatusBar methods (inserting text items by id).
     * But you can create a KStatusBarLabel with a dummy id instead, and use
     * it directly, to get the same look and feel.
     */
    void addStatusBarItem(QWidget * widget, int stretch = 0, bool permanent = false);

    /**
     * Remove a widget from the statusbar for this view.
     */
    void removeStatusBarItem(QWidget * widget);

    /**
     * Show or hide all statusbar items. Used by KoMainWindow during saving.
     */
    void showAllStatusBarItems(bool show);

    /**
     * You have to implement this method and disable/enable certain functionality (actions for example) in
     * your view to allow/disallow editing of the document.
     */
    virtual void updateReadWrite(bool readwrite) = 0;

    /**
     * Return the zoomController for this view.
     */
    virtual KoZoomController *zoomController() const = 0;

    /**
     * @return the view bar. The bar is created only if this function is called.
     */
    QToolBar* viewBar();

    /// create a list of actions that when activated will change the unit on the document.
    QList<QAction*> createChangeUnitActions();

public slots:
    /**
     * Slot to create a new view around the contained @ref #koDocument.
     */
    virtual void newView();

    /**
     * Display a message in the status bar (calls QStatusBar::message())
     * @todo rename to something more generic
     */
    void slotActionStatusText(const QString &text);

    /**
     * End of the message in the status bar (calls QStatusBar::clear())
     * @todo rename to something more generic
     */
    void slotClearStatusText();

protected:
    /**
     * This method handles three events: KParts::PartActivateEvent, KParts::PartSelectEvent
     * and KParts::GUIActivateEvent.
     * The respective handlers are called if such an event is found.
     */
    virtual void customEvent(QEvent *ev);

    /**
     * Handles the event KParts::PartActivateEvent.
     */
    virtual void partActivateEvent(KParts::PartActivateEvent *event);
    /**
     * Handles the event KParts::PartSelectEvent.
     */
    virtual void partSelectEvent(KParts::PartSelectEvent *event);
    /**
     * Handles the event KParts::GUIActivateEvent.
     */
    virtual void guiActivateEvent(KParts::GUIActivateEvent *);


    /**
       Generate a name for this view.
    */
    QString newObjectName();

signals:
    void activated(bool active);
    void selected(bool select);

    void autoScroll(const QPoint &scrollDistance);

    void regionInvalidated(const QRegion &region, bool erase);

    void invalidated();

// KDE invents public signals :)
#undef signals
#define signals public
signals:

    /**
      * Make it possible for plugins to request
      * the embedding of an image into the current
      * document. Used e.g. by the scan-plugin
    */
    void embedImage(const QString &filename);

#undef signals
#define signals protected

protected slots:
    virtual void slotAutoScroll();

private:
    virtual void setupGlobalActions(void);
    KoViewPrivate * const d;
    int autoScrollAcceleration(int offset) const;

};

#endif // KO_VIEW_H
