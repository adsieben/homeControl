#ifndef _HAVE_SNIFF_FS_UTILS_H
#define _HAVE_SNIFF_FS_UTILS_H

// ...
// YOUR HEADER SOURCE CODE
// ...

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <getopt.h>
#include <time.h>
#include "global.h"

/*
 * maximum Modbus packet size. By the standard is 300 bytes
 */
#define MODBUS_MAX_PACKET_SIZE 600

struct cli_args {
    char *serial_port;
    char *output_file;
    char parity;
    int bits;
    uint32_t speed;
    int stop_bits;
    uint32_t bytes_time_interval_us;
    bool low_latency;
    char *ascFileName;
    char *dbFileName;
};


// volatile int rotate_log;

struct pcap_global_header {
    uint32_t magic_number;  /* magic number */
    uint16_t version_major; /* major version number */
    uint16_t version_minor; /* minor version number */
    int32_t  thiszone;      /* GMT to local correction */
    uint32_t sigfigs;       /* accuracy of timestamps */
    uint32_t snaplen;       /* max length of captured packets, in octets */
    uint32_t network;       /* data link type */
} __attribute__((packed));

struct pcap_packet_header {
    uint32_t ts_sec;   /* timestamp seconds */
    uint32_t ts_usec;  /* timestamp microseconds */
    uint32_t incl_len; /* number of octets of packet saved in file */
    uint32_t orig_len; /* actual length of packet */
} __attribute__((packed));


int crc_check(uint8_t *buffer, int length);

speed_t get_baud(uint32_t baud);
void usage(FILE *fp, char *progname, int exit_code);
void parse_args(int argc, char **argv, struct cli_args *args);
void configure_serial_port(int fd, const struct cli_args *args);
void write_global_header(FILE *fp);
void write_packet_header(FILE *fp, int length);
FILE *open_logfile(const char *path);
void signal_handler();
void dump_buffer(uint8_t *buffer, uint16_t length);
int printTSMeasurement( char * filename, char * fs, time_t datetime, float * msmnt, int size, int cnt );
int printMeasurement( char * filename, char * fs, float * msmnt, int size, int cnt );



#endif //_HAVE_SNIFF_FS_UTILS_H
