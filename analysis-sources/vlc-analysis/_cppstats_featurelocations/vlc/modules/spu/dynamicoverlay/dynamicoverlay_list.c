






















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>

#include "dynamicoverlay.h"





int do_ListInit( list_t *p_list )
{
p_list->pp_head = calloc( 16, sizeof( overlay_t * ) );
if( p_list->pp_head == NULL )
return VLC_ENOMEM;

p_list->pp_tail = p_list->pp_head + 16;
return VLC_SUCCESS;
}

int do_ListDestroy( list_t *p_list )
{
for( overlay_t **pp_cur = p_list->pp_head;
pp_cur < p_list->pp_tail;
++pp_cur )
{
if( *pp_cur != NULL )
{
OverlayDestroy( *pp_cur );
free( *pp_cur );
}
}
free( p_list->pp_head );

return VLC_SUCCESS;
}

ssize_t ListAdd( list_t *p_list, overlay_t *p_new )
{

for( overlay_t **pp_cur = p_list->pp_head;
pp_cur < p_list->pp_tail;
++pp_cur )
{
if( *pp_cur == NULL )
{
*pp_cur = p_new;
return pp_cur - p_list->pp_head;
}
}


size_t i_size = p_list->pp_tail - p_list->pp_head;
size_t i_newsize = i_size * 2;
p_list->pp_head = realloc_or_free( p_list->pp_head,
i_newsize * sizeof( overlay_t * ) );
if( p_list->pp_head == NULL )
return VLC_ENOMEM;

p_list->pp_tail = p_list->pp_head + i_newsize;
memset( p_list->pp_head + i_size, 0, i_size * sizeof( overlay_t * ) );
p_list->pp_head[i_size] = p_new;
return i_size;
}

int ListRemove( list_t *p_list, size_t i_idx )
{
int ret;

if( ( i_idx >= (size_t)( p_list->pp_tail - p_list->pp_head ) ) ||
( p_list->pp_head[i_idx] == NULL ) )
{
return VLC_EGENERIC;
}

ret = OverlayDestroy( p_list->pp_head[i_idx] );
free( p_list->pp_head[i_idx] );
p_list->pp_head[i_idx] = NULL;

return ret;
}

overlay_t *ListGet( list_t *p_list, size_t i_idx )
{
if( ( i_idx >= (size_t)( p_list->pp_tail - p_list->pp_head ) ) ||
( p_list->pp_head[i_idx] == NULL ) )
{
return NULL;
}
return p_list->pp_head[i_idx];
}

overlay_t *ListWalk( list_t *p_list )
{
static overlay_t **pp_cur = NULL;

if( pp_cur == NULL )
pp_cur = p_list->pp_head;
else
pp_cur = pp_cur + 1;

for( ; pp_cur < p_list->pp_tail; ++pp_cur )
{
if( ( *pp_cur != NULL ) &&
( (*pp_cur)->b_active )&&
( (*pp_cur)->format.i_chroma != VLC_FOURCC( '\0','\0','\0','\0') ) )
{
return *pp_cur;
}
}
pp_cur = NULL;
return NULL;
}
