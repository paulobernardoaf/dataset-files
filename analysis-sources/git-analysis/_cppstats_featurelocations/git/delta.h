#if !defined(DELTA_H)
#define DELTA_H


struct delta_index;










struct delta_index *
create_delta_index(const void *buf, unsigned long bufsize);






void free_delta_index(struct delta_index *index);






unsigned long sizeof_delta_index(struct delta_index *index);











void *
create_delta(const struct delta_index *index,
const void *buf, unsigned long bufsize,
unsigned long *delta_size, unsigned long max_delta_size);









static inline void *
diff_delta(const void *src_buf, unsigned long src_bufsize,
const void *trg_buf, unsigned long trg_bufsize,
unsigned long *delta_size, unsigned long max_delta_size)
{
struct delta_index *index = create_delta_index(src_buf, src_bufsize);
if (index) {
void *delta = create_delta(index, trg_buf, trg_bufsize,
delta_size, max_delta_size);
free_delta_index(index);
return delta;
}
return NULL;
}








void *patch_delta(const void *src_buf, unsigned long src_size,
const void *delta_buf, unsigned long delta_size,
unsigned long *dst_size);


#define DELTA_SIZE_MIN 4





static inline unsigned long get_delta_hdr_size(const unsigned char **datap,
const unsigned char *top)
{
const unsigned char *data = *datap;
unsigned long cmd, size = 0;
int i = 0;
do {
cmd = *data++;
size |= (cmd & 0x7f) << i;
i += 7;
} while (cmd & 0x80 && data < top);
*datap = data;
return size;
}

#endif
