




















#if !defined(VLC_DIALOG_H_)
#define VLC_DIALOG_H_
#include <stdarg.h>

typedef struct vlc_dialog_provider vlc_dialog_provider;
typedef struct vlc_dialog_id vlc_dialog_id;
typedef struct extension_dialog_t extension_dialog_t;


int
libvlc_InternalDialogInit(libvlc_int_t *p_libvlc);


void
libvlc_InternalDialogClean(libvlc_int_t *p_libvlc);















typedef enum vlc_dialog_question_type
{
VLC_DIALOG_QUESTION_NORMAL,
VLC_DIALOG_QUESTION_WARNING,
VLC_DIALOG_QUESTION_CRITICAL,
} vlc_dialog_question_type;











VLC_API int
vlc_dialog_display_error(vlc_object_t *p_obj, const char *psz_title,
const char *psz_fmt, ...) VLC_FORMAT(3,4);
#define vlc_dialog_display_error(a, b, c, ...) vlc_dialog_display_error(VLC_OBJECT(a), b, c, ##__VA_ARGS__)








VLC_API int
vlc_dialog_display_error_va(vlc_object_t *p_obj, const char *psz_title,
const char *psz_fmt, va_list ap);





















VLC_API int
vlc_dialog_wait_login(vlc_object_t *p_obj, char **ppsz_username,
char **ppsz_password, bool *p_store,
const char *psz_default_username,
const char *psz_title, const char *psz_fmt, ...)
VLC_FORMAT(7,8);
#define vlc_dialog_wait_login(a, b, c, d, e, f, g, ...) vlc_dialog_wait_login(VLC_OBJECT(a), b, c, d, e, f, g, ##__VA_ARGS__)








VLC_API int
vlc_dialog_wait_login_va(vlc_object_t *p_obj, char **ppsz_username,
char **ppsz_password, bool *p_store,
const char *psz_default_username,
const char *psz_title, const char *psz_fmt, va_list ap);


















VLC_API int
vlc_dialog_wait_question(vlc_object_t *p_obj,
vlc_dialog_question_type i_type,
const char *psz_cancel, const char *psz_action1,
const char *psz_action2, const char *psz_title,
const char *psz_fmt, ...) VLC_FORMAT(7,8);
#define vlc_dialog_wait_question(a, b, c, d, e, f, g, ...) vlc_dialog_wait_question(VLC_OBJECT(a), b, c, d, e, f, g, ##__VA_ARGS__)








VLC_API int
vlc_dialog_wait_question_va(vlc_object_t *p_obj,
vlc_dialog_question_type i_type,
const char *psz_cancel, const char *psz_action1,
const char *psz_action2, const char *psz_title,
const char *psz_fmt, va_list ap);
















VLC_API vlc_dialog_id *
vlc_dialog_display_progress(vlc_object_t *p_obj, bool b_indeterminate,
float f_position, const char *psz_cancel,
const char *psz_title, const char *psz_fmt, ...)
VLC_FORMAT(6,7);
#define vlc_dialog_display_progress(a, b, c, d, e, f, ...) vlc_dialog_display_progress(VLC_OBJECT(a), b, c, d, e, f, ##__VA_ARGS__)








VLC_API vlc_dialog_id *
vlc_dialog_display_progress_va(vlc_object_t *p_obj, bool b_indeterminate,
float f_position, const char *psz_cancel,
const char *psz_title, const char *psz_fmt,
va_list ap);









VLC_API int
vlc_dialog_update_progress(vlc_object_t *p_obj, vlc_dialog_id *p_id,
float f_position);
#define vlc_dialog_update_progress(a, b, c) vlc_dialog_update_progress(VLC_OBJECT(a), b, c)











VLC_API int
vlc_dialog_update_progress_text(vlc_object_t *p_obj, vlc_dialog_id *p_id,
float f_position, const char *psz_fmt, ...)
VLC_FORMAT(4, 5);
#define vlc_dialog_update_progress_text(a, b, c, d, ...) vlc_dialog_update_progress_text(VLC_OBJECT(a), b, c, d, ##__VA_ARGS__)








VLC_API int
vlc_dialog_update_progress_text_va(vlc_object_t *p_obj, vlc_dialog_id *p_id,
float f_position, const char *psz_fmt,
va_list ap);









VLC_API void
vlc_dialog_release(vlc_object_t *p_obj, vlc_dialog_id *p_id);
#define vlc_dialog_release(a, b) vlc_dialog_release(VLC_OBJECT(a), b)








VLC_API bool
vlc_dialog_is_cancelled(vlc_object_t *p_obj, vlc_dialog_id *p_id);
#define vlc_dialog_is_cancelled(a, b) vlc_dialog_is_cancelled(VLC_OBJECT(a), b)












typedef struct vlc_dialog_cbs
{







void (*pf_display_error)(void *p_data, const char *psz_title,
const char *psz_text);


















void (*pf_display_login)(void *p_data, vlc_dialog_id *p_id,
const char *psz_title, const char *psz_text,
const char *psz_default_username,
bool b_ask_store);





















void (*pf_display_question)(void *p_data, vlc_dialog_id *p_id,
const char *psz_title, const char *psz_text,
vlc_dialog_question_type i_type,
const char *psz_cancel, const char *psz_action1,
const char *psz_action2);




















void (*pf_display_progress)(void *p_data, vlc_dialog_id *p_id,
const char *psz_title, const char *psz_text,
bool b_indeterminate, float f_position,
const char *psz_cancel);










void (*pf_cancel)(void *p_data, vlc_dialog_id *p_id);









void (*pf_update_progress)(void *p_data, vlc_dialog_id *p_id,
float f_position, const char *psz_text);
} vlc_dialog_cbs;







VLC_API void
vlc_dialog_provider_set_callbacks(vlc_object_t *p_obj,
const vlc_dialog_cbs *p_cbs, void *p_data);
#define vlc_dialog_provider_set_callbacks(a, b, c) vlc_dialog_provider_set_callbacks(VLC_OBJECT(a), b, c)





VLC_API void
vlc_dialog_id_set_context(vlc_dialog_id *p_id, void *p_context);




VLC_API void *
vlc_dialog_id_get_context(vlc_dialog_id *p_id);














VLC_API int
vlc_dialog_id_post_login(vlc_dialog_id *p_id, const char *psz_username,
const char *psz_password, bool b_store);












VLC_API int
vlc_dialog_id_post_action(vlc_dialog_id *p_id, int i_action);











VLC_API int
vlc_dialog_id_dismiss(vlc_dialog_id *p_id);







VLC_API int
vlc_ext_dialog_update(vlc_object_t *p_obj, extension_dialog_t *dialog);
#define vlc_ext_dialog_update(a, b) vlc_ext_dialog_update(VLC_OBJECT(a), b)





typedef void (*vlc_dialog_ext_update_cb)(extension_dialog_t *p_ext_dialog,
void *p_data);








VLC_API void
vlc_dialog_provider_set_ext_callback(vlc_object_t *p_obj,
vlc_dialog_ext_update_cb pf_update,
void *p_data);
#define vlc_dialog_provider_set_ext_callback(a, b, c) vlc_dialog_provider_set_ext_callback(VLC_OBJECT(a), b, c)




#endif
