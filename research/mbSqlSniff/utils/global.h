#ifndef _HAVE_GLOBAL_H
#define _HAVE_GLOBAL_H

#define _VERSION "20240206"
#define _BUILD "b1"

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))        // '!!' to make sure this returns 0 or 1

#define BITMASK_SET(x, mask) ((x) |= (mask))
#define BITMASK_CLEAR(x, mask) ((x) &= (~(mask)))
#define BITMASK_FLIP(x, mask) ((x) ^= (mask))
#define BITMASK_CHECK_ALL(x, mask) (!(~(x) & (mask)))
#define BITMASK_CHECK_ANY(x, mask) ((x) & (mask))

#define REMOTE_ID 4
#define FN_ACT_VALUES "/dev/shm/powerValues.act"
#define FN_SEC_AVG_VALUES "/dev/shm/powerValues.sec.avg"
#define FN_TENSEC_AVG_VALUES "/dev/shm/powerValues.tensec.avg"
// const char * gfilename = "/dev/shm/powerValues";
#define DATADIR "./data/"
#define DB DATADIR "values.sqlite" 
  //expands to "./data/values.sqlite" 
  //(https://stackoverflow.com/questions/5256313/c-c-macro-string-concatenation)

#define PV_AVG_RINGBUFFERSIZE 32
#define INC_AVG_RB_POS( a ) ( ( ++a ) % PV_AVG_RINGBUFFERSIZE )

#define DIE(err) do { perror(err); exit(EXIT_FAILURE); } while (0)

#define PV_VALUENUMBER 11
#define PVF_AVGSEC 0x01
#define PVF_AVGTENSEC 0x02

#define THREAD_NUMBER 32
#define TF_ACTIVE 0x01
#define TF_FINISHED 0x02

struct powerValue {
    int ID;
    int flag;
    time_t dateTime; //seconds
    int val[ PV_VALUENUMBER ];
};

// struct valus2DBParameter {
//     int dateTime; 
//     float * val;
//     int cnt;
//     char tableName[ 256 ]; 
//     int threadID;
// };

#endif //_HAVE_GLOBAL_H
