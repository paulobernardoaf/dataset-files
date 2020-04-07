#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#undef NDEBUG
#include <assert.h>
#include <vlc_common.h>
#include "vlc_arrays.h"
#include <stdio.h>
#include <stdlib.h>
static void test_dictionary_validity (vlc_dictionary_t * p_dict, const char ** our_keys, int size )
{
char ** keys = vlc_dictionary_all_keys( p_dict );
intptr_t i, j;
assert( keys );
for( j = 0; keys[j]; j++ )
{
bool found = false;
for( i = 0; i < size; i++ )
{
if(!strcmp( keys[j], our_keys[i] ))
{
found = true;
break;
}
}
free( keys[j] );
assert( found );
}
free( keys );
for( i = 0; i < size; i++ )
assert( vlc_dictionary_value_for_key( p_dict, our_keys[i] ) == (void *)i );
}
int main (void)
{
static const char * our_keys[] = {
"Hello", "Hella", "flowmeter", "Frostnipped", "frostnipped", "remiform", "quadrifoliolate", "singularity", "unafflicted"
};
const int size = ARRAY_SIZE(our_keys);
char ** keys;
intptr_t i = 0;
vlc_dictionary_t dict;
vlc_dictionary_init( &dict, 0 );
assert( vlc_dictionary_keys_count( &dict ) == 0 );
assert( vlc_dictionary_is_empty( &dict ) );
keys = vlc_dictionary_all_keys( &dict );
assert( keys && !keys[0] );
free(keys);
for( i = 0; i < size; i++ )
{
vlc_dictionary_insert( &dict, our_keys[i], (void *)i );
assert( vlc_dictionary_has_key(&dict, our_keys[i]) );
for( int j = i + 1; j < size; j++ )
assert( !vlc_dictionary_has_key(&dict, our_keys[j]) );
}
assert( !vlc_dictionary_is_empty( &dict ) );
test_dictionary_validity( &dict, our_keys, size );
vlc_dictionary_remove_value_for_key( &dict, our_keys[size-1], NULL, NULL );
test_dictionary_validity( &dict, our_keys, size-1 );
vlc_dictionary_clear( &dict, NULL, NULL );
assert( vlc_dictionary_keys_count( &dict ) == 0 );
assert( vlc_dictionary_is_empty( &dict ) );
return 0;
}
