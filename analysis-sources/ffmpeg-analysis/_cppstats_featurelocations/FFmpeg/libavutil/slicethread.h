

















#if !defined(AVUTIL_SLICETHREAD_H)
#define AVUTIL_SLICETHREAD_H

typedef struct AVSliceThread AVSliceThread;










int avpriv_slicethread_create(AVSliceThread **pctx, void *priv,
void (*worker_func)(void *priv, int jobnr, int threadnr, int nb_jobs, int nb_threads),
void (*main_func)(void *priv),
int nb_threads);







void avpriv_slicethread_execute(AVSliceThread *ctx, int nb_jobs, int execute_main);





void avpriv_slicethread_free(AVSliceThread **pctx);

#endif
