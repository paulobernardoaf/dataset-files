#if !defined(R2_SEARCH_H)
#define R2_SEARCH_H

#include <r_types.h>
#include <r_util.h>
#include <r_list.h>
#include <r_io.h>

#if defined(__cplusplus)
extern "C" {
#endif

R_LIB_VERSION_HEADER(r_search);

enum {
R_SEARCH_ESIL,
R_SEARCH_KEYWORD,
R_SEARCH_REGEXP,
R_SEARCH_PATTERN,
R_SEARCH_STRING,
R_SEARCH_XREFS,
R_SEARCH_AES,
R_SEARCH_DELTAKEY,
R_SEARCH_MAGIC,
R_SEARCH_LAST
};

#define R_SEARCH_DISTANCE_MAX 10

#define R_SEARCH_KEYWORD_TYPE_BINARY 'i'
#define R_SEARCH_KEYWORD_TYPE_STRING 's'

typedef struct r_search_keyword_t {
ut8 *bin_keyword;
ut8 *bin_binmask;
ut32 keyword_length;
ut32 binmask_length;
void *data;
int count;
int kwidx;
int icase; 
int type;
ut64 last; 
} RSearchKeyword;

typedef struct r_search_hit_t {
RSearchKeyword *kw;
ut64 addr;
} RSearchHit;

typedef int (*RSearchCallback)(RSearchKeyword *kw, void *user, ut64 where);

typedef struct r_search_t {
int n_kws; 
int mode;
ut32 pattern_size;
ut32 string_min; 
ut32 string_max; 
void *data; 
void *user; 
RSearchCallback callback;
ut64 nhits;
ut64 maxhits; 
RList *hits;
int distance;
int inverse;
bool overlap; 
int contiguous;
int align;
int (*update)(struct r_search_t *s, ut64 from, const ut8 *buf, int len);
RList *kws; 
RIOBind iob;
char bckwrds;
} RSearch;

#if defined(R_API)

#define R_SEARCH_AES_BOX_SIZE 31

R_API RSearch *r_search_new(int mode);
R_API int r_search_set_mode(RSearch *s, int mode);
R_API RSearch *r_search_free(RSearch *s);


R_API RList *r_search_find(RSearch *s, ut64 addr, const ut8 *buf, int len);
R_API int r_search_update(RSearch *s, ut64 from, const ut8 *buf, long len);
R_API int r_search_update_i(RSearch *s, ut64 from, const ut8 *buf, long len);

R_API void r_search_keyword_free (RSearchKeyword *kw);
R_API RSearchKeyword* r_search_keyword_new(const ut8 *kw, int kwlen, const ut8 *bm, int bmlen, const char *data);
R_API RSearchKeyword* r_search_keyword_new_str(const char *kw, const char *bm, const char *data, int icase);
R_API RSearchKeyword* r_search_keyword_new_wide(const char *kw, const char *bm, const char *data, int icase);
R_API RSearchKeyword* r_search_keyword_new_hex(const char *kwstr, const char *bmstr, const char *data);
R_API RSearchKeyword* r_search_keyword_new_hexmask(const char *kwstr, const char *data);
R_API RSearchKeyword *r_search_keyword_new_regexp (const char *str, const char *data);

R_API int r_search_kw_add(RSearch *s, RSearchKeyword *kw);
R_API void r_search_reset(RSearch *s, int mode);
R_API void r_search_kw_reset(RSearch *s);
R_API void r_search_string_prepare_backward(RSearch *s);
R_API void r_search_kw_reset(RSearch *s);

R_API int r_search_range_add(RSearch *s, ut64 from, ut64 to);
R_API int r_search_range_set(RSearch *s, ut64 from, ut64 to);
R_API int r_search_range_reset(RSearch *s);
R_API int r_search_set_blocksize(RSearch *s, ut32 bsize);

R_API int r_search_bmh(const RSearchKeyword *kw, const ut64 from, const ut8 *buf, const int len, ut64 *out);


R_API int r_search_mybinparse_update(RSearch *s, ut64 from, const ut8 *buf, int len);
R_API int r_search_aes_update(RSearch *s, ut64 from, const ut8 *buf, int len);
R_API int r_search_privkey_update(RSearch *s, ut64 from, const ut8 *buf, int len);
R_API int r_search_magic_update(RSearch *_s, ut64 from, const ut8 *buf, int len);
R_API int r_search_deltakey_update(RSearch *s, ut64 from, const ut8 *buf, int len);
R_API int r_search_strings_update(RSearch *s, ut64 from, const ut8 *buf, int len);
R_API int r_search_regexp_update(RSearch *s, ut64 from, const ut8 *buf, int len);
R_API int r_search_xrefs_update(RSearch *s, ut64 from, const ut8 *buf, int len);

R_API int r_search_hit_new(RSearch *s, RSearchKeyword *kw, ut64 addr);
R_API void r_search_set_distance(RSearch *s, int dist);
R_API int r_search_strings(RSearch *s, ut32 min, ut32 max);
R_API int r_search_set_string_limits(RSearch *s, ut32 min, ut32 max); 

R_API void r_search_set_callback(RSearch *s, RSearchCallback(callback), void *user);
R_API int r_search_begin(RSearch *s);


R_API void r_search_pattern_size(RSearch *s, int size);
R_API int r_search_pattern(RSearch *s, ut64 from, ut64 to);

#if defined(__cplusplus)
}
#endif

#endif
#endif
