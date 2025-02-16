#if !defined(R2_SIGN_H)
#define R2_SIGN_H

#include <r_types.h>
#include <r_anal.h>
#include <r_search.h>

#if defined(__cplusplus)
extern "C" {
#endif

R_LIB_VERSION_HEADER(r_sign);


#define R_SIGN_KEY_MAXSZ 1024
#define R_SIGN_VAL_MAXSZ 10240

#define ZIGN_HASH "sha256"
#define R_ZIGN_HASH R_HASH_SHA256

typedef enum {
R_SIGN_BYTES = 'b', 
R_SIGN_BYTES_MASK= 'm', 
R_SIGN_BYTES_SIZE= 's', 
R_SIGN_ANAL = 'a', 
R_SIGN_COMMENT = 'c', 
R_SIGN_GRAPH = 'g', 
R_SIGN_OFFSET = 'o', 
R_SIGN_NAME = 'n', 
R_SIGN_REFS = 'r', 
R_SIGN_XREFS = 'x', 
R_SIGN_VARS = 'v', 
R_SIGN_TYPES = 't', 
R_SIGN_BBHASH = 'h', 
} RSignType;

typedef struct r_sign_graph_t {
int cc;
int nbbs;
int edges;
int ebbs;
int bbsum;
} RSignGraph;

typedef struct r_sign_bytes_t {
int size;
ut8 *bytes;
ut8 *mask;
} RSignBytes;

typedef struct r_sign_hash_t {
char *bbhash;
} RSignHash;

typedef struct r_sign_item_t {
char *name;
char *realname;
char *comment;
const RSpace *space;

RSignBytes *bytes;
RSignGraph *graph;
ut64 addr;
RList *refs;
RList *xrefs;
RList *vars;
RList *types;
RSignHash *hash;
} RSignItem;

typedef int (*RSignForeachCallback)(RSignItem *it, void *user);
typedef int (*RSignSearchCallback)(RSignItem *it, RSearchKeyword *kw, ut64 addr, void *user);
typedef int (*RSignGraphMatchCallback)(RSignItem *it, RAnalFunction *fcn, void *user);
typedef int (*RSignOffsetMatchCallback)(RSignItem *it, RAnalFunction *fcn, void *user);
typedef int (*RSignHashMatchCallback)(RSignItem *it, RAnalFunction *fcn, void *user);
typedef int (*RSignRefsMatchCallback)(RSignItem *it, RAnalFunction *fcn, void *user);
typedef int (*RSignVarsMatchCallback)(RSignItem *it, RAnalFunction *fcn, void *user);

typedef struct r_sign_search_t {
RSearch *search;
RList *items;
RSignSearchCallback cb;
void *user;
} RSignSearch;

typedef struct r_sign_options_t {
double bytes_diff_threshold;
double graph_diff_threshold;
} RSignOptions;

#if defined(R_API)
R_API bool r_sign_add_bytes(RAnal *a, const char *name, ut64 size, const ut8 *bytes, const ut8 *mask);
R_API bool r_sign_add_anal(RAnal *a, const char *name, ut64 size, const ut8 *bytes, ut64 at);
R_API bool r_sign_add_graph(RAnal *a, const char *name, RSignGraph graph);
R_API bool r_sign_add_addr(RAnal *a, const char *name, ut64 addr);
R_API bool r_sign_add_name(RAnal *a, const char *name, const char *realname);
R_API bool r_sign_add_comment(RAnal *a, const char *name, const char *comment);
R_API bool r_sign_add_refs(RAnal *a, const char *name, RList *refs);
R_API bool r_sign_add_xrefs(RAnal *a, const char *name, RList *xrefs);
R_API bool r_sign_add_vars(RAnal *a, const char *name, RList *vars);
R_API bool r_sign_add_types(RAnal *a, const char *name, RList *vars);
R_API bool r_sign_delete(RAnal *a, const char *name);
R_API void r_sign_list(RAnal *a, int format);
R_API RList *r_sign_get_list(RAnal *a);
R_API bool r_sign_add_hash(RAnal *a, const char *name, int type, const char *val, int len);
R_API bool r_sign_add_bb_hash(RAnal *a, RAnalFunction *fcn, const char *name);
R_API char *r_sign_calc_bbhash(RAnal *a, RAnalFunction *fcn);
R_API bool r_sign_deserialize(RAnal *a, RSignItem *it, const char *k, const char *v);

R_API bool r_sign_foreach(RAnal *a, RSignForeachCallback cb, void *user);

R_API RSignSearch *r_sign_search_new(void);
R_API void r_sign_search_free(RSignSearch *ss);
R_API void r_sign_search_init(RAnal *a, RSignSearch *ss, int minsz, RSignSearchCallback cb, void *user);
R_API int r_sign_search_update(RAnal *a, RSignSearch *ss, ut64 *at, const ut8 *buf, int len);
R_API bool r_sign_match_graph(RAnal *a, RAnalFunction *fcn, int mincc, RSignGraphMatchCallback cb, void *user);
R_API bool r_sign_match_addr(RAnal *a, RAnalFunction *fcn, RSignOffsetMatchCallback cb, void *user);
R_API bool r_sign_match_hash(RAnal *a, RAnalFunction *fcn, RSignHashMatchCallback cb, void *user);
R_API bool r_sign_match_refs(RAnal *a, RAnalFunction *fcn, RSignRefsMatchCallback cb, void *user);
R_API bool r_sign_match_vars(RAnal *a, RAnalFunction *fcn, RSignRefsMatchCallback cb, void *user);
R_API bool r_sign_match_types(RAnal *a, RAnalFunction *fcn, RSignRefsMatchCallback cb, void *user);

R_API bool r_sign_load(RAnal *a, const char *file);
R_API bool r_sign_load_gz(RAnal *a, const char *filename);
R_API char *r_sign_path(RAnal *a, const char *file);
R_API bool r_sign_save(RAnal *a, const char *file);

R_API RSignItem *r_sign_item_new(void);
R_API RSignItem *r_sign_item_dup(RSignItem *it);
R_API void r_sign_item_free(RSignItem *item);

R_API RList *r_sign_fcn_refs(RAnal *a, RAnalFunction *fcn);
R_API RList *r_sign_fcn_xrefs(RAnal *a, RAnalFunction *fcn);
R_API RList *r_sign_fcn_vars(RAnal *a, RAnalFunction *fcn);
R_API RList *r_sign_fcn_types(RAnal *a, RAnalFunction *fcn);

R_API int r_sign_is_flirt(RBuffer *buf);
R_API void r_sign_flirt_dump(const RAnal *anal, const char *flirt_file);
R_API void r_sign_flirt_scan(RAnal *anal, const char *flirt_file);

R_API bool r_sign_diff(RAnal *a, RSignOptions *options, const char *other_space_name);
R_API bool r_sign_diff_by_name(RAnal *a, RSignOptions *options, const char *other_space_name, bool not_matching);

R_API RSignOptions *r_sign_options_new(const char *bytes_thresh, const char *graph_thresh);
R_API void r_sign_options_free(RSignOptions *options);
#endif

#if defined(__cplusplus)
}
#endif

#endif
