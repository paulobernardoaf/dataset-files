#if defined(__cplusplus)
extern "C" {
#endif
void _al_init_timers(void);
int _al_get_active_timers_count(void);
double _al_timer_thread_handle_tick(double interval);
#if defined(__cplusplus)
}
#endif
