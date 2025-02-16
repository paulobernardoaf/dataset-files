









#include "vim.h"


gui_T gui;

#if !defined(FEAT_GUI_GTK)
static void set_guifontwide(char_u *font_name);
#endif
static void gui_check_pos(void);
static void gui_reset_scroll_region(void);
static void gui_outstr(char_u *, int);
static int gui_screenchar(int off, int flags, guicolor_T fg, guicolor_T bg, int back);
static int gui_outstr_nowrap(char_u *s, int len, int flags, guicolor_T fg, guicolor_T bg, int back);
static void gui_delete_lines(int row, int count);
static void gui_insert_lines(int row, int count);
static int gui_xy2colrow(int x, int y, int *colp);
#if defined(FEAT_GUI_TABLINE) || defined(PROTO)
static int gui_has_tabline(void);
#endif
static void gui_do_scrollbar(win_T *wp, int which, int enable);
static void gui_update_horiz_scrollbar(int);
static void gui_set_fg_color(char_u *name);
static void gui_set_bg_color(char_u *name);
static win_T *xy2win(int x, int y, mouse_find_T popup);

#if defined(GUI_MAY_FORK)
static void gui_do_fork(void);

static int gui_read_child_pipe(int fd);


enum {
GUI_CHILD_IO_ERROR,
GUI_CHILD_OK,
GUI_CHILD_FAILED
};
#endif

static void gui_attempt_start(void);

static int can_update_cursor = TRUE; 
static int disable_flush = 0; 






#if defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_MAC)
#define SCROLL_PAST_END
#endif








void
gui_start(char_u *arg UNUSED)
{
char_u *old_term;
static int recursive = 0;
#if defined(GUI_MAY_SPAWN) && defined(EXPERIMENTAL_GUI_CMD)
char *msg = NULL;
#endif

old_term = vim_strsave(T_NAME);

settmode(TMODE_COOK); 
if (full_screen)
cursor_on(); 
full_screen = FALSE;

++recursive;

#if defined(GUI_MAY_FORK)








if (gui.dofork && !vim_strchr(p_go, GO_FORG) && recursive <= 1
#if defined(FEAT_JOB_CHANNEL)
&& !job_any_running()
#endif
)
{
gui_do_fork();
}
else
#endif
#if defined(GUI_MAY_SPAWN)
if (gui.dospawn
#if defined(EXPERIMENTAL_GUI_CMD)
&& gui.dofork
#endif
&& !vim_strchr(p_go, GO_FORG)
&& !anyBufIsChanged()
#if defined(FEAT_JOB_CHANNEL)
&& !job_any_running()
#endif
)
{
#if defined(EXPERIMENTAL_GUI_CMD)
msg =
#endif
gui_mch_do_spawn(arg);
}
else
#endif
{
#if defined(FEAT_GUI_GTK)


if (gui_mch_init_check() != OK)
getout_preserve_modified(1);
#endif
gui_attempt_start();
}

if (!gui.in_use) 
{









termcapinit(old_term);
settmode(TMODE_RAW); 
#if defined(FEAT_TITLE)
set_title_defaults(); 
#endif
#if defined(GUI_MAY_SPAWN) && defined(EXPERIMENTAL_GUI_CMD)
if (msg)
emsg(msg);
#endif
}

vim_free(old_term);



gui_mch_update();
apply_autocmds(gui.in_use ? EVENT_GUIENTER : EVENT_GUIFAILED,
NULL, NULL, FALSE, curbuf);
--recursive;
}











static void
gui_attempt_start(void)
{
static int recursive = 0;

++recursive;
gui.starting = TRUE;

#if defined(FEAT_GUI_GTK)
gui.event_time = GDK_CURRENT_TIME;
#endif

termcapinit((char_u *)"builtin_gui");
gui.starting = recursive - 1;

#if defined(FEAT_GUI_GTK) || defined(FEAT_GUI_X11)
if (gui.in_use)
{
#if defined(FEAT_EVAL)
Window x11_window;
Display *x11_display;

if (gui_get_x11_windis(&x11_window, &x11_display) == OK)
set_vim_var_nr(VV_WINDOWID, (long)x11_window);
#endif


display_errors();
}
#endif
--recursive;
}

#if defined(GUI_MAY_FORK)


#if defined(HAVE_SYS_WAIT_H) || defined(HAVE_UNION_WAIT)
#include <sys/wait.h>
#endif













static void
gui_do_fork(void)
{
int pipefd[2]; 
int pipe_error;
int status;
int exit_status;
pid_t pid = -1;




pipe_error = (pipe(pipefd) < 0);
pid = fork();
if (pid < 0) 
{
emsg(_("E851: Failed to create a new process for the GUI"));
return;
}
else if (pid > 0) 
{



if (!pipe_error)
{


close(pipefd[1]);
status = gui_read_child_pipe(pipefd[0]);
if (status == GUI_CHILD_FAILED)
{



#if defined(__NeXT__)
wait4(pid, &exit_status, 0, (struct rusage *)0);
#else
waitpid(pid, &exit_status, 0);
#endif
emsg(_("E852: The child process failed to start the GUI"));
return;
}
else if (status == GUI_CHILD_IO_ERROR)
{
pipe_error = TRUE;
}

}

if (pipe_error)
ui_delay(301L, TRUE);



if (newline_on_exit)
mch_errmsg("\r\n");





_exit(0);
}


#if defined(FEAT_GUI_GTK)

if (gui_mch_init_check() != OK)
getout_preserve_modified(1);
#endif

#if defined(HAVE_SETSID) || defined(HAVE_SETPGID)




#if defined(HAVE_SETSID)
(void)setsid();
#else
(void)setpgid(0, 0);
#endif
#endif
if (!pipe_error)
close(pipefd[0]);

#if defined(FEAT_GUI_GNOME) && defined(FEAT_SESSION)

gui_mch_forked();
#endif


gui_attempt_start();


if (!pipe_error)
{
if (gui.in_use)
write_eintr(pipefd[1], "ok", 3);
else
write_eintr(pipefd[1], "fail", 5);
close(pipefd[1]);
}


if (!gui.in_use)
getout_preserve_modified(1);
}










static int
gui_read_child_pipe(int fd)
{
long bytes_read;
#define READ_BUFFER_SIZE 10
char buffer[READ_BUFFER_SIZE];

bytes_read = read_eintr(fd, buffer, READ_BUFFER_SIZE - 1);
#undef READ_BUFFER_SIZE
close(fd);
if (bytes_read < 0)
return GUI_CHILD_IO_ERROR;
buffer[bytes_read] = NUL;
if (strcmp(buffer, "ok") == 0)
return GUI_CHILD_OK;
return GUI_CHILD_FAILED;
}

#endif 




void
gui_prepare(int *argc, char **argv)
{
gui.in_use = FALSE; 
gui.starting = FALSE; 
gui_mch_prepare(argc, argv);
}







int
gui_init_check(void)
{
static int result = MAYBE;

if (result != MAYBE)
{
if (result == FAIL)
emsg(_("E229: Cannot start the GUI"));
return result;
}

gui.shell_created = FALSE;
gui.dying = FALSE;
gui.in_focus = TRUE; 
gui.dragged_sb = SBAR_NONE;
gui.dragged_wp = NULL;
gui.pointer_hidden = FALSE;
gui.col = 0;
gui.row = 0;
gui.num_cols = Columns;
gui.num_rows = Rows;

gui.cursor_is_valid = FALSE;
gui.scroll_region_top = 0;
gui.scroll_region_bot = Rows - 1;
gui.scroll_region_left = 0;
gui.scroll_region_right = Columns - 1;
gui.highlight_mask = HL_NORMAL;
gui.char_width = 1;
gui.char_height = 1;
gui.char_ascent = 0;
gui.border_width = 0;

gui.norm_font = NOFONT;
#if !defined(FEAT_GUI_GTK)
gui.bold_font = NOFONT;
gui.ital_font = NOFONT;
gui.boldital_font = NOFONT;
#if defined(FEAT_XFONTSET)
gui.fontset = NOFONTSET;
#endif
#endif
gui.wide_font = NOFONT;
#if !defined(FEAT_GUI_GTK)
gui.wide_bold_font = NOFONT;
gui.wide_ital_font = NOFONT;
gui.wide_boldital_font = NOFONT;
#endif

#if defined(FEAT_MENU)
#if !defined(FEAT_GUI_GTK)
#if defined(FONTSET_ALWAYS)
gui.menu_fontset = NOFONTSET;
#else
gui.menu_font = NOFONT;
#endif
#endif
gui.menu_is_active = TRUE; 
#if !defined(FEAT_GUI_GTK)
gui.menu_height = MENU_DEFAULT_HEIGHT;
gui.menu_width = 0;
#endif
#endif
#if defined(FEAT_TOOLBAR) && (defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_HAIKU))

gui.toolbar_height = 0;
#endif
#if defined(FEAT_FOOTER) && defined(FEAT_GUI_MOTIF)
gui.footer_height = 0;
#endif
#if defined(FEAT_BEVAL_TIP)
gui.tooltip_fontset = NOFONTSET;
#endif

gui.scrollbar_width = gui.scrollbar_height = SB_DEFAULT_WIDTH;
gui.prev_wrap = -1;

#if defined(ALWAYS_USE_GUI) || defined(VIMDLL)
result = OK;
#else
#if defined(FEAT_GUI_GTK)






result = gui.dofork ? gui_mch_early_init_check(TRUE) : gui_mch_init_check();
#else
result = gui_mch_init_check();
#endif
#endif
return result;
}




void
gui_init(void)
{
win_T *wp;
static int recursive = 0;







if (!recursive)
{
++recursive;

clip_init(TRUE);


if (gui_init_check() == FAIL)
{
--recursive;
clip_init(FALSE);
return;
}





set_option_value((char_u *)"paste", 0L, NULL, 0);




#if defined(SYS_MENU_FILE) && defined(FEAT_MENU)
if (vim_strchr(p_go, GO_NOSYSMENU) == NULL)
{
sys_menu = TRUE;
do_source((char_u *)SYS_MENU_FILE, FALSE, DOSO_NONE, NULL);
sys_menu = FALSE;
}
#endif





if (!option_was_set((char_u *)"mouse"))
set_string_option_direct((char_u *)"mouse", -1,
(char_u *)"a", OPT_FREE, SID_NONE);





if (use_gvimrc != NULL)
{
if (STRCMP(use_gvimrc, "NONE") != 0
&& STRCMP(use_gvimrc, "NORC") != 0
&& do_source(use_gvimrc, FALSE, DOSO_NONE, NULL) != OK)
semsg(_("E230: Cannot read from \"%s\""), use_gvimrc);
}
else
{



#if defined(SYS_GVIMRC_FILE)
do_source((char_u *)SYS_GVIMRC_FILE, FALSE, DOSO_NONE, NULL);
#endif










if (process_env((char_u *)"GVIMINIT", FALSE) == FAIL
&& do_source((char_u *)USR_GVIMRC_FILE, TRUE,
DOSO_GVIMRC, NULL) == FAIL
#if defined(USR_GVIMRC_FILE2)
&& do_source((char_u *)USR_GVIMRC_FILE2, TRUE,
DOSO_GVIMRC, NULL) == FAIL
#endif
#if defined(USR_GVIMRC_FILE3)
&& do_source((char_u *)USR_GVIMRC_FILE3, TRUE,
DOSO_GVIMRC, NULL) == FAIL
#endif
)
{
#if defined(USR_GVIMRC_FILE4)
(void)do_source((char_u *)USR_GVIMRC_FILE4, TRUE,
DOSO_GVIMRC, NULL);
#endif
}











if (p_exrc)
{
#if defined(UNIX)
{
stat_T s;



if (mch_stat(GVIMRC_FILE, &s) || s.st_uid != getuid())
secure = p_secure;
}
#else
secure = p_secure;
#endif

if ( fullpathcmp((char_u *)USR_GVIMRC_FILE,
(char_u *)GVIMRC_FILE, FALSE, TRUE) != FPC_SAME
#if defined(SYS_GVIMRC_FILE)
&& fullpathcmp((char_u *)SYS_GVIMRC_FILE,
(char_u *)GVIMRC_FILE, FALSE, TRUE) != FPC_SAME
#endif
#if defined(USR_GVIMRC_FILE2)
&& fullpathcmp((char_u *)USR_GVIMRC_FILE2,
(char_u *)GVIMRC_FILE, FALSE, TRUE) != FPC_SAME
#endif
#if defined(USR_GVIMRC_FILE3)
&& fullpathcmp((char_u *)USR_GVIMRC_FILE3,
(char_u *)GVIMRC_FILE, FALSE, TRUE) != FPC_SAME
#endif
#if defined(USR_GVIMRC_FILE4)
&& fullpathcmp((char_u *)USR_GVIMRC_FILE4,
(char_u *)GVIMRC_FILE, FALSE, TRUE) != FPC_SAME
#endif
)
do_source((char_u *)GVIMRC_FILE, TRUE, DOSO_GVIMRC, NULL);

if (secure == 2)
need_wait_return = TRUE;
secure = 0;
}
}

if (need_wait_return || msg_didany)
wait_return(TRUE);

--recursive;
}


if (gui.in_use)
return;




gui.in_use = TRUE; 
if (gui_mch_init() == FAIL)
goto error;



emsg_on_display = FALSE;
msg_scrolled = 0;
clear_sb_text(TRUE);
need_wait_return = FALSE;
msg_didany = FALSE;




if (gui.border_width < 0)
gui.border_width = 0;




if (font_argument != NULL)
set_option_value((char_u *)"gfn", 0L, (char_u *)font_argument, 0);
if (
#if defined(FEAT_XFONTSET)
(*p_guifontset == NUL
|| gui_init_font(p_guifontset, TRUE) == FAIL) &&
#endif
gui_init_font(*p_guifont == NUL ? hl_get_font_name()
: p_guifont, FALSE) == FAIL)
{
emsg(_("E665: Cannot start GUI, no valid font found"));
goto error2;
}
if (gui_get_wide_font() == FAIL)
emsg(_("E231: 'guifontwide' invalid"));

gui.num_cols = Columns;
gui.num_rows = Rows;
gui_reset_scroll_region();


FOR_ALL_WINDOWS(wp)
{
gui_create_scrollbar(&wp->w_scrollbars[SBAR_LEFT], SBAR_LEFT, wp);
gui_create_scrollbar(&wp->w_scrollbars[SBAR_RIGHT], SBAR_RIGHT, wp);
}
gui_create_scrollbar(&gui.bottom_sbar, SBAR_BOTTOM, NULL);

#if defined(FEAT_MENU)
gui_create_initial_menus(root_menu);
#endif
#if defined(FEAT_SIGN_ICONS)
sign_gui_started();
#endif


gui_init_which_components(NULL);


gui.shell_created = TRUE;

#if defined(FEAT_GUI_MSWIN)





if (gui_mch_maximized())
gui_set_shellsize(FALSE, TRUE, RESIZE_BOTH);
else
gui_set_shellsize(TRUE, TRUE, RESIZE_BOTH);
#else
#if !defined(FEAT_GUI_GTK)
gui_set_shellsize(FALSE, TRUE, RESIZE_BOTH);
#endif
#endif
#if defined(FEAT_GUI_MOTIF) && defined(FEAT_MENU)


gui_mch_compute_menu_height((Widget)0);
#endif




if (gui_mch_open() != FAIL)
{
#if defined(FEAT_TITLE)
maketitle();
resettitle();
#endif
init_gui_options();
#if defined(FEAT_ARABIC)

p_tbidi = FALSE;
#endif
#if defined(FEAT_GUI_GTK)

gui_mch_update();

#if defined(FEAT_MENU)


if (vim_strchr(p_go, GO_MENUS) == NULL)
{
--gui.starting;
gui_mch_enable_menu(FALSE);
++gui.starting;
gui_mch_update();
}
#endif


if (gui_mch_maximized())
gui_set_shellsize(FALSE, TRUE, RESIZE_BOTH);
else
gui_set_shellsize(TRUE, TRUE, RESIZE_BOTH);
#endif


win_new_shellsize();

#if defined(FEAT_BEVAL_GUI)


if (balloonEval != NULL)
{
#if defined(FEAT_VARTABS)
vim_free(balloonEval->vts);
#endif
vim_free(balloonEval);
}
balloonEvalForTerm = FALSE;
#if defined(FEAT_GUI_GTK)
balloonEval = gui_mch_create_beval_area(gui.drawarea, NULL,
&general_beval_cb, NULL);
#else
#if defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA)
{
extern Widget textArea;
balloonEval = gui_mch_create_beval_area(textArea, NULL,
&general_beval_cb, NULL);
}
#else
#if defined(FEAT_GUI_MSWIN)
balloonEval = gui_mch_create_beval_area(NULL, NULL,
&general_beval_cb, NULL);
#endif
#endif
#endif
if (!p_beval)
gui_mch_disable_beval_area(balloonEval);
#endif

#if defined(FEAT_XIM) && defined(FEAT_GUI_GTK)
if (!im_xim_isvalid_imactivate())
emsg(_("E599: Value of 'imactivatekey' is invalid"));
#endif


if (p_ch != 1L)
command_height();

return;
}

error2:
#if defined(FEAT_GUI_X11)

gui_mch_uninit();
#endif

error:
gui.in_use = FALSE;
clip_init(FALSE);
}


void
gui_exit(int rc)
{


free_highlight_fonts();
gui.in_use = FALSE;
gui_mch_exit(rc);
}

#if defined(FEAT_GUI_GTK) || defined(FEAT_GUI_X11) || defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_PHOTON) || defined(FEAT_GUI_MAC) || defined(PROTO)

#define NEED_GUI_UPDATE_SCREEN 1






void
gui_shell_closed(void)
{
cmdmod_T save_cmdmod;

save_cmdmod = cmdmod;


exiting = TRUE;
#if defined(FEAT_BROWSE)
cmdmod.browse = TRUE;
#endif
#if defined(FEAT_GUI_DIALOG) || defined(FEAT_CON_DIALOG)
cmdmod.confirm = TRUE;
#endif


if (!check_changed_any(FALSE, FALSE))
getout(0);

exiting = FALSE;
cmdmod = save_cmdmod;
gui_update_screen(); 
}
#endif









int
gui_init_font(char_u *font_list, int fontset UNUSED)
{
#define FONTLEN 320
char_u font_name[FONTLEN];
int font_list_empty = FALSE;
int ret = FAIL;

if (!gui.in_use)
return FAIL;

font_name[0] = NUL;
if (*font_list == NUL)
font_list_empty = TRUE;
else
{
#if defined(FEAT_XFONTSET)

if (fontset)
ret = gui_mch_init_font(font_list, TRUE);
else
#endif
while (*font_list != NUL)
{

(void)copy_option_part(&font_list, font_name, FONTLEN, ",");






if (gui_mch_init_font(font_name, FALSE) == OK)
{
#if !defined(FEAT_GUI_GTK)


if ((p_guifontwide == NULL || *p_guifontwide == NUL)
&& strstr((char *)font_name, "10646") != NULL)
set_guifontwide(font_name);
#endif
ret = OK;
break;
}
}
}

if (ret != OK
&& STRCMP(font_list, "*") != 0
&& (font_list_empty || gui.norm_font == NOFONT))
{





ret = gui_mch_init_font(NULL, FALSE);
}

if (ret == OK)
{
#if !defined(FEAT_GUI_GTK)

#if defined(FEAT_XFONTSET)
if (gui.fontset != NOFONTSET)
gui_mch_set_fontset(gui.fontset);
else
#endif
gui_mch_set_font(gui.norm_font);
#endif
gui_set_shellsize(FALSE, TRUE, RESIZE_BOTH);
}

return ret;
}

#if !defined(FEAT_GUI_GTK)



static void
set_guifontwide(char_u *name)
{
int i = 0;
char_u wide_name[FONTLEN + 10]; 
char_u *wp = NULL;
char_u *p;
GuiFont font;

wp = wide_name;
for (p = name; *p != NUL; ++p)
{
*wp++ = *p;
if (*p == '-')
{
++i;
if (i == 6) 
{
if (p[1] == '-')
*wp++ = '*';
}
else if (i == 12) 
{
++p;
i = getdigits(&p);
if (i != 0)
{

sprintf((char *)wp, "%d%s", i * 2, p);
font = gui_mch_get_font(wide_name, FALSE);
if (font != NOFONT)
{
gui_mch_free_font(gui.wide_font);
gui.wide_font = font;
set_string_option_direct((char_u *)"gfw", -1,
wide_name, OPT_FREE, 0);
}
}
break;
}
}
}
}
#endif 





int
gui_get_wide_font(void)
{
GuiFont font = NOFONT;
char_u font_name[FONTLEN];
char_u *p;

if (!gui.in_use) 
return OK; 

if (p_guifontwide != NULL && *p_guifontwide != NUL)
{
for (p = p_guifontwide; *p != NUL; )
{

(void)copy_option_part(&p, font_name, FONTLEN, ",");
font = gui_mch_get_font(font_name, FALSE);
if (font != NOFONT)
break;
}
if (font == NOFONT)
return FAIL;
}

gui_mch_free_font(gui.wide_font);
#if defined(FEAT_GUI_GTK)

if (font != NOFONT && gui.norm_font != NOFONT
&& pango_font_description_equal(font, gui.norm_font))
{
gui.wide_font = NOFONT;
gui_mch_free_font(font);
}
else
#endif
gui.wide_font = font;
#if defined(FEAT_GUI_MSWIN)
gui_mch_wide_font_changed();
#else




#endif
return OK;
}

static void
gui_set_cursor(int row, int col)
{
gui.row = row;
gui.col = col;
}




static void
gui_check_pos(void)
{
if (gui.row >= screen_Rows)
gui.row = screen_Rows - 1;
if (gui.col >= screen_Columns)
gui.col = screen_Columns - 1;
if (gui.cursor_row >= screen_Rows || gui.cursor_col >= screen_Columns)
gui.cursor_is_valid = FALSE;
}






void
gui_update_cursor(
int force, 
int clear_selection) 
{
int cur_width = 0;
int cur_height = 0;
int old_hl_mask;
cursorentry_T *shape;
int id;
#if defined(FEAT_TERMINAL)
guicolor_T shape_fg = INVALCOLOR;
guicolor_T shape_bg = INVALCOLOR;
#endif
guicolor_T cfg, cbg, cc; 
int cattr; 
int attr;
attrentry_T *aep = NULL;



if (!can_update_cursor || screen_Columns != gui.num_cols
|| screen_Rows != gui.num_rows)
return;

gui_check_pos();
if (!gui.cursor_is_valid || force
|| gui.row != gui.cursor_row || gui.col != gui.cursor_col)
{
gui_undraw_cursor();
if (gui.row < 0)
return;
#if defined(HAVE_INPUT_METHOD)
if (gui.row != gui.cursor_row || gui.col != gui.cursor_col)
im_set_position(gui.row, gui.col);
#endif
gui.cursor_row = gui.row;
gui.cursor_col = gui.col;


if (!screen_cleared || ScreenLines == NULL)
return;


if (clear_selection)
clip_may_clear_selection(gui.row, gui.row);


if (gui.row >= screen_Rows || gui.col >= screen_Columns)
return;

gui.cursor_is_valid = TRUE;





#if defined(FEAT_TERMINAL)
if (terminal_is_active())
shape = term_get_cursor_shape(&shape_fg, &shape_bg);
else
#endif
shape = &shape_table[get_shape_idx(FALSE)];
if (State & LANGMAP)
id = shape->id_lm;
else
id = shape->id;


cfg = INVALCOLOR;
cbg = INVALCOLOR;
cattr = HL_INVERSE;
gui_mch_set_blinking(shape->blinkwait,
shape->blinkon,
shape->blinkoff);
if (shape->blinkwait == 0 || shape->blinkon == 0
|| shape->blinkoff == 0)
gui_mch_stop_blink(FALSE);
#if defined(FEAT_TERMINAL)
if (shape_bg != INVALCOLOR)
{
cattr = 0;
cfg = shape_fg;
cbg = shape_bg;
}
else
#endif
if (id > 0)
{
cattr = syn_id2colors(id, &cfg, &cbg);
#if defined(HAVE_INPUT_METHOD)
{
static int iid;
guicolor_T fg, bg;

if (
#if defined(FEAT_GUI_GTK) && defined(FEAT_XIM)
preedit_get_status()
#else
im_get_status()
#endif
)
{
iid = syn_name2id((char_u *)"CursorIM");
if (iid > 0)
{
syn_id2colors(iid, &fg, &bg);
if (bg != INVALCOLOR)
cbg = bg;
if (fg != INVALCOLOR)
cfg = fg;
}
}
}
#endif
}





attr = ScreenAttrs[LineOffset[gui.row] + gui.col];
if (attr > HL_ALL)
aep = syn_gui_attr2entry(attr);
if (aep != NULL)
{
attr = aep->ae_attr;
if (cfg == INVALCOLOR)
cfg = ((attr & HL_INVERSE) ? aep->ae_u.gui.bg_color
: aep->ae_u.gui.fg_color);
if (cbg == INVALCOLOR)
cbg = ((attr & HL_INVERSE) ? aep->ae_u.gui.fg_color
: aep->ae_u.gui.bg_color);
}
if (cfg == INVALCOLOR)
cfg = (attr & HL_INVERSE) ? gui.back_pixel : gui.norm_pixel;
if (cbg == INVALCOLOR)
cbg = (attr & HL_INVERSE) ? gui.norm_pixel : gui.back_pixel;

#if defined(FEAT_XIM)
if (aep != NULL)
{
xim_bg_color = ((attr & HL_INVERSE) ? aep->ae_u.gui.fg_color
: aep->ae_u.gui.bg_color);
xim_fg_color = ((attr & HL_INVERSE) ? aep->ae_u.gui.bg_color
: aep->ae_u.gui.fg_color);
if (xim_bg_color == INVALCOLOR)
xim_bg_color = (attr & HL_INVERSE) ? gui.norm_pixel
: gui.back_pixel;
if (xim_fg_color == INVALCOLOR)
xim_fg_color = (attr & HL_INVERSE) ? gui.back_pixel
: gui.norm_pixel;
}
else
{
xim_bg_color = (attr & HL_INVERSE) ? gui.norm_pixel
: gui.back_pixel;
xim_fg_color = (attr & HL_INVERSE) ? gui.back_pixel
: gui.norm_pixel;
}
#endif

attr &= ~HL_INVERSE;
if (cattr & HL_INVERSE)
{
cc = cbg;
cbg = cfg;
cfg = cc;
}
cattr &= ~HL_INVERSE;




if (!gui.in_focus)
{
gui_mch_draw_hollow_cursor(cbg);
return;
}

old_hl_mask = gui.highlight_mask;
if (shape->shape == SHAPE_BLOCK)
{




gui.highlight_mask = (cattr | attr);
(void)gui_screenchar(LineOffset[gui.row] + gui.col,
GUI_MON_IS_CURSOR | GUI_MON_NOCLEAR, cfg, cbg, 0);
}
else
{
#if defined(FEAT_RIGHTLEFT)
int col_off = FALSE;
#endif




if (shape->shape == SHAPE_VER)
{
cur_height = gui.char_height;
cur_width = (gui.char_width * shape->percentage + 99) / 100;
}
else
{
cur_height = (gui.char_height * shape->percentage + 99) / 100;
cur_width = gui.char_width;
}
if (has_mbyte && (*mb_off2cells)(LineOffset[gui.row] + gui.col,
LineOffset[gui.row] + screen_Columns) > 1)
{

if (shape->shape != SHAPE_VER)
cur_width += gui.char_width;
#if defined(FEAT_RIGHTLEFT)
if (CURSOR_BAR_RIGHT)
{




col_off = TRUE;
++gui.col;
}
#endif
}
gui_mch_draw_part_cursor(cur_width, cur_height, cbg);
#if defined(FEAT_RIGHTLEFT)
if (col_off)
--gui.col;
#endif

#if !defined(FEAT_GUI_MSWIN)
gui.highlight_mask = ScreenAttrs[LineOffset[gui.row] + gui.col];
(void)gui_screenchar(LineOffset[gui.row] + gui.col,
GUI_MON_TRS_CURSOR | GUI_MON_NOCLEAR,
(guicolor_T)0, (guicolor_T)0, 0);
#endif
}
gui.highlight_mask = old_hl_mask;
}
}

#if defined(FEAT_MENU) || defined(PROTO)
void
gui_position_menu(void)
{
#if !defined(FEAT_GUI_GTK) && !defined(FEAT_GUI_MOTIF)
if (gui.menu_is_active && gui.in_use)
gui_mch_set_menu_pos(0, 0, gui.menu_width, gui.menu_height);
#endif
}
#endif





static void
gui_position_components(int total_width UNUSED)
{
int text_area_x;
int text_area_y;
int text_area_width;
int text_area_height;


++hold_gui_events;

text_area_x = 0;
if (gui.which_scrollbars[SBAR_LEFT])
text_area_x += gui.scrollbar_width;

text_area_y = 0;
#if defined(FEAT_MENU) && !(defined(FEAT_GUI_GTK) || defined(FEAT_GUI_PHOTON))
gui.menu_width = total_width;
if (gui.menu_is_active)
text_area_y += gui.menu_height;
#endif
#if defined(FEAT_TOOLBAR) && defined(FEAT_GUI_MSWIN)
if (vim_strchr(p_go, GO_TOOLBAR) != NULL)
text_area_y = TOOLBAR_BUTTON_HEIGHT + TOOLBAR_BORDER_HEIGHT;
#endif

#if defined(FEAT_GUI_TABLINE) && (defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_MAC))

if (gui_has_tabline())
text_area_y += gui.tabline_height;
#endif

#if defined(FEAT_TOOLBAR) && (defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_HAIKU))

if (vim_strchr(p_go, GO_TOOLBAR) != NULL)
{
#if defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_HAIKU)
gui_mch_set_toolbar_pos(0, text_area_y,
gui.menu_width, gui.toolbar_height);
#endif
text_area_y += gui.toolbar_height;
}
#endif

#if defined(FEAT_GUI_TABLINE) && defined(FEAT_GUI_HAIKU)
gui_mch_set_tabline_pos(0, text_area_y,
gui.menu_width, gui.tabline_height);
if (gui_has_tabline())
text_area_y += gui.tabline_height;
#endif

text_area_width = gui.num_cols * gui.char_width + gui.border_offset * 2;
text_area_height = gui.num_rows * gui.char_height + gui.border_offset * 2;

gui_mch_set_text_area_pos(text_area_x,
text_area_y,
text_area_width,
text_area_height
#if defined(FEAT_XIM) && !defined(FEAT_GUI_GTK)
+ xim_get_status_area_height()
#endif
);
#if defined(FEAT_MENU)
gui_position_menu();
#endif
if (gui.which_scrollbars[SBAR_BOTTOM])
gui_mch_set_scrollbar_pos(&gui.bottom_sbar,
text_area_x,
text_area_y + text_area_height,
text_area_width,
gui.scrollbar_height);
gui.left_sbar_x = 0;
gui.right_sbar_x = text_area_x + text_area_width;

--hold_gui_events;
}




int
gui_get_base_width(void)
{
int base_width;

base_width = 2 * gui.border_offset;
if (gui.which_scrollbars[SBAR_LEFT])
base_width += gui.scrollbar_width;
if (gui.which_scrollbars[SBAR_RIGHT])
base_width += gui.scrollbar_width;
return base_width;
}




int
gui_get_base_height(void)
{
int base_height;

base_height = 2 * gui.border_offset;
if (gui.which_scrollbars[SBAR_BOTTOM])
base_height += gui.scrollbar_height;
#if defined(FEAT_GUI_GTK)



#else
#if defined(FEAT_MENU)
if (gui.menu_is_active)
base_height += gui.menu_height;
#endif
#if defined(FEAT_TOOLBAR)
if (vim_strchr(p_go, GO_TOOLBAR) != NULL)
#if defined(FEAT_GUI_MSWIN) && defined(FEAT_TOOLBAR)
base_height += (TOOLBAR_BUTTON_HEIGHT + TOOLBAR_BORDER_HEIGHT);
#else
base_height += gui.toolbar_height;
#endif
#endif
#if defined(FEAT_GUI_TABLINE) && (defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_HAIKU))

if (gui_has_tabline())
base_height += gui.tabline_height;
#endif
#if defined(FEAT_FOOTER)
if (vim_strchr(p_go, GO_FOOTER) != NULL)
base_height += gui.footer_height;
#endif
#if defined(FEAT_GUI_MOTIF) && defined(FEAT_MENU)
base_height += gui_mch_text_area_extra_height();
#endif
#endif
return base_height;
}





void
gui_resize_shell(int pixel_width, int pixel_height)
{
static int busy = FALSE;

if (!gui.shell_created) 
return;





if (updating_screen || busy)
{
new_pixel_width = pixel_width;
new_pixel_height = pixel_height;
return;
}

again:
new_pixel_width = 0;
new_pixel_height = 0;
busy = TRUE;

#if defined(FEAT_GUI_HAIKU)
vim_lock_screen();
#endif


out_flush();

gui.num_cols = (pixel_width - gui_get_base_width()) / gui.char_width;
gui.num_rows = (pixel_height - gui_get_base_height()) / gui.char_height;

gui_position_components(pixel_width);
gui_reset_scroll_region();





if (State == ASKMORE || State == CONFIRM)
gui.row = gui.num_rows;



if (gui.num_rows != screen_Rows || gui.num_cols != screen_Columns
|| gui.num_rows != Rows || gui.num_cols != Columns)
shell_resized();

#if defined(FEAT_GUI_HAIKU)
vim_unlock_screen();
#endif

gui_update_scrollbars(TRUE);
gui_update_cursor(FALSE, TRUE);
#if defined(FEAT_XIM) && !defined(FEAT_GUI_GTK)
xim_set_status_area();
#endif

busy = FALSE;




if (new_pixel_height)
{
if (pixel_width == new_pixel_width && pixel_height == new_pixel_height)
{
new_pixel_width = 0;
new_pixel_height = 0;
}
else
{
pixel_width = new_pixel_width;
pixel_height = new_pixel_height;
goto again;
}
}
}




void
gui_may_resize_shell(void)
{
if (new_pixel_height)


gui_resize_shell(new_pixel_width, new_pixel_height);
}

int
gui_get_shellsize(void)
{
Rows = gui.num_rows;
Columns = gui.num_cols;
return OK;
}








void
gui_set_shellsize(
int mustset UNUSED,
int fit_to_display,
int direction) 
{
int base_width;
int base_height;
int width;
int height;
int min_width;
int min_height;
int screen_w;
int screen_h;
#if defined(FEAT_GUI_GTK)
int un_maximize = mustset;
int did_adjust = 0;
#endif
int x = -1, y = -1;

if (!gui.shell_created)
return;

#if defined(MSWIN) || defined(FEAT_GUI_GTK)


if (!mustset && (vim_strchr(p_go, GO_KEEPWINSIZE) != NULL
|| gui_mch_maximized()))
{
gui_mch_newfont();
return;
}
#endif

base_width = gui_get_base_width();
base_height = gui_get_base_height();
if (fit_to_display)

(void)gui_mch_get_winpos(&x, &y);

width = Columns * gui.char_width + base_width;
height = Rows * gui.char_height + base_height;

if (fit_to_display)
{
gui_mch_get_screen_dimensions(&screen_w, &screen_h);
if ((direction & RESIZE_HOR) && width > screen_w)
{
Columns = (screen_w - base_width) / gui.char_width;
if (Columns < MIN_COLUMNS)
Columns = MIN_COLUMNS;
width = Columns * gui.char_width + base_width;
#if defined(FEAT_GUI_GTK)
++did_adjust;
#endif
}
if ((direction & RESIZE_VERT) && height > screen_h)
{
Rows = (screen_h - base_height) / gui.char_height;
check_shellsize();
height = Rows * gui.char_height + base_height;
#if defined(FEAT_GUI_GTK)
++did_adjust;
#endif
}
#if defined(FEAT_GUI_GTK)
if (did_adjust == 2 || (width + gui.char_width >= screen_w
&& height + gui.char_height >= screen_h))

un_maximize = FALSE;
#endif
}
limit_screen_size();
gui.num_cols = Columns;
gui.num_rows = Rows;

min_width = base_width + MIN_COLUMNS * gui.char_width;
min_height = base_height + MIN_LINES * gui.char_height;
min_height += tabline_height() * gui.char_height;

#if defined(FEAT_GUI_GTK)
if (un_maximize)
{


gui_mch_get_screen_dimensions(&screen_w, &screen_h);
if ((width + gui.char_width < screen_w
|| height + gui.char_height * 2 < screen_h)
&& gui_mch_maximized())
gui_mch_unmaximize();
}
#endif

gui_mch_set_shellsize(width, height, min_width, min_height,
base_width, base_height, direction);

if (fit_to_display && x >= 0 && y >= 0)
{



gui_mch_update();
if (gui_mch_get_winpos(&x, &y) == OK && (x < 0 || y < 0))
gui_mch_set_winpos(x < 0 ? 0 : x, y < 0 ? 0 : y);
}

gui_position_components(width);
gui_update_scrollbars(TRUE);
gui_reset_scroll_region();
}




void
gui_new_shellsize(void)
{
gui_reset_scroll_region();
}




static void
gui_reset_scroll_region(void)
{
gui.scroll_region_top = 0;
gui.scroll_region_bot = gui.num_rows - 1;
gui.scroll_region_left = 0;
gui.scroll_region_right = gui.num_cols - 1;
}

static void
gui_start_highlight(int mask)
{
if (mask > HL_ALL) 
gui.highlight_mask = mask;
else 
gui.highlight_mask |= mask;
}

void
gui_stop_highlight(int mask)
{
if (mask > HL_ALL) 
gui.highlight_mask = HL_NORMAL;
else 
gui.highlight_mask &= ~mask;
}





void
gui_clear_block(
int row1,
int col1,
int row2,
int col2)
{

clip_may_clear_selection(row1, row2);

gui_mch_clear_block(row1, col1, row2, col2);


if ( gui.cursor_row >= row1 && gui.cursor_row <= row2
&& gui.cursor_col >= col1 && gui.cursor_col <= col2)
gui.cursor_is_valid = FALSE;
}





void
gui_update_cursor_later(void)
{
OUT_STR(IF_EB("\033|s", ESC_STR "|s"));
}

void
gui_write(
char_u *s,
int len)
{
char_u *p;
int arg1 = 0, arg2 = 0;
int force_cursor = FALSE; 
int force_scrollbar = FALSE;
static win_T *old_curwin = NULL;


#if defined(DEBUG_GUI_WRITE)
{
int i;
char_u *str;

printf("gui_write(%d):\n ", len);
for (i = 0; i < len; i++)
if (s[i] == ESC)
{
if (i != 0)
printf("\n ");
printf("<ESC>");
}
else
{
str = transchar_byte(s[i]);
if (str[0] && str[1])
printf("<%s>", (char *)str);
else
printf("%s", (char *)str);
}
printf("\n");
}
#endif
while (len)
{
if (s[0] == ESC && s[1] == '|')
{
p = s + 2;
if (VIM_ISDIGIT(*p) || (*p == '-' && VIM_ISDIGIT(*(p + 1))))
{
arg1 = getdigits(&p);
if (p > s + len)
break;
if (*p == ';')
{
++p;
arg2 = getdigits(&p);
if (p > s + len)
break;
}
}
switch (*p)
{
case 'C': 
clip_scroll_selection(9999);
gui_mch_clear_all();
gui.cursor_is_valid = FALSE;
force_scrollbar = TRUE;
break;
case 'M': 
gui_set_cursor(arg1, arg2);
break;
case 's': 
force_cursor = TRUE;
break;
case 'R': 
if (arg1 < arg2)
{
gui.scroll_region_top = arg1;
gui.scroll_region_bot = arg2;
}
else
{
gui.scroll_region_top = arg2;
gui.scroll_region_bot = arg1;
}
break;
case 'V': 
if (arg1 < arg2)
{
gui.scroll_region_left = arg1;
gui.scroll_region_right = arg2;
}
else
{
gui.scroll_region_left = arg2;
gui.scroll_region_right = arg1;
}
break;
case 'd': 
gui_delete_lines(gui.row, 1);
break;
case 'D': 
gui_delete_lines(gui.row, arg1);
break;
case 'i': 
gui_insert_lines(gui.row, 1);
break;
case 'I': 
gui_insert_lines(gui.row, arg1);
break;
case '$': 
gui_clear_block(gui.row, gui.col, gui.row,
(int)Columns - 1);
break;
case 'h': 
gui_start_highlight(arg1);
break;
case 'H': 
gui_stop_highlight(arg1);
break;
case 'f': 
gui_mch_flash(arg1 == 0 ? 20 : arg1);
break;
default:
p = s + 1; 
break;
}
len -= (int)(++p - s);
s = p;
}
else if (
#if defined(EBCDIC)
CtrlChar(s[0]) != 0 
#else
s[0] < 0x20 
#endif
#if defined(FEAT_SIGN_ICONS)
&& s[0] != SIGN_BYTE
#if defined(FEAT_NETBEANS_INTG)
&& s[0] != MULTISIGN_BYTE
#endif
#endif
)
{
if (s[0] == '\n') 
{
gui.col = 0;
if (gui.row < gui.scroll_region_bot)
gui.row++;
else
gui_delete_lines(gui.scroll_region_top, 1);
}
else if (s[0] == '\r') 
{
gui.col = 0;
}
else if (s[0] == '\b') 
{
if (gui.col)
--gui.col;
}
else if (s[0] == Ctrl_L) 
{
++gui.col;
}
else if (s[0] == Ctrl_G) 
{
gui_mch_beep();
}


--len; 
++s;
}
else
{
p = s;
while (len > 0 && (
#if defined(EBCDIC)
CtrlChar(*p) == 0
#else
*p >= 0x20
#endif
#if defined(FEAT_SIGN_ICONS)
|| *p == SIGN_BYTE
#if defined(FEAT_NETBEANS_INTG)
|| *p == MULTISIGN_BYTE
#endif
#endif
))
{
len--;
p++;
}
gui_outstr(s, (int)(p - s));
s = p;
}
}



if (force_cursor)
gui_update_cursor(TRUE, TRUE);



if (old_curwin != curwin)
gui.dragged_sb = SBAR_NONE;





if (force_scrollbar || old_curwin != curwin)
gui_update_scrollbars(force_scrollbar);
else
gui_update_horiz_scrollbar(FALSE);
old_curwin = curwin;





#if defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_GTK)
gui.dragged_sb = SBAR_NONE;
#endif

gui_may_flush(); 
}






void
gui_dont_update_cursor(int undraw)
{
if (gui.in_use)
{

if (undraw)
gui_undraw_cursor();
can_update_cursor = FALSE;
}
}

void
gui_can_update_cursor(void)
{
can_update_cursor = TRUE;


}




void
gui_disable_flush(void)
{
++disable_flush;
}




void
gui_enable_flush(void)
{
--disable_flush;
}




void
gui_may_flush(void)
{
if (disable_flush == 0)
gui_mch_flush();
}

static void
gui_outstr(char_u *s, int len)
{
int this_len;
int cells;

if (len == 0)
return;

if (len < 0)
len = (int)STRLEN(s);

while (len > 0)
{
if (has_mbyte)
{

cells = 0;
for (this_len = 0; this_len < len; )
{
cells += (*mb_ptr2cells)(s + this_len);
if (gui.col + cells > Columns)
break;
this_len += (*mb_ptr2len)(s + this_len);
}
if (this_len > len)
this_len = len; 
}
else
if (gui.col + len > Columns)
this_len = Columns - gui.col;
else
this_len = len;

(void)gui_outstr_nowrap(s, this_len,
0, (guicolor_T)0, (guicolor_T)0, 0);
s += this_len;
len -= this_len;

if (len > 0 && gui.col < Columns)
(void)gui_outstr_nowrap((char_u *)" ", 1,
0, (guicolor_T)0, (guicolor_T)0, 0);

if (gui.col >= Columns)
{
gui.col = 0;
gui.row++;
}
}
}






static int
gui_screenchar(
int off, 
int flags,
guicolor_T fg, 
guicolor_T bg, 
int back) 
{
char_u buf[MB_MAXBYTES + 1];


if (enc_utf8 && ScreenLines[off] == 0)
return OK;

if (enc_utf8 && ScreenLinesUC[off] != 0)

return gui_outstr_nowrap(buf, utfc_char2bytes(off, buf),
flags, fg, bg, back);

if (enc_dbcs == DBCS_JPNU && ScreenLines[off] == 0x8e)
{
buf[0] = ScreenLines[off];
buf[1] = ScreenLines2[off];
return gui_outstr_nowrap(buf, 2, flags, fg, bg, back);
}


return gui_outstr_nowrap(ScreenLines + off,
enc_dbcs ? (*mb_ptr2len)(ScreenLines + off) : 1,
flags, fg, bg, back);
}

#if defined(FEAT_GUI_GTK)





static int
gui_screenstr(
int off, 
int len, 
int flags,
guicolor_T fg, 
guicolor_T bg, 
int back) 
{
char_u *buf;
int outlen = 0;
int i;
int retval;

if (len <= 0) 
return OK;

if (enc_utf8)
{
buf = alloc(len * MB_MAXBYTES + 1);
if (buf == NULL)
return OK; 

for (i = off; i < off + len; ++i)
{
if (ScreenLines[i] == 0)
continue; 

if (ScreenLinesUC[i] == 0)
buf[outlen++] = ScreenLines[i];
else
outlen += utfc_char2bytes(i, buf + outlen);
}

buf[outlen] = NUL; 
retval = gui_outstr_nowrap(buf, outlen, flags, fg, bg, back);
vim_free(buf);

return retval;
}
else if (enc_dbcs == DBCS_JPNU)
{
buf = alloc(len * 2 + 1);
if (buf == NULL)
return OK; 

for (i = off; i < off + len; ++i)
{
buf[outlen++] = ScreenLines[i];


if (ScreenLines[i] == 0x8e)
buf[outlen++] = ScreenLines2[i];
else if (MB_BYTE2LEN(ScreenLines[i]) == 2)
buf[outlen++] = ScreenLines[++i];
}

buf[outlen] = NUL; 
retval = gui_outstr_nowrap(buf, outlen, flags, fg, bg, back);
vim_free(buf);

return retval;
}
else
{
return gui_outstr_nowrap(&ScreenLines[off], len,
flags, fg, bg, back);
}
}
#endif 














static int
gui_outstr_nowrap(
char_u *s,
int len,
int flags,
guicolor_T fg, 
guicolor_T bg, 
int back) 
{
long_u highlight_mask;
long_u hl_mask_todo;
guicolor_T fg_color;
guicolor_T bg_color;
guicolor_T sp_color;
#if !defined(FEAT_GUI_GTK)
GuiFont font = NOFONT;
GuiFont wide_font = NOFONT;
#if defined(FEAT_XFONTSET)
GuiFontset fontset = NOFONTSET;
#endif
#endif
attrentry_T *aep = NULL;
int draw_flags;
int col = gui.col;
#if defined(FEAT_SIGN_ICONS)
int draw_sign = FALSE;
int signcol = 0;
char_u extra[18];
#if defined(FEAT_NETBEANS_INTG)
int multi_sign = FALSE;
#endif
#endif

if (len < 0)
len = (int)STRLEN(s);
if (len == 0)
return OK;

#if defined(FEAT_SIGN_ICONS)
if (*s == SIGN_BYTE
#if defined(FEAT_NETBEANS_INTG)
|| *s == MULTISIGN_BYTE
#endif
)
{
#if defined(FEAT_NETBEANS_INTG)
if (*s == MULTISIGN_BYTE)
multi_sign = TRUE;
#endif

if (*curwin->w_p_scl == 'n' && *(curwin->w_p_scl + 1) == 'u' &&
(curwin->w_p_nu || curwin->w_p_rnu))
{
sprintf((char *)extra, "%*c ", number_width(curwin), ' ');
s = extra;
}
else
s = (char_u *)" ";
if (len == 1 && col > 0)
--col;
len = (int)STRLEN(s);
if (len > 2)

signcol = col + len - 3;
else
signcol = col;
draw_sign = TRUE;
highlight_mask = 0;
}
else
#endif
if (gui.highlight_mask > HL_ALL)
{
aep = syn_gui_attr2entry(gui.highlight_mask);
if (aep == NULL) 
highlight_mask = 0;
else
highlight_mask = aep->ae_attr;
}
else
highlight_mask = gui.highlight_mask;
hl_mask_todo = highlight_mask;

#if !defined(FEAT_GUI_GTK)

if (aep != NULL && aep->ae_u.gui.font != NOFONT)
font = aep->ae_u.gui.font;
#if defined(FEAT_XFONTSET)
else if (aep != NULL && aep->ae_u.gui.fontset != NOFONTSET)
fontset = aep->ae_u.gui.fontset;
#endif
else
{
#if defined(FEAT_XFONTSET)
if (gui.fontset != NOFONTSET)
fontset = gui.fontset;
else
#endif
if (hl_mask_todo & (HL_BOLD | HL_STANDOUT))
{
if ((hl_mask_todo & HL_ITALIC) && gui.boldital_font != NOFONT)
{
font = gui.boldital_font;
hl_mask_todo &= ~(HL_BOLD | HL_STANDOUT | HL_ITALIC);
}
else if (gui.bold_font != NOFONT)
{
font = gui.bold_font;
hl_mask_todo &= ~(HL_BOLD | HL_STANDOUT);
}
else
font = gui.norm_font;
}
else if ((hl_mask_todo & HL_ITALIC) && gui.ital_font != NOFONT)
{
font = gui.ital_font;
hl_mask_todo &= ~HL_ITALIC;
}
else
font = gui.norm_font;






if (font == gui.boldital_font && gui.wide_boldital_font)
wide_font = gui.wide_boldital_font;
else if (font == gui.bold_font && gui.wide_bold_font)
wide_font = gui.wide_bold_font;
else if (font == gui.ital_font && gui.wide_ital_font)
wide_font = gui.wide_ital_font;
else if (font == gui.norm_font && gui.wide_font)
wide_font = gui.wide_font;
}
#if defined(FEAT_XFONTSET)
if (fontset != NOFONTSET)
gui_mch_set_fontset(fontset);
else
#endif
gui_mch_set_font(font);
#endif

draw_flags = 0;


bg_color = gui.back_pixel;
if ((flags & GUI_MON_IS_CURSOR) && gui.in_focus)
{
draw_flags |= DRAW_CURSOR;
fg_color = fg;
bg_color = bg;
sp_color = fg;
}
else if (aep != NULL)
{
fg_color = aep->ae_u.gui.fg_color;
if (fg_color == INVALCOLOR)
fg_color = gui.norm_pixel;
bg_color = aep->ae_u.gui.bg_color;
if (bg_color == INVALCOLOR)
bg_color = gui.back_pixel;
sp_color = aep->ae_u.gui.sp_color;
if (sp_color == INVALCOLOR)
sp_color = fg_color;
}
else
{
fg_color = gui.norm_pixel;
sp_color = fg_color;
}

if (highlight_mask & (HL_INVERSE | HL_STANDOUT))
{
#if defined(AMIGA)
gui_mch_set_colors(bg_color, fg_color);
#else
gui_mch_set_fg_color(bg_color);
gui_mch_set_bg_color(fg_color);
#endif
}
else
{
#if defined(AMIGA)
gui_mch_set_colors(fg_color, bg_color);
#else
gui_mch_set_fg_color(fg_color);
gui_mch_set_bg_color(bg_color);
#endif
}
gui_mch_set_sp_color(sp_color);


if (!(flags & GUI_MON_NOCLEAR))
clip_may_clear_selection(gui.row, gui.row);



if (hl_mask_todo & (HL_BOLD | HL_STANDOUT))
draw_flags |= DRAW_BOLD;






if (back != 0 && ((draw_flags & DRAW_BOLD) || (highlight_mask & HL_ITALIC)))
return FAIL;

#if defined(FEAT_GUI_GTK)


if (hl_mask_todo & HL_ITALIC)
draw_flags |= DRAW_ITALIC;


if (hl_mask_todo & HL_UNDERLINE)
draw_flags |= DRAW_UNDERL;

#else

if ((hl_mask_todo & HL_UNDERLINE) || (hl_mask_todo & HL_ITALIC))
draw_flags |= DRAW_UNDERL;
#endif

if (hl_mask_todo & HL_UNDERCURL)
draw_flags |= DRAW_UNDERC;


if (hl_mask_todo & HL_STRIKETHROUGH)
draw_flags |= DRAW_STRIKE;


if (flags & GUI_MON_TRS_CURSOR)
draw_flags |= DRAW_TRANSP;




#if defined(FEAT_GUI_GTK)

len = gui_gtk2_draw_string(gui.row, col, s, len, draw_flags);
#else
if (enc_utf8)
{
int start; 
int cells; 
int thislen; 
int cn; 
int i; 
int c; 
int cl; 
int comping; 
int scol = col; 
int curr_wide = FALSE; 
int prev_wide = FALSE;
int wide_changed;
#if defined(MSWIN)
int sep_comp = FALSE; 
#else
int sep_comp = TRUE; 
#endif



start = 0;
cells = 0;
for (i = 0; i < len; i += cl)
{
c = utf_ptr2char(s + i);
cn = utf_char2cells(c);
comping = utf_iscomposing(c);
if (!comping) 
cells += cn;
if (!comping || sep_comp)
{
if (cn > 1
#if defined(FEAT_XFONTSET)
&& fontset == NOFONTSET
#endif
&& wide_font != NOFONT)
curr_wide = TRUE;
else
curr_wide = FALSE;
}
cl = utf_ptr2len(s + i);
if (cl == 0) 
len = i + cl; 

wide_changed = curr_wide != prev_wide;



if (i + cl >= len || (comping && sep_comp && i > start)
|| wide_changed
#if defined(FEAT_GUI_X11)
|| (cn > 1
#if defined(FEAT_XFONTSET)


&& fontset == NOFONTSET
#endif
)
#endif
)
{
if ((comping && sep_comp) || wide_changed)
thislen = i - start;
else
thislen = i - start + cl;
if (thislen > 0)
{
if (prev_wide)
gui_mch_set_font(wide_font);
gui_mch_draw_string(gui.row, scol, s + start, thislen,
draw_flags);
if (prev_wide)
gui_mch_set_font(font);
start += thislen;
}
scol += cells;
cells = 0;


if (wide_changed && !(comping && sep_comp))
{
scol -= cn;
cl = 0;
}

#if defined(FEAT_GUI_X11)


if (cn > 1 && (draw_flags & DRAW_TRANSP) == 0
#if defined(FEAT_XFONTSET)
&& fontset == NOFONTSET
#endif
&& !wide_changed)
gui_mch_draw_string(gui.row, scol - 1, (char_u *)" ",
1, draw_flags);
#endif
}

if (comping && sep_comp)
{
#if defined(__APPLE_CC__) && TARGET_API_MAC_CARBON

gui_mch_draw_string(gui.row, scol, s + i, cl,
draw_flags | DRAW_TRANSP);
#else
gui_mch_draw_string(gui.row, scol - cn, s + i, cl,
draw_flags | DRAW_TRANSP);
#endif
start = i + cl;
}
prev_wide = curr_wide;
}

len = scol - col;
}
else
{
gui_mch_draw_string(gui.row, col, s, len, draw_flags);
if (enc_dbcs == DBCS_JPNU)
{


len = mb_string2cells(s, len);
}
}
#endif 

if (!(flags & (GUI_MON_IS_CURSOR | GUI_MON_TRS_CURSOR)))
gui.col = col + len;


if (flags & GUI_MON_NOCLEAR)
clip_may_redraw_selection(gui.row, col, len);

if (!(flags & (GUI_MON_IS_CURSOR | GUI_MON_TRS_CURSOR)))
{

if (gui.cursor_row == gui.row
&& gui.cursor_col >= col
&& gui.cursor_col < col + len)
gui.cursor_is_valid = FALSE;
}

#if defined(FEAT_SIGN_ICONS)
if (draw_sign)

gui_mch_drawsign(gui.row, signcol, gui.highlight_mask);
#if defined(FEAT_NETBEANS_INTG) && (defined(FEAT_GUI_X11) || defined(FEAT_GUI_GTK) || defined(FEAT_GUI_MSWIN))

if (multi_sign)
netbeans_draw_multisign_indicator(gui.row);
#endif
#endif

return OK;
}





void
gui_undraw_cursor(void)
{
if (gui.cursor_is_valid)
{


gui_redraw_block(gui.cursor_row,
gui.cursor_col > 0 ? gui.cursor_col - 1 : gui.cursor_col,
gui.cursor_row, gui.cursor_col, GUI_MON_NOCLEAR);



gui.cursor_is_valid = FALSE;
}
}

void
gui_redraw(
int x,
int y,
int w,
int h)
{
int row1, col1, row2, col2;

row1 = Y_2_ROW(y);
col1 = X_2_COL(x);
row2 = Y_2_ROW(y + h - 1);
col2 = X_2_COL(x + w - 1);

gui_redraw_block(row1, col1, row2, col2, GUI_MON_NOCLEAR);








if (gui.row == gui.cursor_row)
gui_update_cursor(TRUE, TRUE);
}





void
gui_redraw_block(
int row1,
int col1,
int row2,
int col2,
int flags) 
{
int old_row, old_col;
long_u old_hl_mask;
int off;
sattr_T first_attr;
int idx, len;
int back, nback;
int orig_col1, orig_col2;


if (!screen_cleared || ScreenLines == NULL)
return;



col1 = check_col(col1);
col2 = check_col(col2);
row1 = check_row(row1);
row2 = check_row(row2);


old_row = gui.row;
old_col = gui.col;
old_hl_mask = gui.highlight_mask;
orig_col1 = col1;
orig_col2 = col2;

for (gui.row = row1; gui.row <= row2; gui.row++)
{


col1 = orig_col1;
col2 = orig_col2;
off = LineOffset[gui.row];
if (enc_dbcs != 0)
{
if (col1 > 0)
col1 -= dbcs_screen_head_off(ScreenLines + off,
ScreenLines + off + col1);
col2 += dbcs_screen_tail_off(ScreenLines + off,
ScreenLines + off + col2);
}
else if (enc_utf8)
{
if (ScreenLines[off + col1] == 0)
{
if (col1 > 0)
--col1;
else
{


vim_snprintf((char *)IObuff, IOSIZE,
"INTERNAL ERROR: NUL in ScreenLines in row %ld",
(long)gui.row);
msg((char *)IObuff);
}
}
#if defined(FEAT_GUI_GTK)
if (col2 + 1 < Columns && ScreenLines[off + col2 + 1] == 0)
++col2;
#endif
}
gui.col = col1;
off = LineOffset[gui.row] + gui.col;
len = col2 - col1 + 1;



for (back = 0; back < col1; ++back)
if (ScreenAttrs[off - 1 - back] != ScreenAttrs[off]
|| ScreenLines[off - 1 - back] == ' ')
break;



while (len > 0)
{
first_attr = ScreenAttrs[off];
gui.highlight_mask = first_attr;
#if !defined(FEAT_GUI_GTK)
if (enc_utf8 && ScreenLinesUC[off] != 0)
{

nback = gui_screenchar(off, flags,
(guicolor_T)0, (guicolor_T)0, back);
if (gui.col < Columns && ScreenLines[off + 1] == 0)
idx = 2;
else
idx = 1;
}
else if (enc_dbcs == DBCS_JPNU && ScreenLines[off] == 0x8e)
{

nback = gui_screenchar(off, flags,
(guicolor_T)0, (guicolor_T)0, back);
idx = 1;
}
else
#endif
{
#if defined(FEAT_GUI_GTK)
for (idx = 0; idx < len; ++idx)
{
if (enc_utf8 && ScreenLines[off + idx] == 0)
continue; 
if (ScreenAttrs[off + idx] != first_attr)
break;
}

nback = gui_screenstr(off, idx, flags,
(guicolor_T)0, (guicolor_T)0, back);
#else
for (idx = 0; idx < len && ScreenAttrs[off + idx] == first_attr;
idx++)
{

if (enc_utf8 && ScreenLinesUC[off + idx] != 0)
break;
if (enc_dbcs == DBCS_JPNU)
{

if (ScreenLines[off + idx] == 0x8e)
break;
if (len > 1 && (*mb_ptr2len)(ScreenLines
+ off + idx) == 2)
++idx; 
}
}
nback = gui_outstr_nowrap(ScreenLines + off, idx, flags,
(guicolor_T)0, (guicolor_T)0, back);
#endif
}
if (nback == FAIL)
{


off -= back;
len += back;
gui.col -= back;
}
else
{
off += idx;
len -= idx;
}
back = 0;
}
}


gui.row = old_row;
gui.col = old_col;
gui.highlight_mask = (int)old_hl_mask;
}

static void
gui_delete_lines(int row, int count)
{
if (count <= 0)
return;

if (row + count > gui.scroll_region_bot)

gui_clear_block(row, gui.scroll_region_left,
gui.scroll_region_bot, gui.scroll_region_right);
else
{
gui_mch_delete_lines(row, count);



if (gui.cursor_row >= row
&& gui.cursor_col >= gui.scroll_region_left
&& gui.cursor_col <= gui.scroll_region_right)
{
if (gui.cursor_row < row + count)
gui.cursor_is_valid = FALSE;
else if (gui.cursor_row <= gui.scroll_region_bot)
gui.cursor_row -= count;
}
}
}

static void
gui_insert_lines(int row, int count)
{
if (count <= 0)
return;

if (row + count > gui.scroll_region_bot)

gui_clear_block(row, gui.scroll_region_left,
gui.scroll_region_bot, gui.scroll_region_right);
else
{
gui_mch_insert_lines(row, count);

if (gui.cursor_row >= gui.row
&& gui.cursor_col >= gui.scroll_region_left
&& gui.cursor_col <= gui.scroll_region_right)
{
if (gui.cursor_row <= gui.scroll_region_bot - count)
gui.cursor_row += count;
else if (gui.cursor_row <= gui.scroll_region_bot)
gui.cursor_is_valid = FALSE;
}
}
}

#if defined(FEAT_TIMERS)



static int
gui_wait_for_chars_3(
long wtime,
int *interrupted UNUSED,
int ignore_input UNUSED)
{
return gui_mch_wait_for_chars(wtime);
}
#endif





static int
gui_wait_for_chars_or_timer(
long wtime,
int *interrupted UNUSED,
int ignore_input UNUSED)
{
#if defined(FEAT_TIMERS)
return ui_wait_for_chars_or_timer(wtime, gui_wait_for_chars_3,
interrupted, ignore_input);
#else
return gui_mch_wait_for_chars(wtime);
#endif
}











static int
gui_wait_for_chars_buf(
char_u *buf,
int maxlen,
long wtime, 
int tb_change_cnt)
{
int retval;

#if defined(FEAT_MENU)


if (wtime != 0)
gui_update_menus(0);
#endif

gui_mch_update();
if (input_available()) 
{
if (buf != NULL && !typebuf_changed(tb_change_cnt))
return read_from_input_buf(buf, (long)maxlen);
return 0;
}
if (wtime == 0) 
return FAIL;


gui_mch_flush();


gui_mch_start_blink();



retval = inchar_loop(buf, maxlen, wtime, tb_change_cnt,
gui_wait_for_chars_or_timer, NULL);

gui_mch_stop_blink(TRUE);

return retval;
}










int
gui_wait_for_chars(long wtime, int tb_change_cnt)
{
return gui_wait_for_chars_buf(NULL, 0, wtime, tb_change_cnt);
}




int
gui_inchar(
char_u *buf,
int maxlen,
long wtime, 
int tb_change_cnt)
{
return gui_wait_for_chars_buf(buf, maxlen, wtime, tb_change_cnt);
}




static void
fill_mouse_coord(char_u *p, int col, int row)
{
p[0] = (char_u)(col / 128 + ' ' + 1);
p[1] = (char_u)(col % 128 + ' ' + 1);
p[2] = (char_u)(row / 128 + ' ' + 1);
p[3] = (char_u)(row % 128 + ' ' + 1);
}

















void
gui_send_mouse_event(
int button,
int x,
int y,
int repeated_click,
int_u modifiers)
{
static int prev_row = 0, prev_col = 0;
static int prev_button = -1;
static int num_clicks = 1;
char_u string[10];
enum key_extra button_char;
int row, col;
#if defined(FEAT_CLIPBOARD)
int checkfor;
int did_clip = FALSE;
#endif




switch (button)
{
case MOUSE_X1:
button_char = KE_X1MOUSE;
goto button_set;
case MOUSE_X2:
button_char = KE_X2MOUSE;
goto button_set;
case MOUSE_4:
button_char = KE_MOUSEDOWN;
goto button_set;
case MOUSE_5:
button_char = KE_MOUSEUP;
goto button_set;
case MOUSE_6:
button_char = KE_MOUSELEFT;
goto button_set;
case MOUSE_7:
button_char = KE_MOUSERIGHT;
button_set:
{

if (hold_gui_events)
return;

string[3] = CSI;
string[4] = KS_EXTRA;
string[5] = (int)button_char;



row = gui_xy2colrow(x, y, &col);
string[6] = (char_u)(col / 128 + ' ' + 1);
string[7] = (char_u)(col % 128 + ' ' + 1);
string[8] = (char_u)(row / 128 + ' ' + 1);
string[9] = (char_u)(row % 128 + ' ' + 1);

if (modifiers == 0)
add_to_input_buf(string + 3, 7);
else
{
string[0] = CSI;
string[1] = KS_MODIFIER;
string[2] = 0;
if (modifiers & MOUSE_SHIFT)
string[2] |= MOD_MASK_SHIFT;
if (modifiers & MOUSE_CTRL)
string[2] |= MOD_MASK_CTRL;
if (modifiers & MOUSE_ALT)
string[2] |= MOD_MASK_ALT;
add_to_input_buf(string, 10);
}
return;
}
}

#if defined(FEAT_CLIPBOARD)

if (clip_star.state == SELECT_IN_PROGRESS)
{
clip_process_selection(button, X_2_COL(x), Y_2_ROW(y), repeated_click);


row = gui_xy2colrow(x, y, &col);
if (button != MOUSE_RELEASE || row != prev_row || col != prev_col)
return;
}


switch (get_real_state())
{
case NORMAL_BUSY:
case OP_PENDING:
#if defined(FEAT_TERMINAL)
case TERMINAL:
#endif
case NORMAL: checkfor = MOUSE_NORMAL; break;
case VISUAL: checkfor = MOUSE_VISUAL; break;
case SELECTMODE: checkfor = MOUSE_VISUAL; break;
case REPLACE:
case REPLACE+LANGMAP:
case VREPLACE:
case VREPLACE+LANGMAP:
case INSERT:
case INSERT+LANGMAP: checkfor = MOUSE_INSERT; break;
case ASKMORE:
case HITRETURN: 


if (Y_2_ROW(y) >= msg_row)
checkfor = MOUSE_NORMAL;
else
checkfor = MOUSE_RETURN;
break;





case CMDLINE:
case CMDLINE+LANGMAP:
if (Y_2_ROW(y) < cmdline_row && button != MOUSE_MIDDLE)
checkfor = MOUSE_NONE;
else
checkfor = MOUSE_COMMAND;
break;

default:
checkfor = MOUSE_NONE;
break;
};






if ((State == NORMAL || State == NORMAL_BUSY || (State & INSERT))
&& Y_2_ROW(y) >= topframe->fr_height + firstwin->w_winrow
&& button != MOUSE_DRAG
#if defined(FEAT_MOUSESHAPE)
&& !drag_status_line
&& !drag_sep_line
#endif
)
checkfor = MOUSE_NONE;




if ((modifiers & MOUSE_CTRL) && (modifiers & MOUSE_SHIFT))
checkfor = MOUSE_NONEF;




if (exmode_active)
checkfor = MOUSE_NONE;








if (!mouse_has(checkfor) || checkfor == MOUSE_COMMAND)
{

if (checkfor != MOUSE_NONEF && VIsual_active && (State & NORMAL))
return;





if (mouse_model_popup() && button == MOUSE_LEFT
&& (modifiers & MOUSE_SHIFT) && !(modifiers & MOUSE_CTRL))
{
button = MOUSE_RIGHT;
modifiers &= ~ MOUSE_SHIFT;
}




if (button == MOUSE_RIGHT)
{
if (clip_star.state == SELECT_CLEARED)
{
if (State & CMDLINE)
{
col = msg_col;
row = msg_row;
}
else
{
col = curwin->w_wcol;
row = curwin->w_wrow + W_WINROW(curwin);
}
clip_start_selection(col, row, FALSE);
}
clip_process_selection(button, X_2_COL(x), Y_2_ROW(y),
repeated_click);
did_clip = TRUE;
}

else if (button == MOUSE_LEFT)
{
clip_start_selection(X_2_COL(x), Y_2_ROW(y), repeated_click);
did_clip = TRUE;
}


if (button != MOUSE_MIDDLE)
{
if (!mouse_has(checkfor) || button == MOUSE_RELEASE)
return;
if (checkfor != MOUSE_COMMAND)
button = MOUSE_LEFT;
}
repeated_click = FALSE;
}

if (clip_star.state != SELECT_CLEARED && !did_clip)
clip_clear_selection(&clip_star);
#endif


if (hold_gui_events)
return;

row = gui_xy2colrow(x, y, &col);





if (button == MOUSE_DRAG)
{
if (row == prev_row && col == prev_col)
return;

if (y < 0)
row = -1;
}






if (curwin->w_topline != gui_prev_topline
#if defined(FEAT_DIFF)
|| curwin->w_topfill != gui_prev_topfill
#endif
)
repeated_click = FALSE;

string[0] = CSI; 
string[1] = KS_MOUSE;
string[2] = KE_FILLER;
if (button != MOUSE_DRAG && button != MOUSE_RELEASE)
{
if (repeated_click)
{




if (button != prev_button || row != prev_row || col != prev_col)
num_clicks = 1;
else if (++num_clicks > 4)
num_clicks = 1;
}
else
num_clicks = 1;
prev_button = button;
gui_prev_topline = curwin->w_topline;
#if defined(FEAT_DIFF)
gui_prev_topfill = curwin->w_topfill;
#endif

string[3] = (char_u)(button | 0x20);
SET_NUM_MOUSE_CLICKS(string[3], num_clicks);
}
else
string[3] = (char_u)button;

string[3] |= modifiers;
fill_mouse_coord(string + 4, col, row);
add_to_input_buf(string, 8);

if (row < 0)
prev_row = 0;
else
prev_row = row;
prev_col = col;





#if defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_GTK)
gui.dragged_sb = SBAR_NONE;
#endif
}






static int
gui_xy2colrow(int x, int y, int *colp)
{
int col = check_col(X_2_COL(x));
int row = check_row(Y_2_ROW(y));

*colp = mb_fix_col(col, row);
return row;
}

#if defined(FEAT_MENU) || defined(PROTO)



void
gui_menu_cb(vimmenu_T *menu)
{
char_u bytes[sizeof(long_u)];


if (hold_gui_events)
return;

bytes[0] = CSI;
bytes[1] = KS_MENU;
bytes[2] = KE_FILLER;
add_to_input_buf(bytes, 3);
add_long_to_buf((long_u)menu, bytes);
add_to_input_buf_csi(bytes, sizeof(long_u));
}
#endif

static int prev_which_scrollbars[3];






void
gui_init_which_components(char_u *oldval UNUSED)
{
#if defined(FEAT_GUI_DARKTHEME)
static int prev_dark_theme = -1;
int using_dark_theme = FALSE;
#endif
#if defined(FEAT_MENU)
static int prev_menu_is_active = -1;
#endif
#if defined(FEAT_TOOLBAR)
static int prev_toolbar = -1;
int using_toolbar = FALSE;
#endif
#if defined(FEAT_GUI_TABLINE)
int using_tabline;
#endif
#if defined(FEAT_FOOTER)
static int prev_footer = -1;
int using_footer = FALSE;
#endif
#if defined(FEAT_MENU)
static int prev_tearoff = -1;
int using_tearoff = FALSE;
#endif

char_u *p;
int i;
#if defined(FEAT_MENU)
int grey_old, grey_new;
char_u *temp;
#endif
win_T *wp;
int need_set_size;
int fix_size;

#if defined(FEAT_MENU)
if (oldval != NULL && gui.in_use)
{



grey_old = (vim_strchr(oldval, GO_GREY) != NULL);
grey_new = (vim_strchr(p_go, GO_GREY) != NULL);
if (grey_old != grey_new)
{
temp = p_go;
p_go = oldval;
gui_update_menus(MENU_ALL_MODES);
p_go = temp;
}
}
gui.menu_is_active = FALSE;
#endif

for (i = 0; i < 3; i++)
gui.which_scrollbars[i] = FALSE;
for (p = p_go; *p; p++)
switch (*p)
{
case GO_LEFT:
gui.which_scrollbars[SBAR_LEFT] = TRUE;
break;
case GO_RIGHT:
gui.which_scrollbars[SBAR_RIGHT] = TRUE;
break;
case GO_VLEFT:
if (win_hasvertsplit())
gui.which_scrollbars[SBAR_LEFT] = TRUE;
break;
case GO_VRIGHT:
if (win_hasvertsplit())
gui.which_scrollbars[SBAR_RIGHT] = TRUE;
break;
case GO_BOT:
gui.which_scrollbars[SBAR_BOTTOM] = TRUE;
break;
#if defined(FEAT_GUI_DARKTHEME)
case GO_DARKTHEME:
using_dark_theme = TRUE;
break;
#endif
#if defined(FEAT_MENU)
case GO_MENUS:
gui.menu_is_active = TRUE;
break;
#endif
case GO_GREY:

break;
#if defined(FEAT_TOOLBAR)
case GO_TOOLBAR:
using_toolbar = TRUE;
break;
#endif
#if defined(FEAT_FOOTER)
case GO_FOOTER:
using_footer = TRUE;
break;
#endif
case GO_TEAROFF:
#if defined(FEAT_MENU)
using_tearoff = TRUE;
#endif
break;
default:

break;
}

if (gui.in_use)
{
need_set_size = 0;
fix_size = FALSE;

#if defined(FEAT_GUI_DARKTHEME)
if (using_dark_theme != prev_dark_theme)
{
gui_mch_set_dark_theme(using_dark_theme);
prev_dark_theme = using_dark_theme;
}
#endif

#if defined(FEAT_GUI_TABLINE)


using_tabline = gui_has_tabline();
if (!gui_mch_showing_tabline() != !using_tabline)
{


i = Rows;
gui_update_tabline();
Rows = i;
need_set_size |= RESIZE_VERT;
if (using_tabline)
fix_size = TRUE;
if (!gui_use_tabline())
redraw_tabline = TRUE; 
}
#endif

for (i = 0; i < 3; i++)
{




if (gui.which_scrollbars[i] != prev_which_scrollbars[i]
|| gui.which_scrollbars[i]
!= curtab->tp_prev_which_scrollbars[i])
{
if (i == SBAR_BOTTOM)
gui_mch_enable_scrollbar(&gui.bottom_sbar,
gui.which_scrollbars[i]);
else
{
FOR_ALL_WINDOWS(wp)
gui_do_scrollbar(wp, i, gui.which_scrollbars[i]);
}
if (gui.which_scrollbars[i] != prev_which_scrollbars[i])
{
if (i == SBAR_BOTTOM)
need_set_size |= RESIZE_VERT;
else
need_set_size |= RESIZE_HOR;
if (gui.which_scrollbars[i])
fix_size = TRUE;
}
}
curtab->tp_prev_which_scrollbars[i] = gui.which_scrollbars[i];
prev_which_scrollbars[i] = gui.which_scrollbars[i];
}

#if defined(FEAT_MENU)
if (gui.menu_is_active != prev_menu_is_active)
{


i = Rows;
gui_mch_enable_menu(gui.menu_is_active);
Rows = i;
prev_menu_is_active = gui.menu_is_active;
need_set_size |= RESIZE_VERT;
if (gui.menu_is_active)
fix_size = TRUE;
}
#endif

#if defined(FEAT_TOOLBAR)
if (using_toolbar != prev_toolbar)
{
gui_mch_show_toolbar(using_toolbar);
prev_toolbar = using_toolbar;
need_set_size |= RESIZE_VERT;
if (using_toolbar)
fix_size = TRUE;
}
#endif
#if defined(FEAT_FOOTER)
if (using_footer != prev_footer)
{
gui_mch_enable_footer(using_footer);
prev_footer = using_footer;
need_set_size |= RESIZE_VERT;
if (using_footer)
fix_size = TRUE;
}
#endif
#if defined(FEAT_MENU) && !(defined(MSWIN) && !defined(FEAT_TEAROFF))
if (using_tearoff != prev_tearoff)
{
gui_mch_toggle_tearoffs(using_tearoff);
prev_tearoff = using_tearoff;
}
#endif
if (need_set_size != 0)
{
#if defined(FEAT_GUI_GTK)
long prev_Columns = Columns;
long prev_Rows = Rows;
#endif



gui_set_shellsize(FALSE, fix_size, need_set_size);

#if defined(FEAT_GUI_GTK)










if (!gui.starting && need_set_size != (RESIZE_VERT | RESIZE_HOR))
(void)char_avail();
if ((need_set_size & RESIZE_VERT) == 0)
Rows = prev_Rows;
if ((need_set_size & RESIZE_HOR) == 0)
Columns = prev_Columns;
#endif
}


if (firstwin->w_winrow != tabline_height())
shell_new_rows(); 
}
}

#if defined(FEAT_GUI_TABLINE) || defined(PROTO)




int
gui_use_tabline(void)
{
return gui.in_use && vim_strchr(p_go, GO_TABLINE) != NULL;
}





static int
gui_has_tabline(void)
{
if (!gui_use_tabline()
|| p_stal == 0
|| (p_stal == 1 && first_tabpage->tp_next == NULL))
return FALSE;
return TRUE;
}





void
gui_update_tabline(void)
{
int showit = gui_has_tabline();
int shown = gui_mch_showing_tabline();

if (!gui.starting && starting == 0)
{


out_flush();

if (!showit != !shown)
gui_mch_show_tabline(showit);
if (showit != 0)
gui_mch_update_tabline();



if (!showit != !shown)
gui_set_shellsize(FALSE, showit, RESIZE_VERT);
}
}




void
get_tabline_label(
tabpage_T *tp,
int tooltip) 
{
int modified = FALSE;
char_u buf[40];
int wincount;
win_T *wp;
char_u **opt;


opt = (tooltip ? &p_gtt : &p_gtl);
if (**opt != NUL)
{
int use_sandbox = FALSE;
int called_emsg_before = called_emsg;
char_u res[MAXPATHL];
tabpage_T *save_curtab;
char_u *opt_name = (char_u *)(tooltip ? "guitabtooltip"
: "guitablabel");

printer_page_num = tabpage_index(tp);
#if defined(FEAT_EVAL)
set_vim_var_nr(VV_LNUM, printer_page_num);
use_sandbox = was_set_insecurely(opt_name, 0);
#endif

curtab->tp_firstwin = firstwin;
curtab->tp_lastwin = lastwin;
curtab->tp_curwin = curwin;
save_curtab = curtab;
curtab = tp;
topframe = curtab->tp_topframe;
firstwin = curtab->tp_firstwin;
lastwin = curtab->tp_lastwin;
curwin = curtab->tp_curwin;
curbuf = curwin->w_buffer;


build_stl_str_hl(curwin, res, MAXPATHL, *opt, use_sandbox,
0, (int)Columns, NULL, NULL);
STRCPY(NameBuff, res);


curtab = save_curtab;
topframe = curtab->tp_topframe;
firstwin = curtab->tp_firstwin;
lastwin = curtab->tp_lastwin;
curwin = curtab->tp_curwin;
curbuf = curwin->w_buffer;

if (called_emsg > called_emsg_before)
set_string_option_direct(opt_name, -1,
(char_u *)"", OPT_FREE, SID_ERROR);
}



if (**opt == NUL || *NameBuff == NUL)
{

get_trans_bufname(tp == curtab ? curbuf : tp->tp_curwin->w_buffer);
if (!tooltip)
shorten_dir(NameBuff);

wp = (tp == curtab) ? firstwin : tp->tp_firstwin;
for (wincount = 0; wp != NULL; wp = wp->w_next, ++wincount)
if (bufIsChanged(wp->w_buffer))
modified = TRUE;
if (modified || wincount > 1)
{
if (wincount > 1)
vim_snprintf((char *)buf, sizeof(buf), "%d", wincount);
else
buf[0] = NUL;
if (modified)
STRCAT(buf, "+");
STRCAT(buf, " ");
STRMOVE(NameBuff + STRLEN(buf), NameBuff);
mch_memmove(NameBuff, buf, STRLEN(buf));
}
}
}






int
send_tabline_event(int nr)
{
char_u string[3];

if (nr == tabpage_index(curtab))
return FALSE;


if (hold_gui_events
#if defined(FEAT_CMDWIN)
|| cmdwin_type != 0
#endif
)
{

gui_mch_set_curtab(tabpage_index(curtab));
return FALSE;
}

string[0] = CSI;
string[1] = KS_TABLINE;
string[2] = KE_FILLER;
add_to_input_buf(string, 3);
string[0] = nr;
add_to_input_buf_csi(string, 1);
return TRUE;
}




void
send_tabline_menu_event(int tabidx, int event)
{
char_u string[3];


if (hold_gui_events)
return;


if (event == TABLINE_MENU_CLOSE && first_tabpage->tp_next == NULL)
return;

string[0] = CSI;
string[1] = KS_TABMENU;
string[2] = KE_FILLER;
add_to_input_buf(string, 3);
string[0] = tabidx;
string[1] = (char_u)(long)event;
add_to_input_buf_csi(string, 2);
}

#endif








void
gui_remove_scrollbars(void)
{
int i;
win_T *wp;

for (i = 0; i < 3; i++)
{
if (i == SBAR_BOTTOM)
gui_mch_enable_scrollbar(&gui.bottom_sbar, FALSE);
else
{
FOR_ALL_WINDOWS(wp)
gui_do_scrollbar(wp, i, FALSE);
}
curtab->tp_prev_which_scrollbars[i] = -1;
}
}

void
gui_create_scrollbar(scrollbar_T *sb, int type, win_T *wp)
{
static int sbar_ident = 0;

sb->ident = sbar_ident++; 
sb->wp = wp;
sb->type = type;
sb->value = 0;
#if defined(FEAT_GUI_ATHENA)
sb->pixval = 0;
#endif
sb->size = 1;
sb->max = 1;
sb->top = 0;
sb->height = 0;
sb->width = 0;
sb->status_height = 0;
gui_mch_create_scrollbar(sb, (wp == NULL) ? SBAR_HORIZ : SBAR_VERT);
}




scrollbar_T *
gui_find_scrollbar(long ident)
{
win_T *wp;

if (gui.bottom_sbar.ident == ident)
return &gui.bottom_sbar;
FOR_ALL_WINDOWS(wp)
{
if (wp->w_scrollbars[SBAR_LEFT].ident == ident)
return &wp->w_scrollbars[SBAR_LEFT];
if (wp->w_scrollbars[SBAR_RIGHT].ident == ident)
return &wp->w_scrollbars[SBAR_RIGHT];
}
return NULL;
}
















void
gui_drag_scrollbar(scrollbar_T *sb, long value, int still_dragging)
{
win_T *wp;
int sb_num;
#if defined(USE_ON_FLY_SCROLL)
colnr_T old_leftcol = curwin->w_leftcol;
linenr_T old_topline = curwin->w_topline;
#if defined(FEAT_DIFF)
int old_topfill = curwin->w_topfill;
#endif
#else
char_u bytes[sizeof(long_u)];
int byte_count;
#endif

if (sb == NULL)
return;


if (hold_gui_events)
return;

#if defined(FEAT_CMDWIN)
if (cmdwin_type != 0 && sb->wp != curwin)
return;
#endif

if (still_dragging)
{
if (sb->wp == NULL)
gui.dragged_sb = SBAR_BOTTOM;
else if (sb == &sb->wp->w_scrollbars[SBAR_LEFT])
gui.dragged_sb = SBAR_LEFT;
else
gui.dragged_sb = SBAR_RIGHT;
gui.dragged_wp = sb->wp;
}
else
{
gui.dragged_sb = SBAR_NONE;
#if defined(FEAT_GUI_GTK)


gui.dragged_wp = NULL;
#endif
}


if (sb->wp != NULL)
sb = &sb->wp->w_scrollbars[0];




if (value < 0)
value = 0;
#if defined(SCROLL_PAST_END)
else if (value > sb->max)
value = sb->max;
#else
if (value > sb->max - sb->size + 1)
value = sb->max - sb->size + 1;
#endif

sb->value = value;

#if defined(USE_ON_FLY_SCROLL)



if (dont_scroll)
return;
#endif


if ((sb->wp == NULL || sb->wp == curwin) && pum_visible())
return;

#if defined(FEAT_RIGHTLEFT)
if (sb->wp == NULL && curwin->w_p_rl)
{
value = sb->max + 1 - sb->size - value;
if (value < 0)
value = 0;
}
#endif

if (sb->wp != NULL) 
{
sb_num = 0;
for (wp = firstwin; wp != sb->wp && wp != NULL; wp = wp->w_next)
sb_num++;
if (wp == NULL)
return;

#if defined(USE_ON_FLY_SCROLL)
current_scrollbar = sb_num;
scrollbar_value = value;
if (State & NORMAL)
{
gui_do_scroll();
setcursor();
}
else if (State & INSERT)
{
ins_scroll();
setcursor();
}
else if (State & CMDLINE)
{
if (msg_scrolled == 0)
{
gui_do_scroll();
redrawcmdline();
}
}
#if defined(FEAT_FOLDING)

sb->value = sb->wp->w_topline - 1;
#endif



if (gui.which_scrollbars[SBAR_RIGHT] && gui.which_scrollbars[SBAR_LEFT])
gui_mch_set_scrollbar_thumb(
&sb->wp->w_scrollbars[
sb == &sb->wp->w_scrollbars[SBAR_RIGHT]
? SBAR_LEFT : SBAR_RIGHT],
sb->value, sb->size, sb->max);

#else
bytes[0] = CSI;
bytes[1] = KS_VER_SCROLLBAR;
bytes[2] = KE_FILLER;
bytes[3] = (char_u)sb_num;
byte_count = 4;
#endif
}
else
{
#if defined(USE_ON_FLY_SCROLL)
scrollbar_value = value;

if (State & NORMAL)
gui_do_horiz_scroll(scrollbar_value, FALSE);
else if (State & INSERT)
ins_horscroll();
else if (State & CMDLINE)
{
if (msg_scrolled == 0)
{
gui_do_horiz_scroll(scrollbar_value, FALSE);
redrawcmdline();
}
}
if (old_leftcol != curwin->w_leftcol)
{
updateWindow(curwin); 
setcursor();
}
#else
bytes[0] = CSI;
bytes[1] = KS_HOR_SCROLLBAR;
bytes[2] = KE_FILLER;
byte_count = 3;
#endif
}

#if defined(USE_ON_FLY_SCROLL)



if (curwin->w_p_scb
&& ((sb->wp == NULL && curwin->w_leftcol != old_leftcol)
|| (sb->wp == curwin && (curwin->w_topline != old_topline
#if defined(FEAT_DIFF)
|| curwin->w_topfill != old_topfill
#endif
))))
{
do_check_scrollbind(TRUE);

FOR_ALL_WINDOWS(wp)
if (wp->w_redr_type > 0)
updateWindow(wp);
setcursor();
}
out_flush_cursor(FALSE, TRUE);
#else
add_to_input_buf(bytes, byte_count);
add_long_to_buf((long_u)value, bytes);
add_to_input_buf_csi(bytes, sizeof(long_u));
#endif
}








void
gui_may_update_scrollbars(void)
{
if (gui.in_use && starting == 0)
{
out_flush();
gui_init_which_components(NULL);
gui_update_scrollbars(TRUE);
}
need_mouse_correct = TRUE;
}

void
gui_update_scrollbars(
int force) 
{
win_T *wp;
scrollbar_T *sb;
long val, size, max; 
int which_sb;
int h, y;
static win_T *prev_curwin = NULL;


gui_update_horiz_scrollbar(force);

#if !defined(MSWIN)


if (!gui.which_scrollbars[SBAR_LEFT] && !gui.which_scrollbars[SBAR_RIGHT])
return;
#endif






if (!force && (gui.dragged_sb == SBAR_LEFT || gui.dragged_sb == SBAR_RIGHT)
&& gui.which_scrollbars[SBAR_LEFT]
&& gui.which_scrollbars[SBAR_RIGHT])
{




which_sb = SBAR_LEFT + SBAR_RIGHT - gui.dragged_sb;
if (gui.dragged_wp != NULL)
gui_mch_set_scrollbar_thumb(
&gui.dragged_wp->w_scrollbars[which_sb],
gui.dragged_wp->w_scrollbars[0].value,
gui.dragged_wp->w_scrollbars[0].size,
gui.dragged_wp->w_scrollbars[0].max);
}


++hold_gui_events;

FOR_ALL_WINDOWS(wp)
{
if (wp->w_buffer == NULL) 
continue;

if (!force && (gui.dragged_sb == SBAR_LEFT
|| gui.dragged_sb == SBAR_RIGHT)
&& gui.dragged_wp == wp)
continue;

#if defined(SCROLL_PAST_END)
max = wp->w_buffer->b_ml.ml_line_count - 1;
#else
max = wp->w_buffer->b_ml.ml_line_count + wp->w_height - 2;
#endif
if (max < 0) 
max = 0;
val = wp->w_topline - 1;
size = wp->w_height;
#if defined(SCROLL_PAST_END)
if (val > max) 
val = max;
#else
if (size > max + 1) 
size = max + 1;
if (val > max - size + 1)
val = max - size + 1;
#endif
if (val < 0) 
val = 0;






sb = &wp->w_scrollbars[0];





if (size < 1 || wp->w_botline - 2 > max)
{




sb->height = 0; 
if (gui.which_scrollbars[SBAR_LEFT])
gui_do_scrollbar(wp, SBAR_LEFT, FALSE);
if (gui.which_scrollbars[SBAR_RIGHT])
gui_do_scrollbar(wp, SBAR_RIGHT, FALSE);
continue;
}
if (force || sb->height != wp->w_height
|| sb->top != wp->w_winrow
|| sb->status_height != wp->w_status_height
|| sb->width != wp->w_width
|| prev_curwin != curwin)
{


sb->height = wp->w_height;
sb->top = wp->w_winrow;
sb->status_height = wp->w_status_height;
sb->width = wp->w_width;


h = (sb->height + sb->status_height) * gui.char_height;
y = sb->top * gui.char_height + gui.border_offset;
#if defined(FEAT_MENU) && !defined(FEAT_GUI_GTK) && !defined(FEAT_GUI_MOTIF) && !defined(FEAT_GUI_PHOTON)
if (gui.menu_is_active)
y += gui.menu_height;
#endif

#if defined(FEAT_TOOLBAR) && (defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_HAIKU))

if (vim_strchr(p_go, GO_TOOLBAR) != NULL)
#if defined(FEAT_GUI_ATHENA) || defined(FEAT_GUI_HAIKU)
y += gui.toolbar_height;
#else
#if defined(FEAT_GUI_MSWIN)
y += TOOLBAR_BUTTON_HEIGHT + TOOLBAR_BORDER_HEIGHT;
#endif
#endif
#endif

#if defined(FEAT_GUI_TABLINE) && defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_HAIKU)
if (gui_has_tabline())
y += gui.tabline_height;
#endif

if (wp->w_winrow == 0)
{

h += gui.border_offset;
y -= gui.border_offset;
}
if (gui.which_scrollbars[SBAR_LEFT])
{
gui_mch_set_scrollbar_pos(&wp->w_scrollbars[SBAR_LEFT],
gui.left_sbar_x, y,
gui.scrollbar_width, h);
gui_do_scrollbar(wp, SBAR_LEFT, TRUE);
}
if (gui.which_scrollbars[SBAR_RIGHT])
{
gui_mch_set_scrollbar_pos(&wp->w_scrollbars[SBAR_RIGHT],
gui.right_sbar_x, y,
gui.scrollbar_width, h);
gui_do_scrollbar(wp, SBAR_RIGHT, TRUE);
}
}





#if defined(FEAT_GUI_ATHENA)
if (max == 0)
y = 0;
else
y = (val * (sb->height + 2) * gui.char_height + max / 2) / max;
if (force || sb->pixval != y || sb->size != size || sb->max != max)
#else
if (force || sb->value != val || sb->size != size || sb->max != max)
#endif
{

sb->value = val;
#if defined(FEAT_GUI_ATHENA)
sb->pixval = y;
#endif
sb->size = size;
sb->max = max;
if (gui.which_scrollbars[SBAR_LEFT]
&& (gui.dragged_sb != SBAR_LEFT || gui.dragged_wp != wp))
gui_mch_set_scrollbar_thumb(&wp->w_scrollbars[SBAR_LEFT],
val, size, max);
if (gui.which_scrollbars[SBAR_RIGHT]
&& (gui.dragged_sb != SBAR_RIGHT || gui.dragged_wp != wp))
gui_mch_set_scrollbar_thumb(&wp->w_scrollbars[SBAR_RIGHT],
val, size, max);
}
}
prev_curwin = curwin;
--hold_gui_events;
}






static void
gui_do_scrollbar(
win_T *wp,
int which, 
int enable) 
{
int midcol = curwin->w_wincol + curwin->w_width / 2;
int has_midcol = (wp->w_wincol <= midcol
&& wp->w_wincol + wp->w_width >= midcol);



if (gui.which_scrollbars[SBAR_RIGHT] != gui.which_scrollbars[SBAR_LEFT])
{


if (!has_midcol)
enable = FALSE;
}
else
{



if (midcol > Columns / 2)
{
if (which == SBAR_LEFT ? wp->w_wincol != 0 : !has_midcol)
enable = FALSE;
}
else
{
if (which == SBAR_RIGHT ? wp->w_wincol + wp->w_width != Columns
: !has_midcol)
enable = FALSE;
}
}
gui_mch_enable_scrollbar(&wp->w_scrollbars[which], enable);
}






int
gui_do_scroll(void)
{
win_T *wp, *save_wp;
int i;
long nlines;
pos_T old_cursor;
linenr_T old_topline;
#if defined(FEAT_DIFF)
int old_topfill;
#endif

for (wp = firstwin, i = 0; i < current_scrollbar; wp = W_NEXT(wp), i++)
if (wp == NULL)
break;
if (wp == NULL)

return FALSE;




nlines = (long)scrollbar_value + 1 - (long)wp->w_topline;
if (nlines == 0)
return FALSE;

save_wp = curwin;
old_topline = wp->w_topline;
#if defined(FEAT_DIFF)
old_topfill = wp->w_topfill;
#endif
old_cursor = wp->w_cursor;
curwin = wp;
curbuf = wp->w_buffer;
if (nlines < 0)
scrolldown(-nlines, gui.dragged_wp == NULL);
else
scrollup(nlines, gui.dragged_wp == NULL);



if (gui.dragged_sb == SBAR_NONE)
gui.dragged_wp = NULL;

if (old_topline != wp->w_topline
#if defined(FEAT_DIFF)
|| old_topfill != wp->w_topfill
#endif
)
{
if (get_scrolloff_value() != 0)
{
cursor_correct(); 
update_topline(); 
}
if (old_cursor.lnum != wp->w_cursor.lnum)
coladvance(wp->w_curswant);
wp->w_scbind_pos = wp->w_topline;
}


validate_cursor();

curwin = save_wp;
curbuf = save_wp->w_buffer;





if (old_topline != wp->w_topline
|| wp->w_redr_type != 0
#if defined(FEAT_DIFF)
|| old_topfill != wp->w_topfill
#endif
)
{
int type = VALID;

if (pum_visible())
{
type = NOT_VALID;
wp->w_lines_valid = 0;
}



if (wp->w_redr_type < type)
wp->w_redr_type = type;
mch_disable_flush();
updateWindow(wp); 
mch_enable_flush();
}


if (pum_visible())
pum_redraw();

return (wp == curwin && !EQUAL_POS(curwin->w_cursor, old_cursor));
}









static colnr_T
scroll_line_len(linenr_T lnum)
{
char_u *p;
colnr_T col;
int w;

p = ml_get(lnum);
col = 0;
if (*p != NUL)
for (;;)
{
w = chartabsize(p, col);
MB_PTR_ADV(p);
if (*p == NUL) 
break;
col += w;
}
return col;
}



static linenr_T longest_lnum = 0;





static linenr_T
gui_find_longest_lnum(void)
{
linenr_T ret = 0;




if (vim_strchr(p_go, GO_HORSCROLL) == NULL
&& curwin->w_topline <= curwin->w_cursor.lnum
&& curwin->w_botline > curwin->w_cursor.lnum
&& curwin->w_botline <= curbuf->b_ml.ml_line_count + 1)
{
linenr_T lnum;
colnr_T n;
long max = 0;




for (lnum = curwin->w_topline; lnum < curwin->w_botline; ++lnum)
{
n = scroll_line_len(lnum);
if (n > (colnr_T)max)
{
max = n;
ret = lnum;
}
else if (n == (colnr_T)max
&& abs((int)(lnum - curwin->w_cursor.lnum))
< abs((int)(ret - curwin->w_cursor.lnum)))
ret = lnum;
}
}
else

ret = curwin->w_cursor.lnum;

return ret;
}

static void
gui_update_horiz_scrollbar(int force)
{
long value, size, max; 

if (!gui.which_scrollbars[SBAR_BOTTOM])
return;

if (!force && gui.dragged_sb == SBAR_BOTTOM)
return;

if (!force && curwin->w_p_wrap && gui.prev_wrap)
return;





if (curwin->w_cursor.lnum > curbuf->b_ml.ml_line_count)
{
gui.bottom_sbar.value = -1;
return;
}

size = curwin->w_width;
if (curwin->w_p_wrap)
{
value = 0;
#if defined(SCROLL_PAST_END)
max = 0;
#else
max = curwin->w_width - 1;
#endif
}
else
{
value = curwin->w_leftcol;

longest_lnum = gui_find_longest_lnum();
max = scroll_line_len(longest_lnum);

if (virtual_active())
{

if (curwin->w_virtcol >= (colnr_T)max)
max = curwin->w_virtcol;
}

#if !defined(SCROLL_PAST_END)
max += curwin->w_width - 1;
#endif


size -= curwin_col_off();
#if !defined(SCROLL_PAST_END)
max -= curwin_col_off();
#endif
}

#if !defined(SCROLL_PAST_END)
if (value > max - size + 1)
value = max - size + 1; 
#endif

#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl)
{
value = max + 1 - size - value;
if (value < 0)
{
size += value;
value = 0;
}
}
#endif
if (!force && value == gui.bottom_sbar.value && size == gui.bottom_sbar.size
&& max == gui.bottom_sbar.max)
return;

gui.bottom_sbar.value = value;
gui.bottom_sbar.size = size;
gui.bottom_sbar.max = max;
gui.prev_wrap = curwin->w_p_wrap;

gui_mch_set_scrollbar_thumb(&gui.bottom_sbar, value, size, max);
}




int
gui_do_horiz_scroll(long_u leftcol, int compute_longest_lnum)
{

if (curwin->w_p_wrap)
return FALSE;

if (curwin->w_leftcol == (colnr_T)leftcol)
return FALSE;

curwin->w_leftcol = (colnr_T)leftcol;



if (vim_strchr(p_go, GO_HORSCROLL) == NULL
&& !virtual_active()
&& (colnr_T)leftcol > scroll_line_len(curwin->w_cursor.lnum))
{
if (compute_longest_lnum)
{
curwin->w_cursor.lnum = gui_find_longest_lnum();
curwin->w_cursor.col = 0;
}

else if (longest_lnum >= curwin->w_topline
&& longest_lnum < curwin->w_botline)
{
curwin->w_cursor.lnum = longest_lnum;
curwin->w_cursor.col = 0;
}
}

return leftcol_changed();
}




void
gui_check_colors(void)
{
if (gui.norm_pixel == gui.back_pixel || gui.norm_pixel == INVALCOLOR)
{
gui_set_bg_color((char_u *)"White");
if (gui.norm_pixel == gui.back_pixel || gui.norm_pixel == INVALCOLOR)
gui_set_fg_color((char_u *)"Black");
}
}

static void
gui_set_fg_color(char_u *name)
{
gui.norm_pixel = gui_get_color(name);
hl_set_fg_color_name(vim_strsave(name));
}

static void
gui_set_bg_color(char_u *name)
{
gui.back_pixel = gui_get_color(name);
hl_set_bg_color_name(vim_strsave(name));
}





guicolor_T
gui_get_color(char_u *name)
{
guicolor_T t;

if (*name == NUL)
return INVALCOLOR;
t = gui_mch_get_color(name);

if (t == INVALCOLOR
#if defined(FEAT_GUI_X11) || defined(FEAT_GUI_GTK)
&& gui.in_use
#endif
)
semsg(_("E254: Cannot allocate color %s"), name);
return t;
}




int
gui_get_lightness(guicolor_T pixel)
{
long_u rgb = (long_u)gui_mch_get_rgb(pixel);

return (int)( (((rgb >> 16) & 0xff) * 299)
+ (((rgb >> 8) & 0xff) * 587)
+ ((rgb & 0xff) * 114)) / 1000;
}

char_u *
gui_bg_default(void)
{
if (gui_get_lightness(gui.back_pixel) < 127)
return (char_u *)"dark";
return (char_u *)"light";
}




void
init_gui_options(void)
{


if (!option_was_set((char_u *)"bg") && STRCMP(p_bg, gui_bg_default()) != 0)
{
set_option_value((char_u *)"bg", 0L, gui_bg_default(), 0);
highlight_changed();
}
}

#if defined(FEAT_GUI_X11) || defined(PROTO)
void
gui_new_scrollbar_colors(void)
{
win_T *wp;


if (!gui.in_use)
return;

FOR_ALL_WINDOWS(wp)
{
gui_mch_set_scrollbar_colors(&(wp->w_scrollbars[SBAR_LEFT]));
gui_mch_set_scrollbar_colors(&(wp->w_scrollbars[SBAR_RIGHT]));
}
gui_mch_set_scrollbar_colors(&gui.bottom_sbar);
}
#endif




void
gui_focus_change(int in_focus)
{




#if 1
gui.in_focus = in_focus;
out_flush_cursor(TRUE, FALSE);

#if defined(FEAT_XIM)
xim_set_focus(in_focus);
#endif




if (!hold_gui_events)
{
char_u bytes[3];

bytes[0] = CSI;
bytes[1] = KS_EXTRA;
bytes[2] = in_focus ? (int)KE_FOCUSGAINED : (int)KE_FOCUSLOST;
add_to_input_buf(bytes, 3);
}
#endif
}





static void
gui_mouse_focus(int x, int y)
{
win_T *wp;
char_u st[8];

#if defined(FEAT_MOUSESHAPE)

wp = xy2win(x, y, IGNORE_POPUP);
#endif


if (p_mousef
&& !hold_gui_events 
&& (State & (NORMAL|INSERT))
&& State != HITRETURN 
&& msg_scrolled == 0 
&& !need_mouse_correct 
&& gui.in_focus) 
{

if (x < 0 || x > Columns * gui.char_width)
return;
#if !defined(FEAT_MOUSESHAPE)
wp = xy2win(x, y, IGNORE_POPUP);
#endif
if (wp == curwin || wp == NULL)
return; 


if (Y_2_ROW(y) < tabline_height())
return;







if (finish_op)
{

st[0] = ESC;
add_to_input_buf(st, 1);
}
st[0] = CSI;
st[1] = KS_MOUSE;
st[2] = KE_FILLER;
st[3] = (char_u)MOUSE_LEFT;
fill_mouse_coord(st + 4,
wp->w_wincol == 0 ? -1 : wp->w_wincol + MOUSE_COLOFF,
wp->w_height + W_WINROW(wp));

add_to_input_buf(st, 8);
st[3] = (char_u)MOUSE_RELEASE;
add_to_input_buf(st, 8);
#if defined(FEAT_GUI_GTK)

if (gtk_main_level() > 0)
gtk_main_quit();
#endif
}
}




void
gui_mouse_moved(int x, int y)
{

if (!gui.in_use || gui.starting)
return;


gui_mouse_focus(x, y);

#if defined(FEAT_PROP_POPUP)
if (popup_visible)


gui_send_mouse_event(MOUSE_DRAG, x, y, FALSE, 0);
#endif
}





win_T *
gui_mouse_window(mouse_find_T popup)
{
int x, y;

if (!(gui.in_use && (p_mousef || popup == FIND_POPUP)))
return NULL;
gui_mch_getmouse(&x, &y);


if (x >= 0 && x <= Columns * gui.char_width
&& y >= 0 && Y_2_ROW(y) >= tabline_height())
return xy2win(x, y, popup);
return NULL;
}




void
gui_mouse_correct(void)
{
win_T *wp = NULL;

need_mouse_correct = FALSE;

wp = gui_mouse_window(IGNORE_POPUP);
if (wp != curwin && wp != NULL) 
{
validate_cline_row();
gui_mch_setmouse((int)W_ENDCOL(curwin) * gui.char_width - 3,
(W_WINROW(curwin) + curwin->w_wrow) * gui.char_height
+ (gui.char_height) / 2);
}
}





static win_T *
xy2win(int x, int y, mouse_find_T popup)
{
int row;
int col;
win_T *wp;

row = Y_2_ROW(y);
col = X_2_COL(x);
if (row < 0 || col < 0) 
return NULL;
wp = mouse_find_win(&row, &col, popup);
if (wp == NULL)
return NULL;
#if defined(FEAT_MOUSESHAPE)
if (State == HITRETURN || State == ASKMORE)
{
if (Y_2_ROW(y) >= msg_row)
update_mouseshape(SHAPE_IDX_MOREL);
else
update_mouseshape(SHAPE_IDX_MORE);
}
else if (row > wp->w_height) 
update_mouseshape(SHAPE_IDX_CLINE);
else if (!(State & CMDLINE) && wp->w_vsep_width > 0 && col == wp->w_width
&& (row != wp->w_height || !stl_connected(wp)) && msg_scrolled == 0)
update_mouseshape(SHAPE_IDX_VSEP);
else if (!(State & CMDLINE) && wp->w_status_height > 0
&& row == wp->w_height && msg_scrolled == 0)
update_mouseshape(SHAPE_IDX_STATUS);
else
update_mouseshape(-2);
#endif
return wp;
}





void
ex_gui(exarg_T *eap)
{
char_u *arg = eap->arg;






if (arg[0] == '-'
&& (arg[1] == 'f' || arg[1] == 'b')
&& (arg[2] == NUL || VIM_ISWHITE(arg[2])))
{
gui.dofork = (arg[1] == 'b');
eap->arg = skipwhite(eap->arg + 2);
}
if (!gui.in_use)
{
#if defined(VIMDLL) && !defined(EXPERIMENTAL_GUI_CMD)
if (!gui.starting)
{
emsg(_(e_nogvim));
return;
}
#endif


msg_clr_eos_force();
#if defined(GUI_MAY_SPAWN)
if (!ends_excmd(*eap->arg))
gui_start(eap->arg);
else
#endif
gui_start(NULL);
#if defined(FEAT_JOB_CHANNEL)
channel_gui_register_all();
#endif
}
if (!ends_excmd(*eap->arg))
ex_next(eap);
}

#if ((defined(FEAT_GUI_X11) || defined(FEAT_GUI_GTK) || defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_PHOTON) || defined(FEAT_GUI_HAIKU)) && defined(FEAT_TOOLBAR)) || defined(PROTO)










static void
gfp_setname(char_u *fname, void *cookie)
{
char_u *gfp_buffer = cookie;

if (STRLEN(fname) >= MAXPATHL)
*gfp_buffer = NUL;
else
STRCPY(gfp_buffer, fname);
}





int
gui_find_bitmap(char_u *name, char_u *buffer, char *ext)
{
if (STRLEN(name) > MAXPATHL - 14)
return FAIL;
vim_snprintf((char *)buffer, MAXPATHL, "bitmaps/%s.%s", name, ext);
if (do_in_runtimepath(buffer, 0, gfp_setname, buffer) == FAIL
|| *buffer == NUL)
return FAIL;
return OK;
}

#if !defined(FEAT_GUI_GTK) || defined(PROTO)







void
gui_find_iconfile(char_u *name, char_u *buffer, char *ext)
{
char_u buf[MAXPATHL + 1];

expand_env(name, buffer, MAXPATHL);
if (!mch_isFullName(buffer) && gui_find_bitmap(buffer, buf, ext) == OK)
STRCPY(buffer, buf);
}
#endif
#endif

#if defined(FEAT_GUI_GTK) || defined(FEAT_GUI_X11) || defined(PROTO)
void
display_errors(void)
{
char_u *p;

if (isatty(2))
fflush(stderr);
else if (error_ga.ga_data != NULL)
{

for (p = (char_u *)error_ga.ga_data; *p != NUL; ++p)
if (!isspace(*p))
{

if (STRLEN(p) > 2000)
STRCPY(p + 2000 - 14, "...(truncated)");
(void)do_dialog(VIM_ERROR, (char_u *)_("Error"),
p, (char_u *)_("&Ok"), 1, NULL, FALSE);
break;
}
ga_clear(&error_ga);
}
}
#endif

#if defined(NO_CONSOLE_INPUT) || defined(PROTO)






int
no_console_input(void)
{
return ((!gui.in_use || gui.starting)
#if !defined(NO_CONSOLE)
&& !isatty(0) && !isatty(2)
#endif
);
}
#endif

#if defined(FIND_REPLACE_DIALOG) || defined(NEED_GUI_UPDATE_SCREEN) || defined(PROTO)





void
gui_update_screen(void)
{
#if defined(FEAT_CONCEAL)
linenr_T conceal_old_cursor_line = 0;
linenr_T conceal_new_cursor_line = 0;
int conceal_update_lines = FALSE;
#endif

update_topline();
validate_cursor();


if (!finish_op && (has_cursormoved()
#if defined(FEAT_PROP_POPUP)
|| popup_visible
#endif
#if defined(FEAT_CONCEAL)
|| curwin->w_p_cole > 0
#endif
) && !EQUAL_POS(last_cursormoved, curwin->w_cursor))
{
if (has_cursormoved())
apply_autocmds(EVENT_CURSORMOVED, NULL, NULL, FALSE, curbuf);
#if defined(FEAT_PROP_POPUP)
if (popup_visible)
popup_check_cursor_pos();
#endif
#if defined(FEAT_CONCEAL)
if (curwin->w_p_cole > 0)
{
conceal_old_cursor_line = last_cursormoved.lnum;
conceal_new_cursor_line = curwin->w_cursor.lnum;
conceal_update_lines = TRUE;
}
#endif
last_cursormoved = curwin->w_cursor;
}

#if defined(FEAT_CONCEAL)
if (conceal_update_lines
&& (conceal_old_cursor_line != conceal_new_cursor_line
|| conceal_cursor_line(curwin)
|| need_cursor_line_redraw))
{
if (conceal_old_cursor_line != conceal_new_cursor_line)
redrawWinline(curwin, conceal_old_cursor_line);
redrawWinline(curwin, conceal_new_cursor_line);
curwin->w_valid &= ~VALID_CROW;
need_cursor_line_redraw = FALSE;
}
#endif
update_screen(0); 
setcursor();
out_flush_cursor(TRUE, FALSE);
}
#endif

#if defined(FIND_REPLACE_DIALOG) || defined(PROTO)





char_u *
get_find_dialog_text(
char_u *arg,
int *wwordp, 
int *mcasep) 
{
char_u *text;

if (*arg == NUL)
text = last_search_pat();
else
text = arg;
if (text != NULL)
{
text = vim_strsave(text);
if (text != NULL)
{
int len = (int)STRLEN(text);
int i;


if (len >= 2 && STRNCMP(text, "\\V", 2) == 0)
{
mch_memmove(text, text + 2, (size_t)(len - 1));
len -= 2;
}


if (len >= 2 && *text == '\\' && (text[1] == 'c' || text[1] == 'C'))
{
*mcasep = (text[1] == 'C');
mch_memmove(text, text + 2, (size_t)(len - 1));
len -= 2;
}


if (len >= 4
&& STRNCMP(text, "\\<", 2) == 0
&& STRNCMP(text + len - 2, "\\>", 2) == 0)
{
*wwordp = TRUE;
mch_memmove(text, text + 2, (size_t)(len - 4));
text[len - 4] = NUL;
}


for (i = 0; i + 1 < len; ++i)
if (text[i] == '\\' && (text[i + 1] == '/'
|| text[i + 1] == '?'))
{
mch_memmove(text + i, text + i + 1, (size_t)(len - i));
--len;
}
}
}
return text;
}





int
gui_do_findrepl(
int flags, 
char_u *find_text,
char_u *repl_text,
int down) 
{
garray_T ga;
int i;
int type = (flags & FRD_TYPE_MASK);
char_u *p;
regmatch_T regmatch;
int save_did_emsg = did_emsg;
static int busy = FALSE;




if (updating_screen || busy)
return FALSE;


if ((type == FRD_REPLACE || type == FRD_REPLACEALL) && text_locked())
return FALSE;

busy = TRUE;

ga_init2(&ga, 1, 100);
if (type == FRD_REPLACEALL)
ga_concat(&ga, (char_u *)"%s/");

ga_concat(&ga, (char_u *)"\\V");
if (flags & FRD_MATCH_CASE)
ga_concat(&ga, (char_u *)"\\C");
else
ga_concat(&ga, (char_u *)"\\c");
if (flags & FRD_WHOLE_WORD)
ga_concat(&ga, (char_u *)"\\<");

p = vim_strsave_escaped(find_text, (char_u *)"/\\");
if (p != NULL)
ga_concat(&ga, p);
vim_free(p);
if (flags & FRD_WHOLE_WORD)
ga_concat(&ga, (char_u *)"\\>");

if (type == FRD_REPLACEALL)
{
ga_concat(&ga, (char_u *)"/");

p = vim_strsave_escaped(repl_text, (char_u *)"/\\");
if (p != NULL)
ga_concat(&ga, p);
vim_free(p);
ga_concat(&ga, (char_u *)"/g");
}
ga_append(&ga, NUL);

if (type == FRD_REPLACE)
{


regmatch.regprog = vim_regcomp(ga.ga_data, RE_MAGIC + RE_STRING);
regmatch.rm_ic = 0;
if (regmatch.regprog != NULL)
{
p = ml_get_cursor();
if (vim_regexec_nl(&regmatch, p, (colnr_T)0)
&& regmatch.startp[0] == p)
{


msg_end_prompt();

if (u_save_cursor() == OK)
{

u_sync(FALSE);

del_bytes((long)(regmatch.endp[0] - regmatch.startp[0]),
FALSE, FALSE);
ins_str(repl_text);
}
}
else
msg(_("No match at cursor, finding next"));
vim_regfree(regmatch.regprog);
}
}

if (type == FRD_REPLACEALL)
{

u_sync(FALSE);
do_cmdline_cmd(ga.ga_data);
}
else
{
int searchflags = SEARCH_MSG + SEARCH_MARK;



if (type == FRD_REPLACE)
searchflags += SEARCH_START;
i = msg_scroll;
if (down)
{
(void)do_search(NULL, '/', '/', ga.ga_data, 1L, searchflags, NULL);
}
else
{


p = vim_strsave_escaped(ga.ga_data, (char_u *)"?");
if (p != NULL)
(void)do_search(NULL, '?', '?', p, 1L, searchflags, NULL);
vim_free(p);
}

msg_scroll = i; 
}



did_emsg = save_did_emsg;

if (State & (NORMAL | INSERT))
{
gui_update_screen(); 
msg_didout = 0; 
need_wait_return = FALSE; 
}

vim_free(ga.ga_data);
busy = FALSE;
return (ga.ga_len > 0);
}

#endif

#if defined(HAVE_DROP_FILE) || defined(PROTO)



static void
gui_wingoto_xy(int x, int y)
{
int row = Y_2_ROW(y);
int col = X_2_COL(x);
win_T *wp;

if (row >= 0 && col >= 0)
{
wp = mouse_find_win(&row, &col, FAIL_POPUP);
if (wp != NULL && wp != curwin)
win_goto(wp);
}
}





static void
drop_callback(void *cookie)
{
char_u *p = cookie;




if (p != NULL)
{
if (mch_isdir(p))
{
if (mch_chdir((char *)p) == 0)
shorten_fnames(TRUE);
}
else if (vim_chdirfile(p, "drop") == OK)
shorten_fnames(TRUE);
vim_free(p);
}


update_screen(NOT_VALID);
#if defined(FEAT_MENU)
gui_update_menus(0);
#endif
#if defined(FEAT_TITLE)
maketitle();
#endif
setcursor();
out_flush_cursor(FALSE, FALSE);
}







void
gui_handle_drop(
int x UNUSED,
int y UNUSED,
int_u modifiers,
char_u **fnames,
int count)
{
int i;
char_u *p;
static int entered = FALSE;






if (entered)
return;
entered = TRUE;





if (State & CMDLINE)
{
shorten_filenames(fnames, count);
for (i = 0; i < count; ++i)
{
if (fnames[i] != NULL)
{
if (i > 0)
add_to_input_buf((char_u*)" ", 1);




#if defined(BACKSLASH_IN_FILENAME)
p = vim_strsave_escaped(fnames[i], (char_u *)" \t\"|");
#else
p = vim_strsave_escaped(fnames[i], (char_u *)"\\ \t\"|");
#endif
if (p != NULL)
add_to_input_buf_csi(p, (int)STRLEN(p));
vim_free(p);
vim_free(fnames[i]);
}
}
vim_free(fnames);
}
else
{


gui_wingoto_xy(x, y);
shorten_filenames(fnames, count);


if ((modifiers & MOUSE_SHIFT) != 0)
p = vim_strsave(fnames[0]);
else
p = NULL;




if (count == 1 && (modifiers & MOUSE_SHIFT) != 0
&& mch_isdir(fnames[0]))
{
vim_free(fnames[0]);
vim_free(fnames);
}
else
handle_drop(count, fnames, (modifiers & MOUSE_CTRL) != 0,
drop_callback, (void *)p);
}

entered = FALSE;
}
#endif
