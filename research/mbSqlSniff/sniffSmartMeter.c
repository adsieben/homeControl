//gcc sniffSmartMeter.c utils/sniffDbUtils.c utils/sniffFSUtils.c -lmodbus -lsqlite3  -lpthread -ldl -lm -o a
// ln -s sniffSmartMeter.20240126.v.0.1.c sniffSmartMeter.c

/*
We are going to make 2 threads, maybe 4 in the future
One thread listen to the USB device to sniff modbus data. 
- Actual values are written to /dev/shm/xxx.act
- averages are written to /dev/shm/xxx.avg 
  and alse get into a ring buffer (array). 
The second thread takes the data from the ring buffer and 
fills the database.

In future releases the sniffed data also should go into a ring buffer.
A second thread calculates the averages and is responisible for the 
files on disk. The third thread is handling the database. While the 
fourth thread manage them all.
*/

/*
 * A sniffer for the Modbus protocol
 * (c) 2020-2022 Alessandro Righi - released under the MIT license
 * (c) 2021 vheat - released under the MIT license
 * https://github.com/alerighi/modbus-sniffer/blob/master/sniffer.c
 */

#define _DEFAULT_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "./utils/sniffDbUtils.h"
#include "./utils/sniffFSUtils.h"
#include "./utils/global.h"

int clearMeasurement( float * msmnt, int size )
{
    while( size-- > 0 ) msmnt[ size ] = 0.0;
    return size++;
}


// float avgMeasurement( float * msmnt, int size )
// {
//     float sum = 0;
//     int cnt = size;
//     while( cnt-- > 0 ) sum += msmnt[ cnt ];
//     return sum/size;
// }

int main(int argc, char **argv)
{
    struct cli_args args = {0};
    int i, port, n_bytes = -1, res, n_packets = 0;
    size_t size = 0;
    uint8_t buffer[MODBUS_MAX_PACKET_SIZE];
    /* 235.10 234.40 236.60 169.00 98.50  90.20  454.10 295.10 135.70 23.10  -594.50
       U1     U2     U3     I1     I2     I3     Pges   P1     P2     P3     UIges
       3456   7..10  11..14 15..18 19..22 23..26 27..30 31..34 35..38 39..42 43..46
    */
    float fMeasurementAct[ 11 ];
    float fMeasurementAvgSec[11]; 
    float fMeasurementAvgTenSec[11]; 
    float fMeasurementAvgMin[11];
    
    int iMeasurementCntAvgSec, iMeasurementCntAvgTenSec, iMeasurementCntAvgMin;
    iMeasurementCntAvgSec = iMeasurementCntAvgTenSec = iMeasurementCntAvgMin = 0;

    struct timeval timeout;
	time_t tNow = time(NULL); /*in seconds*/
    time_t tSecOld, tTenSecOld, tMinOld;
    tSecOld = tTenSecOld = tMinOld = tNow;
//    struct tm tmNow = *localtime(&tNow); /*broken down time*/
    fd_set set;

    char *zErrMsg = 0;

    clearMeasurement( fMeasurementAct, 11 );
    clearMeasurement( fMeasurementAvgSec, 11 );
    clearMeasurement( fMeasurementAvgTenSec, 11 );
    clearMeasurement( fMeasurementAvgMin, 11 );

    signal(SIGUSR1, signal_handler);//from sniffFSUtils

    parse_args(argc, argv, &args);

    // fprintf(stderr, "starting modbus sniffer\n");

    if ((port = open(args.serial_port, O_RDONLY)) < 0)
        DIE("open port");

    configure_serial_port(port, &args);

    res = initDB( args.dbFileName );
    if( res != 1 ){
      DIE("open db");
      return(1);
    }

    while (n_bytes != 0) {

        /* RTFM! these are overwritten after each select call and thus must be inizialized again */
        FD_ZERO(&set);
        FD_SET(port, &set);

        /* also these maybe overwritten in Linux */
        timeout.tv_sec = 0;
        timeout.tv_usec = args.bytes_time_interval_us;

        if ((res = select(port + 1, &set, NULL, NULL, &timeout)) < 0 && errno != EINTR)
            DIE("select");

        /* there is something to read...  */
        if (res > 0) {
            if ((n_bytes = read(port, buffer + size, MODBUS_MAX_PACKET_SIZE - size)) < 0)
                DIE("read port");

            size += n_bytes;
        }

        /* captured an entire packet */
        // if (size > 0 && (res == 0 || size >= MODBUS_MAX_PACKET_SIZE || n_bytes == 0)) {
        // large responses get truncated after 32 bytes, so we simply make another round
        // maybe it is a problem if the message is exactly 32 bytes long
        // be warned (ad7)
        if (size > 0 && size != 32 && (res == 0 || size >= MODBUS_MAX_PACKET_SIZE || n_bytes == 0)) {
        //    fprintf(stderr, "captured packet %d: length = %zu, ", ++n_packets, size);
            tNow = time(NULL);
            if (crc_check(buffer, size)) {
                if( size > 8 )
                {   
                    Buffer2Measurement(fMeasurementAct, 11, ( uint16_t* )( buffer + 3 ),( size - 3 )/2 );
                    printMeasurement( FN_ACT_VALUES, "w", fMeasurementAct, PV_VALUENUMBER );
                    if( tSecOld < tNow )
                    {
                        for( int i = PV_VALUENUMBER; i > 0; i-- ) 
                          fMeasurementAvgSec[ i - 1 ] /= iMeasurementCntAvgSec;
                        printTSMeasurement( FN_SEC_AVG_VALUES, "a", tSecOld, fMeasurementAvgSec, PV_VALUENUMBER );
                        // fprintf( stderr, "Calling sec2db ts %d \n", tSecOld );
                        res = addValues2RingBuffer( tSecOld, fMeasurementAvgSec, PVF_AVGSEC );
                        if( res < 0 ){
                          return(1);
                        }
                        tSecOld = tNow;
                        clearMeasurement( fMeasurementAvgSec, PV_VALUENUMBER );
                        iMeasurementCntAvgSec = 0;
                    }
                    addMeasurement( fMeasurementAvgSec, 11, ( uint16_t* )( buffer + 3 ),( size - 3 )/2 );
                    iMeasurementCntAvgSec++;
                    if( tNow - tTenSecOld > 9 ) //9 seconds should
                    {
                        for( int i = PV_VALUENUMBER; i > 0; i-- ) 
                          fMeasurementAvgTenSec[ i - 1 ] /= iMeasurementCntAvgTenSec;
                        printTSMeasurement( FN_TENSEC_AVG_VALUES, "a", tTenSecOld, fMeasurementAvgTenSec, PV_VALUENUMBER );
                        // fprintf( stderr, "Calling tensec2db ts %d \n", tTenSecOld );
                        res = addValues2RingBuffer( tTenSecOld, fMeasurementAvgTenSec, PVF_AVGTENSEC );
                        if( res < 0 ){
                          return(1);
                        }
                        tTenSecOld = tNow;
                        clearMeasurement( fMeasurementAvgTenSec, PV_VALUENUMBER );
                        iMeasurementCntAvgTenSec = 0;
                    }
                    addMeasurement( fMeasurementAvgTenSec, 11, ( uint16_t* )( buffer + 3 ),( size - 3 )/2 );
                    iMeasurementCntAvgTenSec++;
                    if( tNow - tMinOld > 59 ) //9 seconds should
                    {
                        archiveDB();
                        tMinOld = tNow;
                    }
                }
            }
            size = 0;
        }
    }
    return EXIT_SUCCESS;
}