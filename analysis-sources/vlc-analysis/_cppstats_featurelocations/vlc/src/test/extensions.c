



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#undef NDEBUG

#include <assert.h>

#include <vlc_common.h>
#include <vlc_input_item.h>

static void check_extensions( const char* const* extensions, size_t nb_exts )
{
for( size_t i = 0; i < nb_exts - 1; i++ )
assert( strcmp( extensions[i], extensions[i + 1] ) < 0 );
}

#define CHECK_EXTENSION_WRAPPER( ext_list ) do { const char* const exts[] = { ext_list }; check_extensions( exts, ARRAY_SIZE( exts ) ); } while(0);






int main(void)
{
CHECK_EXTENSION_WRAPPER( MASTER_EXTENSIONS );
CHECK_EXTENSION_WRAPPER( SLAVE_SPU_EXTENSIONS );
CHECK_EXTENSION_WRAPPER( SLAVE_AUDIO_EXTENSIONS );
return 0;
}
