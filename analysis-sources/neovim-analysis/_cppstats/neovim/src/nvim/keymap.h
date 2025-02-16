#include "nvim/strings.h"
#define K_NUL (0xce) 
#define K_SPECIAL (0x80)
#define IS_SPECIAL(c) ((c) < 0)
#define ABBR_OFF 0x100
#define KS_ZERO 255
#define KS_SPECIAL 254
#define KS_EXTRA 253
#define KS_MODIFIER 252
#define KS_MOUSE 251
#define KS_MENU 250
#define KS_VER_SCROLLBAR 249
#define KS_HOR_SCROLLBAR 248
#define KS_SELECT 245
#define K_SELECT_STRING (char_u *)"\200\365X"
#define KS_KEY 242
#define KS_TABLINE 240
#define KS_TABMENU 239
#define KE_FILLER ('X')
#define TERMCAP2KEY(a, b) (-((a) + ((int)(b) << 8)))
#define KEY2TERMCAP0(x) ((-(x)) & 0xff)
#define KEY2TERMCAP1(x) (((unsigned)(-(x)) >> 8) & 0xff)
#define K_SECOND(c) ((c) == K_SPECIAL ? KS_SPECIAL : (c) == NUL ? KS_ZERO : KEY2TERMCAP0(c))
#define K_THIRD(c) (((c) == K_SPECIAL || (c) == NUL) ? KE_FILLER : KEY2TERMCAP1(c))
#define TO_SPECIAL(a, b) ((a) == KS_SPECIAL ? K_SPECIAL : (a) == KS_ZERO ? K_ZERO : TERMCAP2KEY(a, b))
enum key_extra {
KE_NAME = 3 
, KE_S_UP = 4 
, KE_S_DOWN = 5 
, KE_S_F1 = 6 
, KE_S_F2 = 7
, KE_S_F3 = 8
, KE_S_F4 = 9
, KE_S_F5 = 10
, KE_S_F6 = 11
, KE_S_F7 = 12
, KE_S_F8 = 13
, KE_S_F9 = 14
, KE_S_F10 = 15
, KE_S_F11 = 16
, KE_S_F12 = 17
, KE_S_F13 = 18
, KE_S_F14 = 19
, KE_S_F15 = 20
, KE_S_F16 = 21
, KE_S_F17 = 22
, KE_S_F18 = 23
, KE_S_F19 = 24
, KE_S_F20 = 25
, KE_S_F21 = 26
, KE_S_F22 = 27
, KE_S_F23 = 28
, KE_S_F24 = 29
, KE_S_F25 = 30
, KE_S_F26 = 31
, KE_S_F27 = 32
, KE_S_F28 = 33
, KE_S_F29 = 34
, KE_S_F30 = 35
, KE_S_F31 = 36
, KE_S_F32 = 37
, KE_S_F33 = 38
, KE_S_F34 = 39
, KE_S_F35 = 40
, KE_S_F36 = 41
, KE_S_F37 = 42
, KE_MOUSE = 43 
, KE_LEFTMOUSE = 44 
, KE_LEFTDRAG = 45 
, KE_LEFTRELEASE = 46 
, KE_MIDDLEMOUSE = 47 
, KE_MIDDLEDRAG = 48 
, KE_MIDDLERELEASE = 49 
, KE_RIGHTMOUSE = 50 
, KE_RIGHTDRAG = 51 
, KE_RIGHTRELEASE = 52 
, KE_IGNORE = 53 
, KE_TAB = 54 
, KE_S_TAB_OLD = 55 
, KE_XF1 = 57 
, KE_XF2 = 58
, KE_XF3 = 59
, KE_XF4 = 60
, KE_XEND = 61 
, KE_ZEND = 62 
, KE_XHOME = 63 
, KE_ZHOME = 64 
, KE_XUP = 65 
, KE_XDOWN = 66
, KE_XLEFT = 67
, KE_XRIGHT = 68
, KE_LEFTMOUSE_NM = 69 
, KE_LEFTRELEASE_NM = 70 
, KE_S_XF1 = 71 
, KE_S_XF2 = 72
, KE_S_XF3 = 73
, KE_S_XF4 = 74
, KE_MOUSEDOWN = 75 
, KE_MOUSEUP = 76 
, KE_MOUSELEFT = 77 
, KE_MOUSERIGHT = 78 
, KE_KINS = 79 
, KE_KDEL = 80 
, KE_CSI = 81 
, KE_SNR = 82 
, KE_PLUG = 83 
, KE_CMDWIN = 84 
, KE_C_LEFT = 85 
, KE_C_RIGHT = 86 
, KE_C_HOME = 87 
, KE_C_END = 88 
, KE_X1MOUSE = 89 
, KE_X1DRAG = 90
, KE_X1RELEASE = 91
, KE_X2MOUSE = 92
, KE_X2DRAG = 93
, KE_X2RELEASE = 94
, KE_DROP = 95 
, KE_NOP = 97 
, KE_EVENT = 102 
, KE_COMMAND = 104 
};
#define K_ZERO TERMCAP2KEY(KS_ZERO, KE_FILLER)
#define K_UP TERMCAP2KEY('k', 'u')
#define K_KUP TERMCAP2KEY('K', 'u') 
#define K_DOWN TERMCAP2KEY('k', 'd')
#define K_KDOWN TERMCAP2KEY('K', 'd') 
#define K_LEFT TERMCAP2KEY('k', 'l')
#define K_KLEFT TERMCAP2KEY('K', 'l') 
#define K_RIGHT TERMCAP2KEY('k', 'r')
#define K_KRIGHT TERMCAP2KEY('K', 'r') 
#define K_S_UP TERMCAP2KEY(KS_EXTRA, KE_S_UP)
#define K_S_DOWN TERMCAP2KEY(KS_EXTRA, KE_S_DOWN)
#define K_S_LEFT TERMCAP2KEY('#', '4')
#define K_C_LEFT TERMCAP2KEY(KS_EXTRA, KE_C_LEFT)
#define K_S_RIGHT TERMCAP2KEY('%', 'i')
#define K_C_RIGHT TERMCAP2KEY(KS_EXTRA, KE_C_RIGHT)
#define K_S_HOME TERMCAP2KEY('#', '2')
#define K_C_HOME TERMCAP2KEY(KS_EXTRA, KE_C_HOME)
#define K_S_END TERMCAP2KEY('*', '7')
#define K_C_END TERMCAP2KEY(KS_EXTRA, KE_C_END)
#define K_TAB TERMCAP2KEY(KS_EXTRA, KE_TAB)
#define K_S_TAB TERMCAP2KEY('k', 'B')
#define K_XF1 TERMCAP2KEY(KS_EXTRA, KE_XF1)
#define K_XF2 TERMCAP2KEY(KS_EXTRA, KE_XF2)
#define K_XF3 TERMCAP2KEY(KS_EXTRA, KE_XF3)
#define K_XF4 TERMCAP2KEY(KS_EXTRA, KE_XF4)
#define K_XUP TERMCAP2KEY(KS_EXTRA, KE_XUP)
#define K_XDOWN TERMCAP2KEY(KS_EXTRA, KE_XDOWN)
#define K_XLEFT TERMCAP2KEY(KS_EXTRA, KE_XLEFT)
#define K_XRIGHT TERMCAP2KEY(KS_EXTRA, KE_XRIGHT)
#define K_F1 TERMCAP2KEY('k', '1') 
#define K_F2 TERMCAP2KEY('k', '2')
#define K_F3 TERMCAP2KEY('k', '3')
#define K_F4 TERMCAP2KEY('k', '4')
#define K_F5 TERMCAP2KEY('k', '5')
#define K_F6 TERMCAP2KEY('k', '6')
#define K_F7 TERMCAP2KEY('k', '7')
#define K_F8 TERMCAP2KEY('k', '8')
#define K_F9 TERMCAP2KEY('k', '9')
#define K_F10 TERMCAP2KEY('k', ';')
#define K_F11 TERMCAP2KEY('F', '1')
#define K_F12 TERMCAP2KEY('F', '2')
#define K_F13 TERMCAP2KEY('F', '3')
#define K_F14 TERMCAP2KEY('F', '4')
#define K_F15 TERMCAP2KEY('F', '5')
#define K_F16 TERMCAP2KEY('F', '6')
#define K_F17 TERMCAP2KEY('F', '7')
#define K_F18 TERMCAP2KEY('F', '8')
#define K_F19 TERMCAP2KEY('F', '9')
#define K_F20 TERMCAP2KEY('F', 'A')
#define K_F21 TERMCAP2KEY('F', 'B')
#define K_F22 TERMCAP2KEY('F', 'C')
#define K_F23 TERMCAP2KEY('F', 'D')
#define K_F24 TERMCAP2KEY('F', 'E')
#define K_F25 TERMCAP2KEY('F', 'F')
#define K_F26 TERMCAP2KEY('F', 'G')
#define K_F27 TERMCAP2KEY('F', 'H')
#define K_F28 TERMCAP2KEY('F', 'I')
#define K_F29 TERMCAP2KEY('F', 'J')
#define K_F30 TERMCAP2KEY('F', 'K')
#define K_F31 TERMCAP2KEY('F', 'L')
#define K_F32 TERMCAP2KEY('F', 'M')
#define K_F33 TERMCAP2KEY('F', 'N')
#define K_F34 TERMCAP2KEY('F', 'O')
#define K_F35 TERMCAP2KEY('F', 'P')
#define K_F36 TERMCAP2KEY('F', 'Q')
#define K_F37 TERMCAP2KEY('F', 'R')
#define K_S_XF1 TERMCAP2KEY(KS_EXTRA, KE_S_XF1)
#define K_S_XF2 TERMCAP2KEY(KS_EXTRA, KE_S_XF2)
#define K_S_XF3 TERMCAP2KEY(KS_EXTRA, KE_S_XF3)
#define K_S_XF4 TERMCAP2KEY(KS_EXTRA, KE_S_XF4)
#define K_S_F1 TERMCAP2KEY(KS_EXTRA, KE_S_F1) 
#define K_S_F2 TERMCAP2KEY(KS_EXTRA, KE_S_F2)
#define K_S_F3 TERMCAP2KEY(KS_EXTRA, KE_S_F3)
#define K_S_F4 TERMCAP2KEY(KS_EXTRA, KE_S_F4)
#define K_S_F5 TERMCAP2KEY(KS_EXTRA, KE_S_F5)
#define K_S_F6 TERMCAP2KEY(KS_EXTRA, KE_S_F6)
#define K_S_F7 TERMCAP2KEY(KS_EXTRA, KE_S_F7)
#define K_S_F8 TERMCAP2KEY(KS_EXTRA, KE_S_F8)
#define K_S_F9 TERMCAP2KEY(KS_EXTRA, KE_S_F9)
#define K_S_F10 TERMCAP2KEY(KS_EXTRA, KE_S_F10)
#define K_S_F11 TERMCAP2KEY(KS_EXTRA, KE_S_F11)
#define K_S_F12 TERMCAP2KEY(KS_EXTRA, KE_S_F12)
#define K_HELP TERMCAP2KEY('%', '1')
#define K_UNDO TERMCAP2KEY('&', '8')
#define K_BS TERMCAP2KEY('k', 'b')
#define K_INS TERMCAP2KEY('k', 'I')
#define K_KINS TERMCAP2KEY(KS_EXTRA, KE_KINS)
#define K_DEL TERMCAP2KEY('k', 'D')
#define K_KDEL TERMCAP2KEY(KS_EXTRA, KE_KDEL)
#define K_HOME TERMCAP2KEY('k', 'h')
#define K_KHOME TERMCAP2KEY('K', '1') 
#define K_XHOME TERMCAP2KEY(KS_EXTRA, KE_XHOME)
#define K_ZHOME TERMCAP2KEY(KS_EXTRA, KE_ZHOME)
#define K_END TERMCAP2KEY('@', '7')
#define K_KEND TERMCAP2KEY('K', '4') 
#define K_XEND TERMCAP2KEY(KS_EXTRA, KE_XEND)
#define K_ZEND TERMCAP2KEY(KS_EXTRA, KE_ZEND)
#define K_PAGEUP TERMCAP2KEY('k', 'P')
#define K_PAGEDOWN TERMCAP2KEY('k', 'N')
#define K_KPAGEUP TERMCAP2KEY('K', '3') 
#define K_KPAGEDOWN TERMCAP2KEY('K', '5') 
#define K_KORIGIN TERMCAP2KEY('K', '2') 
#define K_KPLUS TERMCAP2KEY('K', '6') 
#define K_KMINUS TERMCAP2KEY('K', '7') 
#define K_KDIVIDE TERMCAP2KEY('K', '8') 
#define K_KMULTIPLY TERMCAP2KEY('K', '9') 
#define K_KENTER TERMCAP2KEY('K', 'A') 
#define K_KPOINT TERMCAP2KEY('K', 'B') 
#define K_K0 TERMCAP2KEY('K', 'C') 
#define K_K1 TERMCAP2KEY('K', 'D') 
#define K_K2 TERMCAP2KEY('K', 'E') 
#define K_K3 TERMCAP2KEY('K', 'F') 
#define K_K4 TERMCAP2KEY('K', 'G') 
#define K_K5 TERMCAP2KEY('K', 'H') 
#define K_K6 TERMCAP2KEY('K', 'I') 
#define K_K7 TERMCAP2KEY('K', 'J') 
#define K_K8 TERMCAP2KEY('K', 'K') 
#define K_K9 TERMCAP2KEY('K', 'L') 
#define K_KCOMMA TERMCAP2KEY('K', 'M') 
#define K_KEQUAL TERMCAP2KEY('K', 'N') 
#define K_MOUSE TERMCAP2KEY(KS_MOUSE, KE_FILLER)
#define K_MENU TERMCAP2KEY(KS_MENU, KE_FILLER)
#define K_VER_SCROLLBAR TERMCAP2KEY(KS_VER_SCROLLBAR, KE_FILLER)
#define K_HOR_SCROLLBAR TERMCAP2KEY(KS_HOR_SCROLLBAR, KE_FILLER)
#define K_SELECT TERMCAP2KEY(KS_SELECT, KE_FILLER)
#define K_TABLINE TERMCAP2KEY(KS_TABLINE, KE_FILLER)
#define K_TABMENU TERMCAP2KEY(KS_TABMENU, KE_FILLER)
#define K_LEFTMOUSE TERMCAP2KEY(KS_EXTRA, KE_LEFTMOUSE)
#define K_LEFTMOUSE_NM TERMCAP2KEY(KS_EXTRA, KE_LEFTMOUSE_NM)
#define K_LEFTDRAG TERMCAP2KEY(KS_EXTRA, KE_LEFTDRAG)
#define K_LEFTRELEASE TERMCAP2KEY(KS_EXTRA, KE_LEFTRELEASE)
#define K_LEFTRELEASE_NM TERMCAP2KEY(KS_EXTRA, KE_LEFTRELEASE_NM)
#define K_MIDDLEMOUSE TERMCAP2KEY(KS_EXTRA, KE_MIDDLEMOUSE)
#define K_MIDDLEDRAG TERMCAP2KEY(KS_EXTRA, KE_MIDDLEDRAG)
#define K_MIDDLERELEASE TERMCAP2KEY(KS_EXTRA, KE_MIDDLERELEASE)
#define K_RIGHTMOUSE TERMCAP2KEY(KS_EXTRA, KE_RIGHTMOUSE)
#define K_RIGHTDRAG TERMCAP2KEY(KS_EXTRA, KE_RIGHTDRAG)
#define K_RIGHTRELEASE TERMCAP2KEY(KS_EXTRA, KE_RIGHTRELEASE)
#define K_X1MOUSE TERMCAP2KEY(KS_EXTRA, KE_X1MOUSE)
#define K_X1MOUSE TERMCAP2KEY(KS_EXTRA, KE_X1MOUSE)
#define K_X1DRAG TERMCAP2KEY(KS_EXTRA, KE_X1DRAG)
#define K_X1RELEASE TERMCAP2KEY(KS_EXTRA, KE_X1RELEASE)
#define K_X2MOUSE TERMCAP2KEY(KS_EXTRA, KE_X2MOUSE)
#define K_X2DRAG TERMCAP2KEY(KS_EXTRA, KE_X2DRAG)
#define K_X2RELEASE TERMCAP2KEY(KS_EXTRA, KE_X2RELEASE)
#define K_IGNORE TERMCAP2KEY(KS_EXTRA, KE_IGNORE)
#define K_NOP TERMCAP2KEY(KS_EXTRA, KE_NOP)
#define K_MOUSEDOWN TERMCAP2KEY(KS_EXTRA, KE_MOUSEDOWN)
#define K_MOUSEUP TERMCAP2KEY(KS_EXTRA, KE_MOUSEUP)
#define K_MOUSELEFT TERMCAP2KEY(KS_EXTRA, KE_MOUSELEFT)
#define K_MOUSERIGHT TERMCAP2KEY(KS_EXTRA, KE_MOUSERIGHT)
#define K_CSI TERMCAP2KEY(KS_EXTRA, KE_CSI)
#define K_SNR TERMCAP2KEY(KS_EXTRA, KE_SNR)
#define K_PLUG TERMCAP2KEY(KS_EXTRA, KE_PLUG)
#define K_CMDWIN TERMCAP2KEY(KS_EXTRA, KE_CMDWIN)
#define K_DROP TERMCAP2KEY(KS_EXTRA, KE_DROP)
#define K_EVENT TERMCAP2KEY(KS_EXTRA, KE_EVENT)
#define K_COMMAND TERMCAP2KEY(KS_EXTRA, KE_COMMAND)
#define MOD_MASK_SHIFT 0x02
#define MOD_MASK_CTRL 0x04
#define MOD_MASK_ALT 0x08 
#define MOD_MASK_META 0x10 
#define MOD_MASK_2CLICK 0x20 
#define MOD_MASK_3CLICK 0x40 
#define MOD_MASK_4CLICK 0x60 
#define MOD_MASK_CMD 0x80 
#define MOD_MASK_MULTI_CLICK (MOD_MASK_2CLICK|MOD_MASK_3CLICK| MOD_MASK_4CLICK)
#define MAX_KEY_NAME_LEN 32
#define MAX_KEY_CODE_LEN 6
#define FLAG_CPO_BSLASH 0x01
#define CPO_TO_CPO_FLAGS ((vim_strchr(p_cpo, CPO_BSLASH) == NULL) ? 0 : FLAG_CPO_BSLASH)
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "keymap.h.generated.h"
#endif
