#if !defined(NVIM_MACROS_H)
#define NVIM_MACROS_H



#if !defined(EXTERN)
#define EXTERN extern
#define INIT(...)
#else
#if !defined(INIT)
#define INIT(...) __VA_ARGS__
#define COMMA ,
#endif
#endif

#if !defined(MIN)
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif
#if !defined(MAX)
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif








#define S_LEN(s) (s), (sizeof(s) - 1)


#define LINEEMPTY(p) (*ml_get(p) == NUL)


#define BUFEMPTY() (curbuf->b_ml.ml_line_count == 1 && *ml_get((linenr_T)1) == NUL)









#define TOUPPER_LOC toupper
#define TOLOWER_LOC tolower


#define TOUPPER_ASC(c) (((c) < 'a' || (c) > 'z') ? (c) : (c) - ('a' - 'A'))
#define TOLOWER_ASC(c) (((c) < 'A' || (c) > 'Z') ? (c) : (c) + ('a' - 'A'))



#define ASCII_ISLOWER(c) ((unsigned)(c) >= 'a' && (unsigned)(c) <= 'z')
#define ASCII_ISUPPER(c) ((unsigned)(c) >= 'A' && (unsigned)(c) <= 'Z')
#define ASCII_ISALPHA(c) (ASCII_ISUPPER(c) || ASCII_ISLOWER(c))
#define ASCII_ISALNUM(c) (ASCII_ISALPHA(c) || ascii_isdigit(c))


#define EMPTY_IF_NULL(x) ((x) ? (x) : (char_u *)"")









#define LANGMAP_ADJUST(c, condition) do { if (*p_langmap && (condition) && (p_lrm || (vgetc_busy ? typebuf_maplen() == 0 : KeyTyped)) && !KeyStuffed && (c) >= 0) { if ((c) < 256) c = langmap_mapchar[c]; else c = langmap_adjust_mb(c); } } while (0)














#define WRITEBIN "wb" 
#define READBIN "rb"
#define APPENDBIN "ab"


#if defined(UNIX) 
#define mch_open_rw(n, f) os_open((n), (f), (mode_t)0600)
#elif defined(WIN32)
#define mch_open_rw(n, f) os_open((n), (f), S_IREAD | S_IWRITE)
#else
#define mch_open_rw(n, f) os_open((n), (f), 0)
#endif

#define REPLACE_NORMAL(s) (((s) & REPLACE_FLAG) && !((s) & VREPLACE_FLAG))

#define UTF_COMPOSINGLIKE(p1, p2) utf_composinglike((p1), (p2))


#define CURSOR_BAR_RIGHT (curwin->w_p_rl && (!(State & CMDLINE) || cmdmsg_rl))









#define MB_PTR_ADV(p) (p += mb_ptr2len((char_u *)p))

#define MB_CPTR_ADV(p) (p += utf_ptr2len(p))

#define MB_PTR_BACK(s, p) (p -= utf_head_off((char_u *)s, (char_u *)p - 1) + 1)


#define MB_CPTR2LEN(p) utf_ptr2len(p)

#define MB_COPY_CHAR(f, t) mb_copy_char((const char_u **)(&f), &t);

#define MB_CHARLEN(p) mb_charlen(p)
#define MB_CHAR2LEN(c) mb_char2len(c)
#define PTR2CHAR(p) utf_ptr2char(p)

#define RESET_BINDING(wp) do { (wp)->w_p_scb = false; (wp)->w_p_crb = false; } while (0)











#define ARRAY_SIZE(arr) ((sizeof(arr)/sizeof((arr)[0])) / ((size_t)(!(sizeof(arr) % sizeof((arr)[0])))))







#define ARRAY_LAST_ENTRY(arr) (arr)[ARRAY_SIZE(arr) - 1]


#define RGB_(r, g, b) ((r << 16) | (g << 8) | b)

#define STR_(x) #x
#define STR(x) STR_(x)

#if !defined(__has_attribute)
#define NVIM_HAS_ATTRIBUTE(x) 0
#elif defined(__clang__) && __clang__ == 1 && (__clang_major__ < 3 || (__clang_major__ == 3 && __clang_minor__ <= 5))




#define NVIM_HAS_ATTRIBUTE(x) 0
#else
#define NVIM_HAS_ATTRIBUTE __has_attribute
#endif

#if NVIM_HAS_ATTRIBUTE(fallthrough) && (!defined(__apple_build_version__) || __apple_build_version__ >= 7000000)

#define FALLTHROUGH __attribute__((fallthrough))
#else
#define FALLTHROUGH
#endif











#define STRUCT_CAST(Type, obj) ((Type *)(obj))



#if defined(WIN32)
#define UV_BUF_LEN(x) (ULONG)(x)
#else
#define UV_BUF_LEN(x) (x)
#endif


#if defined(WIN32)
#define IO_COUNT(x) (unsigned)(x)
#else
#define IO_COUNT(x) (x)
#endif




#if defined(__clang__) && __clang__ == 1
#define PRAGMA_DIAG_PUSH_IGNORE_MISSING_PROTOTYPES _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wmissing-prototypes\"")


#define PRAGMA_DIAG_POP _Pragma("clang diagnostic pop")

#elif defined(__GNUC__)
#define PRAGMA_DIAG_PUSH_IGNORE_MISSING_PROTOTYPES _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wmissing-prototypes\"")


#define PRAGMA_DIAG_POP _Pragma("GCC diagnostic pop")

#else
#define PRAGMA_DIAG_PUSH_IGNORE_MISSING_PROTOTYPES
#define PRAGMA_DIAG_POP
#endif


#endif 
