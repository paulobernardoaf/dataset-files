#include "../dnn_interface.h"
#include "dnn_backend_native.h"
#include "dnn_backend_tf.h"
#include "libavutil/mem.h"
DNNModule *ff_get_dnn_module(DNNBackendType backend_type)
{
DNNModule *dnn_module;
dnn_module = av_malloc(sizeof(DNNModule));
if(!dnn_module){
return NULL;
}
switch(backend_type){
case DNN_NATIVE:
dnn_module->load_model = &ff_dnn_load_model_native;
dnn_module->execute_model = &ff_dnn_execute_model_native;
dnn_module->free_model = &ff_dnn_free_model_native;
break;
case DNN_TF:
#if (CONFIG_LIBTENSORFLOW == 1)
dnn_module->load_model = &ff_dnn_load_model_tf;
dnn_module->execute_model = &ff_dnn_execute_model_tf;
dnn_module->free_model = &ff_dnn_free_model_tf;
#else
av_freep(&dnn_module);
return NULL;
#endif
break;
default:
av_log(NULL, AV_LOG_ERROR, "Module backend_type is not native or tensorflow\n");
av_freep(&dnn_module);
return NULL;
}
return dnn_module;
}
