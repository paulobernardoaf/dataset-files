#include <stdint.h>
typedef enum {DNN_SUCCESS, DNN_ERROR} DNNReturnType;
typedef enum {DNN_NATIVE, DNN_TF} DNNBackendType;
typedef enum {DNN_FLOAT = 1, DNN_UINT8 = 4} DNNDataType;
typedef struct DNNData{
void *data;
DNNDataType dt;
int width, height, channels;
} DNNData;
typedef struct DNNModel{
void *model;
DNNReturnType (*get_input)(void *model, DNNData *input, const char *input_name);
DNNReturnType (*set_input_output)(void *model, DNNData *input, const char *input_name, const char **output_names, uint32_t nb_output);
} DNNModel;
typedef struct DNNModule{
DNNModel *(*load_model)(const char *model_filename);
DNNReturnType (*execute_model)(const DNNModel *model, DNNData *outputs, uint32_t nb_output);
void (*free_model)(DNNModel **model);
} DNNModule;
DNNModule *ff_get_dnn_module(DNNBackendType backend_type);
