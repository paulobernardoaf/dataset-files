












#define HOOK_VER_MAJOR 1
#define HOOK_VER_MINOR 1
#define HOOK_VER_PATCH 4

#define STRINGIFY(s) #s
#define MAKE_VERSION_NAME(major, minor, patch) STRINGIFY(major) "." STRINGIFY(minor) "." STRINGIFY(patch) ".0"

#define HOOK_VERSION_NAME MAKE_VERSION_NAME(HOOK_VER_MAJOR, HOOK_VER_MINOR, HOOK_VER_PATCH)

