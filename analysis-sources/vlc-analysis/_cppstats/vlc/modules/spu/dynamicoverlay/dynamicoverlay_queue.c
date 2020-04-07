#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include "dynamicoverlay.h"
int QueueInit( queue_t *p_queue )
{
memset( p_queue, 0, sizeof( queue_t ) );
p_queue->p_head = NULL;
p_queue->p_tail = NULL;
return VLC_SUCCESS;
}
int QueueDestroy( queue_t *p_queue )
{
command_t *p_cur = p_queue->p_head, *p_temp;
while( p_cur != NULL )
{
p_temp = p_cur;
p_cur = p_cur->p_next;
free( p_temp );
}
p_queue->p_head = NULL;
p_queue->p_tail = NULL;
return VLC_SUCCESS;
}
int QueueEnqueue( queue_t *p_queue, command_t *p_cmd )
{
if( p_queue->p_tail != NULL )
{
p_queue->p_tail->p_next = p_cmd;
}
if( p_queue->p_head == NULL )
{
p_queue->p_head = p_cmd;
}
p_queue->p_tail = p_cmd;
p_cmd->p_next = NULL;
return VLC_SUCCESS;
}
command_t *QueueDequeue( queue_t *p_queue )
{
if( p_queue->p_head == NULL )
{
return NULL;
}
else
{
command_t *p_ret = p_queue->p_head;
if( p_queue->p_head == p_queue->p_tail )
{
p_queue->p_head = p_queue->p_tail = NULL;
}
else
{
p_queue->p_head = p_queue->p_head->p_next;
}
return p_ret;
}
}
int QueueTransfer( queue_t *p_sink, queue_t *p_source )
{
if( p_source->p_head == NULL ) {
return VLC_SUCCESS;
}
if( p_sink->p_head == NULL ) {
p_sink->p_head = p_source->p_head;
p_sink->p_tail = p_source->p_tail;
} else {
p_sink->p_tail->p_next = p_source->p_head;
p_sink->p_tail = p_source->p_tail;
}
p_source->p_head = p_source->p_tail = NULL;
return VLC_SUCCESS;
}
