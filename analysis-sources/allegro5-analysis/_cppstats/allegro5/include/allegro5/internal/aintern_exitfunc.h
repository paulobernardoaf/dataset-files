#if defined(__cplusplus)
extern "C" {
#endif
AL_FUNC(void, _al_add_exit_func, (AL_METHOD(void, func, (void)), const char *desc));
AL_FUNC(void, _al_remove_exit_func, (AL_METHOD(void, func, (void))));
AL_FUNC(void, _al_run_exit_funcs, (void));
#if defined(__cplusplus)
}
#endif
