#if defined(_ALLEGRO_GLX_VERSION_1_3)

AGL_API(GLXFBConfig *, GetFBConfigs, (Display *, int, int *))
AGL_API(GLXFBConfig *, ChooseFBConfig, (Display *, int, const int *, int *))
AGL_API(int, GetFBConfigAttrib, (Display *, GLXFBConfig, int, int *))
AGL_API(XVisualInfo *, GetVisualFromFBConfig, (Display *, GLXFBConfig))
AGL_API(GLXWindow, CreateWindow, (Display *, GLXFBConfig, Window, const int *))
AGL_API(void, DestroyWindow, (Display *, GLXWindow))
AGL_API(GLXPixmap, CreatePixmap, (Display *, GLXFBConfig, Pixmap, const int *))
AGL_API(void, DestroyPixmap, (Display *, GLXPixmap))
AGL_API(GLXPbuffer, CreatePbuffer, (Display *, GLXFBConfig, const int *))
AGL_API(void, DestroyPbuffer, (Display *, GLXPbuffer))
AGL_API(void, QueryDrawable, (Display *, GLXDrawable, int, unsigned int *))
AGL_API(GLXContext, CreateNewContext, (Display *, GLXFBConfig, int, GLXContext, Bool))
AGL_API(Bool, MakeContextCurrent, (Display *, GLXDrawable, GLXDrawable, GLXContext))
AGL_API(GLXDrawable, GetCurrentReadDrawable, (void))
AGL_API(Display *, GetCurrentDisplay, (void))
AGL_API(int, QueryContext, (Display *, GLXContext, int, int *))
AGL_API(void, SelectEvent, (Display *, GLXDrawable, unsigned long))
AGL_API(void, GetSelectedEvent, (Display *, GLXDrawable, unsigned long *))
#endif
#if defined(_ALLEGRO_GLX_VERSION_1_4)

AGL_API(__GLXextFuncPtr, GetProcAddress, (const GLubyte *))
#endif

#if defined(_ALLEGRO_GLX_ARB_get_proc_address)

AGL_API(__GLXextFuncPtr, GetProcAddressARB, (const GLubyte *))
#endif

#if defined(_ALLEGRO_GLX_ARB_create_context)
AGL_API(GLXContext, CreateContextAttribsARB, (Display *, GLXFBConfig, GLXContext, Bool, const int *))
#endif

#if defined(_ALLEGRO_GLX_SGI_swap_control)

AGL_API(int, SwapIntervalSGI, (int))
#endif

#if defined(_ALLEGRO_GLX_SGI_video_sync)

AGL_API(int, GetVideoSyncSGI, (unsigned int *))
AGL_API(int, WaitVideoSyncSGI, (int, int, unsigned int *))
#endif

#if defined(_ALLEGRO_GLX_SGI_make_current_read)

AGL_API(Bool, MakeCurrentReadSGI, (Display *, GLXDrawable, GLXDrawable, GLXContext))
AGL_API(GLXDrawable, GetCurrentReadDrawableSGI, (void))
#endif

#if defined(_ALLEGRO_GLX_SGIX_video_source)


#if defined(_VL_H_)
AGL_API(GLXVideoSourceSGIX, CreateGLXVideoSourceSGIX, (Display *, int, VLServer, VLPath, int, VLNode))
AGL_API(void, DestroyGLXVideoSourceSGIX, (Display *, GLXVideoSourceSGIX))
#endif
#endif

#if defined(_ALLEGRO_GLX_EXT_import_context)

AGL_API(Display *, GetCurrentDisplayEXT, (void))
AGL_API(int, QueryContextInfoEXT, (Display *, GLXContext, int, int *))
AGL_API(GLXContextID, GetContextIDEXT, (const GLXContext))
AGL_API(GLXContext, ImportContextEXT, (Display *, GLXContextID))
AGL_API(void, FreeContextEXT, (Display *, GLXContext))
#endif

#if defined(_ALLEGRO_GLX_SGIX_fbconfig)

AGL_API(int, GetFBConfigAttribSGIX, (Display *, GLXFBConfigSGIX, int, int *))
AGL_API(GLXFBConfigSGIX *, ChooseFBConfigSGIX, (Display *, int, int *, int *))
AGL_API(GLXPixmap, CreateGLXPixmapWithConfigSGIX, (Display *, GLXFBConfigSGIX, Pixmap))
AGL_API(GLXContext, CreateContextWithConfigSGIX, (Display *, GLXFBConfigSGIX, int, GLXContext, Bool))
AGL_API(XVisualInfo *, GetVisualFromFBConfigSGIX, (Display *, GLXFBConfigSGIX))
AGL_API(GLXFBConfigSGIX, GetFBConfigFromVisualSGIX, (Display *, XVisualInfo *))
#endif

#if defined(_ALLEGRO_GLX_SGIX_pbuffer)

AGL_API(GLXPbufferSGIX, CreateGLXPbufferSGIX, (Display *, GLXFBConfigSGIX, unsigned int, unsigned int, int *))
AGL_API(void, DestroyGLXPbufferSGIX, (Display *, GLXPbufferSGIX))
AGL_API(int, QueryGLXPbufferSGIX, (Display *, GLXPbufferSGIX, int, unsigned int *))
AGL_API(void, SelectEventSGIX, (Display *, GLXDrawable, unsigned long))
AGL_API(void, GetSelectedEventSGIX, (Display *, GLXDrawable, unsigned long *))
#endif

#if defined(_ALLEGRO_GLX_SGI_cushion)

AGL_API(void, CushionSGI, (Display *, Window, float))
#endif

#if defined(_ALLEGRO_GLX_SGIX_video_resize)

AGL_API(int, BindChannelToWindowSGIX, (Display *, int, int, Window))
AGL_API(int, ChannelRectSGIX, (Display *, int, int, int, int, int, int))
AGL_API(int, QueryChannelRectSGIX, (Display *, int, int, int *, int *, int *, int *))
AGL_API(int, QueryChannelDeltasSGIX, (Display *, int, int, int *, int *, int *, int *))
AGL_API(int, ChannelRectSyncSGIX, (Display *, int, int, GLenum))
#endif

#if defined(_ALLEGRO_GLX_SGIX_dmbuffer)


#if defined(_DM_BUFFER_H_)
AGL_API(Bool, AssociateDMPbufferSGIX, (Display *, GLXPbufferSGIX, DMparams *, DMbuffer))
#endif
#endif

#if defined(_ALLEGRO_GLX_SGIX_swap_group)

AGL_API(void, JoinSwapGroupSGIX, (Display *, GLXDrawable, GLXDrawable))
#endif

#if defined(_ALLEGRO_GLX_SGIX_swap_barrier)

AGL_API(void, BindSwapBarrierSGIX, (Display *, GLXDrawable, int))
AGL_API(Bool, QueryMaxSwapBarriersSGIX, (Display *, int, int *))
#endif

#if defined(_ALLEGRO_GLX_SUN_get_transparent_index)

AGL_API(Status, GetTransparentIndexSUN, (Display *, Window, Window, long *))
#endif

#if defined(_ALLEGRO_GLX_MESA_copy_sub_buffer)

AGL_API(void, CopySubBufferMESA, (Display *, GLXDrawable, int, int, int, int))
#endif

#if defined(_ALLEGRO_GLX_MESA_pixmap_colormap)

AGL_API(GLXPixmap, CreateGLXPixmapMESA, (Display *, XVisualInfo *, Pixmap, Colormap))
#endif

#if defined(_ALLEGRO_GLX_MESA_release_buffers)

AGL_API(Bool, ReleaseBuffersMESA, (Display *, GLXDrawable))
#endif

#if defined(_ALLEGRO_GLX_MESA_set_3dfx_mode)

AGL_API(Bool, Set3DfxModeMESA, (int))
#endif

#if defined(_ALLEGRO_GLX_OML_sync_control)

AGL_API(Bool, GetSyncValuesOML, (Display *, GLXDrawable, int64_t *, int64_t *, int64_t *))
AGL_API(Bool, GetMscRateOML, (Display *, GLXDrawable, int32_t *, int32_t *))
AGL_API(int64_t, SwapBuffersMscOML, (Display *, GLXDrawable, int64_t, int64_t, int64_t))
AGL_API(Bool, WaitForMscOML, (Display *, GLXDrawable, int64_t, int64_t, int64_t, int64_t *, int64_t *, int64_t *))
AGL_API(Bool, WaitForSbcOML, (Display *, GLXDrawable, int64_t, int64_t *, int64_t *, int64_t *))
#endif


#if defined(_ALLEGRO_GLX_SGIX_hyperpipe)
AGL_API(GLXHyperpipeNetworkSGIX *, QueryHyperpipeNetworkSGIX, (Display *dpy, int *npipes))
AGL_API(int, HyperpipeConfigSGIX, (Display *dpy, int networkId, int npipes, GLXHyperpipeConfigSGIX *cfg, int *hpId))
AGL_API(GLXHyperpipeConfigSGIX *, QueryHyperpipeConfigSGIX, (Display *dpy, int hpId, int *npipes))
AGL_API(int, DestroyHyperpipeConfigSGIX, (Display * dpy, int hpId))
AGL_API(int, BindHyperpipeSGIX, (Display *dpy, int hpId))
AGL_API(int, QueryHyperpipeBestAttribSGIX, (Display *dpy, int timeSlice, int attrib, int size, void *attribList, void *returnAttribList))
AGL_API(int, HyperpipeAttribSGIX, (Display *dpy, int timeSlice, int attrib, int size, void *attribList))
AGL_API(int, QueryHyperpipeAttribSGIX, (Display *dpy, int timeSlice, int attrib, int size, void *returnAttribList))
#endif


#if defined(_ALLEGRO_GLX_MESA_agp_offset)
AGL_API(unsigned int, GetAGPOffsetMESA, (const void *pointer))
#endif


#if defined(_ALLEGRO_GLX_EXT_texture_from_pixmap)
AGL_API(void, BindTexImageEXT, (Display *dpy, GLXDrawable drawable, int buffer, const int *attrib_list))
AGL_API(void, ReleaseTextImageEXT, (Display *dpy, GLXDrawable drawable, int buffer))
#endif

#if defined(_ALLEGRO_GLX_NV_video_output)
AGL_API(int, GetVideoDeviceNV, (Display *dpy, int screen, int numVideoDevices, GLXVideoDeviceNV *pVideoDevice))
AGL_API(int, ReleaseVideoDeviceNV, (Display *dpy, int screen, GLXVideoDeviceNV VideoDevice))
AGL_API(int, BindVideoImageNV, (Display *dpy, GLXVideoDeviceNV VideoDevice, GLXPbuffer pbuf, int iVideoBuffer))
AGL_API(int, ReleaseVideoImageNV, (Display *dpy, GLXPbuffer pbuf))
AGL_API(int, SendPbufferToVideoNV, (Display *dpy, GLXPbuffer pbuf, int iBufferType, unsigned long *pulCounterPbuffer, GLboolean bBlock))
AGL_API(int, GetVideoInfoNV, (Display *dpy, int screen, GLXVideoDeviceNV VideoDevice, unsigned long *pulCounterOutputVideo, unsigned long *pulCounterOutputPbuffer))
#endif

#if defined(_ALLEGRO_GLX_NV_swap_group)
AGL_API(Bool, JoinSwapGroupNV, (Display *dpy, GLXDrawable drawable, GLuint group))
AGL_API(Bool, BindSwapBarrierNV, (Display *dpy, GLuint group, GLuint barrier))
AGL_API(Bool, QuerySwapGroupNV, (Display *dpy, GLXDrawable drawable, GLuint *group, GLuint *barrier))
AGL_API(Bool, QueryMaxSwapGroupsNV, (Display *dpy, int screen, GLuint *maxGroups, GLuint *maxBarriers))
AGL_API(Bool, QueryFrameCountNV, (Display *dpy, int screen, GLuint *count))
AGL_API(Bool, ResetFrameCountNV, (Display *dpy, int screen))
#endif

#if defined(_ALLEGRO_GLX_NV_video_capture)
AGL_API(int, BindVideoCaptureDeviceNV, (Display *dpy, unsigned int video_capture_slot, GLXVideoCaptureDeviceNV device))
AGL_API(GLXVideoCaptureDeviceNV *, EnumerateVideoCaptureDevicesNV, (Display *dpy, int screen, int *nelements))
AGL_API(void, LockVideoCaptureDeviceNV, (Display *dpy, GLXVideoCaptureDeviceNV device))
AGL_API(int, QueryVideoCaptureDeviceNV, (Display *dpy, GLXVideoCaptureDeviceNV device, int attribute, int *value))
AGL_API(void, ReleaseVideoCaptureDeviceNV, (Display *dpy, GLXVideoCaptureDeviceNV device))
#endif

#if defined(_ALLEGRO_GLX_EXT_swap_control)
AGL_API(int, SwapIntervalEXT, (Display *dpy, GLXDrawable drawable, int interval))
#endif

#if defined(_ALLEGRO_GLX_NV_copy_image)
AGL_API(void, CopyImageSubDataNV, (Display *dpy, GLXContext srcCtx, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLXContext dstCtx, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth))
#endif

#if defined(_ALLEGRO_GLX_EXT_create_context_es_profile)

#endif
