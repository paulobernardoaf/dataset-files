

























#if !defined(VLC_COMMON_H)
#define VLC_COMMON_H 1











#include "vlc_config.h"




#include <stdlib.h>
#include <stdarg.h>

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>

#if !defined(__cplusplus)
#include <stdbool.h>
#endif










#if defined(__GNUC__)
#define VLC_GCC_VERSION(maj,min) ((__GNUC__ > (maj)) || (__GNUC__ == (maj) && __GNUC_MINOR__ >= (min)))

#else

#define VLC_GCC_VERSION(maj,min) (0)
#endif


#if defined( _WIN32 ) && defined( __USE_MINGW_ANSI_STDIO )
#undef PRId64
#define PRId64 "lld"
#undef PRIi64
#define PRIi64 "lli"
#undef PRIu64
#define PRIu64 "llu"
#undef PRIo64
#define PRIo64 "llo"
#undef PRIx64
#define PRIx64 "llx"
#define snprintf __mingw_snprintf
#define vsnprintf __mingw_vsnprintf
#define swprintf _snwprintf
#endif


#if defined(__GNUC__)
#define VLC_DEPRECATED __attribute__((deprecated))
#if VLC_GCC_VERSION(6,0)
#define VLC_DEPRECATED_ENUM __attribute__((deprecated))
#else
#define VLC_DEPRECATED_ENUM
#endif

#if defined( _WIN32 ) && !defined( __clang__ )
#define VLC_FORMAT(x,y) __attribute__ ((format(gnu_printf,x,y)))
#else
#define VLC_FORMAT(x,y) __attribute__ ((format(printf,x,y)))
#endif
#define VLC_FORMAT_ARG(x) __attribute__ ((format_arg(x)))
#define VLC_MALLOC __attribute__ ((malloc))
#define VLC_USED __attribute__ ((warn_unused_result))

#else









#define VLC_DEPRECATED









#define VLC_DEPRECATED_ENUM











#define VLC_FORMAT(x,y)










#define VLC_FORMAT_ARG(x)














#define VLC_MALLOC

















#define VLC_USED
#endif

#if defined (__ELF__) || defined (__MACH__)
#define VLC_WEAK __attribute__((weak))
#else







#define VLC_WEAK
#endif


#if defined (__GNUC__) || defined (__clang__)
#define likely(p) __builtin_expect(!!(p), 1)
#define unlikely(p) __builtin_expect(!!(p), 0)
#define unreachable() __builtin_unreachable()
#else







#define likely(p) (!!(p))








#define unlikely(p) (!!(p))











#define unreachable() ((void)0)
#endif












#define vlc_assert_unreachable() (vlc_assert(!"unreachable"), unreachable())














#if defined(__LIBVLC__)
#define vlc_assert(pred) assert(pred)
#else
#define vlc_assert(pred) ((void)0)
#endif


#if defined(__cplusplus)
#define VLC_EXTERN extern "C"
#else
#define VLC_EXTERN
#endif

#if defined (_WIN32) && defined (DLL_EXPORT)
#define VLC_EXPORT __declspec(dllexport)
#elif defined (__GNUC__)
#define VLC_EXPORT __attribute__((visibility("default")))
#else
#define VLC_EXPORT
#endif







#define VLC_API VLC_EXTERN VLC_EXPORT











typedef uint32_t vlc_fourcc_t;

#if defined(WORDS_BIGENDIAN)
#define VLC_FOURCC( a, b, c, d ) ( ((uint32_t)d) | ( ((uint32_t)c) << 8 ) | ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) )


#define VLC_TWOCC( a, b ) ( (uint16_t)(b) | ( (uint16_t)(a) << 8 ) )


#else
#define VLC_FOURCC( a, b, c, d ) ( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )


#define VLC_TWOCC( a, b ) ( (uint16_t)(a) | ( (uint16_t)(b) << 8 ) )


#endif








static inline void vlc_fourcc_to_char( vlc_fourcc_t fcc, char *psz_fourcc )
{
memcpy( psz_fourcc, &fcc, 4 );
}






typedef struct vlc_object_t vlc_object_t;
typedef struct libvlc_int_t libvlc_int_t;
typedef struct date_t date_t;



typedef struct playlist_t playlist_t;
typedef struct playlist_item_t playlist_item_t;
typedef struct services_discovery_t services_discovery_t;
typedef struct vlc_renderer_discovery_t vlc_renderer_discovery_t;
typedef struct vlc_renderer_item_t vlc_renderer_item_t;


typedef struct module_t module_t;
typedef struct module_config_t module_config_t;

typedef struct config_category_t config_category_t;


typedef struct input_item_t input_item_t;
typedef struct input_item_node_t input_item_node_t;
typedef struct input_source_t input_source_t;
typedef struct stream_t stream_t;
typedef struct stream_t demux_t;
typedef struct es_out_t es_out_t;
typedef struct es_out_id_t es_out_id_t;
typedef struct seekpoint_t seekpoint_t;
typedef struct info_t info_t;
typedef struct info_category_t info_category_t;
typedef struct input_attachment_t input_attachment_t;


typedef struct audio_format_t audio_format_t;
typedef struct video_format_t video_format_t;
typedef struct subs_format_t subs_format_t;
typedef struct es_format_t es_format_t;
typedef struct video_palette_t video_palette_t;
typedef struct vlc_es_id_t vlc_es_id_t;


typedef struct audio_output audio_output_t;
typedef audio_format_t audio_sample_format_t;


typedef struct vout_thread_t vout_thread_t;
typedef struct vlc_viewpoint_t vlc_viewpoint_t;

typedef video_format_t video_frame_format_t;
typedef struct picture_t picture_t;


typedef struct spu_t spu_t;
typedef struct subpicture_t subpicture_t;
typedef struct subpicture_region_t subpicture_region_t;

typedef struct image_handler_t image_handler_t;


typedef struct sout_instance_t sout_instance_t;

typedef struct sout_input_t sout_input_t;
typedef struct sout_packetizer_input_t sout_packetizer_input_t;

typedef struct sout_access_out_t sout_access_out_t;

typedef struct sout_mux_t sout_mux_t;

typedef struct sout_stream_t sout_stream_t;

typedef struct config_chain_t config_chain_t;
typedef struct session_descriptor_t session_descriptor_t;


typedef struct decoder_t decoder_t;


typedef struct encoder_t encoder_t;


typedef struct filter_t filter_t;


typedef struct vlc_url_t vlc_url_t;


typedef struct iso639_lang_t iso639_lang_t;


typedef struct block_t block_t;
typedef struct block_fifo_t block_fifo_t;


typedef struct md5_s md5_t;


typedef struct xml_t xml_t;
typedef struct xml_reader_t xml_reader_t;


typedef struct vod_t vod_t;
typedef struct vod_media_t vod_media_t;


typedef struct vlm_t vlm_t;
typedef struct vlm_message_t vlm_message_t;


typedef struct vlc_meta_t vlc_meta_t;
typedef struct input_stats_t input_stats_t;
typedef struct addon_entry_t addon_entry_t;


typedef struct update_t update_t;




typedef union
{
int64_t i_int;
bool b_bool;
float f_float;
char * psz_string;
void * p_address;
struct { int32_t x; int32_t y; } coords;

} vlc_value_t;





#define VLC_SUCCESS (-0)

#define VLC_EGENERIC (-1)

#define VLC_ENOMEM (-2)

#define VLC_ETIMEOUT (-3)

#define VLC_ENOMOD (-4)

#define VLC_ENOOBJ (-5)

#define VLC_ENOVAR (-6)

#define VLC_EBADVAR (-7)

#define VLC_ENOITEM (-8)




typedef int ( * vlc_callback_t ) ( vlc_object_t *, 
char const *, 
vlc_value_t, 
vlc_value_t, 
void * ); 




typedef int ( * vlc_list_callback_t ) ( vlc_object_t *, 
char const *, 
int, 
vlc_value_t *, 
void *); 




#if defined( _WIN32 )
#include <malloc.h>
#if !defined(PATH_MAX)
#define PATH_MAX MAX_PATH
#endif
#include <windows.h>
#endif

#if defined(__APPLE__)
#include <sys/syslimits.h>
#include <AvailabilityMacros.h>
#endif

#if defined(__OS2__)
#define OS2EMX_PLAIN_CHAR
#define INCL_BASE
#define INCL_PM
#include <os2safe.h>
#include <os2.h>
#endif

#include "vlc_tick.h"
#include "vlc_threads.h"









#if !defined(__MAX)
#define __MAX(a, b) ( ((a) > (b)) ? (a) : (b) )
#endif
#if !defined(__MIN)
#define __MIN(a, b) ( ((a) < (b)) ? (a) : (b) )
#endif


#define VLC_CLIP(v, min, max) __MIN(__MAX((v), (min)), (max))






VLC_USED
static inline size_t vlc_align(size_t v, size_t align)
{
return (v + (align - 1)) & ~(align - 1);
}

#if defined(__clang__) && __has_attribute(diagnose_if)
static inline size_t vlc_align(size_t v, size_t align)
__attribute__((diagnose_if(((align & (align - 1)) || (align == 0)),
"align must be power of 2", "error")));
#endif


VLC_USED
static inline int64_t GCD ( int64_t a, int64_t b )
{
while( b )
{
int64_t c = a % b;
a = b;
b = c;
}
return a;
}


VLC_USED
static inline uint8_t clip_uint8_vlc( int32_t a )
{
if( a&(~255) ) return (-a)>>31;
else return a;
}






#define VLC_INT_FUNC(basename) VLC_INT_FUNC_TYPE(basename, unsigned, ) VLC_INT_FUNC_TYPE(basename, unsigned long, l) VLC_INT_FUNC_TYPE(basename, unsigned long long, ll)




#if defined (__GNUC__) || defined (__clang__)
#define VLC_INT_FUNC_TYPE(basename,type,suffix) VLC_USED static inline int vlc_##basename##suffix(type x) { return __builtin_##basename##suffix(x); }





VLC_INT_FUNC(clz)
#else
VLC_USED static inline int vlc_clzll(unsigned long long x)
{
int i = sizeof (x) * 8;

while (x)
{
x >>= 1;
i--;
}
return i;
}

VLC_USED static inline int vlc_clzl(unsigned long x)
{
return vlc_clzll(x) - ((sizeof (long long) - sizeof (long)) * 8);
}

VLC_USED static inline int vlc_clz(unsigned x)
{
return vlc_clzll(x) - ((sizeof (long long) - sizeof (int)) * 8);
}

VLC_USED static inline int vlc_ctz_generic(unsigned long long x)
{
unsigned i = sizeof (x) * 8;

while (x)
{
x <<= 1;
i--;
}
return i;
}

VLC_USED static inline int vlc_parity_generic(unsigned long long x)
{
for (unsigned i = 4 * sizeof (x); i > 0; i /= 2)
x ^= x >> i;
return x & 1;
}

VLC_USED static inline int vlc_popcount_generic(unsigned long long x)
{
int count = 0;
while (x)
{
count += x & 1;
x = x >> 1;
}
return count;
}

#define VLC_INT_FUNC_TYPE(basename,type,suffix) VLC_USED static inline int vlc_##basename##suffix(type x) { return vlc_##basename##_generic(x); }




#endif

VLC_INT_FUNC(ctz)
VLC_INT_FUNC(parity)
VLC_INT_FUNC(popcount)

#if !defined(__cplusplus)
#define VLC_INT_GENERIC(func,x) _Generic((x), unsigned char: func(x), signed char: func(x), unsigned short: func(x), signed short: func(x), unsigned int: func(x), signed int: func(x), unsigned long: func##l(x), signed long: func##l(x), unsigned long long: func##ll(x), signed long long: func##ll(x))























#define clz(x) _Generic((x), unsigned char: (vlc_clz(x) - (sizeof (unsigned) - 1) * 8), unsigned short: (vlc_clz(x) - (sizeof (unsigned) - sizeof (unsigned short)) * 8), unsigned: vlc_clz(x), unsigned long: vlc_clzl(x), unsigned long long: vlc_clzll(x))


















#define ctz(x) VLC_INT_GENERIC(vlc_ctz, x)








#define parity(x) VLC_INT_GENERIC(vlc_parity, x)








#define vlc_popcount(x) _Generic((x), signed char: vlc_popcount((unsigned char)(x)), signed short: vlc_popcount((unsigned short)(x)), default: VLC_INT_GENERIC(vlc_popcount ,x))




#else
VLC_USED static inline int vlc_popcount(unsigned char x)
{
return vlc_popcount((unsigned)x);
}

VLC_USED static inline int vlc_popcount(unsigned short x)
{
return vlc_popcount((unsigned)x);
}

VLC_USED static inline int vlc_popcount(unsigned long x)
{
return vlc_popcountl(x);
}

VLC_USED static inline int vlc_popcount(unsigned long long x)
{
return vlc_popcountll(x);
}
#endif


VLC_USED
static inline uint16_t vlc_bswap16(uint16_t x)
{
return (x << 8) | (x >> 8);
}


VLC_USED
static inline uint32_t vlc_bswap32(uint32_t x)
{
#if defined (__GNUC__) || defined(__clang__)
return __builtin_bswap32 (x);
#else
return ((x & 0x000000FF) << 24)
| ((x & 0x0000FF00) << 8)
| ((x & 0x00FF0000) >> 8)
| ((x & 0xFF000000) >> 24);
#endif
}


VLC_USED
static inline uint64_t vlc_bswap64(uint64_t x)
{
#if defined (__GNUC__) || defined(__clang__)
return __builtin_bswap64 (x);
#elif !defined (__cplusplus)
return ((x & 0x00000000000000FF) << 56)
| ((x & 0x000000000000FF00) << 40)
| ((x & 0x0000000000FF0000) << 24)
| ((x & 0x00000000FF000000) << 8)
| ((x & 0x000000FF00000000) >> 8)
| ((x & 0x0000FF0000000000) >> 24)
| ((x & 0x00FF000000000000) >> 40)
| ((x & 0xFF00000000000000) >> 56);
#else
return ((x & 0x00000000000000FFULL) << 56)
| ((x & 0x000000000000FF00ULL) << 40)
| ((x & 0x0000000000FF0000ULL) << 24)
| ((x & 0x00000000FF000000ULL) << 8)
| ((x & 0x000000FF00000000ULL) >> 8)
| ((x & 0x0000FF0000000000ULL) >> 24)
| ((x & 0x00FF000000000000ULL) >> 40)
| ((x & 0xFF00000000000000ULL) >> 56);
#endif
}






static inline bool uadd_overflow(unsigned a, unsigned b, unsigned *res)
{
#if VLC_GCC_VERSION(5,0) || defined(__clang__)
return __builtin_uadd_overflow(a, b, res);
#else
*res = a + b;
return (a + b) < a;
#endif
}

static inline bool uaddl_overflow(unsigned long a, unsigned long b,
unsigned long *res)
{
#if VLC_GCC_VERSION(5,0) || defined(__clang__)
return __builtin_uaddl_overflow(a, b, res);
#else
*res = a + b;
return (a + b) < a;
#endif
}

static inline bool uaddll_overflow(unsigned long long a, unsigned long long b,
unsigned long long *res)
{
#if VLC_GCC_VERSION(5,0) || defined(__clang__)
return __builtin_uaddll_overflow(a, b, res);
#else
*res = a + b;
return (a + b) < a;
#endif
}

#if !defined(__cplusplus)













#define add_overflow(a,b,r) _Generic(*(r), unsigned: uadd_overflow(a, b, (unsigned *)(r)), unsigned long: uaddl_overflow(a, b, (unsigned long *)(r)), unsigned long long: uaddll_overflow(a, b, (unsigned long long *)(r)))




#else
static inline bool add_overflow(unsigned a, unsigned b, unsigned *res)
{
return uadd_overflow(a, b, res);
}

static inline bool add_overflow(unsigned long a, unsigned long b,
unsigned long *res)
{
return uaddl_overflow(a, b, res);
}

static inline bool add_overflow(unsigned long long a, unsigned long long b,
unsigned long long *res)
{
return uaddll_overflow(a, b, res);
}
#endif

#if !(VLC_GCC_VERSION(5,0) || defined(__clang__))
#include <limits.h>
#endif

static inline bool umul_overflow(unsigned a, unsigned b, unsigned *res)
{
#if VLC_GCC_VERSION(5,0) || defined(__clang__)
return __builtin_umul_overflow(a, b, res);
#else
*res = a * b;
return b > 0 && a > (UINT_MAX / b);
#endif
}

static inline bool umull_overflow(unsigned long a, unsigned long b,
unsigned long *res)
{
#if VLC_GCC_VERSION(5,0) || defined(__clang__)
return __builtin_umull_overflow(a, b, res);
#else
*res = a * b;
return b > 0 && a > (ULONG_MAX / b);
#endif
}

static inline bool umulll_overflow(unsigned long long a, unsigned long long b,
unsigned long long *res)
{
#if VLC_GCC_VERSION(5,0) || defined(__clang__)
return __builtin_umulll_overflow(a, b, res);
#else
*res = a * b;
return b > 0 && a > (ULLONG_MAX / b);
#endif
}

#if !defined(__cplusplus)













#define mul_overflow(a,b,r) _Generic(*(r), unsigned: umul_overflow(a, b, (unsigned *)(r)), unsigned long: umull_overflow(a, b, (unsigned long *)(r)), unsigned long long: umulll_overflow(a, b, (unsigned long long *)(r)))




#else
static inline bool mul_overflow(unsigned a, unsigned b, unsigned *res)
{
return umul_overflow(a, b, res);
}

static inline bool mul_overflow(unsigned long a, unsigned long b,
unsigned long *res)
{
return umull_overflow(a, b, res);
}

static inline bool mul_overflow(unsigned long long a, unsigned long long b,
unsigned long long *res)
{
return umulll_overflow(a, b, res);
}
#endif




#define FREENULL(a) do { free( a ); a = NULL; } while(0)

#define EMPTY_STR(str) (!str || !*str)

#include <vlc_arrays.h>




#if defined(WORDS_BIGENDIAN)
#define hton16(i) ((uint16_t)(i))
#define hton32(i) ((uint32_t)(i))
#define hton64(i) ((uint64_t)(i))
#else
#define hton16(i) vlc_bswap16(i)
#define hton32(i) vlc_bswap32(i)
#define hton64(i) vlc_bswap64(i)
#endif
#define ntoh16(i) hton16(i)
#define ntoh32(i) hton32(i)
#define ntoh64(i) hton64(i)


VLC_USED
static inline uint16_t U16_AT (const void *p)
{
uint16_t x;

memcpy (&x, p, sizeof (x));
return ntoh16 (x);
}


VLC_USED
static inline uint32_t U32_AT (const void *p)
{
uint32_t x;

memcpy (&x, p, sizeof (x));
return ntoh32 (x);
}


VLC_USED
static inline uint64_t U64_AT (const void *p)
{
uint64_t x;

memcpy (&x, p, sizeof (x));
return ntoh64 (x);
}

#define GetWBE(p) U16_AT(p)
#define GetDWBE(p) U32_AT(p)
#define GetQWBE(p) U64_AT(p)


VLC_USED
static inline uint16_t GetWLE (const void *p)
{
uint16_t x;

memcpy (&x, p, sizeof (x));
#if defined(WORDS_BIGENDIAN)
x = vlc_bswap16 (x);
#endif
return x;
}


VLC_USED
static inline uint32_t GetDWLE (const void *p)
{
uint32_t x;

memcpy (&x, p, sizeof (x));
#if defined(WORDS_BIGENDIAN)
x = vlc_bswap32 (x);
#endif
return x;
}


VLC_USED
static inline uint64_t GetQWLE (const void *p)
{
uint64_t x;

memcpy (&x, p, sizeof (x));
#if defined(WORDS_BIGENDIAN)
x = vlc_bswap64 (x);
#endif
return x;
}


static inline void SetWBE (void *p, uint16_t w)
{
w = hton16 (w);
memcpy (p, &w, sizeof (w));
}


static inline void SetDWBE (void *p, uint32_t dw)
{
dw = hton32 (dw);
memcpy (p, &dw, sizeof (dw));
}


static inline void SetQWBE (void *p, uint64_t qw)
{
qw = hton64 (qw);
memcpy (p, &qw, sizeof (qw));
}


static inline void SetWLE (void *p, uint16_t w)
{
#if defined(WORDS_BIGENDIAN)
w = vlc_bswap16 (w);
#endif
memcpy (p, &w, sizeof (w));
}


static inline void SetDWLE (void *p, uint32_t dw)
{
#if defined(WORDS_BIGENDIAN)
dw = vlc_bswap32 (dw);
#endif
memcpy (p, &dw, sizeof (dw));
}


static inline void SetQWLE (void *p, uint64_t qw)
{
#if defined(WORDS_BIGENDIAN)
qw = vlc_bswap64 (qw);
#endif
memcpy (p, &qw, sizeof (qw));
}


#define VLC_UNUSED(x) (void)(x)



#if defined(_WIN32)

#if defined( __MINGW32__ )
#if !defined( _OFF_T_ )
typedef long long _off_t;
typedef _off_t off_t;
#define _OFF_T_
#else
#if defined(off_t)
#undef off_t
#endif
#define off_t long long
#endif
#endif

#if !defined(O_NONBLOCK)
#define O_NONBLOCK 0
#endif



#define swab(a,b,c) swab((char*) (a), (char*) (b), (c))

#endif 

typedef struct {
unsigned num, den;
} vlc_rational_t;

VLC_API bool vlc_ureduce( unsigned *, unsigned *, uint64_t, uint64_t, uint64_t );

#define container_of(ptr, type, member) ((type *)(((char *)(ptr)) - offsetof(type, member)))


VLC_USED VLC_MALLOC
static inline void *vlc_alloc(size_t count, size_t size)
{
return mul_overflow(count, size, &size) ? NULL : malloc(size);
}

VLC_USED
static inline void *vlc_reallocarray(void *ptr, size_t count, size_t size)
{
return mul_overflow(count, size, &size) ? NULL : realloc(ptr, size);
}




VLC_API const char *vlc_gettext(const char *msgid) VLC_FORMAT_ARG(1);
VLC_API const char *vlc_ngettext(const char *s, const char *p, unsigned long n)
VLC_FORMAT_ARG(1) VLC_FORMAT_ARG(2);

#define vlc_pgettext( ctx, id ) vlc_pgettext_aux( ctx "\004" id, id )


VLC_FORMAT_ARG(2)
static inline const char *vlc_pgettext_aux( const char *ctx, const char *id )
{
const char *tr = vlc_gettext( ctx );
return (tr == ctx) ? id : tr;
}




static inline void *xmalloc(size_t len)
{
void *ptr = malloc(len);
if (unlikely(ptr == NULL && len > 0))
abort();
return ptr;
}

static inline void *xrealloc(void *ptr, size_t len)
{
void *nptr = realloc(ptr, len);
if (unlikely(nptr == NULL && len > 0))
abort();
return nptr;
}

static inline char *xstrdup (const char *str)
{
char *ptr = strdup (str);
if (unlikely(ptr == NULL))
abort ();
return ptr;
}




VLC_API const char * VLC_CompileBy( void ) VLC_USED;
VLC_API const char * VLC_CompileHost( void ) VLC_USED;
VLC_API const char * VLC_Compiler( void ) VLC_USED;




#include "vlc_messages.h"
#include "vlc_objects.h"
#include "vlc_variables.h"
#include "vlc_configuration.h"

#if defined( _WIN32 ) || defined( __OS2__ )
#define DIR_SEP_CHAR '\\'
#define DIR_SEP "\\"
#define PATH_SEP_CHAR ';'
#define PATH_SEP ";"
#else
#define DIR_SEP_CHAR '/'
#define DIR_SEP "/"
#define PATH_SEP_CHAR ':'
#define PATH_SEP ":"
#endif

#define LICENSE_MSG _("This program comes with NO WARRANTY, to the extent permitted by " "law.\nYou may redistribute it under the terms of the GNU General " "Public License;\nsee the file named COPYING for details.\n" "Written by the VideoLAN team; see the AUTHORS file.\n")





#endif 
