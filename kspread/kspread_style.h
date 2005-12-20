/* This file is part of the KDE project
   Copyright (C) 2003 Norbert Andres, nandres@web.de

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

#ifndef __kspread_style__
#define __kspread_style__

#include <qbrush.h>
#include <qcolor.h>
#include <qfont.h>
#include <qpen.h>

#include <koOasisStyles.h>

#include "kspread_format.h"

class QDomDocument;
class QDomElement;
class KoGenStyles;

namespace KSpread
{
class CustomStyle;

class Style
{
 public:
  typedef enum E1 { BUILTIN, CUSTOM, AUTO, TENTATIVE } StyleType;

  enum FontFlags
    {
      FBold      = 0x01,
      FUnderline = 0x02,
      FItalic    = 0x04,
      FStrike    = 0x08
    };

  enum Properties
    {
      PDontPrintText = 0x01,
      PCustomFormat  = 0x02,
      PNotProtected  = 0x04,
      PHideAll       = 0x08,
      PHideFormula   = 0x10,
      PMultiRow      = 0x20,
      PVerticalText  = 0x40
    };

    // TODO Stefan: merge with Format::Properties
    enum FlagsSet
    {
      SAlignX          = 0x01,
      SAlignY          = 0x02,
      //SFactor was here
      SPrefix          = 0x08,
      SPostfix         = 0x10,
      SLeftBorder      = 0x20,
      SRightBorder     = 0x40,
      STopBorder       = 0x80,
      SBottomBorder    = 0x100,
      SFallDiagonal    = 0x200,
      SGoUpDiagonal    = 0x400,
      SBackgroundBrush = 0x800,
      SFont            = 0x1000,
      STextPen         = 0x2000,
      SBackgroundColor = 0x4000,
      SFloatFormat     = 0x8000,
      SFloatColor      = 0x10000,
      SMultiRow        = 0x20000,
      SVerticalText    = 0x40000,
      SPrecision       = 0x80000,
      SFormatType      = 0x100000,
      SAngle           = 0x200000,
      SComment         = 0x400000,
      SIndent          = 0x800000,
      SDontPrintText   = 0x1000000,
      SCustomFormat    = 0x2000000,
      SNotProtected    = 0x4000000,
      SHideAll         = 0x8000000,
      SHideFormula     = 0x10000000,
      SFontSize        = 0x20000000,
      SFontFlag        = 0x40000000,
      SFontFamily      = 0x80000000
    };

  Style();
  Style( Style * style );

  virtual ~Style();

    static FormatType formatType( const QString &_format );

    static QString saveOasisStyleNumeric( KoGenStyles &mainStyles, FormatType _style, const QString &_prefix, const QString &_postfix,  int _precision);
    static QString saveOasisStyleNumericDate( KoGenStyles &mainStyles, FormatType _style );
    static QString saveOasisStyleNumericFraction( KoGenStyles &mainStyles, FormatType _style, const QString &_prefix, const QString _suffix );
    static QString saveOasisStyleNumericTime( KoGenStyles& mainStyles, FormatType _style );
    static QString saveOasisStyleNumericCustom( KoGenStyles&mainStyles, FormatType _style );
    static QString saveOasisStyleNumericScientific( KoGenStyles&mainStyles, FormatType _style, const QString &_prefix, const QString _suffix, int _precision );
    static QString saveOasisStyleNumericPercentage( KoGenStyles&mainStyles, FormatType _style, int _precision );
    static QString saveOasisStyleNumericMoney( KoGenStyles&mainStyles, FormatType _style, int _precision );
    static QString saveOasisStyleNumericText( KoGenStyles&mainStyles, FormatType _style, int _precision );
    static QString saveOasisStyleNumericNumber( KoGenStyles&mainStyles, FormatType _style, int _precision );


  StyleType type() const { return m_type; }

  void saveXML( QDomDocument & doc, QDomElement & format ) const;
  bool loadXML( QDomElement & format );

    QString saveOasisStyle( KoGenStyle &style, KoGenStyles &mainStyles );
    void loadOasisStyle( KoOasisStyles& oasisStyles, const QDomElement & element );
    static QString saveOasisBackgroundStyle( KoGenStyles &mainStyles, const QBrush &brush );

  /**
   * Releases this style. The internal reference counter is decremented.
   * @return true, if this style is not used anymore and should be deleted.
   */
  bool release();
  /**
   * Marks this style as used. The internal reference counter is incremented.
   */
  void addRef();
  /**
   * @return the number of references to this style.
   */
  int usage() const { return m_usageCount; }

  bool   hasProperty( Properties p ) const;
  bool   hasFeature( FlagsSet f, bool withoutParent ) const;
  uint   features() const { return m_featuresSet; }

  uint bottomPenValue() const { return m_bottomPenValue; }
  uint rightPenValue() const { return m_rightPenValue; }
  uint leftPenValue() const { return m_leftPenValue; }
  uint topPenValue() const { return m_topPenValue; }

  QPen    const & pen()             const;
  QColor  const & bgColor()         const;
  QPen    const & rightBorderPen()  const;
  QPen    const & bottomBorderPen() const;
  QPen    const & leftBorderPen()   const;
  QPen    const & topBorderPen()    const;
  QPen    const & fallDiagonalPen() const;
  QPen    const & goUpDiagonalPen() const;
  QBrush  const & backGroundBrush() const;
  QString const & strFormat()       const;
  QString const & prefix()          const;
  QString const & postfix()         const;
  QString const & fontFamily()      const;

  Format::Align       alignX()      const;
  Format::AlignY      alignY()      const;
  Format::FloatFormat floatFormat() const;
  Format::FloatColor  floatColor()  const;
  FormatType  formatType()  const;

  Format::Currency const & currency() const;

  QFont  font()        const;
  uint   fontFlags()   const;
  int    fontSize()    const;
  int    precision()   const;
  int    rotateAngle() const;
  double indent()      const;

  Style * setAlignX( Format::Align  alignX );
  Style * setAlignY( Format::AlignY alignY );
  Style * setFont( QFont const & f );
  Style * setFontFamily( QString const & fam );
  Style * setFontFlags( uint flags );
  Style * setFontSize( int size );
  Style * setPen( QPen const & pen );
  Style * setBgColor( QColor const & color );
  Style * setRightBorderPen( QPen const & pen );
  Style * setBottomBorderPen( QPen const & pen );
  Style * setLeftBorderPen( QPen const & pen );
  Style * setTopBorderPen( QPen const & pen );
  Style * setFallDiagonalPen( QPen const & pen );
  Style * setGoUpDiagonalPen( QPen const & pen );
  Style * setRotateAngle( int angle );
  Style * setIndent( double indent );
  Style * setBackGroundBrush( QBrush const & brush );
  Style * setFloatFormat( Format::FloatFormat format );
  Style * setFloatColor( Format::FloatColor color );
  Style * setFormatType( FormatType format );
  Style * setStrFormat( QString const & strFormat );
  Style * setPrecision( int precision );
  Style * setPrefix( QString const & prefix );
  Style * setPostfix( QString const & postfix );
  Style * setCurrency( Format::Currency const & currency );
  Style * setProperty( Properties p );
  Style * clearProperty( Properties p );

  CustomStyle * parent() const;
  QString const & parentName() const { return m_parentName; }
  void setParent( CustomStyle * parent );

 protected:

  CustomStyle * m_parent;
  QString        m_parentName;
  StyleType      m_type;
  uint           m_usageCount;
  uint           m_featuresSet;

  /**
   * Alignment of the text
   */
  Format::Align m_alignX;
  /**
   * Aligment of the text at top middle or bottom
   */
  Format::AlignY m_alignY;

  Format::FloatFormat m_floatFormat;
  /**
   * The color format of a floating point value
   */
  Format::FloatColor m_floatColor;

  FormatType m_formatType;

  /**
   * The font used to draw the text
   */
  QString   m_fontFamily;
  uint      m_fontFlags;
  int       m_fontSize;

  /**
   * The pen used to draw the text
   */
  QPen m_textPen;
  /**
   * The background color
   */
  QColor m_bgColor;

  /**
   * The pen used to draw the right border
   */
  QPen m_rightBorderPen;

  /**
   * The pen used to draw the bottom border
   */
  QPen m_bottomBorderPen;

  /**
   * The pen used to draw the left border
   */
  QPen m_leftBorderPen;

  /**
   * The pen used to draw the top border
   */
  QPen m_topBorderPen;

  /**
   * The pen used to draw the diagonal
   */
  QPen m_fallDiagonalPen;
  /**
   * The pen used to draw the the diagonal which go up
   */
  QPen m_goUpDiagonalPen;

  /**
   * The brush used to draw the background.
   */
  QBrush m_backGroundBrush;

  int m_rotateAngle;
  /**
   * Give indent
   */
  double m_indent;
  /**
   * Format of the content, e.g. #.##0.00, dd/mmm/yyyy,...
   */
  QString m_strFormat;
  /**
   * The precision of the floating point representation
   * If precision is -1, this means that no precision is specified.
   */
  int m_precision;
  /**
   * The prefix of a numeric value ( for example "$" )
   * May be empty.
   */
  QString m_prefix;
  /**
   * The postfix of a numeric value ( for example "DM" )
   * May be empty.
   */
  QString m_postfix;
  /**
   * Currency information:
   * about which currency from which country
   */
  Format::Currency m_currency;

  /**
   * Stores information like: DonPrint, DontShowFormula, Protected...
   */
  uint m_properties;

  uint m_bottomPenValue;
  uint m_rightPenValue;
  uint m_leftPenValue;
  uint m_topPenValue;

  bool featureSet( FlagsSet f ) const { return ( !m_parent || ( m_featuresSet & (uint) f ) ); }
};

class CustomStyle : public Style
{
 public:
  CustomStyle( Style * parent, QString const & name );
  CustomStyle( QString const & name, CustomStyle * parent );
  ~CustomStyle();

  QString const & name()       const { return m_name;       }

  void save( QDomDocument & doc, QDomElement & styles );
    void saveOasis( KoGenStyles &mainStyles );
    void loadOasis( KoOasisStyles& oasisStyles, const QDomElement & style, const QString & name );

 bool loadXML( QDomElement const & style, QString const & name );

  void setType( StyleType type ) { m_type = type; }

  void setName( QString const & name );
  void refreshParentName();
  bool definesAll() const;

  void changeAlignX( Format::Align  alignX );
  void changeAlignY( Format::AlignY alignY );
  void changeFont( QFont const & f );
  void changeFontFamily( QString const & fam );
  void changeFontSize( int size );
  void changeFontFlags( uint flags );
  void changePen( QPen const & pen );
  void changeTextColor( QColor const & color );
  void changeBgColor( QColor const & color );
  void changeRightBorderPen( QPen const & pen );
  void changeBottomBorderPen( QPen const & pen );
  void changeLeftBorderPen( QPen const & pen );
  void changeTopBorderPen( QPen const & pen );
  void changeFallBorderPen( QPen const & pen );
  void changeGoUpBorderPen( QPen const & pen );
  void changeRotateAngle( int angle );
  void changeIndent( double indent );
  void changeBackGroundBrush( QBrush const & brush );
  void changeFloatFormat( Format::FloatFormat format );
  void changeFloatColor( Format::FloatColor color );
  void changeFormatType( FormatType format );
  void changeStrFormat( QString const & strFormat );
  void changePrecision( int precision );
  void changePrefix( QString const & prefix );
  void changePostfix( QString const & postfix );
  void changeCurrency( Format::Currency const & currency );

  void addProperty( Properties p );
  void removeProperty( Properties p );

 private:
  friend class StyleManager;

  QString              m_name;

  CustomStyle();
};

} // namespace KSpread

#endif
