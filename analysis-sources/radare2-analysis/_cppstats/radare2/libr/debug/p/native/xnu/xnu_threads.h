#if __POWERPC__
#define R_REG_T ppc_thread_state_t
#define R_REG_STATE_T PPC_THREAD_STATE
#define R_REG_STATE_SZ PPC_THREAD_STATE_SZ
#elif __arm || __arm64 || __aarch64
#include <mach/arm/thread_status.h>
#if !defined(ARM_THREAD_STATE)
#define ARM_THREAD_STATE 1
#endif
#if !defined(ARM_THREAD_STATE64)
#define ARM_THREAD_STATE64 6
#endif
#define R_REG_T arm_unified_thread_state_t
#define R_REG_STATE_T MACHINE_THREAD_STATE
#define R_REG_STATE_SZ MACHINE_THREAD_STATE_COUNT
#elif __x86_64__ || __i386__
#define R_REG_T x86_thread_state_t
#define R_REG_STATE_T MACHINE_THREAD_STATE
#define R_REG_STATE_SZ MACHINE_THREAD_STATE_COUNT
#endif
#define RETURN_ON_MACH_ERROR(msg, retval)if (kr != KERN_SUCCESS) {mach_error (msg, kr); return ((retval));}
typedef struct _exception_info {
exception_mask_t masks[EXC_TYPES_COUNT];
mach_port_t ports[EXC_TYPES_COUNT];
exception_behavior_t behaviors[EXC_TYPES_COUNT];
thread_state_flavor_t flavors[EXC_TYPES_COUNT];
mach_msg_type_number_t count;
pthread_t thread;
mach_port_t exception_port;
} xnu_exception_info;
typedef struct _xnu_thread {
thread_t port; 
char *name; 
thread_basic_info_data_t basic_info; 
ut8 stepping; 
R_REG_T gpr; 
void *state;
ut32 state_size;
#if __arm64 || __aarch64 || __arm64__ || __aarch64__
union {
arm_debug_state32_t drx32;
arm_debug_state64_t drx64;
} debug;
#elif __arm__ || __arm || __armv7__
union {
arm_debug_state_t drx;
} debug;
#elif __x86_64__ || __i386__
x86_debug_state_t drx;
#endif
ut16 flavor;
ut32 count;
} xnu_thread_t;
typedef struct _exc_msg {
mach_msg_header_t hdr;
mach_msg_body_t msg_body;
mach_msg_port_descriptor_t thread;
mach_msg_port_descriptor_t task;
NDR_record_t NDR;
exception_type_t exception;
mach_msg_type_number_t code_cnt;
#if !__POWERPC__
mach_exception_data_t code;
#endif
char pad[512];
} exc_msg;
typedef struct _rep_msg {
mach_msg_header_t hdr;
NDR_record_t NDR;
kern_return_t ret_code;
} rep_msg;
