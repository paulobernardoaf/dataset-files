























#if !defined(AVFILTER_DNN_DNN_BACKEND_NATIVE_LAYER_PAD_H)
#define AVFILTER_DNN_DNN_BACKEND_NATIVE_LAYER_PAD_H

#include <stdint.h>
#include "dnn_backend_native.h"

typedef enum {LPMP_CONSTANT, LPMP_REFLECT, LPMP_SYMMETRIC} LayerPadModeParam;

typedef struct LayerPadParams{
int32_t paddings[4][2];
LayerPadModeParam mode;
float constant_values;
} LayerPadParams;

int dnn_load_layer_pad(Layer *layer, AVIOContext *model_file_context, int file_size);
int dnn_execute_layer_pad(DnnOperand *operands, const int32_t *input_operand_indexes,
int32_t output_operand_index, const void *parameters);

#endif
