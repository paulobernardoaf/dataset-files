





















#if !defined(VLC_MODULES_H)
#define VLC_MODULES_H 1






typedef int (*vlc_activate_t)(void *func, bool forced, va_list args);
struct vlc_logger;

























VLC_API module_t *vlc_module_load(struct vlc_logger *log, const char *cap,
const char *name, bool strict,
vlc_activate_t probe, ... ) VLC_USED;
#if !defined(__cplusplus)
#define vlc_module_load(ctx, cap, name, strict, ...) _Generic ((ctx), struct vlc_logger *: vlc_module_load((void *)(ctx), cap, name, strict, __VA_ARGS__), void *: vlc_module_load((void *)(ctx), cap, name, strict, __VA_ARGS__), default: vlc_module_load(vlc_object_logger((vlc_object_t *)(ctx)), cap, name, strict, __VA_ARGS__))








#endif

VLC_API module_t * module_need( vlc_object_t *, const char *, const char *, bool ) VLC_USED;
#define module_need(a,b,c,d) module_need(VLC_OBJECT(a),b,c,d)

VLC_USED
static inline module_t *module_need_var(vlc_object_t *obj, const char *cap,
const char *varname)
{
char *list = var_InheritString(obj, varname);
module_t *m = module_need(obj, cap, list, false);

free(list);
return m;
}
#define module_need_var(a,b,c) module_need_var(VLC_OBJECT(a),b,c)

VLC_API void module_unneed( vlc_object_t *, module_t * );
#define module_unneed(a,b) module_unneed(VLC_OBJECT(a),b)








VLC_API bool module_exists(const char *) VLC_USED;







VLC_API module_t *module_find(const char *name) VLC_USED;










VLC_API module_config_t *module_config_get(const module_t *module,
unsigned *restrict psize) VLC_USED;






VLC_API void module_config_free( module_config_t *tab);

VLC_API void module_list_free(module_t **);
VLC_API module_t ** module_list_get(size_t *n) VLC_USED;









VLC_API bool module_provides(const module_t *m, const char *cap);







VLC_API const char * module_get_object(const module_t *m) VLC_USED;








VLC_API const char *module_get_name(const module_t *m, bool longname) VLC_USED;
#define module_GetLongName( m ) module_get_name( m, true )







VLC_API const char *module_get_help(const module_t *m) VLC_USED;







VLC_API const char *module_get_capability(const module_t *m) VLC_USED;







VLC_API int module_get_score(const module_t *m) VLC_USED;








VLC_API const char *module_gettext(const module_t *m, const char *s) VLC_USED;

VLC_USED static inline module_t *module_get_main (void)
{
return module_find ("core");
}
#define module_get_main(a) module_get_main()

VLC_USED static inline bool module_is_main( const module_t * p_module )
{
return !strcmp( module_get_object( p_module ), "core" );
}

#endif 
