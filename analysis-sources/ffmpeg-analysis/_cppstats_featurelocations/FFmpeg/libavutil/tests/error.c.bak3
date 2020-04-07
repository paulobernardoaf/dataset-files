

















#include "libavutil/error.c"

#undef printf

int main(void)
{
int i;

for (i = 0; i < FF_ARRAY_ELEMS(error_entries); i++) {
const struct error_entry *entry = &error_entries[i];
printf("%d: %s [%s]\n", entry->num, av_err2str(entry->num), entry->tag);
}

for (i = 0; i < 256; i++) {
printf("%d: %s\n", -i, av_err2str(-i));
}

return 0;
}
