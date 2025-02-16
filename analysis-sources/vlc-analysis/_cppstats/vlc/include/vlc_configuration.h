#include <sys/types.h> 
#if defined(__cplusplus)
extern "C" {
#endif
struct config_category_t
{
int i_id;
const char *psz_name;
const char *psz_help;
};
typedef union
{
char *psz;
int64_t i;
float f;
} module_value_t;
typedef int (*vlc_string_list_cb)(const char *, char ***, char ***);
typedef int (*vlc_integer_list_cb)(const char *, int64_t **, char ***);
struct module_config_t
{
uint8_t i_type; 
char i_short; 
unsigned b_internal:1; 
unsigned b_unsaveable:1; 
unsigned b_safe:1; 
unsigned b_removed:1; 
const char *psz_type; 
const char *psz_name; 
const char *psz_text; 
const char *psz_longtext; 
module_value_t value; 
module_value_t orig; 
module_value_t min; 
module_value_t max; 
uint16_t list_count; 
union
{
const char **psz; 
const int *i; 
} list; 
const char **list_text; 
void *owner; 
};
VLC_API int config_GetType(const char *name) VLC_USED;
VLC_API int64_t config_GetInt(const char *name) VLC_USED;
VLC_API void config_PutInt(const char *name, int64_t val);
VLC_API float config_GetFloat(const char *name) VLC_USED;
VLC_API void config_PutFloat(const char *name, float val);
VLC_API char *config_GetPsz(const char *name) VLC_USED VLC_MALLOC;
VLC_API void config_PutPsz(const char *name, const char *val);
VLC_API ssize_t config_GetIntChoices(const char *, int64_t **values,
char ***texts) VLC_USED;
VLC_API ssize_t config_GetPszChoices(const char *,
char ***values, char ***texts) VLC_USED;
VLC_API int config_SaveConfigFile( vlc_object_t * );
#define config_SaveConfigFile(a) config_SaveConfigFile(VLC_OBJECT(a))
VLC_API void config_ResetAll(void);
VLC_API module_config_t *config_FindConfig(const char *name) VLC_USED;
typedef enum vlc_system_dir
{
VLC_PKG_DATA_DIR, 
VLC_PKG_LIB_DIR, 
VLC_PKG_LIBEXEC_DIR, 
VLC_PKG_INCLUDE_DIR_RESERVED,
VLC_SYSDATA_DIR, 
VLC_LIB_DIR, 
VLC_LIBEXEC_DIR, 
VLC_INCLUDE_DIR_RESERVED,
VLC_LOCALE_DIR, 
} vlc_sysdir_t;
VLC_API char *config_GetSysPath(vlc_sysdir_t dir, const char *filename)
VLC_USED VLC_MALLOC;
typedef enum vlc_user_dir
{
VLC_HOME_DIR, 
VLC_CONFIG_DIR, 
VLC_USERDATA_DIR, 
VLC_CACHE_DIR, 
VLC_DESKTOP_DIR=0x80,
VLC_DOWNLOAD_DIR,
VLC_TEMPLATES_DIR,
VLC_PUBLICSHARE_DIR,
VLC_DOCUMENTS_DIR,
VLC_MUSIC_DIR,
VLC_PICTURES_DIR,
VLC_VIDEOS_DIR,
} vlc_userdir_t;
VLC_API char * config_GetUserDir( vlc_userdir_t ) VLC_USED VLC_MALLOC;
VLC_API void config_AddIntf(const char *);
VLC_API void config_RemoveIntf(const char *);
VLC_API bool config_ExistIntf(const char *) VLC_USED;
struct config_chain_t
{
config_chain_t *p_next; 
char *psz_name; 
char *psz_value; 
};
VLC_API void config_ChainParse( vlc_object_t *, const char *psz_prefix, const char *const *ppsz_options, const config_chain_t * );
#define config_ChainParse( a, b, c, d ) config_ChainParse( VLC_OBJECT(a), b, c, d )
VLC_API const char *config_ChainParseOptions( config_chain_t **pp_cfg, const char *ppsz_opts );
VLC_API char *config_ChainCreate( char **ppsz_name, config_chain_t **pp_cfg, const char *psz_string ) VLC_USED VLC_MALLOC;
VLC_API void config_ChainDestroy( config_chain_t * );
VLC_API config_chain_t * config_ChainDuplicate( const config_chain_t * ) VLC_USED VLC_MALLOC;
VLC_API char * config_StringUnescape( char *psz_string );
VLC_API char * config_StringEscape( const char *psz_string ) VLC_USED VLC_MALLOC;
#if defined(__cplusplus)
}
#endif
