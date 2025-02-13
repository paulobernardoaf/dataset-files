#if defined(__cplusplus)
extern "C" {
#if 0
} 
#endif
#endif
NORETURN(void rb_assert_failure(const char *, int, const char *, const char *));
#if defined(RUBY_FUNCTION_NAME_STRING)
#define RUBY_ASSERT_FAIL(expr) rb_assert_failure(__FILE__, __LINE__, RUBY_FUNCTION_NAME_STRING, expr)
#else
#define RUBY_ASSERT_FAIL(expr) rb_assert_failure(__FILE__, __LINE__, NULL, expr)
#endif
#define RUBY_ASSERT_MESG(expr, mesg) ((expr) ? (void)0 : RUBY_ASSERT_FAIL(mesg))
#if defined(HAVE_BUILTIN___BUILTIN_CHOOSE_EXPR_CONSTANT_P)
#define RUBY_ASSERT_MESG_WHEN(cond, expr, mesg) ((RUBY_DEBUG+0) ? RUBY_ASSERT_MESG((expr), mesg) : __builtin_choose_expr( __builtin_constant_p(cond), __builtin_choose_expr(cond, RUBY_ASSERT_MESG(expr, mesg), (void)0), RUBY_ASSERT_MESG(!(cond) || (expr), mesg)))
#else
#define RUBY_ASSERT_MESG_WHEN(cond, expr, mesg) RUBY_ASSERT_MESG(!((RUBY_DEBUG+0) || (cond)) || (expr), mesg)
#endif
#define RUBY_ASSERT(expr) RUBY_ASSERT_MESG_WHEN((!RUBY_NDEBUG+0), expr, #expr)
#define RUBY_ASSERT_WHEN(cond, expr) RUBY_ASSERT_MESG_WHEN(cond, expr, #expr)
#define RUBY_ASSERT_ALWAYS(expr) RUBY_ASSERT_MESG_WHEN(TRUE, expr, #expr)
#if !defined(RUBY_DEBUG)
#define RUBY_DEBUG 0
#endif
#if !defined(RUBY_NDEBUG)
#if defined(NDEBUG)
#define RUBY_NDEBUG 1
#else
#define RUBY_NDEBUG 0
#endif
#endif
#if defined(__cplusplus)
#if 0
{ 
#endif
} 
#endif
