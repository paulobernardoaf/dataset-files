





















#include "OMX_Core.h"

#include <vlc_common.h>

OMX_ERRORTYPE (*pf_init) (void);
OMX_ERRORTYPE (*pf_deinit) (void);
OMX_ERRORTYPE (*pf_get_handle) (OMX_HANDLETYPE *, OMX_STRING,
OMX_PTR, OMX_CALLBACKTYPE *);
OMX_ERRORTYPE (*pf_free_handle) (OMX_HANDLETYPE);
OMX_ERRORTYPE (*pf_component_enum)(OMX_STRING, OMX_U32, OMX_U32);
OMX_ERRORTYPE (*pf_get_roles_of_component)(OMX_STRING, OMX_U32 *, OMX_U8 **);

int InitOmxCore(vlc_object_t *p_this);
void DeinitOmxCore(void);

#define MAX_COMPONENTS_LIST_SIZE 32
int CreateComponentsList(vlc_object_t *p_this, const char *psz_role,
char ppsz_components[MAX_COMPONENTS_LIST_SIZE][OMX_MAX_STRINGNAME_SIZE]);

