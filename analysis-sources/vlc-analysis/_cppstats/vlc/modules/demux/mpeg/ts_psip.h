#include "ts_pid_fwd.h"
#if !defined(_DVBPSI_DVBPSI_H_)
#include <dvbpsi/dvbpsi.h>
#endif
#define ATSC_EIT_MAX_DEPTH_MIN1 3 
#define ATSC_TABLE_TYPE_TVCT 0x0000
#define ATSC_TABLE_TYPE_TVCT_NEXT 0x0001
#define ATSC_TABLE_TYPE_CVCT 0x0002
#define ATSC_TABLE_TYPE_CVCT_NEXT 0x0003
#define ATSC_TABLE_TYPE_ETT 0x0004
#define ATSC_TABLE_TYPE_DCCSCT 0x0005
#define ATSC_TABLE_TYPE_EIT_0 0x0100
#define ATSC_TABLE_TYPE_EIT_127 0x017F
#define ATSC_TABLE_TYPE_ETT_0 0x0200
#define ATSC_TABLE_TYPE_ETT_127 0x027F
#define ATSC_TABLE_TYPE_RTT_1 0x0301
#define ATSC_TABLE_TYPE_RTT_255 0x03FF
#define ATSC_TABLE_TYPE_DCCT_00 0x1400
#define ATSC_TABLE_TYPE_DCCT_FF 0x14FF
#define ATSC_BASE_PID 0x1FFB
#define ATSC_MGT_TABLE_ID 0xC7
#define ATSC_TVCT_TABLE_ID 0xC8
#define ATSC_CVCT_TABLE_ID 0xC9
#define ATSC_RRT_TABLE_ID 0xCA
#define ATSC_EIT_TABLE_ID 0xCB
#define ATSC_ETT_TABLE_ID 0xCC
#define ATSC_STT_TABLE_ID 0xCD
#define ATSC_DESCRIPTOR_CONTENT_ADVISORY 0x87
#define ATSC_DESCRIPTOR_EXTENDED_CHANNEL_NAME 0xA0
typedef struct ts_psip_context_t ts_psip_context_t;
void ts_psip_Packet_Push( ts_pid_t *, const uint8_t * );
ts_psip_context_t * ts_psip_context_New( void );
void ts_psip_context_Delete( ts_psip_context_t * );
bool ATSC_Attach_Dvbpsi_Base_Decoders( dvbpsi_t *p_handle, void *p_cb_data );
void ATSC_Detach_Dvbpsi_Decoders( dvbpsi_t *p_handle );
