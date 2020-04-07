


















#if !defined(GRUB_NORMAL_HEADER)
#define GRUB_NORMAL_HEADER 1

extern int grub_normal_exit_level;


void read_handler_list (void);
void free_handler_list (void);


void read_command_list (const char *prefix);


void read_fs_list (const char *prefix);

void grub_context_init (void);
void grub_context_fini (void);

void read_crypto_list (const char *prefix);

void read_terminal_list (const char *prefix);

void grub_set_more (int onoff);

int grub_normal_get_line_counter (void);
void grub_install_newline_hook (void);

#endif 
