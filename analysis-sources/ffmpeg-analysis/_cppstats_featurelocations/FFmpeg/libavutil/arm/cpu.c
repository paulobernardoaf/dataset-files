

















#include "libavutil/cpu.h"
#include "libavutil/cpu_internal.h"
#include "config.h"

#define CORE_FLAG(f) (AV_CPU_FLAG_ ##f * (HAVE_ ##f ##_EXTERNAL || HAVE_ ##f ##_INLINE))


#define CORE_CPU_FLAGS (CORE_FLAG(ARMV5TE) | CORE_FLAG(ARMV6) | CORE_FLAG(ARMV6T2) | CORE_FLAG(VFP) | CORE_FLAG(VFPV3) | CORE_FLAG(NEON))







#if defined __linux__ || defined __ANDROID__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "libavutil/avstring.h"

#define AT_HWCAP 16


#define HWCAP_VFP (1 << 6)
#define HWCAP_EDSP (1 << 7)
#define HWCAP_THUMBEE (1 << 11)
#define HWCAP_NEON (1 << 12)
#define HWCAP_VFPv3 (1 << 13)
#define HWCAP_TLS (1 << 15)

static int get_hwcap(uint32_t *hwcap)
{
struct { uint32_t a_type; uint32_t a_val; } auxv;
FILE *f = fopen("/proc/self/auxv", "r");
int err = -1;

if (!f)
return -1;

while (fread(&auxv, sizeof(auxv), 1, f) > 0) {
if (auxv.a_type == AT_HWCAP) {
*hwcap = auxv.a_val;
err = 0;
break;
}
}

fclose(f);
return err;
}

static int get_cpuinfo(uint32_t *hwcap)
{
FILE *f = fopen("/proc/cpuinfo", "r");
char buf[200];

if (!f)
return -1;

*hwcap = 0;
while (fgets(buf, sizeof(buf), f)) {
if (av_strstart(buf, "Features", NULL)) {
if (strstr(buf, " edsp "))
*hwcap |= HWCAP_EDSP;
if (strstr(buf, " tls "))
*hwcap |= HWCAP_TLS;
if (strstr(buf, " thumbee "))
*hwcap |= HWCAP_THUMBEE;
if (strstr(buf, " vfp "))
*hwcap |= HWCAP_VFP;
if (strstr(buf, " vfpv3 "))
*hwcap |= HWCAP_VFPv3;
if (strstr(buf, " neon ") || strstr(buf, " asimd "))
*hwcap |= HWCAP_NEON;
if (strstr(buf, " fp ")) 
*hwcap |= HWCAP_VFP | HWCAP_VFPv3;
break;
}
}
fclose(f);
return 0;
}

int ff_get_cpu_flags_arm(void)
{
int flags = CORE_CPU_FLAGS;
uint32_t hwcap;

if (get_hwcap(&hwcap) < 0)
if (get_cpuinfo(&hwcap) < 0)
return flags;

#define check_cap(cap, flag) do { if (hwcap & HWCAP_ ##cap) flags |= AV_CPU_FLAG_ ##flag; } while (0)






check_cap(EDSP, ARMV5TE);
check_cap(TLS, ARMV6);
check_cap(THUMBEE, ARMV6T2);
check_cap(VFP, VFP);
check_cap(VFPv3, VFPV3);
check_cap(NEON, NEON);



if (flags & (AV_CPU_FLAG_VFPV3 | AV_CPU_FLAG_NEON))
flags |= AV_CPU_FLAG_ARMV6T2;
else if (flags & (AV_CPU_FLAG_ARMV6T2 | AV_CPU_FLAG_ARMV6))



flags |= AV_CPU_FLAG_SETEND;

if (flags & AV_CPU_FLAG_ARMV6T2)
flags |= AV_CPU_FLAG_ARMV6;


if ((flags & AV_CPU_FLAG_VFP) && !(flags & (AV_CPU_FLAG_VFPV3 | AV_CPU_FLAG_NEON)))
flags |= AV_CPU_FLAG_VFP_VM;

return flags;
}

#else

int ff_get_cpu_flags_arm(void)
{
return AV_CPU_FLAG_ARMV5TE * HAVE_ARMV5TE |
AV_CPU_FLAG_ARMV6 * HAVE_ARMV6 |
AV_CPU_FLAG_ARMV6T2 * HAVE_ARMV6T2 |
AV_CPU_FLAG_VFP * HAVE_VFP |
AV_CPU_FLAG_VFPV3 * HAVE_VFPV3 |
AV_CPU_FLAG_NEON * HAVE_NEON |
AV_CPU_FLAG_SETEND * !(HAVE_NEON | HAVE_VFPV3);
}

#endif

size_t ff_get_cpu_max_align_arm(void)
{
int flags = av_get_cpu_flags();

if (flags & AV_CPU_FLAG_NEON)
return 16;

return 8;
}
