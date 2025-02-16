



















#if !defined(VLC_FINGERPRINTER_H)
#define VLC_FINGERPRINTER_H

#include <vlc_common.h>
#include <vlc_meta.h>
#include <vlc_input_item.h>
#include <vlc_arrays.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct fingerprinter_sys_t fingerprinter_sys_t;

struct fingerprint_request_t
{
input_item_t *p_item;
unsigned int i_duration; 
struct
{
char *psz_fingerprint;
vlc_array_t metas_array;
} results ;
};
typedef struct fingerprint_request_t fingerprint_request_t;

static inline fingerprint_request_t *fingerprint_request_New( input_item_t *p_item )
{
fingerprint_request_t *p_r =
( fingerprint_request_t * ) calloc( 1, sizeof( fingerprint_request_t ) );
if ( !p_r ) return NULL;
p_r->results.psz_fingerprint = NULL;
p_r->i_duration = 0;
input_item_Hold( p_item );
p_r->p_item = p_item;
vlc_array_init( & p_r->results.metas_array ); 
return p_r;
}

static inline void fingerprint_request_Delete( fingerprint_request_t *p_f )
{
input_item_Release( p_f->p_item );
free( p_f->results.psz_fingerprint );
for( size_t i = 0; i < vlc_array_count( & p_f->results.metas_array ); i++ )
vlc_meta_Delete( (vlc_meta_t *) vlc_array_item_at_index( & p_f->results.metas_array, i ) );
free( p_f );
}

struct fingerprinter_thread_t
{
struct vlc_object_t obj;


fingerprinter_sys_t * p_sys;

module_t * p_module;

int ( *pf_enqueue ) ( struct fingerprinter_thread_t *f, fingerprint_request_t *r );
fingerprint_request_t * ( *pf_getresults ) ( struct fingerprinter_thread_t *f );
void ( *pf_apply ) ( fingerprint_request_t *, size_t i_resultid );
};
typedef struct fingerprinter_thread_t fingerprinter_thread_t;

VLC_API fingerprinter_thread_t *fingerprinter_Create( vlc_object_t *p_this );
VLC_API void fingerprinter_Destroy( fingerprinter_thread_t *p_fingerprint );

#if defined(__cplusplus)
}
#endif

#endif
