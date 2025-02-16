

























#if !defined(VLC_FREETYPE_H)
#define VLC_FREETYPE_H








#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_text_style.h> 
#include <vlc_arrays.h> 

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_STROKER_H


#define FT_FLOOR(X) ((X & -64) >> 6)
#define FT_CEIL(X) (((X + 63) & -64) >> 6)
#if !defined(FT_MulFix)
#define FT_MulFix(v, s) (((v)*(s))>>16)
#endif

#if defined(__OS2__)
typedef uint16_t uni_char_t;
#define FREETYPE_TO_UCS "UCS-2LE"
#else
typedef uint32_t uni_char_t;
#if defined(WORDS_BIGENDIAN)
#define FREETYPE_TO_UCS "UCS-4BE"
#else
#define FREETYPE_TO_UCS "UCS-4LE"
#endif
#endif







typedef struct vlc_family_t vlc_family_t;
typedef struct
{
FT_Library p_library; 
FT_Face p_face; 
FT_Stroker p_stroker; 

text_style_t *p_default_style;
text_style_t *p_forced_style; 


float f_shadow_vector_x;
float f_shadow_vector_y;


input_attachment_t **pp_font_attachments;
int i_font_attachments;





vlc_family_t *p_families;




vlc_dictionary_t family_map;






vlc_dictionary_t fallback_map;


vlc_dictionary_t face_map;

int i_fallback_counter;


int i_scale;




char * (*pf_select) (filter_t *, const char* family,
bool bold, bool italic,
int *index, uni_char_t codepoint);





const vlc_family_t * (*pf_get_family) ( filter_t *p_filter, const char *psz_family );







vlc_family_t * (*pf_get_fallbacks) ( filter_t *p_filter, const char *psz_family,
uni_char_t codepoint );

#if defined( _WIN32 )
void *p_dw_sys;
#endif
} filter_sys_t;








FT_Face SelectAndLoadFace( filter_t *p_filter, const text_style_t *p_style,
uni_char_t codepoint );

static inline void BBoxInit( FT_BBox *p_box )
{
p_box->xMin = INT_MAX;
p_box->yMin = INT_MAX;
p_box->xMax = INT_MIN;
p_box->yMax = INT_MIN;
}

static inline void BBoxEnlarge( FT_BBox *p_max, const FT_BBox *p )
{
p_max->xMin = __MIN(p_max->xMin, p->xMin);
p_max->yMin = __MIN(p_max->yMin, p->yMin);
p_max->xMax = __MAX(p_max->xMax, p->xMax);
p_max->yMax = __MAX(p_max->yMax, p->yMax);
}


#endif
