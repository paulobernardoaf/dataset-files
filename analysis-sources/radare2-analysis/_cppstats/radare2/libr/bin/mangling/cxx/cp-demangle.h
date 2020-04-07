struct demangle_operator_info
{
const char *code;
const char *name;
int len;
int args;
};
enum d_builtin_type_print
{
D_PRINT_DEFAULT,
D_PRINT_INT,
D_PRINT_UNSIGNED,
D_PRINT_LONG,
D_PRINT_UNSIGNED_LONG,
D_PRINT_LONG_LONG,
D_PRINT_UNSIGNED_LONG_LONG,
D_PRINT_BOOL,
D_PRINT_FLOAT,
D_PRINT_VOID
};
struct demangle_builtin_type_info
{
const char *name;
int len;
const char *java_name;
int java_len;
enum d_builtin_type_print print;
};
struct d_info
{
const char *s;
const char *send;
int options;
const char *n;
struct demangle_component *comps;
int next_comp;
int num_comps;
struct demangle_component **subs;
int next_sub;
int num_subs;
struct demangle_component *last_name;
int expansion;
int is_expression;
int is_conversion;
};
#define d_peek_char(di) (*((di)->n))
#if !defined(CHECK_DEMANGLER)
#define d_peek_next_char(di) ((di)->n[1])
#define d_advance(di, i) ((di)->n += (i))
#endif
#define d_check_char(di, c) (d_peek_char(di) == c ? ((di)->n++, 1) : 0)
#define d_next_char(di) (d_peek_char(di) == '\0' ? '\0' : *((di)->n++))
#define d_str(di) ((di)->n)
#if defined(CHECK_DEMANGLER)
static inline char
d_peek_next_char (const struct d_info *di)
{
if (!di->n[0])
abort ();
return di->n[1];
}
static inline void
d_advance (struct d_info *di, int i)
{
if (i < 0)
abort ();
while (i--)
{
if (!di->n[0])
abort ();
di->n++;
}
}
#endif
#if defined(IN_GLIBCPP_V3)
#define CP_STATIC_IF_GLIBCPP_V3 static
#else
#define CP_STATIC_IF_GLIBCPP_V3 extern
#endif
#if !defined(IN_GLIBCPP_V3)
extern const struct demangle_operator_info cplus_demangle_operators[];
#endif
#define D_BUILTIN_TYPE_COUNT (33)
CP_STATIC_IF_GLIBCPP_V3
const struct demangle_builtin_type_info
cplus_demangle_builtin_types[D_BUILTIN_TYPE_COUNT];
CP_STATIC_IF_GLIBCPP_V3
struct demangle_component *
cplus_demangle_mangled_name (struct d_info *, int);
CP_STATIC_IF_GLIBCPP_V3
struct demangle_component *
cplus_demangle_type (struct d_info *);
extern void
cplus_demangle_init_info (const char *, int, size_t, struct d_info *);
#undef CP_STATIC_IF_GLIBCPP_V3
