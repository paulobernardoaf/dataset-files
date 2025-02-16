






















#if !defined(VLC_TEXT_STYLE_H)
#define VLC_TEXT_STYLE_H 1

#if defined(__cplusplus)
extern "C" {
#endif








typedef struct
{

char * psz_fontname; 
char * psz_monofontname; 

uint16_t i_features; 
uint16_t i_style_flags; 


float f_font_relsize; 
int i_font_size; 
int i_font_color; 

uint8_t i_font_alpha; 
int i_spacing; 


int i_outline_color; 
uint8_t i_outline_alpha; 
int i_outline_width; 


int i_shadow_color; 
uint8_t i_shadow_alpha; 
int i_shadow_width; 


int i_background_color;
uint8_t i_background_alpha;


enum
{
STYLE_WRAP_DEFAULT = 0, 
STYLE_WRAP_CHAR, 
STYLE_WRAP_NONE, 
} e_wrapinfo;
} text_style_t;

#define STYLE_ALPHA_OPAQUE 0xFF
#define STYLE_ALPHA_TRANSPARENT 0x00


#define STYLE_NO_DEFAULTS 0x0
#define STYLE_FULLY_SET 0xFFFF
#define STYLE_HAS_FONT_COLOR (1 << 0)
#define STYLE_HAS_FONT_ALPHA (1 << 1)
#define STYLE_HAS_FLAGS (1 << 2)
#define STYLE_HAS_OUTLINE_COLOR (1 << 3)
#define STYLE_HAS_OUTLINE_ALPHA (1 << 4)
#define STYLE_HAS_SHADOW_COLOR (1 << 5)
#define STYLE_HAS_SHADOW_ALPHA (1 << 6)
#define STYLE_HAS_BACKGROUND_COLOR (1 << 7)
#define STYLE_HAS_BACKGROUND_ALPHA (1 << 8)
#define STYLE_HAS_WRAP_INFO (1 << 9)


#define STYLE_BOLD (1 << 0)
#define STYLE_ITALIC (1 << 1)
#define STYLE_OUTLINE (1 << 2)
#define STYLE_SHADOW (1 << 3)
#define STYLE_BACKGROUND (1 << 4)
#define STYLE_UNDERLINE (1 << 5)
#define STYLE_STRIKEOUT (1 << 6)
#define STYLE_HALFWIDTH (1 << 7)
#define STYLE_MONOSPACED (1 << 8)
#define STYLE_DOUBLEWIDTH (1 << 9)
#define STYLE_BLINK_FOREGROUND (1 << 10)
#define STYLE_BLINK_BACKGROUND (1 << 11)

#define STYLE_DEFAULT_FONT_SIZE 20
#define STYLE_DEFAULT_REL_FONT_SIZE 6.25


typedef struct text_segment_t text_segment_t;
typedef struct text_segment_ruby_t text_segment_ruby_t;





struct text_segment_ruby_t
{
char *psz_base;
char *psz_rt;
text_segment_ruby_t *p_next;
};













struct text_segment_t {
char *psz_text; 
text_style_t *style; 
text_segment_t *p_next; 
text_segment_ruby_t *p_ruby; 
};




VLC_API text_style_t * text_style_New( void );










VLC_API text_style_t * text_style_Create( int );




VLC_API text_style_t * text_style_Copy( text_style_t *, const text_style_t * );




VLC_API text_style_t * text_style_Duplicate( const text_style_t * );






VLC_API void text_style_Merge( text_style_t *, const text_style_t *, bool b_override );




VLC_API void text_style_Delete( text_style_t * );









VLC_API text_segment_t *text_segment_New( const char * );









VLC_API text_segment_t *text_segment_NewInheritStyle( const text_style_t* p_style );






VLC_API void text_segment_Delete( text_segment_t * );







VLC_API void text_segment_ChainDelete( text_segment_t * );






VLC_API text_segment_t * text_segment_Copy( text_segment_t * );





VLC_API text_segment_ruby_t *text_segment_ruby_New( const char *psz_base,
const char *psz_rt );




VLC_API void text_segment_ruby_ChainDelete( text_segment_ruby_t *p_ruby );





VLC_API text_segment_t *text_segment_FromRuby( text_segment_ruby_t *p_ruby );










VLC_API unsigned int vlc_html_color( const char *psz_value, bool* ok );

#if defined(__cplusplus)
}
#endif

#endif 
