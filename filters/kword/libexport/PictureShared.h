/* This file is part of the KDE project
   Copyright (c) 2001 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002, 2003, 2004 Nicolas GOUTTE <goutte@kde.org>

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
#ifndef KO_PICTURE_SHARED_H
#define KO_PICTURE_SHARED_H

#include <QtCore/QString>
#include <QtCore/QIODevice>
#include <QtGui/QPixmap>

#include "PictureKey.h"

class KXmlWriter;
class QPainter;
class QSize;
class QMimeData;

class PictureBase;

struct Shared
{
    Shared() : count(1) {}
    void ref() { ++count; }
    bool deref() { return !--count; }
    uint count;
};

#include "kword_libexport_export.h"
/**
 * @internal
 * PictureShared is the class that contains the shared part for Picture
 */
class KWORD_LIBEXPORT_EXPORT PictureShared : public Shared
{
public:
    /**
     * Default constructor.
     */
    PictureShared();

    /**
     * Destructor.
     */
    ~PictureShared(void);

    /**
     * @brief Copy constructor
     *
     * This makes a deep copy. Do not use if you want to share!
     */
    PictureShared(const PictureShared &other);

    /**
     * @brief Assignment operator
     *
     * This makes a deep copy. Do not use if you want to share!
     */
    PictureShared& operator=(const PictureShared& other);

    PictureType::Type getType(void) const;

    /**
     * Returns true if the picture is null.
     */
    bool isNull(void) const;

    /**
     * @brief Draw the image in a painter.
     *
     * The parameter @p fastMode allows the picture to be re-sized and drawn quicker if possible
     *
     * The parameters @p width, @p height define the desired size for the picture
     *
     * The other parameters are very similar to QPainter::drawPixmap :
     * (@p x, @p y) define the position in the painter,
     * (@p sx, @p sy) specify the top-left point in pixmap that is to be drawn. The default is (0, 0).
     * (@p sw, @p sh) specify the size of the pixmap that is to be drawn. The default, (-1, -1), means all the way to the bottom
     * right of the pixmap.
     */
    void draw(QPainter& painter, int x, int y, int width, int height, int sx = 0, int sy = 0, int sw = -1, int sh = -1, bool fastMode = false);

    /**
     * Create a dragobject containing this picture.
     * @param dragSource must be 0 when copying to the clipboard
     * @return 0 if the picture is null!
     */
    QMimeData* dragObject(QWidget *dragSource = 0, const char *name = 0);

    bool load(QIODevice* io, const QString& extension);
    bool loadFromBase64(const QByteArray& str);

    /**
     * Save picture into a QIODevice
     * @param io QIODevice used for saving
     */
    bool save(QIODevice* io) const;

    /**
     * OASIS FlatXML support:
     * Save picture as base64-encoded data into an XML writer.
     */
    bool saveAsBase64(KXmlWriter& writer) const;

    void setExtension(const QString& extension);

    QString getExtension(void) const;

    QSize getOriginalSize(void) const;

    /**
     * Clear and set the mode of this PictureShared
     *
     * @param newMode file extension (like "png") giving the wanted mode
     */
    void clearAndSetMode(const QString& newMode);

    /**
     * Reset the PictureShared (but not the key!)
     */
    void clear(void);

    /**
     * Load a file
     *
     * @param fileName the name of the file to load
     */
    bool loadFromFile(const QString& fileName);

    /**
     * Load a potentially broken XPM file (for showcase)
     */
    bool loadXpm(QIODevice* io);

    /**
     * @deprecated
     * Returns a QPixmap from an image
     *
     * @param size the wanted size for the QPixmap
     */
    QPixmap generatePixmap(const QSize& size, bool smoothScale = false);

    QString getMimeType(void) const;

    /**
     * Generate a QImage
     *
     * (always in slow mode)
     *
     * @param size the wanted size for the QImage
     */
    QImage generateImage(const QSize& size);

    bool hasAlphaBuffer() const;
    void setAlphaBuffer(bool enable);
    QImage createAlphaMask(Qt::ImageConversionFlags flags = Qt::AutoColor) const;

    /**
     * Clear any cache
     *
     * It is used to avoid using too much memory
     * especially if the application somehow caches the Picture too.
     */
    void clearCache(void);

    QString uniquePictureId() const;
    void assignPictureId(uint _id);

protected:
    /**
     * @internal
     * Loads a temporary file, probably from a downloaded file
     */
    bool loadTmp(QIODevice* io);

    /// Find type of image, create base accordingly, and load data
    bool identifyAndLoad(const QByteArray& data);

    /**
     * @internal
     * Loads a compressed file
     *
     * @warning risk of endless recurision, be careful when it is called from @see load
     *
     * @param io QIODevice of the compressed file/strea,
     * @param mimeType mimetype of the (de-)compressor
     * @param extension extension of the uncompressed file
     */
    bool loadCompressed(QIODevice* io, const QString& mimeType, const QString& extension);

protected:
    PictureBase* m_base;
    QString m_extension;
    uint m_pictureId;
};

#endif /* KO_PICTURE_SHARED_H */
