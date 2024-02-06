// void tmp();

#include <stdio.h>
#include <time.h>
#include <sqlite3.h>
#include <pthread.h>

#include "sniffDbUtils.h"
#include "global.h"

char sDBFileName [256];
float values[THREAD_NUMBER][11];
pthread_t thread[THREAD_NUMBER];
int threadFlag[THREAD_NUMBER];
int threadLastCalled, threadOldest;
pthread_attr_t attr;
pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER;

struct powerValue pvAvgRingBuffer[ PV_AVG_RINGBUFFERSIZE ];
int pvAvgPos = -1;

struct valus2DBParameter {
    int dateTime; 
    float * val;
    int cnt;
    char tableName[ 256 ]; 
    int threadID;
};

int prepSQL( sqlite3 * db, sqlite3_stmt **SqlStmt, char * tableName );
int insertValues2DB( sqlite3_stmt *SqlStmt, int dateTime, float * val );
// int insertValues2DB( sqlite3_stmt *SqlStmt, int dateTime, double * val );
int values2DB( int dateTime, float * val );
// int secValues2DB( int dateTime, float * val, int cnt, char * tableTame );
// int tensecValues2DB( int dateTime, float * val, int cnt );

void * Values2DB( void * Par );

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
      fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
      return rc;
  }else{
    return 0;
  }    
}

// int insertValues2DB( sqlite3_stmt *SqlStmt, int dateTime, double * val ){
//   //1705769861;235.29;234.31;236.81;263.50;227.84;159.47;1312.22;552.10;488.09;271.92;-306.28
//   //datetime   u1     u2     u3     i1     i2     i3     pSum    p1     p2     p3     vaSum  
//   //1          2      3      4      5      6      7      8       9      10     11     12
//   int ret;
//   for( int jj = 0; jj < 11; jj++){
//     sqlite3_bind_double( SqlStmt, jj + 2, val[ jj ] );
//   }
//   sqlite3_bind_int( SqlStmt, 1, dateTime );
//   ret = sqlite3_step( SqlStmt );
//   sqlite3_reset( SqlStmt );
//   return ret;
// }

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
  sqlite3_reset( SqlStmt );
  return ret;
}

//https://www2.sqlite.org/cvstrac/wiki?p=MultiThreading
int initDB( char * dbFileName ){
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  for( int ii = 0; ii < THREAD_NUMBER; ii++){
    for( int jj = 0; jj < 11; jj++){
      values[ ii ][ jj ] = 0;
    }
    thread[ ii ] = 0;
    threadFlag[ ii ] = 0;
    threadLastCalled = threadOldest = -1;
  }
  sprintf( sDBFileName, "%s", dbFileName );
  return 1;
}

int secValues2DB( int dateTime, float * val, int cnt, char * tableName ){
  // values get overriden by calling unit
  // every time this function is called the values from the calling
  // unit are copied to the values array that are unique for every 
  // thread
    /* Initialize and set thread detached attribute */
  struct valus2DBParameter dbPar;
  //here is no threadls 
//   pthread_mutex_lock( &count_mutex );
  fprintf( stderr, "V2DB called ts %d tnr %d for %s ", dateTime, threadLastCalled, tableName );
  dbPar.threadID = ( ++threadLastCalled ) % THREAD_NUMBER;
  fprintf( stderr, "tnr %d %d", threadLastCalled, dbPar.threadID );
  threadLastCalled %= THREAD_NUMBER;
  fprintf( stderr, "tnr %d %d", threadLastCalled, dbPar.threadID );
//   pthread_mutex_unlock( &count_mutex );
  for( int ii = 0; ii < 11; ii++){
    // fprintf(stderr, "values %5.2f", val[ ii ]);
    values[ dbPar.threadID ][ ii ] = val[ ii ];
  }
  dbPar.dateTime = dateTime;
  dbPar.val = values[ dbPar.threadID ];
  dbPar.cnt = cnt;
  sprintf( dbPar.tableName, "%s", tableName );
//   dbPar.tableName = tableName;
  
  pthread_create( thread + dbPar.threadID, &attr, &Values2DB, ( void * )&dbPar );

  void * res;
  for( int ii = 0; ii < THREAD_NUMBER; ii++ ){
    if( BITMASK_CHECK_ALL( threadFlag[ ii ], TF_FINISHED) ){
        threadFlag[ ii ] = 0;
        pthread_join( thread[ ii ], res);
        fprintf( stderr, " thread %d, %d finished\n", ii, thread[ ii ] );
    }
  }
  return 1;
}

// int tensecValues2DB( int dateTime, float * val ){
// }

// int Values2DB( int dateTime, float * val , char * tableName, int cnt ){
void * Values2DB( void * Par ){
  pthread_mutex_lock( &count_mutex );
  sqlite3 *db;
  sqlite3_stmt *pStmtValAvg;
  struct valus2DBParameter * dbPar = Par;
//   dbPar = ( *stuct valus2DBParameter ) Par;
//   fprintf(stderr, "started a thread" );
//   fprintf(stderr, "started a thread with tn %s", dbPar->tableName );
  fprintf(stderr, "on start %d: fDateTime %d\n", dbPar->threadID, dbPar->dateTime );
  BITMASK_SET( threadFlag[ dbPar->threadID ], TF_ACTIVE );
  int rc = sqlite3_open( sDBFileName, &db);
  // rc = sqlite3_open(DB, &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    pthread_mutex_unlock( &count_mutex );
    return( NULL );
  }
  sqlite3_busy_timeout( db, 10000 );//10s
  prepSQL( db, &pStmtValAvg, dbPar->tableName );

  // rc = insertValues2DB( pStmtSecAvg, time( NULL ), fMeasurementAvgSec );
  for( int i = 10; i >=0; i-- ){
    // fprintf(stderr, "values %5.2f", dbPar->val[ i ]);
    dbPar->val[ i ] /= dbPar->cnt;
  } 
  rc = insertValues2DB( pStmtValAvg, dbPar->dateTime, dbPar->val );
  if( rc != SQLITE_DONE ){
    fprintf(stderr, "Cannot step prepared statement: %s\n", sqlite3_errmsg(db));
    fprintf(stderr, "on err: fDateTime %d ID %d\n", dbPar->dateTime, dbPar->threadID );
    sqlite3_close(db);
    pthread_mutex_unlock( &count_mutex );
    return( NULL );
  }

  sqlite3_finalize(pStmtValAvg);    
  sqlite3_close(db);
  BITMASK_SET( threadFlag[ dbPar->threadID ], TF_FINISHED );
  pthread_mutex_unlock( &count_mutex );

}

