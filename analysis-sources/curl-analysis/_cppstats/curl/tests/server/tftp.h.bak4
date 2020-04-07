#if !defined(HEADER_CURL_SERVER_TFTP_H)
#define HEADER_CURL_SERVER_TFTP_H





















#include "server_setup.h"




#define SEGSIZE 512 

#if defined(__GNUC__) && ((__GNUC__ >= 3) || ((__GNUC__ == 2) && defined(__GNUC_MINOR__) && (__GNUC_MINOR__ >= 7)))

#define PACKED_STRUCT __attribute__((__packed__))
#else
#define PACKED_STRUCT 
#endif





struct tftphdr {
short th_opcode; 
unsigned short th_block; 
char th_data[1]; 
} PACKED_STRUCT;

#define th_stuff th_block
#define th_code th_block
#define th_msg th_data

#define EUNDEF 0
#define ENOTFOUND 1
#define EACCESS 2
#define ENOSPACE 3
#define EBADOP 4
#define EBADID 5
#define EEXISTS 6
#define ENOUSER 7

#endif 
