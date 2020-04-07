#include "../dnn_interface.h"
#include "libavformat/avio.h"
typedef struct DepthToSpaceParams{
int block_size;
} DepthToSpaceParams;
int dnn_load_layer_depth2space(Layer *layer, AVIOContext *model_file_context, int file_size);
int dnn_execute_layer_depth2space(DnnOperand *operands, const int32_t *input_operand_indexes,
int32_t output_operand_index, const void *parameters);
