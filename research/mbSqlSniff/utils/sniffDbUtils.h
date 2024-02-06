#ifndef _HAVE_SNIFF_DB_UTILS_H
#define _HAVE_SNIFF_DB_UTILS_H

#include <sqlite3.h>

// #include "global.h"

#define TABLENAME_SECAVG "tPvSec"
#define TABLENAME_TENSECAVG "tPvTenSec"

int initDB( char * dbFileName );

//remove asap
int secValues2DB( int dateTime, float * val, int cnt, char * tableTame );

#endif //_HAVE_SNIFF_DB_UTILS_H


// ATTACH DATABASE "values.sqlite" AS srcDB;
// insert into tPvSec select * from srcDB.tPvSec where fdatetime > (select max( fdatetime ) from tPvSec );
// insert into tPvTenSec select * from srcDB.tPvTenSec where fdatetime > (select max( fdatetime ) from tPvTenSec );
// delete  from srcDB.tpvsec where srcdb.tpvsec.fdatetime < (select CAST(strftime('%s', 'now', '-24 hour') as INTEGER) as time) ;
// delete  from srcDB.tpvTensec where srcdb.tpvTensec.fdatetime < (select CAST(strftime('%s', 'now', '-24 hour') as INTEGER) as time) ;

// delete from tpvsec where fdatetime >= ( select min( fdatetime ) from srcDB.tpvsec) ;

//select avg( fDateTime ) as time, avg( fPowerSum ) from tPvTenSec group by fDateTime / 100;

//PRAGMA schema.journal_mode; 
//PRAGMA schema.journal_mode=WAL; 
