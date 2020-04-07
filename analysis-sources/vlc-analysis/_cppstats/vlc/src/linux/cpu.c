#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <vlc_common.h>
#include <vlc_cpu.h>
#undef CPU_FLAGS
#if defined (__arm__) || defined (__aarch64__)
#define CPU_FLAGS "Features\t:"
#elif defined (__i386__) || defined (__x86_64__)
#define CPU_FLAGS "flags\t\t:"
#elif defined (__powerpc__) || defined (__powerpc64__)
#define CPU_FLAGS "cpu\t\t:"
#endif
#if defined(CPU_FLAGS)
unsigned vlc_CPU_raw(void)
{
FILE *info = fopen ("/proc/cpuinfo", "rte");
if (info == NULL)
return 0;
char *line = NULL;
size_t linelen = 0;
uint_fast32_t all_caps = 0xFFFFFFFF;
while (getline (&line, &linelen, info) != -1)
{
char *p = line, *cap;
uint_fast32_t core_caps = 0;
#if defined (__arm__)
unsigned ver;
if (sscanf (line, "Processor\t: ARMv%u", &ver) >= 1 && ver >= 6)
core_caps |= VLC_CPU_ARMv6;
#endif
if (strncmp (line, CPU_FLAGS, strlen (CPU_FLAGS)))
continue;
while ((cap = strsep (&p, " ")) != NULL)
{
#if defined (__arm__) || defined (__aarch64__)
if (!strcmp (cap, "neon"))
core_caps |= VLC_CPU_ARM_NEON;
#if defined (__aarch64__)
if (!strcmp (cap, "sve"))
core_caps |= VLC_CPU_ARM_SVE;
#endif
#elif defined (__i386__) || defined (__x86_64__)
if (!strcmp (cap, "mmx"))
core_caps |= VLC_CPU_MMX;
if (!strcmp (cap, "sse"))
core_caps |= VLC_CPU_SSE | VLC_CPU_MMXEXT;
if (!strcmp (cap, "mmxext"))
core_caps |= VLC_CPU_MMXEXT;
if (!strcmp (cap, "sse2"))
core_caps |= VLC_CPU_SSE2;
if (!strcmp (cap, "pni"))
core_caps |= VLC_CPU_SSE3;
if (!strcmp (cap, "ssse3"))
core_caps |= VLC_CPU_SSSE3;
if (!strcmp (cap, "sse4_1"))
core_caps |= VLC_CPU_SSE4_1;
if (!strcmp (cap, "sse4_2"))
core_caps |= VLC_CPU_SSE4_2;
if (!strcmp (cap, "sse4a"))
core_caps |= VLC_CPU_SSE4A;
if (!strcmp (cap, "avx"))
core_caps |= VLC_CPU_AVX;
if (!strcmp (cap, "avx2"))
core_caps |= VLC_CPU_AVX2;
if (!strcmp (cap, "3dnow"))
core_caps |= VLC_CPU_3dNOW;
if (!strcmp (cap, "xop"))
core_caps |= VLC_CPU_XOP;
if (!strcmp (cap, "fma4"))
core_caps |= VLC_CPU_FMA4;
#elif defined (__powerpc__) || defined (__powerpc64__)
if (!strcmp (cap, "altivec supported"))
core_caps |= VLC_CPU_ALTIVEC;
#endif
}
all_caps &= core_caps;
}
fclose (info);
free (line);
if (all_caps == 0xFFFFFFFF) 
all_caps = 0; 
return all_caps;
}
#endif
