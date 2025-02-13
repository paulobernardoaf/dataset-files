



















#if !defined(LIBVLC_CONFIGURATION_H)
#define LIBVLC_CONFIGURATION_H 1

#if defined(__cplusplus)
extern "C" {
#endif



int config_CreateDir( vlc_object_t *, const char * );
int config_AutoSaveConfigFile( vlc_object_t * );

void config_Free (module_config_t *, size_t);

int config_LoadCmdLine ( vlc_object_t *, int, const char *[], int * );
int config_LoadConfigFile( vlc_object_t * );
#define config_LoadCmdLine(a,b,c,d) config_LoadCmdLine(VLC_OBJECT(a),b,c,d)
#define config_LoadConfigFile(a) config_LoadConfigFile(VLC_OBJECT(a))
bool config_PrintHelp (vlc_object_t *);

int config_SortConfig (void);
void config_UnsortConfig (void);

#define CONFIG_CLASS(x) ((x) & ~0x1F)

#define IsConfigStringType(type) (((type) & CONFIG_ITEM_STRING) != 0)

#define IsConfigIntegerType(type) (((type) & CONFIG_ITEM_INTEGER) != 0)

#define IsConfigFloatType(type) ((type) == CONFIG_ITEM_FLOAT)


extern vlc_rwlock_t config_lock;
extern bool config_dirty;

bool config_IsSafe (const char *);









char *config_GetLibDir(void) VLC_USED VLC_MALLOC;


#define CONFIG_FILE "vlcrc"

#if defined(__cplusplus)
}
#endif
#endif
