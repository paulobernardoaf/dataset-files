

















#if !defined(COMPAT_CUDA_DYNLINK_LOADER_H)
#define COMPAT_CUDA_DYNLINK_LOADER_H

#include "libavutil/log.h"
#include "compat/w32dlfcn.h"

#define FFNV_LOAD_FUNC(path) dlopen((path), RTLD_LAZY)
#define FFNV_SYM_FUNC(lib, sym) dlsym((lib), (sym))
#define FFNV_FREE_FUNC(lib) dlclose(lib)
#define FFNV_LOG_FUNC(logctx, msg, ...) av_log(logctx, AV_LOG_ERROR, msg, __VA_ARGS__)
#define FFNV_DEBUG_LOG_FUNC(logctx, msg, ...) av_log(logctx, AV_LOG_DEBUG, msg, __VA_ARGS__)

#include <ffnvcodec/dynlink_loader.h>

#endif 
