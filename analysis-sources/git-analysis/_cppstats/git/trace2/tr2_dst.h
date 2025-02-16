struct strbuf;
#include "trace2/tr2_sysenv.h"
struct tr2_dst {
enum tr2_sysenv_variable sysenv_var;
int fd;
unsigned int initialized : 1;
unsigned int need_close : 1;
unsigned int too_many_files : 1;
};
void tr2_dst_trace_disable(struct tr2_dst *dst);
int tr2_dst_get_trace_fd(struct tr2_dst *dst);
int tr2_dst_trace_want(struct tr2_dst *dst);
void tr2_dst_write_line(struct tr2_dst *dst, struct strbuf *buf_line);
