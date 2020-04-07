#include "get_bits.h"
#define CODE_UNSET -1
#define CODE_BIT_INIT 9
#define DIC_INDEX_INIT 512 
#define DIC_INDEX_MAX 32768 
#define FLUSH_CODE 256
#define FREEZE_CODE 257
#define FIRST_CODE 258
#define MAX_CODE 32767
#define TABLE_SIZE 35023 
typedef struct MLZDict {
int string_code;
int parent_code;
int char_code;
int match_len;
} MLZDict;
typedef struct MLZ {
int dic_code_bit;
int current_dic_index_max;
unsigned int bump_code;
unsigned int flush_code;
int next_code;
int freeze_flag;
MLZDict* dict;
void* context;
} MLZ;
void ff_mlz_init_dict(void* context, MLZ *mlz);
void ff_mlz_flush_dict(MLZ *dict);
int ff_mlz_decompression(MLZ* mlz, GetBitContext* gb, int size, unsigned char *buff);
