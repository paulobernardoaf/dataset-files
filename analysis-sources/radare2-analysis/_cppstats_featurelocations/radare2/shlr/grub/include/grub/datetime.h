

















#if !defined(KERNEL_DATETIME_HEADER)
#define KERNEL_DATETIME_HEADER 1

#include <grub/types.h>
#include <grub/err.h>

struct grub_datetime
{
grub_uint16_t year;
grub_uint8_t month;
grub_uint8_t day;
grub_uint8_t hour;
grub_uint8_t minute;
grub_uint8_t second;
};


#if defined(GRUB_MACHINE_EMU)
grub_err_t EXPORT_FUNC(grub_get_datetime) (struct grub_datetime *datetime);


grub_err_t EXPORT_FUNC(grub_set_datetime) (struct grub_datetime *datetime);
#else
grub_err_t grub_get_datetime (struct grub_datetime *datetime);


grub_err_t grub_set_datetime (struct grub_datetime *datetime);
#endif

int grub_get_weekday (struct grub_datetime *datetime);
char *grub_get_weekday_name (struct grub_datetime *datetime);

void grub_unixtime2datetime (grub_int32_t nix,
struct grub_datetime *datetime);


#endif 
