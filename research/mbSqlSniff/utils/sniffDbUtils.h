#ifndef _HAVE_SNIFF_DB_UTILS_H
#define _HAVE_SNIFF_DB_UTILS_H

#include <sqlite3.h>

// #include "global.h"

#define TABLENAME_SECAVG "tPvSec"
#define TABLENAME_TENSECAVG "tPvTenSec"

#define PV_AVG_RINGBUFFERSIZE 32
#define INC_AVG_RB_POS( a ) ( ( ++a ) % PV_AVG_RINGBUFFERSIZE )

int initDB( char * dbFileName );

int addValues2RingBuffer( int dateTime, float * val, int tableID );
int archiveDB();

#endif //_HAVE_SNIFF_DB_UTILS_H


// ATTACH DATABASE "values.sqlite" AS srcDB;
// insert into tPvSec select * from srcDB.tPvSec where fdatetime > (select max( fdatetime ) from tPvSec );
// insert into tPvTenSec select * from srcDB.tPvTenSec where fdatetime > (select max( fdatetime ) from tPvTenSec );
// delete  from srcDB.tpvsec where srcdb.tpvsec.fdatetime < (select CAST(strftime('%s', 'now', '-24 hour') as INTEGER) as time) ;
// delete  from srcDB.tpvTensec where srcdb.tpvTensec.fdatetime < (select CAST(strftime('%s', 'now', '-24 hour') as INTEGER) as time) ;

// select min(fDateTime) as time, count( fDateTime ) as cnt 
//   from srcDB.tpvsec tp 
//   group by fDateTime/10 
//   having fdatetime > (select CAST(strftime('%s', 'now', '-24 minute') as INTEGER) as time) 
//   order by cnt desc;

// PRAGMA srcDB.wal_checkpoint(PASSIVE);

// delete from tpvsec where fdatetime >= ( select min( fdatetime ) from srcDB.tpvsec) ;

//select avg( fDateTime ) as time, avg( fPowerSum ) from tPvTenSec group by fDateTime / 100;

//PRAGMA schema.journal_mode; 
//PRAGMA schema.journal_mode=WAL; 
// git clone --no-checkout git@github.com:adsieben/homeControl.git tmp 