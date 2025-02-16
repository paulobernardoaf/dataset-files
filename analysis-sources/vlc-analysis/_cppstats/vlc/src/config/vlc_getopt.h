typedef struct vlc_getopt_s
{
char *arg;
int ind;
int opt;
char *nextchar;
int first_nonopt;
int last_nonopt;
} vlc_getopt_t;
struct vlc_option
{
const char *name;
bool has_arg;
int *flag;
int val;
};
extern int vlc_getopt_long(int argc, char *const *argv, const char *shortopts,
const struct vlc_option *longopts, int *longind,
vlc_getopt_t *restrict state);
