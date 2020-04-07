





















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <vlc_common.h>
#include <vlc_md5.h>

typedef struct
{
const char *psz_string;
const char *psz_md5;
} md5_sample_t;

static const md5_sample_t md5_samples[] =
{
{ "", "d41d8cd98f00b204e9800998ecf8427e" },
{ "a", "0cc175b9c0f1b6a831c399e269772661" },
{ "abc", "900150983cd24fb0d6963f7d28e17f72" },
{ "message digest", "f96b697d7cb7938d525a2f31aaf161d0" },
{ "abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b" },
{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
"d174ab98d277d9f5a5611c2c9f419d9f" },
{ "12345678901234567890123456789012345678901234567890123456789012345678901"
"234567890", "57edf4a22be3c955ac49da2e2107b67a" },
{ "azertyuiop", "7682fe272099ea26efe39c890b33675b" },
{ NULL, NULL }
};

static void test_config_StringEscape()
{
for( int i = 0; md5_samples[i].psz_string; i++ )
{
struct md5_s md5;
InitMD5( &md5 );
AddMD5( &md5, md5_samples[i].psz_string, strlen( md5_samples[i].psz_string ) );
EndMD5( &md5 );
char * psz_hash = psz_md5_hash( &md5 );

if( strcmp( psz_hash, md5_samples[i].psz_md5 ) )
{
printf( "Output: %s\nExpected: %s\n", psz_hash,
md5_samples[i].psz_md5 );
abort();
}
free( psz_hash );
}
}

int main( void )
{
test_config_StringEscape();

return 0;
}
