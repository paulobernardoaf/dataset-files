#if !defined(NVIM_OS_STDPATHS_DEFS_H)
#define NVIM_OS_STDPATHS_DEFS_H


typedef enum {
kXDGNone = -1,
kXDGConfigHome, 
kXDGDataHome, 
kXDGCacheHome, 
kXDGRuntimeDir, 
kXDGConfigDirs, 
kXDGDataDirs, 
} XDGVarType;

#endif 
