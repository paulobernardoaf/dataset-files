#if !defined(BANNED_H)
#define BANNED_H








#define BANNED(func) sorry_##func##_is_a_banned_function

#undef strcpy
#define strcpy(x,y) BANNED(strcpy)
#undef strcat
#define strcat(x,y) BANNED(strcat)
#undef strncpy
#define strncpy(x,y,n) BANNED(strncpy)
#undef strncat
#define strncat(x,y,n) BANNED(strncat)

#undef sprintf
#undef vsprintf
#if defined(HAVE_VARIADIC_MACROS)
#define sprintf(...) BANNED(sprintf)
#define vsprintf(...) BANNED(vsprintf)
#else
#define sprintf(buf,fmt,arg) BANNED(sprintf)
#define vsprintf(buf,fmt,arg) BANNED(vsprintf)
#endif

#endif 
