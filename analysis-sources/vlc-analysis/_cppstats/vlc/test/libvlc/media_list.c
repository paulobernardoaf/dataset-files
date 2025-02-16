#include "test.h"
static void test_media_list (const char ** argv, int argc)
{
libvlc_instance_t *vlc;
libvlc_media_t *md, *md1, *md2, *md3, *md4;
libvlc_media_list_t *ml;
int ret;
test_log ("Testing media_list\n");
vlc = libvlc_new (argc, argv);
assert (vlc != NULL);
ml = libvlc_media_list_new (vlc);
assert (ml != NULL);
md1 = libvlc_media_new_path (vlc, "/dev/null");
assert (md1 != NULL);
md2 = libvlc_media_new_path (vlc, "/dev/null");
assert (md2 != NULL);
md3 = libvlc_media_new_path (vlc, "/dev/null");
assert (md3 != NULL);
ret = libvlc_media_list_add_media (ml, md1);
assert (!ret);
ret = libvlc_media_list_add_media (ml, md2);
assert (!ret);
assert( libvlc_media_list_count (ml) == 2 );
assert( libvlc_media_list_index_of_item (ml, md1) == 0 );
assert( libvlc_media_list_index_of_item (ml, md2) == 1 );
ret = libvlc_media_list_remove_index (ml, 0); 
assert (!ret);
assert( libvlc_media_list_index_of_item (ml, md2) == 0 );
ret = libvlc_media_list_add_media (ml, md1); 
assert (!ret);
ret = libvlc_media_list_add_media (ml, md1);
assert (!ret);
assert( libvlc_media_list_count (ml) == 3 );
ret = libvlc_media_list_insert_media (ml, md3, 2);
assert (!ret);
assert( libvlc_media_list_count (ml) == 4 );
assert( libvlc_media_list_index_of_item (ml, md3) == 2 );
md = libvlc_media_list_item_at_index (ml, 0);
assert(md == md2);
libvlc_media_release(md);
md = libvlc_media_list_item_at_index (ml, 2);
assert(md == md3);
libvlc_media_release(md);
ret = libvlc_media_list_remove_index (ml, 4);
assert (ret == -1);
ret = libvlc_media_list_remove_index (ml, 100);
assert (ret == -1);
ret = libvlc_media_list_remove_index (ml, -1);
assert (ret == -1);
libvlc_media_t * p_non_exist =
libvlc_media_list_item_at_index (ml, 4);
assert (p_non_exist == NULL);
p_non_exist = libvlc_media_list_item_at_index (ml, 100);
assert (p_non_exist == NULL);
p_non_exist = libvlc_media_list_item_at_index (ml, -1);
assert (p_non_exist == NULL);
md4 = libvlc_media_new_path (vlc, "/dev/null");
assert (md4 != NULL);
int i_non_exist = 0;
i_non_exist = libvlc_media_list_index_of_item (ml, md4);
assert ( i_non_exist == -1 );
libvlc_media_release (md1);
libvlc_media_release (md2);
libvlc_media_release (md3);
libvlc_media_release (md4);
libvlc_media_list_release (ml);
libvlc_release (vlc);
}
int main (void)
{
test_init();
test_media_list (test_defaults_args, test_defaults_nargs);
return 0;
}
