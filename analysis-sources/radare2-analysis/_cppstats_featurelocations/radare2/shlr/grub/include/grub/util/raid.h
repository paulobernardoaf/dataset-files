


















#if !defined(GRUB_RAID_UTIL_HEADER)
#define GRUB_RAID_UTIL_HEADER 1

#if defined(__linux__)
char** grub_util_raid_getmembers (char *name);
#endif

#endif 
