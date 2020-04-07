#include "dnn_backend_native.h"
typedef enum {RELU, TANH, SIGMOID, NONE, LEAKY_RELU} DNNActivationFunc;
typedef enum {VALID, SAME, SAME_CLAMP_TO_EDGE} DNNConvPaddingParam;
typedef struct ConvolutionalParams{
int32_t input_num, output_num, kernel_size;
DNNActivationFunc activation;
DNNConvPaddingParam padding_method;
int32_t dilation;
int32_t has_bias;
float *kernel;
float *biases;
} ConvolutionalParams;
int dnn_load_layer_conv2d(Layer *layer, AVIOContext *model_file_context, int file_size);
int dnn_execute_layer_conv2d(DnnOperand *operands, const int32_t *input_operand_indexes,
int32_t output_operand_index, const void *parameters);
