/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2011 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KReplaceStrategy_p.h"

#include <QTextCursor>
#include <QTextDocument>
#include <KFind>
#include <KReplaceDialog>
#include <KMessageBox>
#include <KLocale>

#include "FindDirection_p.h"

KReplaceStrategy::KReplaceStrategy(QWidget * parent)
        : m_dialog(new KReplaceDialog(parent))
        , m_replaced(0)
{
    m_dialog->setOptions(KFind::FromCursor);
}

KReplaceStrategy::~KReplaceStrategy()
{
    if (m_dialog->parent()==0)
        delete m_dialog;
}

KFindDialog *KReplaceStrategy::dialog()
{
    return m_dialog;
}

void KReplaceStrategy::reset()
{
    m_replaced = 0;
}

void KReplaceStrategy::displayFinalDialog()
{
    if (m_replaced == 0) {
        KMessageBox::information(m_dialog->parentWidget(), i18n("Found no match\n\nNo text was replaced"));
    } else {
        KMessageBox::information(m_dialog->parentWidget(),
                                 i18np("1 replacement made",
                                       "%1 replacements made", m_replaced));
    }
    reset();
}

bool KReplaceStrategy::foundMatch(QTextCursor &cursor, FindDirection *findDirection)
{
    bool replace = true;
    if ((m_dialog->options() & KReplaceDialog::PromptOnReplace) != 0) {
        findDirection->select(cursor);
        // TODO: not only Yes and No, but Yes, No, All and Cancel
        int value = KMessageBox::questionYesNo(m_dialog->parentWidget(),
                                               i18n("Replace %1 with %2?", m_dialog->pattern(), m_dialog->replacement()));
        if (value != KMessageBox::Yes) {
            replace = false;
        }
    }

    if (replace) {
        cursor.insertText(m_dialog->replacement());
        ++m_replaced;
        int pos = cursor.position() + m_dialog->replacement().length();;
        if (cursor.document()->characterCount() <= pos)
            return false;
        cursor.setPosition(pos);
    }

    return true;
}
