




#include <stdbool.h>
#include <stdint.h>
#include <uv.h>

#include "nvim/os/dl.h"
#include "nvim/os/os.h"
#include "nvim/memory.h"
#include "nvim/message.h"






typedef void (*gen_fn)(void);
typedef const char *(*str_str_fn)(const char *str);
typedef int (*str_int_fn)(const char *str);
typedef const char *(*int_str_fn)(int64_t i);
typedef int (*int_int_fn)(int64_t i);
















bool os_libcall(const char *libname,
const char *funcname,
const char *argv,
int64_t argi,
char **str_out,
int *int_out)
{
if (!libname || !funcname) {
return false;
}

uv_lib_t lib;


if (uv_dlopen(libname, &lib)) {
EMSG2(_("dlerror = \"%s\""), uv_dlerror(&lib));
uv_dlclose(&lib);
return false;
}


gen_fn fn;
if (uv_dlsym(&lib, funcname, (void **) &fn)) {
EMSG2(_("dlerror = \"%s\""), uv_dlerror(&lib));
uv_dlclose(&lib);
return false;
}





if (str_out) {
str_str_fn sfn = (str_str_fn) fn;
int_str_fn ifn = (int_str_fn) fn;

const char *res = argv ? sfn(argv) : ifn(argi);


*str_out = (res && (intptr_t) res != 1 && (intptr_t) res != -1)
? xstrdup(res) : NULL;
} else {
str_int_fn sfn = (str_int_fn) fn;
int_int_fn ifn = (int_int_fn) fn;
*int_out = argv ? sfn(argv) : ifn(argi);
}


uv_dlclose(&lib);

return true;
}
