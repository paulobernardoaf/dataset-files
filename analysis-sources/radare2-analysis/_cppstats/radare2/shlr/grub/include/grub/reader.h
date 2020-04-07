#include <grub/err.h>
typedef grub_err_t (*grub_reader_getline_t) (char **, int, void *);
void grub_rescue_run (void);
