int webvtt_OpenDecoder ( vlc_object_t * );
void webvtt_CloseDecoder ( vlc_object_t * );
int webvtt_OpenDemux ( vlc_object_t * );
int webvtt_OpenDemuxStream (vlc_object_t *);
void webvtt_CloseDemux ( vlc_object_t * );
#if defined(ENABLE_SOUT)
int webvtt_OpenEncoder ( vlc_object_t * );
void webvtt_CloseEncoder ( vlc_object_t * );
#endif
typedef struct webvtt_text_parser_t webvtt_text_parser_t;
enum webvtt_header_line_e
{
WEBVTT_HEADER_STYLE = 1,
WEBVTT_HEADER_REGION,
};
typedef struct
{
vlc_tick_t i_start;
vlc_tick_t i_stop;
char *psz_id;
char *psz_text;
char *psz_attrs;
} webvtt_cue_t;
static inline void webvtt_cue_Init( webvtt_cue_t *c )
{
memset( c, 0, sizeof(*c) );
}
static inline void webvtt_cue_Clean( webvtt_cue_t *c )
{
free( c->psz_attrs );
free( c->psz_text );
free( c->psz_id );
}
webvtt_text_parser_t * webvtt_text_parser_New(
void *priv,
webvtt_cue_t *(*pf_get_cue)( void * ),
void (*pf_cue_done)( void *, webvtt_cue_t * ),
void (*pf_header)( void *, enum webvtt_header_line_e, bool, const char * )
);
void webvtt_text_parser_Delete( webvtt_text_parser_t *p );
void webvtt_text_parser_Feed( webvtt_text_parser_t *p, char *psz_line );
bool webvtt_scan_time( const char *psz, vlc_tick_t *p_time );
