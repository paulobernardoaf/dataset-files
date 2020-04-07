

















#include "../../libvlc/test.h"
#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <vlc_common.h>
#include <vlc_epg.h>
#include <assert.h>

static void assert_current( const vlc_epg_t *p_epg, const char *psz_name )
{
if( (void*)psz_name == (void*)p_epg->p_current )
{
assert( psz_name == NULL );
}
else
{
assert( p_epg->p_current );
assert( p_epg->p_current->psz_name );
assert( psz_name[0] == p_epg->p_current->psz_name[0] );
}
}

static void print_order( const vlc_epg_t *p_epg )
{
printf("order: ");
for( size_t i = 0; i < p_epg->i_event; i++ )
printf("%s ", p_epg->pp_event[i]->psz_name );
if( p_epg->p_current )
printf(" current %s", p_epg->p_current->psz_name );
printf("\n");
}

static void assert_events( const vlc_epg_t *p_epg, const char *psz_names, size_t i_names )
{
assert( p_epg->i_event == i_names );
for( size_t i = 0; i < p_epg->i_event; i++ )
{
assert( p_epg->pp_event[i]->psz_name &&
p_epg->pp_event[i]->psz_name[0] == psz_names[i] );
}
}

#define EPG_ADD(epg, start, duration, a) do {vlc_epg_event_t *p_evt = vlc_epg_event_New( start, start, duration );if( p_evt ){p_evt->psz_name = strdup( a );if( !p_evt->psz_name || !vlc_epg_AddEvent( epg, p_evt ) )vlc_epg_event_Delete( p_evt );}} while( 0 )










int main( void )
{
test_init();

int i=1;


printf("--test %d\n", i++);
vlc_epg_t *p_epg = vlc_epg_New( 0, 0 );
assert(p_epg);
EPG_ADD( p_epg, 42, 20, "A" );
EPG_ADD( p_epg, 62, 20, "B" );
EPG_ADD( p_epg, 82, 20, "C" );
EPG_ADD( p_epg, 102, 20, "D" );
print_order( p_epg );
assert_events( p_epg, "ABCD", 4 );
assert_current( p_epg, NULL );

vlc_epg_SetCurrent( p_epg, 82 );
assert_current( p_epg, "C" );

vlc_epg_Delete( p_epg );



printf("--test %d\n", i++);
p_epg = vlc_epg_New( 0, 0 );
assert(p_epg);
EPG_ADD( p_epg, 82, 20, "C" );
EPG_ADD( p_epg, 62, 20, "B" );
EPG_ADD( p_epg, 102, 20, "D" );
EPG_ADD( p_epg, 42, 20, "A" );
print_order( p_epg );
assert_events( p_epg, "ABCD", 4 );
vlc_epg_Delete( p_epg );


printf("--test %d\n", i++);
p_epg = vlc_epg_New( 0, 0 );
assert(p_epg);
EPG_ADD( p_epg, 142, 20, "F" );
EPG_ADD( p_epg, 122, 20, "E" );
EPG_ADD( p_epg, 102, 20, "D" );
EPG_ADD( p_epg, 82, 20, "C" );
EPG_ADD( p_epg, 42, 20, "A" );
EPG_ADD( p_epg, 62, 20, "B" );
print_order( p_epg );
assert_events( p_epg, "ABCDEF", 6 );
vlc_epg_Delete( p_epg );


printf("--test %d\n", i++);
p_epg = vlc_epg_New( 0, 0 );
assert(p_epg);
EPG_ADD( p_epg, 62, 20, "E" );
EPG_ADD( p_epg, 62, 20, "F" );
EPG_ADD( p_epg, 42, 20, "A" );
vlc_epg_SetCurrent( p_epg, 62 );
EPG_ADD( p_epg, 82, 20, "C" );
EPG_ADD( p_epg, 62, 20, "B" );
EPG_ADD( p_epg, 102, 20, "D" );
print_order( p_epg );
assert_events( p_epg, "ABCD", 4 );
assert_current( p_epg, "B" );
vlc_epg_Delete( p_epg );

return 0;
}
