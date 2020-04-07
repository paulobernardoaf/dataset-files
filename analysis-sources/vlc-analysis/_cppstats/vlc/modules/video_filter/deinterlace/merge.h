#define Merge p_sys->pf_merge
#if defined(__i386__) || defined(__x86_64__)
#define EndMerge() if(p_sys->pf_end_merge) (p_sys->pf_end_merge)()
#else
#define EndMerge() (void)0
#endif
void Merge8BitGeneric( void *_p_dest, const void *_p_s1, const void *_p_s2,
size_t i_bytes );
void Merge16BitGeneric( void *_p_dest, const void *_p_s1, const void *_p_s2,
size_t i_bytes );
#if defined(CAN_COMPILE_C_ALTIVEC)
void MergeAltivec ( void *, const void *, const void *, size_t );
#endif
#if defined(CAN_COMPILE_MMXEXT)
void MergeMMXEXT ( void *, const void *, const void *, size_t );
#endif
#if defined(CAN_COMPILE_3DNOW)
void Merge3DNow ( void *, const void *, const void *, size_t );
#endif
#if defined(CAN_COMPILE_SSE)
void Merge8BitSSE2( void *, const void *, const void *, size_t );
void Merge16BitSSE2( void *, const void *, const void *, size_t );
#endif
#if defined(CAN_COMPILE_ARM)
void merge8_arm_neon (void *, const void *, const void *, size_t);
void merge16_arm_neon (void *, const void *, const void *, size_t);
void merge8_armv6 (void *, const void *, const void *, size_t);
void merge16_armv6 (void *, const void *, const void *, size_t);
#endif
#if defined(CAN_COMPILE_ARM64)
void merge8_arm64_neon (void *, const void *, const void *, size_t);
void merge16_arm64_neon (void *, const void *, const void *, size_t);
#endif
void merge8_arm_sve(void *, const void *, const void *, size_t);
void merge16_arm_sve(void *, const void *, const void *, size_t);
#if defined(CAN_COMPILE_MMXEXT) || defined(CAN_COMPILE_SSE)
void EndMMX ( void );
#endif
#if defined(CAN_COMPILE_3DNOW)
void End3DNow ( void );
#endif
