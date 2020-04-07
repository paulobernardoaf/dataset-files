












#include "vim.h"
#include "version.h"

#if defined(FEAT_PRINTER) || defined(PROTO)



























































#if defined(FEAT_SYN_HL)
static const long_u cterm_color_8[8] =
{
(long_u)0x000000L, (long_u)0xff0000L, (long_u)0x00ff00L, (long_u)0xffff00L,
(long_u)0x0000ffL, (long_u)0xff00ffL, (long_u)0x00ffffL, (long_u)0xffffffL
};

static const long_u cterm_color_16[16] =
{
(long_u)0x000000L, (long_u)0x0000c0L, (long_u)0x008000L, (long_u)0x004080L,
(long_u)0xc00000L, (long_u)0xc000c0L, (long_u)0x808000L, (long_u)0xc0c0c0L,
(long_u)0x808080L, (long_u)0x6060ffL, (long_u)0x00ff00L, (long_u)0x00ffffL,
(long_u)0xff8080L, (long_u)0xff40ffL, (long_u)0xffff00L, (long_u)0xffffffL
};

static int current_syn_id;
#endif

#define PRCOLOR_BLACK (long_u)0
#define PRCOLOR_WHITE (long_u)0xFFFFFFL

static int curr_italic;
static int curr_bold;
static int curr_underline;
static long_u curr_bg;
static long_u curr_fg;
static int page_count;

#if defined(FEAT_POSTSCRIPT)
#define OPT_MBFONT_USECOURIER 0
#define OPT_MBFONT_ASCII 1
#define OPT_MBFONT_REGULAR 2
#define OPT_MBFONT_BOLD 3
#define OPT_MBFONT_OBLIQUE 4
#define OPT_MBFONT_BOLDOBLIQUE 5
#define OPT_MBFONT_NUM_OPTIONS 6

static option_table_T mbfont_opts[OPT_MBFONT_NUM_OPTIONS] =
{
{"c", FALSE, 0, NULL, 0, FALSE},
{"a", FALSE, 0, NULL, 0, FALSE},
{"r", FALSE, 0, NULL, 0, FALSE},
{"b", FALSE, 0, NULL, 0, FALSE},
{"i", FALSE, 0, NULL, 0, FALSE},
{"o", FALSE, 0, NULL, 0, FALSE},
};
#endif




typedef struct
{
int lead_spaces; 
int print_pos; 
colnr_T column; 
linenr_T file_line; 
long_u bytes_printed; 
int ff; 
} prt_pos_T;

static char *parse_list_options(char_u *option_str, option_table_T *table, int table_size);

static colnr_T hardcopy_line(prt_settings_T *psettings, int page_line, prt_pos_T *ppos);





char *
parse_printoptions(void)
{
return parse_list_options(p_popt, printer_opts, OPT_PRINT_NUM_OPTIONS);
}

#if defined(FEAT_POSTSCRIPT) || defined(PROTO)




char *
parse_printmbfont(void)
{
return parse_list_options(p_pmfn, mbfont_opts, OPT_MBFONT_NUM_OPTIONS);
}
#endif










static char *
parse_list_options(
char_u *option_str,
option_table_T *table,
int table_size)
{
option_table_T *old_opts;
char *ret = NULL;
char_u *stringp;
char_u *colonp;
char_u *commap;
char_u *p;
int idx = 0; 
int len;


old_opts = ALLOC_MULT(option_table_T, table_size);
if (old_opts == NULL)
return NULL;

for (idx = 0; idx < table_size; ++idx)
{
old_opts[idx] = table[idx];
table[idx].present = FALSE;
}




stringp = option_str;
while (*stringp)
{
colonp = vim_strchr(stringp, ':');
if (colonp == NULL)
{
ret = N_("E550: Missing colon");
break;
}
commap = vim_strchr(stringp, ',');
if (commap == NULL)
commap = option_str + STRLEN(option_str);

len = (int)(colonp - stringp);

for (idx = 0; idx < table_size; ++idx)
if (STRNICMP(stringp, table[idx].name, len) == 0)
break;

if (idx == table_size)
{
ret = N_("E551: Illegal component");
break;
}
p = colonp + 1;
table[idx].present = TRUE;

if (table[idx].hasnum)
{
if (!VIM_ISDIGIT(*p))
{
ret = N_("E552: digit expected");
break;
}

table[idx].number = getdigits(&p); 
}

table[idx].string = p;
table[idx].strlen = (int)(commap - p);

stringp = commap;
if (*stringp == ',')
++stringp;
}

if (ret != NULL)
{

for (idx = 0; idx < table_size; ++idx)
table[idx] = old_opts[idx];
}
vim_free(old_opts);
return ret;
}


#if defined(FEAT_SYN_HL)




static long_u
darken_rgb(long_u rgb)
{
return ((rgb >> 17) << 16)
+ (((rgb & 0xff00) >> 9) << 8)
+ ((rgb & 0xff) >> 1);
}

static long_u
prt_get_term_color(int colorindex)
{

if (t_colors > 8)
return cterm_color_16[colorindex % 16];
return cterm_color_8[colorindex % 8];
}

static void
prt_get_attr(
int hl_id,
prt_text_attr_T *pattr,
int modec)
{
int colorindex;
long_u fg_color;
long_u bg_color;
char *color;

pattr->bold = (highlight_has_attr(hl_id, HL_BOLD, modec) != NULL);
pattr->italic = (highlight_has_attr(hl_id, HL_ITALIC, modec) != NULL);
pattr->underline = (highlight_has_attr(hl_id, HL_UNDERLINE, modec) != NULL);
pattr->undercurl = (highlight_has_attr(hl_id, HL_UNDERCURL, modec) != NULL);

#if defined(FEAT_GUI) || defined(FEAT_TERMGUICOLORS)
if (USE_24BIT)
{
bg_color = highlight_gui_color_rgb(hl_id, FALSE);
if (bg_color == PRCOLOR_BLACK)
bg_color = PRCOLOR_WHITE;

fg_color = highlight_gui_color_rgb(hl_id, TRUE);
}
else
#endif
{
bg_color = PRCOLOR_WHITE;

color = (char *)highlight_color(hl_id, (char_u *)"fg", modec);
if (color == NULL)
colorindex = 0;
else
colorindex = atoi(color);

if (colorindex >= 0 && colorindex < t_colors)
fg_color = prt_get_term_color(colorindex);
else
fg_color = PRCOLOR_BLACK;
}

if (fg_color == PRCOLOR_WHITE)
fg_color = PRCOLOR_BLACK;
else if (*p_bg == 'd')
fg_color = darken_rgb(fg_color);

pattr->fg_color = fg_color;
pattr->bg_color = bg_color;
}
#endif 

static void
prt_set_fg(long_u fg)
{
if (fg != curr_fg)
{
curr_fg = fg;
mch_print_set_fg(fg);
}
}

static void
prt_set_bg(long_u bg)
{
if (bg != curr_bg)
{
curr_bg = bg;
mch_print_set_bg(bg);
}
}

static void
prt_set_font(int bold, int italic, int underline)
{
if (curr_bold != bold
|| curr_italic != italic
|| curr_underline != underline)
{
curr_underline = underline;
curr_italic = italic;
curr_bold = bold;
mch_print_set_font(bold, italic, underline);
}
}




static void
prt_line_number(
prt_settings_T *psettings,
int page_line,
linenr_T lnum)
{
int i;
char_u tbuf[20];

prt_set_fg(psettings->number.fg_color);
prt_set_bg(psettings->number.bg_color);
prt_set_font(psettings->number.bold, psettings->number.italic, psettings->number.underline);
mch_print_start_line(TRUE, page_line);



sprintf((char *)tbuf, "%6ld", (long)lnum);
for (i = 0; i < 6; i++)
(void)mch_print_text_out(&tbuf[i], 1);

#if defined(FEAT_SYN_HL)
if (psettings->do_syntax)

current_syn_id = -1;
else
#endif
{

prt_set_fg(PRCOLOR_BLACK);
prt_set_bg(PRCOLOR_WHITE);
prt_set_font(FALSE, FALSE, FALSE);
}
}




int
prt_header_height(void)
{
if (printer_opts[OPT_PRINT_HEADERHEIGHT].present)
return printer_opts[OPT_PRINT_HEADERHEIGHT].number;
return 2;
}




int
prt_use_number(void)
{
return (printer_opts[OPT_PRINT_NUMBER].present
&& TOLOWER_ASC(printer_opts[OPT_PRINT_NUMBER].string[0]) == 'y');
}





int
prt_get_unit(int idx)
{
int u = PRT_UNIT_NONE;
int i;
static char *(units[4]) = PRT_UNIT_NAMES;

if (printer_opts[idx].present)
for (i = 0; i < 4; ++i)
if (STRNICMP(printer_opts[idx].string, units[i], 2) == 0)
{
u = i;
break;
}
return u;
}




static void
prt_header(
prt_settings_T *psettings,
int pagenum,
linenr_T lnum UNUSED)
{
int width = psettings->chars_per_line;
int page_line;
char_u *tbuf;
char_u *p;
int l;


if (prt_use_number())
width += PRINT_NUMBER_WIDTH;

tbuf = alloc(width + IOSIZE);
if (tbuf == NULL)
return;

#if defined(FEAT_STL_OPT)
if (*p_header != NUL)
{
linenr_T tmp_lnum, tmp_topline, tmp_botline;
int use_sandbox = FALSE;







tmp_lnum = curwin->w_cursor.lnum;
tmp_topline = curwin->w_topline;
tmp_botline = curwin->w_botline;
curwin->w_cursor.lnum = lnum;
curwin->w_topline = lnum;
curwin->w_botline = lnum + 63;
printer_page_num = pagenum;

#if defined(FEAT_EVAL)
use_sandbox = was_set_insecurely((char_u *)"printheader", 0);
#endif
build_stl_str_hl(curwin, tbuf, (size_t)(width + IOSIZE),
p_header, use_sandbox,
' ', width, NULL, NULL);


curwin->w_cursor.lnum = tmp_lnum;
curwin->w_topline = tmp_topline;
curwin->w_botline = tmp_botline;
}
else
#endif
sprintf((char *)tbuf, _("Page %d"), pagenum);

prt_set_fg(PRCOLOR_BLACK);
prt_set_bg(PRCOLOR_WHITE);
prt_set_font(TRUE, FALSE, FALSE);


page_line = 0 - prt_header_height();
mch_print_start_line(TRUE, page_line);
for (p = tbuf; *p != NUL; )
{
if (mch_print_text_out(p, (l = (*mb_ptr2len)(p))))
{
++page_line;
if (page_line >= 0) 
break;
mch_print_start_line(TRUE, page_line);
}
p += l;
}

vim_free(tbuf);

#if defined(FEAT_SYN_HL)
if (psettings->do_syntax)

current_syn_id = -1;
else
#endif
{

prt_set_fg(PRCOLOR_BLACK);
prt_set_bg(PRCOLOR_WHITE);
prt_set_font(FALSE, FALSE, FALSE);
}
}




static void
prt_message(char_u *s)
{
screen_fill((int)Rows - 1, (int)Rows, 0, (int)Columns, ' ', ' ', 0);
screen_puts(s, (int)Rows - 1, 0, HL_ATTR(HLF_R));
out_flush();
}

void
ex_hardcopy(exarg_T *eap)
{
linenr_T lnum;
int collated_copies, uncollated_copies;
prt_settings_T settings;
long_u bytes_to_print = 0;
int page_line;
int jobsplit;

vim_memset(&settings, 0, sizeof(prt_settings_T));
settings.has_color = TRUE;

#if defined(FEAT_POSTSCRIPT)
if (*eap->arg == '>')
{
char *errormsg = NULL;


if (expand_filename(eap, eap->cmdlinep, &errormsg) == FAIL)
{
if (errormsg != NULL)
emsg(errormsg);
return;
}
settings.outfile = skipwhite(eap->arg + 1);
}
else if (*eap->arg != NUL)
settings.arguments = eap->arg;
#endif








if (mch_print_init(&settings,
curbuf->b_fname == NULL
? (char_u *)buf_spname(curbuf)
: curbuf->b_sfname == NULL
? curbuf->b_fname
: curbuf->b_sfname,
eap->forceit) == FAIL)
return;

#if defined(FEAT_SYN_HL)
#if defined(FEAT_GUI)
if (gui.in_use)
settings.modec = 'g';
else
#endif
if (t_colors > 1)
settings.modec = 'c';
else
settings.modec = 't';

if (!syntax_present(curwin))
settings.do_syntax = FALSE;
else if (printer_opts[OPT_PRINT_SYNTAX].present
&& TOLOWER_ASC(printer_opts[OPT_PRINT_SYNTAX].string[0]) != 'a')
settings.do_syntax =
(TOLOWER_ASC(printer_opts[OPT_PRINT_SYNTAX].string[0]) == 'y');
else
settings.do_syntax = settings.has_color;
#endif


settings.number.fg_color = PRCOLOR_BLACK;
settings.number.bg_color = PRCOLOR_WHITE;
settings.number.bold = FALSE;
settings.number.italic = TRUE;
settings.number.underline = FALSE;
#if defined(FEAT_SYN_HL)



if (prt_use_number() && settings.do_syntax)
{
int id;

id = syn_name2id((char_u *)"LineNr");
if (id > 0)
id = syn_get_final_id(id);

prt_get_attr(id, &settings.number, settings.modec);
}
#endif




for (lnum = eap->line1; lnum <= eap->line2; lnum++)
bytes_to_print += (long_u)STRLEN(skipwhite(ml_get(lnum)));
if (bytes_to_print == 0)
{
msg(_("No text to be printed"));
goto print_fail_no_begin;
}


curr_bg = (long_u)0xffffffffL;
curr_fg = (long_u)0xffffffffL;
curr_italic = MAYBE;
curr_bold = MAYBE;
curr_underline = MAYBE;

prt_set_fg(PRCOLOR_BLACK);
prt_set_bg(PRCOLOR_WHITE);
prt_set_font(FALSE, FALSE, FALSE);
#if defined(FEAT_SYN_HL)
current_syn_id = -1;
#endif

jobsplit = (printer_opts[OPT_PRINT_JOBSPLIT].present
&& TOLOWER_ASC(printer_opts[OPT_PRINT_JOBSPLIT].string[0]) == 'y');

if (!mch_print_begin(&settings))
goto print_fail_no_begin;




page_count = 0;
for (collated_copies = 0;
collated_copies < settings.n_collated_copies;
collated_copies++)
{
prt_pos_T prtpos; 
prt_pos_T page_prtpos; 
int side;

vim_memset(&page_prtpos, 0, sizeof(prt_pos_T));
page_prtpos.file_line = eap->line1;
prtpos = page_prtpos;

if (jobsplit && collated_copies > 0)
{

mch_print_end(&settings);
if (!mch_print_begin(&settings))
goto print_fail_no_begin;
}




for (page_count = 0; prtpos.file_line <= eap->line2; ++page_count)
{




for (uncollated_copies = 0;
uncollated_copies < settings.n_uncollated_copies;
uncollated_copies++)
{

prtpos = page_prtpos;




for (side = 0; side <= settings.duplex; ++side)
{





ui_breakcheck();
if (got_int || settings.user_abort)
goto print_fail;

sprintf((char *)IObuff, _("Printing page %d (%d%%)"),
page_count + 1 + side,
prtpos.bytes_printed > 1000000
? (int)(prtpos.bytes_printed /
(bytes_to_print / 100))
: (int)((prtpos.bytes_printed * 100)
/ bytes_to_print));
if (!mch_print_begin_page(IObuff))
goto print_fail;

if (settings.n_collated_copies > 1)
sprintf((char *)IObuff + STRLEN(IObuff),
_(" Copy %d of %d"),
collated_copies + 1,
settings.n_collated_copies);
prt_message(IObuff);




if (prt_header_height() > 0)
prt_header(&settings, page_count + 1 + side,
prtpos.file_line);

for (page_line = 0; page_line < settings.lines_per_page;
++page_line)
{
prtpos.column = hardcopy_line(&settings,
page_line, &prtpos);
if (prtpos.column == 0)
{

prtpos.bytes_printed +=
STRLEN(skipwhite(ml_get(prtpos.file_line)));
if (++prtpos.file_line > eap->line2)
break; 
}
else if (prtpos.ff)
{


break;
}
}

if (!mch_print_end_page())
goto print_fail;
if (prtpos.file_line > eap->line2)
break; 
}





if (prtpos.file_line > eap->line2 && settings.duplex
&& side == 0
&& uncollated_copies + 1 < settings.n_uncollated_copies)
{
if (!mch_print_blank_page())
goto print_fail;
}
}
if (settings.duplex && prtpos.file_line <= eap->line2)
++page_count;


page_prtpos = prtpos;
}

vim_snprintf((char *)IObuff, IOSIZE, _("Printed: %s"),
settings.jobname);
prt_message(IObuff);
}

print_fail:
if (got_int || settings.user_abort)
{
sprintf((char *)IObuff, "%s", _("Printing aborted"));
prt_message(IObuff);
}
mch_print_end(&settings);

print_fail_no_begin:
mch_print_cleanup();
}





static colnr_T
hardcopy_line(
prt_settings_T *psettings,
int page_line,
prt_pos_T *ppos)
{
colnr_T col;
char_u *line;
int need_break = FALSE;
int outputlen;
int tab_spaces;
long_u print_pos;
#if defined(FEAT_SYN_HL)
prt_text_attr_T attr;
int id;
#endif

if (ppos->column == 0 || ppos->ff)
{
print_pos = 0;
tab_spaces = 0;
if (!ppos->ff && prt_use_number())
prt_line_number(psettings, page_line, ppos->file_line);
ppos->ff = FALSE;
}
else
{

print_pos = ppos->print_pos;
tab_spaces = ppos->lead_spaces;
}

mch_print_start_line(0, page_line);
line = ml_get(ppos->file_line);




for (col = ppos->column; line[col] != NUL && !need_break; col += outputlen)
{
outputlen = 1;
if (has_mbyte && (outputlen = (*mb_ptr2len)(line + col)) < 1)
outputlen = 1;
#if defined(FEAT_SYN_HL)



if (psettings->do_syntax)
{
id = syn_get_id(curwin, ppos->file_line, col, 1, NULL, FALSE);
if (id > 0)
id = syn_get_final_id(id);
else
id = 0;

line = ml_get(ppos->file_line);

if (id != current_syn_id)
{
current_syn_id = id;
prt_get_attr(id, &attr, psettings->modec);
prt_set_font(attr.bold, attr.italic, attr.underline);
prt_set_fg(attr.fg_color);
prt_set_bg(attr.bg_color);
}
}
#endif




if (line[col] == TAB || tab_spaces != 0)
{
if (tab_spaces == 0)
#if defined(FEAT_VARTABS)
tab_spaces = tabstop_padding(print_pos, curbuf->b_p_ts,
curbuf->b_p_vts_array);
#else
tab_spaces = (int)(curbuf->b_p_ts - (print_pos % curbuf->b_p_ts));
#endif

while (tab_spaces > 0)
{
need_break = mch_print_text_out((char_u *)" ", 1);
print_pos++;
tab_spaces--;
if (need_break)
break;
}

if (need_break && tab_spaces > 0)
break;
}
else if (line[col] == FF
&& printer_opts[OPT_PRINT_FORMFEED].present
&& TOLOWER_ASC(printer_opts[OPT_PRINT_FORMFEED].string[0])
== 'y')
{
ppos->ff = TRUE;
need_break = 1;
}
else
{
need_break = mch_print_text_out(line + col, outputlen);
if (has_mbyte)
print_pos += (*mb_ptr2cells)(line + col);
else
print_pos++;
}
}

ppos->lead_spaces = tab_spaces;
ppos->print_pos = (int)print_pos;





if (!ppos->ff
&& (line[col] == NUL
|| (printer_opts[OPT_PRINT_WRAP].present
&& TOLOWER_ASC(printer_opts[OPT_PRINT_WRAP].string[0])
== 'n')))
return 0;
return col;
}

#if defined(FEAT_POSTSCRIPT) || defined(PROTO)






























#define NUM_ELEMENTS(arr) (sizeof(arr)/sizeof((arr)[0]))

#define PRT_PS_DEFAULT_DPI (72) 
#define PRT_PS_DEFAULT_FONTSIZE (10)
#define PRT_PS_DEFAULT_BUFFER_SIZE (80)

struct prt_mediasize_S
{
char *name;
float width; 
float height;
};

#define PRT_MEDIASIZE_LEN (sizeof(prt_mediasize) / sizeof(struct prt_mediasize_S))

static struct prt_mediasize_S prt_mediasize[] =
{
{"A4", 595.0, 842.0},
{"letter", 612.0, 792.0},
{"10x14", 720.0, 1008.0},
{"A3", 842.0, 1191.0},
{"A5", 420.0, 595.0},
{"B4", 729.0, 1032.0},
{"B5", 516.0, 729.0},
{"executive", 522.0, 756.0},
{"folio", 595.0, 935.0},
{"ledger", 1224.0, 792.0}, 
{"legal", 612.0, 1008.0},
{"quarto", 610.0, 780.0},
{"statement", 396.0, 612.0},
{"tabloid", 792.0, 1224.0}
};


struct prt_ps_font_S
{
int wx;
int uline_offset;
int uline_width;
int bbox_min_y;
int bbox_max_y;
char *(ps_fontname[4]);
};

#define PRT_PS_FONT_ROMAN (0)
#define PRT_PS_FONT_BOLD (1)
#define PRT_PS_FONT_OBLIQUE (2)
#define PRT_PS_FONT_BOLDOBLIQUE (3)


static struct prt_ps_font_S prt_ps_courier_font =
{
600,
-100, 50,
-250, 805,
{"Courier", "Courier-Bold", "Courier-Oblique", "Courier-BoldOblique"}
};


static struct prt_ps_font_S prt_ps_mb_font =
{
1000,
-100, 50,
-250, 805,
{NULL, NULL, NULL, NULL}
};


static struct prt_ps_font_S* prt_ps_font;



struct prt_ps_encoding_S
{
char *encoding;
char *cmap_encoding;
int needs_charset;
};

struct prt_ps_charset_S
{
char *charset;
char *cmap_charset;
int has_charset;
};


#define CS_JIS_C_1978 (0x01)
#define CS_JIS_X_1983 (0x02)
#define CS_JIS_X_1990 (0x04)
#define CS_NEC (0x08)
#define CS_MSWINDOWS (0x10)
#define CS_CP932 (0x20)
#define CS_KANJITALK6 (0x40)
#define CS_KANJITALK7 (0x80)


static struct prt_ps_encoding_S j_encodings[] =
{
{"iso-2022-jp", NULL, (CS_JIS_C_1978|CS_JIS_X_1983|CS_JIS_X_1990|
CS_NEC)},
{"euc-jp", "EUC", (CS_JIS_C_1978|CS_JIS_X_1983|CS_JIS_X_1990)},
{"sjis", "RKSJ", (CS_JIS_C_1978|CS_JIS_X_1983|CS_MSWINDOWS|
CS_KANJITALK6|CS_KANJITALK7)},
{"cp932", "RKSJ", CS_JIS_X_1983},
{"ucs-2", "UCS2", CS_JIS_X_1990},
{"utf-8", "UTF8" , CS_JIS_X_1990}
};
static struct prt_ps_charset_S j_charsets[] =
{
{"JIS_C_1978", "78", CS_JIS_C_1978},
{"JIS_X_1983", NULL, CS_JIS_X_1983},
{"JIS_X_1990", "Hojo", CS_JIS_X_1990},
{"NEC", "Ext", CS_NEC},
{"MSWINDOWS", "90ms", CS_MSWINDOWS},
{"CP932", "90ms", CS_JIS_X_1983},
{"KANJITALK6", "83pv", CS_KANJITALK6},
{"KANJITALK7", "90pv", CS_KANJITALK7}
};

#define CS_GB_2312_80 (0x01)
#define CS_GBT_12345_90 (0x02)
#define CS_GBK2K (0x04)
#define CS_SC_MAC (0x08)
#define CS_GBT_90_MAC (0x10)
#define CS_GBK (0x20)
#define CS_SC_ISO10646 (0x40)


static struct prt_ps_encoding_S sc_encodings[] =
{
{"iso-2022", NULL, (CS_GB_2312_80|CS_GBT_12345_90)},
{"gb18030", NULL, CS_GBK2K},
{"euc-cn", "EUC", (CS_GB_2312_80|CS_GBT_12345_90|CS_SC_MAC|
CS_GBT_90_MAC)},
{"gbk", "EUC", CS_GBK},
{"ucs-2", "UCS2", CS_SC_ISO10646},
{"utf-8", "UTF8", CS_SC_ISO10646}
};
static struct prt_ps_charset_S sc_charsets[] =
{
{"GB_2312-80", "GB", CS_GB_2312_80},
{"GBT_12345-90","GBT", CS_GBT_12345_90},
{"MAC", "GBpc", CS_SC_MAC},
{"GBT-90_MAC", "GBTpc", CS_GBT_90_MAC},
{"GBK", "GBK", CS_GBK},
{"GB18030", "GBK2K", CS_GBK2K},
{"ISO10646", "UniGB", CS_SC_ISO10646}
};

#define CS_CNS_PLANE_1 (0x01)
#define CS_CNS_PLANE_2 (0x02)
#define CS_CNS_PLANE_1_2 (0x04)
#define CS_B5 (0x08)
#define CS_ETEN (0x10)
#define CS_HK_GCCS (0x20)
#define CS_HK_SCS (0x40)
#define CS_HK_SCS_ETEN (0x80)
#define CS_MTHKL (0x100)
#define CS_MTHKS (0x200)
#define CS_DLHKL (0x400)
#define CS_DLHKS (0x800)
#define CS_TC_ISO10646 (0x1000)


static struct prt_ps_encoding_S tc_encodings[] =
{
{"iso-2022", NULL, (CS_CNS_PLANE_1|CS_CNS_PLANE_2)},
{"euc-tw", "EUC", CS_CNS_PLANE_1_2},
{"big5", "B5", (CS_B5|CS_ETEN|CS_HK_GCCS|CS_HK_SCS|
CS_HK_SCS_ETEN|CS_MTHKL|CS_MTHKS|CS_DLHKL|
CS_DLHKS)},
{"cp950", "B5", CS_B5},
{"ucs-2", "UCS2", CS_TC_ISO10646},
{"utf-8", "UTF8", CS_TC_ISO10646},
{"utf-16", "UTF16", CS_TC_ISO10646},
{"utf-32", "UTF32", CS_TC_ISO10646}
};
static struct prt_ps_charset_S tc_charsets[] =
{
{"CNS_1992_1", "CNS1", CS_CNS_PLANE_1},
{"CNS_1992_2", "CNS2", CS_CNS_PLANE_2},
{"CNS_1993", "CNS", CS_CNS_PLANE_1_2},
{"BIG5", NULL, CS_B5},
{"CP950", NULL, CS_B5},
{"ETEN", "ETen", CS_ETEN},
{"HK_GCCS", "HKgccs", CS_HK_GCCS},
{"SCS", "HKscs", CS_HK_SCS},
{"SCS_ETEN", "ETHK", CS_HK_SCS_ETEN},
{"MTHKL", "HKm471", CS_MTHKL},
{"MTHKS", "HKm314", CS_MTHKS},
{"DLHKL", "HKdla", CS_DLHKL},
{"DLHKS", "HKdlb", CS_DLHKS},
{"ISO10646", "UniCNS", CS_TC_ISO10646}
};

#define CS_KR_X_1992 (0x01)
#define CS_KR_MAC (0x02)
#define CS_KR_X_1992_MS (0x04)
#define CS_KR_ISO10646 (0x08)


static struct prt_ps_encoding_S k_encodings[] =
{
{"iso-2022-kr", NULL, CS_KR_X_1992},
{"euc-kr", "EUC", (CS_KR_X_1992|CS_KR_MAC)},
{"johab", "Johab", CS_KR_X_1992},
{"cp1361", "Johab", CS_KR_X_1992},
{"uhc", "UHC", CS_KR_X_1992_MS},
{"cp949", "UHC", CS_KR_X_1992_MS},
{"ucs-2", "UCS2", CS_KR_ISO10646},
{"utf-8", "UTF8", CS_KR_ISO10646}
};
static struct prt_ps_charset_S k_charsets[] =
{
{"KS_X_1992", "KSC", CS_KR_X_1992},
{"CP1361", "KSC", CS_KR_X_1992},
{"MAC", "KSCpc", CS_KR_MAC},
{"MSWINDOWS", "KSCms", CS_KR_X_1992_MS},
{"CP949", "KSCms", CS_KR_X_1992_MS},
{"WANSUNG", "KSCms", CS_KR_X_1992_MS},
{"ISO10646", "UniKS", CS_KR_ISO10646}
};


struct prt_ps_mbfont_S
{
int num_encodings;
struct prt_ps_encoding_S *encodings;
int num_charsets;
struct prt_ps_charset_S *charsets;
char *ascii_enc;
char *defcs;
};

static struct prt_ps_mbfont_S prt_ps_mbfonts[] =
{
{
NUM_ELEMENTS(j_encodings),
j_encodings,
NUM_ELEMENTS(j_charsets),
j_charsets,
"jis_roman",
"JIS_X_1983"
},
{
NUM_ELEMENTS(sc_encodings),
sc_encodings,
NUM_ELEMENTS(sc_charsets),
sc_charsets,
"gb_roman",
"GB_2312-80"
},
{
NUM_ELEMENTS(tc_encodings),
tc_encodings,
NUM_ELEMENTS(tc_charsets),
tc_charsets,
"cns_roman",
"BIG5"
},
{
NUM_ELEMENTS(k_encodings),
k_encodings,
NUM_ELEMENTS(k_charsets),
k_charsets,
"ks_roman",
"KS_X_1992"
}
};

struct prt_ps_resource_S
{
char_u name[64];
char_u filename[MAXPATHL + 1];
int type;
char_u title[256];
char_u version[256];
};


#define PRT_RESOURCE_TYPE_PROCSET (0)
#define PRT_RESOURCE_TYPE_ENCODING (1)
#define PRT_RESOURCE_TYPE_CMAP (2)










#define PRT_PROLOG_VERSION ((char_u *)"1.4")
#define PRT_CID_PROLOG_VERSION ((char_u *)"1.0")



static char *prt_resource_types[] =
{
"procset",
"encoding",
"cmap"
};


#define PRT_RESOURCE_HEADER "%!PS-Adobe-"
#define PRT_RESOURCE_RESOURCE "Resource-"
#define PRT_RESOURCE_PROCSET "ProcSet"
#define PRT_RESOURCE_ENCODING "Encoding"
#define PRT_RESOURCE_CMAP "CMap"




#define PRT_DSC_MISC_TYPE (-1)
#define PRT_DSC_TITLE_TYPE (1)
#define PRT_DSC_VERSION_TYPE (2)
#define PRT_DSC_ENDCOMMENTS_TYPE (3)

#define PRT_DSC_TITLE "%%Title:"
#define PRT_DSC_VERSION "%%Version:"
#define PRT_DSC_ENDCOMMENTS "%%EndComments:"

struct prt_dsc_comment_S
{
char *string;
int len;
int type;
};

struct prt_dsc_line_S
{
int type;
char_u *string;
int len;
};


#define SIZEOF_CSTR(s) (sizeof(s) - 1)
static struct prt_dsc_comment_S prt_dsc_table[] =
{
{PRT_DSC_TITLE, SIZEOF_CSTR(PRT_DSC_TITLE), PRT_DSC_TITLE_TYPE},
{PRT_DSC_VERSION, SIZEOF_CSTR(PRT_DSC_VERSION),
PRT_DSC_VERSION_TYPE},
{PRT_DSC_ENDCOMMENTS, SIZEOF_CSTR(PRT_DSC_ENDCOMMENTS),
PRT_DSC_ENDCOMMENTS_TYPE}
};

static void prt_write_file_len(char_u *buffer, int bytes);
static int prt_next_dsc(struct prt_dsc_line_S *p_dsc_line);




static FILE *prt_ps_fd;
static int prt_file_error;
static char_u *prt_ps_file_name = NULL;





static float prt_page_width;
static float prt_page_height;
static float prt_left_margin;
static float prt_right_margin;
static float prt_top_margin;
static float prt_bottom_margin;
static float prt_line_height;
static float prt_first_line_height;
static float prt_char_width;
static float prt_number_width;
static float prt_bgcol_offset;
static float prt_pos_x_moveto = 0.0;
static float prt_pos_y_moveto = 0.0;





static int prt_need_moveto;
static int prt_do_moveto;
static int prt_need_font;
static int prt_font;
static int prt_need_underline;
static int prt_underline;
static int prt_do_underline;
static int prt_need_fgcol;
static int prt_fgcol;
static int prt_need_bgcol;
static int prt_do_bgcol;
static int prt_bgcol;
static int prt_new_bgcol;
static int prt_attribute_change;
static float prt_text_run;
static int prt_page_num;
static int prt_bufsiz;




static int prt_media;
static int prt_portrait;
static int prt_num_copies;
static int prt_duplex;
static int prt_tumble;
static int prt_collate;




static char_u prt_line_buffer[257];
static garray_T prt_ps_buffer;

static int prt_do_conv;
static vimconv_T prt_conv;

static int prt_out_mbyte;
static int prt_custom_cmap;
static char prt_cmap[80];
static int prt_use_courier;
static int prt_in_ascii;
static int prt_half_width;
static char *prt_ascii_encoding;
static char_u prt_hexchar[] = "0123456789abcdef";

static void
prt_write_file_raw_len(char_u *buffer, int bytes)
{
if (!prt_file_error
&& fwrite(buffer, sizeof(char_u), bytes, prt_ps_fd)
!= (size_t)bytes)
{
emsg(_("E455: Error writing to PostScript output file"));
prt_file_error = TRUE;
}
}

static void
prt_write_file(char_u *buffer)
{
prt_write_file_len(buffer, (int)STRLEN(buffer));
}

static void
prt_write_file_len(char_u *buffer, int bytes)
{
#if defined(EBCDIC)
ebcdic2ascii(buffer, bytes);
#endif
prt_write_file_raw_len(buffer, bytes);
}




static void
prt_write_string(char *s)
{
vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer), "%s", s);
prt_write_file(prt_line_buffer);
}




static void
prt_write_int(int i)
{
sprintf((char *)prt_line_buffer, "%d ", i);
prt_write_file(prt_line_buffer);
}




static void
prt_write_boolean(int b)
{
sprintf((char *)prt_line_buffer, "%s ", (b ? "T" : "F"));
prt_write_file(prt_line_buffer);
}




static void
prt_def_font(
char *new_name,
char *encoding,
int height,
char *font)
{
vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
"/_%s /VIM-%s /%s ref\n", new_name, encoding, font);
prt_write_file(prt_line_buffer);
if (prt_out_mbyte)
sprintf((char *)prt_line_buffer, "/%s %d %f /_%s sffs\n",
new_name, height, 500./prt_ps_courier_font.wx, new_name);
else
vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
"/%s %d /_%s ffs\n", new_name, height, new_name);
prt_write_file(prt_line_buffer);
}




static void
prt_def_cidfont(char *new_name, int height, char *cidfont)
{
vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
"/_%s /%s[/%s] vim_composefont\n", new_name, prt_cmap, cidfont);
prt_write_file(prt_line_buffer);
vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
"/%s %d /_%s ffs\n", new_name, height, new_name);
prt_write_file(prt_line_buffer);
}




static void
prt_dup_cidfont(char *original_name, char *new_name)
{
vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
"/%s %s d\n", new_name, original_name);
prt_write_file(prt_line_buffer);
}






static void
prt_real_bits(
double real,
int precision,
int *pinteger,
int *pfraction)
{
int i;
int integer;
float fraction;

integer = (int)real;
fraction = (float)(real - integer);
if (real < (double)integer)
fraction = -fraction;
for (i = 0; i < precision; i++)
fraction *= 10.0;

*pinteger = integer;
*pfraction = (int)(fraction + 0.5);
}






static void
prt_write_real(double val, int prec)
{
int integer;
int fraction;

prt_real_bits(val, prec, &integer, &fraction);

sprintf((char *)prt_line_buffer, "%d", integer);
prt_write_file(prt_line_buffer);

if (fraction != 0)
{

while ((fraction % 10) == 0)
{
prec--;
fraction /= 10;
}

sprintf((char *)prt_line_buffer, ".%0*d", prec, fraction);
prt_write_file(prt_line_buffer);
}
sprintf((char *)prt_line_buffer, " ");
prt_write_file(prt_line_buffer);
}




static void
prt_def_var(char *name, double value, int prec)
{
vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
"/%s ", name);
prt_write_file(prt_line_buffer);
prt_write_real(value, prec);
sprintf((char *)prt_line_buffer, "d\n");
prt_write_file(prt_line_buffer);
}


#define PRT_PS_FONT_TO_USER(scale, size) ((size) * ((scale)/1000.0))

static void
prt_flush_buffer(void)
{
if (prt_ps_buffer.ga_len > 0)
{

if (prt_do_bgcol && (prt_new_bgcol != PRCOLOR_WHITE))
{
int r, g, b;

if (prt_do_moveto)
{
prt_write_real(prt_pos_x_moveto, 2);
prt_write_real(prt_pos_y_moveto, 2);
prt_write_string("m\n");
prt_do_moveto = FALSE;
}


prt_write_real(prt_text_run, 2);
prt_write_real(prt_line_height, 2);


r = ((unsigned)prt_new_bgcol & 0xff0000) >> 16;
g = ((unsigned)prt_new_bgcol & 0xff00) >> 8;
b = prt_new_bgcol & 0xff;
prt_write_real(r / 255.0, 3);
prt_write_real(g / 255.0, 3);
prt_write_real(b / 255.0, 3);
prt_write_string("bg\n");
}


if (prt_do_underline)
{
if (prt_do_moveto)
{
prt_write_real(prt_pos_x_moveto, 2);
prt_write_real(prt_pos_y_moveto, 2);
prt_write_string("m\n");
prt_do_moveto = FALSE;
}


prt_write_real(prt_text_run, 2);
prt_write_string("ul\n");
}



if (prt_out_mbyte)
prt_write_string("<");
else
prt_write_string("(");
prt_write_file_raw_len(prt_ps_buffer.ga_data, prt_ps_buffer.ga_len);
if (prt_out_mbyte)
prt_write_string(">");
else
prt_write_string(")");

if (prt_do_moveto)
{
prt_write_real(prt_pos_x_moveto, 2);
prt_write_real(prt_pos_y_moveto, 2);

prt_write_string("ms\n");
prt_do_moveto = FALSE;
}
else 
prt_write_string("s\n");

ga_clear(&prt_ps_buffer);
ga_init2(&prt_ps_buffer, (int)sizeof(char), prt_bufsiz);
}
}


static void
prt_resource_name(char_u *filename, void *cookie)
{
char_u *resource_filename = cookie;

if (STRLEN(filename) >= MAXPATHL)
*resource_filename = NUL;
else
STRCPY(resource_filename, filename);
}

static int
prt_find_resource(char *name, struct prt_ps_resource_S *resource)
{
char_u *buffer;
int retval;

buffer = alloc(MAXPATHL + 1);
if (buffer == NULL)
return FALSE;

vim_strncpy(resource->name, (char_u *)name, 63);

STRCPY(buffer, "print");
add_pathsep(buffer);
vim_strcat(buffer, (char_u *)name, MAXPATHL);
vim_strcat(buffer, (char_u *)".ps", MAXPATHL);
resource->filename[0] = NUL;
retval = (do_in_runtimepath(buffer, 0, prt_resource_name,
resource->filename)
&& resource->filename[0] != NUL);
vim_free(buffer);
return retval;
}


#define PSLF (0x0a)
#define PSCR (0x0d)



#define PRT_FILE_BUFFER_LEN (2048)
struct prt_resfile_buffer_S
{
char_u buffer[PRT_FILE_BUFFER_LEN];
int len;
int line_start;
int line_end;
};

static struct prt_resfile_buffer_S prt_resfile;

static int
prt_resfile_next_line(void)
{
int idx;


idx = prt_resfile.line_end + 1;
while (idx < prt_resfile.len)
{
if (prt_resfile.buffer[idx] != PSLF && prt_resfile.buffer[idx] != PSCR)
break;
idx++;
}
prt_resfile.line_start = idx;

while (idx < prt_resfile.len)
{
if (prt_resfile.buffer[idx] == PSLF || prt_resfile.buffer[idx] == PSCR)
break;
idx++;
}
prt_resfile.line_end = idx;

return (idx < prt_resfile.len);
}

static int
prt_resfile_strncmp(int offset, char *string, int len)
{

if (len > (prt_resfile.line_end - (prt_resfile.line_start + offset)))
return 1;

return STRNCMP(&prt_resfile.buffer[prt_resfile.line_start + offset],
string, len);
}

static int
prt_resfile_skip_nonws(int offset)
{
int idx;

idx = prt_resfile.line_start + offset;
while (idx < prt_resfile.line_end)
{
if (isspace(prt_resfile.buffer[idx]))
return idx - prt_resfile.line_start;
idx++;
}
return -1;
}

static int
prt_resfile_skip_ws(int offset)
{
int idx;

idx = prt_resfile.line_start + offset;
while (idx < prt_resfile.line_end)
{
if (!isspace(prt_resfile.buffer[idx]))
return idx - prt_resfile.line_start;
idx++;
}
return -1;
}



static int
prt_next_dsc(struct prt_dsc_line_S *p_dsc_line)
{
int comment;
int offset;


if (!prt_resfile_next_line())
return FALSE;


if (prt_resfile_strncmp(0, "%%", 2) != 0)
return FALSE;


for (comment = 0; comment < (int)NUM_ELEMENTS(prt_dsc_table); comment++)
if (prt_resfile_strncmp(0, prt_dsc_table[comment].string,
prt_dsc_table[comment].len) == 0)
break;

if (comment != NUM_ELEMENTS(prt_dsc_table))
{

p_dsc_line->type = prt_dsc_table[comment].type;
offset = prt_dsc_table[comment].len;
}
else
{

p_dsc_line->type = PRT_DSC_MISC_TYPE;
offset = prt_resfile_skip_nonws(0);
if (offset == -1)
return FALSE;
}


offset = prt_resfile_skip_ws(offset);
if (offset == -1)
return FALSE;

p_dsc_line->string = &prt_resfile.buffer[prt_resfile.line_start + offset];
p_dsc_line->len = prt_resfile.line_end - (prt_resfile.line_start + offset);

return TRUE;
}





static int
prt_open_resource(struct prt_ps_resource_S *resource)
{
int offset;
int seen_all;
int seen_title;
int seen_version;
FILE *fd_resource;
struct prt_dsc_line_S dsc_line;

fd_resource = mch_fopen((char *)resource->filename, READBIN);
if (fd_resource == NULL)
{
semsg(_("E624: Can't open file \"%s\""), resource->filename);
return FALSE;
}
vim_memset(prt_resfile.buffer, NUL, PRT_FILE_BUFFER_LEN);


prt_resfile.len = (int)fread((char *)prt_resfile.buffer, sizeof(char_u),
PRT_FILE_BUFFER_LEN, fd_resource);
if (ferror(fd_resource))
{
semsg(_("E457: Can't read PostScript resource file \"%s\""),
resource->filename);
fclose(fd_resource);
return FALSE;
}
fclose(fd_resource);

prt_resfile.line_end = -1;
prt_resfile.line_start = 0;
if (!prt_resfile_next_line())
return FALSE;

offset = 0;

if (prt_resfile_strncmp(offset, PRT_RESOURCE_HEADER,
(int)STRLEN(PRT_RESOURCE_HEADER)) != 0)
{
semsg(_("E618: file \"%s\" is not a PostScript resource file"),
resource->filename);
return FALSE;
}


offset += (int)STRLEN(PRT_RESOURCE_HEADER);
offset = prt_resfile_skip_nonws(offset);
if (offset == -1)
return FALSE;
offset = prt_resfile_skip_ws(offset);
if (offset == -1)
return FALSE;

if (prt_resfile_strncmp(offset, PRT_RESOURCE_RESOURCE,
(int)STRLEN(PRT_RESOURCE_RESOURCE)) != 0)
{
semsg(_("E619: file \"%s\" is not a supported PostScript resource file"),
resource->filename);
return FALSE;
}
offset += (int)STRLEN(PRT_RESOURCE_RESOURCE);


if (prt_resfile_strncmp(offset, PRT_RESOURCE_PROCSET,
(int)STRLEN(PRT_RESOURCE_PROCSET)) == 0)
resource->type = PRT_RESOURCE_TYPE_PROCSET;
else if (prt_resfile_strncmp(offset, PRT_RESOURCE_ENCODING,
(int)STRLEN(PRT_RESOURCE_ENCODING)) == 0)
resource->type = PRT_RESOURCE_TYPE_ENCODING;
else if (prt_resfile_strncmp(offset, PRT_RESOURCE_CMAP,
(int)STRLEN(PRT_RESOURCE_CMAP)) == 0)
resource->type = PRT_RESOURCE_TYPE_CMAP;
else
{
semsg(_("E619: file \"%s\" is not a supported PostScript resource file"),
resource->filename);
return FALSE;
}


resource->title[0] = '\0';
resource->version[0] = '\0';
seen_title = FALSE;
seen_version = FALSE;
seen_all = FALSE;
while (!seen_all && prt_next_dsc(&dsc_line))
{
switch (dsc_line.type)
{
case PRT_DSC_TITLE_TYPE:
vim_strncpy(resource->title, dsc_line.string, dsc_line.len);
seen_title = TRUE;
if (seen_version)
seen_all = TRUE;
break;

case PRT_DSC_VERSION_TYPE:
vim_strncpy(resource->version, dsc_line.string, dsc_line.len);
seen_version = TRUE;
if (seen_title)
seen_all = TRUE;
break;

case PRT_DSC_ENDCOMMENTS_TYPE:

seen_all = TRUE;
break;

case PRT_DSC_MISC_TYPE:

break;
}
}

if (!seen_title || !seen_version)
{
semsg(_("E619: file \"%s\" is not a supported PostScript resource file"),
resource->filename);
return FALSE;
}

return TRUE;
}

static int
prt_check_resource(struct prt_ps_resource_S *resource, char_u *version)
{

if (STRNCMP(resource->version, version, STRLEN(version)))
{
semsg(_("E621: \"%s\" resource file has wrong version"),
resource->name);
return FALSE;
}


return TRUE;
}

static void
prt_dsc_start(void)
{
prt_write_string("%!PS-Adobe-3.0\n");
}

static void
prt_dsc_noarg(char *comment)
{
vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
"%%%%%s\n", comment);
prt_write_file(prt_line_buffer);
}

static void
prt_dsc_textline(char *comment, char *text)
{
vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
"%%%%%s: %s\n", comment, text);
prt_write_file(prt_line_buffer);
}

static void
prt_dsc_text(char *comment, char *text)
{

vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
"%%%%%s: (%s)\n", comment, text);
prt_write_file(prt_line_buffer);
}

#define prt_dsc_atend(c) prt_dsc_text((c), "atend")

static void
prt_dsc_ints(char *comment, int count, int *ints)
{
int i;

vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
"%%%%%s:", comment);
prt_write_file(prt_line_buffer);

for (i = 0; i < count; i++)
{
sprintf((char *)prt_line_buffer, " %d", ints[i]);
prt_write_file(prt_line_buffer);
}

prt_write_string("\n");
}

static void
prt_dsc_resources(
char *comment, 
char *type,
char *string)
{
if (comment != NULL)
vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
"%%%%%s: %s", comment, type);
else
vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
"%%%%+ %s", type);
prt_write_file(prt_line_buffer);

vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
" %s\n", string);
prt_write_file(prt_line_buffer);
}

static void
prt_dsc_font_resource(char *resource, struct prt_ps_font_S *ps_font)
{
int i;

prt_dsc_resources(resource, "font",
ps_font->ps_fontname[PRT_PS_FONT_ROMAN]);
for (i = PRT_PS_FONT_BOLD ; i <= PRT_PS_FONT_BOLDOBLIQUE ; i++)
if (ps_font->ps_fontname[i] != NULL)
prt_dsc_resources(NULL, "font", ps_font->ps_fontname[i]);
}

static void
prt_dsc_requirements(
int duplex,
int tumble,
int collate,
int color,
int num_copies)
{


if (!(duplex || collate || color || (num_copies > 1)))
return;

sprintf((char *)prt_line_buffer, "%%%%Requirements:");
prt_write_file(prt_line_buffer);

if (duplex)
{
prt_write_string(" duplex");
if (tumble)
prt_write_string("(tumble)");
}
if (collate)
prt_write_string(" collate");
if (color)
prt_write_string(" color");
if (num_copies > 1)
{
prt_write_string(" numcopies(");

sprintf((char *)prt_line_buffer, "%d", num_copies);
prt_write_file(prt_line_buffer);
prt_write_string(")");
}
prt_write_string("\n");
}

static void
prt_dsc_docmedia(
char *paper_name,
double width,
double height,
double weight,
char *colour,
char *type)
{
vim_snprintf((char *)prt_line_buffer, sizeof(prt_line_buffer),
"%%%%DocumentMedia: %s ", paper_name);
prt_write_file(prt_line_buffer);
prt_write_real(width, 2);
prt_write_real(height, 2);
prt_write_real(weight, 2);
if (colour == NULL)
prt_write_string("()");
else
prt_write_string(colour);
prt_write_string(" ");
if (type == NULL)
prt_write_string("()");
else
prt_write_string(type);
prt_write_string("\n");
}

void
mch_print_cleanup(void)
{
if (prt_out_mbyte)
{
int i;




for (i = PRT_PS_FONT_ROMAN; i <= PRT_PS_FONT_BOLDOBLIQUE; i++)
{
if (prt_ps_mb_font.ps_fontname[i] != NULL)
VIM_CLEAR(prt_ps_mb_font.ps_fontname[i]);
}
}

if (prt_do_conv)
{
convert_setup(&prt_conv, NULL, NULL);
prt_do_conv = FALSE;
}
if (prt_ps_fd != NULL)
{
fclose(prt_ps_fd);
prt_ps_fd = NULL;
prt_file_error = FALSE;
}
if (prt_ps_file_name != NULL)
VIM_CLEAR(prt_ps_file_name);
}

static float
to_device_units(int idx, double physsize, int def_number)
{
float ret;
int u;
int nr;

u = prt_get_unit(idx);
if (u == PRT_UNIT_NONE)
{
u = PRT_UNIT_PERC;
nr = def_number;
}
else
nr = printer_opts[idx].number;

switch (u)
{
case PRT_UNIT_INCH:
ret = (float)(nr * PRT_PS_DEFAULT_DPI);
break;
case PRT_UNIT_MM:
ret = (float)(nr * PRT_PS_DEFAULT_DPI) / (float)25.4;
break;
case PRT_UNIT_POINT:
ret = (float)nr;
break;
case PRT_UNIT_PERC:
default:
ret = (float)(physsize * nr) / 100;
break;
}

return ret;
}




static void
prt_page_margins(
double width,
double height,
double *left,
double *right,
double *top,
double *bottom)
{
*left = to_device_units(OPT_PRINT_LEFT, width, 10);
*right = width - to_device_units(OPT_PRINT_RIGHT, width, 5);
*top = height - to_device_units(OPT_PRINT_TOP, height, 5);
*bottom = to_device_units(OPT_PRINT_BOT, height, 5);
}

static void
prt_font_metrics(int font_scale)
{
prt_line_height = (float)font_scale;
prt_char_width = (float)PRT_PS_FONT_TO_USER(font_scale, prt_ps_font->wx);
}


static int
prt_get_cpl(void)
{
if (prt_use_number())
{
prt_number_width = PRINT_NUMBER_WIDTH * prt_char_width;


if (prt_out_mbyte)
prt_number_width /= 2;
prt_left_margin += prt_number_width;
}
else
prt_number_width = 0.0;

return (int)((prt_right_margin - prt_left_margin) / prt_char_width);
}

static int
prt_build_cid_fontname(int font, char_u *name, int name_len)
{
char *fontname;

fontname = alloc(name_len + 1);
if (fontname == NULL)
return FALSE;
vim_strncpy((char_u *)fontname, name, name_len);
prt_ps_mb_font.ps_fontname[font] = fontname;

return TRUE;
}




static int
prt_get_lpp(void)
{
int lpp;






prt_bgcol_offset = (float)PRT_PS_FONT_TO_USER(prt_line_height,
prt_ps_font->bbox_min_y);
if ((prt_ps_font->bbox_max_y - prt_ps_font->bbox_min_y) < 1000.0)
{
prt_bgcol_offset -= (float)PRT_PS_FONT_TO_USER(prt_line_height,
(1000.0 - (prt_ps_font->bbox_max_y -
prt_ps_font->bbox_min_y)) / 2);
}


prt_first_line_height = prt_line_height + prt_bgcol_offset;


lpp = (int)((prt_top_margin - prt_bottom_margin) / prt_line_height);


prt_top_margin -= prt_line_height * prt_header_height();

return lpp - prt_header_height();
}

static int
prt_match_encoding(
char *p_encoding,
struct prt_ps_mbfont_S *p_cmap,
struct prt_ps_encoding_S **pp_mbenc)
{
int mbenc;
int enc_len;
struct prt_ps_encoding_S *p_mbenc;

*pp_mbenc = NULL;

enc_len = (int)STRLEN(p_encoding);
p_mbenc = p_cmap->encodings;
for (mbenc = 0; mbenc < p_cmap->num_encodings; mbenc++)
{
if (STRNICMP(p_mbenc->encoding, p_encoding, enc_len) == 0)
{
*pp_mbenc = p_mbenc;
return TRUE;
}
p_mbenc++;
}
return FALSE;
}

static int
prt_match_charset(
char *p_charset,
struct prt_ps_mbfont_S *p_cmap,
struct prt_ps_charset_S **pp_mbchar)
{
int mbchar;
int char_len;
struct prt_ps_charset_S *p_mbchar;


if (*p_charset == NUL)
p_charset = p_cmap->defcs;
char_len = (int)STRLEN(p_charset);
p_mbchar = p_cmap->charsets;
for (mbchar = 0; mbchar < p_cmap->num_charsets; mbchar++)
{
if (STRNICMP(p_mbchar->charset, p_charset, char_len) == 0)
{
*pp_mbchar = p_mbchar;
return TRUE;
}
p_mbchar++;
}
return FALSE;
}

int
mch_print_init(
prt_settings_T *psettings,
char_u *jobname,
int forceit UNUSED)
{
int i;
char *paper_name;
int paper_strlen;
int fontsize;
char_u *p;
double left;
double right;
double top;
double bottom;
int props;
int cmap = 0;
char_u *p_encoding;
struct prt_ps_encoding_S *p_mbenc;
struct prt_ps_encoding_S *p_mbenc_first;
struct prt_ps_charset_S *p_mbchar = NULL;

#if 0













if (forceit)
s_pd.Flags |= PD_RETURNDEFAULT;
#endif




p_encoding = enc_skip(p_penc);
if (*p_encoding == NUL)
p_encoding = enc_skip(p_enc);





p_mbenc = NULL;
props = enc_canon_props(p_encoding);
if (!(props & ENC_8BIT) && ((*p_pmcs != NUL) || !(props & ENC_UNICODE)))
{
int cmap_first = 0;

p_mbenc_first = NULL;
for (cmap = 0; cmap < (int)NUM_ELEMENTS(prt_ps_mbfonts); cmap++)
if (prt_match_encoding((char *)p_encoding, &prt_ps_mbfonts[cmap],
&p_mbenc))
{
if (p_mbenc_first == NULL)
{
p_mbenc_first = p_mbenc;
cmap_first = cmap;
}
if (prt_match_charset((char *)p_pmcs, &prt_ps_mbfonts[cmap],
&p_mbchar))
break;
}


if (p_mbchar == NULL && p_mbenc_first != NULL)
{
p_mbenc = p_mbenc_first;
cmap = cmap_first;
}
}

prt_out_mbyte = (p_mbenc != NULL);
if (prt_out_mbyte)
{

prt_cmap[0] = NUL;

prt_custom_cmap = (p_mbchar == NULL);
if (!prt_custom_cmap)
{

if ((p_mbenc->needs_charset & p_mbchar->has_charset) == 0)
{
emsg(_("E673: Incompatible multi-byte encoding and character set."));
return FALSE;
}


if (p_mbchar->cmap_charset != NULL)
{
vim_strncpy((char_u *)prt_cmap,
(char_u *)p_mbchar->cmap_charset, sizeof(prt_cmap) - 3);
STRCAT(prt_cmap, "-");
}
}
else
{

if (*p_pmcs == NUL)
{
emsg(_("E674: printmbcharset cannot be empty with multi-byte encoding."));
return FALSE;
}
vim_strncpy((char_u *)prt_cmap, p_pmcs, sizeof(prt_cmap) - 3);
STRCAT(prt_cmap, "-");
}


if (p_mbenc->cmap_encoding != NULL && STRLEN(prt_cmap)
+ STRLEN(p_mbenc->cmap_encoding) + 3 < sizeof(prt_cmap))
{
STRCAT(prt_cmap, p_mbenc->cmap_encoding);
STRCAT(prt_cmap, "-");
}
STRCAT(prt_cmap, "H");

if (!mbfont_opts[OPT_MBFONT_REGULAR].present)
{
emsg(_("E675: No default font specified for multi-byte printing."));
return FALSE;
}


if (!prt_build_cid_fontname(PRT_PS_FONT_ROMAN,
mbfont_opts[OPT_MBFONT_REGULAR].string,
mbfont_opts[OPT_MBFONT_REGULAR].strlen))
return FALSE;
if (mbfont_opts[OPT_MBFONT_BOLD].present)
if (!prt_build_cid_fontname(PRT_PS_FONT_BOLD,
mbfont_opts[OPT_MBFONT_BOLD].string,
mbfont_opts[OPT_MBFONT_BOLD].strlen))
return FALSE;
if (mbfont_opts[OPT_MBFONT_OBLIQUE].present)
if (!prt_build_cid_fontname(PRT_PS_FONT_OBLIQUE,
mbfont_opts[OPT_MBFONT_OBLIQUE].string,
mbfont_opts[OPT_MBFONT_OBLIQUE].strlen))
return FALSE;
if (mbfont_opts[OPT_MBFONT_BOLDOBLIQUE].present)
if (!prt_build_cid_fontname(PRT_PS_FONT_BOLDOBLIQUE,
mbfont_opts[OPT_MBFONT_BOLDOBLIQUE].string,
mbfont_opts[OPT_MBFONT_BOLDOBLIQUE].strlen))
return FALSE;



prt_use_courier = mbfont_opts[OPT_MBFONT_USECOURIER].present &&
(TOLOWER_ASC(mbfont_opts[OPT_MBFONT_USECOURIER].string[0]) == 'y');
if (prt_use_courier)
{

if (mbfont_opts[OPT_MBFONT_ASCII].present &&
(TOLOWER_ASC(mbfont_opts[OPT_MBFONT_ASCII].string[0]) == 'y'))
prt_ascii_encoding = "ascii";
else
prt_ascii_encoding = prt_ps_mbfonts[cmap].ascii_enc;
}

prt_ps_font = &prt_ps_mb_font;
}
else
{
prt_use_courier = FALSE;
prt_ps_font = &prt_ps_courier_font;
}




prt_portrait = (!printer_opts[OPT_PRINT_PORTRAIT].present
|| TOLOWER_ASC(printer_opts[OPT_PRINT_PORTRAIT].string[0]) == 'y');
if (printer_opts[OPT_PRINT_PAPER].present)
{
paper_name = (char *)printer_opts[OPT_PRINT_PAPER].string;
paper_strlen = printer_opts[OPT_PRINT_PAPER].strlen;
}
else
{
paper_name = "A4";
paper_strlen = 2;
}
for (i = 0; i < (int)PRT_MEDIASIZE_LEN; ++i)
if (STRLEN(prt_mediasize[i].name) == (unsigned)paper_strlen
&& STRNICMP(prt_mediasize[i].name, paper_name,
paper_strlen) == 0)
break;
if (i == PRT_MEDIASIZE_LEN)
i = 0;
prt_media = i;








if (prt_portrait)
{
prt_page_width = prt_mediasize[i].width;
prt_page_height = prt_mediasize[i].height;
}
else
{
prt_page_width = prt_mediasize[i].height;
prt_page_height = prt_mediasize[i].width;
}





prt_page_margins(prt_page_width, prt_page_height, &left, &right, &top,
&bottom);
prt_left_margin = (float)left;
prt_right_margin = (float)right;
prt_top_margin = (float)top;
prt_bottom_margin = (float)bottom;




fontsize = PRT_PS_DEFAULT_FONTSIZE;
for (p = p_pfn; (p = vim_strchr(p, ':')) != NULL; ++p)
if (p[1] == 'h' && VIM_ISDIGIT(p[2]))
fontsize = atoi((char *)p + 2);
prt_font_metrics(fontsize);





psettings->chars_per_line = prt_get_cpl();
psettings->lines_per_page = prt_get_lpp();


if (psettings->chars_per_line <= 0 || psettings->lines_per_page <= 0)
return FAIL;







psettings->n_collated_copies = 1;
psettings->n_uncollated_copies = 1;
prt_num_copies = 1;
prt_collate = (!printer_opts[OPT_PRINT_COLLATE].present
|| TOLOWER_ASC(printer_opts[OPT_PRINT_COLLATE].string[0]) == 'y');
if (prt_collate)
{

psettings->n_collated_copies = 1;
}
else
{


prt_num_copies = 1;
}

psettings->jobname = jobname;





prt_duplex = TRUE;
prt_tumble = FALSE;
psettings->duplex = 1;
if (printer_opts[OPT_PRINT_DUPLEX].present)
{
if (STRNICMP(printer_opts[OPT_PRINT_DUPLEX].string, "off", 3) == 0)
{
prt_duplex = FALSE;
psettings->duplex = 0;
}
else if (STRNICMP(printer_opts[OPT_PRINT_DUPLEX].string, "short", 5)
== 0)
prt_tumble = TRUE;
}


psettings->user_abort = 0;


if (psettings->outfile == NULL)
{
prt_ps_file_name = vim_tempname('p', TRUE);
if (prt_ps_file_name == NULL)
{
emsg(_(e_notmp));
return FAIL;
}
prt_ps_fd = mch_fopen((char *)prt_ps_file_name, WRITEBIN);
}
else
{
p = expand_env_save(psettings->outfile);
if (p != NULL)
{
prt_ps_fd = mch_fopen((char *)p, WRITEBIN);
vim_free(p);
}
}
if (prt_ps_fd == NULL)
{
emsg(_("E324: Can't open PostScript output file"));
mch_print_cleanup();
return FAIL;
}

prt_bufsiz = psettings->chars_per_line;
if (prt_out_mbyte)
prt_bufsiz *= 2;
ga_init2(&prt_ps_buffer, (int)sizeof(char), prt_bufsiz);

prt_page_num = 0;

prt_attribute_change = FALSE;
prt_need_moveto = FALSE;
prt_need_font = FALSE;
prt_need_fgcol = FALSE;
prt_need_bgcol = FALSE;
prt_need_underline = FALSE;

prt_file_error = FALSE;

return OK;
}

static int
prt_add_resource(struct prt_ps_resource_S *resource)
{
FILE* fd_resource;
char_u resource_buffer[512];
size_t bytes_read;

fd_resource = mch_fopen((char *)resource->filename, READBIN);
if (fd_resource == NULL)
{
semsg(_("E456: Can't open file \"%s\""), resource->filename);
return FALSE;
}
prt_dsc_resources("BeginResource", prt_resource_types[resource->type],
(char *)resource->title);

prt_dsc_textline("BeginDocument", (char *)resource->filename);

for (;;)
{
bytes_read = fread((char *)resource_buffer, sizeof(char_u),
sizeof(resource_buffer), fd_resource);
if (ferror(fd_resource))
{
semsg(_("E457: Can't read PostScript resource file \"%s\""),
resource->filename);
fclose(fd_resource);
return FALSE;
}
if (bytes_read == 0)
break;
prt_write_file_raw_len(resource_buffer, (int)bytes_read);
if (prt_file_error)
{
fclose(fd_resource);
return FALSE;
}
}
fclose(fd_resource);

prt_dsc_noarg("EndDocument");

prt_dsc_noarg("EndResource");

return TRUE;
}

int
mch_print_begin(prt_settings_T *psettings)
{
int bbox[4];
double left;
double right;
double top;
double bottom;
struct prt_ps_resource_S *res_prolog;
struct prt_ps_resource_S *res_encoding;
char buffer[256];
char_u *p_encoding;
struct prt_ps_resource_S *res_cidfont;
struct prt_ps_resource_S *res_cmap;
int retval = FALSE;

res_prolog = (struct prt_ps_resource_S *)
alloc(sizeof(struct prt_ps_resource_S));
res_encoding = (struct prt_ps_resource_S *)
alloc(sizeof(struct prt_ps_resource_S));
res_cidfont = (struct prt_ps_resource_S *)
alloc(sizeof(struct prt_ps_resource_S));
res_cmap = (struct prt_ps_resource_S *)
alloc(sizeof(struct prt_ps_resource_S));
if (res_prolog == NULL || res_encoding == NULL
|| res_cidfont == NULL || res_cmap == NULL)
goto theend;




prt_dsc_start();
prt_dsc_textline("Title", (char *)psettings->jobname);
if (!get_user_name((char_u *)buffer, 256))
STRCPY(buffer, "Unknown");
prt_dsc_textline("For", buffer);
prt_dsc_textline("Creator", VIM_VERSION_LONG);


prt_dsc_textline("CreationDate", get_ctime(time(NULL), FALSE));
prt_dsc_textline("DocumentData", "Clean8Bit");
prt_dsc_textline("Orientation", "Portrait");
prt_dsc_atend("Pages");
prt_dsc_textline("PageOrder", "Ascend");



prt_page_margins(prt_mediasize[prt_media].width,
prt_mediasize[prt_media].height,
&left, &right, &top, &bottom);
bbox[0] = (int)left;
if (prt_portrait)
{



bbox[1] = (int)(top - (psettings->lines_per_page + prt_header_height())
* prt_line_height);
bbox[2] = (int)(left + psettings->chars_per_line * prt_char_width
+ 0.5);
bbox[3] = (int)(top + 0.5);
}
else
{



bbox[1] = (int)bottom;
bbox[2] = (int)(left + ((psettings->lines_per_page
+ prt_header_height()) * prt_line_height) + 0.5);
bbox[3] = (int)(bottom + psettings->chars_per_line * prt_char_width
+ 0.5);
}
prt_dsc_ints("BoundingBox", 4, bbox);

prt_dsc_docmedia(prt_mediasize[prt_media].name,
prt_mediasize[prt_media].width,
prt_mediasize[prt_media].height,
(double)0, NULL, NULL);

if (!prt_out_mbyte || prt_use_courier)
prt_dsc_font_resource("DocumentNeededResources", &prt_ps_courier_font);
if (prt_out_mbyte)
{
prt_dsc_font_resource((prt_use_courier ? NULL
: "DocumentNeededResources"), &prt_ps_mb_font);
if (!prt_custom_cmap)
prt_dsc_resources(NULL, "cmap", prt_cmap);
}


if (!prt_find_resource("prolog", res_prolog))
{
emsg(_("E456: Can't find PostScript resource file \"prolog.ps\""));
goto theend;
}
if (!prt_open_resource(res_prolog))
goto theend;
if (!prt_check_resource(res_prolog, PRT_PROLOG_VERSION))
goto theend;
if (prt_out_mbyte)
{

if (!prt_find_resource("cidfont", res_cidfont))
{
emsg(_("E456: Can't find PostScript resource file \"cidfont.ps\""));
goto theend;
}
if (!prt_open_resource(res_cidfont))
goto theend;
if (!prt_check_resource(res_cidfont, PRT_CID_PROLOG_VERSION))
goto theend;
}





if (!prt_out_mbyte)
{
p_encoding = enc_skip(p_penc);
if (*p_encoding == NUL
|| !prt_find_resource((char *)p_encoding, res_encoding))
{

int props;

p_encoding = enc_skip(p_enc);
props = enc_canon_props(p_encoding);
if (!(props & ENC_8BIT)
|| !prt_find_resource((char *)p_encoding, res_encoding))

{

p_encoding = (char_u *)"latin1";
if (!prt_find_resource((char *)p_encoding, res_encoding))
{
semsg(_("E456: Can't find PostScript resource file \"%s.ps\""),
p_encoding);
goto theend;
}
}
}
if (!prt_open_resource(res_encoding))
goto theend;


}
else
{
p_encoding = enc_skip(p_penc);
if (*p_encoding == NUL)
p_encoding = enc_skip(p_enc);
if (prt_use_courier)
{

if (!prt_find_resource(prt_ascii_encoding, res_encoding))
{
semsg(_("E456: Can't find PostScript resource file \"%s.ps\""),
prt_ascii_encoding);
goto theend;
}
if (!prt_open_resource(res_encoding))
goto theend;


}
}

prt_conv.vc_type = CONV_NONE;
if (!(enc_canon_props(p_enc) & enc_canon_props(p_encoding) & ENC_8BIT)) {

if (FAIL == convert_setup(&prt_conv, p_enc, p_encoding))
{
semsg(_("E620: Unable to convert to print encoding \"%s\""),
p_encoding);
goto theend;
}
prt_do_conv = TRUE;
}
prt_do_conv = prt_conv.vc_type != CONV_NONE;

if (prt_out_mbyte && prt_custom_cmap)
{

if (!prt_find_resource(prt_cmap, res_cmap))
{
semsg(_("E456: Can't find PostScript resource file \"%s.ps\""),
prt_cmap);
goto theend;
}
if (!prt_open_resource(res_cmap))
goto theend;
}


STRCPY(buffer, res_prolog->title);
STRCAT(buffer, " ");
STRCAT(buffer, res_prolog->version);
prt_dsc_resources("DocumentSuppliedResources", "procset", buffer);
if (prt_out_mbyte)
{
STRCPY(buffer, res_cidfont->title);
STRCAT(buffer, " ");
STRCAT(buffer, res_cidfont->version);
prt_dsc_resources(NULL, "procset", buffer);

if (prt_custom_cmap)
{
STRCPY(buffer, res_cmap->title);
STRCAT(buffer, " ");
STRCAT(buffer, res_cmap->version);
prt_dsc_resources(NULL, "cmap", buffer);
}
}
if (!prt_out_mbyte || prt_use_courier)
{
STRCPY(buffer, res_encoding->title);
STRCAT(buffer, " ");
STRCAT(buffer, res_encoding->version);
prt_dsc_resources(NULL, "encoding", buffer);
}
prt_dsc_requirements(prt_duplex, prt_tumble, prt_collate,
#if defined(FEAT_SYN_HL)
psettings->do_syntax
#else
0
#endif
, prt_num_copies);
prt_dsc_noarg("EndComments");




prt_dsc_noarg("BeginDefaults");


if (!prt_out_mbyte || prt_use_courier)
prt_dsc_font_resource("PageResources", &prt_ps_courier_font);
if (prt_out_mbyte)
{
prt_dsc_font_resource((prt_use_courier ? NULL : "PageResources"),
&prt_ps_mb_font);
if (!prt_custom_cmap)
prt_dsc_resources(NULL, "cmap", prt_cmap);
}


prt_dsc_textline("PageMedia", prt_mediasize[prt_media].name);

prt_dsc_noarg("EndDefaults");




prt_dsc_noarg("BeginProlog");


if (!prt_add_resource(res_prolog))
goto theend;
if (prt_out_mbyte)
{

if (!prt_add_resource(res_cidfont))
goto theend;
if (prt_custom_cmap && !prt_add_resource(res_cmap))
goto theend;
}

if (!prt_out_mbyte || prt_use_courier)


if (!prt_add_resource(res_encoding))
goto theend;

prt_dsc_noarg("EndProlog");




prt_dsc_noarg("BeginSetup");


prt_write_int((int)prt_mediasize[prt_media].width);
prt_write_int((int)prt_mediasize[prt_media].height);
prt_write_int(0);
prt_write_string("sps\n");
prt_write_int(prt_num_copies);
prt_write_string("nc\n");
prt_write_boolean(prt_duplex);
prt_write_boolean(prt_tumble);
prt_write_string("dt\n");
prt_write_boolean(prt_collate);
prt_write_string("c\n");


if (!prt_out_mbyte || prt_use_courier)
{


if (prt_use_courier)
p_encoding = (char_u *)prt_ascii_encoding;
prt_dsc_resources("IncludeResource", "font",
prt_ps_courier_font.ps_fontname[PRT_PS_FONT_ROMAN]);
prt_def_font("F0", (char *)p_encoding, (int)prt_line_height,
prt_ps_courier_font.ps_fontname[PRT_PS_FONT_ROMAN]);
prt_dsc_resources("IncludeResource", "font",
prt_ps_courier_font.ps_fontname[PRT_PS_FONT_BOLD]);
prt_def_font("F1", (char *)p_encoding, (int)prt_line_height,
prt_ps_courier_font.ps_fontname[PRT_PS_FONT_BOLD]);
prt_dsc_resources("IncludeResource", "font",
prt_ps_courier_font.ps_fontname[PRT_PS_FONT_OBLIQUE]);
prt_def_font("F2", (char *)p_encoding, (int)prt_line_height,
prt_ps_courier_font.ps_fontname[PRT_PS_FONT_OBLIQUE]);
prt_dsc_resources("IncludeResource", "font",
prt_ps_courier_font.ps_fontname[PRT_PS_FONT_BOLDOBLIQUE]);
prt_def_font("F3", (char *)p_encoding, (int)prt_line_height,
prt_ps_courier_font.ps_fontname[PRT_PS_FONT_BOLDOBLIQUE]);
}
if (prt_out_mbyte)
{





prt_dsc_resources("IncludeResource", "font",
prt_ps_mb_font.ps_fontname[PRT_PS_FONT_ROMAN]);
if (!prt_custom_cmap)
prt_dsc_resources("IncludeResource", "cmap", prt_cmap);
prt_def_cidfont("CF0", (int)prt_line_height,
prt_ps_mb_font.ps_fontname[PRT_PS_FONT_ROMAN]);

if (prt_ps_mb_font.ps_fontname[PRT_PS_FONT_BOLD] != NULL)
{
prt_dsc_resources("IncludeResource", "font",
prt_ps_mb_font.ps_fontname[PRT_PS_FONT_BOLD]);
if (!prt_custom_cmap)
prt_dsc_resources("IncludeResource", "cmap", prt_cmap);
prt_def_cidfont("CF1", (int)prt_line_height,
prt_ps_mb_font.ps_fontname[PRT_PS_FONT_BOLD]);
}
else

prt_dup_cidfont("CF0", "CF1");

if (prt_ps_mb_font.ps_fontname[PRT_PS_FONT_OBLIQUE] != NULL)
{
prt_dsc_resources("IncludeResource", "font",
prt_ps_mb_font.ps_fontname[PRT_PS_FONT_OBLIQUE]);
if (!prt_custom_cmap)
prt_dsc_resources("IncludeResource", "cmap", prt_cmap);
prt_def_cidfont("CF2", (int)prt_line_height,
prt_ps_mb_font.ps_fontname[PRT_PS_FONT_OBLIQUE]);
}
else

prt_dup_cidfont("CF0", "CF2");

if (prt_ps_mb_font.ps_fontname[PRT_PS_FONT_BOLDOBLIQUE] != NULL)
{
prt_dsc_resources("IncludeResource", "font",
prt_ps_mb_font.ps_fontname[PRT_PS_FONT_BOLDOBLIQUE]);
if (!prt_custom_cmap)
prt_dsc_resources("IncludeResource", "cmap", prt_cmap);
prt_def_cidfont("CF3", (int)prt_line_height,
prt_ps_mb_font.ps_fontname[PRT_PS_FONT_BOLDOBLIQUE]);
}
else

prt_dup_cidfont("CF1", "CF3");
}


prt_def_var("UO", PRT_PS_FONT_TO_USER(prt_line_height,
prt_ps_font->uline_offset), 2);
prt_def_var("UW", PRT_PS_FONT_TO_USER(prt_line_height,
prt_ps_font->uline_width), 2);
prt_def_var("BO", prt_bgcol_offset, 2);

prt_dsc_noarg("EndSetup");


retval = !prt_file_error;

theend:
vim_free(res_prolog);
vim_free(res_encoding);
vim_free(res_cidfont);
vim_free(res_cmap);

return retval;
}

void
mch_print_end(prt_settings_T *psettings)
{
prt_dsc_noarg("Trailer");




prt_dsc_ints("Pages", 1, &prt_page_num);

prt_dsc_noarg("EOF");



prt_write_file((char_u *)IF_EB("\004", "\067"));

if (!prt_file_error && psettings->outfile == NULL
&& !got_int && !psettings->user_abort)
{

if (prt_ps_fd != NULL)
{
fclose(prt_ps_fd);
prt_ps_fd = NULL;
}
prt_message((char_u *)_("Sending to printer..."));


if (eval_printexpr(prt_ps_file_name, psettings->arguments) == FAIL)
emsg(_("E365: Failed to print PostScript file"));
else
prt_message((char_u *)_("Print job sent."));
}

mch_print_cleanup();
}

int
mch_print_end_page(void)
{
prt_flush_buffer();

prt_write_string("re sp\n");

prt_dsc_noarg("PageTrailer");

return !prt_file_error;
}

int
mch_print_begin_page(char_u *str UNUSED)
{
int page_num[2];

prt_page_num++;

page_num[0] = page_num[1] = prt_page_num;
prt_dsc_ints("Page", 2, page_num);

prt_dsc_noarg("BeginPageSetup");

prt_write_string("sv\n0 g\n");
prt_in_ascii = !prt_out_mbyte;
if (prt_out_mbyte)
prt_write_string("CF0 sf\n");
else
prt_write_string("F0 sf\n");
prt_fgcol = PRCOLOR_BLACK;
prt_bgcol = PRCOLOR_WHITE;
prt_font = PRT_PS_FONT_ROMAN;


if (!prt_portrait)
{
prt_write_int(-((int)prt_mediasize[prt_media].width));
prt_write_string("sl\n");
}

prt_dsc_noarg("EndPageSetup");


curr_bg = (long_u)0xffffffff;
curr_fg = (long_u)0xffffffff;
curr_bold = MAYBE;

return !prt_file_error;
}

int
mch_print_blank_page(void)
{
return (mch_print_begin_page(NULL) ? (mch_print_end_page()) : FALSE);
}

static float prt_pos_x = 0;
static float prt_pos_y = 0;

void
mch_print_start_line(int margin, int page_line)
{
prt_pos_x = prt_left_margin;
if (margin)
prt_pos_x -= prt_number_width;

prt_pos_y = prt_top_margin - prt_first_line_height -
page_line * prt_line_height;

prt_attribute_change = TRUE;
prt_need_moveto = TRUE;
prt_half_width = FALSE;
}

int
mch_print_text_out(char_u *textp, int len UNUSED)
{
char_u *p = textp;
int need_break;
char_u ch;
char_u ch_buff[8];
float char_width;
float next_pos;
int in_ascii;
int half_width;
char_u *tofree = NULL;

char_width = prt_char_width;







if (prt_use_courier)
{
in_ascii = (len == 1 && *p < 0x80);
if (prt_in_ascii)
{
if (!in_ascii)
{

prt_in_ascii = FALSE;
prt_need_font = TRUE;
prt_attribute_change = TRUE;
}
}
else if (in_ascii)
{

prt_in_ascii = TRUE;
prt_need_font = TRUE;
prt_attribute_change = TRUE;
}
}
if (prt_out_mbyte)
{
half_width = ((*mb_ptr2cells)(p) == 1);
if (half_width)
char_width /= 2;
if (prt_half_width)
{
if (!half_width)
{
prt_half_width = FALSE;
prt_pos_x += prt_char_width/4;
prt_need_moveto = TRUE;
prt_attribute_change = TRUE;
}
}
else if (half_width)
{
prt_half_width = TRUE;
prt_pos_x += prt_char_width/4;
prt_need_moveto = TRUE;
prt_attribute_change = TRUE;
}
}



if (prt_attribute_change)
{
prt_flush_buffer();

prt_text_run = 0;

if (prt_need_moveto)
{
prt_pos_x_moveto = prt_pos_x;
prt_pos_y_moveto = prt_pos_y;
prt_do_moveto = TRUE;

prt_need_moveto = FALSE;
}
if (prt_need_font)
{
if (!prt_in_ascii)
prt_write_string("CF");
else
prt_write_string("F");
prt_write_int(prt_font);
prt_write_string("sf\n");
prt_need_font = FALSE;
}
if (prt_need_fgcol)
{
int r, g, b;
r = ((unsigned)prt_fgcol & 0xff0000) >> 16;
g = ((unsigned)prt_fgcol & 0xff00) >> 8;
b = prt_fgcol & 0xff;

prt_write_real(r / 255.0, 3);
if (r == g && g == b)
prt_write_string("g\n");
else
{
prt_write_real(g / 255.0, 3);
prt_write_real(b / 255.0, 3);
prt_write_string("r\n");
}
prt_need_fgcol = FALSE;
}

if (prt_bgcol != PRCOLOR_WHITE)
{
prt_new_bgcol = prt_bgcol;
if (prt_need_bgcol)
prt_do_bgcol = TRUE;
}
else
prt_do_bgcol = FALSE;
prt_need_bgcol = FALSE;

if (prt_need_underline)
prt_do_underline = prt_underline;
prt_need_underline = FALSE;

prt_attribute_change = FALSE;
}

if (prt_do_conv)
{

tofree = p = string_convert(&prt_conv, p, &len);
if (p == NULL)
{
p = (char_u *)"";
len = 0;
}
}

if (prt_out_mbyte)
{


while (len-- > 0)
{
ch = prt_hexchar[(unsigned)(*p) >> 4];
ga_append(&prt_ps_buffer, ch);
ch = prt_hexchar[(*p) & 0xf];
ga_append(&prt_ps_buffer, ch);
p++;
}
}
else
{



ch = *p;
if (ch < 32 || ch == '(' || ch == ')' || ch == '\\')
{





ga_append(&prt_ps_buffer, IF_EB('\\', 0134));
switch (ch)
{
case BS: ga_append(&prt_ps_buffer, IF_EB('b', 0142)); break;
case TAB: ga_append(&prt_ps_buffer, IF_EB('t', 0164)); break;
case NL: ga_append(&prt_ps_buffer, IF_EB('n', 0156)); break;
case FF: ga_append(&prt_ps_buffer, IF_EB('f', 0146)); break;
case CAR: ga_append(&prt_ps_buffer, IF_EB('r', 0162)); break;
case '(': ga_append(&prt_ps_buffer, IF_EB('(', 0050)); break;
case ')': ga_append(&prt_ps_buffer, IF_EB(')', 0051)); break;
case '\\': ga_append(&prt_ps_buffer, IF_EB('\\', 0134)); break;

default:
sprintf((char *)ch_buff, "%03o", (unsigned int)ch);
#if defined(EBCDIC)
ebcdic2ascii(ch_buff, 3);
#endif
ga_append(&prt_ps_buffer, ch_buff[0]);
ga_append(&prt_ps_buffer, ch_buff[1]);
ga_append(&prt_ps_buffer, ch_buff[2]);
break;
}
}
else
ga_append(&prt_ps_buffer, ch);
}


vim_free(tofree);

prt_text_run += char_width;
prt_pos_x += char_width;


next_pos = prt_pos_x + prt_char_width;
need_break = (next_pos > prt_right_margin) &&
((next_pos - prt_right_margin) > (prt_right_margin*1e-5));

if (need_break)
prt_flush_buffer();

return need_break;
}

void
mch_print_set_font(int iBold, int iItalic, int iUnderline)
{
int font = 0;

if (iBold)
font |= 0x01;
if (iItalic)
font |= 0x02;

if (font != prt_font)
{
prt_font = font;
prt_attribute_change = TRUE;
prt_need_font = TRUE;
}
if (prt_underline != iUnderline)
{
prt_underline = iUnderline;
prt_attribute_change = TRUE;
prt_need_underline = TRUE;
}
}

void
mch_print_set_bg(long_u bgcol)
{
prt_bgcol = (int)bgcol;
prt_attribute_change = TRUE;
prt_need_bgcol = TRUE;
}

void
mch_print_set_fg(long_u fgcol)
{
if (fgcol != (long_u)prt_fgcol)
{
prt_fgcol = (int)fgcol;
prt_attribute_change = TRUE;
prt_need_fgcol = TRUE;
}
}

#endif 
#endif 
