#include <vlc_common.h>
#include <vlc_filter.h>
#include <vlc_text_style.h>
typedef struct buffer_t
{
size_t i_size; 
size_t i_length; 
char *p_memory; 
char *p_begin; 
} buffer_t;
int BufferInit( buffer_t *p_buffer );
int BufferDestroy( buffer_t *p_buffer );
int BufferAdd( buffer_t *p_buffer, const char *p_data, size_t i_len );
int BufferPrintf( buffer_t *p_buffer, const char *p_fmt, ... );
int BufferDel( buffer_t *p_buffer, int i_len );
char *BufferGetToken( buffer_t *p_buffer );
typedef struct commandparams_t
{
int32_t i_id; 
int32_t i_shmid; 
vlc_fourcc_t fourcc;
int32_t i_x; 
int32_t i_y; 
int32_t i_width; 
int32_t i_height; 
int32_t i_alpha; 
text_style_t fontstyle; 
bool b_visible; 
} commandparams_t;
typedef int (*parser_func_t)(char *psz_command, char *psz_end, commandparams_t *p_params );
typedef int (*execute_func_t)( filter_t *p_filter, const commandparams_t *p_params, commandparams_t *p_results );
typedef int (*unparse_func_t)( const commandparams_t *p_results, buffer_t *p_output );
typedef struct commanddesc_t
{
char *psz_command;
bool b_atomic;
parser_func_t pf_parser;
execute_func_t pf_execute;
unparse_func_t pf_unparse;
} commanddesc_t;
typedef struct commanddesc_static_t
{
const char *psz_command;
bool b_atomic;
parser_func_t pf_parser;
execute_func_t pf_execute;
unparse_func_t pf_unparse;
} commanddesc_static_t;
typedef struct command_t
{
struct commanddesc_t *p_command;
int i_status;
commandparams_t params;
commandparams_t results;
struct command_t *p_next;
} command_t;
void RegisterCommand( filter_t *p_filter );
void UnregisterCommand( filter_t *p_filter );
typedef struct queue_t
{
command_t *p_head; 
command_t *p_tail; 
} queue_t;
int QueueInit( queue_t *p_queue );
int QueueDestroy( queue_t *p_queue );
int QueueEnqueue( queue_t *p_queue, command_t *p_cmd );
command_t *QueueDequeue( queue_t *p_queue );
int QueueTransfer( queue_t *p_sink, queue_t *p_source );
typedef struct overlay_t
{
int i_x, i_y;
int i_alpha;
bool b_active;
video_format_t format;
text_style_t *p_fontstyle;
union {
picture_t *p_pic;
char *p_text;
} data;
} overlay_t;
overlay_t *OverlayCreate( void );
int OverlayDestroy( overlay_t *p_ovl );
typedef struct list_t
{
overlay_t **pp_head, **pp_tail;
} list_t;
int do_ListInit( list_t *p_list );
int do_ListDestroy( list_t *p_list );
ssize_t ListAdd( list_t *p_list, overlay_t *p_new );
int ListRemove( list_t *p_list, size_t i_idx );
overlay_t *ListGet( list_t *p_list, size_t i_idx );
overlay_t *ListWalk( list_t *p_list );
typedef struct
{
buffer_t input, output;
int i_inputfd, i_outputfd;
char *psz_inputfile, *psz_outputfile;
commanddesc_t **pp_commands; 
size_t i_commands;
bool b_updated, b_atomic;
queue_t atomic, pending, processed;
list_t overlays;
vlc_mutex_t lock; 
} filter_sys_t;
