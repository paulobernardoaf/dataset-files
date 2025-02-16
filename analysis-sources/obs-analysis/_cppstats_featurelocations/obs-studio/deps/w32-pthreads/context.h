



































#if !defined(PTW32_CONTEXT_H)
#define PTW32_CONTEXT_H

#undef PTW32_PROGCTR

#if defined(_M_IX86) || (defined(_X86_) && !defined(__amd64__))
#define PTW32_PROGCTR(Context) ((Context).Eip)
#endif

#if defined (_M_IA64) || defined(_IA64)
#define PTW32_PROGCTR(Context) ((Context).StIIP)
#endif

#if defined(_MIPS_) || defined(MIPS)
#define PTW32_PROGCTR(Context) ((Context).Fir)
#endif

#if defined(_ALPHA_)
#define PTW32_PROGCTR(Context) ((Context).Fir)
#endif

#if defined(_PPC_)
#define PTW32_PROGCTR(Context) ((Context).Iar)
#endif

#if defined(_AMD64_) || defined(__amd64__)
#define PTW32_PROGCTR(Context) ((Context).Rip)
#endif

#if defined(_ARM_) || defined(ARM)
#define PTW32_PROGCTR(Context) ((Context).Pc)
#endif

#if !defined(PTW32_PROGCTR)
#error Module contains CPU-specific code; modify and recompile.
#endif

#endif
