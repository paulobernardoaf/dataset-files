int ks_values_copy(const char * ppsz_dst[KEY_MAX],
const char *const ppsz_src[KEY_MAX]);
struct ks_list
{
vlc_keystore_entry *p_entries;
unsigned i_count;
unsigned i_max;
};
vlc_keystore_entry *ks_list_new_entry(struct ks_list *p_list);
void ks_list_free(struct ks_list *p_list);
vlc_keystore_entry *ks_list_find_entry(struct ks_list *p_list,
const char *const ppsz_values[KEY_MAX],
unsigned *p_start_index);
