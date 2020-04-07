#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include "libavutil/bprint.h"
#include "libavutil/buffer.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_opencl.h"
#include "libavutil/pixfmt.h"
#include "avfilter.h"
typedef struct OpenCLFilterContext {
const AVClass *class;
AVBufferRef *device_ref;
AVHWDeviceContext *device;
AVOpenCLDeviceContext *hwctx;
cl_program program;
enum AVPixelFormat output_format;
int output_width;
int output_height;
} OpenCLFilterContext;
typedef struct OpenCLKernelArg {
size_t arg_size;
const void *arg_val;
} OpenCLKernelArg;
#define CL_SET_KERNEL_ARG(kernel, arg_num, type, arg) cle = clSetKernelArg(kernel, arg_num, sizeof(type), arg); if (cle != CL_SUCCESS) { av_log(avctx, AV_LOG_ERROR, "Failed to set kernel " "argument %d: error %d.\n", arg_num, cle); err = AVERROR(EIO); goto fail; }
#define CL_FAIL_ON_ERROR(errcode, ...) do { if (cle != CL_SUCCESS) { av_log(avctx, AV_LOG_ERROR, __VA_ARGS__); err = errcode; goto fail; } } while(0)
#define CL_CREATE_KERNEL(ctx, kernel_name) do { ctx->kernel_ ##kernel_name = clCreateKernel(ctx->ocf.program, #kernel_name, &cle); CL_FAIL_ON_ERROR(AVERROR(EIO), "Failed to create %s kernel: %d.\n", #kernel_name, cle); } while(0)
#define CL_RELEASE_KERNEL(k) do { if (k) { cle = clReleaseKernel(k); if (cle != CL_SUCCESS) av_log(avctx, AV_LOG_ERROR, "Failed to release " "OpenCL kernel: %d.\n", cle); } } while(0)
#define CL_RELEASE_MEMORY(m) do { if (m) { cle = clReleaseMemObject(m); if (cle != CL_SUCCESS) av_log(avctx, AV_LOG_ERROR, "Failed to release " "OpenCL memory: %d.\n", cle); } } while(0)
#define CL_RELEASE_QUEUE(q) do { if (q) { cle = clReleaseCommandQueue(q); if (cle != CL_SUCCESS) av_log(avctx, AV_LOG_ERROR, "Failed to release " "OpenCL command queue: %d.\n", cle); } } while(0)
#define CL_ENQUEUE_KERNEL_WITH_ARGS(queue, kernel, global_work_size, local_work_size, event, ...) do { OpenCLKernelArg args[] = {__VA_ARGS__}; for (int i = 0; i < FF_ARRAY_ELEMS(args); i++) { cle = clSetKernelArg(kernel, i, args[i].arg_size, args[i].arg_val); if (cle != CL_SUCCESS) { av_log(avctx, AV_LOG_ERROR, "Failed to set kernel " "argument %d: error %d.\n", i, cle); err = AVERROR(EIO); goto fail; } } cle = clEnqueueNDRangeKernel( queue, kernel, FF_ARRAY_ELEMS(global_work_size), NULL, global_work_size, local_work_size, 0, NULL, event ); CL_FAIL_ON_ERROR(AVERROR(EIO), "Failed to enqueue kernel: %d.\n", cle); } while (0)
#define CL_RUN_KERNEL_WITH_ARGS(queue, kernel, global_work_size, local_work_size, event, ...) do { CL_ENQUEUE_KERNEL_WITH_ARGS( queue, kernel, global_work_size, local_work_size, event, __VA_ARGS__ ); cle = clFinish(queue); CL_FAIL_ON_ERROR(AVERROR(EIO), "Failed to finish command queue: %d.\n", cle); } while (0)
#define CL_CREATE_BUFFER_FLAGS(ctx, buffer_name, flags, size, host_ptr) do { ctx->buffer_name = clCreateBuffer( ctx->ocf.hwctx->context, flags, size, host_ptr, &cle ); CL_FAIL_ON_ERROR(AVERROR(EIO), "Failed to create buffer %s: %d.\n", #buffer_name, cle); } while(0)
#define CL_BLOCKING_WRITE_BUFFER(queue, buffer, size, host_ptr, event) do { cle = clEnqueueWriteBuffer( queue, buffer, CL_TRUE, 0, size, host_ptr, 0, NULL, event ); CL_FAIL_ON_ERROR(AVERROR(EIO), "Failed to write buffer to device: %d.\n", cle); } while(0)
#define CL_CREATE_BUFFER(ctx, buffer_name, size) CL_CREATE_BUFFER_FLAGS(ctx, buffer_name, 0, size, NULL)
int ff_opencl_filter_query_formats(AVFilterContext *avctx);
int ff_opencl_filter_config_input(AVFilterLink *inlink);
int ff_opencl_filter_config_output(AVFilterLink *outlink);
int ff_opencl_filter_init(AVFilterContext *avctx);
void ff_opencl_filter_uninit(AVFilterContext *avctx);
int ff_opencl_filter_load_program(AVFilterContext *avctx,
const char **program_source_array,
int nb_strings);
int ff_opencl_filter_load_program_from_file(AVFilterContext *avctx,
const char *filename);
int ff_opencl_filter_work_size_from_image(AVFilterContext *avctx,
size_t *work_size,
AVFrame *frame, int plane,
int block_alignment);
void ff_opencl_print_const_matrix_3x3(AVBPrint *buf, const char *name_str,
double mat[3][3]);
cl_ulong ff_opencl_get_event_time(cl_event event);
