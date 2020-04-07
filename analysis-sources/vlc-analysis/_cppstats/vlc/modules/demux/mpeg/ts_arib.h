#define TS_ARIB_CDT_PID 0x29
#define TS_ARIB_CDT_TABLE_ID 0xC8
#define TS_ARIB_DR_LOGO_TRANSMISSION 0xCF
#define TS_ARIB_CDT_DATA_TYPE_LOGO 0x01
#define TS_ARIB_LOGO_TYPE_HD_LARGE 0x05
#define TS_ARIB_LOGO_TYPE_HD_SMALL 0x02
#define TS_ARIB_LOGO_TYPE_SD43_LARGE 0x03
#define TS_ARIB_LOGO_TYPE_SD43_SMALL 0x00
#define TS_ARIB_LOGO_TYPE_SD169_LARGE 0x04
#define TS_ARIB_LOGO_TYPE_SD169_SMALL 0x01
typedef struct
{
uint8_t i_transmission_mode;
uint16_t i_logo_id;
uint16_t i_logo_version;
uint16_t i_download_data_id;
uint8_t *p_logo_char;
size_t i_logo_char;
} ts_arib_logo_dr_t;
ts_arib_logo_dr_t * ts_arib_logo_dr_Decode( const uint8_t *, size_t );
void ts_arib_logo_dr_Delete( ts_arib_logo_dr_t * );
bool ts_arib_inject_png_palette( const uint8_t *p_in, size_t i_in,
uint8_t **pp_out, size_t *pi_out );
