






















#if !defined(_LIBVLC_MEDIA_LIST_PATH_H)
#define _LIBVLC_MEDIA_LIST_PATH_H 1

typedef int * libvlc_media_list_path_t; 




static inline void libvlc_media_list_path_dump( const libvlc_media_list_path_t path )
{
if(!path)
{
printf("NULL path\n");
return;
}

for(int i = 0; path[i] != -1; i++)
printf("%s%d", i > 0 ? "/" : "", path[i]);
printf("\n");
}




static inline libvlc_media_list_path_t libvlc_media_list_path_empty( void )
{
libvlc_media_list_path_t ret = xmalloc(sizeof(int));
ret[0] = -1;
return ret;
}




static inline libvlc_media_list_path_t libvlc_media_list_path_with_root_index( int index )
{
libvlc_media_list_path_t ret = xmalloc(sizeof(int)*2);
ret[0] = index;
ret[1] = -1;
return ret;
}




static inline int libvlc_media_list_path_depth( const libvlc_media_list_path_t path )
{
int i;
for( i = 0; path[i] != -1; i++ );
return i;
}




static inline void libvlc_media_list_path_append( libvlc_media_list_path_t * p_path, int index )
{
int old_depth = libvlc_media_list_path_depth( *p_path );
*p_path = xrealloc( *p_path, sizeof(int)*(old_depth+2));
*p_path[old_depth] = index;
*p_path[old_depth+1] = -1;
}




static inline libvlc_media_list_path_t libvlc_media_list_path_copy_by_appending( const libvlc_media_list_path_t path, int index )
{
libvlc_media_list_path_t ret;
int old_depth = libvlc_media_list_path_depth( path );
ret = xmalloc( sizeof(int) * (old_depth + 2) );
memcpy( ret, path, sizeof(int) * old_depth );
ret[old_depth] = index;
ret[old_depth+1] = -1;
return ret;
}




static inline libvlc_media_list_path_t libvlc_media_list_path_copy( const libvlc_media_list_path_t path )
{
libvlc_media_list_path_t ret;
int depth = libvlc_media_list_path_depth( path );
ret = xmalloc( sizeof(int)*(depth+1) );
memcpy( ret, path, sizeof(int)*(depth+1) );
return ret;
}




static libvlc_media_list_path_t
get_path_rec( const libvlc_media_list_path_t path, libvlc_media_list_t * p_current_mlist, libvlc_media_t * p_searched_md )
{
int count = libvlc_media_list_count( p_current_mlist );

for( int i = 0; i < count; i++ )
{
libvlc_media_t * p_md = libvlc_media_list_item_at_index( p_current_mlist, i );

if( p_md == p_searched_md )
return libvlc_media_list_path_copy_by_appending( path, i ); 

libvlc_media_list_t * p_subitems = libvlc_media_subitems( p_md );
libvlc_media_release( p_md );
if( p_subitems )
{
libvlc_media_list_path_t new_path = libvlc_media_list_path_copy_by_appending( path, i );
libvlc_media_list_lock( p_subitems );
libvlc_media_list_path_t ret = get_path_rec( new_path, p_subitems, p_searched_md );
libvlc_media_list_unlock( p_subitems );
free( new_path );
libvlc_media_list_release( p_subitems );
if( ret )
return ret; 
}
}
return NULL;
}




static inline libvlc_media_list_path_t libvlc_media_list_path_of_item( libvlc_media_list_t * p_mlist, libvlc_media_t * p_md )
{
libvlc_media_list_path_t path = libvlc_media_list_path_empty();
libvlc_media_list_path_t ret;
ret = get_path_rec( path, p_mlist, p_md );
free( path );
return ret;
}




static libvlc_media_t *
libvlc_media_list_item_at_path( libvlc_media_list_t * p_mlist, const libvlc_media_list_path_t path )
{
libvlc_media_list_t * p_current_mlist = p_mlist;

for( int i = 0; path[i] != -1; i++ )
{
libvlc_media_t* p_md = libvlc_media_list_item_at_index( p_current_mlist, path[i] );

if( p_current_mlist != p_mlist )
libvlc_media_list_release( p_current_mlist );

if( path[i+1] == -1 )
return p_md;

p_current_mlist = libvlc_media_subitems( p_md );
libvlc_media_release( p_md );

if( !p_current_mlist )
return NULL;


}

if( p_current_mlist != p_mlist )
libvlc_media_list_release( p_current_mlist );
return NULL;
}




static libvlc_media_list_t *
libvlc_media_list_parentlist_at_path( libvlc_media_list_t * p_mlist, const libvlc_media_list_path_t path )
{
libvlc_media_list_t * p_current_mlist = p_mlist;

for( int i = 0; path[i] != -1; i++ )
{
if( p_current_mlist != p_mlist )
libvlc_media_list_release( p_current_mlist );

if( path[i+1] == -1 )
{
libvlc_media_list_retain(p_current_mlist);
return p_current_mlist;
}

libvlc_media_t* p_md = libvlc_media_list_item_at_index( p_current_mlist, path[i] );

p_current_mlist = libvlc_media_subitems( p_md );
libvlc_media_release( p_md );

if( !p_current_mlist )
return NULL;


}

if( p_current_mlist != p_mlist )
libvlc_media_list_release( p_current_mlist );
return NULL;
}




static libvlc_media_list_t *
libvlc_media_list_sublist_at_path( libvlc_media_list_t * p_mlist, const libvlc_media_list_path_t path )
{
libvlc_media_list_t * ret;
libvlc_media_t * p_md = libvlc_media_list_item_at_path( p_mlist, path );
if( !p_md )
return NULL;

ret = libvlc_media_subitems( p_md );
libvlc_media_release( p_md );

return ret;
}

#endif
