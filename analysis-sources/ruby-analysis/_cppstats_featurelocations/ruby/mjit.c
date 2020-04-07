












#include "ruby/config.h"

#if USE_MJIT

#include "constant.h"
#include "id_table.h"
#include "internal.h"
#include "internal/class.h"
#include "internal/cont.h"
#include "internal/file.h"
#include "internal/hash.h"
#include "internal/mjit.h"
#include "internal/warnings.h"

#include "mjit_worker.c"


static void
mjit_copy_job_handler(void *data)
{
mjit_copy_job_t *job = data;
if (stop_worker_p) { 
return;
}

CRITICAL_SECTION_START(3, "in mjit_copy_job_handler");




if (job->finish_p) {
CRITICAL_SECTION_FINISH(3, "in mjit_copy_job_handler");
return;
}
else if (job->iseq == NULL) { 
job->finish_p = true;
CRITICAL_SECTION_FINISH(3, "in mjit_copy_job_handler");
return;
}

const struct rb_iseq_constant_body *body = job->iseq->body;
if (job->is_entries) {
memcpy(job->is_entries, body->is_entries, sizeof(union iseq_inline_storage_entry) * body->is_size);
}

job->finish_p = true;
rb_native_cond_broadcast(&mjit_worker_wakeup);
CRITICAL_SECTION_FINISH(3, "in mjit_copy_job_handler");
}

extern int rb_thread_create_mjit_thread(void (*worker_func)(void));




static char *
get_uniq_filename(unsigned long id, const char *prefix, const char *suffix)
{
char buff[70], *str = buff;
int size = sprint_uniq_filename(buff, sizeof(buff), id, prefix, suffix);
str = 0;
++size;
str = xmalloc(size);
if (size <= (int)sizeof(buff)) {
memcpy(str, buff, size);
}
else {
sprint_uniq_filename(str, size, id, prefix, suffix);
}
return str;
}



void
mjit_gc_start_hook(void)
{
if (!mjit_enabled)
return;
CRITICAL_SECTION_START(4, "mjit_gc_start_hook");
while (in_jit) {
verbose(4, "Waiting wakeup from a worker for GC");
rb_native_cond_wait(&mjit_client_wakeup, &mjit_engine_mutex);
verbose(4, "Getting wakeup from a worker for GC");
}
in_gc = true;
CRITICAL_SECTION_FINISH(4, "mjit_gc_start_hook");
}



void
mjit_gc_exit_hook(void)
{
if (!mjit_enabled)
return;
CRITICAL_SECTION_START(4, "mjit_gc_exit_hook");
in_gc = false;
verbose(4, "Sending wakeup signal to workers after GC");
rb_native_cond_broadcast(&mjit_gc_wakeup);
CRITICAL_SECTION_FINISH(4, "mjit_gc_exit_hook");
}


void
mjit_update_references(const rb_iseq_t *iseq)
{
if (!mjit_enabled)
return;

CRITICAL_SECTION_START(4, "mjit_update_references");
if (iseq->body->jit_unit) {
iseq->body->jit_unit->iseq = (rb_iseq_t *)rb_gc_location((VALUE)iseq->body->jit_unit->iseq);



mjit_call_p = false; 
}




struct rb_mjit_unit *unit = NULL;
list_for_each(&stale_units.head, unit, unode) {
if (unit->iseq == iseq) {
unit->iseq = (rb_iseq_t *)rb_gc_location((VALUE)unit->iseq);
}
}
CRITICAL_SECTION_FINISH(4, "mjit_update_references");
}



void
mjit_free_iseq(const rb_iseq_t *iseq)
{
if (!mjit_enabled)
return;

CRITICAL_SECTION_START(4, "mjit_free_iseq");
if (mjit_copy_job.iseq == iseq) {
mjit_copy_job.iseq = NULL;
}
if (iseq->body->jit_unit) {


iseq->body->jit_unit->iseq = NULL;
}



struct rb_mjit_unit *unit = NULL;
list_for_each(&stale_units.head, unit, unode) {
if (unit->iseq == iseq) {
unit->iseq = NULL;
}
}
CRITICAL_SECTION_FINISH(4, "mjit_free_iseq");
}




static void
free_list(struct rb_mjit_unit_list *list, bool close_handle_p)
{
struct rb_mjit_unit *unit = 0, *next;

list_for_each_safe(&list->head, unit, next, unode) {
list_del(&unit->unode);
if (!close_handle_p) unit->handle = NULL; 

if (list == &stale_units) { 




if (unit->handle && dlclose(unit->handle)) {
mjit_warning("failed to close handle for u%d: %s", unit->id, dlerror());
}
clean_object_files(unit);
free(unit);
}
else {
free_unit(unit);
}
}
list->length = 0;
}


struct mjit_cont {
rb_execution_context_t *ec; 
struct mjit_cont *prev, *next; 
};



static struct mjit_cont *first_cont;



struct mjit_cont *
mjit_cont_new(rb_execution_context_t *ec)
{
struct mjit_cont *cont;

cont = ZALLOC(struct mjit_cont);
cont->ec = ec;

CRITICAL_SECTION_START(3, "in mjit_cont_new");
if (first_cont == NULL) {
cont->next = cont->prev = NULL;
}
else {
cont->prev = NULL;
cont->next = first_cont;
first_cont->prev = cont;
}
first_cont = cont;
CRITICAL_SECTION_FINISH(3, "in mjit_cont_new");

return cont;
}


void
mjit_cont_free(struct mjit_cont *cont)
{
CRITICAL_SECTION_START(3, "in mjit_cont_new");
if (cont == first_cont) {
first_cont = cont->next;
if (first_cont != NULL)
first_cont->prev = NULL;
}
else {
cont->prev->next = cont->next;
if (cont->next != NULL)
cont->next->prev = cont->prev;
}
CRITICAL_SECTION_FINISH(3, "in mjit_cont_new");

xfree(cont);
}


static void
finish_conts(void)
{
struct mjit_cont *cont, *next;

for (cont = first_cont; cont != NULL; cont = next) {
next = cont->next;
xfree(cont);
}
}


static void
create_unit(const rb_iseq_t *iseq)
{
struct rb_mjit_unit *unit;

unit = ZALLOC(struct rb_mjit_unit);
if (unit == NULL)
return;

unit->id = current_unit_num++;
unit->iseq = (rb_iseq_t *)iseq;
iseq->body->jit_unit = unit;
}


static void
mark_ec_units(rb_execution_context_t *ec)
{
const rb_control_frame_t *cfp;

if (ec->vm_stack == NULL)
return;
for (cfp = RUBY_VM_END_CONTROL_FRAME(ec) - 1; ; cfp = RUBY_VM_NEXT_CONTROL_FRAME(cfp)) {
const rb_iseq_t *iseq;
if (cfp->pc && (iseq = cfp->iseq) != NULL
&& imemo_type((VALUE) iseq) == imemo_iseq
&& (iseq->body->jit_unit) != NULL) {
iseq->body->jit_unit->used_code_p = TRUE;
}

if (cfp == ec->cfp)
break; 
}
}



static void
unload_units(void)
{
rb_vm_t *vm = GET_THREAD()->vm;
rb_thread_t *th = NULL;
struct rb_mjit_unit *unit = 0, *next, *worst;
struct mjit_cont *cont;
int delete_num, units_num = active_units.length;



list_for_each_safe(&active_units.head, unit, next, unode) {
if (unit->iseq == NULL) { 
remove_from_list(unit, &active_units);
free_unit(unit);
}
}


list_for_each(&active_units.head, unit, unode) {
assert(unit->iseq != NULL && unit->handle != NULL);
unit->used_code_p = FALSE;
}
list_for_each(&vm->living_threads, th, vmlt_node) {
mark_ec_units(th->ec);
}
for (cont = first_cont; cont != NULL; cont = cont->next) {
mark_ec_units(cont->ec);
}





delete_num = active_units.length / 10;
for (; active_units.length > mjit_opts.max_cache_size - delete_num;) {

worst = NULL;
list_for_each(&active_units.head, unit, unode) {
if (unit->used_code_p) 
continue;

if (worst == NULL || worst->iseq->body->total_calls > unit->iseq->body->total_calls) {
worst = unit;
}
}
if (worst == NULL)
break;


verbose(2, "Unloading unit %d (calls=%lu)", worst->id, worst->iseq->body->total_calls);
assert(worst->handle != NULL);
remove_from_list(worst, &active_units);
free_unit(worst);
}

if (units_num == active_units.length && mjit_opts.wait) {
mjit_opts.max_cache_size++; 
verbose(1, "No units can be unloaded -- incremented max-cache-size to %d for --jit-wait", mjit_opts.max_cache_size);
}
else {
verbose(1, "Too many JIT code -- %d units unloaded", units_num - active_units.length);
}
}

static void
mjit_add_iseq_to_process(const rb_iseq_t *iseq, const struct rb_mjit_compile_info *compile_info)
{
if (!mjit_enabled || pch_status == PCH_FAILED)
return;

RB_DEBUG_COUNTER_INC(mjit_add_iseq_to_process);
iseq->body->jit_func = (mjit_func_t)NOT_READY_JIT_ISEQ_FUNC;
create_unit(iseq);
if (iseq->body->jit_unit == NULL)

return;
if (compile_info != NULL)
iseq->body->jit_unit->compile_info = *compile_info;

CRITICAL_SECTION_START(3, "in add_iseq_to_process");
add_to_list(iseq->body->jit_unit, &unit_queue);
if (active_units.length >= mjit_opts.max_cache_size) {
RB_DEBUG_COUNTER_INC(mjit_unload_units);
unload_units();
}
verbose(3, "Sending wakeup signal to workers in mjit_add_iseq_to_process");
rb_native_cond_broadcast(&mjit_worker_wakeup);
CRITICAL_SECTION_FINISH(3, "in add_iseq_to_process");
}



void
rb_mjit_add_iseq_to_process(const rb_iseq_t *iseq)
{
mjit_add_iseq_to_process(iseq, NULL);
}


#define MJIT_WAIT_TIMEOUT_SECONDS 60

static void
mjit_wait(struct rb_iseq_constant_body *body)
{
struct timeval tv;
int tries = 0;
tv.tv_sec = 0;
tv.tv_usec = 1000;
while (body->jit_func == (mjit_func_t)NOT_READY_JIT_ISEQ_FUNC) {
tries++;
if (tries / 1000 > MJIT_WAIT_TIMEOUT_SECONDS || pch_status == PCH_FAILED) {
CRITICAL_SECTION_START(3, "in rb_mjit_wait_call to set jit_func");
body->jit_func = (mjit_func_t)NOT_COMPILED_JIT_ISEQ_FUNC; 
CRITICAL_SECTION_FINISH(3, "in rb_mjit_wait_call to set jit_func");
mjit_warning("timed out to wait for JIT finish");
break;
}

CRITICAL_SECTION_START(3, "in rb_mjit_wait_call for a client wakeup");
rb_native_cond_broadcast(&mjit_worker_wakeup);
CRITICAL_SECTION_FINISH(3, "in rb_mjit_wait_call for a client wakeup");
rb_thread_wait_for(tv);
}
}



VALUE
rb_mjit_wait_call(rb_execution_context_t *ec, struct rb_iseq_constant_body *body)
{
mjit_wait(body);
if ((uintptr_t)body->jit_func <= (uintptr_t)LAST_JIT_ISEQ_FUNC) {
return Qundef;
}
return body->jit_func(ec, ec->cfp);
}

struct rb_mjit_compile_info*
rb_mjit_iseq_compile_info(const struct rb_iseq_constant_body *body)
{
assert(body->jit_unit != NULL);
return &body->jit_unit->compile_info;
}

void
rb_mjit_recompile_iseq(const rb_iseq_t *iseq)
{
if ((uintptr_t)iseq->body->jit_func <= (uintptr_t)LAST_JIT_ISEQ_FUNC)
return;

verbose(1, "JIT recompile: %s@%s:%d", RSTRING_PTR(iseq->body->location.label),
RSTRING_PTR(rb_iseq_path(iseq)), FIX2INT(iseq->body->location.first_lineno));

CRITICAL_SECTION_START(3, "in rb_mjit_recompile_iseq");
remove_from_list(iseq->body->jit_unit, &active_units);
iseq->body->jit_func = (mjit_func_t)NOT_ADDED_JIT_ISEQ_FUNC;
add_to_list(iseq->body->jit_unit, &stale_units);
CRITICAL_SECTION_FINISH(3, "in rb_mjit_recompile_iseq");

mjit_add_iseq_to_process(iseq, &iseq->body->jit_unit->compile_info);
if (UNLIKELY(mjit_opts.wait)) {
mjit_wait(iseq->body);
}
}

extern VALUE ruby_archlibdir_path, ruby_prefix_path;


static bool
init_header_filename(void)
{
int fd;
#if defined(LOAD_RELATIVE)

VALUE basedir_val;
#endif
const char *basedir = "";
size_t baselen = 0;
char *p;
#if defined(_WIN32)
static const char libpathflag[] =
#if defined(_MSC_VER)
"-LIBPATH:"
#else
"-L"
#endif
;
const size_t libpathflag_len = sizeof(libpathflag) - 1;
#endif

#if defined(LOAD_RELATIVE)
basedir_val = ruby_prefix_path;
basedir = StringValuePtr(basedir_val);
baselen = RSTRING_LEN(basedir_val);
#else
if (getenv("MJIT_SEARCH_BUILD_DIR")) {




struct stat st;
const char *hdr = dlsym(RTLD_DEFAULT, "MJIT_HEADER");
if (!hdr) {
verbose(1, "No MJIT_HEADER");
}
else if (hdr[0] != '/') {
verbose(1, "Non-absolute header file path: %s", hdr);
}
else if (stat(hdr, &st) || !S_ISREG(st.st_mode)) {
verbose(1, "Non-file header file path: %s", hdr);
}
else if ((st.st_uid != getuid()) || (st.st_mode & 022) ||
!rb_path_check(hdr)) {
verbose(1, "Unsafe header file: uid=%ld mode=%#o %s",
(long)st.st_uid, (unsigned)st.st_mode, hdr);
return FALSE;
}
else {


verbose(3, "PRELOADENV("PRELOADENV")=%s", getenv(PRELOADENV));

unsetenv(PRELOADENV);
verbose(3, "MJIT_HEADER: %s", hdr);
header_file = ruby_strdup(hdr);
if (!header_file) return false;
}
}
else
#endif
#if !defined(_MSC_VER)
{

static const char header_name[] = MJIT_HEADER_INSTALL_DIR "/" MJIT_MIN_HEADER_NAME;
const size_t header_name_len = sizeof(header_name) - 1;

header_file = xmalloc(baselen + header_name_len + 1);
p = append_str2(header_file, basedir, baselen);
p = append_str2(p, header_name, header_name_len + 1);

if ((fd = rb_cloexec_open(header_file, O_RDONLY, 0)) < 0) {
verbose(1, "Cannot access header file: %s", header_file);
xfree(header_file);
header_file = NULL;
return false;
}
(void)close(fd);
}

pch_file = get_uniq_filename(0, MJIT_TMP_PREFIX "h", ".h.gch");
#else
{
static const char pch_name[] = MJIT_HEADER_INSTALL_DIR "/" MJIT_PRECOMPILED_HEADER_NAME;
const size_t pch_name_len = sizeof(pch_name) - 1;

pch_file = xmalloc(baselen + pch_name_len + 1);
p = append_str2(pch_file, basedir, baselen);
p = append_str2(p, pch_name, pch_name_len + 1);
if ((fd = rb_cloexec_open(pch_file, O_RDONLY, 0)) < 0) {
verbose(1, "Cannot access precompiled header file: %s", pch_file);
xfree(pch_file);
pch_file = NULL;
return false;
}
(void)close(fd);
}
#endif

#if defined(_WIN32)
basedir_val = ruby_archlibdir_path;
basedir = StringValuePtr(basedir_val);
baselen = RSTRING_LEN(basedir_val);
libruby_pathflag = p = xmalloc(libpathflag_len + baselen + 1);
p = append_str(p, libpathflag);
p = append_str2(p, basedir, baselen);
*p = '\0';
#endif

return true;
}

static enum rb_id_table_iterator_result
valid_class_serials_add_i(ID key, VALUE v, void *unused)
{
rb_const_entry_t *ce = (rb_const_entry_t *)v;
VALUE value = ce->value;

if (!rb_is_const_id(key)) return ID_TABLE_CONTINUE;
if (RB_TYPE_P(value, T_MODULE) || RB_TYPE_P(value, T_CLASS)) {
mjit_add_class_serial(RCLASS_SERIAL(value));
}
return ID_TABLE_CONTINUE;
}

#if defined(_WIN32)
UINT rb_w32_system_tmpdir(WCHAR *path, UINT len);
#endif

static char *
system_default_tmpdir(void)
{

#if defined(_WIN32)
WCHAR tmppath[_MAX_PATH];
UINT len = rb_w32_system_tmpdir(tmppath, numberof(tmppath));
if (len) {
int blen = WideCharToMultiByte(CP_UTF8, 0, tmppath, len, NULL, 0, NULL, NULL);
char *tmpdir = xmalloc(blen + 1);
WideCharToMultiByte(CP_UTF8, 0, tmppath, len, tmpdir, blen, NULL, NULL);
tmpdir[blen] = '\0';
return tmpdir;
}
#elif defined _CS_DARWIN_USER_TEMP_DIR
char path[MAXPATHLEN];
size_t len = confstr(_CS_DARWIN_USER_TEMP_DIR, path, sizeof(path));
if (len > 0) {
char *tmpdir = xmalloc(len);
if (len > sizeof(path)) {
confstr(_CS_DARWIN_USER_TEMP_DIR, tmpdir, len);
}
else {
memcpy(tmpdir, path, len);
}
return tmpdir;
}
#endif
return 0;
}

static int
check_tmpdir(const char *dir)
{
struct stat st;

if (!dir) return FALSE;
if (stat(dir, &st)) return FALSE;
#if !defined(S_ISDIR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
if (!S_ISDIR(st.st_mode)) return FALSE;
#if !defined(_WIN32)
#if !defined(S_IWOTH)
#define S_IWOTH 002
#endif
if (st.st_mode & S_IWOTH) {
#if defined(S_ISVTX)
if (!(st.st_mode & S_ISVTX)) return FALSE;
#else
return FALSE;
#endif
}
if (access(dir, W_OK)) return FALSE;
#endif
return TRUE;
}

static char *
system_tmpdir(void)
{
char *tmpdir;
#define RETURN_ENV(name) if (check_tmpdir(tmpdir = getenv(name))) return ruby_strdup(tmpdir)

RETURN_ENV("TMPDIR");
RETURN_ENV("TMP");
tmpdir = system_default_tmpdir();
if (check_tmpdir(tmpdir)) return tmpdir;
return ruby_strdup("/tmp");
#undef RETURN_ENV
}


#define MIN_CACHE_SIZE 10

#define DEFAULT_MAX_CACHE_SIZE 100

#define DEFAULT_MIN_CALLS_TO_ADD 10000


static bool
start_worker(void)
{
stop_worker_p = false;
worker_stopped = false;

if (!rb_thread_create_mjit_thread(mjit_worker)) {
mjit_enabled = false;
rb_native_mutex_destroy(&mjit_engine_mutex);
rb_native_cond_destroy(&mjit_pch_wakeup);
rb_native_cond_destroy(&mjit_client_wakeup);
rb_native_cond_destroy(&mjit_worker_wakeup);
rb_native_cond_destroy(&mjit_gc_wakeup);
verbose(1, "Failure in MJIT thread initialization\n");
return false;
}
return true;
}


static char*
ruby_strndup(const char *str, size_t n)
{
char *ret = xmalloc(n + 1);
memcpy(ret, str, n);
ret[n] = '\0';
return ret;
}



static char **
split_flags(const char *flags)
{
char *buf[MAXPATHLEN];
int i = 0;
char *next;
for (; flags != NULL; flags = next) {
next = strchr(flags, ' ');
if (next == NULL) {
if (strlen(flags) > 0)
buf[i++] = strdup(flags);
}
else {
if (next > flags)
buf[i++] = ruby_strndup(flags, next - flags);
next++; 
}
}

char **ret = xmalloc(sizeof(char *) * (i + 1));
memcpy(ret, buf, sizeof(char *) * i);
ret[i] = NULL;
return ret;
}




void
mjit_init(const struct mjit_options *opts)
{
mjit_opts = *opts;
mjit_enabled = true;
mjit_call_p = true;


if (mjit_opts.min_calls == 0)
mjit_opts.min_calls = DEFAULT_MIN_CALLS_TO_ADD;
if (mjit_opts.max_cache_size <= 0)
mjit_opts.max_cache_size = DEFAULT_MAX_CACHE_SIZE;
if (mjit_opts.max_cache_size < MIN_CACHE_SIZE)
mjit_opts.max_cache_size = MIN_CACHE_SIZE;


#if defined(_MSC_VER)
pch_status = PCH_SUCCESS; 
#else
pch_status = PCH_NOT_READY;
#endif
cc_path = CC_COMMON_ARGS[0];
verbose(2, "MJIT: CC defaults to %s", cc_path);
cc_common_args = xmalloc(sizeof(CC_COMMON_ARGS));
memcpy((void *)cc_common_args, CC_COMMON_ARGS, sizeof(CC_COMMON_ARGS));
cc_added_args = split_flags(opts->debug_flags);
xfree(opts->debug_flags);
#if MJIT_CFLAGS_PIPE

for (size_t i = 0, j = 0; i < sizeof(CC_COMMON_ARGS) / sizeof(char *); i++) {
if (CC_COMMON_ARGS[i] && strncmp("-save-temps", CC_COMMON_ARGS[i], strlen("-save-temps")) == 0)
continue; 
cc_common_args[j] = CC_COMMON_ARGS[i];
j++;
}
#endif

tmp_dir = system_tmpdir();
verbose(2, "MJIT: tmp_dir is %s", tmp_dir);

if (!init_header_filename()) {
mjit_enabled = false;
verbose(1, "Failure in MJIT header file name initialization\n");
return;
}
pch_owner_pid = getpid();


rb_native_mutex_initialize(&mjit_engine_mutex);
rb_native_cond_initialize(&mjit_pch_wakeup);
rb_native_cond_initialize(&mjit_client_wakeup);
rb_native_cond_initialize(&mjit_worker_wakeup);
rb_native_cond_initialize(&mjit_gc_wakeup);


rb_fiber_init_mjit_cont(GET_EC()->fiber_ptr);


valid_class_serials = rb_hash_new();
rb_obj_hide(valid_class_serials);
rb_gc_register_mark_object(valid_class_serials);
mjit_add_class_serial(RCLASS_SERIAL(rb_cObject));
mjit_add_class_serial(RCLASS_SERIAL(CLASS_OF(rb_vm_top_self())));
if (RCLASS_CONST_TBL(rb_cObject)) {
rb_id_table_foreach(RCLASS_CONST_TBL(rb_cObject), valid_class_serials_add_i, NULL);
}


start_worker();
}

static void
stop_worker(void)
{
rb_execution_context_t *ec = GET_EC();

while (!worker_stopped) {
verbose(3, "Sending cancel signal to worker");
CRITICAL_SECTION_START(3, "in stop_worker");
stop_worker_p = true; 
rb_native_cond_broadcast(&mjit_worker_wakeup);
CRITICAL_SECTION_FINISH(3, "in stop_worker");
RUBY_VM_CHECK_INTS(ec);
}
}


VALUE
mjit_pause(bool wait_p)
{
if (!mjit_enabled) {
rb_raise(rb_eRuntimeError, "MJIT is not enabled");
}
if (worker_stopped) {
return Qfalse;
}


if (wait_p) {
struct timeval tv;
tv.tv_sec = 0;
tv.tv_usec = 1000;

while (unit_queue.length > 0 && active_units.length < mjit_opts.max_cache_size) { 
CRITICAL_SECTION_START(3, "in mjit_pause for a worker wakeup");
rb_native_cond_broadcast(&mjit_worker_wakeup);
CRITICAL_SECTION_FINISH(3, "in mjit_pause for a worker wakeup");
rb_thread_wait_for(tv);
}
}

stop_worker();
return Qtrue;
}


VALUE
mjit_resume(void)
{
if (!mjit_enabled) {
rb_raise(rb_eRuntimeError, "MJIT is not enabled");
}
if (!worker_stopped) {
return Qfalse;
}

if (!start_worker()) {
rb_raise(rb_eRuntimeError, "Failed to resume MJIT worker");
}
return Qtrue;
}


static void
skip_cleaning_object_files(struct rb_mjit_unit_list *list)
{
struct rb_mjit_unit *unit = NULL, *next;


list_for_each_safe(&list->head, unit, next, unode) {
#if !defined(_MSC_VER)
if (unit->o_file) unit->o_file_inherited_p = true;
#endif

#if defined(_WIN32) 
if (unit->so_file) unit->so_file = NULL;
#endif
}
}
















void
mjit_child_after_fork(void)
{
if (!mjit_enabled)
return;



skip_cleaning_object_files(&active_units);


start_worker();
}







void
mjit_finish(bool close_handle_p)
{
if (!mjit_enabled)
return;


verbose(2, "Stopping worker thread");
CRITICAL_SECTION_START(3, "in mjit_finish to wakeup from pch");





while (pch_status == PCH_NOT_READY) {
verbose(3, "Waiting wakeup from make_pch");
rb_native_cond_wait(&mjit_pch_wakeup, &mjit_engine_mutex);
}
CRITICAL_SECTION_FINISH(3, "in mjit_finish to wakeup from pch");


stop_worker();

rb_native_mutex_destroy(&mjit_engine_mutex);
rb_native_cond_destroy(&mjit_pch_wakeup);
rb_native_cond_destroy(&mjit_client_wakeup);
rb_native_cond_destroy(&mjit_worker_wakeup);
rb_native_cond_destroy(&mjit_gc_wakeup);

#if !defined(_MSC_VER)
if (!mjit_opts.save_temps && getpid() == pch_owner_pid)
remove_file(pch_file);

xfree(header_file); header_file = NULL;
#endif
xfree((void *)cc_common_args); cc_common_args = NULL;
for (char **flag = cc_added_args; *flag != NULL; flag++)
xfree(*flag);
xfree((void *)cc_added_args); cc_added_args = NULL;
xfree(tmp_dir); tmp_dir = NULL;
xfree(pch_file); pch_file = NULL;

mjit_call_p = false;
free_list(&unit_queue, close_handle_p);
free_list(&active_units, close_handle_p);
free_list(&compact_units, close_handle_p);
free_list(&stale_units, close_handle_p);
finish_conts();

mjit_enabled = false;
verbose(1, "Successful MJIT finish");
}


void
mjit_mark(void)
{
if (!mjit_enabled)
return;
RUBY_MARK_ENTER("mjit");

CRITICAL_SECTION_START(4, "mjit_mark");
VALUE iseq = (VALUE)mjit_copy_job.iseq;
CRITICAL_SECTION_FINISH(4, "mjit_mark");



if (iseq) rb_gc_mark(iseq);

struct rb_mjit_unit *unit = NULL;
CRITICAL_SECTION_START(4, "mjit_mark");
list_for_each(&unit_queue.head, unit, unode) {
if (unit->iseq) { 
iseq = (VALUE)unit->iseq;
CRITICAL_SECTION_FINISH(4, "mjit_mark rb_gc_mark");



rb_gc_mark(iseq);

CRITICAL_SECTION_START(4, "mjit_mark rb_gc_mark");
}
}
CRITICAL_SECTION_FINISH(4, "mjit_mark");

RUBY_MARK_LEAVE("mjit");
}


void
mjit_mark_cc_entries(const struct rb_iseq_constant_body *const body)
{
const struct rb_callcache **cc_entries;
if (body->jit_unit && (cc_entries = body->jit_unit->cc_entries) != NULL) {

for (unsigned int i = 0; i < body->jit_unit->cc_entries_size; i++) {
const struct rb_callcache *cc = cc_entries[i];
if (cc != NULL) {

rb_gc_mark((VALUE)cc);
rb_gc_mark((VALUE)vm_cc_cme(cc));
}
}
}
}


void
mjit_add_class_serial(rb_serial_t class_serial)
{
if (!mjit_enabled)
return;



rb_hash_aset(valid_class_serials, LONG2FIX(class_serial), Qtrue);
}


void
mjit_remove_class_serial(rb_serial_t class_serial)
{
if (!mjit_enabled)
return;

CRITICAL_SECTION_START(3, "in mjit_remove_class_serial");
rb_hash_delete_entry(valid_class_serials, LONG2FIX(class_serial));
CRITICAL_SECTION_FINISH(3, "in mjit_remove_class_serial");
}

#endif
