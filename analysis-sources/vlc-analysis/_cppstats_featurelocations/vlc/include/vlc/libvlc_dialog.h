



















#if !defined(LIBVLC_DIALOG_H)
#define LIBVLC_DIALOG_H 1

#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct libvlc_dialog_id libvlc_dialog_id;









typedef enum libvlc_dialog_question_type
{
LIBVLC_DIALOG_QUESTION_NORMAL,
LIBVLC_DIALOG_QUESTION_WARNING,
LIBVLC_DIALOG_QUESTION_CRITICAL,
} libvlc_dialog_question_type;




typedef struct libvlc_dialog_cbs
{







void (*pf_display_error)(void *p_data, const char *psz_title,
const char *psz_text);


















void (*pf_display_login)(void *p_data, libvlc_dialog_id *p_id,
const char *psz_title, const char *psz_text,
const char *psz_default_username,
bool b_ask_store);





















void (*pf_display_question)(void *p_data, libvlc_dialog_id *p_id,
const char *psz_title, const char *psz_text,
libvlc_dialog_question_type i_type,
const char *psz_cancel, const char *psz_action1,
const char *psz_action2);




















void (*pf_display_progress)(void *p_data, libvlc_dialog_id *p_id,
const char *psz_title, const char *psz_text,
bool b_indeterminate, float f_position,
const char *psz_cancel);










void (*pf_cancel)(void *p_data, libvlc_dialog_id *p_id);









void (*pf_update_progress)(void *p_data, libvlc_dialog_id *p_id,
float f_position, const char *psz_text);
} libvlc_dialog_cbs;









LIBVLC_API void
libvlc_dialog_set_callbacks(libvlc_instance_t *p_instance,
const libvlc_dialog_cbs *p_cbs, void *p_data);






LIBVLC_API void
libvlc_dialog_set_context(libvlc_dialog_id *p_id, void *p_context);






LIBVLC_API void *
libvlc_dialog_get_context(libvlc_dialog_id *p_id);
















LIBVLC_API int
libvlc_dialog_post_login(libvlc_dialog_id *p_id, const char *psz_username,
const char *psz_password, bool b_store);














LIBVLC_API int
libvlc_dialog_post_action(libvlc_dialog_id *p_id, int i_action);













LIBVLC_API int
libvlc_dialog_dismiss(libvlc_dialog_id *p_id);



#if defined(__cplusplus)
}
#endif

#endif 
