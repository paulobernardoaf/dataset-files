#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_access.h>
#include <vlc_input.h>
#include <vlc_fs.h>
#include <vlc_charset.h>
#include <vlc_dialog.h>
#include <vlc_configuration.h>
static int Open ( vlc_object_t * );
static void Close( vlc_object_t * );
#define HELP_TEXT N_("Support for VDR recordings (http://www.tvdr.de/).")
#define CHAPTER_OFFSET_TEXT N_("Chapter offset in ms")
#define CHAPTER_OFFSET_LONGTEXT N_( "Move all chapters. This value should be set in milliseconds." )
#define FPS_TEXT N_("Frame rate")
#define FPS_LONGTEXT N_( "Default frame rate for chapter import." )
vlc_module_begin ()
set_category( CAT_INPUT )
set_shortname( N_("VDR") )
set_help( HELP_TEXT )
set_subcategory( SUBCAT_INPUT_ACCESS )
set_description( N_("VDR recordings") )
add_integer( "vdr-chapter-offset", 0,
CHAPTER_OFFSET_TEXT, CHAPTER_OFFSET_LONGTEXT, true )
add_float_with_range( "vdr-fps", 25, 1, 1000,
FPS_TEXT, FPS_LONGTEXT, true )
set_capability( "access", 60 )
add_shortcut( "vdr" )
add_shortcut( "directory" )
add_shortcut( "dir" )
add_shortcut( "file" )
set_callbacks( Open, Close )
vlc_module_end ()
#define MIN_CHAPTER_SIZE 5
TYPEDEF_ARRAY( uint64_t, size_array_t );
typedef struct
{
size_array_t file_sizes;
uint64_t offset;
uint64_t size; 
unsigned i_current_file;
int fd;
vlc_meta_t *p_meta;
input_title_t *p_marks;
uint64_t *offsets;
unsigned cur_seekpoint;
float fps;
bool b_ts_format;
} access_sys_t;
#define CURRENT_FILE_SIZE ARRAY_VAL(p_sys->file_sizes, p_sys->i_current_file)
#define FILE_SIZE(pos) ARRAY_VAL(p_sys->file_sizes, pos)
#define FILE_COUNT (unsigned)p_sys->file_sizes.i_size
static int Control( stream_t *, int, va_list );
static ssize_t Read( stream_t *p_access, void *p_buffer, size_t i_len );
static int Seek( stream_t *p_access, uint64_t i_pos);
static void FindSeekpoint( stream_t *p_access );
static bool ScanDirectory( stream_t *p_access );
static char *GetFilePath( stream_t *p_access, unsigned i_file );
static bool ImportNextFile( stream_t *p_access );
static bool SwitchFile( stream_t *p_access, unsigned i_file );
static void OptimizeForRead( int fd );
static void UpdateFileSize( stream_t *p_access );
static FILE *OpenRelativeFile( stream_t *p_access, const char *psz_file );
static bool ReadLine( char **ppsz_line, size_t *pi_size, FILE *p_file );
static void ImportMeta( stream_t *p_access );
static void ImportMarks( stream_t *p_access );
static bool ReadIndexRecord( FILE *p_file, bool b_ts, int64_t i_frame,
uint64_t *pi_offset, uint16_t *pi_file_num );
static int64_t ParseFrameNumber( const char *psz_line, float fps );
static const char *BaseName( const char *psz_path );
static int Open( vlc_object_t *p_this )
{
stream_t *p_access = (stream_t*)p_this;
if( !p_access->psz_filepath )
return VLC_EGENERIC;
bool b_strict = strcmp( p_access->psz_name, "vdr" );
if( b_strict )
{
char psz_extension[4];
int i_length = 0;
const char *psz_name = BaseName( p_access->psz_filepath );
if( sscanf( psz_name, "%*u-%*u-%*u.%*u.%*u.%*u%*[-.]%*u.%3s%n",
psz_extension, &i_length ) != 1 || strcasecmp( psz_extension, "rec" ) ||
( psz_name[i_length] != DIR_SEP_CHAR && psz_name[i_length] != '\0' ) )
return VLC_EGENERIC;
}
struct stat st;
if( vlc_stat( p_access->psz_filepath, &st ) ||
!S_ISDIR( st.st_mode ) )
return VLC_EGENERIC;
access_sys_t *p_sys = vlc_obj_calloc( p_this, 1, sizeof( *p_sys ) );
if( unlikely(p_sys == NULL) )
return VLC_ENOMEM;
p_access->p_sys = p_sys;
p_sys->fd = -1;
p_sys->cur_seekpoint = 0;
p_sys->fps = var_InheritFloat( p_access, "vdr-fps" );
ARRAY_INIT( p_sys->file_sizes );
if( !ScanDirectory( p_access ) ||
!SwitchFile( p_access, 0 ) )
{
Close( p_this );
return VLC_EGENERIC;
}
ACCESS_SET_CALLBACKS( Read, NULL, Control, Seek );
return VLC_SUCCESS;
}
static void Close( vlc_object_t * p_this )
{
stream_t *p_access = (stream_t*)p_this;
access_sys_t *p_sys = p_access->p_sys;
if( p_sys->fd != -1 )
vlc_close( p_sys->fd );
ARRAY_RESET( p_sys->file_sizes );
if( p_sys->p_meta )
vlc_meta_Delete( p_sys->p_meta );
free( p_sys->offsets );
vlc_input_title_Delete( p_sys->p_marks );
}
static bool ScanDirectory( stream_t *p_access )
{
access_sys_t *p_sys = p_access->p_sys;
p_sys->b_ts_format = true;
if( !ImportNextFile( p_access ) )
{
p_sys->b_ts_format = !p_sys->b_ts_format;
if( !ImportNextFile( p_access ) )
return false;
}
while( ImportNextFile( p_access ) )
continue;
ImportMeta( p_access );
ImportMarks( p_access );
return true;
}
static int Control( stream_t *p_access, int i_query, va_list args )
{
access_sys_t *p_sys = p_access->p_sys;
input_title_t ***ppp_title;
int i;
vlc_meta_t *p_meta;
switch( i_query )
{
case STREAM_CAN_SEEK:
case STREAM_CAN_FASTSEEK:
case STREAM_CAN_PAUSE:
case STREAM_CAN_CONTROL_PACE:
*va_arg( args, bool* ) = true;
break;
case STREAM_GET_SIZE:
*va_arg( args, uint64_t* ) = p_sys->size;
break;
case STREAM_GET_PTS_DELAY:
*va_arg( args, vlc_tick_t * ) =
VLC_TICK_FROM_MS(var_InheritInteger( p_access, "file-caching" ));
break;
case STREAM_SET_PAUSE_STATE:
break;
case STREAM_GET_TITLE_INFO:
if( !p_sys->p_marks )
return VLC_EGENERIC;
ppp_title = va_arg( args, input_title_t*** );
*va_arg( args, int* ) = 1;
*ppp_title = malloc( sizeof( **ppp_title ) );
if( !*ppp_title )
return VLC_ENOMEM;
**ppp_title = vlc_input_title_Duplicate( p_sys->p_marks );
break;
case STREAM_GET_TITLE:
*va_arg( args, unsigned * ) = 0;
break;
case STREAM_GET_SEEKPOINT:
*va_arg( args, unsigned * ) = p_sys->cur_seekpoint;
break;
case STREAM_GET_CONTENT_TYPE:
*va_arg( args, char ** ) =
strdup( p_sys->b_ts_format ? "video/MP2T" : "video/MP2P" );
break;
case STREAM_SET_TITLE:
break;
case STREAM_SET_SEEKPOINT:
i = va_arg( args, int );
return Seek( p_access, p_sys->offsets[i] );
case STREAM_GET_META:
p_meta = va_arg( args, vlc_meta_t* );
vlc_meta_Merge( p_meta, p_sys->p_meta );
break;
default:
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
static ssize_t Read( stream_t *p_access, void *p_buffer, size_t i_len )
{
access_sys_t *p_sys = p_access->p_sys;
if( p_sys->fd == -1 )
return 0;
ssize_t i_ret = read( p_sys->fd, p_buffer, i_len );
if( i_ret > 0 )
{
p_sys->offset += i_ret;
UpdateFileSize( p_access );
FindSeekpoint( p_access );
return i_ret;
}
else if( i_ret == 0 )
{
if( p_sys->i_current_file >= FILE_COUNT - 1 )
ImportNextFile( p_access );
SwitchFile( p_access, p_sys->i_current_file + 1 );
return -1;
}
else if( errno == EINTR )
{
return -1;
}
else
{
msg_Err( p_access, "failed to read (%s)", vlc_strerror_c(errno) );
vlc_dialog_display_error( p_access, _("File reading failed"),
_("VLC could not read the file (%s)."),
vlc_strerror(errno) );
SwitchFile( p_access, -1 );
return 0;
}
}
static int Seek( stream_t *p_access, uint64_t i_pos )
{
access_sys_t *p_sys = p_access->p_sys;
i_pos = __MIN( i_pos, p_sys->size );
p_sys->offset = i_pos;
FindSeekpoint( p_access );
unsigned i_file = 0;
while( i_file < FILE_COUNT - 1 &&
i_pos >= FILE_SIZE( i_file ) )
{
i_pos -= FILE_SIZE( i_file );
i_file++;
}
if( !SwitchFile( p_access, i_file ) )
return VLC_EGENERIC;
return lseek( p_sys->fd, i_pos, SEEK_SET ) != -1 ?
VLC_SUCCESS : VLC_EGENERIC;
}
static void FindSeekpoint( stream_t *p_access )
{
access_sys_t *p_sys = p_access->p_sys;
if( !p_sys->p_marks )
return;
int new_seekpoint = p_sys->cur_seekpoint;
if( p_sys->offset < p_sys->offsets[p_sys->cur_seekpoint] )
{
new_seekpoint = 0;
}
while( new_seekpoint + 1 < p_sys->p_marks->i_seekpoint &&
p_sys->offset >= p_sys->offsets[new_seekpoint + 1] )
{
new_seekpoint++;
}
p_sys->cur_seekpoint = new_seekpoint;
}
static char *GetFilePath( stream_t *p_access, unsigned i_file )
{
access_sys_t *sys = p_access->p_sys;
char *psz_path;
if( asprintf( &psz_path, sys->b_ts_format ?
"%s" DIR_SEP "%05u.ts" : "%s" DIR_SEP "%03u.vdr",
p_access->psz_filepath, i_file + 1 ) == -1 )
return NULL;
else
return psz_path;
}
static bool ImportNextFile( stream_t *p_access )
{
access_sys_t *p_sys = p_access->p_sys;
char *psz_path = GetFilePath( p_access, FILE_COUNT );
if( !psz_path )
return false;
struct stat st;
if( vlc_stat( psz_path, &st ) )
{
msg_Dbg( p_access, "could not stat %s: %s", psz_path,
vlc_strerror_c(errno) );
free( psz_path );
return false;
}
if( !S_ISREG( st.st_mode ) )
{
msg_Dbg( p_access, "%s is not a regular file", psz_path );
free( psz_path );
return false;
}
msg_Dbg( p_access, "%s exists", psz_path );
free( psz_path );
ARRAY_APPEND( p_sys->file_sizes, st.st_size );
p_sys->size += st.st_size;
return true;
}
static bool SwitchFile( stream_t *p_access, unsigned i_file )
{
access_sys_t *p_sys = p_access->p_sys;
if( p_sys->fd != -1 && p_sys->i_current_file == i_file )
return true;
if( p_sys->fd != -1 )
{
vlc_close( p_sys->fd );
p_sys->fd = -1;
}
if( i_file >= FILE_COUNT )
return false;
p_sys->i_current_file = i_file;
char *psz_path = GetFilePath( p_access, i_file );
if( !psz_path )
return false;
p_sys->fd = vlc_open( psz_path, O_RDONLY );
if( p_sys->fd == -1 )
{
msg_Err( p_access, "Failed to open %s: %s", psz_path,
vlc_strerror_c(errno) );
goto error;
}
struct stat st;
if( fstat( p_sys->fd, &st ) || !S_ISREG( st.st_mode ) )
{
msg_Err( p_access, "%s is not a regular file", psz_path );
goto error;
}
OptimizeForRead( p_sys->fd );
msg_Dbg( p_access, "opened %s", psz_path );
free( psz_path );
return true;
error:
vlc_dialog_display_error (p_access, _("File reading failed"), _("VLC could not"
" open the file \"%s\" (%s)."), psz_path, vlc_strerror(errno) );
if( p_sys->fd != -1 )
{
vlc_close( p_sys->fd );
p_sys->fd = -1;
}
free( psz_path );
return false;
}
static void OptimizeForRead( int fd )
{
VLC_UNUSED(fd);
#if defined(HAVE_POSIX_FADVISE)
posix_fadvise( fd, 0, 4096, POSIX_FADV_WILLNEED );
posix_fadvise( fd, 0, 0, POSIX_FADV_NOREUSE );
#endif
#if defined(F_RDAHEAD)
fcntl( fd, F_RDAHEAD, 1 );
#endif
#if defined(F_NOCACHE)
fcntl( fd, F_NOCACHE, 0 );
#endif
}
static void UpdateFileSize( stream_t *p_access )
{
access_sys_t *p_sys = p_access->p_sys;
struct stat st;
if( p_sys->size >= p_sys->offset )
return;
if( fstat( p_sys->fd, &st ) )
return;
if( (uint64_t)st.st_size <= CURRENT_FILE_SIZE )
return;
p_sys->size -= CURRENT_FILE_SIZE;
CURRENT_FILE_SIZE = st.st_size;
p_sys->size += CURRENT_FILE_SIZE;
}
static FILE *OpenRelativeFile( stream_t *p_access, const char *psz_file )
{
access_sys_t *sys = p_access->p_sys;
char *psz_path;
if( asprintf( &psz_path, "%s" DIR_SEP "%s%s", p_access->psz_filepath,
psz_file, sys->b_ts_format ? "" : ".vdr" ) == -1 )
return NULL;
FILE *file = vlc_fopen( psz_path, "rb" );
if( !file )
msg_Warn( p_access, "Failed to open %s: %s", psz_path,
vlc_strerror_c(errno) );
free( psz_path );
return file;
}
static bool ReadLine( char **ppsz_line, size_t *pi_size, FILE *p_file )
{
ssize_t read = getline( ppsz_line, pi_size, p_file );
if( read == -1 )
{
free( *ppsz_line );
*ppsz_line = NULL;
return false;
}
if( read > 0 && (*ppsz_line)[ read - 1 ] == '\n' )
(*ppsz_line)[ read - 1 ] = '\0';
EnsureUTF8( *ppsz_line );
return true;
}
static void ImportMeta( stream_t *p_access )
{
access_sys_t *p_sys = p_access->p_sys;
FILE *infofile = OpenRelativeFile( p_access, "info" );
if( !infofile )
return;
vlc_meta_t *p_meta = vlc_meta_New();
p_sys->p_meta = p_meta;
if( !p_meta )
{
fclose( infofile );
return;
}
char *line = NULL;
size_t line_len;
char *psz_title = NULL, *psz_smalltext = NULL, *psz_date = NULL;
while( ReadLine( &line, &line_len, infofile ) )
{
if( !isalpha( (unsigned char)line[0] ) || line[1] != ' ' )
continue;
char tag = line[0];
char *text = line + 2;
if( tag == 'C' )
{
char *psz_name = strchr( text, ' ' );
if( psz_name )
{
*psz_name = '\0';
vlc_meta_AddExtra( p_meta, "Channel", psz_name + 1 );
}
vlc_meta_AddExtra( p_meta, "Transponder", text );
}
else if( tag == 'E' )
{
unsigned i_id, i_start, i_length;
if( sscanf( text, "%u %u %u", &i_id, &i_start, &i_length ) == 3 )
{
char str[50];
struct tm tm;
time_t start = i_start;
localtime_r( &start, &tm );
strftime( str, sizeof(str), "%Y-%m-%d %H:%M", &tm );
vlc_meta_AddExtra( p_meta, "Date", str );
free( psz_date );
psz_date = strdup( str );
i_length = ( i_length + 59 ) / 60;
snprintf( str, sizeof(str), "%u:%02u", i_length / 60, i_length % 60 );
vlc_meta_AddExtra( p_meta, "Duration", str );
}
}
else if( tag == 'T' )
{
free( psz_title );
psz_title = strdup( text );
vlc_meta_AddExtra( p_meta, "Title", text );
}
else if( tag == 'S' )
{
free( psz_smalltext );
psz_smalltext = strdup( text );
vlc_meta_AddExtra( p_meta, "Info", text );
}
else if( tag == 'D' )
{
for( char *p = text; *p; ++p )
{
if( *p == '|' )
*p = '\n';
}
vlc_meta_SetDescription( p_meta, text );
}
else if( tag == 'F' )
{
float fps = atof( text );
if( fps >= 1 )
p_sys->fps = fps;
vlc_meta_AddExtra( p_meta, "Frame Rate", text );
}
else if( tag == 'P' )
{
vlc_meta_AddExtra( p_meta, "Priority", text );
}
else if( tag == 'L' )
{
vlc_meta_AddExtra( p_meta, "Lifetime", text );
}
}
int i_len = 10 +
( psz_title ? strlen( psz_title ) : 0 ) +
( psz_smalltext ? strlen( psz_smalltext ) : 0 ) +
( psz_date ? strlen( psz_date ) : 0 );
char *psz_display = malloc( i_len );
if( psz_display )
{
*psz_display = '\0';
if( psz_title )
strcat( psz_display, psz_title );
if( psz_title && psz_smalltext )
strcat( psz_display, " - " );
if( psz_smalltext )
strcat( psz_display, psz_smalltext );
if( ( psz_title || psz_smalltext ) && psz_date )
{
strcat( psz_display, " (" );
strcat( psz_display, psz_date );
strcat( psz_display, ")" );
}
if( *psz_display )
vlc_meta_SetTitle( p_meta, psz_display );
}
free( psz_display );
free( psz_title );
free( psz_smalltext );
free( psz_date );
fclose( infofile );
}
static void ImportMarks( stream_t *p_access )
{
access_sys_t *p_sys = p_access->p_sys;
FILE *marksfile = OpenRelativeFile( p_access, "marks" );
if( !marksfile )
return;
FILE *indexfile = OpenRelativeFile( p_access, "index" );
if( !indexfile )
{
fclose( marksfile );
return;
}
struct stat st;
if( fstat( fileno( indexfile ), &st ) )
{
fclose( marksfile );
fclose( indexfile );
return;
}
int64_t i_frame_count = st.st_size / 8;
input_title_t *p_marks = vlc_input_title_New();
if( !p_marks )
{
fclose( marksfile );
fclose( indexfile );
return;
}
p_marks->psz_name = strdup( _("VDR Cut Marks") );
p_marks->i_length = i_frame_count * (vlc_tick_t)( CLOCK_FREQ / p_sys->fps );
uint64_t *offsetv = NULL;
int i_chapter_offset = p_sys->fps / 1000 *
var_InheritInteger( p_access, "vdr-chapter-offset" );
int i_min_chapter_size = p_sys->fps * MIN_CHAPTER_SIZE;
int64_t i_prev_chapter = 0;
char *line = NULL;
size_t line_len;
while( ReadLine( &line, &line_len, marksfile ) )
{
int64_t i_frame = ParseFrameNumber( line, p_sys->fps );
if( i_frame - i_prev_chapter < i_min_chapter_size ||
i_frame >= i_frame_count - i_min_chapter_size )
continue;
i_prev_chapter = i_frame;
if( i_frame > -i_chapter_offset )
i_frame += i_chapter_offset;
else
i_frame = 0;
uint64_t i_offset;
uint16_t i_file_number;
if( !ReadIndexRecord( indexfile, p_sys->b_ts_format,
i_frame, &i_offset, &i_file_number ) )
continue;
if( i_file_number < 1 || i_file_number > FILE_COUNT )
continue;
seekpoint_t *sp = vlc_seekpoint_New();
if( !sp )
continue;
sp->i_time_offset = i_frame * (vlc_tick_t)( CLOCK_FREQ / p_sys->fps );
sp->psz_name = strdup( line );
TAB_APPEND( p_marks->i_seekpoint, p_marks->seekpoint, sp );
offsetv = xrealloc(offsetv, p_marks->i_seekpoint * sizeof (*offsetv));
for( int i = 0; i + 1 < i_file_number; ++i )
i_offset += FILE_SIZE(i);
offsetv[p_marks->i_seekpoint - 1] = i_offset;
}
if( p_marks->i_seekpoint > 0 && offsetv[0] > 0 )
{
seekpoint_t *sp = vlc_seekpoint_New();
if( sp )
{
sp->i_time_offset = 0;
sp->psz_name = strdup( _("Start") );
TAB_INSERT( p_marks->i_seekpoint, p_marks->seekpoint, sp, 0 );
offsetv = xrealloc(offsetv,
p_marks->i_seekpoint * sizeof (*offsetv));
memmove(offsetv + 1, offsetv,
(p_marks->i_seekpoint - 1) * sizeof (*offsetv));
offsetv[0] = 0;
}
}
if( p_marks->i_seekpoint > 0 )
{
p_sys->p_marks = p_marks;
p_sys->offsets = offsetv;
}
else
{
vlc_input_title_Delete( p_marks );
free(offsetv);
}
fclose( marksfile );
fclose( indexfile );
}
static bool ReadIndexRecord( FILE *p_file, bool b_ts, int64_t i_frame,
uint64_t *pi_offset, uint16_t *pi_file_num )
{
uint8_t index_record[8];
if( fseek( p_file, sizeof(index_record) * i_frame, SEEK_SET ) != 0 )
return false;
if( fread( &index_record, sizeof(index_record), 1, p_file ) < 1 )
return false;
if( b_ts )
{
uint64_t i_index_entry = GetQWLE( &index_record );
*pi_offset = i_index_entry & UINT64_C(0xFFFFFFFFFF);
*pi_file_num = i_index_entry >> 48;
}
else
{
*pi_offset = GetDWLE( &index_record );
*pi_file_num = index_record[5];
}
return true;
}
static int64_t ParseFrameNumber( const char *psz_line, float fps )
{
unsigned h, m, s, f, n;
n = sscanf( psz_line, "%u:%u:%u.%u", &h, &m, &s, &f );
if( n >= 3 )
{
if( n < 4 )
f = 1;
int64_t i_seconds = (int64_t)h * 3600 + (int64_t)m * 60 + s;
return (int64_t)( i_seconds * (double)fps ) + __MAX(1, f) - 1;
}
int64_t i_frame = strtoll( psz_line, NULL, 10 );
return __MAX(1, i_frame) - 1;
}
static const char *BaseName( const char *psz_path )
{
const char *psz_name = psz_path + strlen( psz_path );
while( psz_name > psz_path && psz_name[-1] == DIR_SEP_CHAR )
--psz_name;
while( psz_name > psz_path && psz_name[-1] != DIR_SEP_CHAR )
--psz_name;
return psz_name;
}
