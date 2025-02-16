#if defined(JEMALLOC_H_TYPES)
typedef struct spin_s spin_t;
#endif 
#if defined(JEMALLOC_H_STRUCTS)
struct spin_s {
unsigned iteration;
};
#endif 
#if defined(JEMALLOC_H_EXTERNS)
#endif 
#if defined(JEMALLOC_H_INLINES)
#if !defined(JEMALLOC_ENABLE_INLINE)
void spin_init(spin_t *spin);
void spin_adaptive(spin_t *spin);
#endif
#if (defined(JEMALLOC_ENABLE_INLINE) || defined(JEMALLOC_SPIN_C_))
JEMALLOC_INLINE void
spin_init(spin_t *spin)
{
spin->iteration = 0;
}
JEMALLOC_INLINE void
spin_adaptive(spin_t *spin)
{
volatile uint64_t i;
for (i = 0; i < (KQU(1) << spin->iteration); i++)
CPU_SPINWAIT;
if (spin->iteration < 63)
spin->iteration++;
}
#endif
#endif 
