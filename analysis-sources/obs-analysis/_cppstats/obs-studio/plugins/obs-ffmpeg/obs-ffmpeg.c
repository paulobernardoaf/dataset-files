#include <obs-module.h>
#include <util/platform.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "obs-ffmpeg-config.h"
#if defined(_WIN32)
#include <dxgi.h>
#include <util/dstr.h>
#include <util/windows/win-version.h>
#endif
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-ffmpeg", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
return "FFmpeg based sources/outputs/encoders";
}
extern struct obs_source_info ffmpeg_source;
extern struct obs_output_info ffmpeg_output;
extern struct obs_output_info ffmpeg_muxer;
extern struct obs_output_info replay_buffer;
extern struct obs_encoder_info aac_encoder_info;
extern struct obs_encoder_info opus_encoder_info;
extern struct obs_encoder_info nvenc_encoder_info;
extern struct obs_output_info ffmpeg_encoded_output_info;
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(55, 27, 100)
#define LIBAVUTIL_VAAPI_AVAILABLE
#endif
#if defined(LIBAVUTIL_VAAPI_AVAILABLE)
extern struct obs_encoder_info vaapi_encoder_info;
#endif
#if !defined(__APPLE__)
static const char *nvenc_check_name = "nvenc_check";
#if defined(_WIN32)
static const int blacklisted_adapters[] = {
0x1298, 
0x1140, 
0x1293, 
0x1290, 
0x0fe1, 
0x0fdf, 
0x1294, 
0x1292, 
0x0fe2, 
0x0fe3, 
0x1140, 
0x0fed, 
0x1340, 
0x1393, 
0x1341, 
0x1398, 
0x1390, 
0x1344, 
0x1299, 
0x134f, 
0x134e, 
0x1349, 
0x1346, 
0x179c, 
0x139c, 
0x1347, 
0x134d, 
0x134b, 
0x1399, 
0x1348, 
0x1d01, 
0x0fc5, 
0x174e, 
0x174d, 
0x1d10, 
0x1d12, 
0x1d11, 
0x1d13, 
0x1d52, 
0x137b, 
0x1d33, 
0x137a, 
};
static const size_t num_blacklisted =
sizeof(blacklisted_adapters) / sizeof(blacklisted_adapters[0]);
static bool is_blacklisted(const int device_id)
{
for (size_t i = 0; i < num_blacklisted; i++) {
const int blacklisted_adapter = blacklisted_adapters[i];
if (device_id == blacklisted_adapter) {
return true;
}
}
return false;
}
typedef HRESULT(WINAPI *create_dxgi_proc)(const IID *, IDXGIFactory1 **);
static bool nvenc_device_available(void)
{
static HMODULE dxgi = NULL;
static create_dxgi_proc create = NULL;
IDXGIFactory1 *factory;
IDXGIAdapter1 *adapter;
bool available = false;
HRESULT hr;
UINT i = 0;
if (!dxgi) {
dxgi = GetModuleHandleW(L"dxgi");
if (!dxgi) {
dxgi = LoadLibraryW(L"dxgi");
if (!dxgi) {
return true;
}
}
}
if (!create) {
create = (create_dxgi_proc)GetProcAddress(dxgi,
"CreateDXGIFactory1");
if (!create) {
return true;
}
}
hr = create(&IID_IDXGIFactory1, &factory);
if (FAILED(hr)) {
return true;
}
while (factory->lpVtbl->EnumAdapters1(factory, i++, &adapter) == S_OK) {
DXGI_ADAPTER_DESC desc;
hr = adapter->lpVtbl->GetDesc(adapter, &desc);
adapter->lpVtbl->Release(adapter);
if (FAILED(hr)) {
continue;
}
if (desc.VendorId == 0x10de && !is_blacklisted(desc.DeviceId)) {
available = true;
goto finish;
}
}
finish:
factory->lpVtbl->Release(factory);
return available;
}
#endif
#if defined(_WIN32)
extern bool load_nvenc_lib(void);
#endif
static bool nvenc_supported(void)
{
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
av_register_all();
#endif
profile_start(nvenc_check_name);
AVCodec *nvenc = avcodec_find_encoder_by_name("nvenc_h264");
void *lib = NULL;
bool success = false;
if (!nvenc) {
goto cleanup;
}
#if defined(_WIN32)
if (!nvenc_device_available()) {
goto cleanup;
}
if (load_nvenc_lib()) {
success = true;
goto finish;
}
#else
lib = os_dlopen("libnvidia-encode.so.1");
#endif
success = !!lib;
cleanup:
if (lib)
os_dlclose(lib);
#if defined(_WIN32)
finish:
#endif
profile_end(nvenc_check_name);
return success;
}
#endif
#if defined(LIBAVUTIL_VAAPI_AVAILABLE)
static bool vaapi_supported(void)
{
AVCodec *vaenc = avcodec_find_encoder_by_name("h264_vaapi");
return !!vaenc;
}
#endif
#if defined(_WIN32)
extern void jim_nvenc_load(void);
extern void jim_nvenc_unload(void);
#endif
#if ENABLE_FFMPEG_LOGGING
extern void obs_ffmpeg_load_logging(void);
extern void obs_ffmpeg_unload_logging(void);
#endif
bool obs_module_load(void)
{
obs_register_source(&ffmpeg_source);
obs_register_output(&ffmpeg_output);
obs_register_output(&ffmpeg_muxer);
obs_register_output(&replay_buffer);
obs_register_encoder(&aac_encoder_info);
obs_register_encoder(&opus_encoder_info);
obs_register_output(&ffmpeg_encoded_output_info);
#if !defined(__APPLE__)
if (nvenc_supported()) {
blog(LOG_INFO, "NVENC supported");
#if defined(_WIN32)
if (get_win_ver_int() > 0x0601) {
jim_nvenc_load();
} else {
nvenc_encoder_info.caps &= ~OBS_ENCODER_CAP_INTERNAL;
}
#endif
obs_register_encoder(&nvenc_encoder_info);
}
#if !defined(_WIN32) && defined(LIBAVUTIL_VAAPI_AVAILABLE)
if (vaapi_supported()) {
blog(LOG_INFO, "FFMPEG VAAPI supported");
obs_register_encoder(&vaapi_encoder_info);
}
#endif
#endif
#if ENABLE_FFMPEG_LOGGING
obs_ffmpeg_load_logging();
#endif
return true;
}
void obs_module_unload(void)
{
#if ENABLE_FFMPEG_LOGGING
obs_ffmpeg_unload_logging();
#endif
#if defined(_WIN32)
jim_nvenc_unload();
#endif
}
