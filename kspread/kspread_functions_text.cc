// built-in text functions
#include "kspread_util.h"
#include "kspread_doc.h"
#include "kspread_table.h"

#include <koscript_parser.h>
#include <koscript_util.h>
#include <koscript_func.h>
#include <koscript_synext.h>

#include <stdlib.h>
#include <math.h>
#include <float.h>

#include <kdebug.h>

// Function: CHAR
bool kspreadfunc_char( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "CHAR", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::IntType, true ) &&
       !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
        return false;

  args[0]->cast( KSValue::IntType );
  int val = args[0]->intValue();
  QString str = QChar( val );

  context.setValue( new KSValue( str ) );
  return true;
}

// Function: CLEAN
bool kspreadfunc_clean( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 1, "CLEAN", true ) )
      return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
      return false;

    QString str(args[0]->stringValue());
    QString result;
    QChar   c;
    int     i;
    int     l = str.length();

    for (i = 0; i < l; ++i)
    {
      c = str[i];
      if (c.isPrint())
        result += c;
    }

    context.setValue(new KSValue(result));

    return true;
}

// Function: CODE
bool kspreadfunc_code( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "CODE", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
    return false;

  QString str = args[0]->stringValue();
  if( str.length() <= 0 )
     return false;

  context.setValue( new KSValue( str[0].unicode() ) );
  return true;
}

// Function: COMPARE
bool kspreadfunc_compare( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 3, "COMPARE", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[1], KSValue::StringType, true ) )
    return false;

  if ( !KSUtil::checkType( context, args[2], KSValue::BoolType, true ) )
    return false;

  int  result = 0;
  bool exact = args[2]->boolValue();

  QString s1 = args[0]->stringValue();
  QString s2 = args[1]->stringValue();

  if (!exact)
    result = s1.lower().localeAwareCompare(s2.lower());
  else
    result = s1.localeAwareCompare(s2);

  if (result < 0)
    result = -1;
  else if (result > 0)
    result = 1;

  context.setValue( new KSValue(result) );
  return true;
}

static bool kspreadfunc_concatenate_helper( KSContext& context, QValueList<KSValue::Ptr>& args, QString& tmp )
{
  QValueList<KSValue::Ptr>::Iterator it = args.begin();
  QValueList<KSValue::Ptr>::Iterator end = args.end();
  QString tmp2;
  for( ; it != end; ++it )
  {
    if ( KSUtil::checkType( context, *it, KSValue::ListType, false ) )
    {
      if ( !kspreadfunc_concatenate_helper( context, (*it)->listValue(), tmp ) )
        return false;
    }
    else if ( KSUtil::checkType( context, *it, KSValue::StringType, true ) )
      tmp+= (*it)->stringValue();
    else if( KSUtil::checkType( context, *it, KSValue::DoubleType, true ) )
      tmp+= KGlobal::locale()->formatNumber((*it)->doubleValue());
    else
      return false;
  }
  return true;
}

// Function: CONCATENATE
bool kspreadfunc_concatenate( KSContext& context )
{
  QString tmp;
  bool b = kspreadfunc_concatenate_helper( context, context.value()->listValue(), tmp );

  if ( b )
    context.setValue( new KSValue( tmp ) );

  return b;
}

// Function: DOLLAR
bool kspreadfunc_dollar( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "DOLLAR", true ) &&
       !KSUtil::checkArgumentsCount( context, 2, "DOLLAR", true ) )
         return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::DoubleType, true ) )
    return false;

  double value = args[0]->doubleValue();
  int decimals = 2;

  if( KSUtil::checkArgumentsCount( context, 2, "DOLLAR", false ) )
    if ( KSUtil::checkType( context, args[0], KSValue::IntType, false ) )
      decimals = args[1]->intValue(); 

  QString result = KGlobal::locale()->formatMoney( value, QString::null, decimals );
  context.setValue( new KSValue( result ) );
  return true;
}

// Function: EXACT
bool kspreadfunc_exact( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 2, "EXACT", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
    return false;
  if ( !KSUtil::checkType( context, args[1], KSValue::StringType, true ) )
    return false;
  bool exact = args[1]->stringValue() == args[0]->stringValue();
  context.setValue( new KSValue(exact) );
  return true;
}

// Function: FIND
bool kspreadfunc_find( KSContext& context )
{
    QString find_text, within_text;
    int start_num = 1;

    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 2, "FIND", true ) &&
         !KSUtil::checkArgumentsCount( context, 3, "FIND", true ) )
           return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
      return false;

    if ( !KSUtil::checkType( context, args[1], KSValue::StringType, true ) )
      return false;

    if ( KSUtil::checkArgumentsCount( context, 3, "FIND", false ) )
      if ( KSUtil::checkType( context, args[2], KSValue::IntType, false ) )
         start_num = args[2]->intValue();

    find_text = args[0]->stringValue();
    within_text = args[1]->stringValue();

    // conforms to Excel behaviour
    if( start_num <= 0 ) return false;
    if( start_num > within_text.length() ) return false;

    int pos = within_text.find( find_text, start_num-1 );
    if( pos < 0 ) return false;

    context.setValue( new KSValue( pos + 1 ) );
    return true;
}

// Function: JOIN
// This is obsolete, use CONCATENATE
bool kspreadfunc_join( KSContext& context )
{
  return kspreadfunc_concatenate( context );
}

// Function: LEFT
bool kspreadfunc_left( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 2, "left", true ) )
      return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
      return false;
    int nb;
    if( KSUtil::checkType( context, args[1], KSValue::DoubleType, false ) )
      nb = (int) args[1]->doubleValue();
    else if( KSUtil::checkType( context, args[1], KSValue::IntType, true ) )
      nb = args[1]->intValue();
    else
      return false;

    QString tmp = args[0]->stringValue().left(nb);
    context.setValue( new KSValue( tmp ) );
    return true;
}

// Function: LEN
bool kspreadfunc_len( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "len", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
    return false;

  int nb=args[0]->stringValue().length();
  context.setValue( new KSValue(nb));
  return true;
}

// Function: LOWER
bool kspreadfunc_lower( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 1, "lower", true ) )
      return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
      return false;

    QString tmp = args[0]->stringValue().lower();
    context.setValue( new KSValue( tmp ) );
    return true;
}

// Function: MID
bool kspreadfunc_mid( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    uint len = 0xffffffff;
    if ( KSUtil::checkArgumentsCount( context, 3, "mid", false ) )
    {
      if( KSUtil::checkType( context, args[2], KSValue::DoubleType, false ) )
        len = (uint) args[2]->doubleValue();
      else if( KSUtil::checkType( context, args[2], KSValue::IntType, true ) )
        len = (uint) args[2]->intValue();
      else
        return false;
    }
    else if ( !KSUtil::checkArgumentsCount( context, 2, "mid", true ) )
      return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
      return false;
    int pos;
    if( KSUtil::checkType( context, args[1], KSValue::DoubleType, false ) )
      pos = (int) args[1]->doubleValue();
    else if( KSUtil::checkType( context, args[1], KSValue::IntType, true ) )
      pos = args[1]->intValue();
    else
      return false;

    QString tmp = args[0]->stringValue().mid( pos, len );
    context.setValue( new KSValue(tmp));
    return true;
}

// Function: PROPER
bool kspreadfunc_proper(KSContext & context)
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if (args.count() != 1)
    return false;

  QString str;

  if (KSUtil::checkType(context, args[0],
                        KSValue::StringType, true))
  {
    unsigned int i;
    str = args[0]->stringValue().lower();
    QChar f;
    bool  first = true;

    for (i = 0; i < str.length(); ++i)
    {
      if (first)
      {
        f = str[i];
        if (f.isNumber())
          continue;

        f = f.upper();

        str[i] = f;
        first = false;

        continue;
      }

      if (str[i] == ' ')
        first = true;
    }
  }

  context.setValue(new KSValue(str));

  return true;
}

// Function: REPLACE
bool kspreadfunc_replace( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();
  QString s;
  QString s1;
  QString s2;

  if ( !KSUtil::checkArgumentsCount( context, 4, "REPLACE", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
  {
    if (!KSUtil::checkType( context, args[0], KSValue::BoolType, true))
      return false;

    s = args[0]->boolValue() ? "True" : "False";
  }
  else
    s = args[0]->stringValue();

  if ( !KSUtil::checkType( context, args[1], KSValue::StringType, true ) )
  {
    if (!KSUtil::checkType( context, args[1], KSValue::BoolType, true))
      return false;

    s1 = args[1]->boolValue() ? "True" : "False";
  }
  else
    s1 = args[1]->stringValue();

  if ( !KSUtil::checkType( context, args[2], KSValue::StringType, true ) )
  {
    if (!KSUtil::checkType( context, args[2], KSValue::BoolType, true))
      return false;

    s2 = args[2]->boolValue() ? "True" : "False";
  }
  else
    s2 = args[2]->stringValue();

  if ( !KSUtil::checkType( context, args[3], KSValue::BoolType, true ) )
    return false;

  bool    b = args[3]->boolValue();

  int p = s.find(s1, 0, b);
  while (p != -1)
  {
    s.replace(p, s1.length(), s2);
    p = s.find(s1, 0, b);
  }

  context.setValue( new KSValue(s) );

  return true;
}

// Function: REPT
bool kspreadfunc_rept( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 2, "REPT", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
    return false;
  if( !KSUtil::checkType( context, args[1], KSValue::DoubleType, true ) )
    return false;

  int nb=(int) args[1]->doubleValue();
  QString tmp=args[0]->stringValue();
  QString tmp1;
  for (int i=0 ;i<nb;i++)
    tmp1+=tmp;
  context.setValue( new KSValue(tmp1));
  return true;
}

// Function: RIGHT
bool kspreadfunc_right( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 2, "right", true ) )
      return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
      return false;
    int nb;
    if( KSUtil::checkType( context, args[1], KSValue::DoubleType, false ) )
      nb = (int) args[1]->doubleValue();
    else if( KSUtil::checkType( context, args[1], KSValue::IntType, true ) )
      nb = args[1]->intValue();
    else
      return false;

    QString tmp = args[0]->stringValue().right(nb);
    context.setValue( new KSValue(tmp));
    return true;
}

// Function: SLEEK
bool kspreadfunc_sleek( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 1, "SLEEK", true ) )
      return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
      return false;

    QString str(args[0]->stringValue());
    QString result;
    QChar   c;
    int     i;
    int     l = str.length();

    for (i = 0; i < l; ++i)
    {
      c = str[i];
      if (!c.isSpace())
        result += c;
    }

    context.setValue(new KSValue(result));

    return true;
}

// Function: TOGGLE
bool kspreadfunc_toggle( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 1, "TOGGLE", true ) )
      return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
      return false;

    QString str(args[0]->stringValue());
    int i;
    int l = str.length();

    for (i = 0; i < l; ++i)
    {
      QChar c = str[i];
      QChar lc = c.lower();
      QChar uc = c.upper();

      if (c == lc) // it is in lowercase
        str[i] = c.upper();
      else if (c == uc) // it is in uppercase
        str[i] = c.lower();
    }

    context.setValue( new KSValue( str ) );

    return true;
}

// Function: TRIM
bool kspreadfunc_trim(KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 1, "trim", true ) )
        return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
        return false;
    QString text=args[0]->stringValue().simplifyWhiteSpace();
    context.setValue( new KSValue(text));
    return true;
}

// Function: UPPER
bool kspreadfunc_upper( KSContext& context )
{
    QValueList<KSValue::Ptr>& args = context.value()->listValue();

    if ( !KSUtil::checkArgumentsCount( context, 1, "upper", true ) )
      return false;

    if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
      return false;

    QString tmp = args[0]->stringValue().upper();
    context.setValue( new KSValue( tmp ) );
    return true;
}

// Function: VALUE
bool kspreadfunc_value( KSContext& context )
{
  QValueList<KSValue::Ptr>& args = context.value()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 1, "VALUE", true ) )
    return false;

  if ( !KSUtil::checkType( context, args[0], KSValue::StringType, true ) )
    return false;

  QString str = args[0]->stringValue();
  if( str.length() <= 0 )
     return false;

  // try to parse the string as number
  bool ok;
  double value = KGlobal::locale()->readNumber(str, &ok);
  if ( !ok )  value = str.toDouble(&ok);
  if( ok )
  {
     context.setValue( new KSValue( value ) );
     return true;
  }

  return false;
  // TODO parse as boolean/date/time
}
