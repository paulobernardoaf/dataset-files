#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include "freetype.h"
#if defined(__cplusplus)
extern "C" {
#endif
#if defined(__APPLE__)
#define SYSTEM_DEFAULT_FONT_FILE "/System/Library/Fonts/HelveticaNeue.dfont"
#define SYSTEM_DEFAULT_FAMILY "Helvetica Neue"
#define SYSTEM_DEFAULT_MONOSPACE_FONT_FILE "/System/Library/Fonts/Monaco.dfont"
#define SYSTEM_DEFAULT_MONOSPACE_FAMILY "Monaco"
#elif defined( _WIN32 )
#define SYSTEM_DEFAULT_FONT_FILE "arial.ttf" 
#define SYSTEM_DEFAULT_FAMILY "Arial"
#define SYSTEM_DEFAULT_MONOSPACE_FONT_FILE "cour.ttf"
#define SYSTEM_DEFAULT_MONOSPACE_FAMILY "Courier New"
#elif defined( __OS2__ )
#define SYSTEM_DEFAULT_FONT_FILE "/psfonts/tnrwt_k.ttf"
#define SYSTEM_DEFAULT_FAMILY "Times New Roman WT K"
#define SYSTEM_DEFAULT_MONOSPACE_FONT_FILE "/psfonts/mtsansdk.ttf"
#define SYSTEM_DEFAULT_MONOSPACE_FAMILY "Monotype Sans Duospace WT K"
#elif defined( __ANDROID__ )
#define SYSTEM_DEFAULT_FONT_FILE "/system/fonts/Roboto-Regular.ttf"
#define SYSTEM_DEFAULT_FAMILY "sans-serif"
#define SYSTEM_DEFAULT_MONOSPACE_FONT_FILE "/system/fonts/DroidSansMono.ttf"
#define SYSTEM_DEFAULT_MONOSPACE_FAMILY "Monospace"
#else
#define SYSTEM_DEFAULT_FONT_FILE "/usr/share/fonts/truetype/freefont/FreeSerifBold.ttf"
#define SYSTEM_DEFAULT_FAMILY "Serif Bold"
#define SYSTEM_DEFAULT_MONOSPACE_FONT_FILE "/usr/share/fonts/truetype/freefont/FreeMono.ttf"
#define SYSTEM_DEFAULT_MONOSPACE_FAMILY "Monospace"
#endif
#if !defined(DEFAULT_FONT_FILE)
#define DEFAULT_FONT_FILE SYSTEM_DEFAULT_FONT_FILE
#endif
#if !defined(DEFAULT_FAMILY)
#define DEFAULT_FAMILY SYSTEM_DEFAULT_FAMILY
#endif
#if !defined(DEFAULT_MONOSPACE_FONT_FILE)
#define DEFAULT_MONOSPACE_FONT_FILE SYSTEM_DEFAULT_MONOSPACE_FONT_FILE
#endif
#if !defined(DEFAULT_MONOSPACE_FAMILY)
#define DEFAULT_MONOSPACE_FAMILY SYSTEM_DEFAULT_MONOSPACE_FAMILY
#endif
typedef struct vlc_font_t vlc_font_t;
struct vlc_font_t
{
vlc_font_t *p_next; 
char *psz_fontfile;
int i_index; 
bool b_bold; 
bool b_italic; 
FT_Face p_face; 
};
struct vlc_family_t
{
vlc_family_t *p_next; 
char *psz_name;
vlc_font_t *p_fonts; 
};
#define FB_LIST_ATTACHMENTS "attachments"
#define FB_LIST_DEFAULT "default"
#define FB_NAME "fallback"
#if defined(HAVE_FONTCONFIG)
vlc_family_t *FontConfig_GetFallbacks( filter_t *p_filter, const char *psz_family,
uni_char_t codepoint );
const vlc_family_t *FontConfig_GetFamily( filter_t *p_filter, const char *psz_family );
int FontConfig_Prepare( filter_t *p_filter );
void FontConfig_Unprepare( void );
#endif 
#if defined( _WIN32 )
const vlc_family_t *DWrite_GetFamily( filter_t *p_filter, const char *psz_family );
vlc_family_t *DWrite_GetFallbacks( filter_t *p_filter, const char *psz_family,
uni_char_t codepoint );
int InitDWrite( filter_t *p_filter );
int ReleaseDWrite( filter_t *p_filter );
int DWrite_GetFontStream( filter_t *p_filter, int i_index, FT_Stream *pp_stream );
#if !VLC_WINSTORE_APP
vlc_family_t *Win32_GetFallbacks( filter_t *p_filter, const char *psz_family,
uni_char_t codepoint );
const vlc_family_t *Win32_GetFamily( filter_t *p_filter, const char *psz_family );
#endif 
#endif 
#if defined(__APPLE__)
vlc_family_t *CoreText_GetFallbacks(filter_t *p_filter, const char *psz_family, uni_char_t codepoint);
const vlc_family_t *CoreText_GetFamily(filter_t *p_filter, const char *psz_family);
#endif 
#if defined(__ANDROID__)
const vlc_family_t *Android_GetFamily( filter_t *p_filter, const char *psz_family );
vlc_family_t *Android_GetFallbacks( filter_t *p_filter, const char *psz_family,
uni_char_t codepoint );
int Android_Prepare( filter_t *p_filter );
#endif 
char* Dummy_Select( filter_t *p_filter, const char* family,
bool b_bold, bool b_italic,
int *i_idx, uni_char_t codepoint );
#define File_Select(a) Dummy_Select(NULL, a, 0, 0, NULL, 0)
char* Generic_Select( filter_t *p_filter, const char* family,
bool b_bold, bool b_italic,
int *i_idx, uni_char_t codepoint );
vlc_family_t *NewFamily( filter_t *p_filter, const char *psz_family,
vlc_family_t **pp_list, vlc_dictionary_t *p_dict,
const char *psz_key );
vlc_font_t *NewFont( char *psz_fontfile, int i_index,
bool b_bold, bool b_italic,
vlc_family_t *p_parent );
void FreeFamiliesAndFonts( vlc_family_t *p_family );
void FreeFamilies( void *p_families, void *p_obj );
vlc_family_t *InitDefaultList( filter_t *p_filter, const char *const *ppsz_default,
int i_size );
#if defined(DEBUG_PLATFORM_FONTS)
void DumpFamily( filter_t *p_filter, const vlc_family_t *p_family,
bool b_dump_fonts, int i_max_families );
void DumpDictionary( filter_t *p_filter, const vlc_dictionary_t *p_dict,
bool b_dump_fonts, int i_max_families );
#endif
char* ToLower( const char *psz_src );
int ConvertToLiveSize( filter_t *p_filter, const text_style_t *p_style );
vlc_family_t *SearchFallbacks( filter_t *p_filter, vlc_family_t *p_fallbacks,
uni_char_t codepoint );
FT_Face GetFace( filter_t *p_filter, vlc_font_t *p_font );
#if defined(__cplusplus)
}
#endif
