#pragma once
#include "calldata.h"
#include "../util/darray.h"
#if defined(__cplusplus)
extern "C" {
#endif
struct decl_param {
char *name;
enum call_param_type type;
uint32_t flags;
};
static inline void decl_param_free(struct decl_param *param)
{
if (param->name)
bfree(param->name);
memset(param, 0, sizeof(struct decl_param));
}
struct decl_info {
char *name;
const char *decl_string;
DARRAY(struct decl_param) params;
};
static inline void decl_info_free(struct decl_info *decl)
{
if (decl) {
for (size_t i = 0; i < decl->params.num; i++)
decl_param_free(decl->params.array + i);
da_free(decl->params);
bfree(decl->name);
memset(decl, 0, sizeof(struct decl_info));
}
}
EXPORT bool parse_decl_string(struct decl_info *decl, const char *decl_string);
#if defined(__cplusplus)
}
#endif
