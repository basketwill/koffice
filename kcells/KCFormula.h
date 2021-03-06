/* This file is part of the KDE project
   Copyright (C) 2003,2004 Ariya Hidayat <ariya@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KC_FORMULA
#define KC_FORMULA

#include <QHash>
#include <QSharedDataPointer>
#include <QString>
#include <QTextStream>
#include <QVariant>
#include <QVector>
#include <QPoint>

#include "kcells_export.h"
#include "KCCell.h"

#define KCELLS_INLINE_ARRAYS

class KLocale;

class KCSheet;
class KCValue;
typedef QHash<KCCell, KCCell> CellIndirection;

/**
 * \ingroup KCValue
 * A formula token.
 */
class KCELLS_EXPORT KCToken
{
public:
    /**
     * token types
     */
    enum Type {
        Unknown = 0, ///< unknown type
        Boolean,     ///< True, False (also i18n-ized)
        Integer,     ///< 14, 3, 1977
        Float,       ///< 3.141592, 1e10, 5.9e-7
        String,      ///< "KOffice", "The quick brown fox..."
        Operator,    ///< +, *, /, -
        KCCell,        ///< $A$1, F4, Sheet2!B5, 'Sales Forecast'!Sum
        Range,       ///< C1:C100
        Identifier,  ///< function name or named area
        Error        ///< error, like #REF!, #VALUE!, ...
    };

    /**
     * operator types
     */
    enum Op {
        InvalidOp = 0,  ///< invalid operator
        Plus,           ///<  + (addition)
        Minus,          ///<  - (subtraction, negation)
        Asterisk,       ///<  * (multiplication)
        Slash,          ///<  / (division)
        Caret,          ///<  ^ (power)
        Intersect,      ///< " " (a space means intersection)
        LeftPar,        ///<  (
        RightPar,       ///<  )
        Comma,          ///<  ,
        Semicolon,      ///<  ; (argument separator)
        Ampersand,      ///<  & (string concat)
        Equal,          ///<  =
        NotEqual,       ///<  <>
        Less,           ///<  <
        Greater,        ///<  >
        LessEqual,      ///<  <=
        GreaterEqual,   ///<  >=
        Percent,        ///<  %
        CurlyBra,       ///<  { (array start)
        CurlyKet,       ///<  } (array end)
        Pipe,           ///<  | (array row separator)
        Union           ///<  ~ (union of ranges)
    };

    /**
     * Creates a token.
     */
    explicit KCToken(Type type = Unknown, const QString& text = QString(), int pos = -1);

    static const KCToken null;

    KCToken(const KCToken&);
    KCToken& operator=(const KCToken&);

    /**
     * Returns type of the token.
     */
    Type type() const {
        return m_type;
    }

    /**
     * Returns text associated with the token.
     *
     * If you want to obtain meaningful value of this token, instead of
     * text(), you might use asInteger(), asFloat(), asString(), sheetName(),
     * etc.
     */
    QString text() const {
        return m_text;
    }

    int pos() const {
        return m_pos;
    }

    /**
     * Returns true if token is a boolean token.
     */
    bool isBoolean() const {
        return m_type == Boolean;
    }

    /**
     * Returns true if token is a integer token.
     */
    bool isInteger() const {
        return m_type == Integer;
    }

    /**
     * Returns true if token is a floating-point token.
     */
    bool isFloat() const {
        return m_type == Float;
    }

    /**
     * Returns true if token is either integer or floating-point token.
     */
    bool isNumber() const {
        return (m_type == Integer) || (m_type == Float);
    }

    /**
     * Returns true if token is a string token.
     */
    bool isString() const {
        return m_type == String;
    }

    /**
     * Returns true if token is an operator token.
     */
    bool isOperator() const {
        return m_type == Operator;
    }

    /**
     * Returns true if token is a cell reference token.
     */
    bool isCell() const {
        return m_type == KCCell;
    }

    /**
     * Returns true if token is a range reference token.
     */
    bool isRange() const {
        return m_type == Range;
    }

    /**
     * Returns true if token is an identifier.
     */
    bool isIdentifier() const {
        return m_type == Identifier;
    }

    /**
     * Returns true if token is a error token.
     */
    bool isError() const {
        return m_type == Error;
    }

    /**
     * Returns boolean value for an boolean token.
     * For any other type of token, return value is undefined.
     */
    bool asBoolean() const;

    /**
     * Returns integer value for an integer token.
     * For any other type of token, returns 0.
     */
    qint64 asInteger() const;

    /**
     * Returns floating-point value for a floating-point token.
     * For any other type of token, returns 0.0.
     */
    double asFloat() const;

    /**
     * Returns string value for a string token.
     * For any other type of token, it returns QString().
     *
     * Note that token text for a string token still has leading and trailing
     * double-quotes, i.e for "KOffice", text() return "KOffice"
     * (with the quotes, 9 characters) while asString() only return KOffice
     * (without quotes, 7 characters).
     */
    QString asString() const;

    /**
     * Returns operator value for an operator token.
     * For any other type of token, returns KCToken::InvalidOp.
     */
    Op asOperator() const;

    /**
     * Returns string value for a error token.
     * For any other type of token, it returns QString().
     */
    QString asError() const;

    /**
     * Returns sheet name in a cell reference token.
     * For any other type of token, it returns QString().
     *
     * If the cell reference doesn't specify sheet name, an empty string
     * is returned. As example, for "Sheet1!B3" , sheetName() returns
     * "Sheet1" while for "A2" sheetName() returns "".
     *
     * When sheet name contains quotes (as if the name has spaces) like
     * in "'Sales Forecast'!F4", sheetName() returns the name
     * without the quotes, i.e "Sales Forecast" in this case.
     */
    QString sheetName() const;

    /**
     * Returns a short description of the token.
     * Should be used only to assist debugging.
     */
    QString description() const;

    /**
     * helper function: return operator of given token text
     * e.g. "*" yields Operator::Asterisk, and so on
     */
    static Op matchOperator(const QString& text);

protected:

    Type m_type;
    QString m_text;
    int m_pos;

};

/**
 * \ingroup KCValue
 * An array of formula tokens.
 *
 */
class Tokens: public QVector<KCToken>
{
public:
    Tokens(): QVector<KCToken>(), m_valid(true) {}
    bool valid() const {
        return m_valid;
    }
    void setValid(bool v) {
        m_valid = v;
    }
protected:
    bool m_valid;
};


/**
 * \ingroup KCValue
 * A formula for a cell.
 *
 * A KCFormula is a equations which perform calculations on values in the cells
 * and sheets. Every formula must start with an equal sign (=).
 *
 *
 */
class KCELLS_EXPORT KCFormula
{
public:
    /**
     * Creates a formula. It must be owned by a sheet.
     */
    KCFormula(KCSheet *sheet, const KCCell& cell);

    /**
     * Creates a formula. It must be owned by a sheet.
     */
    explicit KCFormula(KCSheet *sheet);

    /**
     * Creates a formula that is not owned by any sheet.
     * This might be useful in some cases.
     */
    KCFormula();

    /**
     * Returns a null formula object, this is quicker than creating a new one.
     */
    static KCFormula empty();

    /**
     * Copy constructor.
     */
    KCFormula(const KCFormula&);

    /**
     * Destroys the formula.
     */
    ~KCFormula();

    /**
     * Returns the cell which owns this formula.
     */
    KCSheet* sheet() const;
    /**
     * Returns the cell which owns this formula.
     */
    const KCCell& cell() const;

    /**
     * Sets the expression for this formula.
     */
    void setExpression(const QString& expr);

    /**
     * Gets the expression of this formula.
     */
    QString expression() const;

    /**
     * Clears everything, makes as like a newly constructed formula.
     */
    void clear();

    /**
     * Returns true if the specified expression is valid, i.e. it contains
     * no parsing error.
     * Empty formula (i.e. without expression) is always invalid.
     */
    bool isValid() const;

    /**
     * Returns list of tokens associated with this formula. This has nothing to
     * with the formula evaluation but might be useful, e.g. for syntax
     * highlight or similar features.
     * If the formula contains error, the returned tokens is invalid.
     */
    Tokens tokens() const;

    /**
     * Evaluates the formula and returns the result.
     * The optional cellIndirections parameter can be used to replace all
     * occurences of a references to certain cells with references to
     * different cells. If this mapping is non-empty this does mean
     * that intermediate results can't be cached.
     */
    KCValue eval(CellIndirection cellIndirections = CellIndirection()) const;

    /**
     * Given an expression, this function separates it into tokens.
     * If the expression contains error (e.g. unknown operator, string no terminated)
     * this function returns tokens which is not valid.
     */
    Tokens scan(const QString& expr, const KLocale* locale = 0) const;

    /**
     * Assignment operator.
     */
    KCFormula& operator=(const KCFormula&);

    bool operator==(const KCFormula&) const;
    inline bool operator!=(const KCFormula& o) const {
        return !operator==(o);
    }

    QString dump() const;

protected:

    void compile(const Tokens& tokens) const;

    /**
     * helper function: return true for valid named area
     */
    bool isNamedArea(const QString& expr) const;

    /**
     * helper function for recursive evaluations; makes sure one cell
     * is not evaluated more than once resulting in infinite loops
     */
    KCValue evalRecursive(CellIndirection cellIndirections, QHash<KCCell, KCValue>& values) const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

/**
 * Dumps the formula, should be used only to assist debugging.
 */
QTextStream& operator<<(QTextStream& ts, KCFormula formula);



namespace KCells
{
/**
 * helper function: return true for valid identifier character
 */
bool isIdentifier(const QChar &ch);
}

/***************************************************************************
  QHash/QSet support
****************************************************************************/

inline uint qHash(const KCFormula& formula)
{
    return qHash(formula.expression());
}

Q_DECLARE_METATYPE(KCFormula)
Q_DECLARE_TYPEINFO(KCFormula, Q_MOVABLE_TYPE);

#endif // KC_FORMULA
