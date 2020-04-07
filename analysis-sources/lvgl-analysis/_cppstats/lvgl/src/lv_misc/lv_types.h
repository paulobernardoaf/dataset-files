#if defined(__cplusplus)
extern "C" {
#endif
#if defined(_WIN64)
#define LV_ARCH_64
#endif
#if defined(__GNUC__)
#if defined(__x86_64__) || defined(__ppc64__)
#define LV_ARCH_64
#endif
#endif
enum {
LV_RES_INV = 0, 
LV_RES_OK, 
};
typedef uint8_t lv_res_t;
#if defined(LV_ARCH_64)
typedef uint64_t lv_uintptr_t;
#else
typedef uint32_t lv_uintptr_t;
#endif
#if defined(__cplusplus)
} 
#endif
