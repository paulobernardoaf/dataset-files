#include "dnn_backend_native.h"
#include "libavutil/avassert.h"
#include "dnn_backend_native_layer_maximum.h"
int dnn_load_layer_maximum(Layer *layer, AVIOContext *model_file_context, int file_size)
{
DnnLayerMaximumParams *params;
int dnn_size = 0;
params = av_malloc(sizeof(*params));
if (!params)
return 0;
params->val.u32 = avio_rl32(model_file_context);
dnn_size += 4;
layer->params = params;
layer->input_operand_indexes[0] = (int32_t)avio_rl32(model_file_context);
layer->output_operand_index = (int32_t)avio_rl32(model_file_context);
dnn_size += 8;
return dnn_size;
}
int dnn_execute_layer_maximum(DnnOperand *operands, const int32_t *input_operand_indexes,
int32_t output_operand_index, const void *parameters)
{
const DnnOperand *input = &operands[input_operand_indexes[0]];
DnnOperand *output = &operands[output_operand_index];
const DnnLayerMaximumParams *params = (const DnnLayerMaximumParams *)parameters;
int dims_count;
const float *src;
float *dst;
for (int i = 0; i < 4; ++i)
output->dims[i] = input->dims[i];
output->data_type = input->data_type;
output->length = calculate_operand_data_length(output);
output->data = av_realloc(output->data, output->length);
if (!output->data)
return DNN_ERROR;
dims_count = calculate_operand_dims_count(output);
src = input->data;
dst = output->data;
for (int i = 0; i < dims_count; ++i)
dst[i] = FFMAX(src[i], params->val.y);
return 0;
}
