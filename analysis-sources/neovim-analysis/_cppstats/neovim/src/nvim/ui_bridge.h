#include <uv.h>
#include "nvim/ui.h"
#include "nvim/event/defs.h"
typedef struct ui_bridge_data UIBridgeData;
typedef void(*ui_main_fn)(UIBridgeData *bridge, UI *ui);
struct ui_bridge_data {
UI bridge; 
UI *ui; 
event_scheduler scheduler;
uv_thread_t ui_thread;
ui_main_fn ui_main;
uv_mutex_t mutex;
uv_cond_t cond;
bool ready;
bool stopped;
};
#define CONTINUE(b) do { UIBridgeData *d = (UIBridgeData *)b; uv_mutex_lock(&d->mutex); d->ready = true; uv_cond_signal(&d->cond); uv_mutex_unlock(&d->mutex); } while (0)
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "ui_bridge.h.generated.h"
#endif
