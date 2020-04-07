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
char name[] = "CTR_COUNTER_BIG_ENDIAN";
int value;
char *names_list;
unsigned int names_list_len;
if (crypt_get_constant(name, &value) != 0) exit(EXIT_FAILURE);
printf("\n %s is %d \n\n", name, value);
if (crypt_list_all_constants(NULL, &names_list_len) != 0) exit(EXIT_FAILURE);
printf(" need to allocate %u bytes \n\n", names_list_len);
if ((names_list = malloc(names_list_len)) == NULL) exit(EXIT_FAILURE);
if (crypt_list_all_constants(names_list, &names_list_len) != 0) exit(EXIT_FAILURE);
printf(" supported constants:\n\n%s\n\n", names_list);
free(names_list);
} else if (argc == 2) {
if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
char* base = strdup(basename(argv[0]));
printf("Usage: %s [-a] [-s name]\n\n", base);
_print_line("<no argument>", "The old behavior of the demo");
_print_line("-a", "Only lists all constants");
_print_line("-s name", "List a single constant given as argument");
_print_line("-h", "The help you're looking at");
free(base);
} else if (strcmp(argv[1], "-a") == 0) {
char *names_list;
unsigned int names_list_len;
if (crypt_list_all_constants(NULL, &names_list_len) != 0) exit(EXIT_FAILURE);
if ((names_list = malloc(names_list_len)) == NULL) exit(EXIT_FAILURE);
if (crypt_list_all_constants(names_list, &names_list_len) != 0) exit(EXIT_FAILURE);
printf("%s\n", names_list);
free(names_list);
}
} else if (argc == 3) {
if (strcmp(argv[1], "-s") == 0) {
int value;
if (crypt_get_constant(argv[2], &value) != 0) exit(EXIT_FAILURE);
printf("%s,%u\n", argv[2], value);
}
}
return 0;
}
