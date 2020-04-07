#include "../dnn_interface.h"
#include "libavformat/avio.h"
typedef enum {
DLT_INPUT = 0,
DLT_CONV2D = 1,
DLT_DEPTH_TO_SPACE = 2,
DLT_MIRROR_PAD = 3,
DLT_MAXIMUM = 4,
DLT_COUNT
} DNNLayerType;
typedef enum {DOT_INPUT = 1, DOT_OUTPUT = 2, DOT_INTERMEDIATE = DOT_INPUT | DOT_INPUT} DNNOperandType;
typedef struct Layer{
DNNLayerType type;
int32_t input_operand_indexes[4];
int32_t output_operand_index;
void *params;
} Layer;
typedef struct DnnOperand{
int32_t dims[4];
DNNOperandType type;
DNNDataType data_type;
int8_t isNHWC;
char name[128];
void *data;
int32_t length;
int32_t usedNumbersLeft;
}DnnOperand;
typedef struct InputParams{
int height, width, channels;
} InputParams;
typedef struct ConvolutionalNetwork{
Layer *layers;
int32_t layers_num;
DnnOperand *operands;
int32_t operands_num;
int32_t *output_indexes;
uint32_t nb_output;
} ConvolutionalNetwork;
DNNModel *ff_dnn_load_model_native(const char *model_filename);
DNNReturnType ff_dnn_execute_model_native(const DNNModel *model, DNNData *outputs, uint32_t nb_output);
void ff_dnn_free_model_native(DNNModel **model);
int32_t calculate_operand_data_length(const DnnOperand *oprd);
int32_t calculate_operand_dims_count(const DnnOperand *oprd);
