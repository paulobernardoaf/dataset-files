#if !defined(TRACE2_H)
#define TRACE2_H



































struct child_process;
struct repository;
struct json_writer;






























void trace2_initialize_clock(void);














void trace2_initialize_fl(const char *file, int line);

#define trace2_initialize() trace2_initialize_fl(__FILE__, __LINE__)




int trace2_is_enabled(void);




void trace2_cmd_start_fl(const char *file, int line, const char **argv);

#define trace2_cmd_start(argv) trace2_cmd_start_fl(__FILE__, __LINE__, (argv))










int trace2_cmd_exit_fl(const char *file, int line, int code);

#define trace2_cmd_exit(code) (trace2_cmd_exit_fl(__FILE__, __LINE__, (code)))






void trace2_cmd_error_va_fl(const char *file, int line, const char *fmt,
va_list ap);

#define trace2_cmd_error_va(fmt, ap) trace2_cmd_error_va_fl(__FILE__, __LINE__, (fmt), (ap))








void trace2_cmd_path_fl(const char *file, int line, const char *pathname);

#define trace2_cmd_path(p) trace2_cmd_path_fl(__FILE__, __LINE__, (p))






void trace2_cmd_name_fl(const char *file, int line, const char *name);

#define trace2_cmd_name(v) trace2_cmd_name_fl(__FILE__, __LINE__, (v))







void trace2_cmd_mode_fl(const char *file, int line, const char *mode);

#define trace2_cmd_mode(sv) trace2_cmd_mode_fl(__FILE__, __LINE__, (sv))





void trace2_cmd_alias_fl(const char *file, int line, const char *alias,
const char **argv);

#define trace2_cmd_alias(alias, argv) trace2_cmd_alias_fl(__FILE__, __LINE__, (alias), (argv))


















void trace2_cmd_list_config_fl(const char *file, int line);

#define trace2_cmd_list_config() trace2_cmd_list_config_fl(__FILE__, __LINE__)








void trace2_cmd_set_config_fl(const char *file, int line, const char *key,
const char *value);

#define trace2_cmd_set_config(k, v) trace2_cmd_set_config_fl(__FILE__, __LINE__, (k), (v))
















void trace2_child_start_fl(const char *file, int line,
struct child_process *cmd);

#define trace2_child_start(cmd) trace2_child_start_fl(__FILE__, __LINE__, (cmd))











void trace2_child_exit_fl(const char *file, int line, struct child_process *cmd,
int child_exit_code);

#define trace2_child_exit(cmd, code) trace2_child_exit_fl(__FILE__, __LINE__, (cmd), (code))












int trace2_exec_fl(const char *file, int line, const char *exe,
const char **argv);

#define trace2_exec(exe, argv) trace2_exec_fl(__FILE__, __LINE__, (exe), (argv))









void trace2_exec_result_fl(const char *file, int line, int exec_id, int code);

#define trace2_exec_result(id, code) trace2_exec_result_fl(__FILE__, __LINE__, (id), (code))









void trace2_thread_start_fl(const char *file, int line,
const char *thread_name);

#define trace2_thread_start(thread_name) trace2_thread_start_fl(__FILE__, __LINE__, (thread_name))







void trace2_thread_exit_fl(const char *file, int line);

#define trace2_thread_exit() trace2_thread_exit_fl(__FILE__, __LINE__)









void trace2_def_param_fl(const char *file, int line, const char *param,
const char *value);

#define trace2_def_param(param, value) trace2_def_param_fl(__FILE__, __LINE__, (param), (value))















void trace2_def_repo_fl(const char *file, int line, struct repository *repo);

#define trace2_def_repo(repo) trace2_def_repo_fl(__FILE__, __LINE__, repo)



















void trace2_region_enter_fl(const char *file, int line, const char *category,
const char *label, const struct repository *repo, ...);

#define trace2_region_enter(category, label, repo) trace2_region_enter_fl(__FILE__, __LINE__, (category), (label), (repo))


void trace2_region_enter_printf_va_fl(const char *file, int line,
const char *category, const char *label,
const struct repository *repo,
const char *fmt, va_list ap);

#define trace2_region_enter_printf_va(category, label, repo, fmt, ap) trace2_region_enter_printf_va_fl(__FILE__, __LINE__, (category), (label), (repo), (fmt), (ap))



void trace2_region_enter_printf_fl(const char *file, int line,
const char *category, const char *label,
const struct repository *repo,
const char *fmt, ...);

#if defined(HAVE_VARIADIC_MACROS)
#define trace2_region_enter_printf(category, label, repo, ...) trace2_region_enter_printf_fl(__FILE__, __LINE__, (category), (label), (repo), __VA_ARGS__)


#else

__attribute__((format (region_enter_printf, 4, 5)))
void trace2_region_enter_printf(const char *category, const char *label,
const struct repository *repo, const char *fmt,
...);

#endif













void trace2_region_leave_fl(const char *file, int line, const char *category,
const char *label, const struct repository *repo, ...);

#define trace2_region_leave(category, label, repo) trace2_region_leave_fl(__FILE__, __LINE__, (category), (label), (repo))


void trace2_region_leave_printf_va_fl(const char *file, int line,
const char *category, const char *label,
const struct repository *repo,
const char *fmt, va_list ap);

#define trace2_region_leave_printf_va(category, label, repo, fmt, ap) trace2_region_leave_printf_va_fl(__FILE__, __LINE__, (category), (label), (repo), (fmt), (ap))



void trace2_region_leave_printf_fl(const char *file, int line,
const char *category, const char *label,
const struct repository *repo,
const char *fmt, ...);

#if defined(HAVE_VARIADIC_MACROS)
#define trace2_region_leave_printf(category, label, repo, ...) trace2_region_leave_printf_fl(__FILE__, __LINE__, (category), (label), (repo), __VA_ARGS__)


#else

__attribute__((format (region_leave_printf, 4, 5)))
void trace2_region_leave_printf(const char *category, const char *label,
const struct repository *repo, const char *fmt,
...);

#endif












void trace2_data_string_fl(const char *file, int line, const char *category,
const struct repository *repo, const char *key,
const char *value);

#define trace2_data_string(category, repo, key, value) trace2_data_string_fl(__FILE__, __LINE__, (category), (repo), (key), (value))



void trace2_data_intmax_fl(const char *file, int line, const char *category,
const struct repository *repo, const char *key,
intmax_t value);

#define trace2_data_intmax(category, repo, key, value) trace2_data_intmax_fl(__FILE__, __LINE__, (category), (repo), (key), (value))



void trace2_data_json_fl(const char *file, int line, const char *category,
const struct repository *repo, const char *key,
const struct json_writer *jw);

#define trace2_data_json(category, repo, key, value) trace2_data_json_fl(__FILE__, __LINE__, (category), (repo), (key), (value))











void trace2_printf_va_fl(const char *file, int line, const char *fmt,
va_list ap);

#define trace2_printf_va(fmt, ap) trace2_printf_va_fl(__FILE__, __LINE__, (fmt), (ap))


void trace2_printf_fl(const char *file, int line, const char *fmt, ...);

#if defined(HAVE_VARIADIC_MACROS)
#define trace2_printf(...) trace2_printf_fl(__FILE__, __LINE__, __VA_ARGS__)
#else

__attribute__((format (printf, 1, 2)))
void trace2_printf(const char *fmt, ...);

#endif








enum trace2_process_info_reason {
TRACE2_PROCESS_INFO_STARTUP,
TRACE2_PROCESS_INFO_EXIT,
};

#if defined(GIT_WINDOWS_NATIVE)
void trace2_collect_process_info(enum trace2_process_info_reason reason);
#else
#define trace2_collect_process_info(reason) do { } while (0)


#endif

#endif 
