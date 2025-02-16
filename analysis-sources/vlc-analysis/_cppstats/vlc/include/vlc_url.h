VLC_API char *vlc_path2uri(const char *path, const char *scheme) VLC_MALLOC;
VLC_API char *vlc_uri2path(const char *url) VLC_MALLOC;
VLC_API char *vlc_uri_decode(char *str);
VLC_API char *vlc_uri_decode_duplicate(const char *str) VLC_MALLOC;
VLC_API char *vlc_uri_encode(const char *str) VLC_MALLOC;
VLC_API char *vlc_uri_compose(const vlc_url_t *) VLC_MALLOC;
VLC_API char *vlc_uri_resolve(const char *base, const char *ref) VLC_MALLOC;
VLC_API char *vlc_uri_fixup(const char *) VLC_MALLOC;
struct vlc_url_t
{
char *psz_protocol;
char *psz_username;
char *psz_password;
char *psz_host;
unsigned i_port;
char *psz_path;
char *psz_option;
char *psz_fragment;
char *psz_buffer; 
char *psz_pathbuffer; 
};
VLC_API int vlc_UrlParse(vlc_url_t *url, const char *str);
VLC_API int vlc_UrlParseFixup(vlc_url_t *url, const char *str);
VLC_API void vlc_UrlClean(vlc_url_t *);
