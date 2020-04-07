void
rb_sparc_flush_register_windows(void)
{
#if !defined(__GNUC__)
#define __asm__ asm
#endif
__asm__
#if defined(__GNUC__)
__volatile__
#endif
#if defined(__sparcv9) || defined(__sparc_v9__) || defined(__arch64__)
#if defined(__GNUC__)
("flushw" : : : "%o7")
#else
("flushw")
#endif 
#else
("ta 0x03")
#endif
;
}
