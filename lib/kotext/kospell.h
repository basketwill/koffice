/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>
   Copyright (C) 2004 Zack Rusin <zack@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KOSPELL_H
#define KOSPELL_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_LIBKSPELL2

#include <qobject.h>
#include <qstringlist.h>
#include <qstring.h>

#include <kspell2/backgroundchecker.h>

class KoTextIterator;
class KoTextParag;
class KoTextObject;

/**
 * KOffice spell checking object
 *
 * @author Zack Rusin <zack@kde.org>, David Sweet <dsweet@kde.org>
 * @see KSpell2::Broker
 */
class KoSpell : public KSpell2::BackgroundChecker
{
    Q_OBJECT

public:
    KoSpell( const KSpell2::Broker::Ptr& broker, QObject *parent =0,
             const char *name =0 );
    /**
     * The destructor instructs ISpell/ASpell to write out the personal
     *  dictionary and then terminates ISpell/ASpell.
     */
    virtual ~KoSpell();

    /**
     * Returns whether the speller is already checking something.
     */
    bool checking() const;

    /**
     * Spellchecks a buffer of many words in plain text
     * format.
     *
     * The @p _buffer is not modified.  The signal @ref done() will be
     * sent when @ref check() is finished.
     */
    virtual bool check( KoTextIterator *itr, bool dialog = false );
    virtual bool check( KoTextParag *parag );
    virtual bool checkWordInParagraph( KoTextParag *parag, int pos,
                                       QString& word, int& start );

    KoTextParag  *currentParag() const;
    KoTextObject *currentTextObject() const;
    int currentStartIndex() const;

signals:
    /**
     * Emitted after a paragraph has been checked.
     */
    void paragraphChecked( KoTextParag* );

    void aboutToFeedText();

protected:
    virtual QString getMoreText();
    virtual void finishedCurrentFeed();

private:
    class Private;
    Private *d;
};
#endif
#endif
