#define GPS_UTC_EPOCH_OFFSET 315964800
typedef struct atsc_a65_handle_t atsc_a65_handle_t;
atsc_a65_handle_t *atsc_a65_handle_New( const char *psz_lang );
void atsc_a65_handle_Release( atsc_a65_handle_t * );
char * atsc_a65_Decode_multiple_string( atsc_a65_handle_t *, const uint8_t *, size_t );
char * atsc_a65_Decode_simple_UTF16_string( atsc_a65_handle_t *, const uint8_t *, size_t );
static inline time_t atsc_a65_GPSTimeToEpoch( time_t i_seconds, time_t i_gpstoepoch_leaptime_offset )
{
return i_seconds + GPS_UTC_EPOCH_OFFSET - i_gpstoepoch_leaptime_offset;
}
