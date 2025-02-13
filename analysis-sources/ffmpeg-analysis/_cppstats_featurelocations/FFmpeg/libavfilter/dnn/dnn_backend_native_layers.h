



















#if !defined(AVFILTER_DNN_DNN_BACKEND_NATIVE_LAYERS_H)
#define AVFILTER_DNN_DNN_BACKEND_NATIVE_LAYERS_H

#include <stdint.h>
#include "dnn_backend_native.h"

typedef int (*LAYER_EXEC_FUNC)(DnnOperand *operands, const int32_t *input_operand_indexes,
int32_t output_operand_index, const void *parameters);
typedef int (*LAYER_LOAD_FUNC)(Layer *layer, AVIOContext *model_file_context, int file_size);

typedef struct LayerFunc {
LAYER_EXEC_FUNC pf_exec;
LAYER_LOAD_FUNC pf_load;
}LayerFunc;

extern LayerFunc layer_funcs[DLT_COUNT];

#endif
