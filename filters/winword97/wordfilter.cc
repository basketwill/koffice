#include "wordfilter.h"
#include "wordfilter.moc"

WordFilter::WordFilter(const myFile &mainStream, const myFile &table0Stream,
                       const myFile &table1Stream, const myFile &dataStream) :
                       FilterBase() {

    success=false; // normally true; only at the moment...

    myDoc=0L;
    myDoc=new WinWordDoc(mainStream, table0Stream, table1Stream, dataStream);
    connect(myDoc, SIGNAL(signalFilterError()), this, SLOT(slotFilterError()));

    myKwd=0L;
    myKwd=new KWordDoc();
    connect(myKwd, SIGNAL(signalFilterError()), this, SLOT(slotFilterError()));
}

WordFilter::~WordFilter() {

    kdebug(KDEBUG_INFO, 31000, "WordFilter - DTOR - Anfang");

    if(myDoc) {
        delete myDoc;
        myDoc=0L;
    }
    if(myKwd) {
        delete myKwd;
        myKwd=0L;
    }

    kdebug(KDEBUG_INFO, 31000, "WordFilter - DTOR - Ende");
}
