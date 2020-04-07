

















#if !defined(GRUB_SYMBOL_HEADER)
#define GRUB_SYMBOL_HEADER 1


#define LOCAL(sym) L_ ##sym


#if defined(HAVE_ASM_USCORE)
#define EXT_C(sym) _ ##sym
#else
#define EXT_C(sym) sym
#endif

#define FUNCTION(x) EXT_C(x): .globl EXT_C(x)
#define VARIABLE(x) EXT_C(x): .globl EXT_C(x)


#if !defined(GRUB_SYMBOL_GENERATOR)
#define EXPORT_FUNC(x) x
#define EXPORT_VAR(x) x
#endif 


#define GRUB_EXPORT_START

#define GRUB_MODATTR(name, value)

#define GRUB_EXPORT_END .text
#define GRUB_EXPORT(value) 

#endif 
