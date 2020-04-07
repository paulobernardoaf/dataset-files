

















#if !defined(AVFILTER_GLSLANG_H)
#define AVFILTER_GLSLANG_H

#include <stdlib.h>

#if defined(__cplusplus)
extern "C" {
#endif

int glslang_init(void);
void glslang_uninit(void);

typedef struct GLSlangResult {
int rval;
char *error_msg;

void *data; 
size_t size;
} GLSlangResult;

enum GLSlangStage {
GLSLANG_VERTEX,
GLSLANG_FRAGMENT,
GLSLANG_COMPUTE,
};


GLSlangResult *glslang_compile(const char *glsl, enum GLSlangStage stage);

#if defined(__cplusplus)
}
#endif

#endif 
