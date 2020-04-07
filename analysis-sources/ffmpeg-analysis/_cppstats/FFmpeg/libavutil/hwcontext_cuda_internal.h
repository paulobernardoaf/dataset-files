#include "compat/cuda/dynlink_loader.h"
#include "hwcontext_cuda.h"
struct AVCUDADeviceContextInternal {
CudaFunctions *cuda_dl;
int is_allocated;
CUdevice cuda_device;
int flags;
};
