#if defined(__cplusplus)
extern "C" {
#endif
#include "api.h"
#include "nsis_tchar.h"
#if !defined(NSISCALL)
#define NSISCALL __stdcall
#endif
#define EXDLL_INIT() { g_stringsize=string_size; g_stacktop=stacktop; g_variables=variables; }
typedef struct _stack_t {
struct _stack_t *next;
TCHAR text[1]; 
} stack_t;
enum
{
INST_0, 
INST_1, 
INST_2, 
INST_3, 
INST_4, 
INST_5, 
INST_6, 
INST_7, 
INST_8, 
INST_9, 
INST_R0, 
INST_R1, 
INST_R2, 
INST_R3, 
INST_R4, 
INST_R5, 
INST_R6, 
INST_R7, 
INST_R8, 
INST_R9, 
INST_CMDLINE, 
INST_INSTDIR, 
INST_OUTDIR, 
INST_EXEDIR, 
INST_LANG, 
__INST_LAST
};
extern unsigned int g_stringsize;
extern stack_t **g_stacktop;
extern TCHAR *g_variables;
int NSISCALL popstring(TCHAR *str); 
int NSISCALL popstringn(TCHAR *str, int maxlen); 
int NSISCALL popint(); 
int NSISCALL popint_or(); 
int NSISCALL myatoi(const TCHAR *s); 
unsigned NSISCALL myatou(const TCHAR *s); 
int NSISCALL myatoi_or(const TCHAR *s); 
void NSISCALL pushstring(const TCHAR *str);
void NSISCALL pushint(int value);
TCHAR * NSISCALL getuservariable(const int varnum);
void NSISCALL setuservariable(const int varnum, const TCHAR *var);
#if defined(_UNICODE)
#define PopStringW(x) popstring(x)
#define PushStringW(x) pushstring(x)
#define SetUserVariableW(x,y) setuservariable(x,y)
int NSISCALL PopStringA(char* ansiStr);
void NSISCALL PushStringA(const char* ansiStr);
void NSISCALL GetUserVariableW(const int varnum, wchar_t* wideStr);
void NSISCALL GetUserVariableA(const int varnum, char* ansiStr);
void NSISCALL SetUserVariableA(const int varnum, const char* ansiStr);
#else
#define PopStringA(x) popstring(x)
#define PushStringA(x) pushstring(x)
#define SetUserVariableA(x,y) setuservariable(x,y)
int NSISCALL PopStringW(wchar_t* wideStr);
void NSISCALL PushStringW(wchar_t* wideStr);
void NSISCALL GetUserVariableW(const int varnum, wchar_t* wideStr);
void NSISCALL GetUserVariableA(const int varnum, char* ansiStr);
void NSISCALL SetUserVariableW(const int varnum, const wchar_t* wideStr);
#endif
#if defined(__cplusplus)
}
#endif
