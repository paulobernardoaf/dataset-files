#if !defined(NVIM_HIGHLIGHT_DEFS_H)
#define NVIM_HIGHLIGHT_DEFS_H

#include <inttypes.h>

#include "nvim/macros.h"

typedef int32_t RgbValue;




typedef enum {
HL_INVERSE = 0x01,
HL_BOLD = 0x02,
HL_ITALIC = 0x04,
HL_UNDERLINE = 0x08,
HL_UNDERCURL = 0x10,
HL_STANDOUT = 0x20,
HL_STRIKETHROUGH = 0x40,
HL_NOCOMBINE = 0x80,
HL_BG_INDEXED = 0x0100,
HL_FG_INDEXED = 0x0200,
} HlAttrFlags;



typedef struct attr_entry {
int16_t rgb_ae_attr, cterm_ae_attr; 
RgbValue rgb_fg_color, rgb_bg_color, rgb_sp_color;
int cterm_fg_color, cterm_bg_color;
int hl_blend;
} HlAttrs;

#define HLATTRS_INIT (HlAttrs) { .rgb_ae_attr = 0, .cterm_ae_attr = 0, .rgb_fg_color = -1, .rgb_bg_color = -1, .rgb_sp_color = -1, .cterm_fg_color = 0, .cterm_bg_color = 0, .hl_blend = -1, }












typedef enum {
HLF_8 = 0 

, HLF_EOB 
, HLF_TERM 
, HLF_TERMNC 
, HLF_AT 

, HLF_D 
, HLF_E 
, HLF_I 
, HLF_L 
, HLF_M 
, HLF_CM 
, HLF_N 
, HLF_CLN 
, HLF_R 
, HLF_S 
, HLF_SNC 
, HLF_C 
, HLF_T 
, HLF_V 
, HLF_VNC 
, HLF_W 
, HLF_WM 
, HLF_FL 
, HLF_FC 
, HLF_ADD 
, HLF_CHD 
, HLF_DED 
, HLF_TXD 
, HLF_SC 
, HLF_CONCEAL 
, HLF_SPB 
, HLF_SPC 
, HLF_SPR 
, HLF_SPL 
, HLF_PNI 
, HLF_PSI 
, HLF_PSB 
, HLF_PST 
, HLF_TP 
, HLF_TPS 
, HLF_TPF 
, HLF_CUC 
, HLF_CUL 
, HLF_MC 
, HLF_QFL 
, HLF_0 
, HLF_INACTIVE 
, HLF_MSGSEP 
, HLF_NFLOAT 
, HLF_MSG 
, HLF_COUNT 
} hlf_T;

EXTERN const char *hlf_names[] INIT(= {
[HLF_8] = "SpecialKey",
[HLF_EOB] = "EndOfBuffer",
[HLF_TERM] = "TermCursor",
[HLF_TERMNC] = "TermCursorNC",
[HLF_AT] = "NonText",
[HLF_D] = "Directory",
[HLF_E] = "ErrorMsg",
[HLF_I] = "IncSearch",
[HLF_L] = "Search",
[HLF_M] = "MoreMsg",
[HLF_CM] = "ModeMsg",
[HLF_N] = "LineNr",
[HLF_CLN] = "CursorLineNr",
[HLF_R] = "Question",
[HLF_S] = "StatusLine",
[HLF_SNC] = "StatusLineNC",
[HLF_C] = "VertSplit",
[HLF_T] = "Title",
[HLF_V] = "Visual",
[HLF_VNC] = "VisualNC",
[HLF_W] = "WarningMsg",
[HLF_WM] = "WildMenu",
[HLF_FL] = "Folded",
[HLF_FC] = "FoldColumn",
[HLF_ADD] = "DiffAdd",
[HLF_CHD] = "DiffChange",
[HLF_DED] = "DiffDelete",
[HLF_TXD] = "DiffText",
[HLF_SC] = "SignColumn",
[HLF_CONCEAL] = "Conceal",
[HLF_SPB] = "SpellBad",
[HLF_SPC] = "SpellCap",
[HLF_SPR] = "SpellRare",
[HLF_SPL] = "SpellLocal",
[HLF_PNI] = "Pmenu",
[HLF_PSI] = "PmenuSel",
[HLF_PSB] = "PmenuSbar",
[HLF_PST] = "PmenuThumb",
[HLF_TP] = "TabLine",
[HLF_TPS] = "TabLineSel",
[HLF_TPF] = "TabLineFill",
[HLF_CUC] = "CursorColumn",
[HLF_CUL] = "CursorLine",
[HLF_MC] = "ColorColumn",
[HLF_QFL] = "QuickFixLine",
[HLF_0] = "Whitespace",
[HLF_INACTIVE] = "NormalNC",
[HLF_MSGSEP] = "MsgSeparator",
[HLF_NFLOAT] = "NormalFloat",
[HLF_MSG] = "MsgArea",
});


EXTERN int highlight_attr[HLF_COUNT]; 
EXTERN int highlight_attr_last[HLF_COUNT]; 
EXTERN int highlight_user[9]; 
EXTERN int highlight_stlnc[9]; 
EXTERN int cterm_normal_fg_color INIT(= 0);
EXTERN int cterm_normal_bg_color INIT(= 0);
EXTERN RgbValue normal_fg INIT(= -1);
EXTERN RgbValue normal_bg INIT(= -1);
EXTERN RgbValue normal_sp INIT(= -1);

typedef enum {
kHlUnknown,
kHlUI,
kHlSyntax,
kHlTerminal,
kHlCombine,
kHlBlend,
kHlBlendThrough,
} HlKind;

typedef struct {
HlAttrs attr;
HlKind kind;
int id1;
int id2;
} HlEntry;

#endif 
