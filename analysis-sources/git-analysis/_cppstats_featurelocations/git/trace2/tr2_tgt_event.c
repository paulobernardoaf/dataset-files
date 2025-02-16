#include "cache.h"
#include "config.h"
#include "json-writer.h"
#include "run-command.h"
#include "version.h"
#include "trace2/tr2_dst.h"
#include "trace2/tr2_tbuf.h"
#include "trace2/tr2_sid.h"
#include "trace2/tr2_sysenv.h"
#include "trace2/tr2_tgt.h"
#include "trace2/tr2_tls.h"

static struct tr2_dst tr2dst_event = { TR2_SYSENV_EVENT, 0, 0, 0, 0 };









#define TR2_EVENT_VERSION "2"












static int tr2env_event_max_nesting_levels = 2;





static int tr2env_event_be_brief;

static int fn_init(void)
{
int want = tr2_dst_trace_want(&tr2dst_event);
int max_nesting;
int want_brief;
const char *nesting;
const char *brief;

if (!want)
return want;

nesting = tr2_sysenv_get(TR2_SYSENV_EVENT_NESTING);
if (nesting && *nesting && ((max_nesting = atoi(nesting)) > 0))
tr2env_event_max_nesting_levels = max_nesting;

brief = tr2_sysenv_get(TR2_SYSENV_EVENT_BRIEF);
if (brief && *brief &&
((want_brief = git_parse_maybe_bool(brief)) != -1))
tr2env_event_be_brief = want_brief;

return want;
}

static void fn_term(void)
{
tr2_dst_trace_disable(&tr2dst_event);
}











static void event_fmt_prepare(const char *event_name, const char *file,
int line, const struct repository *repo,
struct json_writer *jw)
{
struct tr2tls_thread_ctx *ctx = tr2tls_get_self();
struct tr2_tbuf tb_now;

jw_object_string(jw, "event", event_name);
jw_object_string(jw, "sid", tr2_sid_get());
jw_object_string(jw, "thread", ctx->thread_name.buf);




if (!tr2env_event_be_brief || !strcmp(event_name, "version") ||
!strcmp(event_name, "atexit")) {
tr2_tbuf_utc_datetime_extended(&tb_now);
jw_object_string(jw, "time", tb_now.buf);
}

if (!tr2env_event_be_brief && file && *file) {
jw_object_string(jw, "file", file);
jw_object_intmax(jw, "line", line);
}

if (repo)
jw_object_intmax(jw, "repo", repo->trace2_repo_id);
}

static void fn_too_many_files_fl(const char *file, int line)
{
const char *event_name = "too_many_files";
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_version_fl(const char *file, int line)
{
const char *event_name = "version";
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_object_string(&jw, "evt", TR2_EVENT_VERSION);
jw_object_string(&jw, "exe", git_version_string);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);

if (tr2dst_event.too_many_files)
fn_too_many_files_fl(file, line);
}

static void fn_start_fl(const char *file, int line,
uint64_t us_elapsed_absolute, const char **argv)
{
const char *event_name = "start";
struct json_writer jw = JSON_WRITER_INIT;
double t_abs = (double)us_elapsed_absolute / 1000000.0;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_object_double(&jw, "t_abs", 6, t_abs);
jw_object_inline_begin_array(&jw, "argv");
jw_array_argv(&jw, argv);
jw_end(&jw);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_exit_fl(const char *file, int line, uint64_t us_elapsed_absolute,
int code)
{
const char *event_name = "exit";
struct json_writer jw = JSON_WRITER_INIT;
double t_abs = (double)us_elapsed_absolute / 1000000.0;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_object_double(&jw, "t_abs", 6, t_abs);
jw_object_intmax(&jw, "code", code);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_signal(uint64_t us_elapsed_absolute, int signo)
{
const char *event_name = "signal";
struct json_writer jw = JSON_WRITER_INIT;
double t_abs = (double)us_elapsed_absolute / 1000000.0;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, __FILE__, __LINE__, NULL, &jw);
jw_object_double(&jw, "t_abs", 6, t_abs);
jw_object_intmax(&jw, "signo", signo);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_atexit(uint64_t us_elapsed_absolute, int code)
{
const char *event_name = "atexit";
struct json_writer jw = JSON_WRITER_INIT;
double t_abs = (double)us_elapsed_absolute / 1000000.0;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, __FILE__, __LINE__, NULL, &jw);
jw_object_double(&jw, "t_abs", 6, t_abs);
jw_object_intmax(&jw, "code", code);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void maybe_add_string_va(struct json_writer *jw, const char *field_name,
const char *fmt, va_list ap)
{
if (fmt && *fmt) {
va_list copy_ap;
struct strbuf buf = STRBUF_INIT;

va_copy(copy_ap, ap);
strbuf_vaddf(&buf, fmt, copy_ap);
va_end(copy_ap);

jw_object_string(jw, field_name, buf.buf);
strbuf_release(&buf);
return;
}
}

static void fn_error_va_fl(const char *file, int line, const char *fmt,
va_list ap)
{
const char *event_name = "error";
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
maybe_add_string_va(&jw, "msg", fmt, ap);






if (fmt && *fmt)
jw_object_string(&jw, "fmt", fmt);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_command_path_fl(const char *file, int line, const char *pathname)
{
const char *event_name = "cmd_path";
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_object_string(&jw, "path", pathname);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_command_name_fl(const char *file, int line, const char *name,
const char *hierarchy)
{
const char *event_name = "cmd_name";
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_object_string(&jw, "name", name);
if (hierarchy && *hierarchy)
jw_object_string(&jw, "hierarchy", hierarchy);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_command_mode_fl(const char *file, int line, const char *mode)
{
const char *event_name = "cmd_mode";
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_object_string(&jw, "name", mode);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_alias_fl(const char *file, int line, const char *alias,
const char **argv)
{
const char *event_name = "alias";
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_object_string(&jw, "alias", alias);
jw_object_inline_begin_array(&jw, "argv");
jw_array_argv(&jw, argv);
jw_end(&jw);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_child_start_fl(const char *file, int line,
uint64_t us_elapsed_absolute,
const struct child_process *cmd)
{
const char *event_name = "child_start";
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_object_intmax(&jw, "child_id", cmd->trace2_child_id);
if (cmd->trace2_hook_name) {
jw_object_string(&jw, "child_class", "hook");
jw_object_string(&jw, "hook_name", cmd->trace2_hook_name);
} else {
const char *child_class =
cmd->trace2_child_class ? cmd->trace2_child_class : "?";
jw_object_string(&jw, "child_class", child_class);
}
if (cmd->dir)
jw_object_string(&jw, "cd", cmd->dir);
jw_object_bool(&jw, "use_shell", cmd->use_shell);
jw_object_inline_begin_array(&jw, "argv");
if (cmd->git_cmd)
jw_array_string(&jw, "git");
jw_array_argv(&jw, cmd->argv);
jw_end(&jw);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_child_exit_fl(const char *file, int line,
uint64_t us_elapsed_absolute, int cid, int pid,
int code, uint64_t us_elapsed_child)
{
const char *event_name = "child_exit";
struct json_writer jw = JSON_WRITER_INIT;
double t_rel = (double)us_elapsed_child / 1000000.0;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_object_intmax(&jw, "child_id", cid);
jw_object_intmax(&jw, "pid", pid);
jw_object_intmax(&jw, "code", code);
jw_object_double(&jw, "t_rel", 6, t_rel);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);

jw_release(&jw);
}

static void fn_thread_start_fl(const char *file, int line,
uint64_t us_elapsed_absolute)
{
const char *event_name = "thread_start";
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_thread_exit_fl(const char *file, int line,
uint64_t us_elapsed_absolute,
uint64_t us_elapsed_thread)
{
const char *event_name = "thread_exit";
struct json_writer jw = JSON_WRITER_INIT;
double t_rel = (double)us_elapsed_thread / 1000000.0;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_object_double(&jw, "t_rel", 6, t_rel);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_exec_fl(const char *file, int line, uint64_t us_elapsed_absolute,
int exec_id, const char *exe, const char **argv)
{
const char *event_name = "exec";
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_object_intmax(&jw, "exec_id", exec_id);
if (exe)
jw_object_string(&jw, "exe", exe);
jw_object_inline_begin_array(&jw, "argv");
jw_array_argv(&jw, argv);
jw_end(&jw);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_exec_result_fl(const char *file, int line,
uint64_t us_elapsed_absolute, int exec_id,
int code)
{
const char *event_name = "exec_result";
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_object_intmax(&jw, "exec_id", exec_id);
jw_object_intmax(&jw, "code", code);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_param_fl(const char *file, int line, const char *param,
const char *value)
{
const char *event_name = "def_param";
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, NULL, &jw);
jw_object_string(&jw, "param", param);
jw_object_string(&jw, "value", value);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_repo_fl(const char *file, int line,
const struct repository *repo)
{
const char *event_name = "def_repo";
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, repo, &jw);
jw_object_string(&jw, "worktree", repo->worktree);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}

static void fn_region_enter_printf_va_fl(const char *file, int line,
uint64_t us_elapsed_absolute,
const char *category,
const char *label,
const struct repository *repo,
const char *fmt, va_list ap)
{
const char *event_name = "region_enter";
struct tr2tls_thread_ctx *ctx = tr2tls_get_self();
if (ctx->nr_open_regions <= tr2env_event_max_nesting_levels) {
struct json_writer jw = JSON_WRITER_INIT;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, repo, &jw);
jw_object_intmax(&jw, "nesting", ctx->nr_open_regions);
if (category)
jw_object_string(&jw, "category", category);
if (label)
jw_object_string(&jw, "label", label);
maybe_add_string_va(&jw, "msg", fmt, ap);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}
}

static void fn_region_leave_printf_va_fl(
const char *file, int line, uint64_t us_elapsed_absolute,
uint64_t us_elapsed_region, const char *category, const char *label,
const struct repository *repo, const char *fmt, va_list ap)
{
const char *event_name = "region_leave";
struct tr2tls_thread_ctx *ctx = tr2tls_get_self();
if (ctx->nr_open_regions <= tr2env_event_max_nesting_levels) {
struct json_writer jw = JSON_WRITER_INIT;
double t_rel = (double)us_elapsed_region / 1000000.0;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, repo, &jw);
jw_object_double(&jw, "t_rel", 6, t_rel);
jw_object_intmax(&jw, "nesting", ctx->nr_open_regions);
if (category)
jw_object_string(&jw, "category", category);
if (label)
jw_object_string(&jw, "label", label);
maybe_add_string_va(&jw, "msg", fmt, ap);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}
}

static void fn_data_fl(const char *file, int line, uint64_t us_elapsed_absolute,
uint64_t us_elapsed_region, const char *category,
const struct repository *repo, const char *key,
const char *value)
{
const char *event_name = "data";
struct tr2tls_thread_ctx *ctx = tr2tls_get_self();
if (ctx->nr_open_regions <= tr2env_event_max_nesting_levels) {
struct json_writer jw = JSON_WRITER_INIT;
double t_abs = (double)us_elapsed_absolute / 1000000.0;
double t_rel = (double)us_elapsed_region / 1000000.0;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, repo, &jw);
jw_object_double(&jw, "t_abs", 6, t_abs);
jw_object_double(&jw, "t_rel", 6, t_rel);
jw_object_intmax(&jw, "nesting", ctx->nr_open_regions);
jw_object_string(&jw, "category", category);
jw_object_string(&jw, "key", key);
jw_object_string(&jw, "value", value);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}
}

static void fn_data_json_fl(const char *file, int line,
uint64_t us_elapsed_absolute,
uint64_t us_elapsed_region, const char *category,
const struct repository *repo, const char *key,
const struct json_writer *value)
{
const char *event_name = "data_json";
struct tr2tls_thread_ctx *ctx = tr2tls_get_self();
if (ctx->nr_open_regions <= tr2env_event_max_nesting_levels) {
struct json_writer jw = JSON_WRITER_INIT;
double t_abs = (double)us_elapsed_absolute / 1000000.0;
double t_rel = (double)us_elapsed_region / 1000000.0;

jw_object_begin(&jw, 0);
event_fmt_prepare(event_name, file, line, repo, &jw);
jw_object_double(&jw, "t_abs", 6, t_abs);
jw_object_double(&jw, "t_rel", 6, t_rel);
jw_object_intmax(&jw, "nesting", ctx->nr_open_regions);
jw_object_string(&jw, "category", category);
jw_object_string(&jw, "key", key);
jw_object_sub_jw(&jw, "value", value);
jw_end(&jw);

tr2_dst_write_line(&tr2dst_event, &jw.json);
jw_release(&jw);
}
}

struct tr2_tgt tr2_tgt_event = {
&tr2dst_event,

fn_init,
fn_term,

fn_version_fl,
fn_start_fl,
fn_exit_fl,
fn_signal,
fn_atexit,
fn_error_va_fl,
fn_command_path_fl,
fn_command_name_fl,
fn_command_mode_fl,
fn_alias_fl,
fn_child_start_fl,
fn_child_exit_fl,
fn_thread_start_fl,
fn_thread_exit_fl,
fn_exec_fl,
fn_exec_result_fl,
fn_param_fl,
fn_repo_fl,
fn_region_enter_printf_va_fl,
fn_region_leave_printf_va_fl,
fn_data_fl,
fn_data_json_fl,
NULL, 
};
