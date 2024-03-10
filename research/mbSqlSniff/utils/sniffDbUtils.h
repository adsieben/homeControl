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
// insert into tPvSec select * from srcDB.tPvSec where fdatetime > ( select ifnull(  max( fdatetime ), 0 ) from tPvSec );
// insert into tPvTenSec select * from srcDB.tPvTenSec where fdatetime > (select ifnull( max( fdatetime ), 0 ) from tPvTenSec );
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

// CREATE TABLE tPvSec (
//     fDateTime    int PRIMARY KEY NOT NULL,
//     fVolt1       double,
//     fVolt2       double,
//     fVolt3       double,
//     fAmp1        double,
//     fAmp2        double,
//     fAmp3        double,
//     fPowerSum    double,
//     fPow1        double,
//     fPow2        double,
//     fPow3        double,
//     fVoltAmpSum  double
// );
// CREATE TABLE tPvTenSec (
//     fDateTime    int PRIMARY KEY NOT NULL,
//     fVolt1       double,
//     fVolt2       double,
//     fVolt3       double,
//     fAmp1        double,
//     fAmp2        double,
//     fAmp3        double,
//     fPowerSum    double,
//     fPow1        double,
//     fPow2        double,
//     fPow3        double,
//     fVoltAmpSum  double
// );

// ATTACH DATABASE " /var/www/html/dev/bin/data/values.20240127.sqlite" AS srcDB;
// insert into tPvSec select * from srcDB.tPvSec where fdatetime > (select max( fdatetime ) from tPvSec );
// insert into tPvTenSec select * from srcDB.tPvTenSec where fdatetime > (select max( fdatetime ) from tPvTenSec );
