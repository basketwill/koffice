/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#ifndef kotextobject_h
#define kotextobject_h

#include <qrichtext_p.h>
#include <koChangeCaseDia.h>
#include <kostyle.h>
class KCommand;
class KoTextFormat;
//#define TIMING_FORMAT
//#include <qdatetime.h>

/**
 * The KoTextFormatInterface is a pure interface that allows access to the
 * "current text format". This is implemented by both KoTextObject and KoTextView.
 * For KoTextView, it's the format under the cursor.
 * For KoTextObject, it's the global format.
 * By changing this format and calling setFormat (with the appropriate flags),
 * it's possible to implement "setBold", "setItalic" etc. only once, whether it applies
 * to a text selection or to complete text objects.
 */
class KoTextFormatInterface
{
public:
    KoTextFormatInterface() {}

    /** Interface for accessing the current format */
    virtual KoTextFormat * currentFormat() const = 0;

    virtual bool rtl() const = 0;

    /** Interface for setting the modified format */
    virtual KCommand *setFormatCommand( KoTextFormat *format, int flags, bool zoomFont = false ) = 0;

    /** Interface for accessing the current parag layout */
    virtual const KoParagLayout * currentParagLayoutFormat() const = 0;

    /** Interface for changing the paragraph layout.
     * @param flags one of the KoParagLayout flags
     * @param marginIndex type of margin. Only used if flags==KoParagLayout::Margins
     */
    virtual KCommand *setParagLayoutFormatCommand( KoParagLayout *newLayout, int flags, int marginIndex=-1) = 0;

    virtual KCommand *setChangeCaseOfTextCommand(KoChangeCaseDia::TypeOfCase _type)=0;

    KoTextDocCommand *deleteTextCommand( KoTextDocument *textdoc, int id, int index, const QMemArray<KoTextStringChar> & str, const CustomItemsMap & customItemsMap, const QValueList<KoParagLayout> & oldParagLayouts );

    void setParagLayoutFormat( KoParagLayout *newLayout,int flags, int marginIndex=-1);
    void setFormat( KoTextFormat * newFormat, int flags, bool zoomFont = false );

    // Warning: use the methods that return a command! The others just leak the commands away
    //void setBold(bool on);
    KCommand *setBoldCommand(bool on);
    //void setItalic(bool on);
    KCommand *setItalicCommand(bool on);
    //void setUnderline(bool on);
    KCommand *setUnderlineCommand(bool on);
    //void setStrikeOut(bool on);
    KCommand *setDoubleUnderlineCommand( bool on );
    KCommand *setUnderlineColorCommand( const QColor &color );
    KCommand *setStrikeOutCommand(bool on);
    //void setTextColor(const QColor &color);
    KCommand *setTextColorCommand(const QColor &color);
    //void setPointSize( int s );
    KCommand *setPointSizeCommand( int s );
    //void setFamily(const QString &font);
    KCommand *setFamilyCommand(const QString &font);
    //void setFont(const QFont &font, bool _subscript, bool _superscript, const QColor &col, const QColor &backGroundColor, int flags);
    KCommand *setFontCommand(const QFont &font, bool _subscript, bool _superscript,  const QColor &col, const QColor &backGroundColor, const QColor &underlineColor, KoTextFormat::UnderlineLineStyle _underlineLineStyle, KoTextFormat::UnderlineLineType _underlineType, KoTextFormat::StrikeOutLineType _strikeOutType, KoTextFormat::StrikeOutLineStyle _strikeOutStyle, KoTextFormat::AttributeStyle _fontAttribute, bool _shadowText, double _relativeTextSize, int _offsetFromBaseLine, bool _wordByWord, const QString &_lang, int flags);
    //void setTextSubScript(bool on);
    KCommand *setTextSubScriptCommand(bool on);
    //void setTextSuperScript(bool on);
    KCommand *setTextSuperScriptCommand(bool on);

    //void setDefaultFormat();
    KCommand *setDefaultFormatCommand();

    //void setTextBackgroundColor(const QColor &);
    KCommand *setTextBackgroundColorCommand(const QColor &);

    //void setAlign(int align);
    KCommand *setAlignCommand(int align);

    //void setMargin(QStyleSheetItem::Margin m, double margin);
    KCommand *setMarginCommand(QStyleSheetItem::Margin m, double margin);

    //void setTabList(const KoTabulatorList & tabList );
    KCommand *setTabListCommand(const KoTabulatorList & tabList );

    //void setCounter(const KoParagCounter & counter );
    KCommand *setCounterCommand(const KoParagCounter & counter );

    KCommand *setLanguageCommand(const QString &);

    KCommand *setShadowTextCommand( bool _b );

    KCommand *setHyphenationCommand( bool _b );


    KCommand *setFontAttributeCommand( KoTextFormat::AttributeStyle _att);

    KCommand *setRelativeTextSizeCommand( double _size );

    KCommand *setOffsetFromBaseLineCommand( int _offset );

    KCommand *setWordByWordCommand( bool _b );


    QColor textColor() const;
    QFont textFont() const;
    QString textFontFamily()const;
    QString language() const;
    QColor textBackgroundColor()const;
    QColor textUnderlineColor()const;

    KoTextFormat::UnderlineLineType underlineLineType()const;
    KoTextFormat::StrikeOutLineType strikeOutLineType()const;
    KoTextFormat::UnderlineLineStyle underlineLineStyle()const;
    KoTextFormat::StrikeOutLineStyle strikeOutLineStyle()const;



    bool textUnderline()const;
    bool textDoubleUnderline()const;

    bool textBold()const;
    bool textStrikeOut()const;
    bool textItalic() const;
    bool textSubScript() const;
    bool textSuperScript() const;
    bool textShadow() const;
    KoTextFormat::AttributeStyle fontAttribute() const;
    double relativeTextSize() const;
    int offsetFromBaseLine()const;
    bool wordByWord()const;
};

/**
 * The KoTextObject is the high-level object that contains a KoTextDocument
 * (the list of paragraphs), and takes care of the operations on it (particularly
 * the undo/redo commands).
 * Editing the text isn't done by KoTextObject but by KoTextView (document/view design).
 */
class KoTextObject : public QObject, public KoTextFormatInterface
{
    Q_OBJECT
public:
    /** Constructor.
     * @param the zoom handler (to be passed to the KoTextDocument ctor)
     * @param defaultFont the font to use by default (@see KoTextFormatCollection)
     * @param defaultStyle the style to use by default (initial pararaph, and when deleting a used style)
     * This constructor creates the contained KoTextDocument automatically.
     */
    KoTextObject( KoZoomHandler *zh, const QFont& defaultFont, const QString &defaultLanguage, bool hyphen, KoStyle* defaultStyle,int _tabStopWidth = -1,
                  QObject* parent = 0, const char *name = 0 );

    /** Alternative constructor.
     * @param textdoc the text document to use in this text object. Ownership is transferred
     * to the text object.
     * @param defaultStyle the style to use by default (initial pararaph, and when deleting a used style)
     * This constructor allows to use a derived class from KoTextDocument.
     */
    KoTextObject( KoTextDocument *textdoc, KoStyle* defaultStyle,
                  QObject* parent = 0, const char *name = 0 );

    virtual ~KoTextObject();


    void setNeedSpellCheck(bool b);
    bool needSpellCheck() const { return m_needsSpellCheck;}
    void setProtectContent(bool b) { m_protectContent = b; }
    bool protectContent() const{ return m_protectContent;}
    /**
     * Return the text document contained in this KoTextObject
     */
    KoTextDocument *textDocument() const { return textdoc; }

    void setAvailableHeight( int avail ) { m_availableHeight = avail; }
    int availableHeight() const;

    void undo();
    void redo();
    /** Terminate our current undo/redo info, to start with a new one */
    void clearUndoRedoInfo();

    /** return true if some text is selected */
    bool hasSelection() const { return textdoc->hasSelection( KoTextDocument::Standard, true ); }
    /** returns the selected text [without formatting] if hasSelection() */
    QString selectedText( int selectionId = KoTextDocument::Standard ) const {
        return textdoc->selectedText( selectionId );
    }
    /** returns true if the given selection has any custom item in it */
    bool selectionHasCustomItems( int selectionId = KoTextDocument::Standard ) const;

    /**
     * The main "insert" method, including undo/redo creation/update.
     * @param cursor the insertion point
     * @param currentFormat the current textformat, to apply to the inserted text
     * @param text the text to be inserted
     * @param checkNewLine if true, @p text is checked for '\n' (as a paragraph delimiter)
     * @param removeSelected whether to remove selected text before - deprecated, better
     * use @ref replaceSelectionCommand instead, to get a single undo/redo command
     * @param commandName the name to give the undo/redo command if we haven't created it already
     * @param customItemsMap the map of custom items to include in the new text
     */
    void insert( KoTextCursor * cursor, KoTextFormat * currentFormat, const QString &text,
                 bool checkNewLine, bool removeSelected, const QString & commandName,
                 CustomItemsMap customItemsMap = CustomItemsMap(),
                 int selectionId = KoTextDocument::Standard,
                 bool repaint = true );
    /**
     * Remove the text currently selected, including undo/redo creation/update.
     * @param cursor the caret position
     * @param selectionId which selection to remove (usually Standard)
     * @param cmdName the name to give the undo/redo command, if we haven't created it already
     */
    void removeSelectedText( KoTextCursor * cursor, int selectionId = KoTextDocument::Standard,
                             const QString & cmdName = QString::null, bool createUndoRedo=true  );

    KCommand * replaceSelectionCommand( KoTextCursor * cursor, const QString & replacement,
                                        int selectionId, const QString & cmdName, bool repaint = true );
    KCommand * removeSelectedTextCommand( KoTextCursor * cursor, int selectionId, bool repaint = true );
    KCommand* insertParagraphCommand( KoTextCursor * cursor );

    void pasteText( KoTextCursor * cursor, const QString & text, KoTextFormat * currentFormat, bool removeSelected );
    void selectAll( bool select );

    /** Highlighting support (for search/replace, spellchecking etc.).
     * Don't forget to ensure the paragraph is visible.
     */
    void highlightPortion( KoTextParag * parag, int index, int length, bool repaint );
    void removeHighlight( bool repaint );

    /** Implementation of setFormatCommand from KoTextFormatInterface - apply change to the whole document */
    KCommand *setFormatCommand( KoTextFormat *format, int flags, bool zoomFont = false );

    /** Set format changes on selection or current cursor.
        Returns a command if the format was applied to a selection */
    KCommand *setFormatCommand( KoTextCursor * cursor, KoTextFormat ** currentFormat, KoTextFormat *format, int flags, bool zoomFont = false, int selectionId = KoTextDocument::Standard );

    /** Selections ids */
    enum SelectionIds {
        HighlightSelection = 1 // used to highlight during search/replace
    };

    enum KeyboardAction { // keep in sync with QTextEdit
	ActionBackspace,
	ActionDelete,
	ActionReturn,
	ActionKill
    };
    /** Executes keyboard action @p action. This is normally called by
     * a key event handler. */
    void doKeyboardAction( KoTextCursor * cursor, KoTextFormat * & currentFormat, KeyboardAction action );

    // -- Paragraph settings --
    KCommand * setCounterCommand( KoTextCursor * cursor, const KoParagCounter & counter, int selectionId = KoTextDocument::Standard );
    KCommand * setAlignCommand( KoTextCursor * cursor, int align , int selectionId = KoTextDocument::Standard);
    KCommand * setLineSpacingCommand( KoTextCursor * cursor, double spacing, KoParagLayout::spacingType _type,int selectionId = KoTextDocument::Standard );
    KCommand * setBordersCommand( KoTextCursor * cursor, const KoBorder& leftBorder, const KoBorder& rightBorder, const KoBorder& topBorder, const KoBorder& bottomBorder, int selectionId = KoTextDocument::Standard );
    KCommand * setMarginCommand( KoTextCursor * cursor, QStyleSheetItem::Margin m, double margin, int selectionId = KoTextDocument::Standard);
    KCommand* setTabListCommand( KoTextCursor * cursor,const KoTabulatorList & tabList , int selectionId = KoTextDocument::Standard );

    KCommand * setShadowCommand( KoTextCursor * cursor,double dist, short int direction, const QColor &col,int selectionId= KoTextDocument::Standard  );
    KCommand * setParagDirectionCommand( KoTextCursor * cursor, QChar::Direction d, int selectionId = KoTextDocument::Standard );
    KCommand * setHyphenationCommand(  KoTextCursor * cursor, bool _hyph );

    KCommand* applyStyle( KoTextCursor * cursor, const KoStyle * style,
                     int selectionId = KoTextDocument::Standard,
                     int paragLayoutFlags = KoParagLayout::All, int formatFlags = KoTextFormat::Format,
                     bool createUndoRedo = true, bool interactive = true, bool emitCommand = true );
    /** Update the paragraph that use the given style, after this style was changed.
     *  The flags tell which changes should be applied.
     *  @param paragLayoutChanged paragraph flags
     *  @param formatChanged format flags
     */
    void applyStyleChange( StyleChangeDefMap changed );
    /** Set format changes on selection or current cursor.
        Creates a command if the format was applied to a selection */
    void setFormat( KoTextCursor * cursor, KoTextFormat ** currentFormat, KoTextFormat *format, int flags, bool zoomFont = false );


    /**
     * Support for treating the whole textobject as a single object
     * Use this format for displaying the properties (font/color/...) of the object.
     * Interface for accessing the current format
     */
    virtual KoTextFormat * currentFormat() const;

    /**
     * Use this format for displaying the properties (Align/counter/...) of the object
     */
    virtual const KoParagLayout * currentParagLayoutFormat() const;

    virtual bool rtl() const;

    /**
     * Support for changing the format in the whole textobject
     */
    virtual KCommand *setParagLayoutFormatCommand( KoParagLayout *newLayout, int flags, int marginIndex=-1);

    // common for setParagLayoutFormatCommand above and KoTextView::setParagLayoutFormatCommand
    KCommand *setParagLayoutFormatCommand( KoTextCursor* cursor, int selectionId, KoParagLayout *newLayout, int flags, int marginIndex );

    /**
     * Support for changing the format in the whole textobject
     */
    virtual void setFormat( KoTextFormat * newFormat, int flags, bool zoomFont = false );

    /** Return the user-visible font size for this format (i.e. LU to pt conversion) */
    int docFontSize( KoTextFormat * format ) const;
    /** Return the font size in LU, for this user-visible font size in pt */
    int zoomedFontSize( int docFontSize ) const;

    /** Set the bottom of the view - in LU */
    void setViewArea( QWidget* w, int maxY );
    /** Make sure that @p parag is formatted */
    void ensureFormatted( KoTextParag * parag, bool emitAfterFormatting = true );
    void setLastFormattedParag( KoTextParag *parag );

    static QChar customItemChar() { return QChar( s_customItemChar ); }

    // Qt should really have support for public signals
    void emitHideCursor() { emit hideCursor(); }
    void emitShowCursor() { emit showCursor(); }
    void emitEnsureCursorVisible() { emit ensureCursorVisible(); }
    void emitUpdateUI( bool updateFormat, bool force = false ) { emit updateUI( updateFormat, force ); }

    void typingStarted();
    void typingDone();

    /**
     * Abort the current formatMore() loop, or prevent the next one from starting.
     * Use with care. This is e.g. for KWFootNoteVariable, so that it can do
     * a frame layout before formatting the main text again.
     * It is important to make sure that formatMore will be called again ;)
     */
    void abortFormatting();

    void selectionChangedNotify( bool enableActions = true );

    void emitNewCommand(KCommand *cmd);

    virtual KCommand *setChangeCaseOfTextCommand(KoChangeCaseDia::TypeOfCase _type);

    KCommand *changeCaseOfText(KoTextCursor *cursor, KoChangeCaseDia::TypeOfCase _type);
    QString textChangedCase(const QString& _text, KoChangeCaseDia::TypeOfCase _type);
    KCommand *changeCaseOfTextParag(int cursorPosStart, int cursorPosEnd,KoChangeCaseDia::TypeOfCase _type,KoTextCursor *cursor, KoTextParag *parag);

#ifndef NDEBUG
    void printRTDebug(int);
#endif

signals:
    /** Emitted by availableHeight() when the available height hasn't been
     * calculated yet or is invalid. Connect to a slot that calls setAvailableHeight() */
    void availableHeightNeeded();

    /** Emitted by formatMore() after formatting a bunch of paragraphs.
     * KWord uses this signal to check for things like 'I need to create a new page'
     */
    void afterFormatting( int bottom, KoTextParag* m_lastFormatted, bool* abort );

    /**
     * Emitted by formatMore() when formatting a "Head 1" paragraph.
     * Used for the Section variable
     */
    void chapterParagraphFormatted( KoTextParag* parag );

    /** Emitted by formatMore() when formatting the first paragraph.
     */
    void formattingFirstParag();

    /** Emitted when a new command has been created and should be added to
     * the main list of commands (usually in the KoDocument).
     * Make sure to connect to that one, otherwise the commands will just leak away...
     */
    void newCommand( KCommand *cmd );

    /** Tell the world that we'd like some repainting to happen */
    void repaintChanged( KoTextObject * );

    void hideCursor();
    void showCursor();
    /** Special hack for undo/redo - used by KoTextView */
    void setCursor( KoTextCursor * cursor );
    /** Emitted when the formatting under the cursor may have changed.
     * The Edit object should re-read settings and update the UI. */
    void updateUI( bool updateFormat, bool force = false );
    /** Same thing, when the current format (of the edit object) was changed */
    void showCurrentFormat();
    /** The views should make sure the cursor is visible */
    void ensureCursorVisible();
    /** Tell the views that the selection changed (for cut/copy...) */
    void selectionChanged( bool hasSelection );

    void showFormatObject(const KoTextFormat &);

    // Keeping track of text modifications - not emitted during loading/closing.
    void paragraphCreated( KoTextParag* parag );
    void paragraphModified( KoTextParag* parag, int /*KoTextParag::ParagModifyType*/, int pos, int length );
    void paragraphDeleted( KoTextParag* parag );

public slots:
    // The default arguments are those used by the formatTimer.
    void formatMore( int count = 10, bool emitAfterFormatting = true );

public: // made public for KWTextFrameSet...

    /** This prepares undoRedoInfo for a paragraph formatting change
     * If this does too much, we could pass an enum flag to it.
     * But the main point is to avoid too much duplicated code */
    void storeParagUndoRedoInfo( KoTextCursor * cursor, int selectionId = KoTextDocument::Standard );
    /** Copies a formatted char, <parag, position>, into undoRedoInfo.text, at position <index>. */
    void copyCharFormatting( KoTextParag *parag, int position, int index /*in text*/, bool moveCustomItems );
    void readFormats( KoTextCursor &c1, KoTextCursor &c2, bool copyParagLayouts = false, bool moveCustomItems = false );

    /**
     * The undo-redo structure holds the _temporary_ information for the current
     * undo/redo command. For instance, when typing "a" and then "b", we don't
     * want a command for each letter. So we keep adding info to this structure,
     * and when the user does something else and we call clear(), it's at that
     * point that the command is created.
     * See also the place-holder command (in fact an empty macro-command is created
     * right at the beginning, so that it's possible to undo at any time).
     */
    struct UndoRedoInfo { // borrowed from QTextEdit
        enum Type { Invalid, Insert, Delete, Return, RemoveSelected };
        UndoRedoInfo( KoTextObject* textobj );
        ~UndoRedoInfo() {}
        void clear();
        bool valid() const;

        KoTextString text; // storage for formatted text
        int id; // id of first parag
        int eid; // id of last parag
        int index; // index (for insertion/deletion)
        Type type; // type of command
        KoTextObject* textobj; // parent
        CustomItemsMap customItemsMap; // character position -> qtextcustomitem
        QValueList<KoParagLayout> oldParagLayouts;
        KoParagLayout newParagLayout;
        KoTextCursor *cursor; // basically a "mark" of the view that started this undo/redo info
        // If the view changes, the next call to checkUndoRedoInfo will terminate the previous view's edition
        KMacroCommand *placeHolderCmd;
    };
    /**
     * Creates a place holder for a command that will be completed later on.
     * This is used for the insert and delete text commands, which are
     * build delayed (see the UndoRedoInfo structure), in order to
     * have an entry in the undo/redo history asap.
     */
    void newPlaceHolderCommand( const QString & name );
    void checkUndoRedoInfo( KoTextCursor * cursor, UndoRedoInfo::Type t );

    /** for KWTextFrameSet */
    UndoRedoInfo & undoRedoInfoStruct() { return undoRedoInfo; }

    void setVisible(bool vis) { m_visible=vis; }
    bool isVisible() const { return m_visible; }

private slots:
    void doChangeInterval();
    /** This is done in a singleShot timer because of macro-commands.
     * We need to do this _after_ terminating the macro command (for instance
     * in the case of undoing a floating-frame insertion, we need to delete
     * the frame first) */
    void slotAfterUndoRedo();
    void slotParagraphModified(KoTextParag *, int, int , int);
    void slotParagraphCreated(KoTextParag *);
    void slotParagraphDeleted(KoTextParag *);
private:
    void init();

private:
    class KoTextObjectPrivate;
    KoTextObjectPrivate* d;
    /** The text document, containing the paragraphs */
    KoTextDocument *textdoc;

    /** The style to use by default (initial pararaph, and when deleting a used style)
        TODO: check that we support 0 */
    KoStyle* m_defaultStyle;

    bool m_visible;

    /** Currently built undo/redo info */
    UndoRedoInfo undoRedoInfo;

    /** All paragraphs up to this one are guaranteed to be formatted.
        The idle-time formatting (formatMore()) pushes this forward.
        Any operation on a paragraph pushes this backward. */
    KoTextParag *m_lastFormatted;
    /** Idle-time formatting */
    QTimer *formatTimer, *changeIntervalTimer;
    int interval;

    /** The total height available for our text object at the moment */
    int m_availableHeight;
    /** Store the "needs" of each view */
    QMap<QWidget *, int> m_mapViewAreas;

    //QPtrDict<int> m_origFontSizes; // Format -> doc font size.

    bool m_highlightSelectionAdded;

#ifdef TIMING_FORMAT
    QTime m_time;
#endif

    static const char s_customItemChar;
    bool m_needsSpellCheck;
    bool m_protectContent;
};

#endif
