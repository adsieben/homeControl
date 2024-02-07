// void tmp();

#include <stdio.h>
#include <time.h>
#include <sqlite3.h>
#include <pthread.h>
#include <unistd.h>

#include "sniffDbUtils.h"
#include "global.h"

pthread_t thread = -1;
pthread_attr_t attr;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

sqlite3 *db;
sqlite3_stmt *pStmtSecAvg;
sqlite3_stmt *pStmtTenSecAvg;

struct powerValue {
    int ID;
    int flag;
    time_t dateTime; //seconds
    float val[ PV_VALUENUMBER ];
};

// struct valus2DBParameter {
//     int dateTime; 
//     float * val;
//     int cnt;
//     char tableName[ 256 ]; 
//     int threadID;
// };

struct powerValue pvAvgRingBuffer[ PV_AVG_RINGBUFFERSIZE ];
int pvAvgPos = -1;

int prepSQL( sqlite3 * db, sqlite3_stmt **SqlStmt, char * tableName );
int insertValues2DB( sqlite3_stmt *SqlStmt, int dateTime, float * val );
// int insertValues2DB( sqlite3_stmt *SqlStmt, int dateTime, double * val );
int values2DB( int dateTime, float * val );
// int secValues2DB( int dateTime, float * val, int cnt, char * tableTame );
// int tensecValues2DB( int dateTime, float * val, int cnt );

int prepSQL( sqlite3 * db, sqlite3_stmt **SqlStmt, char * tableName ){
  //1705769861;235.29;234.31;236.81;263.50;227.84;159.47;1312.22;552.10;488.09;271.92;-306.28
  //datetime   u1     u2     u3     i1     i2     i3     pSum    p1     p2     p3     vaSum  
  //1          2      3      4      5      6      7      8       9      10     11     12
  const char * stmtTmplt = "INSERT INTO %s VALUES ( ?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12 )";
// int sqlite3_prepare_v2(
//   sqlite3 *db,            /* Database handle */
//   const char *zSql,       /* SQL statement, UTF-8 encoded */
//   int nByte,              /* Maximum length of zSql in bytes. */
//   sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
//   const char **pzTail     /* OUT: Pointer to unused portion of zSql */
// );
  char sSqlStmt [256];
  sprintf( sSqlStmt, stmtTmplt, tableName ); 
//   fprintf(stderr, "i am here iii %s\n", sSqlStmt );
  int rc = sqlite3_prepare( db, sSqlStmt, -1, SqlStmt, NULL );
  if (rc != SQLITE_OK) {
      fprintf(stderr, "Cannot prepare statement for table %d: %s\n", tableName, sqlite3_errmsg(db));
      return rc;
  }else{
    return 0;
  }    
}

int insertValues2DB( sqlite3_stmt *SqlStmt, int dateTime, float * val ){
  //1705769861;235.29;234.31;236.81;263.50;227.84;159.47;1312.22;552.10;488.09;271.92;-306.28
  //datetime   u1     u2     u3     i1     i2     i3     pSum    p1     p2     p3     vaSum  
  //1          2      3      4      5      6      7      8       9      10     11     12
  int ret;
  sqlite3_bind_int( SqlStmt, 1, dateTime );
  for( int jj = 0; jj < 11; jj++){
    sqlite3_bind_double( SqlStmt, jj + 2, val[ jj ] );
  }
  ret = sqlite3_step( SqlStmt );
  if( ret != SQLITE_DONE )
  {
    fprintf(stderr, "Cannot step: %s\n", sqlite3_errmsg(db));
    return ret;
  }
  sqlite3_reset( SqlStmt );
  return ret;
}

// int Values2DB( int dateTime, float * val , char * tableName, int cnt ){
void * Values2DB( void * Par ){
  for(;;)
  {
    for( int ii = 0; ii < PV_AVG_RINGBUFFERSIZE; ii++ )
    {
      if( BITMASK_CHECK_ALL( pvAvgRingBuffer[ ii ].flag, PVF_VALUE ) )
      {
        if( BITMASK_CHECK_ALL( pvAvgRingBuffer[ ii ].flag, PVF_AVGSEC ) )
        {
          insertValues2DB( pStmtSecAvg, pvAvgRingBuffer[ ii ].dateTime, pvAvgRingBuffer[ ii ].val );
        }
        else if( BITMASK_CHECK_ALL( pvAvgRingBuffer[ ii ].flag, PVF_AVGTENSEC ) )
        {
          insertValues2DB( pStmtTenSecAvg, pvAvgRingBuffer[ ii ].dateTime, pvAvgRingBuffer[ ii ].val );
        }
        BITMASK_CLEAR( pvAvgRingBuffer[ ii ].flag, 0xFF );
      }
    }
    // ii %= PV_AVG_RINGBUFFERSIZE;
    // ii = 0;
    sleep( 1 );
  }
}

//https://www2.sqlite.org/cvstrac/wiki?p=MultiThreading
int initDB( char * dbFileName ){

  for( int ii = 0; ii < PV_AVG_RINGBUFFERSIZE; ii++ )
  {
    pvAvgRingBuffer[ ii ].ID = 0;
    pvAvgRingBuffer[ ii ].flag = 0;
    pvAvgRingBuffer[ ii ].dateTime = 0; //seconds
    // pvAvgRingBuffer[ ii ].val[ PV_VALUENUMBER ];
  }

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  int rc = sqlite3_open( dbFileName, &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return( -1 );
  }

  sqlite3_busy_timeout( db, 10000 );//10s

  rc = prepSQL( db, &pStmtSecAvg, TABLENAME_SECAVG );
  if (rc != SQLITE_OK) {
      return rc;
  }

  rc = prepSQL( db, &pStmtTenSecAvg, TABLENAME_TENSECAVG );
  if (rc != SQLITE_OK) {
      return rc;
  }

  // pthread_create( thread, &attr, &Values2DB, ( void * )&dbPar );
  pthread_create( &thread, &attr, &Values2DB, NULL );

  // for( int ii = 0; ii < THREAD_NUMBER; ii++){
  //   for( int jj = 0; jj < 11; jj++){
  //     values[ ii ][ jj ] = 0;
  //   }
  //   thread[ ii ] = 0;
  //   threadFlag[ ii ] = 0;
  //   threadLastCalled = threadOldest = -1;
  // }
  // sprintf( sDBFileName, "%s", dbFileName );
  return 1;
}

int finalizeDB( char * dbFileName ){
  sqlite3_finalize(pStmtSecAvg);    
  sqlite3_finalize(pStmtTenSecAvg);    
  sqlite3_close(db);
  // pthread_join( thread[ ii ], res);//exit thread
}

int addValues2RingBuffer( int dateTime, float * val, int tableID )
{
  pvAvgPos = INC_AVG_RB_POS( pvAvgPos ); 
  BITMASK_SET( pvAvgRingBuffer[ pvAvgPos ].flag, PVF_VALUE );
  pvAvgRingBuffer[ pvAvgPos ].dateTime = dateTime;
  BITMASK_SET( pvAvgRingBuffer[ pvAvgPos ].flag, tableID );
  for( int ii = PV_VALUENUMBER - 1; ii >= 0; ii--)
  {
    pvAvgRingBuffer[ pvAvgPos ].val[ ii ] = val[ ii ];
  }
  return pvAvgPos;
};
