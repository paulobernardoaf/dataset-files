#include <vlc_input_item.h>
typedef struct input_fetcher_t input_fetcher_t;
input_fetcher_t *input_fetcher_New( vlc_object_t * );
int input_fetcher_Push( input_fetcher_t *, input_item_t *,
input_item_meta_request_option_t,
const input_fetcher_callbacks_t *, void * );
void input_fetcher_Delete( input_fetcher_t * );
