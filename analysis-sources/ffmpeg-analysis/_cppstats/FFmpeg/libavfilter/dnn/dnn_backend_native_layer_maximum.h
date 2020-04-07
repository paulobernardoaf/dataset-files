#include "libavformat/avio.h"
#include "dnn_backend_native.h"
typedef struct DnnLayerMaximumParams{
union {
uint32_t u32;
float y;
}val;
} DnnLayerMaximumParams;
int dnn_load_layer_maximum(Layer *layer, AVIOContext *model_file_context, int file_size);
int dnn_execute_layer_maximum(DnnOperand *operands, const int32_t *input_operand_indexes,
int32_t output_operand_index, const void *parameters);
