






































#if !defined(SIMDE_ARCH_H)
#define SIMDE_ARCH_H



#if defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)
#if defined(__alpha_ev6__)
#define SIMDE_ARCH_ALPHA 6
#elif defined(__alpha_ev5__)
#define SIMDE_ARCH_ALPHA 5
#elif defined(__alpha_ev4__)
#define SIMDE_ARCH_ALPHA 4
#else
#define SIMDE_ARCH_ALPHA 1
#endif
#endif



#if defined(__AVR_ARCH__)
#define SIMDE_ARCH_AVR __AVR_ARCH__
#endif



#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X66) || defined(_M_AMD64)

#define SIMDE_ARCH_AMD64 1
#endif



#if defined(__ARM_ARCH_8A__)
#define SIMDE_ARCH_ARM 82
#elif defined(__ARM_ARCH_8R__)
#define SIMDE_ARCH_ARM 81
#elif defined(__ARM_ARCH_8__)
#define SIMDE_ARCH_ARM 80
#elif defined(__ARM_ARCH_7S__)
#define SIMDE_ARCH_ARM 74
#elif defined(__ARM_ARCH_7M__)
#define SIMDE_ARCH_ARM 73
#elif defined(__ARM_ARCH_7R__)
#define SIMDE_ARCH_ARM 72
#elif defined(__ARM_ARCH_7A__)
#define SIMDE_ARCH_ARM 71
#elif defined(__ARM_ARCH_7__)
#define SIMDE_ARCH_ARM 70
#elif defined(__ARM_ARCH)
#define SIMDE_ARCH_ARM (__ARM_ARCH * 10)
#elif defined(_M_ARM)
#define SIMDE_ARCH_ARM (_M_ARM * 10)
#elif defined(__arm__) || defined(__thumb__) || defined(__TARGET_ARCH_ARM) || defined(_ARM) || defined(_M_ARM) || defined(_M_ARM)

#define SIMDE_ARCH_ARM 1
#endif



#if defined(__aarch64__) || defined(_M_ARM64)
#define SIMDE_ARCH_AARCH64 10
#endif



#if defined(__bfin) || defined(__BFIN__) || defined(__bfin__)
#define SIMDE_ARCH_BLACKFIN 1
#endif



#if defined(__CRIS_arch_version)
#define SIMDE_ARCH_CRIS __CRIS_arch_version
#elif defined(__cris__) || defined(__cris) || defined(__CRIS) || defined(__CRIS__)

#define SIMDE_ARCH_CRIS 1
#endif



#if defined(__convex_c38__)
#define SIMDE_ARCH_CONVEX 38
#elif defined(__convex_c34__)
#define SIMDE_ARCH_CONVEX 34
#elif defined(__convex_c32__)
#define SIMDE_ARCH_CONVEX 32
#elif defined(__convex_c2__)
#define SIMDE_ARCH_CONVEX 2
#elif defined(__convex__)
#define SIMDE_ARCH_CONVEX 1
#endif



#if defined(__epiphany__)
#define SIMDE_ARCH_EPIPHANY 1
#endif



#if defined(__frv__)
#define SIMDE_ARCH_FRV 1
#endif



#if defined(__H8300__)
#define SIMDE_ARCH_H8300
#endif



#if defined(__PA8000__) || defined(__HPPA20__) || defined(__RISC2_0__) || defined(_PA_RISC2_0)

#define SIMDE_ARCH_HPPA 20
#elif defined(__PA7100__) || defined(__HPPA11__) || defined(_PA_RISC1_1)
#define SIMDE_ARCH_HPPA 11
#elif defined(_PA_RISC1_0)
#define SIMDE_ARCH_HPPA 10
#elif defined(__hppa__) || defined(__HPPA__) || defined(__hppa)
#define SIMDE_ARCH_HPPA 1
#endif



#if defined(_M_IX86)
#define SIMDE_ARCH_X86 (_M_IX86 / 100)
#elif defined(__I86__)
#define SIMDE_ARCH_X86 __I86__
#elif defined(i686) || defined(__i686) || defined(__i686__)
#define SIMDE_ARCH_X86 6
#elif defined(i586) || defined(__i586) || defined(__i586__)
#define SIMDE_ARCH_X86 5
#elif defined(i486) || defined(__i486) || defined(__i486__)
#define SIMDE_ARCH_X86 4
#elif defined(i386) || defined(__i386) || defined(__i386__)
#define SIMDE_ARCH_X86 3
#elif defined(_X86_) || defined(__X86__) || defined(__THW_INTEL__)
#define SIMDE_ARCH_X86 3
#endif



#if defined(__ia64__) || defined(_IA64) || defined(__IA64__) || defined(__ia64) || defined(_M_IA64) || defined(__itanium__)

#define SIMDE_ARCH_IA64 1
#endif



#if defined(__m32r__) || defined(__M32R__)
#define SIMDE_ARCH_M32R
#endif



#if defined(__mc68060__) || defined(__MC68060__)
#define SIMDE_ARCH_M68K 68060
#elif defined(__mc68040__) || defined(__MC68040__)
#define SIMDE_ARCH_M68K 68040
#elif defined(__mc68030__) || defined(__MC68030__)
#define SIMDE_ARCH_M68K 68030
#elif defined(__mc68020__) || defined(__MC68020__)
#define SIMDE_ARCH_M68K 68020
#elif defined(__mc68010__) || defined(__MC68010__)
#define SIMDE_ARCH_M68K 68010
#elif defined(__mc68000__) || defined(__MC68000__)
#define SIMDE_ARCH_M68K 68000
#endif



#if defined(__MICROBLAZE__) || defined(__microblaze__)
#define SIMDE_ARCH_MICROBLAZE
#endif



#if defined(_MIPS_ISA_MIPS64R2)
#define SIMDE_ARCH_MIPS 642
#elif defined(_MIPS_ISA_MIPS64)
#define SIMDE_ARCH_MIPS 640
#elif defined(_MIPS_ISA_MIPS32R2)
#define SIMDE_ARCH_MIPS 322
#elif defined(_MIPS_ISA_MIPS32)
#define SIMDE_ARCH_MIPS 320
#elif defined(_MIPS_ISA_MIPS4)
#define SIMDE_ARCH_MIPS 4
#elif defined(_MIPS_ISA_MIPS3)
#define SIMDE_ARCH_MIPS 3
#elif defined(_MIPS_ISA_MIPS2)
#define SIMDE_ARCH_MIPS 2
#elif defined(_MIPS_ISA_MIPS1)
#define SIMDE_ARCH_MIPS 1
#elif defined(_MIPS_ISA_MIPS) || defined(__mips) || defined(__MIPS__)
#define SIMDE_ARCH_MIPS 1
#endif



#if defined(__MN10300__) || defined(__mn10300__)
#define SIMDE_ARCH_MN10300 1
#endif



#if defined(_M_PPC)
#define SIMDE_ARCH_POWER _M_PPC
#elif defined(_ARCH_PWR8)
#define SIMDE_ARCH_POWER 800
#elif defined(_ARCH_PWR7)
#define SIMDE_ARCH_POWER 700
#elif defined(_ARCH_PWR6)
#define SIMDE_ARCH_POWER 600
#elif defined(_ARCH_PWR5)
#define SIMDE_ARCH_POWER 500
#elif defined(_ARCH_PWR4)
#define SIMDE_ARCH_POWER 400
#elif defined(_ARCH_440) || defined(__ppc440__)
#define SIMDE_ARCH_POWER 440
#elif defined(_ARCH_450) || defined(__ppc450__)
#define SIMDE_ARCH_POWER 450
#elif defined(_ARCH_601) || defined(__ppc601__)
#define SIMDE_ARCH_POWER 601
#elif defined(_ARCH_603) || defined(__ppc603__)
#define SIMDE_ARCH_POWER 603
#elif defined(_ARCH_604) || defined(__ppc604__)
#define SIMDE_ARCH_POWER 604
#elif defined(_ARCH_605) || defined(__ppc605__)
#define SIMDE_ARCH_POWER 605
#elif defined(_ARCH_620) || defined(__ppc620__)
#define SIMDE_ARCH_POWER 620
#elif defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC) || defined(__ppc)


#define SIMDE_ARCH_POWER 1
#endif



#if defined(__sparc_v9__) || defined(__sparcv9)
#define SIMDE_ARCH_SPARC 9
#elif defined(__sparc_v8__) || defined(__sparcv8)
#define SIMDE_ARCH_SPARC 8
#elif defined(__sparc_v7__) || defined(__sparcv7)
#define SIMDE_ARCH_SPARC 7
#elif defined(__sparc_v6__) || defined(__sparcv6)
#define SIMDE_ARCH_SPARC 6
#elif defined(__sparc_v5__) || defined(__sparcv5)
#define SIMDE_ARCH_SPARC 5
#elif defined(__sparc_v4__) || defined(__sparcv4)
#define SIMDE_ARCH_SPARC 4
#elif defined(__sparc_v3__) || defined(__sparcv3)
#define SIMDE_ARCH_SPARC 3
#elif defined(__sparc_v2__) || defined(__sparcv2)
#define SIMDE_ARCH_SPARC 2
#elif defined(__sparc_v1__) || defined(__sparcv1)
#define SIMDE_ARCH_SPARC 1
#elif defined(__sparc__) || defined(__sparc)
#define SIMDE_ARCH_SPARC 1
#endif



#if defined(__sh5__) || defined(__SH5__)
#define SIMDE_ARCH_SUPERH 5
#elif defined(__sh4__) || defined(__SH4__)
#define SIMDE_ARCH_SUPERH 4
#elif defined(__sh3__) || defined(__SH3__)
#define SIMDE_ARCH_SUPERH 3
#elif defined(__sh2__) || defined(__SH2__)
#define SIMDE_ARCH_SUPERH 2
#elif defined(__sh1__) || defined(__SH1__)
#define SIMDE_ARCH_SUPERH 1
#elif defined(__sh__) || defined(__SH__)
#define SIMDE_ARCH_SUPERH 1
#endif



#if defined(__370__) || defined(__THW_370__) || defined(__s390__) || defined(__s390x__) || defined(__zarch__) || defined(__SYSC_ZARCH__)

#define SIMDE_ARCH_SYSTEMZ
#endif



#if defined(_TMS320C6740) || defined(__TMS320C6740__)
#define SIMDE_ARCH_TMS320 6740
#elif defined(_TMS320C6700_PLUS) || defined(__TMS320C6700_PLUS__)
#define SIMDE_ARCH_TMS320 6701
#elif defined(_TMS320C6700) || defined(__TMS320C6700__)
#define SIMDE_ARCH_TMS320 6700
#elif defined(_TMS320C6600) || defined(__TMS320C6600__)
#define SIMDE_ARCH_TMS320 6600
#elif defined(_TMS320C6400_PLUS) || defined(__TMS320C6400_PLUS__)
#define SIMDE_ARCH_TMS320 6401
#elif defined(_TMS320C6400) || defined(__TMS320C6400__)
#define SIMDE_ARCH_TMS320 6400
#elif defined(_TMS320C6200) || defined(__TMS320C6200__)
#define SIMDE_ARCH_TMS320 6200
#elif defined(_TMS320C55X) || defined(__TMS320C55X__)
#define SIMDE_ARCH_TMS320 550
#elif defined(_TMS320C54X) || defined(__TMS320C54X__)
#define SIMDE_ARCH_TMS320 540
#elif defined(_TMS320C28X) || defined(__TMS320C28X__)
#define SIMDE_ARCH_TMS320 280
#endif



#if defined(__xtensa__) || defined(__XTENSA__)
#define SIMDE_ARCH_XTENSA 1
#endif

#endif 
