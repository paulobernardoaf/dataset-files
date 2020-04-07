#include <r_types.h>
#include <r_util.h>
#include <r_list.h>
#include <r_skiplist.h>
#if defined(__cplusplus)
extern "C" {
#endif
R_LIB_VERSION_HEADER(r_flag);
#define R_FLAG_NAME_SIZE 512
#define R_FLAG_ZONE_USE_SDB 0
typedef struct r_flag_zone_item_t {
ut64 from;
ut64 to;
#if R_FLAG_ZONE_USE_SDB
const char *name;
#else
char *name;
#endif
} RFlagZoneItem;
typedef struct r_flags_at_offset_t {
ut64 off;
RList *flags; 
} RFlagsAtOffset;
typedef struct r_flag_item_t {
char *name; 
char *realname; 
bool demangled; 
ut64 offset; 
ut64 size; 
RSpace *space; 
char *color; 
char *comment; 
char *alias; 
} RFlagItem;
typedef struct r_flag_t {
RSpaces spaces; 
st64 base; 
bool realnames;
Sdb *tags;
RNum *num;
RSkipList *by_off; 
HtPP *ht_name; 
PrintfCallback cb_printf;
#if R_FLAG_ZONE_USE_SDB
Sdb *zones;
#else
RList *zones;
#endif
} RFlag;
typedef bool (*RFlagExistAt)(RFlag *f, const char *flag_prefix, ut16 fp_size, ut64 off);
typedef RFlagItem* (*RFlagGet)(RFlag *f, const char *name);
typedef RFlagItem* (*RFlagGetAtAddr) (RFlag *f, ut64);
typedef RFlagItem* (*RFlagGetAt)(RFlag *f, ut64 addr, bool closest);
typedef RFlagItem* (*RFlagSet)(RFlag *f, const char *name, ut64 addr, ut32 size);
typedef bool (*RFlagUnset)(RFlag *f, RFlagItem *item);
typedef bool (*RFlagUnsetName)(RFlag *f, const char *name);
typedef bool (*RFlagUnsetOff)(RFlag *f, ut64 addr);
typedef RSpace *(*RFlagSetSpace)(RFlag *f, const char *name);
typedef bool (*RFlagPopSpace)(RFlag *f);
typedef bool (*RFlagPushSpace)(RFlag *f, const char *name);
typedef bool (*RFlagItemCb)(RFlagItem *fi, void *user);
typedef struct r_flag_bind_t {
int init;
RFlag *f;
RFlagExistAt exist_at;
RFlagGet get;
RFlagGetAt get_at;
RFlagSet set;
RFlagUnset unset;
RFlagUnsetName unset_name;
RFlagUnsetOff unset_off;
RFlagSetSpace set_fs;
RFlagPushSpace push_fs;
RFlagPopSpace pop_fs;
} RFlagBind;
#define r_flag_bind_init(x) memset(&x,0,sizeof(x))
R_API void r_flag_bind(RFlag *io, RFlagBind *bnd);
#if defined(R_API)
R_API RFlag * r_flag_new(void);
R_API RFlag * r_flag_free(RFlag *f);
R_API void r_flag_list(RFlag *f, int rad, const char *pfx);
R_API bool r_flag_exist_at(RFlag *f, const char *flag_prefix, ut16 fp_size, ut64 off);
R_API RFlagItem *r_flag_get(RFlag *f, const char *name);
R_API RFlagItem *r_flag_get_i(RFlag *f, ut64 off);
R_API RFlagItem *r_flag_get_by_spaces(RFlag *f, ut64 off, ...);
R_API RFlagItem *r_flag_get_at(RFlag *f, ut64 off, bool closest);
R_API RList *r_flag_all_list(RFlag *f, bool by_space);
R_API const RList* r_flag_get_list(RFlag *f, ut64 off);
R_API char *r_flag_get_liststr(RFlag *f, ut64 off);
R_API bool r_flag_unset(RFlag *f, RFlagItem *item);
R_API bool r_flag_unset_name(RFlag *f, const char *name);
R_API bool r_flag_unset_off(RFlag *f, ut64 addr);
R_API void r_flag_unset_all (RFlag *f);
R_API RFlagItem *r_flag_set(RFlag *fo, const char *name, ut64 addr, ut32 size);
R_API RFlagItem *r_flag_set_next(RFlag *fo, const char *name, ut64 addr, ut32 size);
R_API void r_flag_item_set_alias(RFlagItem *item, const char *alias);
R_API void r_flag_item_free (RFlagItem *item);
R_API void r_flag_item_set_comment(RFlagItem *item, const char *comment);
R_API void r_flag_item_set_realname(RFlagItem *item, const char *realname);
R_API const char *r_flag_item_set_color(RFlagItem *item, const char *color);
R_API RFlagItem *r_flag_item_clone(RFlagItem *item);
R_API int r_flag_unset_glob(RFlag *f, const char *name);
R_API int r_flag_rename(RFlag *f, RFlagItem *item, const char *name);
R_API int r_flag_relocate(RFlag *f, ut64 off, ut64 off_mask, ut64 to);
R_API bool r_flag_move (RFlag *f, ut64 at, ut64 to);
R_API const char *r_flag_color(RFlag *f, RFlagItem *it, const char *color);
R_API int r_flag_count(RFlag *f, const char *glob);
R_API void r_flag_foreach(RFlag *f, RFlagItemCb cb, void *user);
R_API void r_flag_foreach_prefix(RFlag *f, const char *pfx, int pfx_len, RFlagItemCb cb, void *user);
R_API void r_flag_foreach_range(RFlag *f, ut64 from, ut64 to, RFlagItemCb cb, void *user);
R_API void r_flag_foreach_glob(RFlag *f, const char *glob, RFlagItemCb cb, void *user);
R_API void r_flag_foreach_space(RFlag *f, const RSpace *space, RFlagItemCb cb, void *user);
R_API void r_flag_foreach_space_glob(RFlag *f, const char *glob, const RSpace *space, RFlagItemCb cb, void *user);
static inline RSpace *r_flag_space_get(RFlag *f, const char *name) {
return r_spaces_get (&f->spaces, name);
}
static inline RSpace *r_flag_space_cur(RFlag *f) {
return r_spaces_current (&f->spaces);
}
static inline const char *r_flag_space_cur_name(RFlag *f) {
return r_spaces_current_name (&f->spaces);
}
static inline RSpace *r_flag_space_set(RFlag *f, const char *name) {
return r_spaces_set (&f->spaces, name);
}
static inline bool r_flag_space_unset(RFlag *f, const char *name) {
return r_spaces_unset (&f->spaces, name);
}
static inline bool r_flag_space_rename(RFlag *f, const char *oname, const char *nname) {
return r_spaces_rename (&f->spaces, oname, nname);
}
static inline bool r_flag_space_push(RFlag *f, const char *name) {
return r_spaces_push (&f->spaces, name);
}
static inline bool r_flag_space_pop(RFlag *f) {
return r_spaces_pop (&f->spaces);
}
static inline int r_flag_space_count(RFlag *f, const char *name) {
return r_spaces_count (&f->spaces, name);
}
static inline bool r_flag_space_is_empty(RFlag *f) {
return r_spaces_is_empty (&f->spaces);
}
#define r_flag_space_foreach(f, it, s) r_spaces_foreach (&(f)->spaces, (it), (s))
R_API RList *r_flag_tags_list(RFlag *f, const char *name);
R_API RList *r_flag_tags_set(RFlag *f, const char *name, const char *words);
R_API void r_flag_tags_reset(RFlag *f, const char *name);
R_API RList *r_flag_tags_get(RFlag *f, const char *name);
R_API void r_flag_zone_item_free(void *a);
R_API bool r_flag_zone_add(RFlag *fz, const char *name, ut64 addr);
R_API bool r_flag_zone_del(RFlag *fz, const char *name);
R_API bool r_flag_zone_around(RFlag *fz, ut64 addr, const char **prev, const char **next);
R_API bool r_flag_zone_list(RFlag *fz, int mode);
R_API bool r_flag_zone_reset(RFlag *f);
R_API RList *r_flag_zone_barlist(RFlag *f, ut64 from, ut64 bsize, int rows);
#endif
#if defined(__cplusplus)
}
#endif
