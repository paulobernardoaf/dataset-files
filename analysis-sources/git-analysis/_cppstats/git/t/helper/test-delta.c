#include "test-tool.h"
#include "git-compat-util.h"
#include "delta.h"
#include "cache.h"
static const char usage_str[] =
"test-tool delta (-d|-p) <from_file> <data_file> <out_file>";
int cmd__delta(int argc, const char **argv)
{
int fd;
struct stat st;
void *from_buf, *data_buf, *out_buf;
unsigned long from_size, data_size, out_size;
if (argc != 5 || (strcmp(argv[1], "-d") && strcmp(argv[1], "-p"))) {
fprintf(stderr, "usage: %s\n", usage_str);
return 1;
}
fd = open(argv[2], O_RDONLY);
if (fd < 0 || fstat(fd, &st)) {
perror(argv[2]);
return 1;
}
from_size = st.st_size;
from_buf = xmalloc(from_size);
if (read_in_full(fd, from_buf, from_size) < 0) {
perror(argv[2]);
close(fd);
return 1;
}
close(fd);
fd = open(argv[3], O_RDONLY);
if (fd < 0 || fstat(fd, &st)) {
perror(argv[3]);
return 1;
}
data_size = st.st_size;
data_buf = xmalloc(data_size);
if (read_in_full(fd, data_buf, data_size) < 0) {
perror(argv[3]);
close(fd);
return 1;
}
close(fd);
if (argv[1][1] == 'd')
out_buf = diff_delta(from_buf, from_size,
data_buf, data_size,
&out_size, 0);
else
out_buf = patch_delta(from_buf, from_size,
data_buf, data_size,
&out_size);
if (!out_buf) {
fprintf(stderr, "delta operation failed (returned NULL)\n");
return 1;
}
fd = open (argv[4], O_WRONLY|O_CREAT|O_TRUNC, 0666);
if (fd < 0 || write_in_full(fd, out_buf, out_size) < 0) {
perror(argv[4]);
return 1;
}
return 0;
}
