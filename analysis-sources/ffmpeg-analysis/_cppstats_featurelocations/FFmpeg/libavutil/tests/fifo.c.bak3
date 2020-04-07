

















#include <stdio.h>
#include <stdlib.h>
#include "libavutil/fifo.h"

int main(void)
{

AVFifoBuffer *fifo = av_fifo_alloc(13 * sizeof(int));
int i, j, n, *p;


for (i = 0; av_fifo_space(fifo) >= sizeof(int); i++)
av_fifo_generic_write(fifo, &i, sizeof(int), NULL);


n = av_fifo_size(fifo) / sizeof(int);
for (i = -n + 1; i < n; i++) {
int *v = (int *)av_fifo_peek2(fifo, i * sizeof(int));
printf("%d: %d\n", i, *v);
}
printf("\n");


n = av_fifo_size(fifo) / sizeof(int);
for (i = 0; i < n; i++) {
av_fifo_generic_peek_at(fifo, &j, i * sizeof(int), sizeof(j), NULL);
printf("%d: %d\n", i, j);
}
printf("\n");



n = av_fifo_size(fifo);
p = malloc(n);
if (p == NULL) {
fprintf(stderr, "failed to allocate memory.\n");
exit(1);
}

(void) av_fifo_generic_peek(fifo, p, n, NULL);


n /= sizeof(int);
for(i = 0; i < n; ++i)
printf("%d: %d\n", i, p[i]);

putchar('\n');


for (i = 0; av_fifo_size(fifo) >= sizeof(int); i++) {
av_fifo_generic_read(fifo, &j, sizeof(int), NULL);
printf("%d ", j);
}
printf("\n");


av_fifo_reset(fifo);
fifo->rndx = fifo->wndx = ~(uint32_t)0 - 5;


for (i = 0; av_fifo_space(fifo) >= sizeof(int); i++)
av_fifo_generic_write(fifo, &i, sizeof(int), NULL);


n = av_fifo_size(fifo) / sizeof(int);
for (i = 0; i < n; i++) {
av_fifo_generic_peek_at(fifo, &j, i * sizeof(int), sizeof(j), NULL);
printf("%d: %d\n", i, j);
}
putchar('\n');


(void) av_fifo_grow(fifo, 15 * sizeof(int));


n = av_fifo_size(fifo) / sizeof(int);
for (i = n; av_fifo_space(fifo) >= sizeof(int); ++i)
av_fifo_generic_write(fifo, &i, sizeof(int), NULL);


n = av_fifo_size(fifo) / sizeof(int);
for (i = 0; i < n; i++) {
av_fifo_generic_peek_at(fifo, &j, i * sizeof(int), sizeof(j), NULL);
printf("%d: %d\n", i, j);
}

av_fifo_free(fifo);
free(p);

return 0;
}
