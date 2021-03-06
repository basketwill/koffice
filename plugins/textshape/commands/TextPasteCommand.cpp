/*
 This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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
 * Boston, MA 02110-1301, USA.*/

#include "TextPasteCommand.h"

#include <KoTextEditor.h>

#include <KTextDocument.h>
#include <KTextPaste.h>
#include "TextTool.h"

#include <klocale.h>
#include <kdebug.h>

#include <QApplication>
#include <QMimeData>
#include "ChangeTrackedDeleteCommand.h"
#include "DeleteCommand.h"
#include <KAction>

#ifdef SHOULD_BUILD_RDF
# include <rdf/KoDocumentRdf.h>
#endif

TextPasteCommand::TextPasteCommand(QClipboard::Mode mode, TextTool *tool, QUndoCommand *parent)
    : QUndoCommand (parent),
    m_tool(tool),
    m_first(true),
    m_mode(mode)
{
    setText(i18n("Paste"));
}

void TextPasteCommand::undo()
{
    QUndoCommand::undo();
}

void TextPasteCommand::redo()
{
    KoTextEditor *editor = KTextDocument(m_tool->m_textShapeData->document()).textEditor();
    if (!m_first) {
        QUndoCommand::redo();
    } else {
        editor->cursor()->beginEditBlock();
        m_first = false;
        if (editor->hasSelection()) { //TODO
            if (m_tool->m_actionShowChanges->isChecked())
                editor->addCommand(new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, m_tool));
            else
                editor->addCommand(new DeleteCommand(DeleteCommand::NextChar, m_tool));
        }

        // check for mime type
        const QMimeData *data = QApplication::clipboard()->mimeData(m_mode);

        KOdf::DocumentType odfBasedType = KOdf::TextDocument;
        if (!data->hasFormat(KOdf::mimeType(odfBasedType)))
            odfBasedType = KOdf::OpenOfficeClipboard;
        if (data->hasFormat(KOdf::mimeType(odfBasedType))) {
            Soprano::Model *rdfModel = 0;
#ifdef SHOULD_BUILD_RDF
            bool weOwnRdfModel = true;
            rdfModel = Soprano::createModel();
            if (KoDocumentRdf *rdf = KoDocumentRdf::fromResourceManager(m_tool->canvas())) {
                if (rdfModel) {
                    delete rdfModel;
                }
                rdfModel = rdf->model();
                weOwnRdfModel = false;
            }
#endif

            //kDebug() << "pasting odf text";
            KTextPaste paste(m_tool->m_textShapeData, *editor->cursor(),
                              m_tool->canvas(), rdfModel);
            paste.paste(odfBasedType, data);
            //kDebug() << "done with pasting odf";

#ifdef SHOULD_BUILD_RDF
            if (KoDocumentRdf *rdf = KoDocumentRdf::fromResourceManager(m_tool->canvas())) {
                KoTextEditor *e = KoDocumentRdf::ensureTextTool(m_tool->canvas());
                rdf->updateInlineRdfStatements(e->document());
            }
            if (weOwnRdfModel && rdfModel) {
                delete rdfModel;
            }
#endif
        } else if (data->hasHtml()) {
            editor->cursor()->insertHtml(data->html());
        } else if (data->hasText()) {
            editor->cursor()->insertText(data->text());
        }
        editor->cursor()->endEditBlock();
    }
}
