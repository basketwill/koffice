#ifndef WINWORDDOC_H
#define WINWORDDOC_H

#include <qstring.h>
#include <qlist.h>
#include <paragraph.h>
#include <myfile.h>
#include <fib.h>
#include <kdebug.h>


class WinWordDoc {

public:
    WinWordDoc(const myFile &mainStream, const myFile &table0Stream,
               const myFile &table1Stream, const myFile &dataStream);
    ~WinWordDoc();

    const bool isOk() { return success; }

    void convert();
    const QString part();

private:
    WinWordDoc(const WinWordDoc &);
    const WinWordDoc &operator=(const WinWordDoc &);

    void FIBInfo();
    void readFIB();

    const short char2uni(const unsigned char c);
    inline const unsigned short read16(const unsigned char *d);
    inline const unsigned long read32(const unsigned char *d);

    bool success, ready;
    FIB *fib;
    myFile main, table, data;
    QList<Paragraph> paraList;

    static const short CP2UNI[32];
};
#endif // WINWORDDOC_H
