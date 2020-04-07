



























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ctype.h> 
#include <unistd.h>
#include <sys/stat.h>

#include <vlc_common.h>
#include <vlc_fs.h>
#include <vlc_url.h>

#include "input_internal.h"




static const char *const sub_exts[] = { SLAVE_SPU_EXTENSIONS, "" };

static void strcpy_trim( char *d, const char *s )
{
unsigned char c;


while( ((c = *s) != '\0') && !isalnum(c) )
{
s++;
}
for(;;)
{

while( ((c = *s) != '\0') && isalnum(c) )
{
*d = tolower(c);
s++; d++;
}
if( *s == 0 ) break;

while( ((c = *s) != '\0') && !isalnum(c) )
{
s++;
}
if( *s == 0 ) break;
*d++ = ' ';
}
*d = 0;
}

static void strcpy_strip_ext( char *d, const char *s )
{
unsigned char c;

const char *tmp = strrchr(s, '.');
if( !tmp )
{
strcpy(d, s);
return;
}
else
strlcpy(d, s, tmp - s + 1 );
while( (c = *d) != '\0' )
{
*d = tolower(c);
d++;
}
}

static void strcpy_get_ext( char *d, const char *s )
{
const char *tmp = strrchr(s, '.');
if( !tmp )
strcpy(d, "");
else
strcpy( d, tmp + 1 );
}

static int whiteonly( const char *s )
{
unsigned char c;

while( (c = *s) != '\0' )
{
if( isalnum( c ) )
return 0;
s++;
}
return 1;
}

static int slave_strcmp( const void *a, const void *b )
{
const input_item_slave_t *p_slave0 = *((const input_item_slave_t **) a);
const input_item_slave_t *p_slave1 = *((const input_item_slave_t **) b);

if( p_slave0 == NULL || p_slave1 == NULL )
return 0;


#if defined(HAVE_STRCOLL)
return strcoll( p_slave0->psz_uri, p_slave1->psz_uri );
#else
return strcmp( p_slave0->psz_uri, p_slave1->psz_uri );
#endif
}




int subtitles_Filter( const char *psz_dir_content )
{
const char *tmp = strrchr( psz_dir_content, '.');

if( !tmp )
return 0;
tmp++;

for( int i = 0; sub_exts[i][0]; i++ )
if( strcasecmp( sub_exts[i], tmp ) == 0 )
return 1;
return 0;
}





static char **paths_to_list( const char *psz_dir, char *psz_path )
{
unsigned int i, k, i_nb_subdirs;
char **subdirs; 
char *psz_parser = psz_path;

if( !psz_dir || !psz_path )
return NULL;

for( k = 0, i_nb_subdirs = 1; psz_path[k] != '\0'; k++ )
{
if( psz_path[k] == ',' )
i_nb_subdirs++;
}

subdirs = calloc( i_nb_subdirs + 1, sizeof(char*) );
if( !subdirs )
return NULL;

for( i = 0; psz_parser && *psz_parser != '\0' ; )
{
char *psz_subdir = psz_parser;
psz_parser = strchr( psz_subdir, ',' );
if( psz_parser )
{
*psz_parser++ = '\0';
while( *psz_parser == ' ' )
psz_parser++;
}
if( *psz_subdir == '\0' )
continue;

if( asprintf( &subdirs[i++], "%s%s",
psz_subdir[0] == '.' ? psz_dir : "",
psz_subdir ) == -1 )
break;
}
subdirs[i] = NULL;

return subdirs;
}

















int subtitles_Detect( input_thread_t *p_this, char *psz_path, const char *psz_name_org,
input_item_slave_t ***ppp_slaves, int *p_slaves )
{
int i_fuzzy = var_GetInteger( p_this, "sub-autodetect-fuzzy" );
if ( i_fuzzy == 0 )
return VLC_EGENERIC;
int i_fname_len;
input_item_slave_t **pp_slaves = *ppp_slaves;
int i_slaves = *p_slaves;
char *f_fname_noext = NULL, *f_fname_trim = NULL;
char **subdirs; 

if( !psz_name_org )
return VLC_EGENERIC;

char *psz_fname = vlc_uri2path( psz_name_org );
if( !psz_fname )
return VLC_EGENERIC;


char *f_dir = strdup( psz_fname );
if( f_dir == 0 )
{
free( psz_fname );
return VLC_ENOMEM;
}

const char *f_fname = strrchr( psz_fname, DIR_SEP_CHAR );
if( !f_fname )
{
free( f_dir );
free( psz_fname );
return VLC_EGENERIC;
}
f_fname++; 
f_dir[f_fname - psz_fname] = 0; 

i_fname_len = strlen( f_fname );

f_fname_noext = malloc(i_fname_len + 1);
f_fname_trim = malloc(i_fname_len + 1 );
if( !f_fname_noext || !f_fname_trim )
{
free( f_dir );
free( f_fname_noext );
free( f_fname_trim );
free( psz_fname );
return VLC_ENOMEM;
}

strcpy_strip_ext( f_fname_noext, f_fname );
strcpy_trim( f_fname_trim, f_fname_noext );

subdirs = paths_to_list( f_dir, psz_path );
for( int j = -1; (j == -1) || ( j >= 0 && subdirs != NULL && subdirs[j] != NULL ); j++ )
{
const char *psz_dir = (j < 0) ? f_dir : subdirs[j];
if( psz_dir == NULL || ( j >= 0 && !strcmp( psz_dir, f_dir ) ) )
continue;


DIR *dir = vlc_opendir( psz_dir );
if( dir == NULL )
continue;

msg_Dbg( p_this, "looking for a subtitle file in %s", psz_dir );

const char *psz_name;
while( (psz_name = vlc_readdir( dir )) )
{
if( psz_name[0] == '.' || !subtitles_Filter( psz_name ) )
continue;

char tmp_fname_noext[strlen( psz_name ) + 1];
char tmp_fname_trim[strlen( psz_name ) + 1];
char tmp_fname_ext[strlen( psz_name ) + 1];
const char *tmp;
int i_prio = 0;


strcpy_strip_ext( tmp_fname_noext, psz_name );
strcpy_get_ext( tmp_fname_ext, psz_name );
strcpy_trim( tmp_fname_trim, tmp_fname_noext );

if( !strcmp( tmp_fname_trim, f_fname_trim ) )
{

i_prio = SLAVE_PRIORITY_MATCH_ALL;
}
else if( (tmp = strstr( tmp_fname_trim, f_fname_trim )) )
{

tmp += strlen( f_fname_trim );
if( whiteonly( tmp ) )
{

i_prio = SLAVE_PRIORITY_MATCH_RIGHT;
}
else
{


i_prio = SLAVE_PRIORITY_MATCH_LEFT;
}
}
else if( j == -1 )
{

i_prio = SLAVE_PRIORITY_MATCH_NONE;
}
if( i_prio >= i_fuzzy )
{
struct stat st;
char *path;

size_t i_len = strlen( psz_dir );
const char *psz_format;
if ( i_len == 0 )
continue;
if( psz_dir[i_len - 1] == DIR_SEP_CHAR )
psz_format = "%s%s";
else
psz_format = "%s"DIR_SEP"%s";

if( asprintf( &path, psz_format, psz_dir, psz_name ) < 0 )
continue;

if( strcmp( path, psz_fname )
&& vlc_stat( path, &st ) == 0
&& S_ISREG( st.st_mode ) )
{
msg_Dbg( p_this,
"autodetected subtitle: %s with priority %d",
path, i_prio );
char *psz_uri = vlc_path2uri( path, NULL );
input_item_slave_t *p_sub = psz_uri != NULL ?
input_item_slave_New( psz_uri, SLAVE_TYPE_SPU, i_prio )
: NULL;
if( p_sub )
{
p_sub->b_forced = true;
TAB_APPEND(i_slaves, pp_slaves, p_sub);
}
free( psz_uri );
}
free( path );
}
}
closedir( dir );
}
if( subdirs )
{
for( size_t j = 0; subdirs[j] != NULL; j++ )
free( subdirs[j] );
free( subdirs );
}
free( f_dir );
free( f_fname_trim );
free( f_fname_noext );
free( psz_fname );

for( int i = 0; i < i_slaves; i++ )
{
input_item_slave_t *p_sub = pp_slaves[i];

bool b_reject = false;
char *psz_ext = strrchr( p_sub->psz_uri, '.' );
if( !psz_ext )
continue;
psz_ext++;

if( !strcasecmp( psz_ext, "sub" ) )
{
for( int j = 0; j < i_slaves; j++ )
{
input_item_slave_t *p_sub_inner = pp_slaves[j];


if( p_sub_inner == NULL )
continue;


if( strncasecmp( p_sub->psz_uri, p_sub_inner->psz_uri,
strlen( p_sub->psz_uri ) - 3 ) )
continue;

char *psz_ext_inner = strrchr( p_sub_inner->psz_uri, '.' );
if( !psz_ext_inner )
continue;
psz_ext_inner++;


if( !strcasecmp( psz_ext_inner, "idx" ) )
{
b_reject = true;
break;
}
}
}
else if( !strcasecmp( psz_ext, "cdg" ) )
{
if( p_sub->i_priority < SLAVE_PRIORITY_MATCH_ALL )
b_reject = true;
}
if( b_reject )
{
pp_slaves[i] = NULL;
input_item_slave_Delete( p_sub );
}
}


if( i_slaves > 0 )
qsort( pp_slaves, i_slaves, sizeof (input_item_slave_t*), slave_strcmp );

*ppp_slaves = pp_slaves; 
*p_slaves = i_slaves;
return VLC_SUCCESS;
}
