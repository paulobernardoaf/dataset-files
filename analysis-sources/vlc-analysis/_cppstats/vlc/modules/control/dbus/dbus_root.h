#include "dbus_common.h"
#define DBUS_MPRIS_ROOT_INTERFACE "org.mpris.MediaPlayer2"
DBusHandlerResult handle_root ( DBusConnection *p_conn,
DBusMessage *p_from,
void *p_this );
int RootPropertiesChangedEmit ( intf_thread_t *,
vlc_dictionary_t * );
