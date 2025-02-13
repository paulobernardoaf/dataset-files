#if !defined(NVIM_API_PRIVATE_HANDLE_H)
#define NVIM_API_PRIVATE_HANDLE_H

#include "nvim/vim.h"
#include "nvim/buffer_defs.h"
#include "nvim/api/private/defs.h"

#define HANDLE_DECLS(type, name) type *handle_get_##name(handle_T handle); void handle_register_##name(type *name); void handle_unregister_##name(type *name);





HANDLE_DECLS(buf_T, buffer)

HANDLE_DECLS(win_T, window)

HANDLE_DECLS(tabpage_T, tabpage)

void handle_init(void);


#endif 

