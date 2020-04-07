





















#if !defined(VLC_STRINGS_H)
#define VLC_STRINGS_H 1









typedef struct vlc_player_t vlc_player_t;

static inline int vlc_ascii_toupper( int c )
{
if ( c >= 'a' && c <= 'z' )
return c + ( 'A' - 'a' );
else
return c;
}

static inline int vlc_ascii_tolower( int c )
{
if ( c >= 'A' && c <= 'Z' )
return c + ( 'a' - 'A' );
else
return c;
}








static inline int vlc_ascii_strcasecmp( const char *psz1, const char *psz2 )
{
const char *s1 = psz1;
const char *s2 = psz2;
int d = vlc_ascii_tolower( *s1 ) - vlc_ascii_tolower( *s2 );
while ( *s1 && d == 0)
{
s1++;
s2++;
d = vlc_ascii_tolower( *s1 ) - vlc_ascii_tolower( *s2 );
}

return d;
}

static inline int vlc_ascii_strncasecmp( const char *psz1, const char *psz2, size_t n )
{
const char *s1 = psz1;
const char *s2 = psz2;
const char *s1end = psz1 + n;
int d = vlc_ascii_tolower( *s1 ) - vlc_ascii_tolower( *s2 );
while ( *s1 && s1 < s1end && d == 0)
{
s1++;
s2++;
d = vlc_ascii_tolower( *s1 ) - vlc_ascii_tolower( *s2 );
}

if (s1 == s1end)
return 0;
else
return d;
}













VLC_API void vlc_xml_decode(char *st);













VLC_API char *vlc_xml_encode(const char *str) VLC_MALLOC;











VLC_API char *vlc_b64_encode_binary(const void *base, size_t length)
VLC_USED VLC_MALLOC;










VLC_API char *vlc_b64_encode(const char *str) VLC_USED VLC_MALLOC;

VLC_API size_t vlc_b64_decode_binary_to_buffer(void *p_dst, size_t i_dst_max, const char *psz_src );
VLC_API size_t vlc_b64_decode_binary( uint8_t **pp_dst, const char *psz_src );
VLC_API char * vlc_b64_decode( const char *psz_src );









VLC_API char *vlc_strftime( const char * );













VLC_API char *vlc_strfplayer( vlc_player_t *player, input_item_t *item,
const char *fmt );

static inline char *str_format( vlc_player_t *player, input_item_t *item,
const char *fmt )
{
char *s1 = vlc_strftime( fmt );
char *s2 = vlc_strfplayer( player, item, s1 );
free( s1 );
return s2;
}

VLC_API int vlc_filenamecmp(const char *, const char *);

void filename_sanitize(char *);





#endif
