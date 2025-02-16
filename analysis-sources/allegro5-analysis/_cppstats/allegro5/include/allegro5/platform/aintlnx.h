#if defined(__cplusplus)
extern "C" {
#endif
extern _AL_DRIVER_INFO _al_linux_keyboard_driver_list[];
extern _AL_DRIVER_INFO _al_linux_mouse_driver_list[];
#if defined(ALLEGRO_RASPBERRYPI)
#define AL_MOUSEDRV_LINUX_EVDEV AL_ID('E', 'V', 'D', 'V')
#endif
struct MAPPED_MEMORY {
unsigned int base, size; 
int perms; 
void *data; 
};
extern int __al_linux_have_ioperms;
int __al_linux_init_memory (void);
int __al_linux_shutdown_memory (void);
int __al_linux_map_memory (struct MAPPED_MEMORY *info);
int __al_linux_unmap_memory (struct MAPPED_MEMORY *info);
extern int __al_linux_vt;
extern int __al_linux_console_fd;
extern int __al_linux_prev_vt;
extern int __al_linux_got_text_message;
extern struct termios __al_linux_startup_termio;
extern struct termios __al_linux_work_termio;
int __al_linux_use_console (void);
int __al_linux_leave_console (void);
int __al_linux_console_graphics (void);
int __al_linux_console_text (void);
int __al_linux_wait_for_display (void);
#define SIGRELVT SIGUSR1
#define SIGACQVT SIGUSR2
int __al_linux_init_vtswitch (void);
int __al_linux_done_vtswitch (void);
int __al_linux_set_display_switch_mode (int mode);
void __al_linux_display_switch_lock (int lock, int foreground);
extern volatile int __al_linux_switching_blocked;
#if defined(__cplusplus)
}
#endif
