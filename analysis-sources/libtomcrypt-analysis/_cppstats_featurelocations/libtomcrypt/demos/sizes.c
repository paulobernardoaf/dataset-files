








#include "tomcrypt.h"

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L
#include <libgen.h>
#else
#define basename(x) x
#endif







static void _print_line(const char* cmd, const char* desc)
{
printf(" %-16s - %s\n", cmd, desc);
}

int main(int argc, char **argv)
{
if (argc == 1) {

char name[] = "ltc_hash_descriptor";
unsigned int size;
char *sizes_list;
unsigned int sizes_list_len;
if (crypt_get_size(name, &size) != 0) exit(EXIT_FAILURE);
printf("\n size of '%s' is %u \n\n", name, size);


if (crypt_list_all_sizes(NULL, &sizes_list_len) != 0) exit(EXIT_FAILURE);
printf(" need to allocate %u bytes \n\n", sizes_list_len);


if ((sizes_list = malloc(sizes_list_len)) == NULL) exit(EXIT_FAILURE);
if (crypt_list_all_sizes(sizes_list, &sizes_list_len) != 0) exit(EXIT_FAILURE);
printf(" supported sizes:\n\n%s\n\n", sizes_list);
free(sizes_list);
} else if (argc == 2) {
if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
char* base = strdup(basename(argv[0]));
printf("Usage: %s [-a] [-s name]\n\n", base);
_print_line("<no argument>", "The old behavior of the demo");
_print_line("-a", "Only lists all sizes");
_print_line("-s name", "List a single size given as argument");
_print_line("-h", "The help you're looking at");
free(base);
} else if (strcmp(argv[1], "-a") == 0) {
char *sizes_list;
unsigned int sizes_list_len;

if (crypt_list_all_sizes(NULL, &sizes_list_len) != 0) exit(EXIT_FAILURE);

if ((sizes_list = malloc(sizes_list_len)) == NULL) exit(EXIT_FAILURE);
if (crypt_list_all_sizes(sizes_list, &sizes_list_len) != 0) exit(EXIT_FAILURE);
printf("%s\n", sizes_list);
free(sizes_list);
}
} else if (argc == 3) {
if (strcmp(argv[1], "-s") == 0) {
unsigned int size;
if (crypt_get_size(argv[2], &size) != 0) exit(EXIT_FAILURE);
printf("%s,%u\n", argv[2], size);
}
}
return 0;
}




