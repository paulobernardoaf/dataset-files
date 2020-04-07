













#include "vim.h"

#define SG_TERM 1 
#define SG_CTERM 2 
#define SG_GUI 4 
#define SG_LINK 8 





static char *(hl_name_table[]) =
{"bold", "standout", "underline", "undercurl",
"italic", "reverse", "inverse", "nocombine", "strikethrough", "NONE"};
static int hl_attr_table[] =
{HL_BOLD, HL_STANDOUT, HL_UNDERLINE, HL_UNDERCURL, HL_ITALIC, HL_INVERSE, HL_INVERSE, HL_NOCOMBINE, HL_STRIKETHROUGH, 0};
#define ATTR_COMBINE(attr_a, attr_b) ((((attr_b) & HL_NOCOMBINE) ? attr_b : (attr_a)) | (attr_b))






typedef struct
{
char_u *sg_name; 
char_u *sg_name_u; 
int sg_cleared; 

int sg_term; 
char_u *sg_start; 
char_u *sg_stop; 
int sg_term_attr; 

int sg_cterm; 
int sg_cterm_bold; 
int sg_cterm_fg; 
int sg_cterm_bg; 
int sg_cterm_attr; 

#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)
guicolor_T sg_gui_fg; 
guicolor_T sg_gui_bg; 
#endif
#if defined(FEAT_GUI)
guicolor_T sg_gui_sp; 
GuiFont sg_font; 
#if defined(FEAT_XFONTSET)
GuiFontset sg_fontset; 
#endif
char_u *sg_font_name; 
int sg_gui_attr; 
#endif
#if defined(FEAT_GUI) || defined(FEAT_EVAL)

int sg_gui; 
char_u *sg_gui_fg_name;
char_u *sg_gui_bg_name;
char_u *sg_gui_sp_name;
#endif
int sg_link; 
int sg_set; 
#if defined(FEAT_EVAL)
sctx_T sg_script_ctx; 
#endif
} hl_group_T;


static garray_T highlight_ga;
#define HL_TABLE() ((hl_group_T *)((highlight_ga.ga_data)))




#define ATTR_OFF (HL_ALL + 1)

static void syn_unadd_group(void);
static void set_hl_attr(int idx);
static void highlight_list_one(int id);
static int highlight_list_arg(int id, int didh, int type, int iarg, char_u *sarg, char *name);
static int syn_add_group(char_u *name);
static int hl_has_settings(int idx, int check_link);
static void highlight_clear(int idx);

#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)
static void gui_do_one_color(int idx, int do_menu, int do_tooltip);
#endif
#if defined(FEAT_GUI)
static int set_group_colors(char_u *name, guicolor_T *fgp, guicolor_T *bgp, int do_menu, int use_norm, int do_tooltip);
static void hl_do_font(int idx, char_u *arg, int do_normal, int do_menu, int do_tooltip, int free_font);
#endif








#if defined(FEAT_GUI) || defined(FEAT_EVAL)
#define CENT(a, b) b
#else
#define CENT(a, b) a
#endif
static char *(highlight_init_both[]) = {
CENT("ErrorMsg term=standout ctermbg=DarkRed ctermfg=White",
"ErrorMsg term=standout ctermbg=DarkRed ctermfg=White guibg=Red guifg=White"),
CENT("IncSearch term=reverse cterm=reverse",
"IncSearch term=reverse cterm=reverse gui=reverse"),
CENT("ModeMsg term=bold cterm=bold",
"ModeMsg term=bold cterm=bold gui=bold"),
CENT("NonText term=bold ctermfg=Blue",
"NonText term=bold ctermfg=Blue gui=bold guifg=Blue"),
CENT("StatusLine term=reverse,bold cterm=reverse,bold",
"StatusLine term=reverse,bold cterm=reverse,bold gui=reverse,bold"),
CENT("StatusLineNC term=reverse cterm=reverse",
"StatusLineNC term=reverse cterm=reverse gui=reverse"),
"default link EndOfBuffer NonText",
CENT("VertSplit term=reverse cterm=reverse",
"VertSplit term=reverse cterm=reverse gui=reverse"),
#if defined(FEAT_CLIPBOARD)
CENT("VisualNOS term=underline,bold cterm=underline,bold",
"VisualNOS term=underline,bold cterm=underline,bold gui=underline,bold"),
#endif
#if defined(FEAT_DIFF)
CENT("DiffText term=reverse cterm=bold ctermbg=Red",
"DiffText term=reverse cterm=bold ctermbg=Red gui=bold guibg=Red"),
#endif
CENT("PmenuSbar ctermbg=Grey",
"PmenuSbar ctermbg=Grey guibg=Grey"),
CENT("TabLineSel term=bold cterm=bold",
"TabLineSel term=bold cterm=bold gui=bold"),
CENT("TabLineFill term=reverse cterm=reverse",
"TabLineFill term=reverse cterm=reverse gui=reverse"),
#if defined(FEAT_GUI)
"Cursor guibg=fg guifg=bg",
"lCursor guibg=fg guifg=bg", 
#endif
"default link QuickFixLine Search",
CENT("Normal cterm=NONE", "Normal gui=NONE"),
NULL
};


static char *(highlight_init_light[]) = {
CENT("Directory term=bold ctermfg=DarkBlue",
"Directory term=bold ctermfg=DarkBlue guifg=Blue"),
CENT("LineNr term=underline ctermfg=Brown",
"LineNr term=underline ctermfg=Brown guifg=Brown"),
CENT("CursorLineNr term=bold cterm=underline ctermfg=Brown",
"CursorLineNr term=bold cterm=underline ctermfg=Brown gui=bold guifg=Brown"),
CENT("MoreMsg term=bold ctermfg=DarkGreen",
"MoreMsg term=bold ctermfg=DarkGreen gui=bold guifg=SeaGreen"),
CENT("Question term=standout ctermfg=DarkGreen",
"Question term=standout ctermfg=DarkGreen gui=bold guifg=SeaGreen"),
CENT("Search term=reverse ctermbg=Yellow ctermfg=NONE",
"Search term=reverse ctermbg=Yellow ctermfg=NONE guibg=Yellow guifg=NONE"),
#if defined(FEAT_SPELL)
CENT("SpellBad term=reverse ctermbg=LightRed",
"SpellBad term=reverse ctermbg=LightRed guisp=Red gui=undercurl"),
CENT("SpellCap term=reverse ctermbg=LightBlue",
"SpellCap term=reverse ctermbg=LightBlue guisp=Blue gui=undercurl"),
CENT("SpellRare term=reverse ctermbg=LightMagenta",
"SpellRare term=reverse ctermbg=LightMagenta guisp=Magenta gui=undercurl"),
CENT("SpellLocal term=underline ctermbg=Cyan",
"SpellLocal term=underline ctermbg=Cyan guisp=DarkCyan gui=undercurl"),
#endif
CENT("PmenuThumb ctermbg=Black",
"PmenuThumb ctermbg=Black guibg=Black"),
CENT("Pmenu ctermbg=LightMagenta ctermfg=Black",
"Pmenu ctermbg=LightMagenta ctermfg=Black guibg=LightMagenta"),
CENT("PmenuSel ctermbg=LightGrey ctermfg=Black",
"PmenuSel ctermbg=LightGrey ctermfg=Black guibg=Grey"),
CENT("SpecialKey term=bold ctermfg=DarkBlue",
"SpecialKey term=bold ctermfg=DarkBlue guifg=Blue"),
CENT("Title term=bold ctermfg=DarkMagenta",
"Title term=bold ctermfg=DarkMagenta gui=bold guifg=Magenta"),
CENT("WarningMsg term=standout ctermfg=DarkRed",
"WarningMsg term=standout ctermfg=DarkRed guifg=Red"),
#if defined(FEAT_WILDMENU)
CENT("WildMenu term=standout ctermbg=Yellow ctermfg=Black",
"WildMenu term=standout ctermbg=Yellow ctermfg=Black guibg=Yellow guifg=Black"),
#endif
#if defined(FEAT_FOLDING)
CENT("Folded term=standout ctermbg=Grey ctermfg=DarkBlue",
"Folded term=standout ctermbg=Grey ctermfg=DarkBlue guibg=LightGrey guifg=DarkBlue"),
CENT("FoldColumn term=standout ctermbg=Grey ctermfg=DarkBlue",
"FoldColumn term=standout ctermbg=Grey ctermfg=DarkBlue guibg=Grey guifg=DarkBlue"),
#endif
#if defined(FEAT_SIGNS)
CENT("SignColumn term=standout ctermbg=Grey ctermfg=DarkBlue",
"SignColumn term=standout ctermbg=Grey ctermfg=DarkBlue guibg=Grey guifg=DarkBlue"),
#endif
CENT("Visual term=reverse",
"Visual term=reverse guibg=LightGrey"),
#if defined(FEAT_DIFF)
CENT("DiffAdd term=bold ctermbg=LightBlue",
"DiffAdd term=bold ctermbg=LightBlue guibg=LightBlue"),
CENT("DiffChange term=bold ctermbg=LightMagenta",
"DiffChange term=bold ctermbg=LightMagenta guibg=LightMagenta"),
CENT("DiffDelete term=bold ctermfg=Blue ctermbg=LightCyan",
"DiffDelete term=bold ctermfg=Blue ctermbg=LightCyan gui=bold guifg=Blue guibg=LightCyan"),
#endif
CENT("TabLine term=underline cterm=underline ctermfg=black ctermbg=LightGrey",
"TabLine term=underline cterm=underline ctermfg=black ctermbg=LightGrey gui=underline guibg=LightGrey"),
#if defined(FEAT_SYN_HL)
CENT("CursorColumn term=reverse ctermbg=LightGrey",
"CursorColumn term=reverse ctermbg=LightGrey guibg=Grey90"),
CENT("CursorLine term=underline cterm=underline",
"CursorLine term=underline cterm=underline guibg=Grey90"),
CENT("ColorColumn term=reverse ctermbg=LightRed",
"ColorColumn term=reverse ctermbg=LightRed guibg=LightRed"),
#endif
#if defined(FEAT_CONCEAL)
CENT("Conceal ctermbg=DarkGrey ctermfg=LightGrey",
"Conceal ctermbg=DarkGrey ctermfg=LightGrey guibg=DarkGrey guifg=LightGrey"),
#endif
CENT("MatchParen term=reverse ctermbg=Cyan",
"MatchParen term=reverse ctermbg=Cyan guibg=Cyan"),
#if defined(FEAT_TERMINAL)
CENT("StatusLineTerm term=reverse,bold cterm=bold ctermfg=White ctermbg=DarkGreen",
"StatusLineTerm term=reverse,bold cterm=bold ctermfg=White ctermbg=DarkGreen gui=bold guifg=bg guibg=DarkGreen"),
CENT("StatusLineTermNC term=reverse ctermfg=White ctermbg=DarkGreen",
"StatusLineTermNC term=reverse ctermfg=White ctermbg=DarkGreen guifg=bg guibg=DarkGreen"),
#endif
#if defined(FEAT_MENU)
CENT("ToolbarLine term=underline ctermbg=LightGrey",
"ToolbarLine term=underline ctermbg=LightGrey guibg=LightGrey"),
CENT("ToolbarButton cterm=bold ctermfg=White ctermbg=DarkGrey",
"ToolbarButton cterm=bold ctermfg=White ctermbg=DarkGrey gui=bold guifg=White guibg=Grey40"),
#endif
NULL
};


static char *(highlight_init_dark[]) = {
CENT("Directory term=bold ctermfg=LightCyan",
"Directory term=bold ctermfg=LightCyan guifg=Cyan"),
CENT("LineNr term=underline ctermfg=Yellow",
"LineNr term=underline ctermfg=Yellow guifg=Yellow"),
CENT("CursorLineNr term=bold cterm=underline ctermfg=Yellow",
"CursorLineNr term=bold cterm=underline ctermfg=Yellow gui=bold guifg=Yellow"),
CENT("MoreMsg term=bold ctermfg=LightGreen",
"MoreMsg term=bold ctermfg=LightGreen gui=bold guifg=SeaGreen"),
CENT("Question term=standout ctermfg=LightGreen",
"Question term=standout ctermfg=LightGreen gui=bold guifg=Green"),
CENT("Search term=reverse ctermbg=Yellow ctermfg=Black",
"Search term=reverse ctermbg=Yellow ctermfg=Black guibg=Yellow guifg=Black"),
CENT("SpecialKey term=bold ctermfg=LightBlue",
"SpecialKey term=bold ctermfg=LightBlue guifg=Cyan"),
#if defined(FEAT_SPELL)
CENT("SpellBad term=reverse ctermbg=Red",
"SpellBad term=reverse ctermbg=Red guisp=Red gui=undercurl"),
CENT("SpellCap term=reverse ctermbg=Blue",
"SpellCap term=reverse ctermbg=Blue guisp=Blue gui=undercurl"),
CENT("SpellRare term=reverse ctermbg=Magenta",
"SpellRare term=reverse ctermbg=Magenta guisp=Magenta gui=undercurl"),
CENT("SpellLocal term=underline ctermbg=Cyan",
"SpellLocal term=underline ctermbg=Cyan guisp=Cyan gui=undercurl"),
#endif
CENT("PmenuThumb ctermbg=White",
"PmenuThumb ctermbg=White guibg=White"),
CENT("Pmenu ctermbg=Magenta ctermfg=Black",
"Pmenu ctermbg=Magenta ctermfg=Black guibg=Magenta"),
CENT("PmenuSel ctermbg=Black ctermfg=DarkGrey",
"PmenuSel ctermbg=Black ctermfg=DarkGrey guibg=DarkGrey"),
CENT("Title term=bold ctermfg=LightMagenta",
"Title term=bold ctermfg=LightMagenta gui=bold guifg=Magenta"),
CENT("WarningMsg term=standout ctermfg=LightRed",
"WarningMsg term=standout ctermfg=LightRed guifg=Red"),
#if defined(FEAT_WILDMENU)
CENT("WildMenu term=standout ctermbg=Yellow ctermfg=Black",
"WildMenu term=standout ctermbg=Yellow ctermfg=Black guibg=Yellow guifg=Black"),
#endif
#if defined(FEAT_FOLDING)
CENT("Folded term=standout ctermbg=DarkGrey ctermfg=Cyan",
"Folded term=standout ctermbg=DarkGrey ctermfg=Cyan guibg=DarkGrey guifg=Cyan"),
CENT("FoldColumn term=standout ctermbg=DarkGrey ctermfg=Cyan",
"FoldColumn term=standout ctermbg=DarkGrey ctermfg=Cyan guibg=Grey guifg=Cyan"),
#endif
#if defined(FEAT_SIGNS)
CENT("SignColumn term=standout ctermbg=DarkGrey ctermfg=Cyan",
"SignColumn term=standout ctermbg=DarkGrey ctermfg=Cyan guibg=Grey guifg=Cyan"),
#endif
CENT("Visual term=reverse",
"Visual term=reverse guibg=DarkGrey"),
#if defined(FEAT_DIFF)
CENT("DiffAdd term=bold ctermbg=DarkBlue",
"DiffAdd term=bold ctermbg=DarkBlue guibg=DarkBlue"),
CENT("DiffChange term=bold ctermbg=DarkMagenta",
"DiffChange term=bold ctermbg=DarkMagenta guibg=DarkMagenta"),
CENT("DiffDelete term=bold ctermfg=Blue ctermbg=DarkCyan",
"DiffDelete term=bold ctermfg=Blue ctermbg=DarkCyan gui=bold guifg=Blue guibg=DarkCyan"),
#endif
CENT("TabLine term=underline cterm=underline ctermfg=white ctermbg=DarkGrey",
"TabLine term=underline cterm=underline ctermfg=white ctermbg=DarkGrey gui=underline guibg=DarkGrey"),
#if defined(FEAT_SYN_HL)
CENT("CursorColumn term=reverse ctermbg=DarkGrey",
"CursorColumn term=reverse ctermbg=DarkGrey guibg=Grey40"),
CENT("CursorLine term=underline cterm=underline",
"CursorLine term=underline cterm=underline guibg=Grey40"),
CENT("ColorColumn term=reverse ctermbg=DarkRed",
"ColorColumn term=reverse ctermbg=DarkRed guibg=DarkRed"),
#endif
CENT("MatchParen term=reverse ctermbg=DarkCyan",
"MatchParen term=reverse ctermbg=DarkCyan guibg=DarkCyan"),
#if defined(FEAT_CONCEAL)
CENT("Conceal ctermbg=DarkGrey ctermfg=LightGrey",
"Conceal ctermbg=DarkGrey ctermfg=LightGrey guibg=DarkGrey guifg=LightGrey"),
#endif
#if defined(FEAT_TERMINAL)
CENT("StatusLineTerm term=reverse,bold cterm=bold ctermfg=Black ctermbg=LightGreen",
"StatusLineTerm term=reverse,bold cterm=bold ctermfg=Black ctermbg=LightGreen gui=bold guifg=bg guibg=LightGreen"),
CENT("StatusLineTermNC term=reverse ctermfg=Black ctermbg=LightGreen",
"StatusLineTermNC term=reverse ctermfg=Black ctermbg=LightGreen guifg=bg guibg=LightGreen"),
#endif
#if defined(FEAT_MENU)
CENT("ToolbarLine term=underline ctermbg=DarkGrey",
"ToolbarLine term=underline ctermbg=DarkGrey guibg=Grey50"),
CENT("ToolbarButton cterm=bold ctermfg=Black ctermbg=LightGrey",
"ToolbarButton cterm=bold ctermfg=Black ctermbg=LightGrey gui=bold guifg=Black guibg=LightGrey"),
#endif
NULL
};




int
highlight_num_groups(void)
{
return highlight_ga.ga_len;
}




char_u *
highlight_group_name(int id)
{
return HL_TABLE()[id].sg_name;
}




int
highlight_link_id(int id)
{
return HL_TABLE()[id].sg_link;
}

void
init_highlight(
int both, 
int reset) 
{
int i;
char **pp;
static int had_both = FALSE;
#if defined(FEAT_EVAL)
char_u *p;





p = get_var_value((char_u *)"g:colors_name");
if (p != NULL)
{


char_u *copy_p = vim_strsave(p);
int r;

if (copy_p != NULL)
{
r = load_colors(copy_p);
vim_free(copy_p);
if (r == OK)
return;
}
}

#endif




if (both)
{
had_both = TRUE;
pp = highlight_init_both;
for (i = 0; pp[i] != NULL; ++i)
do_highlight((char_u *)pp[i], reset, TRUE);
}
else if (!had_both)



return;

if (*p_bg == 'l')
pp = highlight_init_light;
else
pp = highlight_init_dark;
for (i = 0; pp[i] != NULL; ++i)
do_highlight((char_u *)pp[i], reset, TRUE);






if (t_colors > 8)
do_highlight((char_u *)(*p_bg == 'l'
? "Visual cterm=NONE ctermbg=LightGrey"
: "Visual cterm=NONE ctermbg=DarkGrey"), FALSE, TRUE);
else
{
do_highlight((char_u *)"Visual cterm=reverse ctermbg=NONE",
FALSE, TRUE);
if (*p_bg == 'l')
do_highlight((char_u *)"Search ctermfg=black", FALSE, TRUE);
}

#if defined(FEAT_SYN_HL)



if (get_var_value((char_u *)"g:syntax_on") != NULL)
{
static int recursive = 0;

if (recursive >= 5)
emsg(_("E679: recursive loop loading syncolor.vim"));
else
{
++recursive;
(void)source_runtime((char_u *)"syntax/syncolor.vim", DIP_ALL);
--recursive;
}
}
#endif
}





int
load_colors(char_u *name)
{
char_u *buf;
int retval = FAIL;
static int recursive = FALSE;




if (recursive)
return OK;

recursive = TRUE;
buf = alloc(STRLEN(name) + 12);
if (buf != NULL)
{
apply_autocmds(EVENT_COLORSCHEMEPRE, name,
curbuf->b_fname, FALSE, curbuf);
sprintf((char *)buf, "colors/%s.vim", name);
retval = source_runtime(buf, DIP_START + DIP_OPT);
vim_free(buf);
apply_autocmds(EVENT_COLORSCHEME, name, curbuf->b_fname, FALSE, curbuf);
}
recursive = FALSE;

return retval;
}

static char *(color_names[28]) = {
"Black", "DarkBlue", "DarkGreen", "DarkCyan",
"DarkRed", "DarkMagenta", "Brown", "DarkYellow",
"Gray", "Grey", "LightGray", "LightGrey",
"DarkGray", "DarkGrey",
"Blue", "LightBlue", "Green", "LightGreen",
"Cyan", "LightCyan", "Red", "LightRed", "Magenta",
"LightMagenta", "Yellow", "LightYellow", "White", "NONE"};








static int color_numbers_16[28] = {0, 1, 2, 3,
4, 5, 6, 6,
7, 7, 7, 7,
8, 8,
9, 9, 10, 10,
11, 11, 12, 12, 13,
13, 14, 14, 15, -1};

static int color_numbers_88[28] = {0, 4, 2, 6,
1, 5, 32, 72,
84, 84, 7, 7,
82, 82,
12, 43, 10, 61,
14, 63, 9, 74, 13,
75, 11, 78, 15, -1};

static int color_numbers_256[28] = {0, 4, 2, 6,
1, 5, 130, 130,
248, 248, 7, 7,
242, 242,
12, 81, 10, 121,
14, 159, 9, 224, 13,
225, 11, 229, 15, -1};

static int color_numbers_8[28] = {0, 4, 2, 6,
1, 5, 3, 3,
7, 7, 7, 7,
0+8, 0+8,
4+8, 4+8, 2+8, 2+8,
6+8, 6+8, 1+8, 1+8, 5+8,
5+8, 3+8, 3+8, 7+8, -1};







int
lookup_color(int idx, int foreground, int *boldp)
{
int color = color_numbers_16[idx];
char_u *p;


if (color < 0)
return -1;

if (t_colors == 8)
{

#if defined(__QNXNTO__)

if (STRNCMP(T_NAME, "qansi", 5) == 0)
color = color_numbers_16[idx];
else
#endif
color = color_numbers_8[idx];
if (foreground)
{


if (color & 8)
*boldp = TRUE;
else
*boldp = FALSE;
}
color &= 7; 
}
else if (t_colors == 16 || t_colors == 88
|| t_colors >= 256)
{





if (*T_CAF != NUL)
p = T_CAF;
else
p = T_CSF;
if (*p != NUL && (t_colors > 256
|| *(p + STRLEN(p) - 1) == 'm'))
{
if (t_colors == 88)
color = color_numbers_88[idx];
else if (t_colors >= 256)
color = color_numbers_256[idx];
else
color = color_numbers_8[idx];
}
#if defined(FEAT_TERMRESPONSE)
if (t_colors >= 256 && color == 15 && is_mac_terminal)


color = 231;
#endif
}
return color;
}






void
do_highlight(
char_u *line,
int forceit,
int init) 
{
char_u *name_end;
char_u *p;
char_u *linep;
char_u *key_start;
char_u *arg_start;
char_u *key = NULL, *arg = NULL;
long i;
int off;
int len;
int attr;
int id;
int idx;
hl_group_T item_before;
int did_change = FALSE;
int dodefault = FALSE;
int doclear = FALSE;
int dolink = FALSE;
int error = FALSE;
int color;
int is_normal_group = FALSE; 
#if defined(FEAT_TERMINAL)
int is_terminal_group = FALSE; 
#endif
#if defined(FEAT_GUI_X11)
int is_menu_group = FALSE; 
int is_scrollbar_group = FALSE; 
int is_tooltip_group = FALSE; 
int do_colors = FALSE; 
#else
#define is_menu_group 0
#define is_tooltip_group 0
#endif
#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)
int did_highlight_changed = FALSE;
#endif




if (ends_excmd(*line))
{
for (i = 1; i <= highlight_ga.ga_len && !got_int; ++i)

highlight_list_one((int)i);
return;
}




name_end = skiptowhite(line);
linep = skipwhite(name_end);




if (STRNCMP(line, "default", name_end - line) == 0)
{
dodefault = TRUE;
line = linep;
name_end = skiptowhite(line);
linep = skipwhite(name_end);
}




if (STRNCMP(line, "clear", name_end - line) == 0)
doclear = TRUE;
if (STRNCMP(line, "link", name_end - line) == 0)
dolink = TRUE;




if (!doclear && !dolink && ends_excmd(*linep))
{
id = syn_namen2id(line, (int)(name_end - line));
if (id == 0)
semsg(_("E411: highlight group not found: %s"), line);
else
highlight_list_one(id);
return;
}




if (dolink)
{
char_u *from_start = linep;
char_u *from_end;
char_u *to_start;
char_u *to_end;
int from_id;
int to_id;

from_end = skiptowhite(from_start);
to_start = skipwhite(from_end);
to_end = skiptowhite(to_start);

if (ends_excmd(*from_start) || ends_excmd(*to_start))
{
semsg(_("E412: Not enough arguments: \":highlight link %s\""),
from_start);
return;
}

if (!ends_excmd(*skipwhite(to_end)))
{
semsg(_("E413: Too many arguments: \":highlight link %s\""), from_start);
return;
}

from_id = syn_check_group(from_start, (int)(from_end - from_start));
if (STRNCMP(to_start, "NONE", 4) == 0)
to_id = 0;
else
to_id = syn_check_group(to_start, (int)(to_end - to_start));

if (from_id > 0 && (!init || HL_TABLE()[from_id - 1].sg_set == 0))
{




if (to_id > 0 && !forceit && !init
&& hl_has_settings(from_id - 1, dodefault))
{
if (SOURCING_NAME == NULL && !dodefault)
emsg(_("E414: group has settings, highlight link ignored"));
}
else if (HL_TABLE()[from_id - 1].sg_link != to_id
#if defined(FEAT_EVAL)
|| HL_TABLE()[from_id - 1].sg_script_ctx.sc_sid
!= current_sctx.sc_sid
#endif
|| HL_TABLE()[from_id - 1].sg_cleared)
{
if (!init)
HL_TABLE()[from_id - 1].sg_set |= SG_LINK;
HL_TABLE()[from_id - 1].sg_link = to_id;
#if defined(FEAT_EVAL)
HL_TABLE()[from_id - 1].sg_script_ctx = current_sctx;
HL_TABLE()[from_id - 1].sg_script_ctx.sc_lnum += SOURCING_LNUM;
#endif
HL_TABLE()[from_id - 1].sg_cleared = FALSE;
redraw_all_later(SOME_VALID);


need_highlight_changed = TRUE;
}
}

return;
}

if (doclear)
{



line = linep;
if (ends_excmd(*line))
{
#if defined(FEAT_GUI)





if (gui.in_use)
{
#if defined(FEAT_BEVAL_TIP)
gui_init_tooltip_font();
#endif
#if defined(FEAT_MENU) && (defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_MOTIF))
gui_init_menu_font();
#endif
}
#if defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_X11)
gui_mch_def_colors();
#endif
#if defined(FEAT_GUI_X11)
#if defined(FEAT_MENU)



gui_mch_new_menu_colors();
#endif
if (gui.in_use)
{
gui_new_scrollbar_colors();
#if defined(FEAT_BEVAL_GUI)
gui_mch_new_tooltip_colors();
#endif
#if defined(FEAT_MENU)
gui_mch_new_menu_font();
#endif
}
#endif




#endif
#if defined(FEAT_EVAL)
do_unlet((char_u *)"colors_name", TRUE);
#endif
restore_cterm_colors();




for (idx = 0; idx < highlight_ga.ga_len; ++idx)
highlight_clear(idx);
init_highlight(TRUE, TRUE);
#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)
if (USE_24BIT)
highlight_gui_started();
else
#endif
highlight_changed();
redraw_later_clear();
return;
}
name_end = skiptowhite(line);
linep = skipwhite(name_end);
}




id = syn_check_group(line, (int)(name_end - line));
if (id == 0) 
return;
idx = id - 1; 


if (dodefault && hl_has_settings(idx, TRUE))
return;


item_before = HL_TABLE()[idx];

if (STRCMP(HL_TABLE()[idx].sg_name_u, "NORMAL") == 0)
is_normal_group = TRUE;
#if defined(FEAT_TERMINAL)
else if (STRCMP(HL_TABLE()[idx].sg_name_u, "TERMINAL") == 0)
is_terminal_group = TRUE;
#endif
#if defined(FEAT_GUI_X11)
else if (STRCMP(HL_TABLE()[idx].sg_name_u, "MENU") == 0)
is_menu_group = TRUE;
else if (STRCMP(HL_TABLE()[idx].sg_name_u, "SCROLLBAR") == 0)
is_scrollbar_group = TRUE;
else if (STRCMP(HL_TABLE()[idx].sg_name_u, "TOOLTIP") == 0)
is_tooltip_group = TRUE;
#endif


if (doclear || (forceit && init))
{
highlight_clear(idx);
if (!doclear)
HL_TABLE()[idx].sg_set = 0;
}

if (!doclear)
while (!ends_excmd(*linep))
{
key_start = linep;
if (*linep == '=')
{
semsg(_("E415: unexpected equal sign: %s"), key_start);
error = TRUE;
break;
}





while (*linep && !VIM_ISWHITE(*linep) && *linep != '=')
++linep;
vim_free(key);
key = vim_strnsave_up(key_start, (int)(linep - key_start));
if (key == NULL)
{
error = TRUE;
break;
}
linep = skipwhite(linep);

if (STRCMP(key, "NONE") == 0)
{
if (!init || HL_TABLE()[idx].sg_set == 0)
{
if (!init)
HL_TABLE()[idx].sg_set |= SG_TERM+SG_CTERM+SG_GUI;
highlight_clear(idx);
}
continue;
}




if (*linep != '=')
{
semsg(_("E416: missing equal sign: %s"), key_start);
error = TRUE;
break;
}
++linep;




linep = skipwhite(linep);
if (*linep == '\'') 
{
arg_start = ++linep;
linep = vim_strchr(linep, '\'');
if (linep == NULL)
{
semsg(_(e_invarg2), key_start);
error = TRUE;
break;
}
}
else
{
arg_start = linep;
linep = skiptowhite(linep);
}
if (linep == arg_start)
{
semsg(_("E417: missing argument: %s"), key_start);
error = TRUE;
break;
}
vim_free(arg);
arg = vim_strnsave(arg_start, (int)(linep - arg_start));
if (arg == NULL)
{
error = TRUE;
break;
}
if (*linep == '\'')
++linep;




if ( STRCMP(key, "TERM") == 0
|| STRCMP(key, "CTERM") == 0
|| STRCMP(key, "GUI") == 0)
{
attr = 0;
off = 0;
while (arg[off] != NUL)
{
for (i = sizeof(hl_attr_table) / sizeof(int); --i >= 0; )
{
len = (int)STRLEN(hl_name_table[i]);
if (STRNICMP(arg + off, hl_name_table[i], len) == 0)
{
attr |= hl_attr_table[i];
off += len;
break;
}
}
if (i < 0)
{
semsg(_("E418: Illegal value: %s"), arg);
error = TRUE;
break;
}
if (arg[off] == ',') 
++off;
}
if (error)
break;
if (*key == 'T')
{
if (!init || !(HL_TABLE()[idx].sg_set & SG_TERM))
{
if (!init)
HL_TABLE()[idx].sg_set |= SG_TERM;
HL_TABLE()[idx].sg_term = attr;
}
}
else if (*key == 'C')
{
if (!init || !(HL_TABLE()[idx].sg_set & SG_CTERM))
{
if (!init)
HL_TABLE()[idx].sg_set |= SG_CTERM;
HL_TABLE()[idx].sg_cterm = attr;
HL_TABLE()[idx].sg_cterm_bold = FALSE;
}
}
#if defined(FEAT_GUI) || defined(FEAT_EVAL)
else
{
if (!init || !(HL_TABLE()[idx].sg_set & SG_GUI))
{
if (!init)
HL_TABLE()[idx].sg_set |= SG_GUI;
HL_TABLE()[idx].sg_gui = attr;
}
}
#endif
}
else if (STRCMP(key, "FONT") == 0)
{

#if defined(FEAT_GUI)
if (HL_TABLE()[idx].sg_font_name != NULL
&& STRCMP(HL_TABLE()[idx].sg_font_name, arg) == 0)
{

}
else if (!gui.shell_created)
{

vim_free(HL_TABLE()[idx].sg_font_name);
HL_TABLE()[idx].sg_font_name = vim_strsave(arg);
did_change = TRUE;
}
else
{
GuiFont temp_sg_font = HL_TABLE()[idx].sg_font;
#if defined(FEAT_XFONTSET)
GuiFontset temp_sg_fontset = HL_TABLE()[idx].sg_fontset;
#endif





HL_TABLE()[idx].sg_font = NOFONT;
#if defined(FEAT_XFONTSET)
HL_TABLE()[idx].sg_fontset = NOFONTSET;
#endif
hl_do_font(idx, arg, is_normal_group, is_menu_group,
is_tooltip_group, FALSE);

#if defined(FEAT_XFONTSET)
if (HL_TABLE()[idx].sg_fontset != NOFONTSET)
{


gui_mch_free_fontset(temp_sg_fontset);
vim_free(HL_TABLE()[idx].sg_font_name);
HL_TABLE()[idx].sg_font_name = vim_strsave(arg);
did_change = TRUE;
}
else
HL_TABLE()[idx].sg_fontset = temp_sg_fontset;
#endif
if (HL_TABLE()[idx].sg_font != NOFONT)
{


gui_mch_free_font(temp_sg_font);
vim_free(HL_TABLE()[idx].sg_font_name);
HL_TABLE()[idx].sg_font_name = vim_strsave(arg);
did_change = TRUE;
}
else
HL_TABLE()[idx].sg_font = temp_sg_font;
}
#endif
}
else if (STRCMP(key, "CTERMFG") == 0 || STRCMP(key, "CTERMBG") == 0)
{
if (!init || !(HL_TABLE()[idx].sg_set & SG_CTERM))
{
if (!init)
HL_TABLE()[idx].sg_set |= SG_CTERM;



if (key[5] == 'F' && HL_TABLE()[idx].sg_cterm_bold)
{
HL_TABLE()[idx].sg_cterm &= ~HL_BOLD;
HL_TABLE()[idx].sg_cterm_bold = FALSE;
}

if (VIM_ISDIGIT(*arg))
color = atoi((char *)arg);
else if (STRICMP(arg, "fg") == 0)
{
if (cterm_normal_fg_color)
color = cterm_normal_fg_color - 1;
else
{
emsg(_("E419: FG color unknown"));
error = TRUE;
break;
}
}
else if (STRICMP(arg, "bg") == 0)
{
if (cterm_normal_bg_color > 0)
color = cterm_normal_bg_color - 1;
else
{
emsg(_("E420: BG color unknown"));
error = TRUE;
break;
}
}
else
{
int bold = MAYBE;


off = TOUPPER_ASC(*arg);
for (i = (sizeof(color_names) / sizeof(char *)); --i >= 0; )
if (off == color_names[i][0]
&& STRICMP(arg + 1, color_names[i] + 1) == 0)
break;
if (i < 0)
{
semsg(_("E421: Color name or number not recognized: %s"), key_start);
error = TRUE;
break;
}

color = lookup_color(i, key[5] == 'F', &bold);



if (bold == TRUE)
{
HL_TABLE()[idx].sg_cterm |= HL_BOLD;
HL_TABLE()[idx].sg_cterm_bold = TRUE;
}
else if (bold == FALSE)
HL_TABLE()[idx].sg_cterm &= ~HL_BOLD;
}



if (key[5] == 'F')
{
HL_TABLE()[idx].sg_cterm_fg = color + 1;
if (is_normal_group)
{
cterm_normal_fg_color = color + 1;
cterm_normal_fg_bold = (HL_TABLE()[idx].sg_cterm & HL_BOLD);
#if defined(FEAT_GUI)

if (!gui.in_use && !gui.starting)
#endif
{
must_redraw = CLEAR;
if (termcap_active && color >= 0)
term_fg_color(color);
}
}
}
else
{
HL_TABLE()[idx].sg_cterm_bg = color + 1;
if (is_normal_group)
{
cterm_normal_bg_color = color + 1;
#if defined(FEAT_GUI)

if (!gui.in_use && !gui.starting)
#endif
{
must_redraw = CLEAR;
if (color >= 0)
{
int dark = -1;

if (termcap_active)
term_bg_color(color);
if (t_colors < 16)
dark = (color == 0 || color == 4);

else if (color < 16)
dark = (color < 7 || color == 8);


if (dark != -1
&& dark != (*p_bg == 'd')
&& !option_was_set((char_u *)"bg"))
{
set_option_value((char_u *)"bg", 0L,
(char_u *)(dark ? "dark" : "light"), 0);
reset_option_was_set((char_u *)"bg");
}
}
}
}
}
}
}
else if (STRCMP(key, "GUIFG") == 0)
{
#if defined(FEAT_GUI) || defined(FEAT_EVAL)
char_u **namep = &HL_TABLE()[idx].sg_gui_fg_name;

if (!init || !(HL_TABLE()[idx].sg_set & SG_GUI))
{
if (!init)
HL_TABLE()[idx].sg_set |= SG_GUI;

#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)

i = color_name2handle(arg);
if (i != INVALCOLOR || STRCMP(arg, "NONE") == 0 || !USE_24BIT)
{
HL_TABLE()[idx].sg_gui_fg = i;
#endif
if (*namep == NULL || STRCMP(*namep, arg) != 0)
{
vim_free(*namep);
if (STRCMP(arg, "NONE") != 0)
*namep = vim_strsave(arg);
else
*namep = NULL;
did_change = TRUE;
}
#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)
#if defined(FEAT_GUI_X11)
if (is_menu_group && gui.menu_fg_pixel != i)
{
gui.menu_fg_pixel = i;
do_colors = TRUE;
}
if (is_scrollbar_group && gui.scroll_fg_pixel != i)
{
gui.scroll_fg_pixel = i;
do_colors = TRUE;
}
#if defined(FEAT_BEVAL_GUI)
if (is_tooltip_group && gui.tooltip_fg_pixel != i)
{
gui.tooltip_fg_pixel = i;
do_colors = TRUE;
}
#endif
#endif
}
#endif
}
#endif
}
else if (STRCMP(key, "GUIBG") == 0)
{
#if defined(FEAT_GUI) || defined(FEAT_EVAL)
char_u **namep = &HL_TABLE()[idx].sg_gui_bg_name;

if (!init || !(HL_TABLE()[idx].sg_set & SG_GUI))
{
if (!init)
HL_TABLE()[idx].sg_set |= SG_GUI;

#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)

i = color_name2handle(arg);
if (i != INVALCOLOR || STRCMP(arg, "NONE") == 0 || !USE_24BIT)
{
HL_TABLE()[idx].sg_gui_bg = i;
#endif
if (*namep == NULL || STRCMP(*namep, arg) != 0)
{
vim_free(*namep);
if (STRCMP(arg, "NONE") != 0)
*namep = vim_strsave(arg);
else
*namep = NULL;
did_change = TRUE;
}
#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)
#if defined(FEAT_GUI_X11)
if (is_menu_group && gui.menu_bg_pixel != i)
{
gui.menu_bg_pixel = i;
do_colors = TRUE;
}
if (is_scrollbar_group && gui.scroll_bg_pixel != i)
{
gui.scroll_bg_pixel = i;
do_colors = TRUE;
}
#if defined(FEAT_BEVAL_GUI)
if (is_tooltip_group && gui.tooltip_bg_pixel != i)
{
gui.tooltip_bg_pixel = i;
do_colors = TRUE;
}
#endif
#endif
}
#endif
}
#endif
}
else if (STRCMP(key, "GUISP") == 0)
{
#if defined(FEAT_GUI) || defined(FEAT_EVAL)
char_u **namep = &HL_TABLE()[idx].sg_gui_sp_name;

if (!init || !(HL_TABLE()[idx].sg_set & SG_GUI))
{
if (!init)
HL_TABLE()[idx].sg_set |= SG_GUI;

#if defined(FEAT_GUI)
i = color_name2handle(arg);
if (i != INVALCOLOR || STRCMP(arg, "NONE") == 0 || !gui.in_use)
{
HL_TABLE()[idx].sg_gui_sp = i;
#endif
if (*namep == NULL || STRCMP(*namep, arg) != 0)
{
vim_free(*namep);
if (STRCMP(arg, "NONE") != 0)
*namep = vim_strsave(arg);
else
*namep = NULL;
did_change = TRUE;
}
#if defined(FEAT_GUI)
}
#endif
}
#endif
}
else if (STRCMP(key, "START") == 0 || STRCMP(key, "STOP") == 0)
{
char_u buf[100];
char_u *tname;

if (!init)
HL_TABLE()[idx].sg_set |= SG_TERM;





if (STRNCMP(arg, "t_", 2) == 0)
{
off = 0;
buf[0] = 0;
while (arg[off] != NUL)
{

for (len = 0; arg[off + len] &&
arg[off + len] != ','; ++len)
;
tname = vim_strnsave(arg + off, len);
if (tname == NULL) 
{
error = TRUE;
break;
}

p = get_term_code(tname);
vim_free(tname);
if (p == NULL) 
p = (char_u *)"";


if ((int)(STRLEN(buf) + STRLEN(p)) >= 99)
{
semsg(_("E422: terminal code too long: %s"), arg);
error = TRUE;
break;
}
STRCAT(buf, p);


off += len;
if (arg[off] == ',') 
++off;
}
}
else
{



for (p = arg, off = 0; off < 100 - 6 && *p; )
{
len = trans_special(&p, buf + off, FALSE, FALSE,
TRUE, NULL);
if (len > 0) 
off += len;
else 
buf[off++] = *p++;
}
buf[off] = NUL;
}
if (error)
break;

if (STRCMP(buf, "NONE") == 0) 
p = NULL;
else
p = vim_strsave(buf);
if (key[2] == 'A')
{
vim_free(HL_TABLE()[idx].sg_start);
HL_TABLE()[idx].sg_start = p;
}
else
{
vim_free(HL_TABLE()[idx].sg_stop);
HL_TABLE()[idx].sg_stop = p;
}
}
else
{
semsg(_("E423: Illegal argument: %s"), key_start);
error = TRUE;
break;
}
HL_TABLE()[idx].sg_cleared = FALSE;




if (!init || !(HL_TABLE()[idx].sg_set & SG_LINK))
HL_TABLE()[idx].sg_link = 0;




linep = skipwhite(linep);
}




if (error && idx == highlight_ga.ga_len)
syn_unadd_group();
else
{
if (is_normal_group)
{
HL_TABLE()[idx].sg_term_attr = 0;
HL_TABLE()[idx].sg_cterm_attr = 0;
#if defined(FEAT_GUI)
HL_TABLE()[idx].sg_gui_attr = 0;




#endif
#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)
if (USE_24BIT)
{
highlight_gui_started();
did_highlight_changed = TRUE;
redraw_all_later(NOT_VALID);
}
#endif
#if defined(FEAT_VTP)
control_console_color_rgb();
#endif
}
#if defined(FEAT_TERMINAL)
else if (is_terminal_group)
set_terminal_default_colors(
HL_TABLE()[idx].sg_cterm_fg, HL_TABLE()[idx].sg_cterm_bg);
#endif
#if defined(FEAT_GUI_X11)
#if defined(FEAT_MENU)
else if (is_menu_group)
{
if (gui.in_use && do_colors)
gui_mch_new_menu_colors();
}
#endif
else if (is_scrollbar_group)
{
if (gui.in_use && do_colors)
gui_new_scrollbar_colors();
else
set_hl_attr(idx);
}
#if defined(FEAT_BEVAL_GUI)
else if (is_tooltip_group)
{
if (gui.in_use && do_colors)
gui_mch_new_tooltip_colors();
}
#endif
#endif
else
set_hl_attr(idx);
#if defined(FEAT_EVAL)
HL_TABLE()[idx].sg_script_ctx = current_sctx;
HL_TABLE()[idx].sg_script_ctx.sc_lnum += SOURCING_LNUM;
#endif
}

vim_free(key);
vim_free(arg);



if ((did_change
|| memcmp(&HL_TABLE()[idx], &item_before, sizeof(item_before)) != 0)
#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)
&& !did_highlight_changed
#endif
)
{



if (!updating_screen)
redraw_all_later(NOT_VALID);
need_highlight_changed = TRUE;
}
}

#if defined(EXITFREE) || defined(PROTO)
void
free_highlight(void)
{
int i;

for (i = 0; i < highlight_ga.ga_len; ++i)
{
highlight_clear(i);
vim_free(HL_TABLE()[i].sg_name);
vim_free(HL_TABLE()[i].sg_name_u);
}
ga_clear(&highlight_ga);
}
#endif





void
restore_cterm_colors(void)
{
#if defined(MSWIN) && !defined(FEAT_GUI_MSWIN)



mch_set_normal_colors();
#else
#if defined(VIMDLL)
if (!gui.in_use)
{
mch_set_normal_colors();
return;
}
#endif
cterm_normal_fg_color = 0;
cterm_normal_fg_bold = 0;
cterm_normal_bg_color = 0;
#if defined(FEAT_TERMGUICOLORS)
cterm_normal_fg_gui_color = INVALCOLOR;
cterm_normal_bg_gui_color = INVALCOLOR;
#endif
#endif
}





static int
hl_has_settings(int idx, int check_link)
{
return ( HL_TABLE()[idx].sg_term_attr != 0
|| HL_TABLE()[idx].sg_cterm_attr != 0
|| HL_TABLE()[idx].sg_cterm_fg != 0
|| HL_TABLE()[idx].sg_cterm_bg != 0
#if defined(FEAT_GUI)
|| HL_TABLE()[idx].sg_gui_attr != 0
|| HL_TABLE()[idx].sg_gui_fg_name != NULL
|| HL_TABLE()[idx].sg_gui_bg_name != NULL
|| HL_TABLE()[idx].sg_gui_sp_name != NULL
|| HL_TABLE()[idx].sg_font_name != NULL
#endif
|| (check_link && (HL_TABLE()[idx].sg_set & SG_LINK)));
}




static void
highlight_clear(int idx)
{
HL_TABLE()[idx].sg_cleared = TRUE;

HL_TABLE()[idx].sg_term = 0;
VIM_CLEAR(HL_TABLE()[idx].sg_start);
VIM_CLEAR(HL_TABLE()[idx].sg_stop);
HL_TABLE()[idx].sg_term_attr = 0;
HL_TABLE()[idx].sg_cterm = 0;
HL_TABLE()[idx].sg_cterm_bold = FALSE;
HL_TABLE()[idx].sg_cterm_fg = 0;
HL_TABLE()[idx].sg_cterm_bg = 0;
HL_TABLE()[idx].sg_cterm_attr = 0;
#if defined(FEAT_GUI) || defined(FEAT_EVAL)
HL_TABLE()[idx].sg_gui = 0;
VIM_CLEAR(HL_TABLE()[idx].sg_gui_fg_name);
VIM_CLEAR(HL_TABLE()[idx].sg_gui_bg_name);
VIM_CLEAR(HL_TABLE()[idx].sg_gui_sp_name);
#endif
#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)
HL_TABLE()[idx].sg_gui_fg = INVALCOLOR;
HL_TABLE()[idx].sg_gui_bg = INVALCOLOR;
#endif
#if defined(FEAT_GUI)
HL_TABLE()[idx].sg_gui_sp = INVALCOLOR;
gui_mch_free_font(HL_TABLE()[idx].sg_font);
HL_TABLE()[idx].sg_font = NOFONT;
#if defined(FEAT_XFONTSET)
gui_mch_free_fontset(HL_TABLE()[idx].sg_fontset);
HL_TABLE()[idx].sg_fontset = NOFONTSET;
#endif
VIM_CLEAR(HL_TABLE()[idx].sg_font_name);
HL_TABLE()[idx].sg_gui_attr = 0;
#endif
#if defined(FEAT_EVAL)


if (HL_TABLE()[idx].sg_link == 0)
{
HL_TABLE()[idx].sg_script_ctx.sc_sid = 0;
HL_TABLE()[idx].sg_script_ctx.sc_lnum = 0;
}
#endif
}

#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS) || defined(PROTO)





void
set_normal_colors(void)
{
#if defined(FEAT_GUI)
#if defined(FEAT_TERMGUICOLORS)
if (gui.in_use)
#endif
{
if (set_group_colors((char_u *)"Normal",
&gui.norm_pixel, &gui.back_pixel,
FALSE, TRUE, FALSE))
{
gui_mch_new_colors();
must_redraw = CLEAR;
}
#if defined(FEAT_GUI_X11)
if (set_group_colors((char_u *)"Menu",
&gui.menu_fg_pixel, &gui.menu_bg_pixel,
TRUE, FALSE, FALSE))
{
#if defined(FEAT_MENU)
gui_mch_new_menu_colors();
#endif
must_redraw = CLEAR;
}
#if defined(FEAT_BEVAL_GUI)
if (set_group_colors((char_u *)"Tooltip",
&gui.tooltip_fg_pixel, &gui.tooltip_bg_pixel,
FALSE, FALSE, TRUE))
{
#if defined(FEAT_TOOLBAR)
gui_mch_new_tooltip_colors();
#endif
must_redraw = CLEAR;
}
#endif
if (set_group_colors((char_u *)"Scrollbar",
&gui.scroll_fg_pixel, &gui.scroll_bg_pixel,
FALSE, FALSE, FALSE))
{
gui_new_scrollbar_colors();
must_redraw = CLEAR;
}
#endif
}
#endif
#if defined(FEAT_TERMGUICOLORS)
#if defined(FEAT_GUI)
else
#endif
{
int idx;

idx = syn_name2id((char_u *)"Normal") - 1;
if (idx >= 0)
{
gui_do_one_color(idx, FALSE, FALSE);



if (cterm_normal_fg_gui_color != HL_TABLE()[idx].sg_gui_fg
|| cterm_normal_bg_gui_color != HL_TABLE()[idx].sg_gui_bg)
{


cterm_normal_fg_gui_color = HL_TABLE()[idx].sg_gui_fg;
cterm_normal_bg_gui_color = HL_TABLE()[idx].sg_gui_bg;
must_redraw = CLEAR;
}
}
}
#endif
}
#endif

#if defined(FEAT_GUI) || defined(PROTO)



static int
set_group_colors(
char_u *name,
guicolor_T *fgp,
guicolor_T *bgp,
int do_menu,
int use_norm,
int do_tooltip)
{
int idx;

idx = syn_name2id(name) - 1;
if (idx >= 0)
{
gui_do_one_color(idx, do_menu, do_tooltip);

if (HL_TABLE()[idx].sg_gui_fg != INVALCOLOR)
*fgp = HL_TABLE()[idx].sg_gui_fg;
else if (use_norm)
*fgp = gui.def_norm_pixel;
if (HL_TABLE()[idx].sg_gui_bg != INVALCOLOR)
*bgp = HL_TABLE()[idx].sg_gui_bg;
else if (use_norm)
*bgp = gui.def_back_pixel;
return TRUE;
}
return FALSE;
}





char_u *
hl_get_font_name(void)
{
int id;
char_u *s;

id = syn_name2id((char_u *)"Normal");
if (id > 0)
{
s = HL_TABLE()[id - 1].sg_font_name;
if (s != NULL)
return s;
}
return (char_u *)"";
}





void
hl_set_font_name(char_u *font_name)
{
int id;

id = syn_name2id((char_u *)"Normal");
if (id > 0)
{
vim_free(HL_TABLE()[id - 1].sg_font_name);
HL_TABLE()[id - 1].sg_font_name = vim_strsave(font_name);
}
}





void
hl_set_bg_color_name(
char_u *name) 
{
int id;

if (name != NULL)
{
id = syn_name2id((char_u *)"Normal");
if (id > 0)
{
vim_free(HL_TABLE()[id - 1].sg_gui_bg_name);
HL_TABLE()[id - 1].sg_gui_bg_name = name;
}
}
}





void
hl_set_fg_color_name(
char_u *name) 
{
int id;

if (name != NULL)
{
id = syn_name2id((char_u *)"Normal");
if (id > 0)
{
vim_free(HL_TABLE()[id - 1].sg_gui_fg_name);
HL_TABLE()[id - 1].sg_gui_fg_name = name;
}
}
}





static GuiFont
font_name2handle(char_u *name)
{
if (STRCMP(name, "NONE") == 0)
return NOFONT;

return gui_mch_get_font(name, TRUE);
}

#if defined(FEAT_XFONTSET)




static GuiFontset
fontset_name2handle(char_u *name, int fixed_width)
{
if (STRCMP(name, "NONE") == 0)
return NOFONTSET;

return gui_mch_get_fontset(name, TRUE, fixed_width);
}
#endif




static void
hl_do_font(
int idx,
char_u *arg,
int do_normal, 
int do_menu UNUSED, 
int do_tooltip UNUSED, 
int free_font) 
{
#if defined(FEAT_XFONTSET)


if (*p_guifontset != NUL
#if defined(FONTSET_ALWAYS)
|| do_menu
#endif
#if defined(FEAT_BEVAL_TIP)

|| do_tooltip
#endif
)
{
if (free_font)
gui_mch_free_fontset(HL_TABLE()[idx].sg_fontset);
HL_TABLE()[idx].sg_fontset = fontset_name2handle(arg, 0
#if defined(FONTSET_ALWAYS)
|| do_menu
#endif
#if defined(FEAT_BEVAL_TIP)
|| do_tooltip
#endif
);
}
if (HL_TABLE()[idx].sg_fontset != NOFONTSET)
{


if (do_normal)
gui_init_font(arg, TRUE);
#if (defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA)) && defined(FEAT_MENU)
if (do_menu)
{
#if defined(FONTSET_ALWAYS)
gui.menu_fontset = HL_TABLE()[idx].sg_fontset;
#else

gui.menu_font = HL_TABLE()[idx].sg_fontset;
#endif
gui_mch_new_menu_font();
}
#if defined(FEAT_BEVAL_GUI)
if (do_tooltip)
{





gui.tooltip_fontset = (XFontSet)HL_TABLE()[idx].sg_fontset;
gui_mch_new_tooltip_font();
}
#endif
#endif
}
else
#endif
{
if (free_font)
gui_mch_free_font(HL_TABLE()[idx].sg_font);
HL_TABLE()[idx].sg_font = font_name2handle(arg);


if (HL_TABLE()[idx].sg_font != NOFONT)
{
if (do_normal)
gui_init_font(arg, FALSE);
#if !defined(FONTSET_ALWAYS)
#if (defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA)) && defined(FEAT_MENU)
if (do_menu)
{
gui.menu_font = HL_TABLE()[idx].sg_font;
gui_mch_new_menu_font();
}
#endif
#endif
}
}
}

#endif 

#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS) || defined(PROTO)




guicolor_T
color_name2handle(char_u *name)
{
if (STRCMP(name, "NONE") == 0)
return INVALCOLOR;

if (STRICMP(name, "fg") == 0 || STRICMP(name, "foreground") == 0)
{
#if defined(FEAT_TERMGUICOLORS) && defined(FEAT_GUI)
if (gui.in_use)
#endif
#if defined(FEAT_GUI)
return gui.norm_pixel;
#endif
#if defined(FEAT_TERMGUICOLORS)
if (cterm_normal_fg_gui_color != INVALCOLOR)
return cterm_normal_fg_gui_color;

return GUI_GET_COLOR((char_u *)(*p_bg == 'l' ? "black" : "white"));
#endif
}
if (STRICMP(name, "bg") == 0 || STRICMP(name, "background") == 0)
{
#if defined(FEAT_TERMGUICOLORS) && defined(FEAT_GUI)
if (gui.in_use)
#endif
#if defined(FEAT_GUI)
return gui.back_pixel;
#endif
#if defined(FEAT_TERMGUICOLORS)
if (cterm_normal_bg_gui_color != INVALCOLOR)
return cterm_normal_bg_gui_color;

return GUI_GET_COLOR((char_u *)(*p_bg == 'l' ? "white" : "black"));
#endif
}

return GUI_GET_COLOR(name);
}
#endif






static garray_T term_attr_table = {0, 0, 0, 0, NULL};

#define TERM_ATTR_ENTRY(idx) ((attrentry_T *)term_attr_table.ga_data)[idx]

static garray_T cterm_attr_table = {0, 0, 0, 0, NULL};

#define CTERM_ATTR_ENTRY(idx) ((attrentry_T *)cterm_attr_table.ga_data)[idx]

#if defined(FEAT_GUI)
static garray_T gui_attr_table = {0, 0, 0, 0, NULL};

#define GUI_ATTR_ENTRY(idx) ((attrentry_T *)gui_attr_table.ga_data)[idx]
#endif







static int
get_attr_entry(garray_T *table, attrentry_T *aep)
{
int i;
attrentry_T *taep;
static int recursive = FALSE;




table->ga_itemsize = sizeof(attrentry_T);
table->ga_growsize = 7;




for (i = 0; i < table->ga_len; ++i)
{
taep = &(((attrentry_T *)table->ga_data)[i]);
if ( aep->ae_attr == taep->ae_attr
&& (
#if defined(FEAT_GUI)
(table == &gui_attr_table
&& (aep->ae_u.gui.fg_color == taep->ae_u.gui.fg_color
&& aep->ae_u.gui.bg_color
== taep->ae_u.gui.bg_color
&& aep->ae_u.gui.sp_color
== taep->ae_u.gui.sp_color
&& aep->ae_u.gui.font == taep->ae_u.gui.font
#if defined(FEAT_XFONTSET)
&& aep->ae_u.gui.fontset == taep->ae_u.gui.fontset
#endif
))
||
#endif
(table == &term_attr_table
&& (aep->ae_u.term.start == NULL)
== (taep->ae_u.term.start == NULL)
&& (aep->ae_u.term.start == NULL
|| STRCMP(aep->ae_u.term.start,
taep->ae_u.term.start) == 0)
&& (aep->ae_u.term.stop == NULL)
== (taep->ae_u.term.stop == NULL)
&& (aep->ae_u.term.stop == NULL
|| STRCMP(aep->ae_u.term.stop,
taep->ae_u.term.stop) == 0))
|| (table == &cterm_attr_table
&& aep->ae_u.cterm.fg_color
== taep->ae_u.cterm.fg_color
&& aep->ae_u.cterm.bg_color
== taep->ae_u.cterm.bg_color
#if defined(FEAT_TERMGUICOLORS)
&& aep->ae_u.cterm.fg_rgb
== taep->ae_u.cterm.fg_rgb
&& aep->ae_u.cterm.bg_rgb
== taep->ae_u.cterm.bg_rgb
#endif
)))

return i + ATTR_OFF;
}

if (table->ga_len + ATTR_OFF > MAX_TYPENR)
{





if (recursive)
{
emsg(_("E424: Too many different highlighting attributes in use"));
return 0;
}
recursive = TRUE;

clear_hl_tables();

must_redraw = CLEAR;

for (i = 0; i < highlight_ga.ga_len; ++i)
set_hl_attr(i);

recursive = FALSE;
}




if (ga_grow(table, 1) == FAIL)
return 0;

taep = &(((attrentry_T *)table->ga_data)[table->ga_len]);
vim_memset(taep, 0, sizeof(attrentry_T));
taep->ae_attr = aep->ae_attr;
#if defined(FEAT_GUI)
if (table == &gui_attr_table)
{
taep->ae_u.gui.fg_color = aep->ae_u.gui.fg_color;
taep->ae_u.gui.bg_color = aep->ae_u.gui.bg_color;
taep->ae_u.gui.sp_color = aep->ae_u.gui.sp_color;
taep->ae_u.gui.font = aep->ae_u.gui.font;
#if defined(FEAT_XFONTSET)
taep->ae_u.gui.fontset = aep->ae_u.gui.fontset;
#endif
}
#endif
if (table == &term_attr_table)
{
if (aep->ae_u.term.start == NULL)
taep->ae_u.term.start = NULL;
else
taep->ae_u.term.start = vim_strsave(aep->ae_u.term.start);
if (aep->ae_u.term.stop == NULL)
taep->ae_u.term.stop = NULL;
else
taep->ae_u.term.stop = vim_strsave(aep->ae_u.term.stop);
}
else if (table == &cterm_attr_table)
{
taep->ae_u.cterm.fg_color = aep->ae_u.cterm.fg_color;
taep->ae_u.cterm.bg_color = aep->ae_u.cterm.bg_color;
#if defined(FEAT_TERMGUICOLORS)
taep->ae_u.cterm.fg_rgb = aep->ae_u.cterm.fg_rgb;
taep->ae_u.cterm.bg_rgb = aep->ae_u.cterm.bg_rgb;
#endif
}
++table->ga_len;
return (table->ga_len - 1 + ATTR_OFF);
}

#if defined(FEAT_TERMINAL) || defined(PROTO)




int
get_cterm_attr_idx(int attr, int fg, int bg)
{
attrentry_T at_en;

vim_memset(&at_en, 0, sizeof(attrentry_T));
#if defined(FEAT_TERMGUICOLORS)
at_en.ae_u.cterm.fg_rgb = INVALCOLOR;
at_en.ae_u.cterm.bg_rgb = INVALCOLOR;
#endif
at_en.ae_attr = attr;
at_en.ae_u.cterm.fg_color = fg;
at_en.ae_u.cterm.bg_color = bg;
return get_attr_entry(&cterm_attr_table, &at_en);
}
#endif

#if (defined(FEAT_TERMINAL) && defined(FEAT_TERMGUICOLORS)) || defined(PROTO)




int
get_tgc_attr_idx(int attr, guicolor_T fg, guicolor_T bg)
{
attrentry_T at_en;

vim_memset(&at_en, 0, sizeof(attrentry_T));
at_en.ae_attr = attr;
if (fg == INVALCOLOR && bg == INVALCOLOR)
{


at_en.ae_u.cterm.fg_rgb = CTERMCOLOR;
at_en.ae_u.cterm.bg_rgb = CTERMCOLOR;
}
else
{
at_en.ae_u.cterm.fg_rgb = fg;
at_en.ae_u.cterm.bg_rgb = bg;
}
return get_attr_entry(&cterm_attr_table, &at_en);
}
#endif

#if (defined(FEAT_TERMINAL) && defined(FEAT_GUI)) || defined(PROTO)




int
get_gui_attr_idx(int attr, guicolor_T fg, guicolor_T bg)
{
attrentry_T at_en;

vim_memset(&at_en, 0, sizeof(attrentry_T));
at_en.ae_attr = attr;
at_en.ae_u.gui.fg_color = fg;
at_en.ae_u.gui.bg_color = bg;
return get_attr_entry(&gui_attr_table, &at_en);
}
#endif




void
clear_hl_tables(void)
{
int i;
attrentry_T *taep;

#if defined(FEAT_GUI)
ga_clear(&gui_attr_table);
#endif
for (i = 0; i < term_attr_table.ga_len; ++i)
{
taep = &(((attrentry_T *)term_attr_table.ga_data)[i]);
vim_free(taep->ae_u.term.start);
vim_free(taep->ae_u.term.stop);
}
ga_clear(&term_attr_table);
ga_clear(&cterm_attr_table);
}










int
hl_combine_attr(int char_attr, int prim_attr)
{
attrentry_T *char_aep = NULL;
attrentry_T *spell_aep;
attrentry_T new_en;

if (char_attr == 0)
return prim_attr;
if (char_attr <= HL_ALL && prim_attr <= HL_ALL)
return ATTR_COMBINE(char_attr, prim_attr);
#if defined(FEAT_GUI)
if (gui.in_use)
{
if (char_attr > HL_ALL)
char_aep = syn_gui_attr2entry(char_attr);
if (char_aep != NULL)
new_en = *char_aep;
else
{
vim_memset(&new_en, 0, sizeof(new_en));
new_en.ae_u.gui.fg_color = INVALCOLOR;
new_en.ae_u.gui.bg_color = INVALCOLOR;
new_en.ae_u.gui.sp_color = INVALCOLOR;
if (char_attr <= HL_ALL)
new_en.ae_attr = char_attr;
}

if (prim_attr <= HL_ALL)
new_en.ae_attr = ATTR_COMBINE(new_en.ae_attr, prim_attr);
else
{
spell_aep = syn_gui_attr2entry(prim_attr);
if (spell_aep != NULL)
{
new_en.ae_attr = ATTR_COMBINE(new_en.ae_attr,
spell_aep->ae_attr);
if (spell_aep->ae_u.gui.fg_color != INVALCOLOR)
new_en.ae_u.gui.fg_color = spell_aep->ae_u.gui.fg_color;
if (spell_aep->ae_u.gui.bg_color != INVALCOLOR)
new_en.ae_u.gui.bg_color = spell_aep->ae_u.gui.bg_color;
if (spell_aep->ae_u.gui.sp_color != INVALCOLOR)
new_en.ae_u.gui.sp_color = spell_aep->ae_u.gui.sp_color;
if (spell_aep->ae_u.gui.font != NOFONT)
new_en.ae_u.gui.font = spell_aep->ae_u.gui.font;
#if defined(FEAT_XFONTSET)
if (spell_aep->ae_u.gui.fontset != NOFONTSET)
new_en.ae_u.gui.fontset = spell_aep->ae_u.gui.fontset;
#endif
}
}
return get_attr_entry(&gui_attr_table, &new_en);
}
#endif

if (IS_CTERM)
{
if (char_attr > HL_ALL)
char_aep = syn_cterm_attr2entry(char_attr);
if (char_aep != NULL)
new_en = *char_aep;
else
{
vim_memset(&new_en, 0, sizeof(new_en));
#if defined(FEAT_TERMGUICOLORS)
new_en.ae_u.cterm.bg_rgb = INVALCOLOR;
new_en.ae_u.cterm.fg_rgb = INVALCOLOR;
#endif
if (char_attr <= HL_ALL)
new_en.ae_attr = char_attr;
}

if (prim_attr <= HL_ALL)
new_en.ae_attr = ATTR_COMBINE(new_en.ae_attr, prim_attr);
else
{
spell_aep = syn_cterm_attr2entry(prim_attr);
if (spell_aep != NULL)
{
new_en.ae_attr = ATTR_COMBINE(new_en.ae_attr,
spell_aep->ae_attr);
if (spell_aep->ae_u.cterm.fg_color > 0)
new_en.ae_u.cterm.fg_color = spell_aep->ae_u.cterm.fg_color;
if (spell_aep->ae_u.cterm.bg_color > 0)
new_en.ae_u.cterm.bg_color = spell_aep->ae_u.cterm.bg_color;
#if defined(FEAT_TERMGUICOLORS)


if (COLOR_INVALID(spell_aep->ae_u.cterm.fg_rgb)
&& COLOR_INVALID(spell_aep->ae_u.cterm.bg_rgb))
{
if (spell_aep->ae_u.cterm.fg_color > 0)
new_en.ae_u.cterm.fg_rgb = CTERMCOLOR;
if (spell_aep->ae_u.cterm.bg_color > 0)
new_en.ae_u.cterm.bg_rgb = CTERMCOLOR;
}
else
{
if (spell_aep->ae_u.cterm.fg_rgb != INVALCOLOR)
new_en.ae_u.cterm.fg_rgb = spell_aep->ae_u.cterm.fg_rgb;
if (spell_aep->ae_u.cterm.bg_rgb != INVALCOLOR)
new_en.ae_u.cterm.bg_rgb = spell_aep->ae_u.cterm.bg_rgb;
}
#endif
}
}
return get_attr_entry(&cterm_attr_table, &new_en);
}

if (char_attr > HL_ALL)
char_aep = syn_term_attr2entry(char_attr);
if (char_aep != NULL)
new_en = *char_aep;
else
{
vim_memset(&new_en, 0, sizeof(new_en));
if (char_attr <= HL_ALL)
new_en.ae_attr = char_attr;
}

if (prim_attr <= HL_ALL)
new_en.ae_attr = ATTR_COMBINE(new_en.ae_attr, prim_attr);
else
{
spell_aep = syn_term_attr2entry(prim_attr);
if (spell_aep != NULL)
{
new_en.ae_attr = ATTR_COMBINE(new_en.ae_attr, spell_aep->ae_attr);
if (spell_aep->ae_u.term.start != NULL)
{
new_en.ae_u.term.start = spell_aep->ae_u.term.start;
new_en.ae_u.term.stop = spell_aep->ae_u.term.stop;
}
}
}
return get_attr_entry(&term_attr_table, &new_en);
}

#if defined(FEAT_GUI)
attrentry_T *
syn_gui_attr2entry(int attr)
{
attr -= ATTR_OFF;
if (attr >= gui_attr_table.ga_len) 
return NULL;
return &(GUI_ATTR_ENTRY(attr));
}
#endif





int
syn_attr2attr(int attr)
{
attrentry_T *aep;

#if defined(FEAT_GUI)
if (gui.in_use)
aep = syn_gui_attr2entry(attr);
else
#endif
if (IS_CTERM)
aep = syn_cterm_attr2entry(attr);
else
aep = syn_term_attr2entry(attr);

if (aep == NULL) 
return 0;
return aep->ae_attr;
}


attrentry_T *
syn_term_attr2entry(int attr)
{
attr -= ATTR_OFF;
if (attr >= term_attr_table.ga_len) 
return NULL;
return &(TERM_ATTR_ENTRY(attr));
}

attrentry_T *
syn_cterm_attr2entry(int attr)
{
attr -= ATTR_OFF;
if (attr >= cterm_attr_table.ga_len) 
return NULL;
return &(CTERM_ATTR_ENTRY(attr));
}

#define LIST_ATTR 1
#define LIST_STRING 2
#define LIST_INT 3

static void
highlight_list_one(int id)
{
hl_group_T *sgp;
int didh = FALSE;

sgp = &HL_TABLE()[id - 1]; 

if (message_filtered(sgp->sg_name))
return;

didh = highlight_list_arg(id, didh, LIST_ATTR,
sgp->sg_term, NULL, "term");
didh = highlight_list_arg(id, didh, LIST_STRING,
0, sgp->sg_start, "start");
didh = highlight_list_arg(id, didh, LIST_STRING,
0, sgp->sg_stop, "stop");

didh = highlight_list_arg(id, didh, LIST_ATTR,
sgp->sg_cterm, NULL, "cterm");
didh = highlight_list_arg(id, didh, LIST_INT,
sgp->sg_cterm_fg, NULL, "ctermfg");
didh = highlight_list_arg(id, didh, LIST_INT,
sgp->sg_cterm_bg, NULL, "ctermbg");

#if defined(FEAT_GUI) || defined(FEAT_EVAL)
didh = highlight_list_arg(id, didh, LIST_ATTR,
sgp->sg_gui, NULL, "gui");
didh = highlight_list_arg(id, didh, LIST_STRING,
0, sgp->sg_gui_fg_name, "guifg");
didh = highlight_list_arg(id, didh, LIST_STRING,
0, sgp->sg_gui_bg_name, "guibg");
didh = highlight_list_arg(id, didh, LIST_STRING,
0, sgp->sg_gui_sp_name, "guisp");
#endif
#if defined(FEAT_GUI)
didh = highlight_list_arg(id, didh, LIST_STRING,
0, sgp->sg_font_name, "font");
#endif

if (sgp->sg_link && !got_int)
{
(void)syn_list_header(didh, 9999, id);
didh = TRUE;
msg_puts_attr("links to", HL_ATTR(HLF_D));
msg_putchar(' ');
msg_outtrans(HL_TABLE()[HL_TABLE()[id - 1].sg_link - 1].sg_name);
}

if (!didh)
highlight_list_arg(id, didh, LIST_STRING, 0, (char_u *)"cleared", "");
#if defined(FEAT_EVAL)
if (p_verbose > 0)
last_set_msg(sgp->sg_script_ctx);
#endif
}

static int
highlight_list_arg(
int id,
int didh,
int type,
int iarg,
char_u *sarg,
char *name)
{
char_u buf[100];
char_u *ts;
int i;

if (got_int)
return FALSE;
if (type == LIST_STRING ? (sarg != NULL) : (iarg != 0))
{
ts = buf;
if (type == LIST_INT)
sprintf((char *)buf, "%d", iarg - 1);
else if (type == LIST_STRING)
ts = sarg;
else 
{
buf[0] = NUL;
for (i = 0; hl_attr_table[i] != 0; ++i)
{
if (iarg & hl_attr_table[i])
{
if (buf[0] != NUL)
vim_strcat(buf, (char_u *)",", 100);
vim_strcat(buf, (char_u *)hl_name_table[i], 100);
iarg &= ~hl_attr_table[i]; 
}
}
}

(void)syn_list_header(didh,
(int)(vim_strsize(ts) + STRLEN(name) + 1), id);
didh = TRUE;
if (!got_int)
{
if (*name != NUL)
{
msg_puts_attr(name, HL_ATTR(HLF_D));
msg_puts_attr("=", HL_ATTR(HLF_D));
}
msg_outtrans(ts);
}
}
return didh;
}

#if (((defined(FEAT_EVAL) || defined(FEAT_PRINTER))) && defined(FEAT_SYN_HL)) || defined(PROTO)




char_u *
highlight_has_attr(
int id,
int flag,
int modec) 
{
int attr;

if (id <= 0 || id > highlight_ga.ga_len)
return NULL;

#if defined(FEAT_GUI) || defined(FEAT_EVAL)
if (modec == 'g')
attr = HL_TABLE()[id - 1].sg_gui;
else
#endif
if (modec == 'c')
attr = HL_TABLE()[id - 1].sg_cterm;
else
attr = HL_TABLE()[id - 1].sg_term;

if (attr & flag)
return (char_u *)"1";
return NULL;
}
#endif

#if (defined(FEAT_SYN_HL) && defined(FEAT_EVAL)) || defined(PROTO)



char_u *
highlight_color(
int id,
char_u *what, 
int modec) 
{
static char_u name[20];
int n;
int fg = FALSE;
int sp = FALSE;
int font = FALSE;

if (id <= 0 || id > highlight_ga.ga_len)
return NULL;

if (TOLOWER_ASC(what[0]) == 'f' && TOLOWER_ASC(what[1]) == 'g')
fg = TRUE;
else if (TOLOWER_ASC(what[0]) == 'f' && TOLOWER_ASC(what[1]) == 'o'
&& TOLOWER_ASC(what[2]) == 'n' && TOLOWER_ASC(what[3]) == 't')
font = TRUE;
else if (TOLOWER_ASC(what[0]) == 's' && TOLOWER_ASC(what[1]) == 'p')
sp = TRUE;
else if (!(TOLOWER_ASC(what[0]) == 'b' && TOLOWER_ASC(what[1]) == 'g'))
return NULL;
if (modec == 'g')
{
#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)
#if defined(FEAT_GUI)

if (font)
return HL_TABLE()[id - 1].sg_font_name;
#endif


if ((USE_24BIT) && what[2] == '#')
{
guicolor_T color;
long_u rgb;
static char_u buf[10];

if (fg)
color = HL_TABLE()[id - 1].sg_gui_fg;
else if (sp)
#if defined(FEAT_GUI)
color = HL_TABLE()[id - 1].sg_gui_sp;
#else
color = INVALCOLOR;
#endif
else
color = HL_TABLE()[id - 1].sg_gui_bg;
if (color == INVALCOLOR)
return NULL;
rgb = (long_u)GUI_MCH_GET_RGB(color);
sprintf((char *)buf, "#%02x%02x%02x",
(unsigned)(rgb >> 16),
(unsigned)(rgb >> 8) & 255,
(unsigned)rgb & 255);
return buf;
}
#endif
if (fg)
return (HL_TABLE()[id - 1].sg_gui_fg_name);
if (sp)
return (HL_TABLE()[id - 1].sg_gui_sp_name);
return (HL_TABLE()[id - 1].sg_gui_bg_name);
}
if (font || sp)
return NULL;
if (modec == 'c')
{
if (fg)
n = HL_TABLE()[id - 1].sg_cterm_fg - 1;
else
n = HL_TABLE()[id - 1].sg_cterm_bg - 1;
if (n < 0)
return NULL;
sprintf((char *)name, "%d", n);
return name;
}

return NULL;
}
#endif

#if (defined(FEAT_SYN_HL) && (defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)) && defined(FEAT_PRINTER)) || defined(PROTO)





long_u
highlight_gui_color_rgb(
int id,
int fg) 
{
guicolor_T color;

if (id <= 0 || id > highlight_ga.ga_len)
return 0L;

if (fg)
color = HL_TABLE()[id - 1].sg_gui_fg;
else
color = HL_TABLE()[id - 1].sg_gui_bg;

if (color == INVALCOLOR)
return 0L;

return GUI_MCH_GET_RGB(color);
}
#endif





int
syn_list_header(
int did_header, 
int outlen, 
int id) 
{
int endcol = 19;
int newline = TRUE;
int name_col = 0;

if (!did_header)
{
msg_putchar('\n');
if (got_int)
return TRUE;
msg_outtrans(HL_TABLE()[id - 1].sg_name);
name_col = msg_col;
endcol = 15;
}
else if (msg_col + outlen + 1 >= Columns)
{
msg_putchar('\n');
if (got_int)
return TRUE;
}
else
{
if (msg_col >= endcol) 
newline = FALSE;
}

if (msg_col >= endcol) 
endcol = msg_col + 1;
if (Columns <= endcol) 
endcol = Columns - 1;

msg_advance(endcol);


if (!did_header)
{
if (endcol == Columns - 1 && endcol <= name_col)
msg_putchar(' ');
msg_puts_attr("xxx", syn_id2attr(id));
msg_putchar(' ');
}

return newline;
}





static void
set_hl_attr(
int idx) 
{
attrentry_T at_en;
hl_group_T *sgp = HL_TABLE() + idx;


if (sgp->sg_name_u != NULL && STRCMP(sgp->sg_name_u, "NORMAL") == 0)
return;

#if defined(FEAT_GUI)




if (sgp->sg_gui_fg == INVALCOLOR
&& sgp->sg_gui_bg == INVALCOLOR
&& sgp->sg_gui_sp == INVALCOLOR
&& sgp->sg_font == NOFONT
#if defined(FEAT_XFONTSET)
&& sgp->sg_fontset == NOFONTSET
#endif
)
{
sgp->sg_gui_attr = sgp->sg_gui;
}
else
{
at_en.ae_attr = sgp->sg_gui;
at_en.ae_u.gui.fg_color = sgp->sg_gui_fg;
at_en.ae_u.gui.bg_color = sgp->sg_gui_bg;
at_en.ae_u.gui.sp_color = sgp->sg_gui_sp;
at_en.ae_u.gui.font = sgp->sg_font;
#if defined(FEAT_XFONTSET)
at_en.ae_u.gui.fontset = sgp->sg_fontset;
#endif
sgp->sg_gui_attr = get_attr_entry(&gui_attr_table, &at_en);
}
#endif




if (sgp->sg_start == NULL && sgp->sg_stop == NULL)
sgp->sg_term_attr = sgp->sg_term;
else
{
at_en.ae_attr = sgp->sg_term;
at_en.ae_u.term.start = sgp->sg_start;
at_en.ae_u.term.stop = sgp->sg_stop;
sgp->sg_term_attr = get_attr_entry(&term_attr_table, &at_en);
}





if (sgp->sg_cterm_fg == 0 && sgp->sg_cterm_bg == 0
#if defined(FEAT_TERMGUICOLORS)
&& sgp->sg_gui_fg == INVALCOLOR
&& sgp->sg_gui_bg == INVALCOLOR
#endif
)
sgp->sg_cterm_attr = sgp->sg_cterm;
else
{
at_en.ae_attr = sgp->sg_cterm;
at_en.ae_u.cterm.fg_color = sgp->sg_cterm_fg;
at_en.ae_u.cterm.bg_color = sgp->sg_cterm_bg;
#if defined(FEAT_TERMGUICOLORS)
#if defined(MSWIN)
#if defined(VIMDLL)

if (!gui.in_use && !gui.starting)
#endif
{
int id;
guicolor_T fg, bg;

id = syn_name2id((char_u *)"Normal");
if (id > 0)
{
syn_id2colors(id, &fg, &bg);
if (sgp->sg_gui_fg == INVALCOLOR)
sgp->sg_gui_fg = fg;
if (sgp->sg_gui_bg == INVALCOLOR)
sgp->sg_gui_bg = bg;
}

}
#endif
at_en.ae_u.cterm.fg_rgb = GUI_MCH_GET_RGB2(sgp->sg_gui_fg);
at_en.ae_u.cterm.bg_rgb = GUI_MCH_GET_RGB2(sgp->sg_gui_bg);
if (at_en.ae_u.cterm.fg_rgb == INVALCOLOR
&& at_en.ae_u.cterm.bg_rgb == INVALCOLOR)
{


at_en.ae_u.cterm.fg_rgb = CTERMCOLOR;
at_en.ae_u.cterm.bg_rgb = CTERMCOLOR;
}
#endif
sgp->sg_cterm_attr = get_attr_entry(&cterm_attr_table, &at_en);
}
}





int
syn_name2id(char_u *name)
{
int i;
char_u name_u[200];




vim_strncpy(name_u, name, 199);
vim_strup(name_u);
for (i = highlight_ga.ga_len; --i >= 0; )
if (HL_TABLE()[i].sg_name_u != NULL
&& STRCMP(name_u, HL_TABLE()[i].sg_name_u) == 0)
break;
return i + 1;
}





int
syn_name2attr(char_u *name)
{
int id = syn_name2id(name);

if (id != 0)
return syn_id2attr(id);
return 0;
}

#if defined(FEAT_EVAL) || defined(PROTO)



int
highlight_exists(char_u *name)
{
return (syn_name2id(name) > 0);
}

#if defined(FEAT_SEARCH_EXTRA) || defined(PROTO)




char_u *
syn_id2name(int id)
{
if (id <= 0 || id > highlight_ga.ga_len)
return (char_u *)"";
return HL_TABLE()[id - 1].sg_name;
}
#endif
#endif




int
syn_namen2id(char_u *linep, int len)
{
char_u *name;
int id = 0;

name = vim_strnsave(linep, len);
if (name != NULL)
{
id = syn_name2id(name);
vim_free(name);
}
return id;
}







int
syn_check_group(char_u *pp, int len)
{
int id;
char_u *name;

name = vim_strnsave(pp, len);
if (name == NULL)
return 0;

id = syn_name2id(name);
if (id == 0) 
id = syn_add_group(name);
else
vim_free(name);
return id;
}






static int
syn_add_group(char_u *name)
{
char_u *p;
char_u *name_up;


for (p = name; *p != NUL; ++p)
{
if (!vim_isprintc(*p))
{
emsg(_("E669: Unprintable character in group name"));
vim_free(name);
return 0;
}
else if (!ASCII_ISALNUM(*p) && *p != '_')
{


msg_source(HL_ATTR(HLF_W));
msg(_("W18: Invalid character in group name"));
break;
}
}




if (highlight_ga.ga_data == NULL)
{
highlight_ga.ga_itemsize = sizeof(hl_group_T);
highlight_ga.ga_growsize = 10;
}

if (highlight_ga.ga_len >= MAX_HL_ID)
{
emsg(_("E849: Too many highlight and syntax groups"));
vim_free(name);
return 0;
}




if (ga_grow(&highlight_ga, 1) == FAIL)
{
vim_free(name);
return 0;
}

name_up = vim_strsave_up(name);
if (name_up == NULL)
{
vim_free(name);
return 0;
}

vim_memset(&(HL_TABLE()[highlight_ga.ga_len]), 0, sizeof(hl_group_T));
HL_TABLE()[highlight_ga.ga_len].sg_name = name;
HL_TABLE()[highlight_ga.ga_len].sg_name_u = name_up;
#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)
HL_TABLE()[highlight_ga.ga_len].sg_gui_bg = INVALCOLOR;
HL_TABLE()[highlight_ga.ga_len].sg_gui_fg = INVALCOLOR;
#if defined(FEAT_GUI)
HL_TABLE()[highlight_ga.ga_len].sg_gui_sp = INVALCOLOR;
#endif
#endif
++highlight_ga.ga_len;

return highlight_ga.ga_len; 
}





static void
syn_unadd_group(void)
{
--highlight_ga.ga_len;
vim_free(HL_TABLE()[highlight_ga.ga_len].sg_name);
vim_free(HL_TABLE()[highlight_ga.ga_len].sg_name_u);
}




int
syn_id2attr(int hl_id)
{
int attr;
hl_group_T *sgp;

hl_id = syn_get_final_id(hl_id);
sgp = &HL_TABLE()[hl_id - 1]; 

#if defined(FEAT_GUI)



if (gui.in_use)
attr = sgp->sg_gui_attr;
else
#endif
if (IS_CTERM)
attr = sgp->sg_cterm_attr;
else
attr = sgp->sg_term_attr;

return attr;
}

#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS) || defined(PROTO)




int
syn_id2colors(int hl_id, guicolor_T *fgp, guicolor_T *bgp)
{
hl_group_T *sgp;

hl_id = syn_get_final_id(hl_id);
sgp = &HL_TABLE()[hl_id - 1]; 

*fgp = sgp->sg_gui_fg;
*bgp = sgp->sg_gui_bg;
return sgp->sg_gui;
}
#endif

#if (defined(MSWIN) && (!defined(FEAT_GUI_MSWIN) || defined(VIMDLL)) && defined(FEAT_TERMGUICOLORS)) || defined(FEAT_TERMINAL) || defined(PROTO)



void
syn_id2cterm_bg(int hl_id, int *fgp, int *bgp)
{
hl_group_T *sgp;

hl_id = syn_get_final_id(hl_id);
sgp = &HL_TABLE()[hl_id - 1]; 
*fgp = sgp->sg_cterm_fg - 1;
*bgp = sgp->sg_cterm_bg - 1;
}
#endif




int
syn_get_final_id(int hl_id)
{
int count;
hl_group_T *sgp;

if (hl_id > highlight_ga.ga_len || hl_id < 1)
return 0; 





for (count = 100; --count >= 0; )
{
sgp = &HL_TABLE()[hl_id - 1]; 
if (sgp->sg_link == 0 || sgp->sg_link > highlight_ga.ga_len)
break;
hl_id = sgp->sg_link;
}

return hl_id;
}

#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS) || defined(PROTO)





void
highlight_gui_started(void)
{
int idx;


if (USE_24BIT)
set_normal_colors();

for (idx = 0; idx < highlight_ga.ga_len; ++idx)
gui_do_one_color(idx, FALSE, FALSE);

highlight_changed();
}

static void
gui_do_one_color(
int idx,
int do_menu UNUSED, 
int do_tooltip UNUSED) 
{
int didit = FALSE;

#if defined(FEAT_GUI)
#if defined(FEAT_TERMGUICOLORS)
if (gui.in_use)
#endif
if (HL_TABLE()[idx].sg_font_name != NULL)
{
hl_do_font(idx, HL_TABLE()[idx].sg_font_name, FALSE, do_menu,
do_tooltip, TRUE);
didit = TRUE;
}
#endif
if (HL_TABLE()[idx].sg_gui_fg_name != NULL)
{
HL_TABLE()[idx].sg_gui_fg =
color_name2handle(HL_TABLE()[idx].sg_gui_fg_name);
didit = TRUE;
}
if (HL_TABLE()[idx].sg_gui_bg_name != NULL)
{
HL_TABLE()[idx].sg_gui_bg =
color_name2handle(HL_TABLE()[idx].sg_gui_bg_name);
didit = TRUE;
}
#if defined(FEAT_GUI)
if (HL_TABLE()[idx].sg_gui_sp_name != NULL)
{
HL_TABLE()[idx].sg_gui_sp =
color_name2handle(HL_TABLE()[idx].sg_gui_sp_name);
didit = TRUE;
}
#endif
if (didit) 
set_hl_attr(idx);
}
#endif

#if defined(USER_HIGHLIGHT) && defined(FEAT_STL_OPT)



static void
combine_stl_hlt(
int id,
int id_S,
int id_alt,
int hlcnt,
int i,
int hlf,
int *table)
{
hl_group_T *hlt = HL_TABLE();

if (id_alt == 0)
{
vim_memset(&hlt[hlcnt + i], 0, sizeof(hl_group_T));
hlt[hlcnt + i].sg_term = highlight_attr[hlf];
hlt[hlcnt + i].sg_cterm = highlight_attr[hlf];
#if defined(FEAT_GUI) || defined(FEAT_EVAL)
hlt[hlcnt + i].sg_gui = highlight_attr[hlf];
#endif
}
else
mch_memmove(&hlt[hlcnt + i],
&hlt[id_alt - 1],
sizeof(hl_group_T));
hlt[hlcnt + i].sg_link = 0;

hlt[hlcnt + i].sg_term ^=
hlt[id - 1].sg_term ^ hlt[id_S - 1].sg_term;
if (hlt[id - 1].sg_start != hlt[id_S - 1].sg_start)
hlt[hlcnt + i].sg_start = hlt[id - 1].sg_start;
if (hlt[id - 1].sg_stop != hlt[id_S - 1].sg_stop)
hlt[hlcnt + i].sg_stop = hlt[id - 1].sg_stop;
hlt[hlcnt + i].sg_cterm ^=
hlt[id - 1].sg_cterm ^ hlt[id_S - 1].sg_cterm;
if (hlt[id - 1].sg_cterm_fg != hlt[id_S - 1].sg_cterm_fg)
hlt[hlcnt + i].sg_cterm_fg = hlt[id - 1].sg_cterm_fg;
if (hlt[id - 1].sg_cterm_bg != hlt[id_S - 1].sg_cterm_bg)
hlt[hlcnt + i].sg_cterm_bg = hlt[id - 1].sg_cterm_bg;
#if defined(FEAT_GUI) || defined(FEAT_EVAL)
hlt[hlcnt + i].sg_gui ^=
hlt[id - 1].sg_gui ^ hlt[id_S - 1].sg_gui;
#endif
#if defined(FEAT_GUI)
if (hlt[id - 1].sg_gui_fg != hlt[id_S - 1].sg_gui_fg)
hlt[hlcnt + i].sg_gui_fg = hlt[id - 1].sg_gui_fg;
if (hlt[id - 1].sg_gui_bg != hlt[id_S - 1].sg_gui_bg)
hlt[hlcnt + i].sg_gui_bg = hlt[id - 1].sg_gui_bg;
if (hlt[id - 1].sg_gui_sp != hlt[id_S - 1].sg_gui_sp)
hlt[hlcnt + i].sg_gui_sp = hlt[id - 1].sg_gui_sp;
if (hlt[id - 1].sg_font != hlt[id_S - 1].sg_font)
hlt[hlcnt + i].sg_font = hlt[id - 1].sg_font;
#if defined(FEAT_XFONTSET)
if (hlt[id - 1].sg_fontset != hlt[id_S - 1].sg_fontset)
hlt[hlcnt + i].sg_fontset = hlt[id - 1].sg_fontset;
#endif
#endif
highlight_ga.ga_len = hlcnt + i + 1;
set_hl_attr(hlcnt + i); 
table[i] = syn_id2attr(hlcnt + i + 1);
}
#endif









int
highlight_changed(void)
{
int hlf;
int i;
char_u *p;
int attr;
char_u *end;
int id;
#if defined(USER_HIGHLIGHT)
char_u userhl[30]; 
#if defined(FEAT_STL_OPT)
int id_S = -1;
int id_SNC = 0;
#if defined(FEAT_TERMINAL)
int id_ST = 0;
int id_STNC = 0;
#endif
int hlcnt;
#endif
#endif
static int hl_flags[HLF_COUNT] = HL_FLAGS;

need_highlight_changed = FALSE;




for (hlf = 0; hlf < (int)HLF_COUNT; ++hlf)
highlight_attr[hlf] = 0;





for (i = 0; i < 2; ++i)
{
if (i)
p = p_hl;
else
p = get_highlight_default();
if (p == NULL) 
continue;

while (*p)
{
for (hlf = 0; hlf < (int)HLF_COUNT; ++hlf)
if (hl_flags[hlf] == *p)
break;
++p;
if (hlf == (int)HLF_COUNT || *p == NUL)
return FAIL;





attr = 0;
for ( ; *p && *p != ','; ++p) 
{
if (VIM_ISWHITE(*p)) 
continue;

if (attr > HL_ALL) 
return FAIL;

switch (*p)
{
case 'b': attr |= HL_BOLD;
break;
case 'i': attr |= HL_ITALIC;
break;
case '-':
case 'n': 
break;
case 'r': attr |= HL_INVERSE;
break;
case 's': attr |= HL_STANDOUT;
break;
case 'u': attr |= HL_UNDERLINE;
break;
case 'c': attr |= HL_UNDERCURL;
break;
case 't': attr |= HL_STRIKETHROUGH;
break;
case ':': ++p; 
if (attr || *p == NUL) 
return FAIL;
end = vim_strchr(p, ',');
if (end == NULL)
end = p + STRLEN(p);
id = syn_check_group(p, (int)(end - p));
if (id == 0)
return FAIL;
attr = syn_id2attr(id);
p = end - 1;
#if defined(FEAT_STL_OPT) && defined(USER_HIGHLIGHT)
if (hlf == (int)HLF_SNC)
id_SNC = syn_get_final_id(id);
#if defined(FEAT_TERMINAL)
else if (hlf == (int)HLF_ST)
id_ST = syn_get_final_id(id);
else if (hlf == (int)HLF_STNC)
id_STNC = syn_get_final_id(id);
#endif
else if (hlf == (int)HLF_S)
id_S = syn_get_final_id(id);
#endif
break;
default: return FAIL;
}
}
highlight_attr[hlf] = attr;

p = skip_to_option_part(p); 
}
}

#if defined(USER_HIGHLIGHT)











#if defined(FEAT_STL_OPT)
if (ga_grow(&highlight_ga, 28) == FAIL)
return FAIL;
hlcnt = highlight_ga.ga_len;
if (id_S == -1)
{


vim_memset(&HL_TABLE()[hlcnt + 27], 0, sizeof(hl_group_T));
HL_TABLE()[hlcnt + 18].sg_term = highlight_attr[HLF_S];
id_S = hlcnt + 19;
}
#endif
for (i = 0; i < 9; i++)
{
sprintf((char *)userhl, "User%d", i + 1);
id = syn_name2id(userhl);
if (id == 0)
{
highlight_user[i] = 0;
#if defined(FEAT_STL_OPT)
highlight_stlnc[i] = 0;
#if defined(FEAT_TERMINAL)
highlight_stlterm[i] = 0;
highlight_stltermnc[i] = 0;
#endif
#endif
}
else
{
highlight_user[i] = syn_id2attr(id);
#if defined(FEAT_STL_OPT)
combine_stl_hlt(id, id_S, id_SNC, hlcnt, i,
HLF_SNC, highlight_stlnc);
#if defined(FEAT_TERMINAL)
combine_stl_hlt(id, id_S, id_ST, hlcnt + 9, i,
HLF_ST, highlight_stlterm);
combine_stl_hlt(id, id_S, id_STNC, hlcnt + 18, i,
HLF_STNC, highlight_stltermnc);
#endif
#endif
}
}
#if defined(FEAT_STL_OPT)
highlight_ga.ga_len = hlcnt;
#endif

#endif 

return OK;
}

static void highlight_list(void);
static void highlight_list_two(int cnt, int attr);




void
set_context_in_highlight_cmd(expand_T *xp, char_u *arg)
{
char_u *p;


xp->xp_context = EXPAND_HIGHLIGHT;
xp->xp_pattern = arg;
include_link = 2;
include_default = 1;


if (*arg != NUL)
{
p = skiptowhite(arg);
if (*p != NUL) 
{
include_default = 0;
if (STRNCMP("default", arg, p - arg) == 0)
{
arg = skipwhite(p);
xp->xp_pattern = arg;
p = skiptowhite(arg);
}
if (*p != NUL) 
{
include_link = 0;
if (arg[1] == 'i' && arg[0] == 'N')
highlight_list();
if (STRNCMP("link", arg, p - arg) == 0
|| STRNCMP("clear", arg, p - arg) == 0)
{
xp->xp_pattern = skipwhite(p);
p = skiptowhite(xp->xp_pattern);
if (*p != NUL) 
{
xp->xp_pattern = skipwhite(p);
p = skiptowhite(xp->xp_pattern);
}
}
if (*p != NUL) 
xp->xp_context = EXPAND_NOTHING;
}
}
}
}




static void
highlight_list(void)
{
int i;

for (i = 10; --i >= 0; )
highlight_list_two(i, HL_ATTR(HLF_D));
for (i = 40; --i >= 0; )
highlight_list_two(99, 0);
}

static void
highlight_list_two(int cnt, int attr)
{
msg_puts_attr(&("N \bI \b! \b"[cnt / 11]), attr);
msg_clr_eos();
out_flush();
ui_delay(cnt == 99 ? 40L : (long)cnt * 50L, FALSE);
}




char_u *
get_highlight_name(expand_T *xp UNUSED, int idx)
{
return get_highlight_name_ext(xp, idx, TRUE);
}





char_u *
get_highlight_name_ext(expand_T *xp UNUSED, int idx, int skip_cleared)
{
if (idx < 0)
return NULL;



if (skip_cleared && idx < highlight_ga.ga_len && HL_TABLE()[idx].sg_cleared)
return (char_u *)"";

if (idx == highlight_ga.ga_len && include_none != 0)
return (char_u *)"none";
if (idx == highlight_ga.ga_len + include_none && include_default != 0)
return (char_u *)"default";
if (idx == highlight_ga.ga_len + include_none + include_default
&& include_link != 0)
return (char_u *)"link";
if (idx == highlight_ga.ga_len + include_none + include_default + 1
&& include_link != 0)
return (char_u *)"clear";
if (idx >= highlight_ga.ga_len)
return NULL;
return HL_TABLE()[idx].sg_name;
}

#if defined(FEAT_GUI) || defined(PROTO)




void
free_highlight_fonts(void)
{
int idx;

for (idx = 0; idx < highlight_ga.ga_len; ++idx)
{
gui_mch_free_font(HL_TABLE()[idx].sg_font);
HL_TABLE()[idx].sg_font = NOFONT;
#if defined(FEAT_XFONTSET)
gui_mch_free_fontset(HL_TABLE()[idx].sg_fontset);
HL_TABLE()[idx].sg_fontset = NOFONTSET;
#endif
}

gui_mch_free_font(gui.norm_font);
#if defined(FEAT_XFONTSET)
gui_mch_free_fontset(gui.fontset);
#endif
#if !defined(FEAT_GUI_GTK)
gui_mch_free_font(gui.bold_font);
gui_mch_free_font(gui.ital_font);
gui_mch_free_font(gui.boldital_font);
#endif
}
#endif


#if defined(FEAT_SEARCH_EXTRA) || defined(PROTO)

#define SEARCH_HL_PRIORITY 0








static int
match_add(
win_T *wp,
char_u *grp,
char_u *pat,
int prio,
int id,
list_T *pos_list,
char_u *conceal_char UNUSED) 
{
matchitem_T *cur;
matchitem_T *prev;
matchitem_T *m;
int hlg_id;
regprog_T *regprog = NULL;
int rtype = SOME_VALID;

if (*grp == NUL || (pat != NULL && *pat == NUL))
return -1;
if (id < -1 || id == 0)
{
semsg(_("E799: Invalid ID: %d (must be greater than or equal to 1)"),
id);
return -1;
}
if (id != -1)
{
cur = wp->w_match_head;
while (cur != NULL)
{
if (cur->id == id)
{
semsg(_("E801: ID already taken: %d"), id);
return -1;
}
cur = cur->next;
}
}
if ((hlg_id = syn_namen2id(grp, (int)STRLEN(grp))) == 0)
{
semsg(_(e_nogroup), grp);
return -1;
}
if (pat != NULL && (regprog = vim_regcomp(pat, RE_MAGIC)) == NULL)
{
semsg(_(e_invarg2), pat);
return -1;
}


while (id == -1)
{
cur = wp->w_match_head;
while (cur != NULL && cur->id != wp->w_next_match_id)
cur = cur->next;
if (cur == NULL)
id = wp->w_next_match_id;
wp->w_next_match_id++;
}


m = ALLOC_CLEAR_ONE(matchitem_T);
m->id = id;
m->priority = prio;
m->pattern = pat == NULL ? NULL : vim_strsave(pat);
m->hlg_id = hlg_id;
m->match.regprog = regprog;
m->match.rmm_ic = FALSE;
m->match.rmm_maxcol = 0;
#if defined(FEAT_CONCEAL)
m->conceal_char = 0;
if (conceal_char != NULL)
m->conceal_char = (*mb_ptr2char)(conceal_char);
#endif


if (pos_list != NULL)
{
linenr_T toplnum = 0;
linenr_T botlnum = 0;
listitem_T *li;
int i;

range_list_materialize(pos_list);
for (i = 0, li = pos_list->lv_first; li != NULL && i < MAXPOSMATCH;
i++, li = li->li_next)
{
linenr_T lnum = 0;
colnr_T col = 0;
int len = 1;
list_T *subl;
listitem_T *subli;
int error = FALSE;

if (li->li_tv.v_type == VAR_LIST)
{
subl = li->li_tv.vval.v_list;
if (subl == NULL)
goto fail;
subli = subl->lv_first;
if (subli == NULL)
goto fail;
lnum = tv_get_number_chk(&subli->li_tv, &error);
if (error == TRUE)
goto fail;
if (lnum == 0)
{
--i;
continue;
}
m->pos.pos[i].lnum = lnum;
subli = subli->li_next;
if (subli != NULL)
{
col = tv_get_number_chk(&subli->li_tv, &error);
if (error == TRUE)
goto fail;
subli = subli->li_next;
if (subli != NULL)
{
len = tv_get_number_chk(&subli->li_tv, &error);
if (error == TRUE)
goto fail;
}
}
m->pos.pos[i].col = col;
m->pos.pos[i].len = len;
}
else if (li->li_tv.v_type == VAR_NUMBER)
{
if (li->li_tv.vval.v_number == 0)
{
--i;
continue;
}
m->pos.pos[i].lnum = li->li_tv.vval.v_number;
m->pos.pos[i].col = 0;
m->pos.pos[i].len = 0;
}
else
{
emsg(_("E290: List or number required"));
goto fail;
}
if (toplnum == 0 || lnum < toplnum)
toplnum = lnum;
if (botlnum == 0 || lnum >= botlnum)
botlnum = lnum + 1;
}


if (toplnum != 0)
{
if (wp->w_buffer->b_mod_set)
{
if (wp->w_buffer->b_mod_top > toplnum)
wp->w_buffer->b_mod_top = toplnum;
if (wp->w_buffer->b_mod_bot < botlnum)
wp->w_buffer->b_mod_bot = botlnum;
}
else
{
wp->w_buffer->b_mod_set = TRUE;
wp->w_buffer->b_mod_top = toplnum;
wp->w_buffer->b_mod_bot = botlnum;
wp->w_buffer->b_mod_xlines = 0;
}
m->pos.toplnum = toplnum;
m->pos.botlnum = botlnum;
rtype = VALID;
}
}



cur = wp->w_match_head;
prev = cur;
while (cur != NULL && prio >= cur->priority)
{
prev = cur;
cur = cur->next;
}
if (cur == prev)
wp->w_match_head = m;
else
prev->next = m;
m->next = cur;

redraw_win_later(wp, rtype);
return id;

fail:
vim_free(m);
return -1;
}





static int
match_delete(win_T *wp, int id, int perr)
{
matchitem_T *cur = wp->w_match_head;
matchitem_T *prev = cur;
int rtype = SOME_VALID;

if (id < 1)
{
if (perr == TRUE)
semsg(_("E802: Invalid ID: %d (must be greater than or equal to 1)"),
id);
return -1;
}
while (cur != NULL && cur->id != id)
{
prev = cur;
cur = cur->next;
}
if (cur == NULL)
{
if (perr == TRUE)
semsg(_("E803: ID not found: %d"), id);
return -1;
}
if (cur == prev)
wp->w_match_head = cur->next;
else
prev->next = cur->next;
vim_regfree(cur->match.regprog);
vim_free(cur->pattern);
if (cur->pos.toplnum != 0)
{
if (wp->w_buffer->b_mod_set)
{
if (wp->w_buffer->b_mod_top > cur->pos.toplnum)
wp->w_buffer->b_mod_top = cur->pos.toplnum;
if (wp->w_buffer->b_mod_bot < cur->pos.botlnum)
wp->w_buffer->b_mod_bot = cur->pos.botlnum;
}
else
{
wp->w_buffer->b_mod_set = TRUE;
wp->w_buffer->b_mod_top = cur->pos.toplnum;
wp->w_buffer->b_mod_bot = cur->pos.botlnum;
wp->w_buffer->b_mod_xlines = 0;
}
rtype = VALID;
}
vim_free(cur);
redraw_win_later(wp, rtype);
return 0;
}




void
clear_matches(win_T *wp)
{
matchitem_T *m;

while (wp->w_match_head != NULL)
{
m = wp->w_match_head->next;
vim_regfree(wp->w_match_head->match.regprog);
vim_free(wp->w_match_head->pattern);
vim_free(wp->w_match_head);
wp->w_match_head = m;
}
redraw_win_later(wp, SOME_VALID);
}





static matchitem_T *
get_match(win_T *wp, int id)
{
matchitem_T *cur = wp->w_match_head;

while (cur != NULL && cur->id != id)
cur = cur->next;
return cur;
}




void
init_search_hl(win_T *wp, match_T *search_hl)
{
matchitem_T *cur;



cur = wp->w_match_head;
while (cur != NULL)
{
cur->hl.rm = cur->match;
if (cur->hlg_id == 0)
cur->hl.attr = 0;
else
cur->hl.attr = syn_id2attr(cur->hlg_id);
cur->hl.buf = wp->w_buffer;
cur->hl.lnum = 0;
cur->hl.first_lnum = 0;
#if defined(FEAT_RELTIME)

profile_setlimit(p_rdt, &(cur->hl.tm));
#endif
cur = cur->next;
}
search_hl->buf = wp->w_buffer;
search_hl->lnum = 0;
search_hl->first_lnum = 0;

}





static int
next_search_hl_pos(
match_T *shl, 
linenr_T lnum,
posmatch_T *posmatch, 
colnr_T mincol) 
{
int i;
int found = -1;

for (i = posmatch->cur; i < MAXPOSMATCH; i++)
{
llpos_T *pos = &posmatch->pos[i];

if (pos->lnum == 0)
break;
if (pos->len == 0 && pos->col < mincol)
continue;
if (pos->lnum == lnum)
{
if (found >= 0)
{


if (pos->col < posmatch->pos[found].col)
{
llpos_T tmp = *pos;

*pos = posmatch->pos[found];
posmatch->pos[found] = tmp;
}
}
else
found = i;
}
}
posmatch->cur = 0;
if (found >= 0)
{
colnr_T start = posmatch->pos[found].col == 0
? 0 : posmatch->pos[found].col - 1;
colnr_T end = posmatch->pos[found].col == 0
? MAXCOL : start + posmatch->pos[found].len;

shl->lnum = lnum;
shl->rm.startpos[0].lnum = 0;
shl->rm.startpos[0].col = start;
shl->rm.endpos[0].lnum = 0;
shl->rm.endpos[0].col = end;
shl->is_addpos = TRUE;
posmatch->cur = found + 1;
return 1;
}
return 0;
}









static void
next_search_hl(
win_T *win,
match_T *search_hl,
match_T *shl, 
linenr_T lnum,
colnr_T mincol, 
matchitem_T *cur) 
{
linenr_T l;
colnr_T matchcol;
long nmatched;
int called_emsg_before = called_emsg;


if (lnum < search_first_line || lnum > search_last_line)
{
shl->lnum = 0;
return;
}

if (shl->lnum != 0)
{




l = shl->lnum + shl->rm.endpos[0].lnum - shl->rm.startpos[0].lnum;
if (lnum > l)
shl->lnum = 0;
else if (lnum < l || shl->rm.endpos[0].col > mincol)
return;
}





for (;;)
{
#if defined(FEAT_RELTIME)

if (profile_passed_limit(&(shl->tm)))
{
shl->lnum = 0; 
break;
}
#endif





if (shl->lnum == 0)
matchcol = 0;
else if (vim_strchr(p_cpo, CPO_SEARCH) == NULL
|| (shl->rm.endpos[0].lnum == 0
&& shl->rm.endpos[0].col <= shl->rm.startpos[0].col))
{
char_u *ml;

matchcol = shl->rm.startpos[0].col;
ml = ml_get_buf(shl->buf, lnum, FALSE) + matchcol;
if (*ml == NUL)
{
++matchcol;
shl->lnum = 0;
break;
}
if (has_mbyte)
matchcol += mb_ptr2len(ml);
else
++matchcol;
}
else
matchcol = shl->rm.endpos[0].col;

shl->lnum = lnum;
if (shl->rm.regprog != NULL)
{


int regprog_is_copy = (shl != search_hl && cur != NULL
&& shl == &cur->hl
&& cur->match.regprog == cur->hl.rm.regprog);
int timed_out = FALSE;

nmatched = vim_regexec_multi(&shl->rm, win, shl->buf, lnum,
matchcol,
#if defined(FEAT_RELTIME)
&(shl->tm), &timed_out
#else
NULL, NULL
#endif
);

if (regprog_is_copy)
cur->match.regprog = cur->hl.rm.regprog;

if (called_emsg > called_emsg_before || got_int || timed_out)
{

if (shl == search_hl)
{

vim_regfree(shl->rm.regprog);
set_no_hlsearch(TRUE);
}
shl->rm.regprog = NULL;
shl->lnum = 0;
got_int = FALSE; 
break;
}
}
else if (cur != NULL)
nmatched = next_search_hl_pos(shl, lnum, &(cur->pos), matchcol);
else
nmatched = 0;
if (nmatched == 0)
{
shl->lnum = 0; 
break;
}
if (shl->rm.startpos[0].lnum > 0
|| shl->rm.startpos[0].col >= mincol
|| nmatched > 1
|| shl->rm.endpos[0].col > mincol)
{
shl->lnum += shl->rm.startpos[0].lnum;
break; 
}
}
}




void
prepare_search_hl(win_T *wp, match_T *search_hl, linenr_T lnum)
{
matchitem_T *cur; 
match_T *shl; 
int shl_flag; 

int pos_inprogress; 

int n;




cur = wp->w_match_head;
shl_flag = WIN_IS_POPUP(wp); 
while (cur != NULL || shl_flag == FALSE)
{
if (shl_flag == FALSE)
{
shl = search_hl;
shl_flag = TRUE;
}
else
shl = &cur->hl;
if (shl->rm.regprog != NULL
&& shl->lnum == 0
&& re_multiline(shl->rm.regprog))
{
if (shl->first_lnum == 0)
{
#if defined(FEAT_FOLDING)
for (shl->first_lnum = lnum;
shl->first_lnum > wp->w_topline; --shl->first_lnum)
if (hasFoldingWin(wp, shl->first_lnum - 1,
NULL, NULL, TRUE, NULL))
break;
#else
shl->first_lnum = wp->w_topline;
#endif
}
if (cur != NULL)
cur->pos.cur = 0;
pos_inprogress = TRUE;
n = 0;
while (shl->first_lnum < lnum && (shl->rm.regprog != NULL
|| (cur != NULL && pos_inprogress)))
{
next_search_hl(wp, search_hl, shl, shl->first_lnum, (colnr_T)n,
shl == search_hl ? NULL : cur);
pos_inprogress = cur == NULL || cur->pos.cur == 0
? FALSE : TRUE;
if (shl->lnum != 0)
{
shl->first_lnum = shl->lnum
+ shl->rm.endpos[0].lnum
- shl->rm.startpos[0].lnum;
n = shl->rm.endpos[0].col;
}
else
{
++shl->first_lnum;
n = 0;
}
}
}
if (shl != search_hl && cur != NULL)
cur = cur->next;
}
}






int
prepare_search_hl_line(
win_T *wp,
linenr_T lnum,
colnr_T mincol,
char_u **line,
match_T *search_hl,
int *search_attr)
{
matchitem_T *cur; 
match_T *shl; 
int shl_flag; 

int area_highlighting = FALSE;






cur = wp->w_match_head;
shl_flag = WIN_IS_POPUP(wp);
while (cur != NULL || shl_flag == FALSE)
{
if (shl_flag == FALSE)
{
shl = search_hl;
shl_flag = TRUE;
}
else
shl = &cur->hl;
shl->startcol = MAXCOL;
shl->endcol = MAXCOL;
shl->attr_cur = 0;
shl->is_addpos = FALSE;
if (cur != NULL)
cur->pos.cur = 0;
next_search_hl(wp, search_hl, shl, lnum, mincol,
shl == search_hl ? NULL : cur);



*line = ml_get_buf(wp->w_buffer, lnum, FALSE);

if (shl->lnum != 0 && shl->lnum <= lnum)
{
if (shl->lnum == lnum)
shl->startcol = shl->rm.startpos[0].col;
else
shl->startcol = 0;
if (lnum == shl->lnum + shl->rm.endpos[0].lnum
- shl->rm.startpos[0].lnum)
shl->endcol = shl->rm.endpos[0].col;
else
shl->endcol = MAXCOL;

if (shl->startcol == shl->endcol)
{
if (has_mbyte && (*line)[shl->endcol] != NUL)
shl->endcol += (*mb_ptr2len)((*line) + shl->endcol);
else
++shl->endcol;
}
if ((long)shl->startcol < mincol) 
{
shl->attr_cur = shl->attr;
*search_attr = shl->attr;
}
area_highlighting = TRUE;
}
if (shl != search_hl && cur != NULL)
cur = cur->next;
}
return area_highlighting;
}









int
update_search_hl(
win_T *wp,
linenr_T lnum,
colnr_T col,
char_u **line,
match_T *search_hl,
int *has_match_conc UNUSED,
int *match_conc UNUSED,
int did_line_attr,
int lcs_eol_one)
{
matchitem_T *cur; 
match_T *shl; 
int shl_flag; 

int pos_inprogress; 

int search_attr = 0;



cur = wp->w_match_head;
shl_flag = WIN_IS_POPUP(wp);
while (cur != NULL || shl_flag == FALSE)
{
if (shl_flag == FALSE
&& ((cur != NULL
&& cur->priority > SEARCH_HL_PRIORITY)
|| cur == NULL))
{
shl = search_hl;
shl_flag = TRUE;
}
else
shl = &cur->hl;
if (cur != NULL)
cur->pos.cur = 0;
pos_inprogress = TRUE;
while (shl->rm.regprog != NULL || (cur != NULL && pos_inprogress))
{
if (shl->startcol != MAXCOL
&& col >= shl->startcol
&& col < shl->endcol)
{
int next_col = col + mb_ptr2len(*line + col);

if (shl->endcol < next_col)
shl->endcol = next_col;
shl->attr_cur = shl->attr;
#if defined(FEAT_CONCEAL)


if (cur != NULL
&& shl != search_hl
&& syn_name2id((char_u *)"Conceal") == cur->hlg_id)
{
*has_match_conc = col == shl->startcol ? 2 : 1;
*match_conc = cur->conceal_char;
}
else
*has_match_conc = *match_conc = 0;
#endif
}
else if (col == shl->endcol)
{
shl->attr_cur = 0;
next_search_hl(wp, search_hl, shl, lnum, col,
shl == search_hl ? NULL : cur);
pos_inprogress = !(cur == NULL || cur->pos.cur == 0);



*line = ml_get_buf(wp->w_buffer, lnum, FALSE);

if (shl->lnum == lnum)
{
shl->startcol = shl->rm.startpos[0].col;
if (shl->rm.endpos[0].lnum == 0)
shl->endcol = shl->rm.endpos[0].col;
else
shl->endcol = MAXCOL;

if (shl->startcol == shl->endcol)
{


if (has_mbyte)
shl->endcol += (*mb_ptr2len)(*line + shl->endcol);
else
++shl->endcol;
}



continue;
}
}
break;
}
if (shl != search_hl && cur != NULL)
cur = cur->next;
}



cur = wp->w_match_head;
shl_flag = WIN_IS_POPUP(wp);
while (cur != NULL || shl_flag == FALSE)
{
if (shl_flag == FALSE
&& ((cur != NULL
&& cur->priority > SEARCH_HL_PRIORITY)
|| cur == NULL))
{
shl = search_hl;
shl_flag = TRUE;
}
else
shl = &cur->hl;
if (shl->attr_cur != 0)
search_attr = shl->attr_cur;
if (shl != search_hl && cur != NULL)
cur = cur->next;
}

if (*(*line + col) == NUL && (did_line_attr >= 1
|| (wp->w_p_list && lcs_eol_one == -1)))
search_attr = 0;
return search_attr;
}

int
get_prevcol_hl_flag(win_T *wp, match_T *search_hl, long curcol)
{
long prevcol = curcol;
int prevcol_hl_flag = FALSE;
matchitem_T *cur; 


if ((long)(wp->w_p_wrap ? wp->w_skipcol : wp->w_leftcol) > prevcol)
++prevcol;

if (!search_hl->is_addpos && prevcol == (long)search_hl->startcol)
prevcol_hl_flag = TRUE;
else
{
cur = wp->w_match_head;
while (cur != NULL)
{
if (!cur->hl.is_addpos && prevcol == (long)cur->hl.startcol)
{
prevcol_hl_flag = TRUE;
break;
}
cur = cur->next;
}
}
return prevcol_hl_flag;
}





void
get_search_match_hl(win_T *wp, match_T *search_hl, long col, int *char_attr)
{
matchitem_T *cur; 
match_T *shl; 
int shl_flag; 


cur = wp->w_match_head;
shl_flag = WIN_IS_POPUP(wp);
while (cur != NULL || shl_flag == FALSE)
{
if (shl_flag == FALSE
&& ((cur != NULL
&& cur->priority > SEARCH_HL_PRIORITY)
|| cur == NULL))
{
shl = search_hl;
shl_flag = TRUE;
}
else
shl = &cur->hl;
if (col - 1 == (long)shl->startcol
&& (shl == search_hl || !shl->is_addpos))
*char_attr = shl->attr;
if (shl != search_hl && cur != NULL)
cur = cur->next;
}
}

#endif 

#if defined(FEAT_EVAL) || defined(PROTO)
#if defined(FEAT_SEARCH_EXTRA)
static int
matchadd_dict_arg(typval_T *tv, char_u **conceal_char, win_T **win)
{
dictitem_T *di;

if (tv->v_type != VAR_DICT)
{
emsg(_(e_dictreq));
return FAIL;
}

if (dict_find(tv->vval.v_dict, (char_u *)"conceal", -1) != NULL)
*conceal_char = dict_get_string(tv->vval.v_dict,
(char_u *)"conceal", FALSE);

if ((di = dict_find(tv->vval.v_dict, (char_u *)"window", -1)) != NULL)
{
*win = find_win_by_nr_or_id(&di->di_tv);
if (*win == NULL)
{
emsg(_(e_invalwindow));
return FAIL;
}
}

return OK;
}
#endif




void
f_clearmatches(typval_T *argvars UNUSED, typval_T *rettv UNUSED)
{
#if defined(FEAT_SEARCH_EXTRA)
win_T *win = get_optional_window(argvars, 0);

if (win != NULL)
clear_matches(win);
#endif
}




void
f_getmatches(typval_T *argvars UNUSED, typval_T *rettv UNUSED)
{
#if defined(FEAT_SEARCH_EXTRA)
dict_T *dict;
matchitem_T *cur;
int i;
win_T *win = get_optional_window(argvars, 0);

if (rettv_list_alloc(rettv) == FAIL || win == NULL)
return;

cur = win->w_match_head;
while (cur != NULL)
{
dict = dict_alloc();
if (dict == NULL)
return;
if (cur->match.regprog == NULL)
{

for (i = 0; i < MAXPOSMATCH; ++i)
{
llpos_T *llpos;
char buf[30]; 
list_T *l;

llpos = &cur->pos.pos[i];
if (llpos->lnum == 0)
break;
l = list_alloc();
if (l == NULL)
break;
list_append_number(l, (varnumber_T)llpos->lnum);
if (llpos->col > 0)
{
list_append_number(l, (varnumber_T)llpos->col);
list_append_number(l, (varnumber_T)llpos->len);
}
sprintf(buf, "pos%d", i + 1);
dict_add_list(dict, buf, l);
}
}
else
{
dict_add_string(dict, "pattern", cur->pattern);
}
dict_add_string(dict, "group", syn_id2name(cur->hlg_id));
dict_add_number(dict, "priority", (long)cur->priority);
dict_add_number(dict, "id", (long)cur->id);
#if defined(FEAT_CONCEAL)
if (cur->conceal_char)
{
char_u buf[MB_MAXBYTES + 1];

buf[(*mb_char2bytes)((int)cur->conceal_char, buf)] = NUL;
dict_add_string(dict, "conceal", (char_u *)&buf);
}
#endif
list_append_dict(rettv->vval.v_list, dict);
cur = cur->next;
}
#endif
}




void
f_setmatches(typval_T *argvars UNUSED, typval_T *rettv UNUSED)
{
#if defined(FEAT_SEARCH_EXTRA)
list_T *l;
listitem_T *li;
dict_T *d;
list_T *s = NULL;
win_T *win = get_optional_window(argvars, 1);

rettv->vval.v_number = -1;
if (argvars[0].v_type != VAR_LIST)
{
emsg(_(e_listreq));
return;
}
if (win == NULL)
return;

if ((l = argvars[0].vval.v_list) != NULL)
{


li = l->lv_first;
while (li != NULL)
{
if (li->li_tv.v_type != VAR_DICT
|| (d = li->li_tv.vval.v_dict) == NULL)
{
emsg(_(e_invarg));
return;
}
if (!(dict_find(d, (char_u *)"group", -1) != NULL
&& (dict_find(d, (char_u *)"pattern", -1) != NULL
|| dict_find(d, (char_u *)"pos1", -1) != NULL)
&& dict_find(d, (char_u *)"priority", -1) != NULL
&& dict_find(d, (char_u *)"id", -1) != NULL))
{
emsg(_(e_invarg));
return;
}
li = li->li_next;
}

clear_matches(win);
li = l->lv_first;
while (li != NULL)
{
int i = 0;
char buf[30]; 
dictitem_T *di;
char_u *group;
int priority;
int id;
char_u *conceal;

d = li->li_tv.vval.v_dict;
if (dict_find(d, (char_u *)"pattern", -1) == NULL)
{
if (s == NULL)
{
s = list_alloc();
if (s == NULL)
return;
}


for (i = 1; i < 9; i++)
{
sprintf((char *)buf, (char *)"pos%d", i);
if ((di = dict_find(d, (char_u *)buf, -1)) != NULL)
{
if (di->di_tv.v_type != VAR_LIST)
return;

list_append_tv(s, &di->di_tv);
s->lv_refcount++;
}
else
break;
}
}

group = dict_get_string(d, (char_u *)"group", TRUE);
priority = (int)dict_get_number(d, (char_u *)"priority");
id = (int)dict_get_number(d, (char_u *)"id");
conceal = dict_find(d, (char_u *)"conceal", -1) != NULL
? dict_get_string(d, (char_u *)"conceal", TRUE)
: NULL;
if (i == 0)
{
match_add(win, group,
dict_get_string(d, (char_u *)"pattern", FALSE),
priority, id, NULL, conceal);
}
else
{
match_add(win, group, NULL, priority, id, s, conceal);
list_unref(s);
s = NULL;
}
vim_free(group);
vim_free(conceal);

li = li->li_next;
}
rettv->vval.v_number = 0;
}
#endif
}




void
f_matchadd(typval_T *argvars UNUSED, typval_T *rettv UNUSED)
{
#if defined(FEAT_SEARCH_EXTRA)
char_u buf[NUMBUFLEN];
char_u *grp = tv_get_string_buf_chk(&argvars[0], buf); 
char_u *pat = tv_get_string_buf_chk(&argvars[1], buf); 
int prio = 10; 
int id = -1;
int error = FALSE;
char_u *conceal_char = NULL;
win_T *win = curwin;

rettv->vval.v_number = -1;

if (grp == NULL || pat == NULL)
return;
if (argvars[2].v_type != VAR_UNKNOWN)
{
prio = (int)tv_get_number_chk(&argvars[2], &error);
if (argvars[3].v_type != VAR_UNKNOWN)
{
id = (int)tv_get_number_chk(&argvars[3], &error);
if (argvars[4].v_type != VAR_UNKNOWN
&& matchadd_dict_arg(&argvars[4], &conceal_char, &win) == FAIL)
return;
}
}
if (error == TRUE)
return;
if (id >= 1 && id <= 3)
{
semsg(_("E798: ID is reserved for \":match\": %d"), id);
return;
}

rettv->vval.v_number = match_add(win, grp, pat, prio, id, NULL,
conceal_char);
#endif
}




void
f_matchaddpos(typval_T *argvars UNUSED, typval_T *rettv UNUSED)
{
#if defined(FEAT_SEARCH_EXTRA)
char_u buf[NUMBUFLEN];
char_u *group;
int prio = 10;
int id = -1;
int error = FALSE;
list_T *l;
char_u *conceal_char = NULL;
win_T *win = curwin;

rettv->vval.v_number = -1;

group = tv_get_string_buf_chk(&argvars[0], buf);
if (group == NULL)
return;

if (argvars[1].v_type != VAR_LIST)
{
semsg(_(e_listarg), "matchaddpos()");
return;
}
l = argvars[1].vval.v_list;
if (l == NULL)
return;

if (argvars[2].v_type != VAR_UNKNOWN)
{
prio = (int)tv_get_number_chk(&argvars[2], &error);
if (argvars[3].v_type != VAR_UNKNOWN)
{
id = (int)tv_get_number_chk(&argvars[3], &error);

if (argvars[4].v_type != VAR_UNKNOWN
&& matchadd_dict_arg(&argvars[4], &conceal_char, &win) == FAIL)
return;
}
}
if (error == TRUE)
return;


if (id == 1 || id == 2)
{
semsg(_("E798: ID is reserved for \":match\": %d"), id);
return;
}

rettv->vval.v_number = match_add(win, group, NULL, prio, id, l,
conceal_char);
#endif
}




void
f_matcharg(typval_T *argvars UNUSED, typval_T *rettv)
{
if (rettv_list_alloc(rettv) == OK)
{
#if defined(FEAT_SEARCH_EXTRA)
int id = (int)tv_get_number(&argvars[0]);
matchitem_T *m;

if (id >= 1 && id <= 3)
{
if ((m = (matchitem_T *)get_match(curwin, id)) != NULL)
{
list_append_string(rettv->vval.v_list,
syn_id2name(m->hlg_id), -1);
list_append_string(rettv->vval.v_list, m->pattern, -1);
}
else
{
list_append_string(rettv->vval.v_list, NULL, -1);
list_append_string(rettv->vval.v_list, NULL, -1);
}
}
#endif
}
}




void
f_matchdelete(typval_T *argvars UNUSED, typval_T *rettv UNUSED)
{
#if defined(FEAT_SEARCH_EXTRA)
win_T *win = get_optional_window(argvars, 1);

if (win == NULL)
rettv->vval.v_number = -1;
else
rettv->vval.v_number = match_delete(win,
(int)tv_get_number(&argvars[0]), TRUE);
#endif
}
#endif

#if defined(FEAT_SEARCH_EXTRA) || defined(PROTO)





void
ex_match(exarg_T *eap)
{
char_u *p;
char_u *g = NULL;
char_u *end;
int c;
int id;

if (eap->line2 <= 3)
id = eap->line2;
else
{
emsg(_(e_invcmd));
return;
}


if (!eap->skip)
match_delete(curwin, id, FALSE);

if (ends_excmd(*eap->arg))
end = eap->arg;
else if ((STRNICMP(eap->arg, "none", 4) == 0
&& (VIM_ISWHITE(eap->arg[4]) || ends_excmd(eap->arg[4]))))
end = eap->arg + 4;
else
{
p = skiptowhite(eap->arg);
if (!eap->skip)
g = vim_strnsave(eap->arg, (int)(p - eap->arg));
p = skipwhite(p);
if (*p == NUL)
{

vim_free(g);
semsg(_(e_invarg2), eap->arg);
return;
}
end = skip_regexp(p + 1, *p, TRUE);
if (!eap->skip)
{
if (*end != NUL && !ends_excmd(*skipwhite(end + 1)))
{
vim_free(g);
eap->errmsg = e_trailing;
return;
}
if (*end != *p)
{
vim_free(g);
semsg(_(e_invarg2), p);
return;
}

c = *end;
*end = NUL;
match_add(curwin, g, p + 1, 10, id, NULL, NULL);
vim_free(g);
*end = c;
}
}
eap->nextcmd = find_nextcmd(end);
}
#endif
