#include "libavutil/file.c"
#undef printf
int main(void)
{
uint8_t *buf;
size_t size;
if (av_file_map("file.c", &buf, &size, 0, NULL) < 0)
return 1;
buf[0] = 's';
printf("%s", buf);
av_file_unmap(buf, size);
return 0;
}
