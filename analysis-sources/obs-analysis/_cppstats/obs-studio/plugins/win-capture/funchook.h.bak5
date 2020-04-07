#pragma once

#include <stdbool.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#else
#if defined(_MSC_VER) && !defined(inline)
#define inline __inline
#endif
#endif

enum hook_type {
HOOKTYPE_FORWARD_OVERWRITE,
HOOKTYPE_FORWARD_CHAIN,
HOOKTYPE_REVERSE_CHAIN
};

struct func_hook {
void *call_addr;

uintptr_t func_addr; 
uintptr_t hook_addr; 
void *bounce_addr;
const char *name;
enum hook_type type;
bool is_64bit_jump;
bool hooked;
bool started;
bool attempted_bounce;
uint8_t unhook_data[14];
uint8_t rehook_data[14];
};

extern void hook_init(struct func_hook *hook, void *func_addr, void *hook_addr,
const char *name);
extern void hook_start(struct func_hook *hook);
extern void do_hook(struct func_hook *hook, bool force);
extern void unhook(struct func_hook *hook);

static inline void rehook(struct func_hook *hook)
{
do_hook(hook, false);
}

static inline void force_rehook(struct func_hook *hook)
{
do_hook(hook, true);
}

#if defined(__cplusplus)
}
#endif
