#include "r_lib.h"
#include "r_core.h"
#include "r_lang.h"
static int ac = 0;
static const char **av = NULL;
static bool lang_c_set_argv(RLang *lang, int argc, const char **argv) {
ac = argc;
av = argv;
return true;
}
static int lang_c_file(RLang *lang, const char *file) {
char *a, *cc, *p, name[512];
const char *libpath, *libname;
void *lib;
if (strlen (file) > (sizeof (name) - 10)) {
return false;
}
if (!strstr (file, ".c")) {
sprintf (name, "%s.c", file);
} else {
strcpy (name, file);
}
if (!r_file_exists (name)) {
eprintf ("file not found (%s)\n", name);
return false;
}
a = (char*)r_str_lchr (name, '/');
if (a) {
*a = 0;
libpath = name;
libname = a + 1;
} else {
libpath = ".";
libname = name;
}
r_sys_setenv ("PKG_CONFIG_PATH", R2_LIBDIR"/pkgconfig");
p = strstr (name, ".c");
if (p) {
*p=0;
}
cc = r_sys_getenv ("CC");
if (!cc || !*cc) {
cc = strdup ("gcc");
}
char *buf = r_str_newf ("%s -fPIC -shared %s -o %s/lib%s."R_LIB_EXT
" $(pkg-config --cflags --libs r_core)", cc, file, libpath, libname);
free (cc);
if (r_sandbox_system (buf, 1) != 0) {
free (buf);
return false;
}
free (buf);
buf = r_str_newf ("%s/lib%s."R_LIB_EXT, libpath, libname);
lib = r_lib_dl_open (buf);
if (lib) {
void (*fcn)(RCore *, int argc, const char **argv);
fcn = r_lib_dl_sym (lib, "entry");
if (fcn) {
fcn (lang->user, ac, av);
ac = 0;
av = NULL;
} else {
eprintf ("Cannot find 'entry' symbol in library\n");
}
r_lib_dl_close (lib);
} else {
eprintf ("Cannot open library\n");
}
r_file_rm (buf); 
free (buf);
return 0;
}
static int lang_c_init(void *user) {
return true;
}
static int lang_c_run(RLang *lang, const char *code, int len) {
FILE *fd = r_sandbox_fopen (".tmp.c", "w");
if (fd) {
fputs ("#include <r_core.h>\n\nvoid entry(RCore *core, int argc, const char **argv) {\n", fd);
fputs (code, fd);
fputs ("\n}\n", fd);
fclose (fd);
lang_c_file (lang, ".tmp.c");
r_file_rm (".tmp.c");
} else eprintf ("Cannot open .tmp.c\n");
return true;
}
static RLangPlugin r_lang_plugin_c = {
.name = "c",
.ext = "c",
.desc = "C language extension",
.license = "LGPL",
.run = lang_c_run,
.init = (void*)lang_c_init,
.run_file = (void*)lang_c_file,
.set_argv = (void*)lang_c_set_argv,
};
