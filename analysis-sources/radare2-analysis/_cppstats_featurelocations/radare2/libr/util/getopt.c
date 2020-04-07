






#include <r_util.h>

#define BADCH (int)'?'
#define BADARG (int)':'
#define EMSG ""

R_API void r_getopt_init(RGetopt *opt, int argc, const char **argv, const char *ostr) {
memset (opt, 0, sizeof (RGetopt));
opt->err = 1;
opt->ind = 1;
opt->opt = 0;
opt->reset = 0;
opt->arg = NULL;
opt->argc = argc;
opt->argv = argv;
opt->ostr = ostr;
}

R_API int r_getopt_next(RGetopt *opt) {
static const char *place = EMSG; 
const char *oli; 

if (opt->reset || !*place) { 
opt->reset = 0;
if (opt->ind >= opt->argc || *(place = opt->argv[opt->ind]) != '-') {
place = EMSG;
return -1;
}
if (place[1] && *++place == '-') { 
opt->ind++;
place = EMSG;
return -1;
}
}

if ((opt->opt = (int)*place++) == (int)':' || !(oli = strchr (opt->ostr, opt->opt))) {




if (opt->opt == '-') {
return -1;
}
if (!*place) {
opt->ind++;
}
if (opt->err && *opt->ostr != ':') {
(void)eprintf ("%s: illegal option -- %c\n", opt->argv[0], opt->opt);
}
return BADCH;
}
if (*++oli == ':') { 
if (*place) { 
opt->arg = place;
} else if (opt->argc <= ++opt->ind) { 
place = EMSG;
if (*opt->ostr == ':') {
return BADARG;
}
if (opt->err) {
(void)eprintf ("%s: option requires an argument -- %c\n", opt->argv[0], opt->opt);
}
return BADCH;
} else { 
opt->arg = opt->argv[opt->ind];
}
place = EMSG;
opt->ind++;
} else {
opt->arg = NULL;
if (!*place) {
opt->ind++;
}
}

return opt->opt;
}
