
























#if !defined(VLC_CPU_H)
#define VLC_CPU_H 1




VLC_API unsigned vlc_CPU(void);







unsigned vlc_CPU_raw(void);

#if defined (__i386__) || defined (__x86_64__)
#define HAVE_FPU 1
#define VLC_CPU_MMX 0x00000008
#define VLC_CPU_3dNOW 0x00000010
#define VLC_CPU_MMXEXT 0x00000020
#define VLC_CPU_SSE 0x00000040
#define VLC_CPU_SSE2 0x00000080
#define VLC_CPU_SSE3 0x00000100
#define VLC_CPU_SSSE3 0x00000200
#define VLC_CPU_SSE4_1 0x00000400
#define VLC_CPU_SSE4_2 0x00000800
#define VLC_CPU_SSE4A 0x00001000
#define VLC_CPU_AVX 0x00002000
#define VLC_CPU_AVX2 0x00004000
#define VLC_CPU_XOP 0x00008000
#define VLC_CPU_FMA4 0x00010000

#if defined (__MMX__)
#define vlc_CPU_MMX() (1)
#define VLC_MMX
#else
#define vlc_CPU_MMX() ((vlc_CPU() & VLC_CPU_MMX) != 0)
#define VLC_MMX __attribute__ ((__target__ ("mmx")))
#endif

#if defined (__SSE__)
#define vlc_CPU_MMXEXT() (1)
#define vlc_CPU_SSE() (1)
#define VLC_SSE
#else
#define vlc_CPU_MMXEXT() ((vlc_CPU() & VLC_CPU_MMXEXT) != 0)
#define vlc_CPU_SSE() ((vlc_CPU() & VLC_CPU_SSE) != 0)
#define VLC_SSE __attribute__ ((__target__ ("sse")))
#endif

#if defined(__SSE2__)
#define vlc_CPU_SSE2() (1)
#else
#define vlc_CPU_SSE2() ((vlc_CPU() & VLC_CPU_SSE2) != 0)
#endif

#if defined(__SSE3__)
#define vlc_CPU_SSE3() (1)
#else
#define vlc_CPU_SSE3() ((vlc_CPU() & VLC_CPU_SSE3) != 0)
#endif

#if defined(__SSSE3__)
#define vlc_CPU_SSSE3() (1)
#else
#define vlc_CPU_SSSE3() ((vlc_CPU() & VLC_CPU_SSSE3) != 0)
#endif

#if defined(__SSE4_1__)
#define vlc_CPU_SSE4_1() (1)
#else
#define vlc_CPU_SSE4_1() ((vlc_CPU() & VLC_CPU_SSE4_1) != 0)
#endif

#if defined(__SSE4_2__)
#define vlc_CPU_SSE4_2() (1)
#else
#define vlc_CPU_SSE4_2() ((vlc_CPU() & VLC_CPU_SSE4_2) != 0)
#endif

#if defined(__SSE4A__)
#define vlc_CPU_SSE4A() (1)
#else
#define vlc_CPU_SSE4A() ((vlc_CPU() & VLC_CPU_SSE4A) != 0)
#endif

#if defined(__AVX__)
#define vlc_CPU_AVX() (1)
#else
#define vlc_CPU_AVX() ((vlc_CPU() & VLC_CPU_AVX) != 0)
#endif

#if defined(__AVX2__)
#define vlc_CPU_AVX2() (1)
#else
#define vlc_CPU_AVX2() ((vlc_CPU() & VLC_CPU_AVX2) != 0)
#endif

#if defined(__3dNOW__)
#define vlc_CPU_3dNOW() (1)
#else
#define vlc_CPU_3dNOW() ((vlc_CPU() & VLC_CPU_3dNOW) != 0)
#endif

#if defined(__XOP__)
#define vlc_CPU_XOP() (1)
#else
#define vlc_CPU_XOP() ((vlc_CPU() & VLC_CPU_XOP) != 0)
#endif

#if defined(__FMA4__)
#define vlc_CPU_FMA4() (1)
#else
#define vlc_CPU_FMA4() ((vlc_CPU() & VLC_CPU_FMA4) != 0)
#endif

#elif defined (__ppc__) || defined (__ppc64__) || defined (__powerpc__)
#define HAVE_FPU 1
#define VLC_CPU_ALTIVEC 2

#if defined(ALTIVEC)
#define vlc_CPU_ALTIVEC() (1)
#else
#define vlc_CPU_ALTIVEC() ((vlc_CPU() & VLC_CPU_ALTIVEC) != 0)
#endif

#elif defined (__arm__)
#if defined (__VFP_FP__) && !defined (__SOFTFP__)
#define HAVE_FPU 1
#else
#define HAVE_FPU 0
#endif
#define VLC_CPU_ARMv6 4
#define VLC_CPU_ARM_NEON 2

#if defined (__ARM_ARCH_7A__)
#define VLC_CPU_ARM_ARCH 7
#elif defined (__ARM_ARCH_6__) || defined (__ARM_ARCH_6T2__)
#define VLC_CPU_ARM_ARCH 6
#else
#define VLC_CPU_ARM_ARCH 4
#endif

#if (VLC_CPU_ARM_ARCH >= 6)
#define vlc_CPU_ARMv6() (1)
#else
#define vlc_CPU_ARMv6() ((vlc_CPU() & VLC_CPU_ARMv6) != 0)
#endif

#if defined(__ARM_NEON__)
#define vlc_CPU_ARM_NEON() (1)
#else
#define vlc_CPU_ARM_NEON() ((vlc_CPU() & VLC_CPU_ARM_NEON) != 0)
#endif

#elif defined (__aarch64__)
#define HAVE_FPU 1
#define VLC_CPU_ARM_NEON 0x1
#define VLC_CPU_ARM_SVE 0x2

#if defined(__ARM_NEON)
#define vlc_CPU_ARM_NEON() (1)
#else
#define vlc_CPU_ARM_NEON() ((vlc_CPU() & VLC_CPU_ARM_NEON) != 0)
#endif

#if defined(__ARM_FEATURE_SVE)
#define vlc_CPU_ARM_SVE() (1)
#else
#define vlc_CPU_ARM_SVE() ((vlc_CPU() & VLC_CPU_ARM_SVE) != 0)
#endif

#elif defined (__sparc__)
#define HAVE_FPU 1

#elif defined (__mips_hard_float)
#define HAVE_FPU 1

#else





#define HAVE_FPU 0

#endif

#endif 
