
















#pragma once

#include <windows.h>
#include "../c99defs.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct reg_dword {
LSTATUS status;
DWORD size;
DWORD return_value;
};

EXPORT void get_reg_dword(HKEY hkey, LPCWSTR sub_key, LPCWSTR value_name,
struct reg_dword *info);

#if defined(__cplusplus)
}
#endif
