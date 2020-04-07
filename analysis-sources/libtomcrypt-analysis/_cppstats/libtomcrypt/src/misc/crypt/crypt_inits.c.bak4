







#include "tomcrypt_private.h"









#if defined(LTM_DESC)
void init_LTM(void)
{
ltc_mp = ltm_desc;
}
#endif

#if defined(TFM_DESC)
void init_TFM(void)
{
ltc_mp = tfm_desc;
}
#endif

#if defined(GMP_DESC)
void init_GMP(void)
{
ltc_mp = gmp_desc;
}
#endif

int crypt_mp_init(const char* mpi)
{
if (mpi == NULL) return CRYPT_ERROR;
switch (mpi[0]) {
#if defined(LTM_DESC)
case 'l':
case 'L':
ltc_mp = ltm_desc;
return CRYPT_OK;
#endif
#if defined(TFM_DESC)
case 't':
case 'T':
ltc_mp = tfm_desc;
return CRYPT_OK;
#endif
#if defined(GMP_DESC)
case 'g':
case 'G':
ltc_mp = gmp_desc;
return CRYPT_OK;
#endif
#if defined(EXT_MATH_LIB)
case 'e':
case 'E':
{
extern ltc_math_descriptor EXT_MATH_LIB;
ltc_mp = EXT_MATH_LIB;
}

#if defined(LTC_TEST_DBG)
#define NAME_VALUE(s) #s"="NAME(s)
#define NAME(s) #s
printf("EXT_MATH_LIB = %s\n", NAME_VALUE(EXT_MATH_LIB));
#undef NAME_VALUE
#undef NAME
#endif

return CRYPT_OK;
#endif
default:
#if defined(LTC_TEST_DBG)
printf("Unknown/Invalid MPI provider: %s\n", mpi);
#endif
return CRYPT_ERROR;
}
}





