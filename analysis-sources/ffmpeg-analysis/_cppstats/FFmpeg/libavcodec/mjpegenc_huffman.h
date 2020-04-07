#include <stdint.h>
typedef struct MJpegEncHuffmanContext {
int val_count[256];
} MJpegEncHuffmanContext;
void ff_mjpeg_encode_huffman_init(MJpegEncHuffmanContext *s);
static inline void ff_mjpeg_encode_huffman_increment(MJpegEncHuffmanContext *s,
uint8_t val)
{
s->val_count[val]++;
}
void ff_mjpeg_encode_huffman_close(MJpegEncHuffmanContext *s,
uint8_t bits[17], uint8_t val[],
int max_nval);
typedef struct PTable {
int value; 
int prob; 
} PTable;
typedef struct PackageMergerList {
int nitems; 
int item_idx[515]; 
int probability[514]; 
int items[257 * 16]; 
} PackageMergerList;
typedef struct HuffTable {
int code; 
int length; 
} HuffTable;
void ff_mjpegenc_huffman_compute_bits(PTable *prob_table, HuffTable *distincts,
int size, int max_length);
