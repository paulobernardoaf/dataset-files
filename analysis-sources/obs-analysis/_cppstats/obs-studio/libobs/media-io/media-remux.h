#include "../util/c99defs.h"
#pragma once
struct media_remux_job;
typedef struct media_remux_job *media_remux_job_t;
typedef bool(media_remux_progress_callback)(void *data, float percent);
#if defined(__cplusplus)
extern "C" {
#endif
EXPORT bool media_remux_job_create(media_remux_job_t *job,
const char *in_filename,
const char *out_filename);
EXPORT bool media_remux_job_process(media_remux_job_t job,
media_remux_progress_callback callback,
void *data);
EXPORT void media_remux_job_destroy(media_remux_job_t job);
#if defined(__cplusplus)
}
#endif
