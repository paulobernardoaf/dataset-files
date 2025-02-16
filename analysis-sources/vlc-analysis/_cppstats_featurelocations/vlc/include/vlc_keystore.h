



















#if !defined(VLC_KEYSTORE_H)
#define VLC_KEYSTORE_H

#include <vlc_common.h>

typedef struct vlc_keystore vlc_keystore;
typedef struct vlc_keystore_entry vlc_keystore_entry;
typedef struct vlc_credential vlc_credential;


int
libvlc_InternalKeystoreInit(libvlc_int_t *p_libvlc);


void
libvlc_InternalKeystoreClean(libvlc_int_t *p_libvlc);














enum vlc_keystore_key {
KEY_PROTOCOL,
KEY_USER,
KEY_SERVER,
KEY_PATH,
KEY_PORT,
KEY_REALM,
KEY_AUTHTYPE,
KEY_MAX,
};
#define VLC_KEYSTORE_VALUES_INIT(ppsz_values) memset(ppsz_values, 0, sizeof(const char *) * KEY_MAX)




struct vlc_keystore_entry
{

char * ppsz_values[KEY_MAX];

uint8_t * p_secret;

size_t i_secret_len;
};














VLC_API vlc_keystore *
vlc_keystore_create(vlc_object_t *p_parent);
#define vlc_keystore_create(x) vlc_keystore_create(VLC_OBJECT(x))




VLC_API void
vlc_keystore_release(vlc_keystore *p_keystore);















VLC_API int
vlc_keystore_store(vlc_keystore *p_keystore,
const char *const ppsz_values[KEY_MAX],
const uint8_t* p_secret, ssize_t i_secret_len,
const char *psz_label);











VLC_API unsigned int
vlc_keystore_find(vlc_keystore *p_keystore,
const char *const ppsz_values[KEY_MAX],
vlc_keystore_entry **pp_entries) VLC_USED;











VLC_API unsigned int
vlc_keystore_remove(vlc_keystore *p_keystore,
const char *const ppsz_values[KEY_MAX]);




VLC_API void
vlc_keystore_release_entries(vlc_keystore_entry *p_entries, unsigned int i_count);










struct vlc_credential
{

const vlc_url_t *p_url;


const char *psz_realm;


const char *psz_authtype;

const char *psz_username;

const char *psz_password;


enum {
GET_FROM_URL,
GET_FROM_OPTION,
GET_FROM_MEMORY_KEYSTORE,
GET_FROM_KEYSTORE,
GET_FROM_DIALOG,
} i_get_order;

vlc_keystore *p_keystore;
vlc_keystore_entry *p_entries;
unsigned int i_entries_count;

char *psz_split_domain;
char *psz_var_username;
char *psz_var_password;

char *psz_dialog_username;
char *psz_dialog_password;
bool b_from_keystore;
bool b_store;
};








VLC_API void
vlc_credential_init(vlc_credential *p_credential, const vlc_url_t *p_url);




VLC_API void
vlc_credential_clean(vlc_credential *p_credential);






















VLC_API bool
vlc_credential_get(vlc_credential *p_credential, vlc_object_t *p_parent,
const char *psz_option_username,
const char *psz_option_password,
const char *psz_dialog_title,
const char *psz_dialog_fmt, ...) VLC_FORMAT(6, 7);
#define vlc_credential_get(a, b, c, d, e, f, ...) vlc_credential_get(a, VLC_OBJECT(b), c, d, e, f, ##__VA_ARGS__)












VLC_API bool
vlc_credential_store(vlc_credential *p_credential, vlc_object_t *p_parent);
#define vlc_credential_store(a, b) vlc_credential_store(a, VLC_OBJECT(b))








#define VLC_KEYSTORE_NAME "libVLC"

static inline int
vlc_keystore_entry_set_secret(vlc_keystore_entry *p_entry,
const uint8_t *p_secret, size_t i_secret_len)
{
p_entry->p_secret = (uint8_t*) malloc(i_secret_len);
if (!p_entry->p_secret)
return VLC_EGENERIC;
memcpy(p_entry->p_secret, p_secret, i_secret_len);
p_entry->i_secret_len = i_secret_len;
return VLC_SUCCESS;
}

static inline void
vlc_keystore_release_entry(vlc_keystore_entry *p_entry)
{
for (unsigned int j = 0; j < KEY_MAX; ++j)
{
free(p_entry->ppsz_values[j]);
p_entry->ppsz_values[j] = NULL;
}
free(p_entry->p_secret);
p_entry->p_secret = NULL;
}

typedef struct vlc_keystore_sys vlc_keystore_sys;
struct vlc_keystore
{
struct vlc_object_t obj;
module_t *p_module;
vlc_keystore_sys *p_sys;


int (*pf_store)(vlc_keystore *p_keystore,
const char *const ppsz_values[KEY_MAX],
const uint8_t *p_secret,
size_t i_secret_len, const char *psz_label);

unsigned int (*pf_find)(vlc_keystore *p_keystore,
const char *const ppsz_values[KEY_MAX],
vlc_keystore_entry **pp_entries);


unsigned int (*pf_remove)(vlc_keystore *p_keystore,
const char *const ppsz_values[KEY_MAX]);
};



#endif
