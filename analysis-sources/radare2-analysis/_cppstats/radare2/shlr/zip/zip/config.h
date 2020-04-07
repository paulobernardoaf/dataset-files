#if !defined(fseeko)
#define fseeko fseek
#endif
#define HAVE_DLFCN_H 1
#define HAVE_FSEEKO 1
#define HAVE_FTELLO 1
#define HAVE_GETOPT 1
#define HAVE_INT16_T 1
#define HAVE_INT32_T 1
#define HAVE_INT64_T 1
#define HAVE_INT8_T 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIBZ 0 
#define HAVE_MEMORY_H 1
#define HAVE_MKSTEMP 1
#define HAVE_OPEN 1
#define HAVE_SNPRINTF 1
#if !defined(_MSC_VER)
#define HAVE_SSIZE_T 1
#endif
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRCASECMP 1
#define HAVE_STRDUP 1
#if !defined(_MSC_VER)
#define HAVE_STRINGS_H 1
#endif
#define HAVE_STRING_H 1
#undef HAVE_STRUCT_TM_TM_ZONE
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_TM_ZONE 1
#define HAVE_UINT16_T 1
#define HAVE_UINT32_T 1
#define HAVE_UINT64_T 1
#define HAVE_UINT8_T 1
#if defined(_MSC_VER)
#define HAVE_UNISTD_H 0
#else
#define HAVE_UNISTD_H 1
#endif
#define LT_OBJDIR ".libs/"
#define PACKAGE "libzip"
#define PACKAGE_BUGREPORT "libzip@nih.at"
#define PACKAGE_NAME "libzip"
#define PACKAGE_STRING "libzip 0.11.1"
#define PACKAGE_TARNAME "libzip"
#define PACKAGE_URL ""
#define PACKAGE_VERSION "0.11.1"
#define SIZEOF_INT 4
#define SIZEOF_LONG 8
#define SIZEOF_LONG_LONG 8
#define SIZEOF_OFF_T 8
#define SIZEOF_SHORT 2
#define SIZEOF_SIZE_T 8
#define STDC_HEADERS 1
#define VERSION "0.11.1"
#if !defined(HAVE_SSIZE_T)
#if SIZEOF_SIZE_T == SIZEOF_INT
typedef int ssize_t;
#elif SIZEOF_SIZE_T == SIZEOF_LONG
typedef long ssize_t;
#elif SIZEOF_SIZE_T == SIZEOF_LONG_LONG
typedef long long ssize_t;
#else
#error no suitable type for ssize_t found
#endif
#endif
