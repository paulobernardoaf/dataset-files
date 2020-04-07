

















#if !defined(VLC_TS_PSI_EIT_H)
#define VLC_TS_PSI_EIT_H

#include "ts_pid_fwd.h"



#define TS_SI_SDT_PID 0x11
#define TS_SI_EIT_PID 0x12
#define TS_SI_TDT_PID 0x14

#define TS_SI_TDT_TABLE_ID 0x70
#define TS_SI_TOT_TABLE_ID 0x73

#define TS_SI_RUNSTATUS_UNDEFINED 0x00
#define TS_SI_RUNSTATUS_STOPPED 0x01
#define TS_SI_RUNSTATUS_STARTING 0x02
#define TS_SI_RUNSTATUS_PAUSING 0x03
#define TS_SI_RUNSTATUS_RUNNING 0x04
#define TS_SI_RUNSTATUS_OFF_AIR 0x05

void ts_si_Packet_Push( ts_pid_t *, const uint8_t * );

bool ts_attach_SI_Tables_Decoders( ts_pid_t * );

#endif
