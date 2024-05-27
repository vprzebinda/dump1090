#ifndef __AIRCRAFT_STATE_H
#define __AIRCRAFT_STATE_H

#include <ctype.h>
#include <pthread.h>
#include <sys/timeb.h>
#include "anet.h"
#include "rtl-sdr.h"

#define MODES_ASYNC_BUF_NUMBER     16
#define MODES_LONG_MSG_BYTES     14
#define MODES_MAX_BITERRORS        2                          // Global max for fixable bit erros

// Program global state
struct ModesType {                             // Internal state
    pthread_t       reader_thread;

    pthread_mutex_t data_mutex;      // Mutex to synchronize buffer access
    pthread_cond_t  data_cond;       // Conditional variable associated
    uint16_t       *pData          [MODES_ASYNC_BUF_NUMBER]; // Raw IQ sample buffers from RTL
    struct timeb    stSystemTimeRTL[MODES_ASYNC_BUF_NUMBER]; // System time when RTL passed us this block
    int             iDataIn;         // Fifo input pointer
    int             iDataOut;        // Fifo output pointer
    int             iDataReady;      // Fifo content count
    int             iDataLost;       // Count of missed buffers

    uint16_t       *pFileData;       // Raw IQ samples buffer (from a File)
    uint16_t       *magnitude;       // Magnitude vector
    uint64_t        timestampBlk;    // Timestamp of the start of the current block
    struct timeb    stSystemTimeBlk; // System time when RTL passed us currently processing this block
    int             fd;              // --ifile option file descriptor
    uint32_t       *icao_cache;      // Recently seen ICAO addresses cache
    uint16_t       *maglut;          // I/Q -> Magnitude lookup table
    int             exit;            // Exit from the main loop when true

    // RTLSDR
    int           dev_index;
    int           gain;
    int           enable_agc;
    rtlsdr_dev_t *dev;
    int           freq;
    int           ppm_error;

    // Networking
    char           aneterr[ANET_ERR_LEN];
    struct client *clients;          // Our clients
    int            sbsos;            // SBS output listening socket
    int            ros;              // Raw output listening socket
    int            ris;              // Raw input listening socket
    int            bos;              // Beast output listening socket
    int            bis;              // Beast input listening socket
    int            https;            // HTTP listening socket
    char          *rawOut;           // Buffer for building raw output data
    int            rawOutUsed;       // How much of the buffer is currently used
    char          *beastOut;         // Buffer for building beast output data
    int            beastOutUsed;     // How much if the buffer is currently used
#ifdef _WIN32
    WSADATA        wsaData;          // Windows socket initialisation
#endif

    // Configuration
    char *filename;                  // Input form file, --ifile option
    int   phase_enhance;             // Enable phase enhancement if true
    int   nfix_crc;                  // Number of crc bit error(s) to correct
    int   check_crc;                 // Only display messages with good CRC
    int   raw;                       // Raw output format
    int   beast;                     // Beast binary format output
    int   mode_ac;                   // Enable decoding of SSR Modes A & C
    int   debug;                     // Debugging mode
    int   net;                       // Enable networking
    int   net_only;                  // Enable just networking
    int   net_heartbeat_count;       // TCP heartbeat counter
    int   net_heartbeat_rate;        // TCP heartbeat rate
    int   net_output_sbs_port;       // SBS output TCP port
    int   net_output_raw_size;       // Minimum Size of the output raw data
    int   net_output_raw_rate;       // Rate (in 64mS increments) of output raw data
    int   net_output_raw_rate_count; // Rate (in 64mS increments) of output raw data
    int   net_output_raw_port;       // Raw output TCP port
    int   net_input_raw_port;        // Raw input TCP port
    int   net_output_beast_port;     // Beast output TCP port
    int   net_input_beast_port;      // Beast input TCP port
    char  *net_bind_address;         // Bind address
    int   net_http_port;             // HTTP port
    int   net_sndbuf_size;           // TCP output buffer size (64Kb * 2^n)
    int   quiet;                     // Suppress stdout
    int   interactive;               // Interactive mode
    int   interactive_rows;          // Interactive mode: max number of rows
    int   interactive_display_ttl;   // Interactive mode: TTL display
    int   interactive_delete_ttl;    // Interactive mode: TTL before deletion
    int   stats;                     // Print stats at exit in --ifile mode
    int   onlyaddr;                  // Print only ICAO addresses
    int   metric;                    // Use metric units
    int   mlat;                      // Use Beast ascii format for raw data output, i.e. @...; iso *...;
    int   interactive_rtl1090;       // flight table in interactive mode is formatted like RTL1090

    // User details
    double fUserLat;                // Users receiver/antenna lat/lon needed for initial surface location
    double fUserLon;                // Users receiver/antenna lat/lon needed for initial surface location
    int    bUserFlags;              // Flags relating to the user details

    // Interactive mode
    struct aircraft *aircrafts;
    uint64_t         interactive_last_update; // Last screen update in milliseconds
    time_t           last_cleanup_time;       // Last cleanup time in seconds

    // DF List mode
    int             bEnableDFLogging; // Set to enable DF Logging
    pthread_mutex_t pDF_mutex;        // Mutex to synchronize pDF access
    struct stDF    *pDF;              // Pointer to DF list

    // Statistics
    unsigned int stat_valid_preamble;
    unsigned int stat_demodulated0;
    unsigned int stat_demodulated1;
    unsigned int stat_demodulated2;
    unsigned int stat_demodulated3;
    unsigned int stat_goodcrc;
    unsigned int stat_badcrc;
    unsigned int stat_fixed;

    // Histogram of fixed bit errors: index 0 for single bit erros,
    // index 1 for double bit errors etc.
    unsigned int stat_bit_fix[MODES_MAX_BITERRORS];

    unsigned int stat_http_requests;
    unsigned int stat_sbs_connections;
    unsigned int stat_raw_connections;
    unsigned int stat_beast_connections;
    unsigned int stat_out_of_phase;
    unsigned int stat_ph_demodulated0;
    unsigned int stat_ph_demodulated1;
    unsigned int stat_ph_demodulated2;
    unsigned int stat_ph_demodulated3;
    unsigned int stat_ph_goodcrc;
    unsigned int stat_ph_badcrc;
    unsigned int stat_ph_fixed;
    // Histogram of fixed bit errors: index 0 for single bit erros,
    // index 1 for double bit errors etc.
    unsigned int stat_ph_bit_fix[MODES_MAX_BITERRORS];

    unsigned int stat_DF_Len_Corrected;
    unsigned int stat_DF_Type_Corrected;
    unsigned int stat_ModeAC;

    unsigned int stat_blocks_processed;
    unsigned int stat_blocks_dropped;
};

extern struct ModesType Modes;

struct stDF {
    struct stDF     *pNext;                      // Pointer to next item in the linked list
    struct stDF     *pPrev;                      // Pointer to previous item in the linked list
    struct aircraft *pAircraft;                  // Pointer to the Aircraft structure for this DF
    time_t           seen;                       // Dos/UNIX Time at which the this packet was received
    uint64_t         llTimestamp;                // Timestamp at which the this packet was received
    uint32_t         addr;                       // Timestamp at which the this packet was received
    unsigned char    msg[MODES_LONG_MSG_BYTES];  // the binary
};

extern struct stDF tDF;

#endif // __AIRCRAFT_STATE_H