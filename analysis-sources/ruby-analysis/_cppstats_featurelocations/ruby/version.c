










#include "ruby/ruby.h"
#include "version.h"
#include "vm_core.h"
#include "mjit.h"
#include <stdio.h>

#if !defined(EXIT_SUCCESS)
#define EXIT_SUCCESS 0
#endif

#define PRINT(type) puts(ruby_##type)
#define MKSTR(type) rb_obj_freeze(rb_usascii_str_new_static(ruby_##type, sizeof(ruby_##type)-1))
#define MKINT(name) INT2FIX(ruby_##name)

const int ruby_api_version[] = {
RUBY_API_VERSION_MAJOR,
RUBY_API_VERSION_MINOR,
RUBY_API_VERSION_TEENY,
};
#define RUBY_VERSION STRINGIZE(RUBY_VERSION_MAJOR) "." STRINGIZE(RUBY_VERSION_MINOR) "." STRINGIZE(RUBY_VERSION_TEENY) ""



#if !defined(RUBY_FULL_REVISION)
#define RUBY_FULL_REVISION RUBY_REVISION
#endif
const char ruby_version[] = RUBY_VERSION;
const char ruby_revision[] = RUBY_FULL_REVISION;
const char ruby_release_date[] = RUBY_RELEASE_DATE;
const char ruby_platform[] = RUBY_PLATFORM;
const int ruby_patchlevel = RUBY_PATCHLEVEL;
const char ruby_description[] = RUBY_DESCRIPTION_WITH("");
static const char ruby_description_with_jit[] = RUBY_DESCRIPTION_WITH(" +JIT");
const char ruby_copyright[] = RUBY_COPYRIGHT;
const char ruby_engine[] = "ruby";


void
Init_version(void)
{
enum {ruby_patchlevel = RUBY_PATCHLEVEL};
VALUE version;
VALUE ruby_engine_name;



rb_define_global_const("RUBY_VERSION", (version = MKSTR(version)));



rb_define_global_const("RUBY_RELEASE_DATE", MKSTR(release_date));



rb_define_global_const("RUBY_PLATFORM", MKSTR(platform));




rb_define_global_const("RUBY_PATCHLEVEL", MKINT(patchlevel));



rb_define_global_const("RUBY_REVISION", MKSTR(revision));



rb_define_global_const("RUBY_COPYRIGHT", MKSTR(copyright));



rb_define_global_const("RUBY_ENGINE", ruby_engine_name = MKSTR(engine));
ruby_set_script_name(ruby_engine_name);



rb_define_global_const("RUBY_ENGINE_VERSION", (1 ? version : MKSTR(version)));

rb_provide("ruby2_keywords.rb");
}

#if USE_MJIT
#define MJIT_OPTS_ON mjit_opts.on
#else
#define MJIT_OPTS_ON 0
#endif

void
Init_ruby_description(void)
{
VALUE description;

if (MJIT_OPTS_ON) {
description = MKSTR(description_with_jit);
}
else {
description = MKSTR(description);
}




rb_define_global_const("RUBY_DESCRIPTION", description);
}


void
ruby_show_version(void)
{
if (MJIT_OPTS_ON) {
PRINT(description_with_jit);
}
else {
PRINT(description);
}
#if defined(RUBY_LAST_COMMIT_TITLE)
fputs("last_commit=" RUBY_LAST_COMMIT_TITLE, stdout);
#endif
#if defined(HAVE_MALLOC_CONF)
if (malloc_conf) printf("malloc_conf=%s\n", malloc_conf);
#endif
fflush(stdout);
}


void
ruby_show_copyright(void)
{
PRINT(copyright);
fflush(stdout);
}
