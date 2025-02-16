#include "builtin.h"
#include "transport.h"

static const char usage_msg[] =
"git remote-fd <remote> <url>";
















#define MAXCOMMAND 4096

static void command_loop(int input_fd, int output_fd)
{
char buffer[MAXCOMMAND];

while (1) {
size_t i;
if (!fgets(buffer, MAXCOMMAND - 1, stdin)) {
if (ferror(stdin))
die("Input error");
return;
}

i = strlen(buffer);
while (i > 0 && isspace(buffer[i - 1]))
buffer[--i] = 0;

if (!strcmp(buffer, "capabilities")) {
printf("*connect\n\n");
fflush(stdout);
} else if (!strncmp(buffer, "connect ", 8)) {
printf("\n");
fflush(stdout);
if (bidirectional_transfer_loop(input_fd,
output_fd))
die("Copying data between file descriptors failed");
return;
} else {
die("Bad command: %s", buffer);
}
}
}

int cmd_remote_fd(int argc, const char **argv, const char *prefix)
{
int input_fd = -1;
int output_fd = -1;
char *end;

if (argc != 3)
usage(usage_msg);

input_fd = (int)strtoul(argv[2], &end, 10);

if ((end == argv[2]) || (*end != ',' && *end != '/' && *end))
die("Bad URL syntax");

if (*end == '/' || !*end) {
output_fd = input_fd;
} else {
char *end2;
output_fd = (int)strtoul(end + 1, &end2, 10);

if ((end2 == end + 1) || (*end2 != '/' && *end2))
die("Bad URL syntax");
}

command_loop(input_fd, output_fd);
return 0;
}
