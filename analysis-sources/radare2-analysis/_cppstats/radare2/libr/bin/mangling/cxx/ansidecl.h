#if defined(__cplusplus)
extern "C" {
#endif
#if !defined(GCC_VERSION)
#define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#endif 
#if defined (__STDC__) || defined(__cplusplus) || defined (_AIX) || (defined (__mips) && defined (_SYSTYPE_SVR4)) || defined(__WINDOWS__)
#define ANSI_PROTOTYPES 1
#define PTR void *
#define PTRCONST void *const
#define LONG_DOUBLE long double
#if !defined(PARAMS)
#define PARAMS(ARGS) ARGS
#endif
#define VPARAMS(ARGS) ARGS
#define VA_START(VA_LIST, VAR) va_start(VA_LIST, VAR)
#define VA_OPEN(AP, VAR) { va_list AP; va_start(AP, VAR); { struct Qdmy
#define VA_CLOSE(AP) } va_end(AP); }
#define VA_FIXEDARG(AP, T, N) struct Qdmy
#undef const
#undef volatile
#undef signed
#undef inline
#if __STDC_VERSION__ >= 199901L || defined(__cplusplus) || (defined(__SUNPRO_C) && defined(__C99FEATURES__))
#else
#if GCC_VERSION >= 2007
#define inline __inline__ 
#else
#define inline 
#endif
#endif
#if !defined(IN_GCC)
#define CONST const
#define VOLATILE volatile
#define SIGNED signed
#define PROTO(type, name, arglist) type name arglist
#define EXFUN(name, proto) name proto
#define DEFUN(name, arglist, args) name(args)
#define DEFUN_VOID(name) name(void)
#define AND ,
#define DOTS , ...
#define NOARGS void
#endif 
#else 
#undef ANSI_PROTOTYPES
#define PTR char *
#define PTRCONST PTR
#define LONG_DOUBLE double
#define PARAMS(args) ()
#define VPARAMS(args) (va_alist) va_dcl
#define VA_START(va_list, var) va_start(va_list)
#define VA_OPEN(AP, VAR) { va_list AP; va_start(AP); { struct Qdmy
#define VA_CLOSE(AP) } va_end(AP); }
#define VA_FIXEDARG(AP, TYPE, NAME) TYPE NAME = va_arg(AP, TYPE)
#undef const
#undef volatile
#undef signed
#undef inline
#define const
#define volatile
#define signed
#define inline
#if !defined(IN_GCC)
#define CONST
#define VOLATILE
#define SIGNED
#define PROTO(type, name, arglist) type name ()
#define EXFUN(name, proto) name()
#define DEFUN(name, arglist, args) name arglist args;
#define DEFUN_VOID(name) name()
#define AND ;
#define DOTS
#define NOARGS
#endif 
#endif 
#if (GCC_VERSION < 2007)
#define __attribute__(x)
#endif
#if !defined(ATTRIBUTE_MALLOC)
#if (GCC_VERSION >= 2096)
#define ATTRIBUTE_MALLOC __attribute__ ((__malloc__))
#else
#define ATTRIBUTE_MALLOC
#endif 
#endif 
#if !defined(ATTRIBUTE_UNUSED_LABEL)
#if !defined(__cplusplus)
#if GCC_VERSION >= 2093
#define ATTRIBUTE_UNUSED_LABEL ATTRIBUTE_UNUSED
#else
#define ATTRIBUTE_UNUSED_LABEL
#endif
#else
#if GCC_VERSION >= 4005
#define ATTRIBUTE_UNUSED_LABEL ATTRIBUTE_UNUSED ;
#else
#define ATTRIBUTE_UNUSED_LABEL
#endif
#endif
#endif
#if !defined(ATTRIBUTE_UNUSED)
#define ATTRIBUTE_UNUSED __attribute__ ((__unused__))
#endif 
#if ! defined(__cplusplus) || (GCC_VERSION >= 3004)
#define ARG_UNUSED(NAME) NAME ATTRIBUTE_UNUSED
#else 
#define ARG_UNUSED(NAME) NAME
#endif 
#if !defined(ATTRIBUTE_NORETURN)
#define ATTRIBUTE_NORETURN __attribute__ ((__noreturn__))
#endif 
#if !defined(ATTRIBUTE_NONNULL)
#if (GCC_VERSION >= 3003)
#define ATTRIBUTE_NONNULL(m) __attribute__ ((__nonnull__ (m)))
#else
#define ATTRIBUTE_NONNULL(m)
#endif 
#endif 
#if !defined(ATTRIBUTE_PURE)
#if (GCC_VERSION >= 3000)
#define ATTRIBUTE_PURE __attribute__ ((__pure__))
#else
#define ATTRIBUTE_PURE
#endif 
#endif 
#if !defined(ATTRIBUTE_PRINTF)
#define ATTRIBUTE_PRINTF(m, n) __attribute__ ((__format__ (__printf__, m, n))) ATTRIBUTE_NONNULL(m)
#define ATTRIBUTE_PRINTF_1 ATTRIBUTE_PRINTF(1, 2)
#define ATTRIBUTE_PRINTF_2 ATTRIBUTE_PRINTF(2, 3)
#define ATTRIBUTE_PRINTF_3 ATTRIBUTE_PRINTF(3, 4)
#define ATTRIBUTE_PRINTF_4 ATTRIBUTE_PRINTF(4, 5)
#define ATTRIBUTE_PRINTF_5 ATTRIBUTE_PRINTF(5, 6)
#endif 
#if !defined(ATTRIBUTE_FPTR_PRINTF)
#if (GCC_VERSION >= 3001)
#define ATTRIBUTE_FPTR_PRINTF(m, n) ATTRIBUTE_PRINTF(m, n)
#else
#define ATTRIBUTE_FPTR_PRINTF(m, n)
#endif 
#define ATTRIBUTE_FPTR_PRINTF_1 ATTRIBUTE_FPTR_PRINTF(1, 2)
#define ATTRIBUTE_FPTR_PRINTF_2 ATTRIBUTE_FPTR_PRINTF(2, 3)
#define ATTRIBUTE_FPTR_PRINTF_3 ATTRIBUTE_FPTR_PRINTF(3, 4)
#define ATTRIBUTE_FPTR_PRINTF_4 ATTRIBUTE_FPTR_PRINTF(4, 5)
#define ATTRIBUTE_FPTR_PRINTF_5 ATTRIBUTE_FPTR_PRINTF(5, 6)
#endif 
#if !defined(ATTRIBUTE_NULL_PRINTF)
#if (GCC_VERSION >= 3003)
#define ATTRIBUTE_NULL_PRINTF(m, n) __attribute__ ((__format__ (__printf__, m, n)))
#else
#define ATTRIBUTE_NULL_PRINTF(m, n)
#endif 
#define ATTRIBUTE_NULL_PRINTF_1 ATTRIBUTE_NULL_PRINTF(1, 2)
#define ATTRIBUTE_NULL_PRINTF_2 ATTRIBUTE_NULL_PRINTF(2, 3)
#define ATTRIBUTE_NULL_PRINTF_3 ATTRIBUTE_NULL_PRINTF(3, 4)
#define ATTRIBUTE_NULL_PRINTF_4 ATTRIBUTE_NULL_PRINTF(4, 5)
#define ATTRIBUTE_NULL_PRINTF_5 ATTRIBUTE_NULL_PRINTF(5, 6)
#endif 
#if !defined(ATTRIBUTE_SENTINEL)
#if (GCC_VERSION >= 3005)
#define ATTRIBUTE_SENTINEL __attribute__ ((__sentinel__))
#else
#define ATTRIBUTE_SENTINEL
#endif 
#endif 
#if !defined(ATTRIBUTE_ALIGNED_ALIGNOF)
#if (GCC_VERSION >= 3000)
#define ATTRIBUTE_ALIGNED_ALIGNOF(m) __attribute__ ((__aligned__ (__alignof__ (m))))
#else
#define ATTRIBUTE_ALIGNED_ALIGNOF(m)
#endif 
#endif 
#if !defined(ATTRIBUTE_PACKED)
#define ATTRIBUTE_PACKED __attribute__ ((packed))
#endif
#if !defined(ATTRIBUTE_COLD)
#if (GCC_VERSION >= 4003)
#define ATTRIBUTE_COLD __attribute__ ((__cold__))
#else
#define ATTRIBUTE_COLD
#endif 
#endif 
#if !defined(ATTRIBUTE_HOT)
#if (GCC_VERSION >= 4003)
#define ATTRIBUTE_HOT __attribute__ ((__hot__))
#else
#define ATTRIBUTE_HOT
#endif 
#endif 
#if GCC_VERSION < 2008
#define __extension__
#endif
#if defined(__cplusplus)
#define EXPORTED_CONST extern const
#else
#define EXPORTED_CONST const
#endif
#if defined(__cplusplus)
}
#endif
