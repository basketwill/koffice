include("common.qs");

var listFormat1 = QTextListFormat.clone(defaultListFormat);
listFormat1.setStyle(QTextListFormat.ListDisc);
setFormatProperty(listFormat1, KListStyle.BulletCharacter, 0x25CF);
setFormatProperty(listFormat1, KListStyle.MinimumWidth, 18);
setFormatProperty(listFormat1, KListStyle.Indent, 18);
var list1 = cursor.createList(listFormat1);
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);

var list2Format = new QTextListFormat.clone(defaultListFormat);
list2Format.setStyle(QTextListFormat.ListDisc);
setFormatProperty(list2Format, KListStyle.BulletCharacter, 0x2022);
setFormatProperty(list2Format, KListStyle.MinimumWidth, 18);
setFormatProperty(list2Format, KListStyle.Indent, 18);
var list2 = cursor.insertList(list2Format);
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);

cursor.insertBlock();
list1.add(cursor.block());
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);

document;
