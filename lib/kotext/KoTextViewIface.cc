/* This file is part of the KDE project
   Copyright (C) 2002 Laurent MONTEL <lmontel@mandrakesoft.com>

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

#include "KoTextViewIface.h"
#include "kotextview.h"
#include <kapplication.h>
#include <dcopclient.h>
#include <kdebug.h>

KoTextViewIface::KoTextViewIface( KoTextView *_textview )
    : DCOPObject()
{
   m_textView = _textview;
}

void KoTextViewIface::insertSoftHyphen()
{
   m_textView->insertSoftHyphen();
}

void KoTextViewIface::insertText( const QString &text )
{
    m_textView->insertText(text);
}


void KoTextViewIface::setBold(bool b)
{
    m_textView->setBold(b);
}

void KoTextViewIface::setItalic(bool on)
{
    m_textView->setItalic(on);
}

void KoTextViewIface::setUnderline(bool on)
{
    m_textView->setUnderline(on);
}

void KoTextViewIface::setStrikeOut(bool on)
{
    m_textView->setStrikeOut(on);
}

void KoTextViewIface::setPointSize( int s )
{
    m_textView->setPointSize(s);
}

void KoTextViewIface::setTextSubScript(bool on)
{
    m_textView->setTextSubScript(on);
}

void KoTextViewIface::setTextSuperScript(bool on)
{
    m_textView->setTextSuperScript(on);
}

void KoTextViewIface::setDefaultFormat()
{
    m_textView->setDefaultFormat();
}

QColor KoTextViewIface::textColor() const
{
    return m_textView->textColor();
}

QString KoTextViewIface::textFontFamily()const
{
    return m_textView->textFontFamily();
}

QColor KoTextViewIface::textBackgroundColor()const
{
    return m_textView->textBackgroundColor();
}

void KoTextViewIface::setTextColor(const QColor &color)
{
    m_textView->setTextColor(color);
}

void KoTextViewIface::setTextBackgroundColor(const QColor &color)
{
    m_textView->setTextBackgroundColor(color);
}

void KoTextViewIface::setAlign(int align)
{
    m_textView->setAlign(align);
}

void KoTextViewIface::setAlign(const QString &align)
{
    if( align=="AlignLeft")
        m_textView->setAlign(Qt::AlignLeft);
    else if (align=="AlignRight")
        m_textView->setAlign(Qt::AlignRight);
    else if (align=="AlignCenter")
        m_textView->setAlign(Qt::AlignCenter);
    else if (align=="AlignJustify")
        m_textView->setAlign(Qt::AlignJustify);
    else
    {
        kdDebug()<<"Align value don't recognize...\n";
        m_textView->setAlign(Qt::AlignLeft);
    }
}

bool KoTextViewIface::isReadWrite() const
{
    return m_textView->isReadWrite();
}

void KoTextViewIface::setReadWrite( bool b )
{
    m_textView->setReadWrite(b);
}

void KoTextViewIface::hideCursor()
{
    m_textView->hideCursor();
}

void KoTextViewIface::showCursor()
{
    m_textView->showCursor();
}
