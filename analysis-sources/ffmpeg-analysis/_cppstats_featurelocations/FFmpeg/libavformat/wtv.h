




















#if !defined(AVFORMAT_WTV_H)
#define AVFORMAT_WTV_H

#include "riff.h"
#include "asf.h"

#define WTV_SECTOR_BITS 12
#define WTV_SECTOR_SIZE (1 << WTV_SECTOR_BITS)
#define WTV_BIGSECTOR_BITS 18
#define WTV_PAD8(x) (((x) + 7) & ~7)

extern const uint8_t ff_timeline_le16[16];
extern const uint8_t ff_timeline_table_0_entries_Events_le16[62];
extern const uint8_t ff_table_0_entries_legacy_attrib_le16[58];
extern const uint8_t ff_table_0_entries_time_le16[40];

extern const ff_asf_guid ff_dir_entry_guid;
extern const ff_asf_guid ff_wtv_guid;
extern const ff_asf_guid ff_timestamp_guid;
extern const ff_asf_guid ff_data_guid;
extern const ff_asf_guid ff_SBE2_STREAM_DESC_EVENT;
extern const ff_asf_guid ff_stream1_guid;
extern const ff_asf_guid ff_sync_guid;
extern const ff_asf_guid ff_index_guid;
extern const ff_asf_guid ff_mediatype_audio;
extern const ff_asf_guid ff_mediatype_video;
extern const ff_asf_guid ff_format_none;
extern const AVCodecGuid ff_video_guids[];

extern const ff_asf_guid ff_DSATTRIB_TRANSPORT_PROPERTIES;
extern const ff_asf_guid ff_metadata_guid;
extern const ff_asf_guid ff_stream2_guid;
extern const ff_asf_guid ff_mediasubtype_cpfilters_processed;
extern const ff_asf_guid ff_format_cpfilters_processed;
extern const ff_asf_guid ff_format_waveformatex;
extern const ff_asf_guid ff_format_mpeg2_video;
extern const ff_asf_guid ff_format_videoinfo2;

#endif 
