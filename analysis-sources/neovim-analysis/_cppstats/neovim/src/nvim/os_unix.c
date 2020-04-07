#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include "nvim/api/private/handle.h"
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/os_unix.h"
#include "nvim/buffer.h"
#include "nvim/charset.h"
#include "nvim/eval.h"
#include "nvim/ex_cmds.h"
#include "nvim/fileio.h"
#include "nvim/getchar.h"
#include "nvim/main.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/mouse.h"
#include "nvim/garray.h"
#include "nvim/path.h"
#include "nvim/screen.h"
#include "nvim/strings.h"
#include "nvim/syntax.h"
#include "nvim/ui.h"
#include "nvim/types.h"
#include "nvim/os/os.h"
#include "nvim/os/time.h"
#include "nvim/os/input.h"
#include "nvim/os/shell.h"
#include "nvim/os/signal.h"
#include "nvim/msgpack_rpc/helpers.h"
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "os_unix.c.generated.h"
#endif
#if defined(HAVE_ACL)
#if defined(HAVE_SYS_ACL_H)
#include <sys/acl.h>
#endif
#if defined(HAVE_SYS_ACCESS_H)
#include <sys/access.h>
#endif
vim_acl_T mch_get_acl(const char_u *fname)
{
vim_acl_T ret = NULL;
return ret;
}
void mch_set_acl(const char_u *fname, vim_acl_T aclent)
{
if (aclent == NULL)
return;
}
void mch_free_acl(vim_acl_T aclent)
{
if (aclent == NULL)
return;
}
#endif
