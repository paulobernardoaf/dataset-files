#include <stddef.h>
typedef struct vim_state VimState;
typedef int(*state_check_callback)(VimState *state);
typedef int(*state_execute_callback)(VimState *state, int key);
struct vim_state {
state_check_callback check;
state_execute_callback execute;
};
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "state.h.generated.h"
#endif
