


















#if !defined(VLC_SCAN_LIST_H)
#define VLC_SCAN_LIST_H

typedef struct scan_list_entry_t scan_list_entry_t;

typedef struct scan_list_entry_t
{
char *psz_channel;

unsigned i_freq;
unsigned i_bw;
unsigned i_rate;

scan_modulation_t modulation;
scan_coderate_t coderate_lp;
scan_coderate_t coderate_hp;
scan_coderate_t inner_fec;
scan_guard_t guard_interval;
scan_delivery_t delivery;
scan_polarization_t polarization;

scan_list_entry_t *p_next;

} scan_list_entry_t;

scan_list_entry_t * scan_list_dvbv5_load( vlc_object_t *, const char *, size_t * );
scan_list_entry_t * scan_list_dvbv3_load( vlc_object_t *, const char *, size_t * );

void scan_list_entries_release( scan_list_entry_t * );

#endif
