#define RUBY_AUTHOR "Yukihiro Matsumoto"
#define RUBY_BIRTH_YEAR 1993
#define RUBY_BIRTH_MONTH 2
#define RUBY_BIRTH_DAY 24
#define RUBY_API_VERSION_MAJOR 2
#define RUBY_API_VERSION_MINOR 8
#define RUBY_API_VERSION_TEENY 0
#define RUBY_API_VERSION_CODE (RUBY_API_VERSION_MAJOR*10000+RUBY_API_VERSION_MINOR*100+RUBY_API_VERSION_TEENY)
#if defined(RUBY_EXTERN)
#if defined(__cplusplus)
extern "C" {
#if 0
} 
#endif
#endif
RUBY_SYMBOL_EXPORT_BEGIN
RUBY_EXTERN const int ruby_api_version[3];
RUBY_EXTERN const char ruby_version[];
RUBY_EXTERN const char ruby_release_date[];
RUBY_EXTERN const char ruby_platform[];
RUBY_EXTERN const int ruby_patchlevel;
RUBY_EXTERN const char ruby_description[];
RUBY_EXTERN const char ruby_copyright[];
RUBY_EXTERN const char ruby_engine[];
RUBY_SYMBOL_EXPORT_END
#if defined(__cplusplus)
#if 0
{ 
#endif
} 
#endif
#endif
