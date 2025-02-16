struct index_state;
typedef const char *(*nth_line_fn_t)(void *data, long lno);
int parse_range_arg(const char *arg,
nth_line_fn_t nth_line_cb,
void *cb_data, long lines, long anchor,
long *begin, long *end,
const char *path, struct index_state *istate);
const char *skip_range_arg(const char *arg, struct index_state *istate);
