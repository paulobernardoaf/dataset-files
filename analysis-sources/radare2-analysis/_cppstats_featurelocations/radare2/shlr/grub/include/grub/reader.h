


















#if !defined(GRUB_READER_HEADER)
#define GRUB_READER_HEADER 1

#include <grub/err.h>

typedef grub_err_t (*grub_reader_getline_t) (char **, int, void *);

void grub_rescue_run (void);

#endif 
