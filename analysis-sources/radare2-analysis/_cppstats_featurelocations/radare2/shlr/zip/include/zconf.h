






#if !defined(ZCONF_H)
#define ZCONF_H







#if defined(Z_PREFIX)
#define Z_PREFIX_SET


#define _dist_code z__dist_code
#define _length_code z__length_code
#define _tr_align z__tr_align
#define _tr_flush_bits z__tr_flush_bits
#define _tr_flush_block z__tr_flush_block
#define _tr_init z__tr_init
#define _tr_stored_block z__tr_stored_block
#define _tr_tally z__tr_tally
#define adler32 z_adler32
#define adler32_combine z_adler32_combine
#define adler32_combine64 z_adler32_combine64
#if !defined(Z_SOLO)
#define compress z_compress
#define compress2 z_compress2
#define compressBound z_compressBound
#endif
#define crc32 z_crc32
#define crc32_combine z_crc32_combine
#define crc32_combine64 z_crc32_combine64
#define deflate z_deflate
#define deflateBound z_deflateBound
#define deflateCopy z_deflateCopy
#define deflateEnd z_deflateEnd
#define deflateInit2_ z_deflateInit2_
#define deflateInit_ z_deflateInit_
#define deflateParams z_deflateParams
#define deflatePending z_deflatePending
#define deflatePrime z_deflatePrime
#define deflateReset z_deflateReset
#define deflateResetKeep z_deflateResetKeep
#define deflateSetDictionary z_deflateSetDictionary
#define deflateSetHeader z_deflateSetHeader
#define deflateTune z_deflateTune
#define deflate_copyright z_deflate_copyright
#define get_crc_table z_get_crc_table
#if !defined(Z_SOLO)
#define gz_error z_gz_error
#define gz_intmax z_gz_intmax
#define gz_strwinerror z_gz_strwinerror
#define gzbuffer z_gzbuffer
#define gzclearerr z_gzclearerr
#define gzclose z_gzclose
#define gzclose_r z_gzclose_r
#define gzclose_w z_gzclose_w
#define gzdirect z_gzdirect
#define gzdopen z_gzdopen
#define gzeof z_gzeof
#define gzerror z_gzerror
#define gzflush z_gzflush
#define gzgetc z_gzgetc
#define gzgetc_ z_gzgetc_
#define gzgets z_gzgets
#define gzoffset z_gzoffset
#define gzoffset64 z_gzoffset64
#define gzopen z_gzopen
#define gzopen64 z_gzopen64
#if defined(__WINDOWS__)
#define gzopen_w z_gzopen_w
#endif
#define gzprintf z_gzprintf
#define gzvprintf z_gzvprintf
#define gzputc z_gzputc
#define gzputs z_gzputs
#define gzread z_gzread
#define gzrewind z_gzrewind
#define gzseek z_gzseek
#define gzseek64 z_gzseek64
#define gzsetparams z_gzsetparams
#define gztell z_gztell
#define gztell64 z_gztell64
#define gzungetc z_gzungetc
#define gzwrite z_gzwrite
#endif
#define inflate z_inflate
#define inflateBack z_inflateBack
#define inflateBackEnd z_inflateBackEnd
#define inflateBackInit_ z_inflateBackInit_
#define inflateCopy z_inflateCopy
#define inflateEnd z_inflateEnd
#define inflateGetHeader z_inflateGetHeader
#define inflateInit2_ z_inflateInit2_
#define inflateInit_ z_inflateInit_
#define inflateMark z_inflateMark
#define inflatePrime z_inflatePrime
#define inflateReset z_inflateReset
#define inflateReset2 z_inflateReset2
#define inflateSetDictionary z_inflateSetDictionary
#define inflateGetDictionary z_inflateGetDictionary
#define inflateSync z_inflateSync
#define inflateSyncPoint z_inflateSyncPoint
#define inflateUndermine z_inflateUndermine
#define inflateResetKeep z_inflateResetKeep
#define inflate_copyright z_inflate_copyright
#define inflate_fast z_inflate_fast
#define inflate_table z_inflate_table
#if !defined(Z_SOLO)
#define uncompress z_uncompress
#endif
#define zError z_zError
#if !defined(Z_SOLO)
#define zcalloc z_zcalloc
#define zcfree z_zcfree
#endif
#define zlibCompileFlags z_zlibCompileFlags
#define zlibVersion z_zlibVersion


#define Byte z_Byte
#define Bytef z_Bytef
#define alloc_func z_alloc_func
#define charf z_charf
#define free_func z_free_func
#if !defined(Z_SOLO)
#define gzFile z_gzFile
#endif
#define gz_header z_gz_header
#define gz_headerp z_gz_headerp
#define in_func z_in_func
#define intf z_intf
#define out_func z_out_func
#define uInt z_uInt
#define uIntf z_uIntf
#define uLong z_uLong
#define uLongf z_uLongf
#define voidp z_voidp
#define voidpc z_voidpc
#define voidpf z_voidpf


#define gz_header_s z_gz_header_s
#define internal_state z_internal_state

#endif

#if defined(__MSDOS__) && !defined(MSDOS)
#define MSDOS
#endif
#if (defined(OS_2) || defined(__OS2__)) && !defined(OS2)
#define OS2
#endif
#if defined(_WINDOWS) && !defined(WINDOWS)
#define WINDOWS
#endif
#if defined(__WINDOWS__) || defined(_WIN32_WCE) || defined(__WIN32__)
#if !defined(WIN32)
#define WIN32
#endif
#endif
#if (defined(MSDOS) || defined(OS2) || defined(WINDOWS)) && !defined(WIN32)
#if !defined(__GNUC__) && !defined(__FLAT__) && !defined(__386__)
#if !defined(SYS16BIT)
#define SYS16BIT
#endif
#endif
#endif





#if defined(SYS16BIT)
#define MAXSEG_64K
#endif
#if defined(MSDOS)
#define UNALIGNED_OK
#endif

#if defined(__STDC_VERSION__)
#if !defined(STDC)
#define STDC
#endif
#if __STDC_VERSION__ >= 199901L
#if !defined(STDC99)
#define STDC99
#endif
#endif
#endif
#if !defined(STDC) && (defined(__STDC__) || defined(__cplusplus))
#define STDC
#endif
#if !defined(STDC) && (defined(__GNUC__) || defined(__BORLANDC__))
#define STDC
#endif
#if !defined(STDC) && (defined(MSDOS) || defined(WINDOWS) || defined(WIN32))
#define STDC
#endif
#if !defined(STDC) && (defined(OS2) || defined(__HOS_AIX__))
#define STDC
#endif

#if defined(__OS400__) && !defined(STDC) 
#define STDC
#endif

#if !defined(STDC)
#if !defined(const)
#define const 
#endif
#endif

#if defined(ZLIB_CONST) && !defined(z_const)
#define z_const const
#else
#define z_const
#endif


#if defined(__MWERKS__)||defined(applec)||defined(THINK_C)||defined(__SC__)
#define NO_DUMMY_DECL
#endif


#if !defined(MAX_MEM_LEVEL)
#if defined(MAXSEG_64K)
#define MAX_MEM_LEVEL 8
#else
#define MAX_MEM_LEVEL 9
#endif
#endif






#if !defined(MAX_WBITS)
#define MAX_WBITS 15 
#endif
















#if !defined(OF)
#if defined(STDC)
#define OF(args) args
#else
#define OF(args) ()
#endif
#endif

#if !defined(Z_ARG)
#if defined(STDC) || defined(Z_HAVE_STDARG_H)
#define Z_ARG(args) args
#else
#define Z_ARG(args) ()
#endif
#endif







#if defined(SYS16BIT)
#if defined(M_I86SM) || defined(M_I86MM)

#define SMALL_MEDIUM
#if defined(_MSC_VER)
#define FAR _far
#else
#define FAR far
#endif
#endif
#if (defined(__SMALL__) || defined(__MEDIUM__))

#define SMALL_MEDIUM
#if defined(__BORLANDC__)
#define FAR _far
#else
#define FAR far
#endif
#endif
#endif

#if defined(WINDOWS) || defined(WIN32)



#if defined(ZLIB_DLL)
#if defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#if defined(ZLIB_INTERNAL)
#define ZEXTERN extern __declspec(dllexport)
#else
#define ZEXTERN extern __declspec(dllimport)
#endif
#endif
#endif 




#if defined(ZLIB_WINAPI)
#if defined(FAR)
#undef FAR
#endif
#include <windows.h>


#define ZEXPORT WINAPI
#if defined(WIN32)
#define ZEXPORTVA WINAPIV
#else
#define ZEXPORTVA FAR CDECL
#endif
#endif
#endif

#if defined (__BEOS__)
#if defined(ZLIB_DLL)
#if defined(ZLIB_INTERNAL)
#define ZEXPORT __declspec(dllexport)
#define ZEXPORTVA __declspec(dllexport)
#else
#define ZEXPORT __declspec(dllimport)
#define ZEXPORTVA __declspec(dllimport)
#endif
#endif
#endif

#if !defined(ZEXTERN)
#define ZEXTERN extern
#endif
#if !defined(ZEXPORT)
#define ZEXPORT
#endif
#if !defined(ZEXPORTVA)
#define ZEXPORTVA
#endif

#if !defined(FAR)
#define FAR
#endif

#if !defined(__MACTYPES__)
typedef unsigned char Byte; 
#endif
typedef unsigned int uInt; 
typedef unsigned long uLong; 

#if defined(SMALL_MEDIUM)

#define Bytef Byte FAR
#else
typedef Byte FAR Bytef;
#endif
typedef char FAR charf;
typedef int FAR intf;
typedef uInt FAR uIntf;
typedef uLong FAR uLongf;

#if defined(STDC)
typedef void const *voidpc;
typedef void FAR *voidpf;
typedef void *voidp;
#else
typedef Byte const *voidpc;
typedef Byte FAR *voidpf;
typedef Byte *voidp;
#endif

#if !defined(Z_U4) && !defined(Z_SOLO) && defined(STDC)
#include <limits.h>
#if (UINT_MAX == 0xffffffffUL)
#define Z_U4 unsigned
#elif (ULONG_MAX == 0xffffffffUL)
#define Z_U4 unsigned long
#elif (USHRT_MAX == 0xffffffffUL)
#define Z_U4 unsigned short
#endif
#endif

#if defined(Z_U4)
typedef Z_U4 z_crc_t;
#else
typedef unsigned long z_crc_t;
#endif

#if defined(HAVE_UNISTD_H)
#define Z_HAVE_UNISTD_H
#endif

#if defined(HAVE_STDARG_H)
#define Z_HAVE_STDARG_H
#endif



#if defined(STDC) || defined(Z_HAVE_STDARG_H)
#if !defined(Z_SOLO)
#include <stdarg.h> 
#endif
#endif

#if defined(__WINDOWS__)
#if !defined(Z_SOLO)
#include <stddef.h> 
#endif
#endif







#if defined(_LARGEFILE64_SOURCE) && -_LARGEFILE64_SOURCE - -1 == 1
#undef _LARGEFILE64_SOURCE
#endif

#if defined(__WATCOMC__) && !defined(Z_HAVE_UNISTD_H)
#define Z_HAVE_UNISTD_H
#endif
#if !defined(Z_SOLO)
#if defined(Z_HAVE_UNISTD_H) || defined(_LARGEFILE64_SOURCE)
#include <unistd.h> 
#if defined(VMS)
#include <unixio.h> 
#endif
#if !defined(z_off_t)
#define z_off_t off_t
#endif
#endif
#endif

#if defined(_LFS64_LARGEFILE) && _LFS64_LARGEFILE-0
#define Z_LFS64
#endif

#if defined(_LARGEFILE64_SOURCE) && defined(Z_LFS64)
#define Z_LARGE64
#endif

#if defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS-0 == 64 && defined(Z_LFS64)
#define Z_WANT64
#endif

#if !defined(SEEK_SET) && !defined(Z_SOLO)
#define SEEK_SET 0 
#define SEEK_CUR 1 
#define SEEK_END 2 
#endif

#if !defined(z_off_t)
#define z_off_t long
#endif

#if !defined(__WINDOWS__) && defined(Z_LARGE64)
#define z_off64_t off64_t
#else
#if defined(__WINDOWS__) && !defined(__GNUC__) && !defined(Z_SOLO)
#define z_off64_t __int64
#else
#define z_off64_t z_off_t
#endif
#endif


#if defined(__MVS__)
#pragma map(deflateInit_,"DEIN")
#pragma map(deflateInit2_,"DEIN2")
#pragma map(deflateEnd,"DEEND")
#pragma map(deflateBound,"DEBND")
#pragma map(inflateInit_,"ININ")
#pragma map(inflateInit2_,"ININ2")
#pragma map(inflateEnd,"INEND")
#pragma map(inflateSync,"INSY")
#pragma map(inflateSetDictionary,"INSEDI")
#pragma map(compressBound,"CMBND")
#pragma map(inflate_table,"INTABL")
#pragma map(inflate_fast,"INFA")
#pragma map(inflate_copyright,"INCOPY")
#endif

#endif 
