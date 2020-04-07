#include <grub/dl.h>
#include <grub/list.h>
#include <grub/misc.h>
#include <grub/types.h>
#include <grub/symbol.h>
struct grub_test
{
struct grub_test *next;
char *name;
void (*main) (void);
};
typedef struct grub_test *grub_test_t;
extern grub_test_t grub_test_list;
void grub_test_register (const char *name, void (*test) (void));
void grub_test_unregister (const char *name);
int grub_test_run (grub_test_t test);
void grub_test_nonzero (int cond, const char *file,
const char *func, grub_uint32_t line,
const char *fmt, ...)
__attribute__ ((format (printf, 5, 6)));
#define grub_test_assert(cond, ...) grub_test_nonzero(cond, GRUB_FILE, __FUNCTION__, __LINE__, ##__VA_ARGS__, "assert failed: %s", #cond)
#define GRUB_UNIT_TEST(name, funp) void grub_unit_test_init (void) { grub_test_register (name, funp); } void grub_unit_test_fini (void) { grub_test_unregister (name); }
#define GRUB_FUNCTIONAL_TEST(name, funp) GRUB_MOD_INIT(functional_test_##funp) { grub_test_register (name, funp); } GRUB_MOD_FINI(functional_test_##funp) { grub_test_unregister (name); }
