#include "../dnn_interface.h"
DNNModel *ff_dnn_load_model_tf(const char *model_filename);
DNNReturnType ff_dnn_execute_model_tf(const DNNModel *model, DNNData *outputs, uint32_t nb_output);
void ff_dnn_free_model_tf(DNNModel **model);
