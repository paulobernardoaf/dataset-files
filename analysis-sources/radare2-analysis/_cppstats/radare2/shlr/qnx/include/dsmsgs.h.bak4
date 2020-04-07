



































#if !defined(__DSMSGS_H__)
#define __DSMSGS_H__




#define PDEBUG_PROTOVER_MAJOR 0x00000000
#define PDEBUG_PROTOVER_MINOR 0x00000003

#include <stddef.h>





#define PDEBUG_ENOERR 0 
#define PDEBUG_ENOPTY 1 
#define PDEBUG_ETHREAD 2 
#define PDEBUG_ECONINV 3 
#define PDEBUG_ESPAWN 4 
#define PDEBUG_EPROCFS 5 
#define PDEBUG_EPROCSTOP 6 
#define PDEBUG_EQPSINFO 7 
#define PDEBUG_EQMEMMODEL 8 
#define PDEBUG_EQPROXY 9 
#define PDEBUG_EQDBG 10 


























#define SET_CHANNEL_RESET 0
#define SET_CHANNEL_DEBUG 1
#define SET_CHANNEL_TEXT 2
#define SET_CHANNEL_NAK 0xff





#define DS_DATA_MAX_SIZE 1024
#define DS_DATA_RCV_SIZE(msg, total) ((total) - (sizeof(*(msg)) - DS_DATA_MAX_SIZE))

#define DS_MSG_OKSTATUS_FLAG 0x20000000
#define DS_MSG_OKDATA_FLAG 0x40000000
#define DS_MSG_NO_RESPONSE 0x80000000

#define QNXNTO_NSIG 57 



#define DSHDR_MSG_BIG_ENDIAN 0x80
struct DShdr {
ut8 cmd;
ut8 subcmd;
ut8 mid;
ut8 channel;
};


enum {
DStMsg_connect, 
DStMsg_disconnect, 
DStMsg_select, 
DStMsg_mapinfo, 
DStMsg_load, 
DStMsg_attach, 
DStMsg_detach, 
DStMsg_kill, 
DStMsg_stop, 
DStMsg_memrd, 
DStMsg_memwr, 
DStMsg_regrd, 
DStMsg_regwr, 
DStMsg_run, 
DStMsg_brk, 
DStMsg_fileopen, 
DStMsg_filerd, 
DStMsg_filewr, 
DStMsg_fileclose, 
DStMsg_pidlist, 
DStMsg_cwd, 
DStMsg_env, 
DStMsg_base_address, 
DStMsg_protover, 
DStMsg_handlesig, 
DStMsg_cpuinfo, 
DStMsg_tidnames, 
DStMsg_procfsinfo, 

DSrMsg_err = 32, 
DSrMsg_ok, 
DSrMsg_okstatus, 
DSrMsg_okdata, 

DShMsg_notify = 64 
};


enum {
DSMSG_LOAD_DEBUG,
DSMSG_LOAD_RUN,
DSMSG_LOAD_RUN_PERSIST,
DSMSG_LOAD_INHERIT_ENV = 0x80
};

enum {
DSMSG_ENV_CLEARARGV,
DSMSG_ENV_ADDARG,
DSMSG_ENV_CLEARENV,
DSMSG_ENV_SETENV,
DSMSG_ENV_SETENV_MORE
};

enum { DSMSG_STOP_PID,
DSMSG_STOP_PIDS };

enum { DSMSG_SELECT_SET,
DSMSG_SELECT_QUERY };

enum { DSMSG_KILL_PIDTID,
DSMSG_KILL_PID,
DSMSG_KILL_PIDS };

enum { DSMSG_MEM_VIRTUAL,
DSMSG_MEM_PHYSICAL,
DSMSG_MEM_IO,
DSMSG_MEM_BASEREL };

enum {
DSMSG_REG_GENERAL,
DSMSG_REG_FLOAT,
DSMSG_REG_SYSTEM,
DSMSG_REG_ALT,
DSMSG_REG_END
};

enum {
DSMSG_RUN,
DSMSG_RUN_COUNT,
DSMSG_RUN_RANGE,
};

enum {
DSMSG_PIDLIST_BEGIN,
DSMSG_PIDLIST_NEXT,
DSMSG_PIDLIST_SPECIFIC,
DSMSG_PIDLIST_SPECIFIC_TID, 
}; 

enum {
DSMSG_CWD_QUERY,
DSMSG_CWD_SET,
};

enum {
DSMSG_MAPINFO_BEGIN = 0x01,
DSMSG_MAPINFO_NEXT = 0x02,
DSMSG_MAPINFO_SPECIFIC = 0x04,
DSMSG_MAPINFO_ELF = 0x80,
};

enum {
DSMSG_PROTOVER_MINOR =
0x000000FF, 
DSMSG_PROTOVER_MAJOR =
0x0000FF00, 
};

enum {
DSMSG_BRK_EXEC = 0x0001, 
DSMSG_BRK_RD = 0x0002, 
DSMSG_BRK_WR = 0x0004, 
DSMSG_BRK_RW = 0x0006, 
DSMSG_BRK_MODIFY = 0x0008, 
DSMSG_BRK_RDM = 0x000a, 
DSMSG_BRK_WRM = 0x000c, 
DSMSG_BRK_RWM =
0x000e, 
DSMSG_BRK_HW = 0x0010, 
};

enum {
DSMSG_NOTIFY_PIDLOAD, 
DSMSG_NOTIFY_TIDLOAD, 
DSMSG_NOTIFY_DLLLOAD, 
DSMSG_NOTIFY_PIDUNLOAD, 
DSMSG_NOTIFY_TIDUNLOAD, 
DSMSG_NOTIFY_DLLUNLOAD, 
DSMSG_NOTIFY_BRK, 
DSMSG_NOTIFY_STEP, 
DSMSG_NOTIFY_SIGEV, 
DSMSG_NOTIFY_STOPPED 
};




typedef struct {
struct DShdr hdr;
ut8 major;
ut8 minor;
ut8 spare[2];
} DStMsg_connect_t;


typedef struct { struct DShdr hdr; } DStMsg_disconnect_t;


typedef struct {
struct DShdr hdr;
st32 pid;
st32 tid;
} DStMsg_select_t;



typedef struct {
struct DShdr hdr;
st32 pid;
st32 addr;
} DStMsg_mapinfo_t;


typedef struct {
struct DShdr hdr;
st32 argc;
st32 envc;
char cmdline[DS_DATA_MAX_SIZE];
} DStMsg_load_t;


typedef struct {
struct DShdr hdr;
st32 pid;
} DStMsg_attach_t;

typedef DStMsg_attach_t DStMsg_procfsinfo_t;


typedef struct {
struct DShdr hdr;
st32 pid;
} DStMsg_detach_t;


typedef struct {
struct DShdr hdr;
st32 signo;
} DStMsg_kill_t;


typedef struct { struct DShdr hdr; } DStMsg_stop_t;


typedef struct {
struct DShdr hdr;
ut32 spare0;
ut64 addr;
ut16 size;
} DStMsg_memrd_t;


typedef struct {
struct DShdr hdr;
ut32 spare0;
ut64 addr;
ut8 data[DS_DATA_MAX_SIZE];
} DStMsg_memwr_t;


typedef struct {
struct DShdr hdr;
ut16 offset;
ut16 size;
} DStMsg_regrd_t;


typedef struct {
struct DShdr hdr;
ut16 offset;
ut8 data[DS_DATA_MAX_SIZE];
} DStMsg_regwr_t;


typedef struct {
struct DShdr hdr;
union {
ut32 count;
ut32 addr[2];
} step;
} DStMsg_run_t;


typedef struct {
struct DShdr hdr;
ut32 addr;
ut32 size;
} DStMsg_brk_t;


typedef struct {
struct DShdr hdr;
st32 mode;
st32 perms;
char pathname[DS_DATA_MAX_SIZE];
} DStMsg_fileopen_t;


typedef struct {
struct DShdr hdr;
ut16 size;
} DStMsg_filerd_t;


typedef struct {
struct DShdr hdr;
ut8 data[DS_DATA_MAX_SIZE];
} DStMsg_filewr_t;


typedef struct {
struct DShdr hdr;
st32 mtime;
} DStMsg_fileclose_t;


typedef struct {
struct DShdr hdr;
st32 pid; 
st32 tid; 
} DStMsg_pidlist_t;


typedef struct {
struct DShdr hdr;
ut8 path[DS_DATA_MAX_SIZE];
} DStMsg_cwd_t;


typedef struct {
struct DShdr hdr;
char data[DS_DATA_MAX_SIZE];
} DStMsg_env_t;


typedef struct { struct DShdr hdr; } DStMsg_baseaddr_t;


typedef struct {
struct DShdr hdr;
ut8 major;
ut8 minor;
} DStMsg_protover_t;


typedef struct {
struct DShdr hdr;
ut8 signals[QNXNTO_NSIG];
ut32 sig_to_pass;
} DStMsg_handlesig_t;


typedef struct {
struct DShdr hdr;
ut32 spare;
} DStMsg_cpuinfo_t;


typedef struct {
struct DShdr hdr;
ut32 spare;
} DStMsg_tidnames_t;




#define NOTIFY_HDR_SIZE offsetof (DShMsg_notify_t, un)
#define NOTIFY_MEMBER_SIZE(member) sizeof(member)

typedef struct {
struct DShdr hdr;
st32 pid;
st32 tid;
union {
struct {
ut32 codeoff;
ut32 dataoff;
ut16 ostype;
ut16 cputype;
ut32 cpuid; 
char name[DS_DATA_MAX_SIZE];
} pidload;
struct {
st32 status;
} pidunload;
struct {
st32 status;
ut8 faulted;
ut8 reserved[3];
} pidunload_v3;
struct {
ut32 ip;
ut32 dp;
ut32 flags; 
} brk;
struct {
ut32 ip;
ut32 lastip;
} step;
struct {
st32 signo;
st32 code;
st32 value;
} sigev;
} un;
} DShMsg_notify_t;




typedef struct {
struct DShdr hdr;
st32 err;
} DSrMsg_err_t;


typedef struct { struct DShdr hdr; } DSrMsg_ok_t;






typedef struct {
struct DShdr hdr;
st32 status;
} DSrMsg_okstatus_t;


struct dslinkmap {
ut32 addr;
ut32 size;
ut32 flags;
ut32 debug_vaddr;
ut64 offset;
};

struct dsmapinfo {
struct dsmapinfo *next;
ut32 spare0;
ut64 ino;
ut32 dev;
ut32 spare1;
struct dslinkmap text;
struct dslinkmap data;
char name[256];
};

struct dspidlist {
st32 pid;
st32 num_tids; 
st32 spare[6];
struct tidinfo {
st16 tid;
ut8 state;
ut8 flags;
} tids[1]; 
char name[1]; 
};

struct dscpuinfo {
ut32 cpuflags;
ut32 spare1;
ut32 spare2;
ut32 spare3;
};

struct dstidnames {
ut32 numtids;
ut32 numleft;
ut32 spare1;
ut32 spare2;
char data[1]; 
};



typedef struct {
struct DShdr hdr;
ut8 data[DS_DATA_MAX_SIZE];
} DSrMsg_okdata_t;


typedef union {
struct DShdr hdr;
DStMsg_connect_t connect;
DStMsg_disconnect_t disconnect;
DStMsg_select_t select;
DStMsg_load_t load;
DStMsg_attach_t attach;
DStMsg_procfsinfo_t procfsinfo;
DStMsg_detach_t detach;
DStMsg_kill_t kill;
DStMsg_stop_t stop;
DStMsg_memrd_t memrd;
DStMsg_memwr_t memwr;
DStMsg_regrd_t regrd;
DStMsg_regwr_t regwr;
DStMsg_run_t run;
DStMsg_brk_t brk;
DStMsg_fileopen_t fileopen;
DStMsg_filerd_t filerd;
DStMsg_filewr_t filewr;
DStMsg_fileclose_t fileclose;
DStMsg_pidlist_t pidlist;
DStMsg_mapinfo_t mapinfo;
DStMsg_cwd_t cwd;
DStMsg_env_t env;
DStMsg_baseaddr_t baseaddr;
DStMsg_protover_t protover;
DStMsg_handlesig_t handlesig;
DStMsg_cpuinfo_t cpuinfo;
DStMsg_tidnames_t tidnames;
DShMsg_notify_t notify;
DSrMsg_err_t err;
DSrMsg_ok_t ok;
DSrMsg_okstatus_t okstatus;
DSrMsg_okdata_t okdata;
} DSMsg_union_t;


#define TS_TEXT_MAX_SIZE 100


enum {
TSMsg_text, 
TSMsg_done, 
TSMsg_start, 
TSMsg_stop, 
TSMsg_ack, 
};

struct TShdr {
ut8 cmd;
ut8 console;
ut8 spare1;
ut8 channel;
};




typedef struct {
struct TShdr hdr;
char text[TS_TEXT_MAX_SIZE];
} TSMsg_text_t;


typedef struct { struct TShdr hdr; } TSMsg_done_t;


typedef struct { struct TShdr hdr; } TSMsg_flowctl_t;


typedef struct { struct TShdr hdr; } TSMsg_ack_t;

#endif
