/* This file is part of the KDE project

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

#include <qtextstream.h>
#include <xmltree.h>

#include <qfile.h>

XMLTree::XMLTree():QObject()
{
  QDomElement e;

  table = 0L;
  root = new QDomDocument("spreadsheet");

  root->appendChild(root->createProcessingInstruction
		    ("xml", "version=\"1.0\" encoding =\"UTF-8\""));

  doc = root->createElement("spreadsheet");
  doc.setAttribute("author", "OLEFilter");
  doc.setAttribute("email", "unknown");
  doc.setAttribute("editor", "KSpread");
  doc.setAttribute("mime", "application/x-kspread");
  root->appendChild(doc);

  paper = root->createElement("paper");
  paper.setAttribute("format", "A4");
  paper.setAttribute("orientation", "Portrait");
  doc.appendChild(paper);

  e = root->createElement("borders");
  e.setAttribute("left", 20);
  e.setAttribute("top", 20);
  e.setAttribute("right", 20);
  e.setAttribute("bottom", 20);
  paper.appendChild(e);

  map = root->createElement("map");
  doc.appendChild(map);
}

XMLTree::~XMLTree()
{
  delete root;
  root=0L;
}

const QDomDocument* const XMLTree::part()
{
  return root;
}

const QDomElement XMLTree::getFont(Q_UINT16 xf)
{
  Q_UINT16 fontid = xfs[xf]->ifnt;
  QDomElement font = root->createElement("font");

  if (fontid > 3)
    fontid--;
  font.setAttribute("family", fonts[fontid]->rgch);
  font.setAttribute("size", fonts[fontid]->dyHeight / 20);
  font.setAttribute("weight", fonts[fontid]->bls / 8);
  if ((fonts[fontid]->bls / 8) != 50)
    font.setAttribute("bold", "yes");
  if ((fonts[fontid]->grbit & 0x02) == 2)
    font.setAttribute("italic", "yes");
  if (fonts[fontid]->uls != 0)
    font.setAttribute("underlined", "yes");

  return font;
}

const QDomElement XMLTree::getFormat(Q_UINT16 xf)
{
  int align, pos, precision = -1;

  QString s;
  QDomElement format = root->createElement("format");

  align = (xfs[xf]->info2 & 0x07) == 0 ? 4 :  xfs[xf]->info2 & 0x07;
  format.setAttribute("align", align); // kspread doesn't support align=0

  if (xfs[xf]->ifmt < 0x31)
    s = "General";
  else {
    s = QString::fromLatin1(formats[xfs[xf]->ifmt]->rgch,
			    formats[xfs[xf]->ifmt]->cch);

    if ((pos = s.find("0.0")) != -1) {
      pos += 3;
      precision += 2;
      while (s.mid(pos++, 1) == "0")
	precision++;
    }
  }
  format.setAttribute("precision", precision);

  if (s.find("DM", 0, false) != -1) {
    format.setAttribute("postfix", "DM");
    format.setAttribute("faktor", 1);
  }
  if (s.find("%", 0, false) != -1) {
    format.setAttribute("postfix", "%");
    format.setAttribute("faktor", 100);
  }
  if (s.find("General", 0, false) != -1) {
    format.setAttribute("faktor", 1);
  }
  // need to add float and floatcolor

  format.appendChild(getFont(xf));

  return format;
}

bool XMLTree::_1904(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_addin(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_addmenu(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_array(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_autofilter(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_autofilterinfo(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_backup(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_blank(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_bof(Q_UINT16 size, QDataStream& body)
{
  Q_UINT16 type;

  body >> biff >> type;

  if (biff != BIFF_5_7 && biff != BIFF_8)
    return false;
  if (type == 0x10) {
    if (table != 0L)
      delete table;
    table = tables.dequeue();
  }
  return true;
}

bool XMLTree::_bookbool(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_boolerr(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_bottommargin(Q_UINT16 size, QDataStream& body)
{
  double value;
  body >> value;
  (paper.namedItem("borders")).toElement().setAttribute("bottom", value);

  return true;
}

bool XMLTree::_boundsheet(Q_UINT16 size, QDataStream& body)
{
  QDomElement *e;

  if (biff == BIFF_5_7) {
    Q_UINT8 length;
    Q_UINT16 type;
    Q_UINT32 skip;
    body >> skip >> type >> length;
    if ((type & 0x0f) == 0) {
      char *name = new char[length];
      body.readRawBytes(name, length);
      QString s = QString::fromLatin1(name, length);

      delete []name;

      e = new QDomElement(root->createElement("table"));
      e->setAttribute("name", s);
      map.appendChild(*e);
      tables.enqueue(e);
    }
  }
  else if (biff == BIFF_8) {
    Q_UINT16 type, length;
    Q_UINT32 skip;
    body >> skip >> type >> length;
    if ((type & 0x0f) == 0) {
      char *name = new char[length];
      body.readRawBytes(name, length);
      QString s = QString::fromLatin1(name, length);

      delete []name;
      e = new QDomElement(root->createElement("table"));
      e->setAttribute("name", s);
      map.appendChild(*e);
      tables.enqueue(e);
    }
  }
  return true;
}

bool XMLTree::_calccount(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_calcmode(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_cf(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_condfmt(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_codename(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_codepage(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_colinfo(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_cont(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_coordlist(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_country(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_crn(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_dbcell(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_dcon(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_dconbin(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_dconname(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_dconref(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_defaultrowheight(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_defcolwidth(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_delmenu(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_delta(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_dimensions(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_docroute(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_dsf(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_dv(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_dval(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_edg(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_eof(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_externcount(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_externname(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_externsheet(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_extsst(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_filepass(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_filesharing(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_filesharing2(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_filtermode(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_fngroupcount(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_fngroupname(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_font(Q_UINT16 size, QDataStream& body)
{
  int i;
  static int count;
  QChar *c;
  Q_UINT8 lsb, msb;

  font_rec *f = new font_rec;
  body >> f->dyHeight >> f->grbit >> f->icv >> f->bls >> f->sss;
  body >> f->uls >> f->bFamily >> f->bCharSet >> f->reserved >> f->cch;
  if (biff == BIFF_5_7) {
    for (i = 0; i < f->cch; i++) {
      body >> lsb;
      c = new QChar(lsb, 0);
      f->rgch += *c;
    }
  }
  else if (biff == BIFF_8) {
    body >> lsb;
    for (i = 0; i < f->cch; i++) {
      body >> lsb >> msb;
      c = new QChar(lsb, msb);
      f->rgch += *c;
    }
  }
  fonts.insert(count++, f);

  return true;
}

bool XMLTree::_footer(Q_UINT16 size, QDataStream& body)
{
  static int count;

  if (count++ == 0) {
    QDomElement e;
    Q_UINT8 length;
    body >> length;
    char *name = new char[length];
    body.readRawBytes(name, length);
    QString s = QString::fromLatin1(name, length);

    e = root->createElement("foot");
    e.setAttribute("left", "");
    //e.setAttribute("center", s);  // BUG?!? (Werner)
    e.setAttribute("center", "");   // replaced by this line :)
    e.setAttribute("right", "");
    paper.appendChild(e);
  }
  return true;
}

bool XMLTree::_format(Q_UINT16 size, QDataStream& body)
{
  Q_UINT16 id;
  format_rec *f = new format_rec;
  body >> id >> f->cch;
  f->rgch = new char[f->cch];
  body.readRawBytes(f->rgch, f->cch);
  formats.insert(id, f);

  return true;
}

bool XMLTree::_formula(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_gcw(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_gridset(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_guts(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_hcenter(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_header(Q_UINT16 size, QDataStream& body)
{
  static int count;

  if (count++ == 0) {
    QDomElement e;
    Q_UINT8 length;
    body >> length;
    char *name = new char[length];
    body.readRawBytes(name, length);
    QString s = QString::fromLatin1(name, length);

    e = root->createElement("head");
    e.setAttribute("left", "");
    //e.setAttribute("center", s);  // BUG?!? (Werner)
    e.setAttribute("center", "");   // replaced by this line :)
    e.setAttribute("right", "");
    paper.appendChild(e);
  }
  return true;
}

bool XMLTree::_hideobj(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_hlink(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_horizontalpagebreaks(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_imdata(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_index(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_interfaceend(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_interfacehdr(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_iteration(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_label(Q_UINT16 size, QDataStream& body)
{
  Q_UINT16 row, column, xf, length;

  body >> row >> column >> xf >> length;
  QDomElement e = root->createElement("cell");
  e.appendChild(getFormat(xf));

  char *name = new char[length];
  body.readRawBytes(name, length);
  QString s = QString::fromLatin1(name, length);
  e.setAttribute("row", (int) ++row);
  e.setAttribute("column", (int) ++column);
  QDomElement text = root->createElement("text");
  text.appendChild(root->createTextNode(s));
  e.appendChild(text);
  table->appendChild(e);

  delete []name;
  return true;
}

bool XMLTree::_labelsst(Q_UINT16 size, QDataStream& body)
{
  Q_UINT16 row, column, xf;
  Q_UINT32 isst;

  body >> row >> column >> xf >> isst;
  QDomElement e = root->createElement("cell");
  e.appendChild(getFormat(xf));

  e.setAttribute("row", (int) ++row);
  e.setAttribute("column", (int) ++column);
  QDomElement text = root->createElement("text");
  text.appendChild(root->createTextNode(sst[isst]->latin1()));
  e.appendChild(text);
  table->appendChild(e);

  return true;
}

bool XMLTree::_leftmargin(Q_UINT16 size, QDataStream& body)
{
  double value;
  body >> value;
  (paper.namedItem("borders")).toElement().setAttribute("left", value);

  return true;
}

bool XMLTree::_lhngraph(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_lhrecord(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_lpr(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_mms(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_msodrawing(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_msodrawinggroup(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_msodrawingselection(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_mulblank(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_mulrk(Q_UINT16 size, QDataStream& body)
{
  int i;
  double value = 0;
  Q_UINT16 first, last, row, xf;
  Q_UINT32 number, t[2];

  body >> row >> first;
  last = (size-6)/6;
  for (i=0; i < last-first+1; ++i) {
    body >> xf >> number;

    switch (number & 0x03) {
    case 0:
      t[0] = 0;
      t[1] = number & 0xfffffffc;
      value = *((double*) t);
      break;
    case 1:
      t[0] = 0;
      t[1] = number & 0xfffffffc;
      value = *((double*) t) / 100;
      break;
    case 2:
      value = (double) (number >> 2);
      break;
    case 3:
      value = (double) (number >> 2) / 100;
      break;
    }

    QDomElement e = root->createElement("cell");
    e.appendChild(getFormat(xf));
    QString s=QString::number(value, 'f');
    e.setAttribute("row", row+1);
    e.setAttribute("column", first+i+1);
    QDomElement text = root->createElement("text");
    text.appendChild(root->createTextNode(s));
    e.appendChild(text);
    table->appendChild(e);
  }

  return true;
}

bool XMLTree::_name(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_note(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_number(Q_UINT16 size, QDataStream& body)
{
  double value;
  Q_UINT16 row, column, xf;
  body >> row >> column >> xf >> value;

  QDomElement e = root->createElement("cell");
  e.appendChild(getFormat(xf));
  QString s=QString::number(value, 'f');
  e.setAttribute("row", (int) ++row);
  e.setAttribute("column", (int) ++column);
  QDomElement text = root->createElement("text");
  text.appendChild(root->createTextNode(s));
  e.appendChild(text);
  table->appendChild(e);

  return true;
}

bool XMLTree::_obj(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_objprotect(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_obproj(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_olesize(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_palette(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_pane(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_paramqry(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_password(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_pls(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_precision(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_printgridlines(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_printheaders(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_protect(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_prot4rev(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_qsi(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_recipname(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_refmode(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_refreshall(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_rightmargin(Q_UINT16 size, QDataStream& body)
{
  double value;
  body >> value;
  (paper.namedItem("borders")).toElement().setAttribute("right", value);

  return true;
}

bool XMLTree::_rk(Q_UINT16 size, QDataStream& body)
{
  double value = 0;
  Q_UINT32 number, t[2];
  Q_UINT16 row, column, xf;
  body >> row >> column >> xf >> number;

  switch (number & 0x03) {
  case 0:
    t[0] = 0;
    t[1] = number & 0xfffffffc;
    value = *((double*) t);
    break;
  case 1:
    t[0] = 0;
    t[1] = number & 0xfffffffc;
    value = *((double*) t) / 100;
    break;
  case 2:
    value = (double) (number >> 2);
    break;
  case 3:
    value = (double) (number >> 2) / 100;
    break;
  }

  QDomElement e = root->createElement("cell");
  e.appendChild(getFormat(xf));
  QString s=QString::number(value, 'f');
  e.setAttribute("row", (int) ++row);
  e.setAttribute("column", (int) ++column);
  QDomElement text = root->createElement("text");
  text.appendChild(root->createTextNode(s));
  e.appendChild(text);
  table->appendChild(e);

  return true;
}

bool XMLTree::_row(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_rstring(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_saverecalc(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_scenario(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_scenman(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_scenprotect(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_scl(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_selection(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_setup(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_shrfmla(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sort(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sound(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sst(Q_UINT16 size, QDataStream& body)
{
  char *name;
  QString *s;
  Q_UINT8 grbit;
  Q_UINT16 cch;
  Q_UINT32 count, skip;

  body >> count >> skip;
  for (int i=0; i < (int) count; ++i) {
    body >> cch >> grbit;
    name = new char[cch+1];
    body.readRawBytes(name, cch);
    name[cch]='\0';    s = new QString(name);
    sst.insert(i, s);
    delete[] name;
  }

  return true;
}

bool XMLTree::_standardwidth(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_string(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_style(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sub(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_supbook(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxdb(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxdbex(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxdi(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxex(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxext(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxfdbtype(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxfilt(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxformat(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxformula(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxfmla(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxidstm(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxivd(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxli(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxname(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxpair(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxpi(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxrule(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxstring(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxselect(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxtbl(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxtbpg(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxtbrgiitm(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxvd(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxvdex(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxvi(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxview(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_sxvs(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_tabid(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_tabidconf(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_table(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_templt(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_topmargin(Q_UINT16 size, QDataStream& body)
{
  double value;
  body >> value;
  (paper.namedItem("borders")).toElement().setAttribute("top", value);

  return true;
}

bool XMLTree::_txo(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_uddesc(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_uncalced(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_userbview(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_usersviewbegin(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_usersviewend(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_useselfs(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_vcenter(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_verticalpagebreaks(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_window1(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_window2(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_windowprotect(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_writeaccess(Q_UINT16 size, QDataStream& body)
{
  Q_UINT8 length;

  body >> length;

  if (biff == BIFF_8) {
    Q_UINT16 skip;
    body >> skip;
  }

  char *name = new char[length];
  body.readRawBytes(name, length);
  QString s = QString::fromLatin1(name, length);
  doc.setAttribute("author", s);

  delete []name;

  return true;
}

bool XMLTree::_writeprot(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_wsbool(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_xct(Q_UINT16, QDataStream&)
{
  return true;
}

bool XMLTree::_xf(Q_UINT16 size, QDataStream& body)
{
  static int count;

  xf_rec *x = new xf_rec;
  body >> x->ifnt >> x->ifmt >> x->info1 >> x->info2 >> x->info3;
  body >> x->info4 >> x->info5 >> x->info6 >> x->info7;
  xfs.insert(count++, x);

  return true;
}

bool XMLTree::_xl5modify(Q_UINT16, QDataStream&)
{
  return true;
}

