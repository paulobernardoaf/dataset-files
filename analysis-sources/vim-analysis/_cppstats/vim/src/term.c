#define tgetstr tgetstr_defined_wrong
#include "vim.h"
#if defined(HAVE_TGETENT)
#if defined(HAVE_TERMIOS_H)
#include <termios.h> 
#endif
#if defined(HAVE_TERMCAP_H)
#include <termcap.h>
#endif
#if defined(VMS)
#define TPUTSFUNCAST
#else
#if defined(HAVE_OUTFUNTYPE)
#define TPUTSFUNCAST (outfuntype)
#else
#define TPUTSFUNCAST (int (*)())
#endif
#endif
#endif
#undef tgetstr
struct builtin_term
{
int bt_entry;
char *bt_string;
};
#define BT_EXTRA_KEYS 0x101
static void parse_builtin_tcap(char_u *s);
static void gather_termleader(void);
#if defined(FEAT_TERMRESPONSE)
static void req_codes_from_term(void);
static void req_more_codes_from_term(void);
static void got_code_from_term(char_u *code, int len);
static void check_for_codes_from_term(void);
#endif
static void del_termcode_idx(int idx);
static int find_term_bykeys(char_u *src);
static int term_is_builtin(char_u *name);
static int term_7to8bit(char_u *p);
#if defined(HAVE_TGETENT)
static char *tgetent_error(char_u *, char_u *);
char *tgetstr(char *, char **);
#if defined(FEAT_TERMRESPONSE)
#if 0
#define DEBUG_TERMRESPONSE
static void log_tr(const char *fmt, ...);
#define LOG_TR(msg) log_tr msg
#else
#define LOG_TR(msg) do { } while (0)
#endif
typedef enum {
STATUS_GET, 
STATUS_SENT, 
STATUS_GOT, 
STATUS_FAIL 
} request_progress_T;
typedef struct {
request_progress_T tr_progress;
time_t tr_start; 
} termrequest_T;
#define TERMREQUEST_INIT {STATUS_GET, -1}
static termrequest_T crv_status = TERMREQUEST_INIT;
static termrequest_T u7_status = TERMREQUEST_INIT;
#if defined(FEAT_TERMINAL)
static termrequest_T rfg_status = TERMREQUEST_INIT;
static int fg_r = 0;
static int fg_g = 0;
static int fg_b = 0;
static int bg_r = 255;
static int bg_g = 255;
static int bg_b = 255;
#endif
static termrequest_T rbg_status = TERMREQUEST_INIT;
static termrequest_T rbm_status = TERMREQUEST_INIT;
static termrequest_T rcs_status = TERMREQUEST_INIT;
static termrequest_T winpos_status = TERMREQUEST_INIT;
static termrequest_T *all_termrequests[] = {
&crv_status,
&u7_status,
#if defined(FEAT_TERMINAL)
&rfg_status,
#endif
&rbg_status,
&rbm_status,
&rcs_status,
&winpos_status,
NULL
};
#endif
#if !defined(HAVE_OSPEED)
#if defined(OSPEED_EXTERN)
extern short ospeed;
#else
short ospeed;
#endif
#endif
#if !defined(HAVE_UP_BC_PC)
#if defined(UP_BC_PC_EXTERN)
extern char *UP, *BC, PC;
#else
char *UP, *BC, PC;
#endif
#endif
#define TGETSTR(s, p) vim_tgetstr((s), (p))
#define TGETENT(b, t) tgetent((char *)(b), (char *)(t))
static char_u *vim_tgetstr(char *s, char_u **pp);
#endif 
static int detected_8bit = FALSE; 
#if defined(FEAT_TERMRESPONSE)
static int initial_cursor_shape = 0;
static int initial_cursor_shape_blink = FALSE;
static int initial_cursor_blink = FALSE;
#endif
static struct builtin_term builtin_termcaps[] =
{
#if defined(FEAT_GUI)
{(int)KS_NAME, "gui"},
{(int)KS_CE, IF_EB("\033|$", ESC_STR "|$")},
{(int)KS_AL, IF_EB("\033|i", ESC_STR "|i")},
#if defined(TERMINFO)
{(int)KS_CAL, IF_EB("\033|%p1%dI", ESC_STR "|%p1%dI")},
#else
{(int)KS_CAL, IF_EB("\033|%dI", ESC_STR "|%dI")},
#endif
{(int)KS_DL, IF_EB("\033|d", ESC_STR "|d")},
#if defined(TERMINFO)
{(int)KS_CDL, IF_EB("\033|%p1%dD", ESC_STR "|%p1%dD")},
{(int)KS_CS, IF_EB("\033|%p1%d;%p2%dR", ESC_STR "|%p1%d;%p2%dR")},
{(int)KS_CSV, IF_EB("\033|%p1%d;%p2%dV", ESC_STR "|%p1%d;%p2%dV")},
#else
{(int)KS_CDL, IF_EB("\033|%dD", ESC_STR "|%dD")},
{(int)KS_CS, IF_EB("\033|%d;%dR", ESC_STR "|%d;%dR")},
{(int)KS_CSV, IF_EB("\033|%d;%dV", ESC_STR "|%d;%dV")},
#endif
{(int)KS_CL, IF_EB("\033|C", ESC_STR "|C")},
{(int)KS_ME, IF_EB("\033|31H", ESC_STR "|31H")}, 
{(int)KS_MR, IF_EB("\033|1h", ESC_STR "|1h")}, 
{(int)KS_MD, IF_EB("\033|2h", ESC_STR "|2h")}, 
{(int)KS_SE, IF_EB("\033|16H", ESC_STR "|16H")}, 
{(int)KS_SO, IF_EB("\033|16h", ESC_STR "|16h")}, 
{(int)KS_UE, IF_EB("\033|8H", ESC_STR "|8H")}, 
{(int)KS_US, IF_EB("\033|8h", ESC_STR "|8h")}, 
{(int)KS_UCE, IF_EB("\033|8C", ESC_STR "|8C")}, 
{(int)KS_UCS, IF_EB("\033|8c", ESC_STR "|8c")}, 
{(int)KS_STE, IF_EB("\033|4C", ESC_STR "|4C")}, 
{(int)KS_STS, IF_EB("\033|4c", ESC_STR "|4c")}, 
{(int)KS_CZR, IF_EB("\033|4H", ESC_STR "|4H")}, 
{(int)KS_CZH, IF_EB("\033|4h", ESC_STR "|4h")}, 
{(int)KS_VB, IF_EB("\033|f", ESC_STR "|f")},
{(int)KS_MS, "y"},
{(int)KS_UT, "y"},
{(int)KS_XN, "y"},
{(int)KS_LE, "\b"}, 
{(int)KS_ND, "\014"}, 
#if defined(TERMINFO)
{(int)KS_CM, IF_EB("\033|%p1%d;%p2%dM", ESC_STR "|%p1%d;%p2%dM")},
#else
{(int)KS_CM, IF_EB("\033|%d;%dM", ESC_STR "|%d;%dM")},
#endif
#endif
#if !defined(NO_BUILTIN_TCAPS)
#if defined(AMIGA) || defined(ALL_BUILTIN_TCAPS)
{(int)KS_NAME, "amiga"},
{(int)KS_CE, "\033[K"},
{(int)KS_CD, "\033[J"},
{(int)KS_AL, "\033[L"},
#if defined(TERMINFO)
{(int)KS_CAL, "\033[%p1%dL"},
#else
{(int)KS_CAL, "\033[%dL"},
#endif
{(int)KS_DL, "\033[M"},
#if defined(TERMINFO)
{(int)KS_CDL, "\033[%p1%dM"},
#else
{(int)KS_CDL, "\033[%dM"},
#endif
{(int)KS_CL, "\014"},
{(int)KS_VI, "\033[0 p"},
{(int)KS_VE, "\033[1 p"},
{(int)KS_ME, "\033[0m"},
{(int)KS_MR, "\033[7m"},
{(int)KS_MD, "\033[1m"},
{(int)KS_SE, "\033[0m"},
{(int)KS_SO, "\033[33m"},
{(int)KS_US, "\033[4m"},
{(int)KS_UE, "\033[0m"},
{(int)KS_CZH, "\033[3m"},
{(int)KS_CZR, "\033[0m"},
#if defined(__MORPHOS__) || defined(__AROS__)
{(int)KS_CCO, "8"}, 
#if defined(TERMINFO)
{(int)KS_CAB, "\033[4%p1%dm"},
{(int)KS_CAF, "\033[3%p1%dm"},
#else
{(int)KS_CAB, "\033[4%dm"}, 
{(int)KS_CAF, "\033[3%dm"}, 
#endif
{(int)KS_OP, "\033[m"}, 
#endif
{(int)KS_MS, "y"},
{(int)KS_UT, "y"}, 
{(int)KS_LE, "\b"},
#if defined(TERMINFO)
{(int)KS_CM, "\033[%i%p1%d;%p2%dH"},
#else
{(int)KS_CM, "\033[%i%d;%dH"},
#endif
#if defined(__MORPHOS__)
{(int)KS_SR, "\033M"},
#endif
#if defined(TERMINFO)
{(int)KS_CRI, "\033[%p1%dC"},
#else
{(int)KS_CRI, "\033[%dC"},
#endif
{K_UP, "\233A"},
{K_DOWN, "\233B"},
{K_LEFT, "\233D"},
{K_RIGHT, "\233C"},
{K_S_UP, "\233T"},
{K_S_DOWN, "\233S"},
{K_S_LEFT, "\233 A"},
{K_S_RIGHT, "\233 @"},
{K_S_TAB, "\233Z"},
{K_F1, "\233\060~"},
{K_F2, "\233\061~"},
{K_F3, "\233\062~"},
{K_F4, "\233\063~"},
{K_F5, "\233\064~"},
{K_F6, "\233\065~"},
{K_F7, "\233\066~"},
{K_F8, "\233\067~"},
{K_F9, "\233\070~"},
{K_F10, "\233\071~"},
{K_S_F1, "\233\061\060~"},
{K_S_F2, "\233\061\061~"},
{K_S_F3, "\233\061\062~"},
{K_S_F4, "\233\061\063~"},
{K_S_F5, "\233\061\064~"},
{K_S_F6, "\233\061\065~"},
{K_S_F7, "\233\061\066~"},
{K_S_F8, "\233\061\067~"},
{K_S_F9, "\233\061\070~"},
{K_S_F10, "\233\061\071~"},
{K_HELP, "\233?~"},
{K_INS, "\233\064\060~"}, 
{K_PAGEUP, "\233\064\061~"}, 
{K_PAGEDOWN, "\233\064\062~"}, 
{K_HOME, "\233\064\064~"}, 
{K_END, "\233\064\065~"}, 
{BT_EXTRA_KEYS, ""},
{TERMCAP2KEY('#', '2'), "\233\065\064~"}, 
{TERMCAP2KEY('#', '3'), "\233\065\060~"}, 
{TERMCAP2KEY('*', '7'), "\233\065\065~"}, 
#endif
#if defined(__BEOS__) || defined(ALL_BUILTIN_TCAPS)
{(int)KS_NAME, "beos-ansi"},
{(int)KS_CE, "\033[K"},
{(int)KS_CD, "\033[J"},
{(int)KS_AL, "\033[L"},
#if defined(TERMINFO)
{(int)KS_CAL, "\033[%p1%dL"},
#else
{(int)KS_CAL, "\033[%dL"},
#endif
{(int)KS_DL, "\033[M"},
#if defined(TERMINFO)
{(int)KS_CDL, "\033[%p1%dM"},
#else
{(int)KS_CDL, "\033[%dM"},
#endif
#if defined(BEOS_PR_OR_BETTER)
#if defined(TERMINFO)
{(int)KS_CS, "\033[%i%p1%d;%p2%dr"},
#else
{(int)KS_CS, "\033[%i%d;%dr"}, 
#endif
#endif
{(int)KS_CL, "\033[H\033[2J"},
#if defined(notyet)
{(int)KS_VI, "[VI]"}, 
{(int)KS_VE, "[VE]"}, 
#endif
{(int)KS_ME, "\033[m"}, 
{(int)KS_MR, "\033[7m"}, 
{(int)KS_MD, "\033[1m"}, 
{(int)KS_SO, "\033[31m"}, 
{(int)KS_SE, "\033[m"}, 
{(int)KS_CZH, "\033[35m"}, 
{(int)KS_CZR, "\033[m"}, 
{(int)KS_US, "\033[4m"}, 
{(int)KS_UE, "\033[m"}, 
{(int)KS_CCO, "8"}, 
#if defined(TERMINFO)
{(int)KS_CAB, "\033[4%p1%dm"},
{(int)KS_CAF, "\033[3%p1%dm"},
#else
{(int)KS_CAB, "\033[4%dm"}, 
{(int)KS_CAF, "\033[3%dm"}, 
#endif
{(int)KS_OP, "\033[m"}, 
{(int)KS_MS, "y"}, 
{(int)KS_UT, "y"}, 
{(int)KS_LE, "\b"},
#if defined(TERMINFO)
{(int)KS_CM, "\033[%i%p1%d;%p2%dH"},
#else
{(int)KS_CM, "\033[%i%d;%dH"},
#endif
{(int)KS_SR, "\033M"},
#if defined(TERMINFO)
{(int)KS_CRI, "\033[%p1%dC"},
#else
{(int)KS_CRI, "\033[%dC"},
#endif
#if defined(BEOS_DR8)
{(int)KS_DB, ""}, 
#endif
{K_UP, "\033[A"},
{K_DOWN, "\033[B"},
{K_LEFT, "\033[D"},
{K_RIGHT, "\033[C"},
#endif
#if defined(UNIX) || defined(ALL_BUILTIN_TCAPS) || defined(SOME_BUILTIN_TCAPS)
{(int)KS_NAME, "ansi"},
{(int)KS_CE, IF_EB("\033[K", ESC_STR "[K")},
{(int)KS_AL, IF_EB("\033[L", ESC_STR "[L")},
#if defined(TERMINFO)
{(int)KS_CAL, IF_EB("\033[%p1%dL", ESC_STR "[%p1%dL")},
#else
{(int)KS_CAL, IF_EB("\033[%dL", ESC_STR "[%dL")},
#endif
{(int)KS_DL, IF_EB("\033[M", ESC_STR "[M")},
#if defined(TERMINFO)
{(int)KS_CDL, IF_EB("\033[%p1%dM", ESC_STR "[%p1%dM")},
#else
{(int)KS_CDL, IF_EB("\033[%dM", ESC_STR "[%dM")},
#endif
{(int)KS_CL, IF_EB("\033[H\033[2J", ESC_STR "[H" ESC_STR_nc "[2J")},
{(int)KS_ME, IF_EB("\033[0m", ESC_STR "[0m")},
{(int)KS_MR, IF_EB("\033[7m", ESC_STR "[7m")},
{(int)KS_MS, "y"},
{(int)KS_UT, "y"}, 
{(int)KS_LE, "\b"},
#if defined(TERMINFO)
{(int)KS_CM, IF_EB("\033[%i%p1%d;%p2%dH", ESC_STR "[%i%p1%d;%p2%dH")},
#else
{(int)KS_CM, IF_EB("\033[%i%d;%dH", ESC_STR "[%i%d;%dH")},
#endif
#if defined(TERMINFO)
{(int)KS_CRI, IF_EB("\033[%p1%dC", ESC_STR "[%p1%dC")},
#else
{(int)KS_CRI, IF_EB("\033[%dC", ESC_STR "[%dC")},
#endif
#endif
#if defined(ALL_BUILTIN_TCAPS)
{(int)KS_NAME, "pcansi"},
{(int)KS_DL, "\033[M"},
{(int)KS_AL, "\033[L"},
{(int)KS_CE, "\033[K"},
{(int)KS_CL, "\033[2J"},
{(int)KS_ME, "\033[0m"},
{(int)KS_MR, "\033[5m"}, 
{(int)KS_MD, "\033[1m"}, 
{(int)KS_SE, "\033[0m"}, 
{(int)KS_SO, "\033[31m"}, 
{(int)KS_CZH, "\033[34;43m"}, 
{(int)KS_CZR, "\033[0m"}, 
{(int)KS_US, "\033[36;41m"}, 
{(int)KS_UE, "\033[0m"}, 
{(int)KS_CCO, "8"}, 
#if defined(TERMINFO)
{(int)KS_CAB, "\033[4%p1%dm"},
{(int)KS_CAF, "\033[3%p1%dm"},
#else
{(int)KS_CAB, "\033[4%dm"}, 
{(int)KS_CAF, "\033[3%dm"}, 
#endif
{(int)KS_OP, "\033[0m"}, 
{(int)KS_MS, "y"},
{(int)KS_UT, "y"}, 
{(int)KS_LE, "\b"},
#if defined(TERMINFO)
{(int)KS_CM, "\033[%i%p1%d;%p2%dH"},
#else
{(int)KS_CM, "\033[%i%d;%dH"},
#endif
#if defined(TERMINFO)
{(int)KS_CRI, "\033[%p1%dC"},
#else
{(int)KS_CRI, "\033[%dC"},
#endif
{K_UP, "\316H"},
{K_DOWN, "\316P"},
{K_LEFT, "\316K"},
{K_RIGHT, "\316M"},
{K_S_LEFT, "\316s"},
{K_S_RIGHT, "\316t"},
{K_F1, "\316;"},
{K_F2, "\316<"},
{K_F3, "\316="},
{K_F4, "\316>"},
{K_F5, "\316?"},
{K_F6, "\316@"},
{K_F7, "\316A"},
{K_F8, "\316B"},
{K_F9, "\316C"},
{K_F10, "\316D"},
{K_F11, "\316\205"}, 
{K_F12, "\316\206"}, 
{K_S_F1, "\316T"},
{K_S_F2, "\316U"},
{K_S_F3, "\316V"},
{K_S_F4, "\316W"},
{K_S_F5, "\316X"},
{K_S_F6, "\316Y"},
{K_S_F7, "\316Z"},
{K_S_F8, "\316["},
{K_S_F9, "\316\\"},
{K_S_F10, "\316]"},
{K_S_F11, "\316\207"}, 
{K_S_F12, "\316\210"}, 
{K_INS, "\316R"},
{K_DEL, "\316S"},
{K_HOME, "\316G"},
{K_END, "\316O"},
{K_PAGEDOWN, "\316Q"},
{K_PAGEUP, "\316I"},
#endif
#if defined(MSWIN) || defined(ALL_BUILTIN_TCAPS)
{(int)KS_NAME, "win32"},
{(int)KS_CE, "\033|K"}, 
{(int)KS_AL, "\033|L"}, 
#if defined(TERMINFO)
{(int)KS_CAL, "\033|%p1%dL"}, 
#else
{(int)KS_CAL, "\033|%dL"}, 
#endif
{(int)KS_DL, "\033|M"}, 
#if defined(TERMINFO)
{(int)KS_CDL, "\033|%p1%dM"}, 
{(int)KS_CSV, "\033|%p1%d;%p2%dV"},
#else
{(int)KS_CDL, "\033|%dM"}, 
{(int)KS_CSV, "\033|%d;%dV"},
#endif
{(int)KS_CL, "\033|J"}, 
{(int)KS_CD, "\033|j"}, 
{(int)KS_VI, "\033|v"}, 
{(int)KS_VE, "\033|V"}, 
{(int)KS_ME, "\033|0m"}, 
{(int)KS_MR, "\033|112m"}, 
{(int)KS_MD, "\033|15m"}, 
#if 1
{(int)KS_SO, "\033|31m"}, 
{(int)KS_SE, "\033|0m"}, 
#else
{(int)KS_SO, "\033|F"}, 
{(int)KS_SE, "\033|f"}, 
#endif
{(int)KS_CZH, "\033|225m"}, 
{(int)KS_CZR, "\033|0m"}, 
{(int)KS_US, "\033|67m"}, 
{(int)KS_UE, "\033|0m"}, 
{(int)KS_CCO, "16"}, 
#if defined(TERMINFO)
{(int)KS_CAB, "\033|%p1%db"}, 
{(int)KS_CAF, "\033|%p1%df"}, 
#else
{(int)KS_CAB, "\033|%db"}, 
{(int)KS_CAF, "\033|%df"}, 
#endif
{(int)KS_MS, "y"}, 
{(int)KS_UT, "y"},
{(int)KS_XN, "y"},
{(int)KS_LE, "\b"},
#if defined(TERMINFO)
{(int)KS_CM, "\033|%i%p1%d;%p2%dH"}, 
#else
{(int)KS_CM, "\033|%i%d;%dH"}, 
#endif
{(int)KS_VB, "\033|B"}, 
{(int)KS_TI, "\033|S"}, 
{(int)KS_TE, "\033|E"}, 
#if defined(TERMINFO)
{(int)KS_CS, "\033|%i%p1%d;%p2%dr"}, 
#else
{(int)KS_CS, "\033|%i%d;%dr"}, 
#endif
#if defined(FEAT_TERMGUICOLORS)
{(int)KS_8F, "\033|38;2;%lu;%lu;%lum"},
{(int)KS_8B, "\033|48;2;%lu;%lu;%lum"},
#endif
{K_UP, "\316H"},
{K_DOWN, "\316P"},
{K_LEFT, "\316K"},
{K_RIGHT, "\316M"},
{K_S_UP, "\316\304"},
{K_S_DOWN, "\316\317"},
{K_S_LEFT, "\316\311"},
{K_C_LEFT, "\316s"},
{K_S_RIGHT, "\316\313"},
{K_C_RIGHT, "\316t"},
{K_S_TAB, "\316\017"},
{K_F1, "\316;"},
{K_F2, "\316<"},
{K_F3, "\316="},
{K_F4, "\316>"},
{K_F5, "\316?"},
{K_F6, "\316@"},
{K_F7, "\316A"},
{K_F8, "\316B"},
{K_F9, "\316C"},
{K_F10, "\316D"},
{K_F11, "\316\205"},
{K_F12, "\316\206"},
{K_S_F1, "\316T"},
{K_S_F2, "\316U"},
{K_S_F3, "\316V"},
{K_S_F4, "\316W"},
{K_S_F5, "\316X"},
{K_S_F6, "\316Y"},
{K_S_F7, "\316Z"},
{K_S_F8, "\316["},
{K_S_F9, "\316\\"},
{K_S_F10, "\316]"},
{K_S_F11, "\316\207"},
{K_S_F12, "\316\210"},
{K_INS, "\316R"},
{K_DEL, "\316S"},
{K_HOME, "\316G"},
{K_S_HOME, "\316\302"},
{K_C_HOME, "\316w"},
{K_END, "\316O"},
{K_S_END, "\316\315"},
{K_C_END, "\316u"},
{K_PAGEDOWN, "\316Q"},
{K_PAGEUP, "\316I"},
{K_KPLUS, "\316N"},
{K_KMINUS, "\316J"},
{K_KMULTIPLY, "\316\067"},
{K_K0, "\316\332"},
{K_K1, "\316\336"},
{K_K2, "\316\342"},
{K_K3, "\316\346"},
{K_K4, "\316\352"},
{K_K5, "\316\356"},
{K_K6, "\316\362"},
{K_K7, "\316\366"},
{K_K8, "\316\372"},
{K_K9, "\316\376"},
{K_BS, "\316x"},
#endif
#if defined(VMS) || defined(ALL_BUILTIN_TCAPS)
{(int)KS_NAME, "vt320"},
{(int)KS_CE, IF_EB("\033[K", ESC_STR "[K")},
{(int)KS_AL, IF_EB("\033[L", ESC_STR "[L")},
#if defined(TERMINFO)
{(int)KS_CAL, IF_EB("\033[%p1%dL", ESC_STR "[%p1%dL")},
#else
{(int)KS_CAL, IF_EB("\033[%dL", ESC_STR "[%dL")},
#endif
{(int)KS_DL, IF_EB("\033[M", ESC_STR "[M")},
#if defined(TERMINFO)
{(int)KS_CDL, IF_EB("\033[%p1%dM", ESC_STR "[%p1%dM")},
#else
{(int)KS_CDL, IF_EB("\033[%dM", ESC_STR "[%dM")},
#endif
{(int)KS_CL, IF_EB("\033[H\033[2J", ESC_STR "[H" ESC_STR_nc "[2J")},
{(int)KS_CD, IF_EB("\033[J", ESC_STR "[J")},
{(int)KS_CCO, "8"}, 
{(int)KS_ME, IF_EB("\033[0m", ESC_STR "[0m")},
{(int)KS_MR, IF_EB("\033[7m", ESC_STR "[7m")},
{(int)KS_MD, IF_EB("\033[1m", ESC_STR "[1m")}, 
{(int)KS_SE, IF_EB("\033[22m", ESC_STR "[22m")},
{(int)KS_UE, IF_EB("\033[24m", ESC_STR "[24m")},
{(int)KS_US, IF_EB("\033[4m", ESC_STR "[4m")}, 
{(int)KS_CZH, IF_EB("\033[34;43m", ESC_STR "[34;43m")}, 
{(int)KS_CZR, IF_EB("\033[0m", ESC_STR "[0m")}, 
{(int)KS_CAB, IF_EB("\033[4%dm", ESC_STR "[4%dm")}, 
{(int)KS_CAF, IF_EB("\033[3%dm", ESC_STR "[3%dm")}, 
{(int)KS_CSB, IF_EB("\033[102;%dm", ESC_STR "[102;%dm")}, 
{(int)KS_CSF, IF_EB("\033[101;%dm", ESC_STR "[101;%dm")}, 
{(int)KS_MS, "y"},
{(int)KS_UT, "y"},
{(int)KS_XN, "y"},
{(int)KS_LE, "\b"},
#if defined(TERMINFO)
{(int)KS_CM, IF_EB("\033[%i%p1%d;%p2%dH",
ESC_STR "[%i%p1%d;%p2%dH")},
#else
{(int)KS_CM, IF_EB("\033[%i%d;%dH", ESC_STR "[%i%d;%dH")},
#endif
#if defined(TERMINFO)
{(int)KS_CRI, IF_EB("\033[%p1%dC", ESC_STR "[%p1%dC")},
#else
{(int)KS_CRI, IF_EB("\033[%dC", ESC_STR "[%dC")},
#endif
{K_UP, IF_EB("\033[A", ESC_STR "[A")},
{K_DOWN, IF_EB("\033[B", ESC_STR "[B")},
{K_RIGHT, IF_EB("\033[C", ESC_STR "[C")},
{K_LEFT, IF_EB("\033[D", ESC_STR "[D")},
{K_F1, IF_EB("\033[11~", ESC_STR "[11~")},
{K_F2, IF_EB("\033[12~", ESC_STR "[12~")},
{K_F3, IF_EB("\033[13~", ESC_STR "[13~")},
{K_F4, IF_EB("\033[14~", ESC_STR "[14~")},
{K_F5, IF_EB("\033[15~", ESC_STR "[15~")},
{K_F6, IF_EB("\033[17~", ESC_STR "[17~")},
{K_F7, IF_EB("\033[18~", ESC_STR "[18~")},
{K_F8, IF_EB("\033[19~", ESC_STR "[19~")},
{K_F9, IF_EB("\033[20~", ESC_STR "[20~")},
{K_F10, IF_EB("\033[21~", ESC_STR "[21~")},
{K_F11, IF_EB("\033[23~", ESC_STR "[23~")},
{K_F12, IF_EB("\033[24~", ESC_STR "[24~")},
{K_F13, IF_EB("\033[25~", ESC_STR "[25~")},
{K_F14, IF_EB("\033[26~", ESC_STR "[26~")},
{K_F15, IF_EB("\033[28~", ESC_STR "[28~")}, 
{K_F16, IF_EB("\033[29~", ESC_STR "[29~")}, 
{K_F17, IF_EB("\033[31~", ESC_STR "[31~")},
{K_F18, IF_EB("\033[32~", ESC_STR "[32~")},
{K_F19, IF_EB("\033[33~", ESC_STR "[33~")},
{K_F20, IF_EB("\033[34~", ESC_STR "[34~")},
{K_INS, IF_EB("\033[2~", ESC_STR "[2~")},
{K_DEL, IF_EB("\033[3~", ESC_STR "[3~")},
{K_HOME, IF_EB("\033[1~", ESC_STR "[1~")},
{K_END, IF_EB("\033[4~", ESC_STR "[4~")},
{K_PAGEUP, IF_EB("\033[5~", ESC_STR "[5~")},
{K_PAGEDOWN, IF_EB("\033[6~", ESC_STR "[6~")},
{K_KPLUS, IF_EB("\033Ok", ESC_STR "Ok")}, 
{K_KMINUS, IF_EB("\033Om", ESC_STR "Om")}, 
{K_KDIVIDE, IF_EB("\033Oo", ESC_STR "Oo")}, 
{K_KMULTIPLY, IF_EB("\033Oj", ESC_STR "Oj")}, 
{K_KENTER, IF_EB("\033OM", ESC_STR "OM")}, 
{K_K0, IF_EB("\033Op", ESC_STR "Op")}, 
{K_K1, IF_EB("\033Oq", ESC_STR "Oq")}, 
{K_K2, IF_EB("\033Or", ESC_STR "Or")}, 
{K_K3, IF_EB("\033Os", ESC_STR "Os")}, 
{K_K4, IF_EB("\033Ot", ESC_STR "Ot")}, 
{K_K5, IF_EB("\033Ou", ESC_STR "Ou")}, 
{K_K6, IF_EB("\033Ov", ESC_STR "Ov")}, 
{K_K7, IF_EB("\033Ow", ESC_STR "Ow")}, 
{K_K8, IF_EB("\033Ox", ESC_STR "Ox")}, 
{K_K9, IF_EB("\033Oy", ESC_STR "Oy")}, 
{K_BS, "\x7f"}, 
#endif
#if defined(ALL_BUILTIN_TCAPS) || defined(__MINT__)
{(int)KS_NAME, "vt52"},
{(int)KS_CE, IF_EB("\033K", ESC_STR "K")},
{(int)KS_CD, IF_EB("\033J", ESC_STR "J")},
#if defined(TERMINFO)
{(int)KS_CM, IF_EB("\033Y%p1%' '%+%c%p2%' '%+%c",
ESC_STR "Y%p1%' '%+%c%p2%' '%+%c")},
#else
{(int)KS_CM, IF_EB("\033Y%+ %+ ", ESC_STR "Y%+ %+ ")},
#endif
{(int)KS_LE, "\b"},
{(int)KS_SR, IF_EB("\033I", ESC_STR "I")},
{(int)KS_AL, IF_EB("\033L", ESC_STR "L")},
{(int)KS_DL, IF_EB("\033M", ESC_STR "M")},
{K_UP, IF_EB("\033A", ESC_STR "A")},
{K_DOWN, IF_EB("\033B", ESC_STR "B")},
{K_LEFT, IF_EB("\033D", ESC_STR "D")},
{K_RIGHT, IF_EB("\033C", ESC_STR "C")},
{K_F1, IF_EB("\033P", ESC_STR "P")},
{K_F2, IF_EB("\033Q", ESC_STR "Q")},
{K_F3, IF_EB("\033R", ESC_STR "R")},
#if defined(__MINT__)
{(int)KS_CL, IF_EB("\033E", ESC_STR "E")},
{(int)KS_VE, IF_EB("\033e", ESC_STR "e")},
{(int)KS_VI, IF_EB("\033f", ESC_STR "f")},
{(int)KS_SO, IF_EB("\033p", ESC_STR "p")},
{(int)KS_SE, IF_EB("\033q", ESC_STR "q")},
{K_S_UP, IF_EB("\033a", ESC_STR "a")},
{K_S_DOWN, IF_EB("\033b", ESC_STR "b")},
{K_S_LEFT, IF_EB("\033d", ESC_STR "d")},
{K_S_RIGHT, IF_EB("\033c", ESC_STR "c")},
{K_F4, IF_EB("\033S", ESC_STR "S")},
{K_F5, IF_EB("\033T", ESC_STR "T")},
{K_F6, IF_EB("\033U", ESC_STR "U")},
{K_F7, IF_EB("\033V", ESC_STR "V")},
{K_F8, IF_EB("\033W", ESC_STR "W")},
{K_F9, IF_EB("\033X", ESC_STR "X")},
{K_F10, IF_EB("\033Y", ESC_STR "Y")},
{K_S_F1, IF_EB("\033p", ESC_STR "p")},
{K_S_F2, IF_EB("\033q", ESC_STR "q")},
{K_S_F3, IF_EB("\033r", ESC_STR "r")},
{K_S_F4, IF_EB("\033s", ESC_STR "s")},
{K_S_F5, IF_EB("\033t", ESC_STR "t")},
{K_S_F6, IF_EB("\033u", ESC_STR "u")},
{K_S_F7, IF_EB("\033v", ESC_STR "v")},
{K_S_F8, IF_EB("\033w", ESC_STR "w")},
{K_S_F9, IF_EB("\033x", ESC_STR "x")},
{K_S_F10, IF_EB("\033y", ESC_STR "y")},
{K_INS, IF_EB("\033I", ESC_STR "I")},
{K_HOME, IF_EB("\033E", ESC_STR "E")},
{K_PAGEDOWN, IF_EB("\033b", ESC_STR "b")},
{K_PAGEUP, IF_EB("\033a", ESC_STR "a")},
#else
{(int)KS_CL, IF_EB("\033H\033J", ESC_STR "H" ESC_STR_nc "J")},
{(int)KS_MS, "y"},
#endif
#endif
#if defined(UNIX) || defined(ALL_BUILTIN_TCAPS) || defined(SOME_BUILTIN_TCAPS)
{(int)KS_NAME, "xterm"},
{(int)KS_CE, IF_EB("\033[K", ESC_STR "[K")},
{(int)KS_AL, IF_EB("\033[L", ESC_STR "[L")},
#if defined(TERMINFO)
{(int)KS_CAL, IF_EB("\033[%p1%dL", ESC_STR "[%p1%dL")},
#else
{(int)KS_CAL, IF_EB("\033[%dL", ESC_STR "[%dL")},
#endif
{(int)KS_DL, IF_EB("\033[M", ESC_STR "[M")},
#if defined(TERMINFO)
{(int)KS_CDL, IF_EB("\033[%p1%dM", ESC_STR "[%p1%dM")},
#else
{(int)KS_CDL, IF_EB("\033[%dM", ESC_STR "[%dM")},
#endif
#if defined(TERMINFO)
{(int)KS_CS, IF_EB("\033[%i%p1%d;%p2%dr",
ESC_STR "[%i%p1%d;%p2%dr")},
#else
{(int)KS_CS, IF_EB("\033[%i%d;%dr", ESC_STR "[%i%d;%dr")},
#endif
{(int)KS_CL, IF_EB("\033[H\033[2J", ESC_STR "[H" ESC_STR_nc "[2J")},
{(int)KS_CD, IF_EB("\033[J", ESC_STR "[J")},
{(int)KS_ME, IF_EB("\033[m", ESC_STR "[m")},
{(int)KS_MR, IF_EB("\033[7m", ESC_STR "[7m")},
{(int)KS_MD, IF_EB("\033[1m", ESC_STR "[1m")},
{(int)KS_UE, IF_EB("\033[m", ESC_STR "[m")},
{(int)KS_US, IF_EB("\033[4m", ESC_STR "[4m")},
{(int)KS_STE, IF_EB("\033[29m", ESC_STR "[29m")},
{(int)KS_STS, IF_EB("\033[9m", ESC_STR "[9m")},
{(int)KS_MS, "y"},
{(int)KS_UT, "y"},
{(int)KS_LE, "\b"},
{(int)KS_VI, IF_EB("\033[?25l", ESC_STR "[?25l")},
{(int)KS_VE, IF_EB("\033[?25h", ESC_STR "[?25h")},
{(int)KS_VS, IF_EB("\033[?12h", ESC_STR "[?12h")},
{(int)KS_CVS, IF_EB("\033[?12l", ESC_STR "[?12l")},
#if defined(TERMINFO)
{(int)KS_CSH, IF_EB("\033[%p1%d q", ESC_STR "[%p1%d q")},
#else
{(int)KS_CSH, IF_EB("\033[%d q", ESC_STR "[%d q")},
#endif
{(int)KS_CRC, IF_EB("\033[?12$p", ESC_STR "[?12$p")},
{(int)KS_CRS, IF_EB("\033P$q q\033\\", ESC_STR "P$q q" ESC_STR "\\")},
#if defined(TERMINFO)
{(int)KS_CM, IF_EB("\033[%i%p1%d;%p2%dH",
ESC_STR "[%i%p1%d;%p2%dH")},
#else
{(int)KS_CM, IF_EB("\033[%i%d;%dH", ESC_STR "[%i%d;%dH")},
#endif
{(int)KS_SR, IF_EB("\033M", ESC_STR "M")},
#if defined(TERMINFO)
{(int)KS_CRI, IF_EB("\033[%p1%dC", ESC_STR "[%p1%dC")},
#else
{(int)KS_CRI, IF_EB("\033[%dC", ESC_STR "[%dC")},
#endif
{(int)KS_KS, IF_EB("\033[?1h\033=", ESC_STR "[?1h" ESC_STR_nc "=")},
{(int)KS_KE, IF_EB("\033[?1l\033>", ESC_STR "[?1l" ESC_STR_nc ">")},
#if defined(FEAT_XTERM_SAVE)
{(int)KS_TI, IF_EB("\0337\033[?47h", ESC_STR "7" ESC_STR_nc "[?47h")},
{(int)KS_TE, IF_EB("\033[?47l\0338",
ESC_STR_nc "[?47l" ESC_STR_nc "8")},
#endif
{(int)KS_CTI, IF_EB("\033[>4;2m", ESC_STR_nc "[>4;2m")},
{(int)KS_CTE, IF_EB("\033[>4;m", ESC_STR_nc "[>4;m")},
{(int)KS_CIS, IF_EB("\033]1;", ESC_STR "]1;")},
{(int)KS_CIE, "\007"},
{(int)KS_TS, IF_EB("\033]2;", ESC_STR "]2;")},
{(int)KS_FS, "\007"},
{(int)KS_CSC, IF_EB("\033]12;", ESC_STR "]12;")},
{(int)KS_CEC, "\007"},
#if defined(TERMINFO)
{(int)KS_CWS, IF_EB("\033[8;%p1%d;%p2%dt",
ESC_STR "[8;%p1%d;%p2%dt")},
{(int)KS_CWP, IF_EB("\033[3;%p1%d;%p2%dt",
ESC_STR "[3;%p1%d;%p2%dt")},
{(int)KS_CGP, IF_EB("\033[13t", ESC_STR "[13t")},
#else
{(int)KS_CWS, IF_EB("\033[8;%d;%dt", ESC_STR "[8;%d;%dt")},
{(int)KS_CWP, IF_EB("\033[3;%d;%dt", ESC_STR "[3;%d;%dt")},
{(int)KS_CGP, IF_EB("\033[13t", ESC_STR "[13t")},
#endif
{(int)KS_CRV, IF_EB("\033[>c", ESC_STR "[>c")},
{(int)KS_RFG, IF_EB("\033]10;?\007", ESC_STR "]10;?\007")},
{(int)KS_RBG, IF_EB("\033]11;?\007", ESC_STR "]11;?\007")},
{(int)KS_U7, IF_EB("\033[6n", ESC_STR "[6n")},
#if defined(FEAT_TERMGUICOLORS)
{(int)KS_8F, IF_EB("\033[38;2;%lu;%lu;%lum", ESC_STR "[38;2;%lu;%lu;%lum")},
{(int)KS_8B, IF_EB("\033[48;2;%lu;%lu;%lum", ESC_STR "[48;2;%lu;%lu;%lum")},
#endif
{(int)KS_CBE, IF_EB("\033[?2004h", ESC_STR "[?2004h")},
{(int)KS_CBD, IF_EB("\033[?2004l", ESC_STR "[?2004l")},
{(int)KS_CST, IF_EB("\033[22;2t", ESC_STR "[22;2t")},
{(int)KS_CRT, IF_EB("\033[23;2t", ESC_STR "[23;2t")},
{(int)KS_SSI, IF_EB("\033[22;1t", ESC_STR "[22;1t")},
{(int)KS_SRI, IF_EB("\033[23;1t", ESC_STR "[23;1t")},
{K_UP, IF_EB("\033O*A", ESC_STR "O*A")},
{K_DOWN, IF_EB("\033O*B", ESC_STR "O*B")},
{K_RIGHT, IF_EB("\033O*C", ESC_STR "O*C")},
{K_LEFT, IF_EB("\033O*D", ESC_STR "O*D")},
{K_XUP, IF_EB("\033[1;*A", ESC_STR "[1;*A")},
{K_XDOWN, IF_EB("\033[1;*B", ESC_STR "[1;*B")},
{K_XRIGHT, IF_EB("\033[1;*C", ESC_STR "[1;*C")},
{K_XLEFT, IF_EB("\033[1;*D", ESC_STR "[1;*D")},
{K_XF1, IF_EB("\033O*P", ESC_STR "O*P")},
{K_XF2, IF_EB("\033O*Q", ESC_STR "O*Q")},
{K_XF3, IF_EB("\033O*R", ESC_STR "O*R")},
{K_XF4, IF_EB("\033O*S", ESC_STR "O*S")},
{K_F1, IF_EB("\033[11;*~", ESC_STR "[11;*~")},
{K_F2, IF_EB("\033[12;*~", ESC_STR "[12;*~")},
{K_F3, IF_EB("\033[13;*~", ESC_STR "[13;*~")},
{K_F4, IF_EB("\033[14;*~", ESC_STR "[14;*~")},
{K_F5, IF_EB("\033[15;*~", ESC_STR "[15;*~")},
{K_F6, IF_EB("\033[17;*~", ESC_STR "[17;*~")},
{K_F7, IF_EB("\033[18;*~", ESC_STR "[18;*~")},
{K_F8, IF_EB("\033[19;*~", ESC_STR "[19;*~")},
{K_F9, IF_EB("\033[20;*~", ESC_STR "[20;*~")},
{K_F10, IF_EB("\033[21;*~", ESC_STR "[21;*~")},
{K_F11, IF_EB("\033[23;*~", ESC_STR "[23;*~")},
{K_F12, IF_EB("\033[24;*~", ESC_STR "[24;*~")},
{K_S_TAB, IF_EB("\033[Z", ESC_STR "[Z")},
{K_HELP, IF_EB("\033[28;*~", ESC_STR "[28;*~")},
{K_UNDO, IF_EB("\033[26;*~", ESC_STR "[26;*~")},
{K_INS, IF_EB("\033[2;*~", ESC_STR "[2;*~")},
{K_HOME, IF_EB("\033[1;*H", ESC_STR "[1;*H")},
{K_KHOME, IF_EB("\033[1;*~", ESC_STR "[1;*~")},
{K_XHOME, IF_EB("\033O*H", ESC_STR "O*H")}, 
{K_ZHOME, IF_EB("\033[7;*~", ESC_STR "[7;*~")}, 
{K_END, IF_EB("\033[1;*F", ESC_STR "[1;*F")},
{K_KEND, IF_EB("\033[4;*~", ESC_STR "[4;*~")},
{K_XEND, IF_EB("\033O*F", ESC_STR "O*F")}, 
{K_ZEND, IF_EB("\033[8;*~", ESC_STR "[8;*~")},
{K_PAGEUP, IF_EB("\033[5;*~", ESC_STR "[5;*~")},
{K_PAGEDOWN, IF_EB("\033[6;*~", ESC_STR "[6;*~")},
{K_KPLUS, IF_EB("\033O*k", ESC_STR "O*k")}, 
{K_KMINUS, IF_EB("\033O*m", ESC_STR "O*m")}, 
{K_KDIVIDE, IF_EB("\033O*o", ESC_STR "O*o")}, 
{K_KMULTIPLY, IF_EB("\033O*j", ESC_STR "O*j")}, 
{K_KENTER, IF_EB("\033O*M", ESC_STR "O*M")}, 
{K_KPOINT, IF_EB("\033O*n", ESC_STR "O*n")}, 
{K_K0, IF_EB("\033O*p", ESC_STR "O*p")}, 
{K_K1, IF_EB("\033O*q", ESC_STR "O*q")}, 
{K_K2, IF_EB("\033O*r", ESC_STR "O*r")}, 
{K_K3, IF_EB("\033O*s", ESC_STR "O*s")}, 
{K_K4, IF_EB("\033O*t", ESC_STR "O*t")}, 
{K_K5, IF_EB("\033O*u", ESC_STR "O*u")}, 
{K_K6, IF_EB("\033O*v", ESC_STR "O*v")}, 
{K_K7, IF_EB("\033O*w", ESC_STR "O*w")}, 
{K_K8, IF_EB("\033O*x", ESC_STR "O*x")}, 
{K_K9, IF_EB("\033O*y", ESC_STR "O*y")}, 
{K_KDEL, IF_EB("\033[3;*~", ESC_STR "[3;*~")}, 
{K_PS, IF_EB("\033[200~", ESC_STR "[200~")}, 
{K_PE, IF_EB("\033[201~", ESC_STR "[201~")}, 
{BT_EXTRA_KEYS, ""},
{TERMCAP2KEY('k', '0'), IF_EB("\033[10;*~", ESC_STR "[10;*~")}, 
{TERMCAP2KEY('F', '3'), IF_EB("\033[25;*~", ESC_STR "[25;*~")}, 
{TERMCAP2KEY('F', '6'), IF_EB("\033[29;*~", ESC_STR "[29;*~")}, 
{TERMCAP2KEY('F', '7'), IF_EB("\033[31;*~", ESC_STR "[31;*~")}, 
{TERMCAP2KEY('F', '8'), IF_EB("\033[32;*~", ESC_STR "[32;*~")}, 
{TERMCAP2KEY('F', '9'), IF_EB("\033[33;*~", ESC_STR "[33;*~")}, 
{TERMCAP2KEY('F', 'A'), IF_EB("\033[34;*~", ESC_STR "[34;*~")}, 
{TERMCAP2KEY('F', 'B'), IF_EB("\033[42;*~", ESC_STR "[42;*~")}, 
{TERMCAP2KEY('F', 'C'), IF_EB("\033[43;*~", ESC_STR "[43;*~")}, 
{TERMCAP2KEY('F', 'D'), IF_EB("\033[44;*~", ESC_STR "[44;*~")}, 
{TERMCAP2KEY('F', 'E'), IF_EB("\033[45;*~", ESC_STR "[45;*~")}, 
{TERMCAP2KEY('F', 'F'), IF_EB("\033[46;*~", ESC_STR "[46;*~")}, 
{TERMCAP2KEY('F', 'G'), IF_EB("\033[47;*~", ESC_STR "[47;*~")}, 
{TERMCAP2KEY('F', 'H'), IF_EB("\033[48;*~", ESC_STR "[48;*~")}, 
{TERMCAP2KEY('F', 'I'), IF_EB("\033[49;*~", ESC_STR "[49;*~")}, 
{TERMCAP2KEY('F', 'J'), IF_EB("\033[50;*~", ESC_STR "[50;*~")}, 
{TERMCAP2KEY('F', 'K'), IF_EB("\033[51;*~", ESC_STR "[51;*~")}, 
{TERMCAP2KEY('F', 'L'), IF_EB("\033[52;*~", ESC_STR "[52;*~")}, 
{TERMCAP2KEY('F', 'M'), IF_EB("\033[53;*~", ESC_STR "[53;*~")}, 
{TERMCAP2KEY('F', 'N'), IF_EB("\033[54;*~", ESC_STR "[54;*~")}, 
{TERMCAP2KEY('F', 'O'), IF_EB("\033[55;*~", ESC_STR "[55;*~")}, 
{TERMCAP2KEY('F', 'P'), IF_EB("\033[56;*~", ESC_STR "[56;*~")}, 
{TERMCAP2KEY('F', 'Q'), IF_EB("\033[57;*~", ESC_STR "[57;*~")}, 
{TERMCAP2KEY('F', 'R'), IF_EB("\033[58;*~", ESC_STR "[58;*~")}, 
#endif
#if defined(UNIX) || defined(ALL_BUILTIN_TCAPS)
{(int)KS_NAME, "iris-ansi"},
{(int)KS_CE, "\033[K"},
{(int)KS_CD, "\033[J"},
{(int)KS_AL, "\033[L"},
#if defined(TERMINFO)
{(int)KS_CAL, "\033[%p1%dL"},
#else
{(int)KS_CAL, "\033[%dL"},
#endif
{(int)KS_DL, "\033[M"},
#if defined(TERMINFO)
{(int)KS_CDL, "\033[%p1%dM"},
#else
{(int)KS_CDL, "\033[%dM"},
#endif
#if 0 
#if defined(TERMINFO)
{(int)KS_CS, "\033[%i%p1%d;%p2%dr"},
#else
{(int)KS_CS, "\033[%i%d;%dr"},
#endif
#endif
{(int)KS_CL, "\033[H\033[2J"},
{(int)KS_VE, "\033[9/y\033[12/y"}, 
{(int)KS_VS, "\033[10/y\033[=1h\033[=2l"}, 
{(int)KS_TI, "\033[=6h"},
{(int)KS_TE, "\033[=6l"},
{(int)KS_SE, "\033[21;27m"},
{(int)KS_SO, "\033[1;7m"},
{(int)KS_ME, "\033[m"},
{(int)KS_MR, "\033[7m"},
{(int)KS_MD, "\033[1m"},
{(int)KS_CCO, "8"}, 
{(int)KS_CZH, "\033[3m"}, 
{(int)KS_CZR, "\033[23m"}, 
{(int)KS_US, "\033[4m"}, 
{(int)KS_UE, "\033[24m"}, 
#if defined(TERMINFO)
{(int)KS_CAB, "\033[4%p1%dm"}, 
{(int)KS_CAF, "\033[3%p1%dm"}, 
{(int)KS_CSB, "\033[102;%p1%dm"}, 
{(int)KS_CSF, "\033[101;%p1%dm"}, 
#else
{(int)KS_CAB, "\033[4%dm"}, 
{(int)KS_CAF, "\033[3%dm"}, 
{(int)KS_CSB, "\033[102;%dm"}, 
{(int)KS_CSF, "\033[101;%dm"}, 
#endif
{(int)KS_MS, "y"}, 
{(int)KS_UT, "y"}, 
{(int)KS_LE, "\b"},
#if defined(TERMINFO)
{(int)KS_CM, "\033[%i%p1%d;%p2%dH"},
#else
{(int)KS_CM, "\033[%i%d;%dH"},
#endif
{(int)KS_SR, "\033M"},
#if defined(TERMINFO)
{(int)KS_CRI, "\033[%p1%dC"},
#else
{(int)KS_CRI, "\033[%dC"},
#endif
{(int)KS_CIS, "\033P3.y"},
{(int)KS_CIE, "\234"}, 
{(int)KS_TS, "\033P1.y"},
{(int)KS_FS, "\234"}, 
#if defined(TERMINFO)
{(int)KS_CWS, "\033[203;%p1%d;%p2%d/y"},
{(int)KS_CWP, "\033[205;%p1%d;%p2%d/y"},
#else
{(int)KS_CWS, "\033[203;%d;%d/y"},
{(int)KS_CWP, "\033[205;%d;%d/y"},
#endif
{K_UP, "\033[A"},
{K_DOWN, "\033[B"},
{K_LEFT, "\033[D"},
{K_RIGHT, "\033[C"},
{K_S_UP, "\033[161q"},
{K_S_DOWN, "\033[164q"},
{K_S_LEFT, "\033[158q"},
{K_S_RIGHT, "\033[167q"},
{K_F1, "\033[001q"},
{K_F2, "\033[002q"},
{K_F3, "\033[003q"},
{K_F4, "\033[004q"},
{K_F5, "\033[005q"},
{K_F6, "\033[006q"},
{K_F7, "\033[007q"},
{K_F8, "\033[008q"},
{K_F9, "\033[009q"},
{K_F10, "\033[010q"},
{K_F11, "\033[011q"},
{K_F12, "\033[012q"},
{K_S_F1, "\033[013q"},
{K_S_F2, "\033[014q"},
{K_S_F3, "\033[015q"},
{K_S_F4, "\033[016q"},
{K_S_F5, "\033[017q"},
{K_S_F6, "\033[018q"},
{K_S_F7, "\033[019q"},
{K_S_F8, "\033[020q"},
{K_S_F9, "\033[021q"},
{K_S_F10, "\033[022q"},
{K_S_F11, "\033[023q"},
{K_S_F12, "\033[024q"},
{K_INS, "\033[139q"},
{K_HOME, "\033[H"},
{K_END, "\033[146q"},
{K_PAGEUP, "\033[150q"},
{K_PAGEDOWN, "\033[154q"},
#endif
#if defined(DEBUG) || defined(ALL_BUILTIN_TCAPS)
{(int)KS_NAME, "debug"},
{(int)KS_CE, "[CE]"},
{(int)KS_CD, "[CD]"},
{(int)KS_AL, "[AL]"},
#if defined(TERMINFO)
{(int)KS_CAL, "[CAL%p1%d]"},
#else
{(int)KS_CAL, "[CAL%d]"},
#endif
{(int)KS_DL, "[DL]"},
#if defined(TERMINFO)
{(int)KS_CDL, "[CDL%p1%d]"},
#else
{(int)KS_CDL, "[CDL%d]"},
#endif
#if defined(TERMINFO)
{(int)KS_CS, "[%p1%dCS%p2%d]"},
#else
{(int)KS_CS, "[%dCS%d]"},
#endif
#if defined(TERMINFO)
{(int)KS_CSV, "[%p1%dCSV%p2%d]"},
#else
{(int)KS_CSV, "[%dCSV%d]"},
#endif
#if defined(TERMINFO)
{(int)KS_CAB, "[CAB%p1%d]"},
{(int)KS_CAF, "[CAF%p1%d]"},
{(int)KS_CSB, "[CSB%p1%d]"},
{(int)KS_CSF, "[CSF%p1%d]"},
#else
{(int)KS_CAB, "[CAB%d]"},
{(int)KS_CAF, "[CAF%d]"},
{(int)KS_CSB, "[CSB%d]"},
{(int)KS_CSF, "[CSF%d]"},
#endif
{(int)KS_OP, "[OP]"},
{(int)KS_LE, "[LE]"},
{(int)KS_CL, "[CL]"},
{(int)KS_VI, "[VI]"},
{(int)KS_VE, "[VE]"},
{(int)KS_VS, "[VS]"},
{(int)KS_ME, "[ME]"},
{(int)KS_MR, "[MR]"},
{(int)KS_MB, "[MB]"},
{(int)KS_MD, "[MD]"},
{(int)KS_SE, "[SE]"},
{(int)KS_SO, "[SO]"},
{(int)KS_UE, "[UE]"},
{(int)KS_US, "[US]"},
{(int)KS_UCE, "[UCE]"},
{(int)KS_UCS, "[UCS]"},
{(int)KS_STE, "[STE]"},
{(int)KS_STS, "[STS]"},
{(int)KS_MS, "[MS]"},
{(int)KS_UT, "[UT]"},
{(int)KS_XN, "[XN]"},
#if defined(TERMINFO)
{(int)KS_CM, "[%p1%dCM%p2%d]"},
#else
{(int)KS_CM, "[%dCM%d]"},
#endif
{(int)KS_SR, "[SR]"},
#if defined(TERMINFO)
{(int)KS_CRI, "[CRI%p1%d]"},
#else
{(int)KS_CRI, "[CRI%d]"},
#endif
{(int)KS_VB, "[VB]"},
{(int)KS_KS, "[KS]"},
{(int)KS_KE, "[KE]"},
{(int)KS_TI, "[TI]"},
{(int)KS_TE, "[TE]"},
{(int)KS_CIS, "[CIS]"},
{(int)KS_CIE, "[CIE]"},
{(int)KS_CSC, "[CSC]"},
{(int)KS_CEC, "[CEC]"},
{(int)KS_TS, "[TS]"},
{(int)KS_FS, "[FS]"},
#if defined(TERMINFO)
{(int)KS_CWS, "[%p1%dCWS%p2%d]"},
{(int)KS_CWP, "[%p1%dCWP%p2%d]"},
#else
{(int)KS_CWS, "[%dCWS%d]"},
{(int)KS_CWP, "[%dCWP%d]"},
#endif
{(int)KS_CRV, "[CRV]"},
{(int)KS_U7, "[U7]"},
{(int)KS_RFG, "[RFG]"},
{(int)KS_RBG, "[RBG]"},
{K_UP, "[KU]"},
{K_DOWN, "[KD]"},
{K_LEFT, "[KL]"},
{K_RIGHT, "[KR]"},
{K_XUP, "[xKU]"},
{K_XDOWN, "[xKD]"},
{K_XLEFT, "[xKL]"},
{K_XRIGHT, "[xKR]"},
{K_S_UP, "[S-KU]"},
{K_S_DOWN, "[S-KD]"},
{K_S_LEFT, "[S-KL]"},
{K_C_LEFT, "[C-KL]"},
{K_S_RIGHT, "[S-KR]"},
{K_C_RIGHT, "[C-KR]"},
{K_F1, "[F1]"},
{K_XF1, "[xF1]"},
{K_F2, "[F2]"},
{K_XF2, "[xF2]"},
{K_F3, "[F3]"},
{K_XF3, "[xF3]"},
{K_F4, "[F4]"},
{K_XF4, "[xF4]"},
{K_F5, "[F5]"},
{K_F6, "[F6]"},
{K_F7, "[F7]"},
{K_F8, "[F8]"},
{K_F9, "[F9]"},
{K_F10, "[F10]"},
{K_F11, "[F11]"},
{K_F12, "[F12]"},
{K_S_F1, "[S-F1]"},
{K_S_XF1, "[S-xF1]"},
{K_S_F2, "[S-F2]"},
{K_S_XF2, "[S-xF2]"},
{K_S_F3, "[S-F3]"},
{K_S_XF3, "[S-xF3]"},
{K_S_F4, "[S-F4]"},
{K_S_XF4, "[S-xF4]"},
{K_S_F5, "[S-F5]"},
{K_S_F6, "[S-F6]"},
{K_S_F7, "[S-F7]"},
{K_S_F8, "[S-F8]"},
{K_S_F9, "[S-F9]"},
{K_S_F10, "[S-F10]"},
{K_S_F11, "[S-F11]"},
{K_S_F12, "[S-F12]"},
{K_HELP, "[HELP]"},
{K_UNDO, "[UNDO]"},
{K_BS, "[BS]"},
{K_INS, "[INS]"},
{K_KINS, "[KINS]"},
{K_DEL, "[DEL]"},
{K_KDEL, "[KDEL]"},
{K_HOME, "[HOME]"},
{K_S_HOME, "[C-HOME]"},
{K_C_HOME, "[C-HOME]"},
{K_KHOME, "[KHOME]"},
{K_XHOME, "[XHOME]"},
{K_ZHOME, "[ZHOME]"},
{K_END, "[END]"},
{K_S_END, "[C-END]"},
{K_C_END, "[C-END]"},
{K_KEND, "[KEND]"},
{K_XEND, "[XEND]"},
{K_ZEND, "[ZEND]"},
{K_PAGEUP, "[PAGEUP]"},
{K_PAGEDOWN, "[PAGEDOWN]"},
{K_KPAGEUP, "[KPAGEUP]"},
{K_KPAGEDOWN, "[KPAGEDOWN]"},
{K_MOUSE, "[MOUSE]"},
{K_KPLUS, "[KPLUS]"},
{K_KMINUS, "[KMINUS]"},
{K_KDIVIDE, "[KDIVIDE]"},
{K_KMULTIPLY, "[KMULTIPLY]"},
{K_KENTER, "[KENTER]"},
{K_KPOINT, "[KPOINT]"},
{K_PS, "[PASTE-START]"},
{K_PE, "[PASTE-END]"},
{K_K0, "[K0]"},
{K_K1, "[K1]"},
{K_K2, "[K2]"},
{K_K3, "[K3]"},
{K_K4, "[K4]"},
{K_K5, "[K5]"},
{K_K6, "[K6]"},
{K_K7, "[K7]"},
{K_K8, "[K8]"},
{K_K9, "[K9]"},
#endif
#endif 
{(int)KS_NAME, "dumb"},
{(int)KS_CL, "\014"},
#if defined(TERMINFO)
{(int)KS_CM, IF_EB("\033[%i%p1%d;%p2%dH",
ESC_STR "[%i%p1%d;%p2%dH")},
#else
{(int)KS_CM, IF_EB("\033[%i%d;%dH", ESC_STR "[%i%d;%dH")},
#endif
{(int)KS_NAME, NULL}
}; 
#if defined(FEAT_TERMGUICOLORS) || defined(PROTO)
static guicolor_T
termgui_mch_get_color(char_u *name)
{
return gui_get_color_cmn(name);
}
guicolor_T
termgui_get_color(char_u *name)
{
guicolor_T t;
if (*name == NUL)
return INVALCOLOR;
t = termgui_mch_get_color(name);
if (t == INVALCOLOR)
semsg(_("E254: Cannot allocate color %s"), name);
return t;
}
guicolor_T
termgui_mch_get_rgb(guicolor_T color)
{
return color;
}
#endif
#if defined(AMIGA)
#define DEFAULT_TERM (char_u *)"amiga"
#endif
#if defined(MSWIN)
#define DEFAULT_TERM (char_u *)"win32"
#endif
#if defined(UNIX) && !defined(__MINT__)
#define DEFAULT_TERM (char_u *)"ansi"
#endif
#if defined(__MINT__)
#define DEFAULT_TERM (char_u *)"vt52"
#endif
#if defined(VMS)
#define DEFAULT_TERM (char_u *)"vt320"
#endif
#if defined(__BEOS__)
#undef DEFAULT_TERM
#define DEFAULT_TERM (char_u *)"beos-ansi"
#endif
#if defined(__HAIKU__)
#undef DEFAULT_TERM
#define DEFAULT_TERM (char_u *)"xterm"
#endif
#if !defined(DEFAULT_TERM)
#define DEFAULT_TERM (char_u *)"dumb"
#endif
char_u *(term_strings[(int)KS_LAST + 1]);
static int need_gather = FALSE; 
static char_u termleader[256 + 1]; 
#if defined(FEAT_TERMRESPONSE)
static int check_for_codes = FALSE; 
static int is_not_xterm = FALSE; 
#endif
static struct builtin_term *
find_builtin_term(char_u *term)
{
struct builtin_term *p;
p = builtin_termcaps;
while (p->bt_string != NULL)
{
if (p->bt_entry == (int)KS_NAME)
{
#if defined(UNIX)
if (STRCMP(p->bt_string, "iris-ansi") == 0 && vim_is_iris(term))
return p;
else if (STRCMP(p->bt_string, "xterm") == 0 && vim_is_xterm(term))
return p;
else
#endif
#if defined(VMS)
if (STRCMP(p->bt_string, "vt320") == 0 && vim_is_vt300(term))
return p;
else
#endif
if (STRCMP(term, p->bt_string) == 0)
return p;
}
++p;
}
return p;
}
static void
parse_builtin_tcap(char_u *term)
{
struct builtin_term *p;
char_u name[2];
int term_8bit;
p = find_builtin_term(term);
term_8bit = term_is_8bit(term);
if (p->bt_string == NULL)
return;
for (++p; p->bt_entry != (int)KS_NAME && p->bt_entry != BT_EXTRA_KEYS; ++p)
{
if ((int)p->bt_entry >= 0) 
{
if (term_strings[p->bt_entry] == NULL
|| term_strings[p->bt_entry] == empty_option)
{
#if defined(FEAT_EVAL)
int opt_idx = -1;
#endif
if (term_8bit && term_7to8bit((char_u *)p->bt_string) != 0)
{
char_u *s, *t;
s = vim_strsave((char_u *)p->bt_string);
if (s != NULL)
{
for (t = s; *t; ++t)
if (term_7to8bit(t))
{
*t = term_7to8bit(t);
STRMOVE(t + 1, t + 2);
}
term_strings[p->bt_entry] = s;
#if defined(FEAT_EVAL)
opt_idx =
#endif
set_term_option_alloced(
&term_strings[p->bt_entry]);
}
}
else
{
term_strings[p->bt_entry] = (char_u *)p->bt_string;
#if defined(FEAT_EVAL)
opt_idx = get_term_opt_idx(&term_strings[p->bt_entry]);
#endif
}
#if defined(FEAT_EVAL)
set_term_option_sctx_idx(NULL, opt_idx);
#endif
}
}
else
{
name[0] = KEY2TERMCAP0((int)p->bt_entry);
name[1] = KEY2TERMCAP1((int)p->bt_entry);
if (find_termcode(name) == NULL)
add_termcode(name, (char_u *)p->bt_string, term_8bit);
}
}
}
static void
set_color_count(int nr)
{
char_u nr_colors[20]; 
t_colors = nr;
if (t_colors > 1)
sprintf((char *)nr_colors, "%d", t_colors);
else
*nr_colors = NUL;
set_string_option_direct((char_u *)"t_Co", -1, nr_colors, OPT_FREE, 0);
}
#if defined(FEAT_TERMRESPONSE)
static void
may_adjust_color_count(int val)
{
if (val != t_colors)
{
set_keep_msg_from_hist();
set_color_count(val);
init_highlight(TRUE, FALSE);
#if defined(DEBUG_TERMRESPONSE)
{
int r = redraw_asap(CLEAR);
log_tr("Received t_Co, redraw_asap(): %d", r);
}
#else
redraw_asap(CLEAR);
#endif
}
}
#endif
#if defined(HAVE_TGETENT)
static char *(key_names[]) =
{
#if defined(FEAT_TERMRESPONSE)
"Co",
#endif
"ku", "kd", "kr", "kl",
"#2", "#4", "%i", "*7",
"k1", "k2", "k3", "k4", "k5", "k6",
"k7", "k8", "k9", "k;", "F1", "F2",
"%1", "&8", "kb", "kI", "kD", "kh",
"@7", "kP", "kN", "K1", "K3", "K4", "K5", "kB",
NULL
};
#endif
#if defined(HAVE_TGETENT)
static void
get_term_entries(int *height, int *width)
{
static struct {
enum SpecialKey dest; 
char *name; 
} string_names[] =
{ {KS_CE, "ce"}, {KS_AL, "al"}, {KS_CAL,"AL"},
{KS_DL, "dl"}, {KS_CDL,"DL"}, {KS_CS, "cs"},
{KS_CL, "cl"}, {KS_CD, "cd"},
{KS_VI, "vi"}, {KS_VE, "ve"}, {KS_MB, "mb"},
{KS_ME, "me"}, {KS_MR, "mr"},
{KS_MD, "md"}, {KS_SE, "se"}, {KS_SO, "so"},
{KS_CZH,"ZH"}, {KS_CZR,"ZR"}, {KS_UE, "ue"},
{KS_US, "us"}, {KS_UCE, "Ce"}, {KS_UCS, "Cs"},
{KS_STE,"Te"}, {KS_STS,"Ts"},
{KS_CM, "cm"}, {KS_SR, "sr"},
{KS_CRI,"RI"}, {KS_VB, "vb"}, {KS_KS, "ks"},
{KS_KE, "ke"}, {KS_TI, "ti"}, {KS_TE, "te"},
{KS_CTI, "TI"}, {KS_CTE, "TE"},
{KS_BC, "bc"}, {KS_CSB,"Sb"}, {KS_CSF,"Sf"},
{KS_CAB,"AB"}, {KS_CAF,"AF"}, {KS_LE, "le"},
{KS_ND, "nd"}, {KS_OP, "op"}, {KS_CRV, "RV"},
{KS_VS, "vs"}, {KS_CVS, "VS"},
{KS_CIS, "IS"}, {KS_CIE, "IE"},
{KS_CSC, "SC"}, {KS_CEC, "EC"},
{KS_TS, "ts"}, {KS_FS, "fs"},
{KS_CWP, "WP"}, {KS_CWS, "WS"},
{KS_CSI, "SI"}, {KS_CEI, "EI"},
{KS_U7, "u7"}, {KS_RFG, "RF"}, {KS_RBG, "RB"},
{KS_8F, "8f"}, {KS_8B, "8b"},
{KS_CBE, "BE"}, {KS_CBD, "BD"},
{KS_CPS, "PS"}, {KS_CPE, "PE"},
{KS_CST, "ST"}, {KS_CRT, "RT"},
{KS_SSI, "Si"}, {KS_SRI, "Ri"},
{(enum SpecialKey)0, NULL}
};
int i;
char_u *p;
static char_u tstrbuf[TBUFSZ];
char_u *tp = tstrbuf;
for (i = 0; string_names[i].name != NULL; ++i)
{
if (TERM_STR(string_names[i].dest) == NULL
|| TERM_STR(string_names[i].dest) == empty_option)
{
TERM_STR(string_names[i].dest) = TGETSTR(string_names[i].name, &tp);
#if defined(FEAT_EVAL)
set_term_option_sctx_idx(string_names[i].name, -1);
#endif
}
}
if ((T_MS == NULL || T_MS == empty_option) && tgetflag("ms") > 0)
T_MS = (char_u *)"y";
if ((T_XS == NULL || T_XS == empty_option) && tgetflag("xs") > 0)
T_XS = (char_u *)"y";
if ((T_XN == NULL || T_XN == empty_option) && tgetflag("xn") > 0)
T_XN = (char_u *)"y";
if ((T_DB == NULL || T_DB == empty_option) && tgetflag("db") > 0)
T_DB = (char_u *)"y";
if ((T_DA == NULL || T_DA == empty_option) && tgetflag("da") > 0)
T_DA = (char_u *)"y";
if ((T_UT == NULL || T_UT == empty_option) && tgetflag("ut") > 0)
T_UT = (char_u *)"y";
for (i = 0; key_names[i] != NULL; ++i)
if (find_termcode((char_u *)key_names[i]) == NULL)
{
p = TGETSTR(key_names[i], &tp);
if (p != NULL
&& (*p != Ctrl_H
|| key_names[i][0] != 'k'
|| key_names[i][1] != 'l'))
add_termcode((char_u *)key_names[i], p, FALSE);
}
if (*height == 0)
*height = tgetnum("li");
if (*width == 0)
*width = tgetnum("co");
if (TERM_STR(KS_CCO) == NULL || TERM_STR(KS_CCO) == empty_option)
{
set_color_count(tgetnum("Co"));
#if defined(FEAT_EVAL)
set_term_option_sctx_idx("Co", -1);
#endif
}
#if !defined(hpux)
BC = (char *)TGETSTR("bc", &tp);
UP = (char *)TGETSTR("up", &tp);
p = TGETSTR("pc", &tp);
if (p)
PC = *p;
#endif
}
#endif
static void
report_term_error(char *error_msg, char_u *term)
{
struct builtin_term *termp;
mch_errmsg("\r\n");
if (error_msg != NULL)
{
mch_errmsg(error_msg);
mch_errmsg("\r\n");
}
mch_errmsg("'");
mch_errmsg((char *)term);
mch_errmsg(_("' not known. Available builtin terminals are:"));
mch_errmsg("\r\n");
for (termp = &(builtin_termcaps[0]); termp->bt_string != NULL; ++termp)
{
if (termp->bt_entry == (int)KS_NAME)
{
#if defined(HAVE_TGETENT)
mch_errmsg(" builtin_");
#else
mch_errmsg(" ");
#endif
mch_errmsg(termp->bt_string);
mch_errmsg("\r\n");
}
}
}
static void
report_default_term(char_u *term)
{
mch_errmsg(_("defaulting to '"));
mch_errmsg((char *)term);
mch_errmsg("'\r\n");
if (emsg_silent == 0)
{
screen_start(); 
out_flush();
if (!is_not_a_term())
ui_delay(2007L, TRUE);
}
}
int
set_termname(char_u *term)
{
struct builtin_term *termp;
#if defined(HAVE_TGETENT)
int builtin_first = p_tbi;
int try;
int termcap_cleared = FALSE;
#endif
int width = 0, height = 0;
char *error_msg = NULL;
char_u *bs_p, *del_p;
if (silent_mode)
return OK;
detected_8bit = FALSE; 
if (term_is_builtin(term))
{
term += 8;
#if defined(HAVE_TGETENT)
builtin_first = 1;
#endif
}
#if defined(HAVE_TGETENT)
for (try = builtin_first ? 0 : 1; try < 3; ++try)
{
if (try == 1)
{
char_u tbuf[TBUFSZ];
if ((error_msg = tgetent_error(tbuf, term)) == NULL)
{
if (!termcap_cleared)
{
clear_termoptions(); 
termcap_cleared = TRUE;
}
get_term_entries(&height, &width);
}
}
else 
#endif 
{
#if defined(HAVE_TGETENT)
if (try == 2 && builtin_first && termcap_cleared)
break;
#endif
termp = find_builtin_term(term);
if (termp->bt_string == NULL) 
{
#if defined(HAVE_TGETENT)
if (try == 0) 
continue;
if (termcap_cleared) 
break;
#endif
report_term_error(error_msg, term);
if (starting != NO_SCREEN)
{
screen_start(); 
wait_return(TRUE);
return FAIL;
}
term = DEFAULT_TERM;
report_default_term(term);
set_string_option_direct((char_u *)"term", -1, term,
OPT_FREE, 0);
display_errors();
}
out_flush();
#if defined(HAVE_TGETENT)
if (!termcap_cleared)
{
#endif
clear_termoptions(); 
#if defined(HAVE_TGETENT)
termcap_cleared = TRUE;
}
#endif
parse_builtin_tcap(term);
#if defined(FEAT_GUI)
if (term_is_gui(term))
{
out_flush();
gui_init();
if (!gui.in_use)
return FAIL;
#if defined(HAVE_TGETENT)
break; 
#endif
}
#endif 
}
#if defined(HAVE_TGETENT)
}
#endif
if (STRCMP(term, "pcterm") == 0)
T_CCS = (char_u *)"yes";
else
T_CCS = empty_option;
#if defined(UNIX)
#if defined(FEAT_GUI)
if (!gui.in_use)
#endif
get_stty();
#endif
#if defined(FEAT_GUI)
if (!gui.in_use)
#endif
{
bs_p = find_termcode((char_u *)"kb");
del_p = find_termcode((char_u *)"kD");
if (bs_p == NULL || *bs_p == NUL)
add_termcode((char_u *)"kb", (bs_p = (char_u *)CTRL_H_STR), FALSE);
if ((del_p == NULL || *del_p == NUL) &&
(bs_p == NULL || *bs_p != DEL))
add_termcode((char_u *)"kD", (char_u *)DEL_STR, FALSE);
}
#if defined(UNIX) || defined(VMS)
term_is_xterm = vim_is_xterm(term);
#endif
#if defined(FEAT_TERMRESPONSE)
is_not_xterm = FALSE;
is_mac_terminal = FALSE;
#endif
#if defined(UNIX) || defined(VMS)
{
char_u *p = (char_u *)"";
#if defined(FEAT_MOUSE_XTERM)
if (use_xterm_like_mouse(term))
{
if (use_xterm_mouse())
p = NULL; 
else
p = (char_u *)"xterm";
}
#endif
if (p != NULL)
{
set_option_value((char_u *)"ttym", 0L, p, 0);
reset_option_was_set((char_u *)"ttym");
}
if (p == NULL
#if defined(FEAT_GUI)
|| gui.in_use
#endif
)
check_mouse_termcode(); 
}
#else
set_mouse_termcode(KS_MOUSE, (char_u *)"\233M");
#endif
#if defined(USE_TERM_CONSOLE)
if (STRCMP(term, DEFAULT_TERM) != 0)
term_console = FALSE;
else
{
term_console = TRUE;
#if defined(AMIGA)
win_resize_on(); 
#endif
}
#endif
#if defined(UNIX) || defined(VMS)
if (vim_is_fastterm(term))
p_tf = TRUE;
#endif
#if defined(USE_TERM_CONSOLE)
if (term_console)
p_tf = TRUE;
#endif
ttest(TRUE); 
full_screen = TRUE; 
set_term_defaults(); 
#if defined(FEAT_TERMRESPONSE)
LOG_TR(("setting crv_status to STATUS_GET"));
crv_status.tr_progress = STATUS_GET; 
#endif
if (starting != NO_SCREEN)
{
starttermcap(); 
setmouse(); 
#if defined(FEAT_TITLE)
maketitle(); 
#endif
}
if (width <= 0 || height <= 0)
{
width = 80;
#if defined(MSWIN)
height = 25; 
#else
height = 24; 
#endif
}
set_shellsize(width, height, FALSE); 
if (starting != NO_SCREEN)
{
if (scroll_region)
scroll_region_reset(); 
check_map_keycodes(); 
{
buf_T *buf;
aco_save_T aco;
FOR_ALL_BUFFERS(buf)
{
if (curbuf->b_ml.ml_mfp != NULL)
{
aucmd_prepbuf(&aco, buf);
apply_autocmds(EVENT_TERMCHANGED, NULL, NULL, FALSE,
curbuf);
aucmd_restbuf(&aco);
}
}
}
}
#if defined(FEAT_TERMRESPONSE)
may_req_termresponse();
#endif
return OK;
}
#if defined(HAVE_TGETENT)
static char *
tgetent_error(char_u *tbuf, char_u *term)
{
int i;
i = TGETENT(tbuf, term);
if (i < 0 
#if defined(TGETENT_ZERO_ERR)
|| i == 0 
#endif
)
{
(void)TGETENT(tbuf, "dumb");
if (i < 0)
#if defined(TGETENT_ZERO_ERR)
return _("E557: Cannot open termcap file");
if (i == 0)
#endif
#if defined(TERMINFO)
return _("E558: Terminal entry not found in terminfo");
#else
return _("E559: Terminal entry not found in termcap");
#endif
}
return NULL;
}
static char_u *
vim_tgetstr(char *s, char_u **pp)
{
char *p;
p = tgetstr(s, (char **)pp);
if (p == (char *)-1)
p = NULL;
return (char_u *)p;
}
#endif 
#if defined(HAVE_TGETENT) && (defined(UNIX) || defined(VMS) || defined(MACOS_X))
void
getlinecol(
long *cp, 
long *rp) 
{
char_u tbuf[TBUFSZ];
if (T_NAME != NULL && *T_NAME != NUL && tgetent_error(tbuf, T_NAME) == NULL)
{
if (*cp == 0)
*cp = tgetnum("co");
if (*rp == 0)
*rp = tgetnum("li");
}
}
#endif 
int
add_termcap_entry(char_u *name, int force)
{
char_u *term;
int key;
struct builtin_term *termp;
#if defined(HAVE_TGETENT)
char_u *string;
int i;
int builtin_first;
char_u tbuf[TBUFSZ];
char_u tstrbuf[TBUFSZ];
char_u *tp = tstrbuf;
char *error_msg = NULL;
#endif
#if defined(FEAT_GUI)
if (gui.in_use || gui.starting)
return gui_mch_haskey(name);
#endif
if (!force && find_termcode(name) != NULL) 
return OK;
term = T_NAME;
if (term == NULL || *term == NUL) 
return FAIL;
if (term_is_builtin(term)) 
{
term += 8;
#if defined(HAVE_TGETENT)
builtin_first = TRUE;
#endif
}
#if defined(HAVE_TGETENT)
else
builtin_first = p_tbi;
#endif
#if defined(HAVE_TGETENT)
for (i = 0; i < 2; ++i)
{
if ((!builtin_first) == i)
#endif
{
termp = find_builtin_term(term);
if (termp->bt_string != NULL) 
{
key = TERMCAP2KEY(name[0], name[1]);
++termp;
while (termp->bt_entry != (int)KS_NAME)
{
if ((int)termp->bt_entry == key)
{
add_termcode(name, (char_u *)termp->bt_string,
term_is_8bit(term));
return OK;
}
++termp;
}
}
}
#if defined(HAVE_TGETENT)
else
{
error_msg = tgetent_error(tbuf, term);
if (error_msg == NULL)
{
string = TGETSTR((char *)name, &tp);
if (string != NULL && *string != NUL)
{
add_termcode(name, string, FALSE);
return OK;
}
}
}
}
#endif
if (SOURCING_NAME == NULL)
{
#if defined(HAVE_TGETENT)
if (error_msg != NULL)
emsg(error_msg);
else
#endif
semsg(_("E436: No \"%s\" entry in termcap"), name);
}
return FAIL;
}
static int
term_is_builtin(char_u *name)
{
return (STRNCMP(name, "builtin_", (size_t)8) == 0);
}
int
term_is_8bit(char_u *name)
{
return (detected_8bit || strstr((char *)name, "8bit") != NULL);
}
static int
term_7to8bit(char_u *p)
{
if (*p == ESC)
{
if (p[1] == '[')
return CSI;
if (p[1] == ']')
return OSC;
if (p[1] == 'O')
return 0x8f;
}
return 0;
}
#if defined(FEAT_GUI) || defined(PROTO)
int
term_is_gui(char_u *name)
{
return (STRCMP(name, "builtin_gui") == 0 || STRCMP(name, "gui") == 0);
}
#endif
#if !defined(HAVE_TGETENT) || defined(AMIGA) || defined(PROTO)
char_u *
tltoa(unsigned long i)
{
static char_u buf[16];
char_u *p;
p = buf + 15;
*p = '\0';
do
{
--p;
*p = (char_u) (i % 10 + '0');
i /= 10;
}
while (i > 0 && p > buf);
return p;
}
#endif
#if !defined(HAVE_TGETENT)
static char *
tgoto(char *cm, int x, int y)
{
static char buf[30];
char *p, *s, *e;
if (!cm)
return "OOPS";
e = buf + 29;
for (s = buf; s < e && *cm; cm++)
{
if (*cm != '%')
{
*s++ = *cm;
continue;
}
switch (*++cm)
{
case 'd':
p = (char *)tltoa((unsigned long)y);
y = x;
while (*p)
*s++ = *p++;
break;
case 'i':
x++;
y++;
break;
case '+':
*s++ = (char)(*++cm + y);
y = x;
break;
case '%':
*s++ = *cm;
break;
default:
return "OOPS";
}
}
*s = '\0';
return buf;
}
#endif 
void
termcapinit(char_u *name)
{
char_u *term;
if (name != NULL && *name == NUL)
name = NULL; 
term = name;
#if defined(__BEOS__)
if (term == NULL
&& strcmp((char *)mch_getenv((char_u *)"TERM"), "ansi") == 0)
term = DEFAULT_TERM;
#endif
#if !defined(MSWIN)
if (term == NULL)
term = mch_getenv((char_u *)"TERM");
#endif
if (term == NULL || *term == NUL)
term = DEFAULT_TERM;
set_string_option_direct((char_u *)"term", -1, term, OPT_FREE, 0);
set_string_default("term", term);
set_string_default("ttytype", term);
set_termname(T_NAME != NULL ? T_NAME : term);
}
#define OUT_SIZE 2047
static char_u out_buf[OUT_SIZE + 1];
static int out_pos = 0; 
#define MAX_ESC_SEQ_LEN 80
void
out_flush(void)
{
int len;
if (out_pos != 0)
{
len = out_pos;
out_pos = 0;
ui_write(out_buf, len);
}
}
void
out_flush_cursor(
int force UNUSED, 
int clear_selection UNUSED) 
{
mch_disable_flush();
out_flush();
mch_enable_flush();
#if defined(FEAT_GUI)
if (gui.in_use)
{
gui_update_cursor(force, clear_selection);
gui_may_flush();
}
#endif
}
void
out_flush_check(void)
{
if (enc_dbcs != 0 && out_pos >= OUT_SIZE - MB_MAXBYTES)
out_flush();
}
#if defined(FEAT_GUI)
void
out_trash(void)
{
out_pos = 0;
}
#endif
void
out_char(unsigned c)
{
#if defined(UNIX) || defined(VMS) || defined(AMIGA) || defined(MACOS_X)
if (c == '\n') 
out_char('\r');
#endif
out_buf[out_pos++] = c;
if (out_pos >= OUT_SIZE || p_wd)
out_flush();
}
static void
out_char_nf(unsigned c)
{
out_buf[out_pos++] = c;
if (out_pos >= OUT_SIZE)
out_flush();
}
void
out_str_nf(char_u *s)
{
if (out_pos > OUT_SIZE - MAX_ESC_SEQ_LEN)
out_flush();
while (*s)
out_char_nf(*s++);
if (p_wd)
out_flush();
}
void
out_str_cf(char_u *s)
{
if (s != NULL && *s)
{
#if defined(HAVE_TGETENT)
char_u *p;
#endif
#if defined(FEAT_GUI)
if (gui.in_use)
{
out_str_nf(s);
return;
}
#endif
if (out_pos > OUT_SIZE - MAX_ESC_SEQ_LEN)
out_flush();
#if defined(HAVE_TGETENT)
for (p = s; *s; ++s)
{
if (*s == '$' && *(s + 1) == '<')
{
char_u save_c = *s;
int duration = atoi((char *)s + 2);
*s = NUL;
tputs((char *)p, 1, TPUTSFUNCAST out_char_nf);
*s = save_c;
out_flush();
#if defined(ELAPSED_FUNC)
p = vim_strchr(s, '>');
if (p == NULL || duration <= 0)
{
p = s;
}
else
{
++p;
do_sleep(duration);
}
#else
p = s;
#endif
break;
}
}
tputs((char *)p, 1, TPUTSFUNCAST out_char_nf);
#else
while (*s)
out_char_nf(*s++);
#endif
if (p_wd)
out_flush();
}
}
void
out_str(char_u *s)
{
if (s != NULL && *s)
{
#if defined(FEAT_GUI)
if (gui.in_use)
{
out_str_nf(s);
return;
}
#endif
if (out_pos > OUT_SIZE - MAX_ESC_SEQ_LEN)
out_flush();
#if defined(HAVE_TGETENT)
tputs((char *)s, 1, TPUTSFUNCAST out_char_nf);
#else
while (*s)
out_char_nf(*s++);
#endif
if (p_wd)
out_flush();
}
}
void
term_windgoto(int row, int col)
{
OUT_STR(tgoto((char *)T_CM, col, row));
}
void
term_cursor_right(int i)
{
OUT_STR(tgoto((char *)T_CRI, 0, i));
}
void
term_append_lines(int line_count)
{
OUT_STR(tgoto((char *)T_CAL, 0, line_count));
}
void
term_delete_lines(int line_count)
{
OUT_STR(tgoto((char *)T_CDL, 0, line_count));
}
#if defined(HAVE_TGETENT) || defined(PROTO)
void
term_set_winpos(int x, int y)
{
if (x < 0)
x = 0;
if (y < 0)
y = 0;
OUT_STR(tgoto((char *)T_CWP, y, x));
}
#if defined(FEAT_TERMRESPONSE) || defined(PROTO)
static int
can_get_termresponse()
{
return cur_tmode == TMODE_RAW
&& termcap_active
#if defined(UNIX)
&& (is_not_a_term() || (isatty(1) && isatty(read_cmd_fd)))
#endif
&& p_ek;
}
static void
termrequest_sent(termrequest_T *status)
{
status->tr_progress = STATUS_SENT;
status->tr_start = time(NULL);
}
static int
termrequest_any_pending()
{
int i;
time_t now = time(NULL);
for (i = 0; all_termrequests[i] != NULL; ++i)
{
if (all_termrequests[i]->tr_progress == STATUS_SENT)
{
if (all_termrequests[i]->tr_start > 0 && now > 0
&& all_termrequests[i]->tr_start + 2 < now)
all_termrequests[i]->tr_progress = STATUS_FAIL;
else
return TRUE;
}
}
return FALSE;
}
static int winpos_x = -1;
static int winpos_y = -1;
static int did_request_winpos = 0;
#if defined(FEAT_EVAL) || defined(FEAT_TERMINAL) || defined(PROTO)
int
term_get_winpos(int *x, int *y, varnumber_T timeout)
{
int count = 0;
int prev_winpos_x = winpos_x;
int prev_winpos_y = winpos_y;
if (*T_CGP == NUL || !can_get_termresponse())
return FAIL;
winpos_x = -1;
winpos_y = -1;
++did_request_winpos;
termrequest_sent(&winpos_status);
OUT_STR(T_CGP);
out_flush();
while (count++ <= timeout / 10 && !got_int)
{
(void)vpeekc_nomap();
if (winpos_x >= 0 && winpos_y >= 0)
{
*x = winpos_x;
*y = winpos_y;
return OK;
}
ui_delay(10L, FALSE);
}
winpos_x = prev_winpos_x;
winpos_y = prev_winpos_y;
if (timeout < 10 && prev_winpos_y >= 0 && prev_winpos_x >= 0)
{
*x = winpos_x;
*y = winpos_y;
return OK;
}
return FALSE;
}
#endif
#endif
void
term_set_winsize(int height, int width)
{
OUT_STR(tgoto((char *)T_CWS, width, height));
}
#endif
static void
term_color(char_u *s, int n)
{
char buf[20];
int i = *s == CSI ? 1 : 2;
if (n >= 8 && t_colors >= 16
&& ((s[0] == ESC && s[1] == '[')
#if defined(FEAT_VTP) && defined(FEAT_TERMGUICOLORS)
|| (s[0] == ESC && s[1] == '|')
#endif
|| (s[0] == CSI && (i = 1) == 1))
&& s[i] != NUL
&& (STRCMP(s + i + 1, "%p1%dm") == 0
|| STRCMP(s + i + 1, "%dm") == 0)
&& (s[i] == '3' || s[i] == '4'))
{
#if defined(TERMINFO)
char *format = "%s%s%%p1%%dm";
#else
char *format = "%s%s%%dm";
#endif
char *lead = i == 2 ? (
#if defined(FEAT_VTP) && defined(FEAT_TERMGUICOLORS)
s[1] == '|' ? IF_EB("\033|", ESC_STR "|") :
#endif
IF_EB("\033[", ESC_STR "[")) : "\233";
char *tail = s[i] == '3' ? (n >= 16 ? "38;5;" : "9")
: (n >= 16 ? "48;5;" : "10");
sprintf(buf, format, lead, tail);
OUT_STR(tgoto(buf, 0, n >= 16 ? n : n - 8));
}
else
OUT_STR(tgoto((char *)s, 0, n));
}
void
term_fg_color(int n)
{
if (*T_CAF)
term_color(T_CAF, n);
else if (*T_CSF)
term_color(T_CSF, n);
}
void
term_bg_color(int n)
{
if (*T_CAB)
term_color(T_CAB, n);
else if (*T_CSB)
term_color(T_CSB, n);
}
char_u *
term_bg_default(void)
{
#if defined(MSWIN)
return (char_u *)"dark";
#else
char_u *p;
if (STRCMP(T_NAME, "linux") == 0
|| STRCMP(T_NAME, "screen.linux") == 0
|| STRNCMP(T_NAME, "cygwin", 6) == 0
|| STRNCMP(T_NAME, "putty", 5) == 0
|| ((p = mch_getenv((char_u *)"COLORFGBG")) != NULL
&& (p = vim_strrchr(p, ';')) != NULL
&& ((p[1] >= '0' && p[1] <= '6') || p[1] == '8')
&& p[2] == NUL))
return (char_u *)"dark";
return (char_u *)"light";
#endif
}
#if defined(FEAT_TERMGUICOLORS) || defined(PROTO)
#define RED(rgb) (((long_u)(rgb) >> 16) & 0xFF)
#define GREEN(rgb) (((long_u)(rgb) >> 8) & 0xFF)
#define BLUE(rgb) (((long_u)(rgb) ) & 0xFF)
static void
term_rgb_color(char_u *s, guicolor_T rgb)
{
#define MAX_COLOR_STR_LEN 100
char buf[MAX_COLOR_STR_LEN];
vim_snprintf(buf, MAX_COLOR_STR_LEN,
(char *)s, RED(rgb), GREEN(rgb), BLUE(rgb));
OUT_STR(buf);
}
void
term_fg_rgb_color(guicolor_T rgb)
{
term_rgb_color(T_8F, rgb);
}
void
term_bg_rgb_color(guicolor_T rgb)
{
term_rgb_color(T_8B, rgb);
}
#endif
#if (defined(FEAT_TITLE) && (defined(UNIX) || defined(VMS) || defined(MACOS_X))) || defined(PROTO)
void
term_settitle(char_u *title)
{
OUT_STR(tgoto((char *)T_TS, 0, 0)); 
out_str_nf(title);
out_str(T_FS); 
out_flush();
}
void
term_push_title(int which)
{
if ((which & SAVE_RESTORE_TITLE) && T_CST != NULL && *T_CST != NUL)
{
OUT_STR(T_CST);
out_flush();
}
if ((which & SAVE_RESTORE_ICON) && T_SSI != NULL && *T_SSI != NUL)
{
OUT_STR(T_SSI);
out_flush();
}
}
void
term_pop_title(int which)
{
if ((which & SAVE_RESTORE_TITLE) && T_CRT != NULL && *T_CRT != NUL)
{
OUT_STR(T_CRT);
out_flush();
}
if ((which & SAVE_RESTORE_ICON) && T_SRI != NULL && *T_SRI != NUL)
{
OUT_STR(T_SRI);
out_flush();
}
}
#endif
void
ttest(int pairs)
{
char_u *env_colors;
check_options(); 
if (*T_CM == NUL)
emsg(_("E437: terminal capability \"cm\" required"));
if (*T_CS != NUL)
scroll_region = TRUE;
else
scroll_region = FALSE;
if (pairs)
{
if (*T_ME == NUL)
T_ME = T_MR = T_MD = T_MB = empty_option;
if (*T_SO == NUL || *T_SE == NUL)
T_SO = T_SE = empty_option;
if (*T_US == NUL || *T_UE == NUL)
T_US = T_UE = empty_option;
if (*T_CZH == NUL || *T_CZR == NUL)
T_CZH = T_CZR = empty_option;
if (*T_VE == NUL)
T_VI = empty_option;
if (*T_ME == NUL)
{
T_ME = T_SE;
T_MR = T_SO;
T_MD = T_SO;
}
if (*T_SO == NUL)
{
T_SE = T_ME;
if (*T_MR == NUL)
T_SO = T_MD;
else
T_SO = T_MR;
}
if (*T_CZH == NUL)
{
T_CZR = T_ME;
if (*T_MR == NUL)
T_CZH = T_MD;
else
T_CZH = T_MR;
}
if (*T_CSB == NUL || *T_CSF == NUL)
{
T_CSB = empty_option;
T_CSF = empty_option;
}
if (*T_CAB == NUL || *T_CAF == NUL)
{
T_CAB = empty_option;
T_CAF = empty_option;
}
if (*T_CSB == NUL && *T_CAB == NUL)
free_one_termoption(T_CCO);
p_wiv = (*T_XS != NUL);
}
need_gather = TRUE;
t_colors = atoi((char *)T_CCO);
env_colors = mch_getenv((char_u *)"COLORS");
if (env_colors != NULL && isdigit(*env_colors))
{
int colors = atoi((char *)env_colors);
if (colors != t_colors)
set_color_count(colors);
}
}
#if (defined(FEAT_GUI) && (defined(FEAT_MENU) || !defined(USE_ON_FLY_SCROLL))) || defined(PROTO)
void
add_long_to_buf(long_u val, char_u *dst)
{
int i;
int shift;
for (i = 1; i <= (int)sizeof(long_u); i++)
{
shift = 8 * (sizeof(long_u) - i);
dst[i - 1] = (char_u) ((val >> shift) & 0xff);
}
}
static int
get_long_from_buf(char_u *buf, long_u *val)
{
int len;
char_u bytes[sizeof(long_u)];
int i;
int shift;
*val = 0;
len = get_bytes_from_buf(buf, bytes, (int)sizeof(long_u));
if (len != -1)
{
for (i = 0; i < (int)sizeof(long_u); i++)
{
shift = 8 * (sizeof(long_u) - 1 - i);
*val += (long_u)bytes[i] << shift;
}
}
return len;
}
#endif
int
get_bytes_from_buf(char_u *buf, char_u *bytes, int num_bytes)
{
int len = 0;
int i;
char_u c;
for (i = 0; i < num_bytes; i++)
{
if ((c = buf[len++]) == NUL)
return -1;
if (c == K_SPECIAL)
{
if (buf[len] == NUL || buf[len + 1] == NUL) 
return -1;
if (buf[len++] == (int)KS_ZERO)
c = NUL;
if (buf[len++] == (int)KE_CSI)
c = CSI;
}
else if (c == CSI && buf[len] == KS_EXTRA
&& buf[len + 1] == (int)KE_CSI)
len += 2;
bytes[i] = c;
}
return len;
}
void
check_shellsize(void)
{
if (Rows < min_rows()) 
Rows = min_rows();
limit_screen_size();
}
void
limit_screen_size(void)
{
if (Columns < MIN_COLUMNS)
Columns = MIN_COLUMNS;
else if (Columns > 10000)
Columns = 10000;
if (Rows > 1000)
Rows = 1000;
}
void
win_new_shellsize(void)
{
static int old_Rows = 0;
static int old_Columns = 0;
if (old_Rows != Rows || old_Columns != Columns)
ui_new_shellsize();
if (old_Rows != Rows)
{
if (p_window == old_Rows - 1 || old_Rows == 0)
p_window = Rows - 1;
old_Rows = Rows;
shell_new_rows(); 
}
if (old_Columns != Columns)
{
old_Columns = Columns;
shell_new_columns(); 
}
}
void
shell_resized(void)
{
set_shellsize(0, 0, FALSE);
}
void
shell_resized_check(void)
{
int old_Rows = Rows;
int old_Columns = Columns;
if (!exiting
#if defined(FEAT_GUI)
&& !gui.starting
#endif
)
{
(void)ui_get_shellsize();
check_shellsize();
if (old_Rows != Rows || old_Columns != Columns)
shell_resized();
}
}
void
set_shellsize(int width, int height, int mustset)
{
static int busy = FALSE;
if (busy)
return;
if (width < 0 || height < 0) 
return;
if (State == HITRETURN || State == SETWSIZE)
{
State = SETWSIZE;
return;
}
if (updating_screen)
return;
if (curwin->w_buffer == NULL)
return;
++busy;
#if defined(AMIGA)
out_flush(); 
#endif
if (mustset || (ui_get_shellsize() == FAIL && height != 0))
{
Rows = height;
Columns = width;
check_shellsize();
ui_set_shellsize(mustset);
}
else
check_shellsize();
if (State != ASKMORE && State != EXTERNCMD && State != CONFIRM)
screenclear();
else
screen_start(); 
if (starting != NO_SCREEN)
{
#if defined(FEAT_TITLE)
maketitle();
#endif
changed_line_abv_curs();
invalidate_botline();
if (State == ASKMORE || State == EXTERNCMD || State == CONFIRM
|| exmode_active)
{
screenalloc(FALSE);
repeat_message();
}
else
{
if (curwin->w_p_scb)
do_check_scrollbind(TRUE);
if (State & CMDLINE)
{
update_screen(NOT_VALID);
redrawcmdline();
}
else
{
update_topline();
if (pum_visible())
{
redraw_later(NOT_VALID);
ins_compl_show_pum();
}
update_screen(NOT_VALID);
if (redrawing())
setcursor();
}
}
cursor_on(); 
}
out_flush();
--busy;
}
void
settmode(int tmode)
{
#if defined(FEAT_GUI)
if (gui.in_use)
return;
#endif
if (full_screen)
{
if (tmode != TMODE_COOK || cur_tmode != TMODE_COOK)
{
#if defined(FEAT_TERMRESPONSE)
#if defined(FEAT_GUI)
if (!gui.in_use && !gui.starting)
#endif
{
if (tmode != TMODE_RAW && termrequest_any_pending())
(void)vpeekc_nomap();
check_for_codes_from_term();
}
#endif
if (tmode != TMODE_RAW)
mch_setmouse(FALSE); 
if (termcap_active)
{
if (tmode != TMODE_RAW)
out_str(T_BD); 
else
out_str(T_BE); 
}
out_flush();
mch_settmode(tmode); 
cur_tmode = tmode;
if (tmode == TMODE_RAW)
setmouse(); 
out_flush();
}
#if defined(FEAT_TERMRESPONSE)
may_req_termresponse();
#endif
}
}
void
starttermcap(void)
{
if (full_screen && !termcap_active)
{
out_str(T_TI); 
out_str(T_CTI); 
out_str(T_KS); 
out_str(T_BE); 
out_flush();
termcap_active = TRUE;
screen_start(); 
#if defined(FEAT_TERMRESPONSE)
#if defined(FEAT_GUI)
if (!gui.in_use && !gui.starting)
#endif
{
may_req_termresponse();
if (crv_status.tr_progress == STATUS_SENT)
check_for_codes_from_term();
}
#endif
}
}
void
stoptermcap(void)
{
screen_stop_highlight();
reset_cterm_colors();
if (termcap_active)
{
#if defined(FEAT_TERMRESPONSE)
#if defined(FEAT_GUI)
if (!gui.in_use && !gui.starting)
#endif
{
if (termrequest_any_pending())
{
#if defined(UNIX)
mch_delay(100L, FALSE);
#endif
#if defined(TCIFLUSH)
if (exiting)
tcflush(fileno(stdin), TCIFLUSH);
#endif
}
check_for_codes_from_term();
}
#endif
out_str(T_BD); 
out_str(T_KE); 
out_flush();
termcap_active = FALSE;
cursor_on(); 
out_str(T_CTE); 
out_str(T_TE); 
screen_start(); 
out_flush();
}
}
#if defined(FEAT_TERMRESPONSE) || defined(PROTO)
void
may_req_termresponse(void)
{
if (crv_status.tr_progress == STATUS_GET
&& can_get_termresponse()
&& starting == 0
&& *T_CRV != NUL)
{
LOG_TR(("Sending CRV request"));
out_str(T_CRV);
termrequest_sent(&crv_status);
out_flush();
(void)vpeekc_nomap();
}
}
void
may_req_ambiguous_char_width(void)
{
if (u7_status.tr_progress == STATUS_GET
&& can_get_termresponse()
&& starting == 0
&& *T_U7 != NUL
&& !option_was_set((char_u *)"ambiwidth"))
{
char_u buf[16];
LOG_TR(("Sending U7 request"));
term_windgoto(1, 0);
buf[mb_char2bytes(0x25bd, buf)] = 0;
out_str(buf);
out_str(T_U7);
termrequest_sent(&u7_status);
out_flush();
screen_stop_highlight();
term_windgoto(1, 0);
out_str((char_u *)" ");
term_windgoto(0, 0);
screen_start();
out_flush();
(void)vpeekc_nomap();
}
}
void
may_req_bg_color(void)
{
if (can_get_termresponse() && starting == 0)
{
int didit = FALSE;
#if defined(FEAT_TERMINAL)
if (rfg_status.tr_progress == STATUS_GET && *T_RFG != NUL)
{
LOG_TR(("Sending FG request"));
out_str(T_RFG);
termrequest_sent(&rfg_status);
didit = TRUE;
}
#endif
if (rbg_status.tr_progress == STATUS_GET && *T_RBG != NUL)
{
LOG_TR(("Sending BG request"));
out_str(T_RBG);
termrequest_sent(&rbg_status);
didit = TRUE;
}
if (didit)
{
out_flush();
(void)vpeekc_nomap();
}
}
}
#if defined(DEBUG_TERMRESPONSE)
static void
log_tr(const char *fmt, ...)
{
static FILE *fd_tr = NULL;
static proftime_T start;
proftime_T now;
va_list ap;
if (fd_tr == NULL)
{
fd_tr = fopen("termresponse.log", "w");
profile_start(&start);
}
now = start;
profile_end(&now);
fprintf(fd_tr, "%s: %s ", profile_msg(&now),
must_redraw == NOT_VALID ? "NV"
: must_redraw == CLEAR ? "CL" : " ");
va_start(ap, fmt);
vfprintf(fd_tr, fmt, ap);
va_end(ap);
fputc('\n', fd_tr);
fflush(fd_tr);
}
#endif
#endif
int
swapping_screen(void)
{
return (full_screen && *T_TI != NUL);
}
void
scroll_start(void)
{
if (*T_VS != NUL && *T_CVS != NUL)
{
out_str(T_VS);
out_str(T_CVS);
screen_start(); 
}
}
static int cursor_is_off = FALSE;
void
cursor_on_force(void)
{
out_str(T_VE);
cursor_is_off = FALSE;
}
void
cursor_on(void)
{
if (cursor_is_off)
cursor_on_force();
}
void
cursor_off(void)
{
if (full_screen && !cursor_is_off)
{
out_str(T_VI); 
cursor_is_off = TRUE;
}
}
#if defined(CURSOR_SHAPE) || defined(PROTO)
void
term_cursor_mode(int forced)
{
static int showing_mode = -1;
char_u *p;
if (!full_screen || *T_CEI == NUL)
{
#if defined(FEAT_TERMRESPONSE)
if (forced && initial_cursor_shape > 0)
term_cursor_shape(initial_cursor_shape, initial_cursor_blink);
#endif
return;
}
if ((State & REPLACE) == REPLACE)
{
if (forced || showing_mode != REPLACE)
{
if (*T_CSR != NUL)
p = T_CSR; 
else
p = T_CSI; 
if (*p != NUL)
{
out_str(p);
showing_mode = REPLACE;
}
}
}
else if (State & INSERT)
{
if ((forced || showing_mode != INSERT) && *T_CSI != NUL)
{
out_str(T_CSI); 
showing_mode = INSERT;
}
}
else if (forced || showing_mode != NORMAL)
{
out_str(T_CEI); 
showing_mode = NORMAL;
}
}
#if defined(FEAT_TERMINAL) || defined(PROTO)
void
term_cursor_color(char_u *color)
{
if (*T_CSC != NUL)
{
out_str(T_CSC); 
out_str_nf(color);
out_str(T_CEC); 
out_flush();
}
}
#endif
int
blink_state_is_inverted()
{
#if defined(FEAT_TERMRESPONSE)
return rbm_status.tr_progress == STATUS_GOT
&& rcs_status.tr_progress == STATUS_GOT
&& initial_cursor_blink != initial_cursor_shape_blink;
#else
return FALSE;
#endif
}
void
term_cursor_shape(int shape, int blink)
{
if (*T_CSH != NUL)
{
OUT_STR(tgoto((char *)T_CSH, 0, shape * 2 - blink));
out_flush();
}
else
{
int do_blink = blink;
if (blink_state_is_inverted())
do_blink = !blink;
if (do_blink && *T_VS != NUL)
{
out_str(T_VS);
out_flush();
}
else if (!do_blink && *T_CVS != NUL)
{
out_str(T_CVS);
out_flush();
}
}
}
#endif
void
scroll_region_set(win_T *wp, int off)
{
OUT_STR(tgoto((char *)T_CS, W_WINROW(wp) + wp->w_height - 1,
W_WINROW(wp) + off));
if (*T_CSV != NUL && wp->w_width != Columns)
OUT_STR(tgoto((char *)T_CSV, wp->w_wincol + wp->w_width - 1,
wp->w_wincol));
screen_start(); 
}
void
scroll_region_reset(void)
{
OUT_STR(tgoto((char *)T_CS, (int)Rows - 1, 0));
if (*T_CSV != NUL)
OUT_STR(tgoto((char *)T_CSV, (int)Columns - 1, 0));
screen_start(); 
}
static struct termcode
{
char_u name[2]; 
char_u *code; 
int len; 
int modlen; 
} *termcodes = NULL;
static int tc_max_len = 0; 
static int tc_len = 0; 
static int termcode_star(char_u *code, int len);
void
clear_termcodes(void)
{
while (tc_len > 0)
vim_free(termcodes[--tc_len].code);
VIM_CLEAR(termcodes);
tc_max_len = 0;
#if defined(HAVE_TGETENT)
BC = (char *)empty_option;
UP = (char *)empty_option;
PC = NUL; 
ospeed = 0;
#endif
need_gather = TRUE; 
}
#define ATC_FROM_TERM 55
void
add_termcode(char_u *name, char_u *string, int flags)
{
struct termcode *new_tc;
int i, j;
char_u *s;
int len;
if (string == NULL || *string == NUL)
{
del_termcode(name);
return;
}
#if defined(MSWIN) && !defined(FEAT_GUI)
s = vim_strnsave(string, (int)STRLEN(string) + 1);
#else
#if defined(VIMDLL)
if (!gui.in_use)
s = vim_strnsave(string, (int)STRLEN(string) + 1);
else
#endif
s = vim_strsave(string);
#endif
if (s == NULL)
return;
if (flags != 0 && flags != ATC_FROM_TERM && term_7to8bit(string) != 0)
{
STRMOVE(s, s + 1);
s[0] = term_7to8bit(string);
}
#if defined(MSWIN) && (!defined(FEAT_GUI) || defined(VIMDLL))
#if defined(VIMDLL)
if (!gui.in_use)
#endif
{
if (s[0] == K_NUL)
{
STRMOVE(s + 1, s);
s[1] = 3;
}
}
#endif
len = (int)STRLEN(s);
need_gather = TRUE; 
if (tc_len == tc_max_len)
{
tc_max_len += 20;
new_tc = ALLOC_MULT(struct termcode, tc_max_len);
if (new_tc == NULL)
{
tc_max_len -= 20;
return;
}
for (i = 0; i < tc_len; ++i)
new_tc[i] = termcodes[i];
vim_free(termcodes);
termcodes = new_tc;
}
for (i = 0; i < tc_len; ++i)
{
if (termcodes[i].name[0] < name[0])
continue;
if (termcodes[i].name[0] == name[0])
{
if (termcodes[i].name[1] < name[1])
continue;
if (termcodes[i].name[1] == name[1])
{
if (flags == ATC_FROM_TERM && (j = termcode_star(
termcodes[i].code, termcodes[i].len)) > 0)
{
if (len == termcodes[i].len - j
&& STRNCMP(s, termcodes[i].code, len - 1) == 0
&& s[len - 1]
== termcodes[i].code[termcodes[i].len - 1])
{
vim_free(s);
return;
}
}
else
{
vim_free(termcodes[i].code);
--tc_len;
break;
}
}
}
for (j = tc_len; j > i; --j)
termcodes[j] = termcodes[j - 1];
break;
}
termcodes[i].name[0] = name[0];
termcodes[i].name[1] = name[1];
termcodes[i].code = s;
termcodes[i].len = len;
termcodes[i].modlen = 0;
j = termcode_star(s, len);
if (j > 0)
termcodes[i].modlen = len - 1 - j;
++tc_len;
}
static int
termcode_star(char_u *code, int len)
{
if (len >= 3 && code[len - 2] == '*')
{
if (len >= 5 && code[len - 3] == ';')
return 2;
else
return 1;
}
return 0;
}
char_u *
find_termcode(char_u *name)
{
int i;
for (i = 0; i < tc_len; ++i)
if (termcodes[i].name[0] == name[0] && termcodes[i].name[1] == name[1])
return termcodes[i].code;
return NULL;
}
char_u *
get_termcode(int i)
{
if (i >= tc_len)
return NULL;
return &termcodes[i].name[0];
}
int
get_termcode_len(int idx)
{
return termcodes[idx].len;
}
void
del_termcode(char_u *name)
{
int i;
if (termcodes == NULL) 
return;
need_gather = TRUE; 
for (i = 0; i < tc_len; ++i)
if (termcodes[i].name[0] == name[0] && termcodes[i].name[1] == name[1])
{
del_termcode_idx(i);
return;
}
}
static void
del_termcode_idx(int idx)
{
int i;
vim_free(termcodes[idx].code);
--tc_len;
for (i = idx; i < tc_len; ++i)
termcodes[i] = termcodes[i + 1];
}
#if defined(FEAT_TERMRESPONSE)
static void
switch_to_8bit(void)
{
int i;
int c;
if (!term_is_8bit(T_NAME))
{
for (i = 0; i < tc_len; ++i)
{
c = term_7to8bit(termcodes[i].code);
if (c != 0)
{
STRMOVE(termcodes[i].code + 1, termcodes[i].code + 2);
termcodes[i].code[0] = c;
}
}
need_gather = TRUE; 
}
detected_8bit = TRUE;
LOG_TR(("Switching to 8 bit"));
}
#endif
#if defined(CHECK_DOUBLE_CLICK)
static linenr_T orig_topline = 0;
#if defined(FEAT_DIFF)
static int orig_topfill = 0;
#endif
#endif
#if defined(CHECK_DOUBLE_CLICK) || defined(PROTO)
void
set_mouse_topline(win_T *wp)
{
orig_topline = wp->w_topline;
#if defined(FEAT_DIFF)
orig_topfill = wp->w_topfill;
#endif
}
int
is_mouse_topline(win_T *wp)
{
return orig_topline == wp->w_topline
#if defined(FEAT_DIFF)
&& orig_topfill == wp->w_topfill
#endif
;
}
#endif
static int
put_string_in_typebuf(
int offset,
int slen,
char_u *string,
int new_slen,
char_u *buf,
int bufsize,
int *buflen)
{
int extra = new_slen - slen;
string[new_slen] = NUL;
if (buf == NULL)
{
if (extra < 0)
del_typebuf(-extra, offset);
else if (extra > 0)
ins_typebuf(string + slen, REMAP_YES, offset, FALSE, FALSE);
mch_memmove(typebuf.tb_buf + typebuf.tb_off + offset, string,
(size_t)new_slen);
}
else
{
if (extra < 0)
mch_memmove(buf + offset, buf + offset - extra,
(size_t)(*buflen + offset + extra));
else if (extra > 0)
{
if (*buflen + extra + new_slen >= bufsize)
return FAIL;
mch_memmove(buf + offset + extra, buf + offset,
(size_t)(*buflen - offset));
}
mch_memmove(buf + offset, string, (size_t)new_slen);
*buflen = *buflen + extra + new_slen;
}
return OK;
}
int
decode_modifiers(int n)
{
int code = n - 1;
int modifiers = 0;
if (code & 1)
modifiers |= MOD_MASK_SHIFT;
if (code & 2)
modifiers |= MOD_MASK_ALT;
if (code & 4)
modifiers |= MOD_MASK_CTRL;
if (code & 8)
modifiers |= MOD_MASK_META;
return modifiers;
}
static int
modifiers2keycode(int modifiers, int *key, char_u *string)
{
int new_slen = 0;
if (modifiers != 0)
{
*key = simplify_key(*key, &modifiers);
if (modifiers != 0)
{
string[new_slen++] = K_SPECIAL;
string[new_slen++] = (int)KS_MODIFIER;
string[new_slen++] = modifiers;
}
}
return new_slen;
}
int
check_termcode(
int max_offset,
char_u *buf,
int bufsize,
int *buflen)
{
char_u *tp;
char_u *p;
int slen = 0; 
int modslen;
int len;
int retval = 0;
int offset;
char_u key_name[2];
int modifiers;
char_u *modifiers_start = NULL;
int key;
int new_slen; 
char_u string[MAX_KEY_CODE_LEN + 1];
int i, j;
int idx = 0;
int cpo_koffset;
cpo_koffset = (vim_strchr(p_cpo, CPO_KOFFSET) != NULL);
if (need_gather)
gather_termleader();
for (offset = 0; offset < max_offset; ++offset)
{
if (buf == NULL)
{
if (offset >= typebuf.tb_len)
break;
tp = typebuf.tb_buf + typebuf.tb_off + offset;
len = typebuf.tb_len - offset; 
}
else
{
if (offset >= *buflen)
break;
tp = buf + offset;
len = *buflen - offset;
}
if (*tp == K_SPECIAL)
{
offset += 2; 
continue;
}
i = *tp;
for (p = termleader; *p && *p != i; ++p)
;
if (*p == NUL)
continue;
if (*tp == ESC && !p_ek && (State & INSERT))
continue;
key_name[0] = NUL; 
key_name[1] = NUL; 
modifiers = 0; 
#if defined(FEAT_GUI)
if (gui.in_use)
{
if (*tp == CSI) 
{
if (len < 3)
return -1; 
slen = 3;
key_name[0] = tp[1];
key_name[1] = tp[2];
}
}
else
#endif 
{
for (idx = 0; idx < tc_len; ++idx)
{
slen = termcodes[idx].len;
modifiers_start = NULL;
if (cpo_koffset && offset && len < slen)
continue;
if (STRNCMP(termcodes[idx].code, tp,
(size_t)(slen > len ? len : slen)) == 0)
{
if (len < slen) 
return -1; 
if (termcodes[idx].name[0] == 'K'
&& VIM_ISDIGIT(termcodes[idx].name[1]))
{
for (j = idx + 1; j < tc_len; ++j)
if (termcodes[j].len == slen &&
STRNCMP(termcodes[idx].code,
termcodes[j].code, slen) == 0)
{
idx = j;
break;
}
}
key_name[0] = termcodes[idx].name[0];
key_name[1] = termcodes[idx].name[1];
break;
}
if (termcodes[idx].modlen > 0)
{
modslen = termcodes[idx].modlen;
if (cpo_koffset && offset && len < modslen)
continue;
if (STRNCMP(termcodes[idx].code, tp,
(size_t)(modslen > len ? len : modslen)) == 0)
{
int n;
if (len <= modslen) 
return -1; 
if (tp[modslen] == termcodes[idx].code[slen - 1])
slen = modslen + 1; 
else if (tp[modslen] != ';' && modslen == slen - 3)
continue; 
else
{
for (j = slen - 2; j < len && (isdigit(tp[j])
|| tp[j] == '-' || tp[j] == ';'); ++j)
;
++j;
if (len < j) 
return -1; 
if (tp[j - 1] != termcodes[idx].code[slen - 1])
continue; 
modifiers_start = tp + slen - 2;
n = atoi((char *)modifiers_start);
modifiers |= decode_modifiers(n);
slen = j;
}
key_name[0] = termcodes[idx].name[0];
key_name[1] = termcodes[idx].name[1];
break;
}
}
}
}
#if defined(FEAT_TERMRESPONSE)
if (key_name[0] == NUL
#if defined(FEAT_MOUSE_DEC)
|| key_name[0] == KS_DEC_MOUSE
#endif
#if defined(FEAT_MOUSE_PTERM)
|| key_name[0] == KS_PTERM_MOUSE
#endif
)
{
char_u *argp = tp[0] == ESC ? tp + 2 : tp + 1;
if (((tp[0] == ESC && len >= 3 && tp[1] == '[')
|| (tp[0] == CSI && len >= 2))
&& (VIM_ISDIGIT(*argp) || *argp == '>' || *argp == '?'))
{
int first = -1; 
int trail; 
int arg[3] = {-1, -1, -1}; 
int argc; 
char_u *ap = argp;
int csi_len;
if (!VIM_ISDIGIT(*ap))
first = *ap++;
for (argc = 0; argc < 3; )
{
if (ap >= tp + len)
{
not_enough:
LOG_TR(("Not enough characters for CSI sequence"));
return -1;
}
if (*ap == ';')
arg[argc++] = -1; 
else if (VIM_ISDIGIT(*ap))
{
arg[argc] = 0;
for (;;)
{
if (ap >= tp + len)
goto not_enough;
if (!VIM_ISDIGIT(*ap))
break;
arg[argc] = arg[argc] * 10 + (*ap - '0');
++ap;
}
++argc;
}
if (*ap == ';')
++ap;
else
break;
}
while (ap < tp + len
&& !(*ap >= '{' && *ap <= '~')
&& !ASCII_ISALPHA(*ap))
++ap;
if (ap >= tp + len)
goto not_enough;
trail = *ap;
csi_len = (int)(ap - tp) + 1;
if (first == -1 && argc == 2 && trail == 'R')
{
if (arg[0] == 2 && arg[1] >= 2)
{
char *aw = NULL;
LOG_TR(("Received U7 status: %s", tp));
u7_status.tr_progress = STATUS_GOT;
did_cursorhold = TRUE;
if (arg[1] == 2)
aw = "single";
else if (arg[1] == 3)
aw = "double";
if (aw != NULL && STRCMP(aw, p_ambw) != 0)
{
set_option_value((char_u *)"ambw", 0L,
(char_u *)aw, 0);
#if defined(DEBUG_TERMRESPONSE)
{
int r = redraw_asap(CLEAR);
log_tr("set 'ambiwidth', redraw_asap(): %d", r);
}
#else
redraw_asap(CLEAR);
#endif
}
}
key_name[0] = (int)KS_EXTRA;
key_name[1] = (int)KE_IGNORE;
slen = csi_len;
#if defined(FEAT_EVAL)
set_vim_var_string(VV_TERMU7RESP, tp, slen);
#endif
}
else if (*T_CRV != NUL && ap > argp + 1 && trail == 'c')
{
int version = arg[1];
LOG_TR(("Received CRV response: %s", tp));
crv_status.tr_progress = STATUS_GOT;
did_cursorhold = TRUE;
if (tp[0] == CSI)
switch_to_8bit();
if (version > 20000)
version = 0;
if (first == '>' && argc == 3)
{
int need_flush = FALSE;
int is_iterm2 = FALSE;
int is_mintty = FALSE;
int is_screen = FALSE;
if (arg[0] == 77)
is_mintty = TRUE;
if (version >= 141)
{
LOG_TR(("Enable checking for XT codes"));
check_for_codes = TRUE;
need_gather = TRUE;
req_codes_from_term();
}
if (version == 100 && arg[0] == 0 && arg[2] == 0)
{
if (mch_getenv((char_u *)"COLORS") == NULL)
may_adjust_color_count(256);
if (!option_was_set((char_u *)"ttym"))
set_option_value((char_u *)"ttym", 0L,
(char_u *)"sgr", 0);
}
if (version == 95)
{
if (arg[0] == 1 && arg[2] == 0)
{
is_not_xterm = TRUE;
is_mac_terminal = TRUE;
}
else if (arg[0] == 0 && arg[2] == 0)
is_iterm2 = TRUE;
else if (arg[0] == 0 && arg[2] == -1)
is_not_xterm = TRUE;
}
if (arg[0] == 83)
is_screen = TRUE;
if (!option_was_set((char_u *)"ttym"))
{
if ((!is_screen && version >= 277)
|| is_iterm2
|| is_mac_terminal
|| is_mintty
|| (is_screen && arg[1] >= 40700))
set_option_value((char_u *)"ttym", 0L,
(char_u *)"sgr", 0);
else if (version >= 95)
set_option_value((char_u *)"ttym", 0L,
(char_u *)"xterm2", 0);
}
if (arg[1] >= 2500)
is_not_xterm = TRUE;
else if (version == 136 && arg[2] == 0)
is_not_xterm = TRUE;
else if (version == 115 && arg[0] == 0 && arg[2] == 0)
is_not_xterm = TRUE;
if (version < 95)
is_not_xterm = TRUE;
if (rcs_status.tr_progress == STATUS_GET
&& version >= 279
&& !is_not_xterm
&& *T_CSH != NUL
&& *T_CRS != NUL)
{
LOG_TR(("Sending cursor style request"));
out_str(T_CRS);
termrequest_sent(&rcs_status);
need_flush = TRUE;
}
if (rbm_status.tr_progress == STATUS_GET
&& !is_not_xterm
&& *T_CRC != NUL)
{
LOG_TR(("Sending cursor blink mode request"));
out_str(T_CRC);
termrequest_sent(&rbm_status);
need_flush = TRUE;
}
if (need_flush)
out_flush();
}
slen = csi_len;
#if defined(FEAT_EVAL)
set_vim_var_string(VV_TERMRESPONSE, tp, slen);
#endif
apply_autocmds(EVENT_TERMRESPONSE,
NULL, NULL, FALSE, curbuf);
key_name[0] = (int)KS_EXTRA;
key_name[1] = (int)KE_IGNORE;
}
else if (rbm_status.tr_progress == STATUS_SENT
&& first == '?'
&& ap == argp + 6
&& arg[0] == 12
&& ap[-1] == '$'
&& trail == 'y')
{
initial_cursor_blink = (arg[1] == '1');
rbm_status.tr_progress = STATUS_GOT;
LOG_TR(("Received cursor blinking mode response: %s", tp));
key_name[0] = (int)KS_EXTRA;
key_name[1] = (int)KE_IGNORE;
slen = csi_len;
#if defined(FEAT_EVAL)
set_vim_var_string(VV_TERMBLINKRESP, tp, slen);
#endif
}
else if (did_request_winpos && argc == 3 && arg[0] == 3
&& trail == 't')
{
winpos_x = arg[1];
winpos_y = arg[2];
key_name[0] = (int)KS_EXTRA;
key_name[1] = (int)KE_IGNORE;
slen = csi_len;
if (--did_request_winpos <= 0)
winpos_status.tr_progress = STATUS_GOT;
}
else if ((arg[0] == 27 && argc == 3 && trail == '~')
|| (argc == 2 && trail == 'u'))
{
seenModifyOtherKeys = TRUE;
if (trail == 'u')
key = arg[0];
else
key = arg[2];
modifiers = decode_modifiers(arg[1]);
if (modifiers == MOD_MASK_SHIFT
&& ((key >= '@' && key <= 'Z')
|| key == '^' || key == '_'
|| (key >= '{' && key <= '~')))
modifiers = 0;
if ((modifiers & MOD_MASK_CTRL) && ASCII_ISALPHA(key))
key = TOUPPER_ASC(key);
new_slen = modifiers2keycode(modifiers, &key, string);
slen = csi_len;
if (has_mbyte)
new_slen += (*mb_char2bytes)(key, string + new_slen);
else
string[new_slen++] = key;
if (put_string_in_typebuf(offset, slen, string, new_slen,
buf, bufsize, buflen) == FAIL)
return -1;
return len + new_slen - slen + offset;
}
}
else if ((*T_RBG != NUL || *T_RFG != NUL)
&& ((tp[0] == ESC && len >= 2 && tp[1] == ']')
|| tp[0] == OSC))
{
j = 1 + (tp[0] == ESC);
if (len >= j + 3 && (argp[0] != '1'
|| (argp[1] != '1' && argp[1] != '0')
|| argp[2] != ';'))
i = 0; 
else
for (i = j; i < len; ++i)
if (tp[i] == '\007' || (tp[0] == OSC ? tp[i] == STERM
: (tp[i] == ESC && i + 1 < len && tp[i + 1] == '\\')))
{
int is_bg = argp[1] == '1';
int is_4digit = i - j >= 21 && tp[j + 11] == '/'
&& tp[j + 16] == '/';
if (i - j >= 15 && STRNCMP(tp + j + 3, "rgb:", 4) == 0
&& (is_4digit
|| (tp[j + 9] == '/' && tp[i + 12 == '/'])))
{
char_u *tp_r = tp + j + 7;
char_u *tp_g = tp + j + (is_4digit ? 12 : 10);
char_u *tp_b = tp + j + (is_4digit ? 17 : 13);
#if defined(FEAT_TERMINAL)
int rval, gval, bval;
rval = hexhex2nr(tp_r);
gval = hexhex2nr(tp_b);
bval = hexhex2nr(tp_g);
#endif
if (is_bg)
{
char *new_bg_val = (3 * '6' < *tp_r + *tp_g +
*tp_b) ? "light" : "dark";
LOG_TR(("Received RBG response: %s", tp));
rbg_status.tr_progress = STATUS_GOT;
#if defined(FEAT_TERMINAL)
bg_r = rval;
bg_g = gval;
bg_b = bval;
#endif
if (!option_was_set((char_u *)"bg")
&& STRCMP(p_bg, new_bg_val) != 0)
{
set_option_value((char_u *)"bg", 0L,
(char_u *)new_bg_val, 0);
reset_option_was_set((char_u *)"bg");
redraw_asap(CLEAR);
}
}
#if defined(FEAT_TERMINAL)
else
{
LOG_TR(("Received RFG response: %s", tp));
rfg_status.tr_progress = STATUS_GOT;
fg_r = rval;
fg_g = gval;
fg_b = bval;
}
#endif
}
key_name[0] = (int)KS_EXTRA;
key_name[1] = (int)KE_IGNORE;
slen = i + 1 + (tp[i] == ESC);
#if defined(FEAT_EVAL)
set_vim_var_string(is_bg ? VV_TERMRBGRESP
: VV_TERMRFGRESP, tp, slen);
#endif
break;
}
if (i == len)
{
LOG_TR(("not enough characters for RB"));
return -1;
}
}
else if ((check_for_codes || rcs_status.tr_progress == STATUS_SENT)
&& ((tp[0] == ESC && len >= 2 && tp[1] == 'P')
|| tp[0] == DCS))
{
j = 1 + (tp[0] == ESC);
if (len < j + 3)
i = len; 
else if ((argp[1] != '+' && argp[1] != '$') || argp[2] != 'r')
i = 0; 
else if (argp[1] == '+')
for (i = j; i < len; ++i)
{
if ((tp[i] == ESC && i + 1 < len && tp[i + 1] == '\\')
|| tp[i] == STERM)
{
if (i - j >= 3)
got_code_from_term(tp + j, i);
key_name[0] = (int)KS_EXTRA;
key_name[1] = (int)KE_IGNORE;
slen = i + 1 + (tp[i] == ESC);
break;
}
}
else
{
for (i = j + 3; i < len; ++i)
{
if (i - j == 3 && !isdigit(tp[i]))
break;
if (i - j == 4 && tp[i] != ' ')
break;
if (i - j == 5 && tp[i] != 'q')
break;
if (i - j == 6 && tp[i] != ESC && tp[i] != STERM)
break;
if ((i - j == 6 && tp[i] == STERM)
|| (i - j == 7 && tp[i] == '\\'))
{
int number = argp[3] - '0';
number = number == 0 ? 1 : number;
initial_cursor_shape = (number + 1) / 2;
initial_cursor_shape_blink =
(number & 1) ? FALSE : TRUE;
rcs_status.tr_progress = STATUS_GOT;
LOG_TR(("Received cursor shape response: %s", tp));
key_name[0] = (int)KS_EXTRA;
key_name[1] = (int)KE_IGNORE;
slen = i + 1;
#if defined(FEAT_EVAL)
set_vim_var_string(VV_TERMSTYLERESP, tp, slen);
#endif
break;
}
}
}
if (i == len)
{
LOG_TR(("not enough characters for XT"));
return -1;
}
}
}
#endif
if (key_name[0] == NUL)
continue; 
#if defined(FEAT_GUI)
if (gui.in_use
&& key_name[0] == (int)KS_EXTRA
&& (key_name[1] == (int)KE_X1MOUSE
|| key_name[1] == (int)KE_X2MOUSE
|| key_name[1] == (int)KE_MOUSELEFT
|| key_name[1] == (int)KE_MOUSERIGHT
|| key_name[1] == (int)KE_MOUSEDOWN
|| key_name[1] == (int)KE_MOUSEUP))
{
char_u bytes[6];
int num_bytes = get_bytes_from_buf(tp + slen, bytes, 4);
if (num_bytes == -1) 
return -1;
mouse_col = 128 * (bytes[0] - ' ' - 1) + bytes[1] - ' ' - 1;
mouse_row = 128 * (bytes[2] - ' ' - 1) + bytes[3] - ' ' - 1;
slen += num_bytes;
}
else
#endif
if (key_name[0] == KS_MOUSE
#if defined(FEAT_MOUSE_GPM)
|| key_name[0] == KS_GPM_MOUSE
#endif
#if defined(FEAT_MOUSE_JSB)
|| key_name[0] == KS_JSBTERM_MOUSE
#endif
#if defined(FEAT_MOUSE_NET)
|| key_name[0] == KS_NETTERM_MOUSE
#endif
#if defined(FEAT_MOUSE_DEC)
|| key_name[0] == KS_DEC_MOUSE
#endif
#if defined(FEAT_MOUSE_PTERM)
|| key_name[0] == KS_PTERM_MOUSE
#endif
#if defined(FEAT_MOUSE_URXVT)
|| key_name[0] == KS_URXVT_MOUSE
#endif
|| key_name[0] == KS_SGR_MOUSE
|| key_name[0] == KS_SGR_MOUSE_RELEASE)
{
if (check_termcode_mouse(tp, &slen, key_name, modifiers_start, idx,
&modifiers) == -1)
return -1;
}
#if defined(FEAT_GUI)
#if defined(FEAT_MENU)
else if (key_name[0] == (int)KS_MENU)
{
long_u val;
int num_bytes = get_long_from_buf(tp + slen, &val);
if (num_bytes == -1)
return -1;
current_menu = (vimmenu_T *)val;
slen += num_bytes;
if (check_menu_pointer(root_menu, current_menu) == FAIL)
{
key_name[0] = KS_EXTRA;
key_name[1] = (int)KE_IGNORE;
}
}
#endif
#if defined(FEAT_GUI_TABLINE)
else if (key_name[0] == (int)KS_TABLINE)
{
char_u bytes[6];
int num_bytes = get_bytes_from_buf(tp + slen, bytes, 1);
if (num_bytes == -1)
return -1;
current_tab = (int)bytes[0];
if (current_tab == 255) 
current_tab = -1;
slen += num_bytes;
}
else if (key_name[0] == (int)KS_TABMENU)
{
char_u bytes[6];
int num_bytes = get_bytes_from_buf(tp + slen, bytes, 2);
if (num_bytes == -1)
return -1;
current_tab = (int)bytes[0];
current_tabmenu = (int)bytes[1];
slen += num_bytes;
}
#endif
#if !defined(USE_ON_FLY_SCROLL)
else if (key_name[0] == (int)KS_VER_SCROLLBAR)
{
long_u val;
char_u bytes[6];
int num_bytes;
j = 0;
for (i = 0; tp[j] == CSI && tp[j + 1] == KS_VER_SCROLLBAR
&& tp[j + 2] != NUL; ++i)
{
j += 3;
num_bytes = get_bytes_from_buf(tp + j, bytes, 1);
if (num_bytes == -1)
break;
if (i == 0)
current_scrollbar = (int)bytes[0];
else if (current_scrollbar != (int)bytes[0])
break;
j += num_bytes;
num_bytes = get_long_from_buf(tp + j, &val);
if (num_bytes == -1)
break;
scrollbar_value = val;
j += num_bytes;
slen = j;
}
if (i == 0) 
return -1;
}
else if (key_name[0] == (int)KS_HOR_SCROLLBAR)
{
long_u val;
int num_bytes;
j = 0;
for (i = 0; tp[j] == CSI && tp[j + 1] == KS_HOR_SCROLLBAR
&& tp[j + 2] != NUL; ++i)
{
j += 3;
num_bytes = get_long_from_buf(tp + j, &val);
if (num_bytes == -1)
break;
scrollbar_value = val;
j += num_bytes;
slen = j;
}
if (i == 0) 
return -1;
}
#endif 
#endif 
key = handle_x_keys(TERMCAP2KEY(key_name[0], key_name[1]));
new_slen = modifiers2keycode(modifiers, &key, string);
key_name[0] = KEY2TERMCAP0(key);
key_name[1] = KEY2TERMCAP1(key);
if (key_name[0] == KS_KEY)
{
if (has_mbyte)
new_slen += (*mb_char2bytes)(key_name[1], string + new_slen);
else
string[new_slen++] = key_name[1];
}
else if (new_slen == 0 && key_name[0] == KS_EXTRA
&& key_name[1] == KE_IGNORE)
{
retval = KEYLEN_REMOVED;
}
else
{
string[new_slen++] = K_SPECIAL;
string[new_slen++] = key_name[0];
string[new_slen++] = key_name[1];
}
if (put_string_in_typebuf(offset, slen, string, new_slen,
buf, bufsize, buflen) == FAIL)
return -1;
return retval == 0 ? (len + new_slen - slen + offset) : retval;
}
#if defined(FEAT_TERMRESPONSE)
LOG_TR(("normal character"));
#endif
return 0; 
}
#if (defined(FEAT_TERMINAL) && defined(FEAT_TERMRESPONSE)) || defined(PROTO)
void
term_get_fg_color(char_u *r, char_u *g, char_u *b)
{
if (rfg_status.tr_progress == STATUS_GOT)
{
*r = fg_r;
*g = fg_g;
*b = fg_b;
}
}
void
term_get_bg_color(char_u *r, char_u *g, char_u *b)
{
if (rbg_status.tr_progress == STATUS_GOT)
{
*r = bg_r;
*g = bg_g;
*b = bg_b;
}
}
#endif
char_u *
replace_termcodes(
char_u *from,
char_u **bufp,
int flags,
int *did_simplify)
{
int i;
int slen;
int key;
int dlen = 0;
char_u *src;
int do_backslash; 
int do_special; 
int do_key_code; 
char_u *result; 
do_backslash = (vim_strchr(p_cpo, CPO_BSLASH) == NULL);
do_special = (vim_strchr(p_cpo, CPO_SPECI) == NULL)
|| (flags & REPTERM_SPECIAL);
do_key_code = (vim_strchr(p_cpo, CPO_KEYCODE) == NULL);
result = alloc(STRLEN(from) * 6 + 1);
if (result == NULL) 
{
*bufp = NULL;
return from;
}
src = from;
if ((flags & REPTERM_FROM_PART) && src[0] == '#' && VIM_ISDIGIT(src[1]))
{
result[dlen++] = K_SPECIAL;
result[dlen++] = 'k';
if (src[1] == '0')
result[dlen++] = ';'; 
else
result[dlen++] = src[1]; 
src += 2;
}
while (*src != NUL)
{
if (do_special && ((flags & REPTERM_DO_LT)
|| STRNCMP(src, "<lt>", 4) != 0))
{
#if defined(FEAT_EVAL)
if (STRNICMP(src, "<SID>", 5) == 0)
{
if (current_sctx.sc_sid <= 0)
emsg(_(e_usingsid));
else
{
src += 5;
result[dlen++] = K_SPECIAL;
result[dlen++] = (int)KS_EXTRA;
result[dlen++] = (int)KE_SNR;
sprintf((char *)result + dlen, "%ld",
(long)current_sctx.sc_sid);
dlen += (int)STRLEN(result + dlen);
result[dlen++] = '_';
continue;
}
}
#endif
slen = trans_special(&src, result + dlen, TRUE, FALSE,
(flags & REPTERM_NO_SIMPLIFY) == 0, did_simplify);
if (slen)
{
dlen += slen;
continue;
}
}
if (do_key_code)
{
i = find_term_bykeys(src);
if (i >= 0)
{
result[dlen++] = K_SPECIAL;
result[dlen++] = termcodes[i].name[0];
result[dlen++] = termcodes[i].name[1];
src += termcodes[i].len;
continue;
}
}
#if defined(FEAT_EVAL)
if (do_special)
{
char_u *p, *s, len;
if (STRNICMP(src, "<Leader>", 8) == 0)
{
len = 8;
p = get_var_value((char_u *)"g:mapleader");
}
else if (STRNICMP(src, "<LocalLeader>", 13) == 0)
{
len = 13;
p = get_var_value((char_u *)"g:maplocalleader");
}
else
{
len = 0;
p = NULL;
}
if (len != 0)
{
if (p == NULL || *p == NUL || STRLEN(p) > 8 * 6)
s = (char_u *)"\\";
else
s = p;
while (*s != NUL)
result[dlen++] = *s++;
src += len;
continue;
}
}
#endif
key = *src;
if (key == Ctrl_V || (do_backslash && key == '\\'))
{
++src; 
if (*src == NUL)
{
if (flags & REPTERM_FROM_PART)
result[dlen++] = key;
break;
}
}
for (i = (*mb_ptr2len)(src); i > 0; --i)
{
if (*src == K_SPECIAL)
{
result[dlen++] = K_SPECIAL;
result[dlen++] = KS_SPECIAL;
result[dlen++] = KE_FILLER;
}
#if defined(FEAT_GUI)
else if (*src == CSI)
{
result[dlen++] = K_SPECIAL;
result[dlen++] = KS_EXTRA;
result[dlen++] = (int)KE_CSI;
}
#endif
else
result[dlen++] = *src;
++src;
}
}
result[dlen] = NUL;
if ((*bufp = vim_strsave(result)) != NULL)
from = *bufp;
vim_free(result);
return from;
}
static int
find_term_bykeys(char_u *src)
{
int i;
int slen = (int)STRLEN(src);
for (i = 0; i < tc_len; ++i)
{
if (slen == termcodes[i].len
&& STRNCMP(termcodes[i].code, src, (size_t)slen) == 0)
return i;
}
return -1;
}
static void
gather_termleader(void)
{
int i;
int len = 0;
#if defined(FEAT_GUI)
if (gui.in_use)
termleader[len++] = CSI; 
#endif
#if defined(FEAT_TERMRESPONSE)
if (check_for_codes || *T_CRS != NUL)
termleader[len++] = DCS; 
#endif
termleader[len] = NUL;
for (i = 0; i < tc_len; ++i)
if (vim_strchr(termleader, termcodes[i].code[0]) == NULL)
{
termleader[len++] = termcodes[i].code[0];
termleader[len] = NUL;
}
need_gather = FALSE;
}
void
show_termcodes(void)
{
int col;
int *items;
int item_count;
int run;
int row, rows;
int cols;
int i;
int len;
#define INC3 27 
#define INC2 40 
#define GAP 2 
if (tc_len == 0) 
return;
items = ALLOC_MULT(int, tc_len);
if (items == NULL)
return;
msg_puts_title(_("\n--- Terminal keys ---"));
for (run = 1; run <= 3 && !got_int; ++run)
{
item_count = 0;
for (i = 0; i < tc_len; i++)
{
len = show_one_termcode(termcodes[i].name,
termcodes[i].code, FALSE);
if (len <= INC3 - GAP ? run == 1
: len <= INC2 - GAP ? run == 2
: run == 3)
items[item_count++] = i;
}
if (run <= 2)
{
cols = (Columns + GAP) / (run == 1 ? INC3 : INC2);
if (cols == 0)
cols = 1;
rows = (item_count + cols - 1) / cols;
}
else 
rows = item_count;
for (row = 0; row < rows && !got_int; ++row)
{
msg_putchar('\n'); 
if (got_int) 
break;
col = 0;
for (i = row; i < item_count; i += rows)
{
msg_col = col; 
show_one_termcode(termcodes[items[i]].name,
termcodes[items[i]].code, TRUE);
if (run == 2)
col += INC2;
else
col += INC3;
}
out_flush();
ui_breakcheck();
}
}
vim_free(items);
}
int
show_one_termcode(char_u *name, char_u *code, int printit)
{
char_u *p;
int len;
if (name[0] > '~')
{
IObuff[0] = ' ';
IObuff[1] = ' ';
IObuff[2] = ' ';
IObuff[3] = ' ';
}
else
{
IObuff[0] = 't';
IObuff[1] = '_';
IObuff[2] = name[0];
IObuff[3] = name[1];
}
IObuff[4] = ' ';
p = get_special_key_name(TERMCAP2KEY(name[0], name[1]), 0);
if (p[1] != 't')
STRCPY(IObuff + 5, p);
else
IObuff[5] = NUL;
len = (int)STRLEN(IObuff);
do
IObuff[len++] = ' ';
while (len < 17);
IObuff[len] = NUL;
if (code == NULL)
len += 4;
else
len += vim_strsize(code);
if (printit)
{
msg_puts((char *)IObuff);
if (code == NULL)
msg_puts("NULL");
else
msg_outtrans(code);
}
return len;
}
#if defined(FEAT_TERMRESPONSE) || defined(PROTO)
static int xt_index_in = 0;
static int xt_index_out = 0;
static void
req_codes_from_term(void)
{
xt_index_out = 0;
xt_index_in = 0;
req_more_codes_from_term();
}
static void
req_more_codes_from_term(void)
{
char buf[11];
int old_idx = xt_index_out;
if (exiting)
return;
while (xt_index_out < xt_index_in + 10 && key_names[xt_index_out] != NULL)
{
char *key_name = key_names[xt_index_out];
LOG_TR(("Requesting XT %d: %s", xt_index_out, key_name));
sprintf(buf, "\033P+q%02x%02x\033\\", key_name[0], key_name[1]);
out_str_nf((char_u *)buf);
++xt_index_out;
}
if (xt_index_out != old_idx)
out_flush();
}
static void
got_code_from_term(char_u *code, int len)
{
#define XT_LEN 100
char_u name[3];
char_u str[XT_LEN];
int i;
int j = 0;
int c;
if (code[0] == '1' && code[7] == '=' && len / 2 < XT_LEN)
{
name[0] = hexhex2nr(code + 3);
name[1] = hexhex2nr(code + 5);
name[2] = NUL;
for (i = 0; key_names[i] != NULL; ++i)
{
if (STRCMP(key_names[i], name) == 0)
{
xt_index_in = i;
break;
}
}
LOG_TR(("Received XT %d: %s", xt_index_in, (char *)name));
if (key_names[i] != NULL)
{
for (i = 8; (c = hexhex2nr(code + i)) >= 0; i += 2)
str[j++] = c;
str[j] = NUL;
if (name[0] == 'C' && name[1] == 'o')
{
i = atoi((char *)str);
may_adjust_color_count(i);
}
else
{
i = find_term_bykeys(str);
if (i >= 0)
del_termcode_idx(i);
add_termcode(name, str, ATC_FROM_TERM);
}
}
}
++xt_index_in;
req_more_codes_from_term();
}
static void
check_for_codes_from_term(void)
{
int c;
if (xt_index_out == 0 || xt_index_out == xt_index_in)
return;
++no_mapping;
++allow_keys;
for (;;)
{
c = vpeekc();
if (c == NUL) 
break;
if (c != K_SPECIAL && c != K_IGNORE)
break;
c = vgetc();
if (c != K_IGNORE)
{
vungetc(c);
break;
}
}
--no_mapping;
--allow_keys;
}
#endif
#if (defined(MSWIN) && (!defined(FEAT_GUI) || defined(VIMDLL))) || defined(PROTO)
static char ksme_str[20];
static char ksmr_str[20];
static char ksmd_str[20];
void
update_tcap(int attr)
{
struct builtin_term *p;
p = find_builtin_term(DEFAULT_TERM);
sprintf(ksme_str, IF_EB("\033|%dm", ESC_STR "|%dm"), attr);
sprintf(ksmd_str, IF_EB("\033|%dm", ESC_STR "|%dm"),
attr | 0x08); 
sprintf(ksmr_str, IF_EB("\033|%dm", ESC_STR "|%dm"),
((attr & 0x0F) << 4) | ((attr & 0xF0) >> 4));
while (p->bt_string != NULL)
{
if (p->bt_entry == (int)KS_ME)
p->bt_string = &ksme_str[0];
else if (p->bt_entry == (int)KS_MR)
p->bt_string = &ksmr_str[0];
else if (p->bt_entry == (int)KS_MD)
p->bt_string = &ksmd_str[0];
++p;
}
}
#if defined(FEAT_TERMGUICOLORS)
#define KSSIZE 20
struct ks_tbl_s
{
int code; 
char *vtp; 
char *vtp2; 
char buf[KSSIZE]; 
char vbuf[KSSIZE]; 
char v2buf[KSSIZE]; 
char arr[KSSIZE]; 
};
static struct ks_tbl_s ks_tbl[] =
{
{(int)KS_ME, "\033|0m", "\033|0m"}, 
{(int)KS_MR, "\033|7m", "\033|7m"}, 
{(int)KS_MD, "\033|1m", "\033|1m"}, 
{(int)KS_SO, "\033|91m", "\033|91m"}, 
{(int)KS_SE, "\033|39m", "\033|39m"}, 
{(int)KS_CZH, "\033|95m", "\033|95m"}, 
{(int)KS_CZR, "\033|0m", "\033|0m"}, 
{(int)KS_US, "\033|4m", "\033|4m"}, 
{(int)KS_UE, "\033|24m", "\033|24m"}, 
#if defined(TERMINFO)
{(int)KS_CAB, "\033|%p1%db", "\033|%p14%dm"}, 
{(int)KS_CAF, "\033|%p1%df", "\033|%p13%dm"}, 
{(int)KS_CS, "\033|%p1%d;%p2%dR", "\033|%p1%d;%p2%dR"},
{(int)KS_CSV, "\033|%p1%d;%p2%dV", "\033|%p1%d;%p2%dV"},
#else
{(int)KS_CAB, "\033|%db", "\033|4%dm"}, 
{(int)KS_CAF, "\033|%df", "\033|3%dm"}, 
{(int)KS_CS, "\033|%d;%dR", "\033|%d;%dR"},
{(int)KS_CSV, "\033|%d;%dV", "\033|%d;%dV"},
#endif
{(int)KS_CCO, "256", "256"}, 
{(int)KS_NAME} 
};
static struct builtin_term *
find_first_tcap(
char_u *name,
int code)
{
struct builtin_term *p;
for (p = find_builtin_term(name); p->bt_string != NULL; ++p)
if (p->bt_entry == code)
return p;
return NULL;
}
#endif
void
swap_tcap(void)
{
#if defined(FEAT_TERMGUICOLORS)
static int init_done = FALSE;
static int curr_mode;
struct ks_tbl_s *ks;
struct builtin_term *bt;
int mode;
enum
{
CMODEINDEX,
CMODE24,
CMODE256
};
if (!init_done)
{
for (ks = ks_tbl; ks->code != (int)KS_NAME; ks++)
{
bt = find_first_tcap(DEFAULT_TERM, ks->code);
if (bt != NULL)
{
STRNCPY(ks->buf, bt->bt_string, KSSIZE);
STRNCPY(ks->vbuf, ks->vtp, KSSIZE);
STRNCPY(ks->v2buf, ks->vtp2, KSSIZE);
STRNCPY(ks->arr, bt->bt_string, KSSIZE);
bt->bt_string = &ks->arr[0];
}
}
init_done = TRUE;
curr_mode = CMODEINDEX;
}
if (p_tgc)
mode = CMODE24;
else if (t_colors >= 256)
mode = CMODE256;
else
mode = CMODEINDEX;
for (ks = ks_tbl; ks->code != (int)KS_NAME; ks++)
{
bt = find_first_tcap(DEFAULT_TERM, ks->code);
if (bt != NULL)
{
switch (curr_mode)
{
case CMODEINDEX:
STRNCPY(&ks->buf[0], bt->bt_string, KSSIZE);
break;
case CMODE24:
STRNCPY(&ks->vbuf[0], bt->bt_string, KSSIZE);
break;
default:
STRNCPY(&ks->v2buf[0], bt->bt_string, KSSIZE);
}
}
}
if (mode != curr_mode)
{
for (ks = ks_tbl; ks->code != (int)KS_NAME; ks++)
{
bt = find_first_tcap(DEFAULT_TERM, ks->code);
if (bt != NULL)
{
switch (mode)
{
case CMODEINDEX:
STRNCPY(bt->bt_string, &ks->buf[0], KSSIZE);
break;
case CMODE24:
STRNCPY(bt->bt_string, &ks->vbuf[0], KSSIZE);
break;
default:
STRNCPY(bt->bt_string, &ks->v2buf[0], KSSIZE);
}
}
}
curr_mode = mode;
}
#endif
}
#endif
#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS) || defined(PROTO)
static int
hex_digit(int c)
{
if (isdigit(c))
return c - '0';
c = TOLOWER_ASC(c);
if (c >= 'a' && c <= 'f')
return c - 'a' + 10;
return 0x1ffffff;
}
#if defined(VIMDLL)
static guicolor_T
gui_adjust_rgb(guicolor_T c)
{
if (gui.in_use)
return c;
else
return ((c & 0xff) << 16) | (c & 0x00ff00) | ((c >> 16) & 0xff);
}
#else
#define gui_adjust_rgb(c) (c)
#endif
guicolor_T
gui_get_color_cmn(char_u *name)
{
#if defined(RGB) && defined(MSWIN) && !defined(FEAT_GUI)
#undef RGB
#endif
#if !defined(RGB)
#define RGB(r, g, b) ((r<<16) | (g<<8) | (b))
#endif
#define LINE_LEN 100
FILE *fd;
char line[LINE_LEN];
char_u *fname;
int r, g, b, i;
guicolor_T color;
struct rgbcolor_table_S {
char_u *color_name;
guicolor_T color;
};
static struct rgbcolor_table_S rgb_table[] = {
{(char_u *)"black", RGB(0x00, 0x00, 0x00)},
{(char_u *)"blue", RGB(0x00, 0x00, 0xFF)},
{(char_u *)"brown", RGB(0xA5, 0x2A, 0x2A)},
{(char_u *)"cyan", RGB(0x00, 0xFF, 0xFF)},
{(char_u *)"darkblue", RGB(0x00, 0x00, 0x8B)},
{(char_u *)"darkcyan", RGB(0x00, 0x8B, 0x8B)},
{(char_u *)"darkgray", RGB(0xA9, 0xA9, 0xA9)},
{(char_u *)"darkgreen", RGB(0x00, 0x64, 0x00)},
{(char_u *)"darkgrey", RGB(0xA9, 0xA9, 0xA9)},
{(char_u *)"darkmagenta", RGB(0x8B, 0x00, 0x8B)},
{(char_u *)"darkred", RGB(0x8B, 0x00, 0x00)},
{(char_u *)"darkyellow", RGB(0x8B, 0x8B, 0x00)}, 
{(char_u *)"gray", RGB(0xBE, 0xBE, 0xBE)},
{(char_u *)"green", RGB(0x00, 0xFF, 0x00)},
{(char_u *)"grey", RGB(0xBE, 0xBE, 0xBE)},
{(char_u *)"grey40", RGB(0x66, 0x66, 0x66)},
{(char_u *)"grey50", RGB(0x7F, 0x7F, 0x7F)},
{(char_u *)"grey90", RGB(0xE5, 0xE5, 0xE5)},
{(char_u *)"lightblue", RGB(0xAD, 0xD8, 0xE6)},
{(char_u *)"lightcyan", RGB(0xE0, 0xFF, 0xFF)},
{(char_u *)"lightgray", RGB(0xD3, 0xD3, 0xD3)},
{(char_u *)"lightgreen", RGB(0x90, 0xEE, 0x90)},
{(char_u *)"lightgrey", RGB(0xD3, 0xD3, 0xD3)},
{(char_u *)"lightmagenta", RGB(0xFF, 0x8B, 0xFF)}, 
{(char_u *)"lightred", RGB(0xFF, 0x8B, 0x8B)}, 
{(char_u *)"lightyellow", RGB(0xFF, 0xFF, 0xE0)},
{(char_u *)"magenta", RGB(0xFF, 0x00, 0xFF)},
{(char_u *)"red", RGB(0xFF, 0x00, 0x00)},
{(char_u *)"seagreen", RGB(0x2E, 0x8B, 0x57)},
{(char_u *)"white", RGB(0xFF, 0xFF, 0xFF)},
{(char_u *)"yellow", RGB(0xFF, 0xFF, 0x00)},
};
static struct rgbcolor_table_S *colornames_table;
static int size = 0;
if (name[0] == '#' && STRLEN(name) == 7)
{
color = RGB(((hex_digit(name[1]) << 4) + hex_digit(name[2])),
((hex_digit(name[3]) << 4) + hex_digit(name[4])),
((hex_digit(name[5]) << 4) + hex_digit(name[6])));
if (color > 0xffffff)
return INVALCOLOR;
return gui_adjust_rgb(color);
}
for (i = 0; i < (int)(sizeof(rgb_table) / sizeof(rgb_table[0])); i++)
if (STRICMP(name, rgb_table[i].color_name) == 0)
return gui_adjust_rgb(rgb_table[i].color);
if (size == 0)
{
int counting;
fname = expand_env_save((char_u *)"$VIMRUNTIME/rgb.txt");
if (fname == NULL)
return INVALCOLOR;
fd = fopen((char *)fname, "rt");
vim_free(fname);
if (fd == NULL)
{
if (p_verbose > 1)
verb_msg(_("Cannot open $VIMRUNTIME/rgb.txt"));
size = -1; 
return INVALCOLOR;
}
for (counting = 1; counting >= 0; --counting)
{
if (!counting)
{
colornames_table = ALLOC_MULT(struct rgbcolor_table_S, size);
if (colornames_table == NULL)
{
fclose(fd);
return INVALCOLOR;
}
rewind(fd);
}
size = 0;
while (!feof(fd))
{
size_t len;
int pos;
vim_ignoredp = fgets(line, LINE_LEN, fd);
len = strlen(line);
if (len <= 1 || line[len - 1] != '\n')
continue;
line[len - 1] = '\0';
i = sscanf(line, "%d %d %d %n", &r, &g, &b, &pos);
if (i != 3)
continue;
if (!counting)
{
char_u *s = vim_strsave((char_u *)line + pos);
if (s == NULL)
{
fclose(fd);
return INVALCOLOR;
}
colornames_table[size].color_name = s;
colornames_table[size].color = (guicolor_T)RGB(r, g, b);
}
size++;
if (size == 10000)
break;
}
}
fclose(fd);
}
for (i = 0; i < size; i++)
if (STRICMP(name, colornames_table[i].color_name) == 0)
return gui_adjust_rgb(colornames_table[i].color);
return INVALCOLOR;
}
guicolor_T
gui_get_rgb_color_cmn(int r, int g, int b)
{
guicolor_T color = RGB(r, g, b);
if (color > 0xffffff)
return INVALCOLOR;
return gui_adjust_rgb(color);
}
#endif
#if (defined(MSWIN) && (!defined(FEAT_GUI_MSWIN) || defined(VIMDLL))) || defined(FEAT_TERMINAL) || defined(PROTO)
static int cube_value[] = {
0x00, 0x5F, 0x87, 0xAF, 0xD7, 0xFF
};
static int grey_ramp[] = {
0x08, 0x12, 0x1C, 0x26, 0x30, 0x3A, 0x44, 0x4E, 0x58, 0x62, 0x6C, 0x76,
0x80, 0x8A, 0x94, 0x9E, 0xA8, 0xB2, 0xBC, 0xC6, 0xD0, 0xDA, 0xE4, 0xEE
};
#if defined(FEAT_TERMINAL)
#include "libvterm/include/vterm.h" 
#else
#define VTERM_ANSI_INDEX_NONE 0
#endif
static char_u ansi_table[16][4] = {
{ 0, 0, 0, 1}, 
{224, 0, 0, 2}, 
{ 0, 224, 0, 3}, 
{224, 224, 0, 4}, 
{ 0, 0, 224, 5}, 
{224, 0, 224, 6}, 
{ 0, 224, 224, 7}, 
{224, 224, 224, 8}, 
{128, 128, 128, 9}, 
{255, 64, 64, 10}, 
{ 64, 255, 64, 11}, 
{255, 255, 64, 12}, 
{ 64, 64, 255, 13}, 
{255, 64, 255, 14}, 
{ 64, 255, 255, 15}, 
{255, 255, 255, 16}, 
};
void
cterm_color2rgb(int nr, char_u *r, char_u *g, char_u *b, char_u *ansi_idx)
{
int idx;
if (nr < 16)
{
*r = ansi_table[nr][0];
*g = ansi_table[nr][1];
*b = ansi_table[nr][2];
*ansi_idx = ansi_table[nr][3];
}
else if (nr < 232)
{
idx = nr - 16;
*r = cube_value[idx / 36 % 6];
*g = cube_value[idx / 6 % 6];
*b = cube_value[idx % 6];
*ansi_idx = VTERM_ANSI_INDEX_NONE;
}
else if (nr < 256)
{
idx = nr - 232;
*r = grey_ramp[idx];
*g = grey_ramp[idx];
*b = grey_ramp[idx];
*ansi_idx = VTERM_ANSI_INDEX_NONE;
}
else
{
*r = 0;
*g = 0;
*b = 0;
*ansi_idx = 0;
}
}
#endif
void
term_replace_bs_del_keycode(char_u *ta_buf, int ta_len, int len)
{
int i;
int c;
for (i = ta_len; i < ta_len + len; ++i)
{
if (ta_buf[i] == CSI && len - i > 2)
{
c = TERMCAP2KEY(ta_buf[i + 1], ta_buf[i + 2]);
if (c == K_DEL || c == K_KDEL || c == K_BS)
{
mch_memmove(ta_buf + i + 1, ta_buf + i + 3,
(size_t)(len - i - 2));
if (c == K_DEL || c == K_KDEL)
ta_buf[i] = DEL;
else
ta_buf[i] = Ctrl_H;
len -= 2;
}
}
else if (ta_buf[i] == '\r')
ta_buf[i] = '\n';
if (has_mbyte)
i += (*mb_ptr2len_len)(ta_buf + i, ta_len + len - i) - 1;
}
}
