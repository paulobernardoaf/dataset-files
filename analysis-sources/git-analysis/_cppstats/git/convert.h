#include "hash.h"
#include "string-list.h"
struct index_state;
struct strbuf;
#define CONV_EOL_RNDTRP_DIE (1<<0) 
#define CONV_EOL_RNDTRP_WARN (1<<1) 
#define CONV_EOL_RENORMALIZE (1<<2) 
#define CONV_EOL_KEEP_CRLF (1<<3) 
#define CONV_WRITE_OBJECT (1<<4) 
extern int global_conv_flags_eol;
enum auto_crlf {
AUTO_CRLF_FALSE = 0,
AUTO_CRLF_TRUE = 1,
AUTO_CRLF_INPUT = -1
};
extern enum auto_crlf auto_crlf;
enum eol {
EOL_UNSET,
EOL_CRLF,
EOL_LF,
#if defined(NATIVE_CRLF)
EOL_NATIVE = EOL_CRLF
#else
EOL_NATIVE = EOL_LF
#endif
};
enum ce_delay_state {
CE_NO_DELAY = 0,
CE_CAN_DELAY = 1,
CE_RETRY = 2
};
struct delayed_checkout {
enum ce_delay_state state;
struct string_list filters;
struct string_list paths;
};
struct checkout_metadata {
const char *refname;
struct object_id treeish;
struct object_id blob;
};
extern enum eol core_eol;
extern char *check_roundtrip_encoding;
const char *get_cached_convert_stats_ascii(const struct index_state *istate,
const char *path);
const char *get_wt_convert_stats_ascii(const char *path);
const char *get_convert_attr_ascii(const struct index_state *istate,
const char *path);
int convert_to_git(const struct index_state *istate,
const char *path, const char *src, size_t len,
struct strbuf *dst, int conv_flags);
int convert_to_working_tree(const struct index_state *istate,
const char *path, const char *src,
size_t len, struct strbuf *dst,
const struct checkout_metadata *meta);
int async_convert_to_working_tree(const struct index_state *istate,
const char *path, const char *src,
size_t len, struct strbuf *dst,
const struct checkout_metadata *meta,
void *dco);
int async_query_available_blobs(const char *cmd,
struct string_list *available_paths);
int renormalize_buffer(const struct index_state *istate,
const char *path, const char *src, size_t len,
struct strbuf *dst);
static inline int would_convert_to_git(const struct index_state *istate,
const char *path)
{
return convert_to_git(istate, path, NULL, 0, NULL, 0);
}
void convert_to_git_filter_fd(const struct index_state *istate,
const char *path, int fd,
struct strbuf *dst,
int conv_flags);
int would_convert_to_git_filter_fd(const struct index_state *istate,
const char *path);
void init_checkout_metadata(struct checkout_metadata *meta, const char *refname,
const struct object_id *treeish,
const struct object_id *blob);
void clone_checkout_metadata(struct checkout_metadata *dst,
const struct checkout_metadata *src,
const struct object_id *blob);
void reset_parsed_attributes(void);
struct stream_filter; 
struct stream_filter *get_stream_filter(const struct index_state *istate,
const char *path,
const struct object_id *);
void free_stream_filter(struct stream_filter *);
int is_null_stream_filter(struct stream_filter *);
int stream_filter(struct stream_filter *,
const char *input, size_t *isize_p,
char *output, size_t *osize_p);
