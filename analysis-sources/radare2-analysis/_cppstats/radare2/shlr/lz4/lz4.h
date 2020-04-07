#if defined (__cplusplus)
extern "C" {
#endif
#if !defined(LZ4_H_2983827168210)
#define LZ4_H_2983827168210
#include <r_types.h>
#include <stddef.h> 
#if !defined(LZ4LIB_VISIBILITY)
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define LZ4LIB_VISIBILITY __attribute__ ((visibility ("default")))
#else
#define LZ4LIB_VISIBILITY
#endif
#endif
#if defined(LZ4_DLL_EXPORT) && (LZ4_DLL_EXPORT==1)
#define LZ4LIB_API __declspec(dllexport) LZ4LIB_VISIBILITY
#elif defined(LZ4_DLL_IMPORT) && (LZ4_DLL_IMPORT==1)
#define LZ4LIB_API __declspec(dllimport) LZ4LIB_VISIBILITY 
#else
#define LZ4LIB_API LZ4LIB_VISIBILITY
#endif
#define LZ4_VERSION_MAJOR 1 
#define LZ4_VERSION_MINOR 8 
#define LZ4_VERSION_RELEASE 0 
#define LZ4_VERSION_NUMBER (LZ4_VERSION_MAJOR *100*100 + LZ4_VERSION_MINOR *100 + LZ4_VERSION_RELEASE)
#define LZ4_LIB_VERSION LZ4_VERSION_MAJOR.LZ4_VERSION_MINOR.LZ4_VERSION_RELEASE
#define LZ4_QUOTE(str) #str
#define LZ4_EXPAND_AND_QUOTE(str) LZ4_QUOTE(str)
#define LZ4_VERSION_STRING LZ4_EXPAND_AND_QUOTE(LZ4_LIB_VERSION)
LZ4LIB_API int LZ4_versionNumber (void); 
LZ4LIB_API const char* LZ4_versionString (void); 
#if !defined(LZ4_MEMORY_USAGE)
#define LZ4_MEMORY_USAGE 14
#endif
LZ4LIB_API int LZ4_compress_default(const char* source, char* dest, int sourceSize, int maxDestSize);
LZ4LIB_API int LZ4_decompress_safe (const char* source, char* dest, int compressedSize, int maxDecompressedSize);
#define LZ4_MAX_INPUT_SIZE 0x7E000000 
#define LZ4_COMPRESSBOUND(isize) ((unsigned)(isize) > (unsigned)LZ4_MAX_INPUT_SIZE ? 0 : (isize) + ((isize)/255) + 16)
LZ4LIB_API int LZ4_compressBound(int inputSize);
LZ4LIB_API int LZ4_compress_fast (const char* source, char* dest, int sourceSize, int maxDestSize, int acceleration);
LZ4LIB_API int LZ4_sizeofState(void);
LZ4LIB_API int LZ4_compress_fast_extState (void* state, const char* source, char* dest, int inputSize, int maxDestSize, int acceleration);
LZ4LIB_API int LZ4_compress_destSize (const char* source, char* dest, int* sourceSizePtr, int targetDestSize);
LZ4LIB_API int LZ4_decompress_fast (const char* source, char* dest, int originalSize);
LZ4LIB_API int LZ4_decompress_safe_partial (const char* source, char* dest, int compressedSize, int targetOutputSize, int maxDecompressedSize);
typedef union LZ4_stream_u LZ4_stream_t; 
LZ4LIB_API LZ4_stream_t* LZ4_createStream(void);
LZ4LIB_API void LZ4_freeStream (LZ4_stream_t* streamPtr);
LZ4LIB_API void LZ4_resetStream (LZ4_stream_t* streamPtr);
LZ4LIB_API int LZ4_loadDict (LZ4_stream_t* streamPtr, const char* dictionary, int dictSize);
LZ4LIB_API int LZ4_compress_fast_continue (LZ4_stream_t* streamPtr, const char* src, char* dst, int srcSize, int dstCapacity, int acceleration);
LZ4LIB_API int LZ4_saveDict (LZ4_stream_t* streamPtr, char* safeBuffer, int dictSize);
typedef union LZ4_streamDecode_u LZ4_streamDecode_t; 
LZ4LIB_API LZ4_streamDecode_t* LZ4_createStreamDecode(void);
LZ4LIB_API void LZ4_freeStreamDecode (LZ4_streamDecode_t* LZ4_stream);
LZ4LIB_API int LZ4_setStreamDecode (LZ4_streamDecode_t* LZ4_streamDecode, const char* dictionary, int dictSize);
LZ4LIB_API int LZ4_decompress_safe_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int compressedSize, int maxDecompressedSize);
LZ4LIB_API int LZ4_decompress_fast_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int originalSize);
LZ4LIB_API int LZ4_decompress_safe_usingDict (const char* source, char* dest, int compressedSize, int maxDecompressedSize, const char* dictStart, int dictSize);
LZ4LIB_API int LZ4_decompress_fast_usingDict (const char* source, char* dest, int originalSize, const char* dictStart, int dictSize);
#define LZ4_HASHLOG (LZ4_MEMORY_USAGE-2)
#define LZ4_HASHTABLESIZE (1 << LZ4_MEMORY_USAGE)
#define LZ4_HASH_SIZE_U32 (1 << LZ4_HASHLOG) 
#if defined(__cplusplus) || (defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) )
#include <stdint.h>
typedef struct {
uint32_t hashTable[LZ4_HASH_SIZE_U32];
uint32_t currentOffset;
uint32_t initCheck;
const uint8_t* dictionary;
uint8_t* bufferStart; 
uint32_t dictSize;
} LZ4_stream_t_internal;
typedef struct {
const uint8_t* externalDict;
size_t extDictSize;
const uint8_t* prefixEnd;
size_t prefixSize;
} LZ4_streamDecode_t_internal;
#else
typedef struct {
unsigned int hashTable[LZ4_HASH_SIZE_U32];
unsigned int currentOffset;
unsigned int initCheck;
const unsigned char* dictionary;
unsigned char* bufferStart; 
unsigned int dictSize;
} LZ4_stream_t_internal;
typedef struct {
const unsigned char* externalDict;
size_t extDictSize;
const unsigned char* prefixEnd;
size_t prefixSize;
} LZ4_streamDecode_t_internal;
#endif
#define LZ4_STREAMSIZE_U64 ((1 << (LZ4_MEMORY_USAGE-3)) + 4)
#define LZ4_STREAMSIZE (LZ4_STREAMSIZE_U64 * sizeof(unsigned long long))
union LZ4_stream_u {
unsigned long long table[LZ4_STREAMSIZE_U64];
LZ4_stream_t_internal internal_donotuse;
} ; 
#define LZ4_STREAMDECODESIZE_U64 4
#define LZ4_STREAMDECODESIZE (LZ4_STREAMDECODESIZE_U64 * sizeof(unsigned long long))
union LZ4_streamDecode_u {
unsigned long long table[LZ4_STREAMDECODESIZE_U64];
LZ4_streamDecode_t_internal internal_donotuse;
} ; 
#if defined(LZ4_DISABLE_DEPRECATE_WARNINGS)
#define LZ4_DEPRECATED(message) 
#else
#define LZ4_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#if defined(__clang__) 
#define LZ4_DEPRECATED(message) __attribute__((deprecated(message)))
#elif defined (__cplusplus) && (__cplusplus >= 201402) 
#define LZ4_DEPRECATED(message) [[deprecated(message)]]
#elif (LZ4_GCC_VERSION >= 405)
#define LZ4_DEPRECATED(message) __attribute__((deprecated(message)))
#elif (LZ4_GCC_VERSION >= 301)
#define LZ4_DEPRECATED(message) __attribute__((deprecated))
#elif defined(_MSC_VER)
#define LZ4_DEPRECATED(message) __declspec(deprecated(message))
#else
#pragma message("WARNING: You need to implement LZ4_DEPRECATED for this compiler")
#define LZ4_DEPRECATED(message)
#endif
#endif 
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_default() instead") int LZ4_compress (const char* source, char* dest, int sourceSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_default() instead") int LZ4_compress_limitedOutput (const char* source, char* dest, int sourceSize, int maxOutputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_fast_extState() instead") int LZ4_compress_withState (void* state, const char* source, char* dest, int inputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_fast_extState() instead") int LZ4_compress_limitedOutput_withState (void* state, const char* source, char* dest, int inputSize, int maxOutputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_fast_continue() instead") int LZ4_compress_continue (LZ4_stream_t* LZ4_streamPtr, const char* source, char* dest, int inputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_fast_continue() instead") int LZ4_compress_limitedOutput_continue (LZ4_stream_t* LZ4_streamPtr, const char* source, char* dest, int inputSize, int maxOutputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_decompress_fast() instead") int LZ4_uncompress (const char* source, char* dest, int outputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_decompress_safe() instead") int LZ4_uncompress_unknownOutputSize (const char* source, char* dest, int isize, int maxOutputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_createStream() instead") void* LZ4_create (char* inputBuffer);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_createStream() instead") int LZ4_sizeofStreamState(void);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_resetStream() instead") int LZ4_resetStreamState(void* state, char* inputBuffer);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_saveDict() instead") char* LZ4_slideInputBuffer (void* state);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_decompress_safe_usingDict() instead") int LZ4_decompress_safe_withPrefix64k (const char* src, char* dst, int compressedSize, int maxDstSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_decompress_fast_usingDict() instead") int LZ4_decompress_fast_withPrefix64k (const char* src, char* dst, int originalSize);
#endif 
#if defined (__cplusplus)
}
#endif
