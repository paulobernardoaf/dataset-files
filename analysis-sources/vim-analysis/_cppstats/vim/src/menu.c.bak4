













#include "vim.h"

#if defined(FEAT_MENU) || defined(PROTO)

#define MENUDEPTH 10 

#if defined(FEAT_GUI_MSWIN)
static int add_menu_path(char_u *, vimmenu_T *, int *, char_u *, int);
#else
static int add_menu_path(char_u *, vimmenu_T *, int *, char_u *);
#endif
static int menu_nable_recurse(vimmenu_T *menu, char_u *name, int modes, int enable);
static int remove_menu(vimmenu_T **, char_u *, int, int silent);
static void free_menu(vimmenu_T **menup);
static void free_menu_string(vimmenu_T *, int);
static int show_menus(char_u *, int);
static void show_menus_recursive(vimmenu_T *, int, int);
static char_u *menu_name_skip(char_u *name);
static int menu_name_equal(char_u *name, vimmenu_T *menu);
static int menu_namecmp(char_u *name, char_u *mname);
static int get_menu_cmd_modes(char_u *, int, int *, int *);
static char_u *popup_mode_name(char_u *name, int idx);
static char_u *menu_text(char_u *text, int *mnemonic, char_u **actext);

#if defined(FEAT_GUI_MSWIN) && defined(FEAT_TEAROFF)
static void gui_create_tearoffs_recurse(vimmenu_T *menu, const char_u *pname, int *pri_tab, int pri_idx);
static void gui_add_tearoff(char_u *tearpath, int *pri_tab, int pri_idx);
static void gui_destroy_tearoffs_recurse(vimmenu_T *menu);
static int s_tearoffs = FALSE;
#endif

static int menu_is_hidden(char_u *name);
static int menu_is_tearoff(char_u *name);

#if defined(FEAT_MULTI_LANG) || defined(FEAT_TOOLBAR)
static char_u *menu_skip_part(char_u *p);
#endif
#if defined(FEAT_MULTI_LANG)
static char_u *menutrans_lookup(char_u *name, int len);
static void menu_unescape_name(char_u *p);
#endif

static char_u *menu_translate_tab_and_shift(char_u *arg_start);


static char *menu_mode_chars[] = {"n", "v", "s", "o", "i", "c", "tl", "t"};

static char_u e_notsubmenu[] = N_("E327: Part of menu-item path is not sub-menu");
static char_u e_nomenu[] = N_("E329: No menu \"%s\"");

#if defined(FEAT_TOOLBAR)
static const char *toolbar_names[] =
{
"New", "Open", "Save", "Undo", "Redo",
"Cut", "Copy", "Paste", "Print", "Help",
"Find", "SaveAll", "SaveSesn", "NewSesn", "LoadSesn",
"RunScript", "Replace", "WinClose", "WinMax", "WinMin",
"WinSplit", "Shell", "FindPrev", "FindNext", "FindHelp",
"Make", "TagJump", "RunCtags", "WinVSplit", "WinMaxWidth",
"WinMinWidth", "Exit"
};
#define TOOLBAR_NAME_COUNT (sizeof(toolbar_names) / sizeof(char *))
#endif




static int
menu_is_winbar(char_u *name)
{
return (STRNCMP(name, "WinBar", 6) == 0);
}

int
winbar_height(win_T *wp)
{
if (wp->w_winbar != NULL && wp->w_winbar->children != NULL)
return 1;
return 0;
}

static vimmenu_T **
get_root_menu(char_u *name)
{
if (menu_is_winbar(name))
return &curwin->w_winbar;
return &root_menu;
}




void
ex_menu(
exarg_T *eap) 
{
char_u *menu_path;
int modes;
char_u *map_to;
int noremap;
int silent = FALSE;
int special = FALSE;
int unmenu;
char_u *map_buf;
char_u *arg;
char_u *p;
int i;
#if defined(FEAT_GUI) && !defined(FEAT_GUI_GTK)
int old_menu_height;
#if defined(FEAT_TOOLBAR) && !defined(FEAT_GUI_MSWIN)
int old_toolbar_height;
#endif
#endif
int pri_tab[MENUDEPTH + 1];
int enable = MAYBE; 

#if defined(FEAT_TOOLBAR)
char_u *icon = NULL;
#endif
vimmenu_T menuarg;
vimmenu_T **root_menu_ptr;

modes = get_menu_cmd_modes(eap->cmd, eap->forceit, &noremap, &unmenu);
arg = eap->arg;

for (;;)
{
if (STRNCMP(arg, "<script>", 8) == 0)
{
noremap = REMAP_SCRIPT;
arg = skipwhite(arg + 8);
continue;
}
if (STRNCMP(arg, "<silent>", 8) == 0)
{
silent = TRUE;
arg = skipwhite(arg + 8);
continue;
}
if (STRNCMP(arg, "<special>", 9) == 0)
{
special = TRUE;
arg = skipwhite(arg + 9);
continue;
}
break;
}



if (STRNCMP(arg, "icon=", 5) == 0)
{
arg += 5;
#if defined(FEAT_TOOLBAR)
icon = arg;
#endif
while (*arg != NUL && *arg != ' ')
{
if (*arg == '\\')
STRMOVE(arg, arg + 1);
MB_PTR_ADV(arg);
}
if (*arg != NUL)
{
*arg++ = NUL;
arg = skipwhite(arg);
}
}




for (p = arg; *p; ++p)
if (!VIM_ISDIGIT(*p) && *p != '.')
break;
if (VIM_ISWHITE(*p))
{
for (i = 0; i < MENUDEPTH && !VIM_ISWHITE(*arg); ++i)
{
pri_tab[i] = getdigits(&arg);
if (pri_tab[i] == 0)
pri_tab[i] = 500;
if (*arg == '.')
++arg;
}
arg = skipwhite(arg);
}
else if (eap->addr_count && eap->line2 != 0)
{
pri_tab[0] = eap->line2;
i = 1;
}
else
i = 0;
while (i < MENUDEPTH)
pri_tab[i++] = 500;
pri_tab[MENUDEPTH] = -1; 




if (STRNCMP(arg, "enable", 6) == 0 && VIM_ISWHITE(arg[6]))
{
enable = TRUE;
arg = skipwhite(arg + 6);
}
else if (STRNCMP(arg, "disable", 7) == 0 && VIM_ISWHITE(arg[7]))
{
enable = FALSE;
arg = skipwhite(arg + 7);
}




if (*arg == NUL)
{
show_menus(arg, modes);
return;
}

#if defined(FEAT_TOOLBAR)



menuarg.iconidx = -1;
menuarg.icon_builtin = FALSE;
if (menu_is_toolbar(arg))
{
menu_path = menu_skip_part(arg);
if (*menu_path == '.')
{
p = menu_skip_part(++menu_path);
if (STRNCMP(menu_path, "BuiltIn", 7) == 0)
{
if (skipdigits(menu_path + 7) == p)
{
menuarg.iconidx = atoi((char *)menu_path + 7);
if (menuarg.iconidx >= (int)TOOLBAR_NAME_COUNT)
menuarg.iconidx = -1;
else
menuarg.icon_builtin = TRUE;
}
}
else
{
for (i = 0; i < (int)TOOLBAR_NAME_COUNT; ++i)
if (STRNCMP(toolbar_names[i], menu_path, p - menu_path)
== 0)
{
menuarg.iconidx = i;
break;
}
}
}
}
#endif

menu_path = arg;
if (*menu_path == '.')
{
semsg(_(e_invarg2), menu_path);
goto theend;
}

map_to = menu_translate_tab_and_shift(arg);




if (*map_to == NUL && !unmenu && enable == MAYBE)
{
show_menus(menu_path, modes);
goto theend;
}
else if (*map_to != NUL && (unmenu || enable != MAYBE))
{
emsg(_(e_trailing));
goto theend;
}
#if defined(FEAT_GUI) && !(defined(FEAT_GUI_GTK) || defined(FEAT_GUI_PHOTON))
old_menu_height = gui.menu_height;
#if defined(FEAT_TOOLBAR) && !defined(FEAT_GUI_MSWIN)
old_toolbar_height = gui.toolbar_height;
#endif
#endif

root_menu_ptr = get_root_menu(menu_path);
if (root_menu_ptr == &curwin->w_winbar)

redraw_later(NOT_VALID);

if (enable != MAYBE)
{





if (STRCMP(menu_path, "*") == 0) 
menu_path = (char_u *)"";

if (menu_is_popup(menu_path))
{
for (i = 0; i < MENU_INDEX_TIP; ++i)
if (modes & (1 << i))
{
p = popup_mode_name(menu_path, i);
if (p != NULL)
{
menu_nable_recurse(*root_menu_ptr, p, MENU_ALL_MODES,
enable);
vim_free(p);
}
}
}
menu_nable_recurse(*root_menu_ptr, menu_path, modes, enable);
}
else if (unmenu)
{



if (STRCMP(menu_path, "*") == 0) 
menu_path = (char_u *)"";




if (menu_is_popup(menu_path))
{
for (i = 0; i < MENU_INDEX_TIP; ++i)
if (modes & (1 << i))
{
p = popup_mode_name(menu_path, i);
if (p != NULL)
{
remove_menu(root_menu_ptr, p, MENU_ALL_MODES, TRUE);
vim_free(p);
}
}
}


remove_menu(root_menu_ptr, menu_path, modes, FALSE);
}
else
{




if (STRICMP(map_to, "<nop>") == 0) 
{
map_to = (char_u *)"";
map_buf = NULL;
}
else if (modes & MENU_TIP_MODE)
map_buf = NULL; 
else
map_to = replace_termcodes(map_to, &map_buf,
REPTERM_DO_LT | (special ? REPTERM_SPECIAL : 0), NULL);
menuarg.modes = modes;
#if defined(FEAT_TOOLBAR)
menuarg.iconfile = icon;
#endif
menuarg.noremap[0] = noremap;
menuarg.silent[0] = silent;
add_menu_path(menu_path, &menuarg, pri_tab, map_to
#if defined(FEAT_GUI_MSWIN)
, TRUE
#endif
);




if (menu_is_popup(menu_path))
{
for (i = 0; i < MENU_INDEX_TIP; ++i)
if (modes & (1 << i))
{
p = popup_mode_name(menu_path, i);
if (p != NULL)
{

menuarg.modes = modes;
#if defined(FEAT_TOOLBAR)
menuarg.iconfile = NULL;
menuarg.iconidx = -1;
menuarg.icon_builtin = FALSE;
#endif
add_menu_path(p, &menuarg, pri_tab, map_to
#if defined(FEAT_GUI_MSWIN)
, TRUE
#endif
);
vim_free(p);
}
}
}

vim_free(map_buf);
}

#if defined(FEAT_GUI) && !(defined(FEAT_GUI_GTK))

if (gui.in_use
&& (gui.menu_height != old_menu_height
#if defined(FEAT_TOOLBAR) && !defined(FEAT_GUI_MSWIN)
|| gui.toolbar_height != old_toolbar_height
#endif
))
gui_set_shellsize(FALSE, FALSE, RESIZE_VERT);
#endif
if (root_menu_ptr == &curwin->w_winbar)
{
int h = winbar_height(curwin);

if (h != curwin->w_winbar_height)
{
if (h == 0)
++curwin->w_height;
else if (curwin->w_height > 0)
--curwin->w_height;
curwin->w_winbar_height = h;
}
}

theend:
;
}




static int
add_menu_path(
char_u *menu_path,
vimmenu_T *menuarg, 

int *pri_tab,
char_u *call_data
#if defined(FEAT_GUI_MSWIN)
, int addtearoff 
#endif
)
{
char_u *path_name;
int modes = menuarg->modes;
vimmenu_T **menup;
vimmenu_T *menu = NULL;
vimmenu_T *parent;
vimmenu_T **lower_pri;
char_u *p;
char_u *name;
char_u *dname;
char_u *next_name;
int i;
int c;
int d;
#if defined(FEAT_GUI)
int idx;
int new_idx;
#endif
int pri_idx = 0;
int old_modes = 0;
int amenu;
#if defined(FEAT_MULTI_LANG)
char_u *en_name;
char_u *map_to = NULL;
#endif
vimmenu_T **root_menu_ptr;


path_name = vim_strsave(menu_path);
if (path_name == NULL)
return FAIL;
root_menu_ptr = get_root_menu(menu_path);
menup = root_menu_ptr;
parent = NULL;
name = path_name;
while (*name)
{


next_name = menu_name_skip(name);
#if defined(FEAT_MULTI_LANG)
map_to = menutrans_lookup(name, (int)STRLEN(name));
if (map_to != NULL)
{
en_name = name;
name = map_to;
}
else
en_name = NULL;
#endif
dname = menu_text(name, NULL, NULL);
if (dname == NULL)
goto erret;
if (*dname == NUL)
{

emsg(_("E792: Empty menu name"));
goto erret;
}


lower_pri = menup;
#if defined(FEAT_GUI)
idx = 0;
new_idx = 0;
#endif
menu = *menup;
while (menu != NULL)
{
if (menu_name_equal(name, menu) || menu_name_equal(dname, menu))
{
if (*next_name == NUL && menu->children != NULL)
{
if (!sys_menu)
emsg(_("E330: Menu path must not lead to a sub-menu"));
goto erret;
}
if (*next_name != NUL && menu->children == NULL
#if defined(FEAT_GUI_MSWIN)
&& addtearoff
#endif
)
{
if (!sys_menu)
emsg(_(e_notsubmenu));
goto erret;
}
break;
}
menup = &menu->next;



if (parent != NULL || menu_is_menubar(menu->name))
{
#if defined(FEAT_GUI)
++idx;
#endif
if (menu->priority <= pri_tab[pri_idx])
{
lower_pri = menup;
#if defined(FEAT_GUI)
new_idx = idx;
#endif
}
}
menu = menu->next;
}

if (menu == NULL)
{
if (*next_name == NUL && parent == NULL)
{
emsg(_("E331: Must not add menu items directly to menu bar"));
goto erret;
}

if (menu_is_separator(dname) && *next_name != NUL)
{
emsg(_("E332: Separator cannot be part of a menu path"));
goto erret;
}


menu = ALLOC_CLEAR_ONE(vimmenu_T);
if (menu == NULL)
goto erret;

menu->modes = modes;
menu->enabled = MENU_ALL_MODES;
menu->name = vim_strsave(name);

menu->dname = menu_text(name, &menu->mnemonic, &menu->actext);
#if defined(FEAT_MULTI_LANG)
if (en_name != NULL)
{
menu->en_name = vim_strsave(en_name);
menu->en_dname = menu_text(en_name, NULL, NULL);
}
else
{
menu->en_name = NULL;
menu->en_dname = NULL;
}
#endif
menu->priority = pri_tab[pri_idx];
menu->parent = parent;
#if defined(FEAT_GUI_MOTIF)
menu->sensitive = TRUE; 
#endif
#if defined(FEAT_BEVAL_TIP)
menu->tip = NULL;
#endif
#if defined(FEAT_GUI_ATHENA)
menu->image = None; 
#endif




menu->next = *lower_pri;
*lower_pri = menu;

old_modes = 0;

#if defined(FEAT_TOOLBAR)
menu->iconidx = menuarg->iconidx;
menu->icon_builtin = menuarg->icon_builtin;
if (*next_name == NUL && menuarg->iconfile != NULL)
menu->iconfile = vim_strsave(menuarg->iconfile);
#endif
#if defined(FEAT_GUI_MSWIN) && defined(FEAT_TEAROFF)

if (parent != NULL && menu_is_tearoff(parent->children->dname))
parent->children->modes |= modes;
#endif
}
else
{
old_modes = menu->modes;






#if defined(FEAT_GUI_MSWIN)

if (addtearoff)
#endif
{
menu->modes |= modes;
menu->enabled |= modes;
}
}

#if defined(FEAT_GUI)




if ((old_modes & MENU_ALL_MODES) == 0
&& (menu->modes & MENU_ALL_MODES) != 0)
{
if (gui.in_use) 
{
if (*next_name == NUL)
{

gui_mch_add_menu_item(menu, new_idx);


force_menu_update = TRUE;
}
else
{

gui_mch_add_menu(menu, new_idx);
}
}

#if defined(FEAT_GUI_MSWIN) & defined(FEAT_TEAROFF)

if ( addtearoff
&& *next_name
&& vim_strchr(p_go, GO_TEAROFF) != NULL
&& menu_is_menubar(name)
#if defined(VIMDLL)
&& (gui.in_use || gui.starting)
#endif
)
{
char_u *tearpath;






tearpath = alloc(STRLEN(menu_path) + TEAR_LEN + 2);
if (tearpath != NULL)
{
char_u *s;
int idx;

STRCPY(tearpath, menu_path);
idx = (int)(next_name - path_name - 1);
for (s = tearpath; *s && s < tearpath + idx; MB_PTR_ADV(s))
{
if ((*s == '\\' || *s == Ctrl_V) && s[1])
{
++idx;
++s;
}
}
tearpath[idx] = NUL;
gui_add_tearoff(tearpath, pri_tab, pri_idx);
vim_free(tearpath);
}
}
#endif
}
#endif 

menup = &menu->children;
parent = menu;
name = next_name;
VIM_CLEAR(dname);
if (pri_tab[pri_idx + 1] != -1)
++pri_idx;
}
vim_free(path_name);





amenu = ((modes & (MENU_NORMAL_MODE | MENU_INSERT_MODE)) ==
(MENU_NORMAL_MODE | MENU_INSERT_MODE));
if (sys_menu)
modes &= ~old_modes;

if (menu != NULL && modes)
{
#if defined(FEAT_GUI)
menu->cb = gui_menu_cb;
#endif
p = (call_data == NULL) ? NULL : vim_strsave(call_data);


for (i = 0; i < MENU_MODES; ++i)
{
if (modes & (1 << i))
{

free_menu_string(menu, i);




c = 0;
d = 0;
if (amenu && call_data != NULL && *call_data != NUL
#if defined(FEAT_GUI_MSWIN)
&& addtearoff
#endif
)
{
switch (1 << i)
{
case MENU_VISUAL_MODE:
case MENU_SELECT_MODE:
case MENU_OP_PENDING_MODE:
case MENU_CMDLINE_MODE:
c = Ctrl_C;
break;
case MENU_INSERT_MODE:
c = Ctrl_BSL;
d = Ctrl_O;
break;
}
}

if (c != 0)
{
menu->strings[i] = alloc(STRLEN(call_data) + 5);
if (menu->strings[i] != NULL)
{
menu->strings[i][0] = c;
if (d == 0)
STRCPY(menu->strings[i] + 1, call_data);
else
{
menu->strings[i][1] = d;
STRCPY(menu->strings[i] + 2, call_data);
}
if (c == Ctrl_C)
{
int len = (int)STRLEN(menu->strings[i]);


menu->strings[i][len] = Ctrl_BSL;
menu->strings[i][len + 1] = Ctrl_G;
menu->strings[i][len + 2] = NUL;
}
}
}
else
menu->strings[i] = p;
menu->noremap[i] = menuarg->noremap[0];
menu->silent[i] = menuarg->silent[0];
}
}
#if defined(FEAT_TOOLBAR) && !defined(FEAT_GUI_MSWIN) && (defined(FEAT_BEVAL_GUI) || defined(FEAT_GUI_GTK))


if (modes & MENU_TIP_MODE)
gui_mch_menu_set_tip(menu);
#endif
}
return OK;

erret:
vim_free(path_name);
vim_free(dname);



while (parent != NULL && parent->children == NULL)
{
if (parent->parent == NULL)
menup = root_menu_ptr;
else
menup = &parent->parent->children;
for ( ; *menup != NULL && *menup != parent; menup = &((*menup)->next))
;
if (*menup == NULL) 
break;
parent = parent->parent;
free_menu(menup);
}
return FAIL;
}





static int
menu_nable_recurse(
vimmenu_T *menu,
char_u *name,
int modes,
int enable)
{
char_u *p;

if (menu == NULL)
return OK; 


p = menu_name_skip(name);


while (menu != NULL)
{
if (*name == NUL || *name == '*' || menu_name_equal(name, menu))
{
if (*p != NUL)
{
if (menu->children == NULL)
{
emsg(_(e_notsubmenu));
return FAIL;
}
if (menu_nable_recurse(menu->children, p, modes, enable)
== FAIL)
return FAIL;
}
else
if (enable)
menu->enabled |= modes;
else
menu->enabled &= ~modes;






if (*name != NUL && *name != '*')
break;
}
menu = menu->next;
}
if (*name != NUL && *name != '*' && menu == NULL)
{
semsg(_(e_nomenu), name);
return FAIL;
}

#if defined(FEAT_GUI)

force_menu_update = TRUE;
#endif

return OK;
}





static int
remove_menu(
vimmenu_T **menup,
char_u *name,
int modes,
int silent) 
{
vimmenu_T *menu;
vimmenu_T *child;
char_u *p;

if (*menup == NULL)
return OK; 


p = menu_name_skip(name);


while ((menu = *menup) != NULL)
{
if (*name == NUL || menu_name_equal(name, menu))
{
if (*p != NUL && menu->children == NULL)
{
if (!silent)
emsg(_(e_notsubmenu));
return FAIL;
}
if ((menu->modes & modes) != 0x0)
{
#if defined(FEAT_GUI_MSWIN) & defined(FEAT_TEAROFF)




if (*p == NUL && modes == MENU_ALL_MODES)
{
if (IsWindow(menu->tearoff_handle))
DestroyWindow(menu->tearoff_handle);
}
#endif
if (remove_menu(&menu->children, p, modes, silent) == FAIL)
return FAIL;
}
else if (*name != NUL)
{
if (!silent)
emsg(_(e_menuothermode));
return FAIL;
}






if (*name != NUL)
break;



menu->modes &= ~modes;
if (modes & MENU_TIP_MODE)
free_menu_string(menu, MENU_INDEX_TIP);
if ((menu->modes & MENU_ALL_MODES) == 0)
free_menu(menup);
else
menup = &menu->next;
}
else
menup = &menu->next;
}
if (*name != NUL)
{
if (menu == NULL)
{
if (!silent)
semsg(_(e_nomenu), name);
return FAIL;
}



menu->modes &= ~modes;
#if defined(FEAT_GUI_MSWIN) & defined(FEAT_TEAROFF)
if ((s_tearoffs) && (menu->children != NULL)) 
child = menu->children->next; 
else
#endif
child = menu->children;
for ( ; child != NULL; child = child->next)
menu->modes |= child->modes;
if (modes & MENU_TIP_MODE)
{
free_menu_string(menu, MENU_INDEX_TIP);
#if defined(FEAT_TOOLBAR) && !defined(FEAT_GUI_MSWIN) && (defined(FEAT_BEVAL_GUI) || defined(FEAT_GUI_GTK))


if (gui.in_use)
gui_mch_menu_set_tip(menu);
#endif
}
if ((menu->modes & MENU_ALL_MODES) == 0)
{

#if defined(FEAT_GUI_MSWIN) & defined(FEAT_TEAROFF)
if (s_tearoffs && menu->children != NULL) 
free_menu(&menu->children);
#endif
*menup = menu;
free_menu(menup);
}
}

return OK;
}




void
remove_winbar(win_T *wp)
{
remove_menu(&wp->w_winbar, (char_u *)"", MENU_ALL_MODES, TRUE);
vim_free(wp->w_winbar_items);
}




static void
free_menu(vimmenu_T **menup)
{
int i;
vimmenu_T *menu;

menu = *menup;

#if defined(FEAT_GUI)


if (gui.in_use)
gui_mch_destroy_menu(menu);
#endif



*menup = menu->next;
vim_free(menu->name);
vim_free(menu->dname);
#if defined(FEAT_MULTI_LANG)
vim_free(menu->en_name);
vim_free(menu->en_dname);
#endif
vim_free(menu->actext);
#if defined(FEAT_TOOLBAR)
vim_free(menu->iconfile);
#if defined(FEAT_GUI_MOTIF)
vim_free(menu->xpm_fname);
#endif
#endif
for (i = 0; i < MENU_MODES; i++)
free_menu_string(menu, i);
vim_free(menu);

#if defined(FEAT_GUI)

force_menu_update = TRUE;
#endif
}




static void
free_menu_string(vimmenu_T *menu, int idx)
{
int count = 0;
int i;

for (i = 0; i < MENU_MODES; i++)
if (menu->strings[i] == menu->strings[idx])
count++;
if (count == 1)
vim_free(menu->strings[idx]);
menu->strings[idx] = NULL;
}




static int
show_menus(char_u *path_name, int modes)
{
char_u *p;
char_u *name;
vimmenu_T *menu;
vimmenu_T *parent = NULL;

name = path_name = vim_strsave(path_name);
if (path_name == NULL)
return FAIL;
menu = *get_root_menu(path_name);


while (*name)
{
p = menu_name_skip(name);
while (menu != NULL)
{
if (menu_name_equal(name, menu))
{

if (*p != NUL && menu->children == NULL)
{
emsg(_(e_notsubmenu));
vim_free(path_name);
return FAIL;
}
else if ((menu->modes & modes) == 0x0)
{
emsg(_(e_menuothermode));
vim_free(path_name);
return FAIL;
}
break;
}
menu = menu->next;
}
if (menu == NULL)
{
semsg(_(e_nomenu), name);
vim_free(path_name);
return FAIL;
}
name = p;
parent = menu;
menu = menu->children;
}
vim_free(path_name);



msg_puts_title(_("\n--- Menus ---"));

show_menus_recursive(parent, modes, 0);
return OK;
}




static void
show_menus_recursive(vimmenu_T *menu, int modes, int depth)
{
int i;
int bit;

if (menu != NULL && (menu->modes & modes) == 0x0)
return;

if (menu != NULL)
{
msg_putchar('\n');
if (got_int) 
return;
for (i = 0; i < depth; i++)
msg_puts(" ");
if (menu->priority)
{
msg_outnum((long)menu->priority);
msg_puts(" ");
}

msg_outtrans_attr(menu->name, HL_ATTR(HLF_D));
}

if (menu != NULL && menu->children == NULL)
{
for (bit = 0; bit < MENU_MODES; bit++)
if ((menu->modes & modes & (1 << bit)) != 0)
{
msg_putchar('\n');
if (got_int) 
return;
for (i = 0; i < depth + 2; i++)
msg_puts(" ");
msg_puts(menu_mode_chars[bit]);
if (menu->noremap[bit] == REMAP_NONE)
msg_putchar('*');
else if (menu->noremap[bit] == REMAP_SCRIPT)
msg_putchar('&');
else
msg_putchar(' ');
if (menu->silent[bit])
msg_putchar('s');
else
msg_putchar(' ');
if ((menu->modes & menu->enabled & (1 << bit)) == 0)
msg_putchar('-');
else
msg_putchar(' ');
msg_puts(" ");
if (*menu->strings[bit] == NUL)
msg_puts_attr("<Nop>", HL_ATTR(HLF_8));
else
msg_outtrans_special(menu->strings[bit], FALSE, 0);
}
}
else
{
if (menu == NULL)
{
menu = root_menu;
depth--;
}
else
menu = menu->children;


for (; menu != NULL && !got_int; menu = menu->next)
if (!menu_is_hidden(menu->dname))
show_menus_recursive(menu, modes, depth + 1);
}
}




static vimmenu_T *expand_menu = NULL;
static vimmenu_T *expand_menu_alt = NULL;
static int expand_modes = 0x0;
static int expand_emenu; 




char_u *
set_context_in_menu_cmd(
expand_T *xp,
char_u *cmd,
char_u *arg,
int forceit)
{
char_u *after_dot;
char_u *p;
char_u *path_name = NULL;
char_u *name;
int unmenu;
vimmenu_T *menu;
int expand_menus;

xp->xp_context = EXPAND_UNSUCCESSFUL;



for (p = arg; *p; ++p)
if (!VIM_ISDIGIT(*p) && *p != '.')
break;

if (!VIM_ISWHITE(*p))
{
if (STRNCMP(arg, "enable", 6) == 0
&& (arg[6] == NUL || VIM_ISWHITE(arg[6])))
p = arg + 6;
else if (STRNCMP(arg, "disable", 7) == 0
&& (arg[7] == NUL || VIM_ISWHITE(arg[7])))
p = arg + 7;
else
p = arg;
}

while (*p != NUL && VIM_ISWHITE(*p))
++p;

arg = after_dot = p;

for (; *p && !VIM_ISWHITE(*p); ++p)
{
if ((*p == '\\' || *p == Ctrl_V) && p[1] != NUL)
p++;
else if (*p == '.')
after_dot = p + 1;
}


expand_menus = !((*cmd == 't' && cmd[1] == 'e') || *cmd == 'p');
expand_emenu = (*cmd == 'e');
if (expand_menus && VIM_ISWHITE(*p))
return NULL; 
if (*p == NUL) 
{
int try_alt_menu = TRUE;






expand_modes = get_menu_cmd_modes(cmd, forceit, NULL, &unmenu);
if (!unmenu)
expand_modes = MENU_ALL_MODES;

menu = root_menu;
if (after_dot != arg)
{
path_name = alloc(after_dot - arg);
if (path_name == NULL)
return NULL;
vim_strncpy(path_name, arg, after_dot - arg - 1);
}
name = path_name;
while (name != NULL && *name)
{
p = menu_name_skip(name);
while (menu != NULL)
{
if (menu_name_equal(name, menu))
{

if ((*p != NUL && menu->children == NULL)
|| ((menu->modes & expand_modes) == 0x0))
{




vim_free(path_name);
return NULL;
}
break;
}
menu = menu->next;
if (menu == NULL && try_alt_menu)
{
menu = curwin->w_winbar;
try_alt_menu = FALSE;
}
}
if (menu == NULL)
{

vim_free(path_name);
return NULL;
}
name = p;
menu = menu->children;
try_alt_menu = FALSE;
}
vim_free(path_name);

xp->xp_context = expand_menus ? EXPAND_MENUNAMES : EXPAND_MENUS;
xp->xp_pattern = after_dot;
expand_menu = menu;
if (expand_menu == root_menu)
expand_menu_alt = curwin->w_winbar;
else
expand_menu_alt = NULL;
}
else 
xp->xp_context = EXPAND_NOTHING;
return NULL;
}





char_u *
get_menu_name(expand_T *xp UNUSED, int idx)
{
static vimmenu_T *menu = NULL;
static int did_alt_menu = FALSE;
char_u *str;
#if defined(FEAT_MULTI_LANG)
static int should_advance = FALSE;
#endif

if (idx == 0) 
{
menu = expand_menu;
did_alt_menu = FALSE;
#if defined(FEAT_MULTI_LANG)
should_advance = FALSE;
#endif
}


while (menu != NULL && (menu_is_hidden(menu->dname)
|| menu_is_separator(menu->dname)
|| menu_is_tearoff(menu->dname)
|| menu->children == NULL))
{
menu = menu->next;
if (menu == NULL && !did_alt_menu)
{
menu = expand_menu_alt;
did_alt_menu = TRUE;
}
}

if (menu == NULL) 
return NULL;

if (menu->modes & expand_modes)
#if defined(FEAT_MULTI_LANG)
if (should_advance)
str = menu->en_dname;
else
{
#endif
str = menu->dname;
#if defined(FEAT_MULTI_LANG)
if (menu->en_dname == NULL)
should_advance = TRUE;
}
#endif
else
str = (char_u *)"";

#if defined(FEAT_MULTI_LANG)
if (should_advance)
#endif
{

menu = menu->next;
if (menu == NULL && !did_alt_menu)
{
menu = expand_menu_alt;
did_alt_menu = TRUE;
}
}

#if defined(FEAT_MULTI_LANG)
should_advance = !should_advance;
#endif

return str;
}





char_u *
get_menu_names(expand_T *xp UNUSED, int idx)
{
static vimmenu_T *menu = NULL;
static int did_alt_menu = FALSE;
#define TBUFFER_LEN 256
static char_u tbuffer[TBUFFER_LEN]; 
char_u *str;
#if defined(FEAT_MULTI_LANG)
static int should_advance = FALSE;
#endif

if (idx == 0) 
{
menu = expand_menu;
did_alt_menu = FALSE;
#if defined(FEAT_MULTI_LANG)
should_advance = FALSE;
#endif
}


while (menu != NULL
&& ( menu_is_hidden(menu->dname)
|| (expand_emenu && menu_is_separator(menu->dname))
|| menu_is_tearoff(menu->dname)
#if !defined(FEAT_BROWSE)
|| menu->dname[STRLEN(menu->dname) - 1] == '.'
#endif
))
{
menu = menu->next;
if (menu == NULL && !did_alt_menu)
{
menu = expand_menu_alt;
did_alt_menu = TRUE;
}
}

if (menu == NULL) 
return NULL;

if (menu->modes & expand_modes)
{
if (menu->children != NULL)
{
#if defined(FEAT_MULTI_LANG)
if (should_advance)
vim_strncpy(tbuffer, menu->en_dname, TBUFFER_LEN - 2);
else
{
#endif
vim_strncpy(tbuffer, menu->dname, TBUFFER_LEN - 2);
#if defined(FEAT_MULTI_LANG)
if (menu->en_dname == NULL)
should_advance = TRUE;
}
#endif


STRCAT(tbuffer, "\001");
str = tbuffer;
}
else
#if defined(FEAT_MULTI_LANG)
{
if (should_advance)
str = menu->en_dname;
else
{
#endif
str = menu->dname;
#if defined(FEAT_MULTI_LANG)
if (menu->en_dname == NULL)
should_advance = TRUE;
}
}
#endif
}
else
str = (char_u *)"";

#if defined(FEAT_MULTI_LANG)
if (should_advance)
#endif
{

menu = menu->next;
if (menu == NULL && !did_alt_menu)
{
menu = expand_menu_alt;
did_alt_menu = TRUE;
}
}

#if defined(FEAT_MULTI_LANG)
should_advance = !should_advance;
#endif

return str;
}






static char_u *
menu_name_skip(char_u *name)
{
char_u *p;

for (p = name; *p && *p != '.'; MB_PTR_ADV(p))
{
if (*p == '\\' || *p == Ctrl_V)
{
STRMOVE(p, p + 1);
if (*p == NUL)
break;
}
}
if (*p)
*p++ = NUL;
return p;
}





static int
menu_name_equal(char_u *name, vimmenu_T *menu)
{
#if defined(FEAT_MULTI_LANG)
if (menu->en_name != NULL
&& (menu_namecmp(name, menu->en_name)
|| menu_namecmp(name, menu->en_dname)))
return TRUE;
#endif
return menu_namecmp(name, menu->name) || menu_namecmp(name, menu->dname);
}

static int
menu_namecmp(char_u *name, char_u *mname)
{
int i;

for (i = 0; name[i] != NUL && name[i] != TAB; ++i)
if (name[i] != mname[i])
break;
return ((name[i] == NUL || name[i] == TAB)
&& (mname[i] == NUL || mname[i] == TAB));
}









static int
get_menu_cmd_modes(
char_u *cmd,
int forceit, 
int *noremap,
int *unmenu)
{
int modes;

switch (*cmd++)
{
case 'v': 
modes = MENU_VISUAL_MODE | MENU_SELECT_MODE;
break;
case 'x': 
modes = MENU_VISUAL_MODE;
break;
case 's': 
modes = MENU_SELECT_MODE;
break;
case 'o': 
modes = MENU_OP_PENDING_MODE;
break;
case 'i': 
modes = MENU_INSERT_MODE;
break;
case 't':
if (*cmd == 'l') 
{
modes = MENU_TERMINAL_MODE;
++cmd;
break;
}
modes = MENU_TIP_MODE; 
break;
case 'c': 
modes = MENU_CMDLINE_MODE;
break;
case 'a': 
modes = MENU_INSERT_MODE | MENU_CMDLINE_MODE | MENU_NORMAL_MODE
| MENU_VISUAL_MODE | MENU_SELECT_MODE
| MENU_OP_PENDING_MODE;
break;
case 'n':
if (*cmd != 'o') 
{
modes = MENU_NORMAL_MODE;
break;
}

default:
--cmd;
if (forceit) 
modes = MENU_INSERT_MODE | MENU_CMDLINE_MODE;
else 
modes = MENU_NORMAL_MODE | MENU_VISUAL_MODE | MENU_SELECT_MODE
| MENU_OP_PENDING_MODE;
}

if (noremap != NULL)
*noremap = (*cmd == 'n' ? REMAP_NONE : REMAP_YES);
if (unmenu != NULL)
*unmenu = (*cmd == 'u');
return modes;
}





static char_u *
get_menu_mode_str(int modes)
{
if ((modes & (MENU_INSERT_MODE | MENU_CMDLINE_MODE | MENU_NORMAL_MODE |
MENU_VISUAL_MODE | MENU_SELECT_MODE | MENU_OP_PENDING_MODE))
== (MENU_INSERT_MODE | MENU_CMDLINE_MODE | MENU_NORMAL_MODE |
MENU_VISUAL_MODE | MENU_SELECT_MODE | MENU_OP_PENDING_MODE))
return (char_u *)"a";
if ((modes & (MENU_NORMAL_MODE | MENU_VISUAL_MODE | MENU_SELECT_MODE |
MENU_OP_PENDING_MODE))
== (MENU_NORMAL_MODE | MENU_VISUAL_MODE | MENU_SELECT_MODE |
MENU_OP_PENDING_MODE))
return (char_u *)" ";
if ((modes & (MENU_INSERT_MODE | MENU_CMDLINE_MODE))
== (MENU_INSERT_MODE | MENU_CMDLINE_MODE))
return (char_u *)"!";
if ((modes & (MENU_VISUAL_MODE | MENU_SELECT_MODE))
== (MENU_VISUAL_MODE | MENU_SELECT_MODE))
return (char_u *)"v";
if (modes & MENU_VISUAL_MODE)
return (char_u *)"x";
if (modes & MENU_SELECT_MODE)
return (char_u *)"s";
if (modes & MENU_OP_PENDING_MODE)
return (char_u *)"o";
if (modes & MENU_INSERT_MODE)
return (char_u *)"i";
if (modes & MENU_TERMINAL_MODE)
return (char_u *)"tl";
if (modes & MENU_CMDLINE_MODE)
return (char_u *)"c";
if (modes & MENU_NORMAL_MODE)
return (char_u *)"n";
if (modes & MENU_TIP_MODE)
return (char_u *)"t";

return (char_u *)"";
}





static char_u *
popup_mode_name(char_u *name, int idx)
{
char_u *p;
int len = (int)STRLEN(name);
char *mode_chars = menu_mode_chars[idx];
int mode_chars_len = (int)strlen(mode_chars);
int i;

p = vim_strnsave(name, len + mode_chars_len);
if (p != NULL)
{
mch_memmove(p + 5 + mode_chars_len, p + 5, (size_t)(len - 4));
for (i = 0; i < mode_chars_len; ++i)
p[5 + i] = menu_mode_chars[idx][i];
}
return p;
}

#if defined(FEAT_GUI) || defined(PROTO)





int
get_menu_index(vimmenu_T *menu, int state)
{
int idx;

if ((state & INSERT))
idx = MENU_INDEX_INSERT;
else if (state & CMDLINE)
idx = MENU_INDEX_CMDLINE;
#if defined(FEAT_TERMINAL)
else if (term_use_loop())
idx = MENU_INDEX_TERMINAL;
#endif
else if (VIsual_active)
{
if (VIsual_select)
idx = MENU_INDEX_SELECT;
else
idx = MENU_INDEX_VISUAL;
}
else if (state == HITRETURN || state == ASKMORE)
idx = MENU_INDEX_CMDLINE;
else if (finish_op)
idx = MENU_INDEX_OP_PENDING;
else if ((state & NORMAL))
idx = MENU_INDEX_NORMAL;
else
idx = MENU_INDEX_INVALID;

if (idx != MENU_INDEX_INVALID && menu->strings[idx] == NULL)
idx = MENU_INDEX_INVALID;
return idx;
}
#endif








static char_u *
menu_text(char_u *str, int *mnemonic, char_u **actext)
{
char_u *p;
char_u *text;


p = vim_strchr(str, TAB);
if (p != NULL)
{
if (actext != NULL)
*actext = vim_strsave(p + 1);
text = vim_strnsave(str, (int)(p - str));
}
else
text = vim_strsave(str);


for (p = text; p != NULL; )
{
p = vim_strchr(p, '&');
if (p != NULL)
{
if (p[1] == NUL) 
break;
if (mnemonic != NULL && p[1] != '&')
#if !defined(__MVS__) || defined(MOTIF390_MNEMONIC_FIXED)
*mnemonic = p[1];
#else
{






char c = p[1];
__etoa_l(&c, 1);
*mnemonic = c;
}
#endif
STRMOVE(p, p + 1);
p = p + 1;
}
}
return text;
}




int
menu_is_menubar(char_u *name)
{
return (!menu_is_popup(name)
&& !menu_is_toolbar(name)
&& !menu_is_winbar(name)
&& *name != MNU_HIDDEN_CHAR);
}




int
menu_is_popup(char_u *name)
{
return (STRNCMP(name, "PopUp", 5) == 0);
}

#if (defined(FEAT_GUI_MOTIF) && (XmVersion <= 1002)) || defined(PROTO)



int
menu_is_child_of_popup(vimmenu_T *menu)
{
while (menu->parent != NULL)
menu = menu->parent;
return menu_is_popup(menu->name);
}
#endif




int
menu_is_toolbar(char_u *name)
{
return (STRNCMP(name, "ToolBar", 7) == 0);
}





int
menu_is_separator(char_u *name)
{
return (name[0] == '-' && name[STRLEN(name) - 1] == '-');
}




static int
menu_is_hidden(char_u *name)
{
return (name[0] == ']') || (menu_is_popup(name) && name[5] != NUL);
}




static int
menu_is_tearoff(char_u *name UNUSED)
{
#if defined(FEAT_GUI)
return (STRCMP(name, TEAR_STRING) == 0);
#else
return FALSE;
#endif
}

#if defined(FEAT_GUI) || defined(FEAT_TERM_POPUP_MENU) || defined(PROTO)

static int
get_menu_mode(void)
{
#if defined(FEAT_TERMINAL)
if (term_use_loop())
return MENU_INDEX_TERMINAL;
#endif
if (VIsual_active)
{
if (VIsual_select)
return MENU_INDEX_SELECT;
return MENU_INDEX_VISUAL;
}
if (State & INSERT)
return MENU_INDEX_INSERT;
if ((State & CMDLINE) || State == ASKMORE || State == HITRETURN)
return MENU_INDEX_CMDLINE;
if (finish_op)
return MENU_INDEX_OP_PENDING;
if (State & NORMAL)
return MENU_INDEX_NORMAL;
if (State & LANGMAP) 
return MENU_INDEX_INSERT;
return MENU_INDEX_INVALID;
}

int
get_menu_mode_flag(void)
{
int mode = get_menu_mode();

if (mode == MENU_INDEX_INVALID)
return 0;
return 1 << mode;
}






void
show_popupmenu(void)
{
vimmenu_T *menu;
int menu_mode;
char* mode;
int mode_len;

menu_mode = get_menu_mode();
if (menu_mode == MENU_INDEX_INVALID)
return;
mode = menu_mode_chars[menu_mode];
mode_len = (int)strlen(mode);

apply_autocmds(EVENT_MENUPOPUP, (char_u*)mode, NULL, FALSE, curbuf);

for (menu = root_menu; menu != NULL; menu = menu->next)
if (STRNCMP("PopUp", menu->name, 5) == 0 && STRNCMP(menu->name + 5, mode, mode_len) == 0)
break;


if (menu != NULL && menu->children != NULL)
{
#if defined(FEAT_GUI)
if (gui.in_use)
{


gui_update_menus(0);
gui_mch_show_popupmenu(menu);
}
#endif
#if defined(FEAT_GUI) && defined(FEAT_TERM_POPUP_MENU)
else
#endif
#if defined(FEAT_TERM_POPUP_MENU)
pum_show_popupmenu(menu);
#endif
}
}
#endif

#if defined(FEAT_GUI) || defined(PROTO)







int
check_menu_pointer(vimmenu_T *root, vimmenu_T *menu_to_check)
{
vimmenu_T *p;

for (p = root; p != NULL; p = p->next)
if (p == menu_to_check
|| (p->children != NULL
&& check_menu_pointer(p->children, menu_to_check) == OK))
return OK;
return FAIL;
}








void
gui_create_initial_menus(vimmenu_T *menu)
{
int idx = 0;

while (menu != NULL)
{

if (menu->modes & MENU_ALL_MODES)
{
if (menu->children != NULL)
{
gui_mch_add_menu(menu, idx);
gui_create_initial_menus(menu->children);
}
else
gui_mch_add_menu_item(menu, idx);
}
menu = menu->next;
++idx;
}
}




static void
gui_update_menus_recurse(vimmenu_T *menu, int mode)
{
int grey;

while (menu)
{
if ((menu->modes & menu->enabled & mode)
#if defined(FEAT_GUI_MSWIN) && defined(FEAT_TEAROFF)
|| menu_is_tearoff(menu->dname)
#endif
)
grey = FALSE;
else
grey = TRUE;
#if defined(FEAT_GUI_ATHENA)

gui_mch_menu_grey(menu, grey);
#else


if (vim_strchr(p_go, GO_GREY) != NULL || menu->parent == NULL
#if defined(FEAT_TOOLBAR)
|| menu_is_toolbar(menu->parent->name)
#endif
)
gui_mch_menu_grey(menu, grey);
else
gui_mch_menu_hidden(menu, grey);
#endif
gui_update_menus_recurse(menu->children, mode);
menu = menu->next;
}
}






void
gui_update_menus(int modes)
{
static int prev_mode = -1;
int mode = 0;

if (modes != 0x0)
mode = modes;
else
mode = get_menu_mode_flag();

if (force_menu_update || mode != prev_mode)
{
gui_update_menus_recurse(root_menu, mode);
gui_mch_draw_menubar();
prev_mode = mode;
force_menu_update = FALSE;
}
}

#if defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_GTK) || defined(FEAT_GUI_PHOTON) || defined(PROTO)





int
gui_is_menu_shortcut(int key)
{
vimmenu_T *menu;

if (key < 256)
key = TOLOWER_LOC(key);
for (menu = root_menu; menu != NULL; menu = menu->next)
if (menu->mnemonic == key
|| (menu->mnemonic < 256 && TOLOWER_LOC(menu->mnemonic) == key))
return TRUE;
return FALSE;
}
#endif
#endif 

#if (defined(FEAT_GUI_MSWIN) && defined(FEAT_TEAROFF)) || defined(PROTO)






void
gui_mch_toggle_tearoffs(int enable)
{
int pri_tab[MENUDEPTH + 1];
int i;

if (enable)
{
for (i = 0; i < MENUDEPTH; ++i)
pri_tab[i] = 500;
pri_tab[MENUDEPTH] = -1;
gui_create_tearoffs_recurse(root_menu, (char_u *)"", pri_tab, 0);
}
else
gui_destroy_tearoffs_recurse(root_menu);
s_tearoffs = enable;
}




static void
gui_create_tearoffs_recurse(
vimmenu_T *menu,
const char_u *pname,
int *pri_tab,
int pri_idx)
{
char_u *newpname = NULL;
int len;
char_u *s;
char_u *d;

if (pri_tab[pri_idx + 1] != -1)
++pri_idx;
while (menu != NULL)
{
if (menu->children != NULL && menu_is_menubar(menu->name))
{


len = (int)STRLEN(pname) + (int)STRLEN(menu->name);
for (s = menu->name; *s; ++s)
if (*s == '.' || *s == '\\')
++len;
newpname = alloc(len + TEAR_LEN + 2);
if (newpname != NULL)
{
STRCPY(newpname, pname);
d = newpname + STRLEN(newpname);
for (s = menu->name; *s; ++s)
{
if (*s == '.' || *s == '\\')
*d++ = '\\';
*d++ = *s;
}
*d = NUL;


if (STRCMP(menu->children->name, TEAR_STRING) != 0)
{
gui_add_tearoff(newpname, pri_tab, pri_idx - 1);
*d = NUL; 
}

STRCAT(newpname, ".");
gui_create_tearoffs_recurse(menu->children, newpname,
pri_tab, pri_idx);
vim_free(newpname);
}
}
menu = menu->next;
}
}





static void
gui_add_tearoff(char_u *tearpath, int *pri_tab, int pri_idx)
{
char_u *tbuf;
int t;
vimmenu_T menuarg;

tbuf = alloc(5 + (unsigned int)STRLEN(tearpath));
if (tbuf != NULL)
{
tbuf[0] = K_SPECIAL;
tbuf[1] = K_SECOND(K_TEAROFF);
tbuf[2] = K_THIRD(K_TEAROFF);
STRCPY(tbuf + 3, tearpath);
STRCAT(tbuf + 3, "\r");

STRCAT(tearpath, ".");
STRCAT(tearpath, TEAR_STRING);


t = pri_tab[pri_idx + 1];
pri_tab[pri_idx + 1] = 1;

#if defined(FEAT_TOOLBAR)
menuarg.iconfile = NULL;
menuarg.iconidx = -1;
menuarg.icon_builtin = FALSE;
#endif
menuarg.noremap[0] = REMAP_NONE;
menuarg.silent[0] = TRUE;

menuarg.modes = MENU_ALL_MODES;
add_menu_path(tearpath, &menuarg, pri_tab, tbuf, FALSE);

menuarg.modes = MENU_TIP_MODE;
add_menu_path(tearpath, &menuarg, pri_tab,
(char_u *)_("Tear off this menu"), FALSE);

pri_tab[pri_idx + 1] = t;
vim_free(tbuf);
}
}




static void
gui_destroy_tearoffs_recurse(vimmenu_T *menu)
{
while (menu)
{
if (menu->children)
{

if (STRCMP(menu->children->name, TEAR_STRING) == 0)
{

free_menu(&menu->children);
}
if (menu->children != NULL) 
gui_destroy_tearoffs_recurse(menu->children);
}
menu = menu->next;
}
}

#endif 







void
execute_menu(exarg_T *eap, vimmenu_T *menu, int mode_idx)
{
int idx = mode_idx;

if (idx < 0)
{

if (restart_edit
#if defined(FEAT_EVAL)
&& !current_sctx.sc_sid
#endif
)
{
idx = MENU_INDEX_INSERT;
}
#if defined(FEAT_TERMINAL)
else if (term_use_loop())
{
idx = MENU_INDEX_TERMINAL;
}
#endif
else if (VIsual_active)
{
idx = MENU_INDEX_VISUAL;
}
else if (eap != NULL && eap->addr_count)
{
pos_T tpos;

idx = MENU_INDEX_VISUAL;





if ((curbuf->b_visual.vi_start.lnum == eap->line1)
&& (curbuf->b_visual.vi_end.lnum) == eap->line2)
{

VIsual_mode = curbuf->b_visual.vi_mode;
tpos = curbuf->b_visual.vi_end;
curwin->w_cursor = curbuf->b_visual.vi_start;
curwin->w_curswant = curbuf->b_visual.vi_curswant;
}
else
{

VIsual_mode = 'V';
curwin->w_cursor.lnum = eap->line1;
curwin->w_cursor.col = 1;
tpos.lnum = eap->line2;
tpos.col = MAXCOL;
tpos.coladd = 0;
}


VIsual_active = TRUE;
VIsual_reselect = TRUE;
check_cursor();
VIsual = curwin->w_cursor;
curwin->w_cursor = tpos;

check_cursor();



if (*p_sel == 'e' && gchar_cursor() != NUL)
++curwin->w_cursor.col;
}
}


if (idx == -1 || eap == NULL)
idx = MENU_INDEX_NORMAL;

if (idx != MENU_INDEX_INVALID && menu->strings[idx] != NULL
&& (menu->modes & (1 << idx)))
{



if (eap == NULL
#if defined(FEAT_EVAL)
|| current_sctx.sc_sid != 0
#endif
)
{
save_state_T save_state;

++ex_normal_busy;
if (save_current_state(&save_state))
exec_normal_cmd(menu->strings[idx], menu->noremap[idx],
menu->silent[idx]);
restore_current_state(&save_state);
--ex_normal_busy;
}
else
ins_typebuf(menu->strings[idx], menu->noremap[idx], 0,
TRUE, menu->silent[idx]);
}
else if (eap != NULL)
{
char_u *mode;

switch (idx)
{
case MENU_INDEX_VISUAL:
mode = (char_u *)"Visual";
break;
case MENU_INDEX_SELECT:
mode = (char_u *)"Select";
break;
case MENU_INDEX_OP_PENDING:
mode = (char_u *)"Op-pending";
break;
case MENU_INDEX_TERMINAL:
mode = (char_u *)"Terminal";
break;
case MENU_INDEX_INSERT:
mode = (char_u *)"Insert";
break;
case MENU_INDEX_CMDLINE:
mode = (char_u *)"Cmdline";
break;

default:
mode = (char_u *)"Normal";
}
semsg(_("E335: Menu not defined for %s mode"), mode);
}
}





static vimmenu_T *
menu_getbyname(char_u *name_arg)
{
char_u *name;
char_u *saved_name;
vimmenu_T *menu;
char_u *p;
int gave_emsg = FALSE;

saved_name = vim_strsave(name_arg);
if (saved_name == NULL)
return NULL;

menu = *get_root_menu(saved_name);
name = saved_name;
while (*name)
{

p = menu_name_skip(name);

while (menu != NULL)
{
if (menu_name_equal(name, menu))
{
if (*p == NUL && menu->children != NULL)
{
emsg(_("E333: Menu path must lead to a menu item"));
gave_emsg = TRUE;
menu = NULL;
}
else if (*p != NUL && menu->children == NULL)
{
emsg(_(e_notsubmenu));
menu = NULL;
}
break;
}
menu = menu->next;
}
if (menu == NULL || *p == NUL)
break;
menu = menu->children;
name = p;
}
vim_free(saved_name);
if (menu == NULL)
{
if (!gave_emsg)
semsg(_("E334: Menu not found: %s"), name_arg);
return NULL;
}

return menu;
}





void
ex_emenu(exarg_T *eap)
{
vimmenu_T *menu;
char_u *arg = eap->arg;
int mode_idx = -1;

if (arg[0] && VIM_ISWHITE(arg[1]))
{
switch (arg[0])
{
case 'n': mode_idx = MENU_INDEX_NORMAL; break;
case 'v': mode_idx = MENU_INDEX_VISUAL; break;
case 's': mode_idx = MENU_INDEX_SELECT; break;
case 'o': mode_idx = MENU_INDEX_OP_PENDING; break;
case 't': mode_idx = MENU_INDEX_TERMINAL; break;
case 'i': mode_idx = MENU_INDEX_INSERT; break;
case 'c': mode_idx = MENU_INDEX_CMDLINE; break;
default: semsg(_(e_invarg2), arg);
return;
}
arg = skipwhite(arg + 2);
}

menu = menu_getbyname(arg);
if (menu == NULL)
return;


execute_menu(eap, menu, mode_idx);
}




void
winbar_click(win_T *wp, int col)
{
int idx;

if (wp->w_winbar_items == NULL)
return;
for (idx = 0; wp->w_winbar_items[idx].wb_menu != NULL; ++idx)
{
winbar_item_T *item = &wp->w_winbar_items[idx];

if (col >= item->wb_startcol && col <= item->wb_endcol)
{
win_T *save_curwin = NULL;
pos_T save_visual = VIsual;
int save_visual_active = VIsual_active;
int save_visual_select = VIsual_select;
int save_visual_reselect = VIsual_reselect;
int save_visual_mode = VIsual_mode;

if (wp != curwin)
{


save_curwin = curwin;
VIsual_active = FALSE;
curwin = wp;
curbuf = curwin->w_buffer;
check_cursor();
}


execute_menu(NULL, item->wb_menu, -1);

if (save_curwin != NULL && win_valid(save_curwin))
{
curwin = save_curwin;
curbuf = curwin->w_buffer;
VIsual = save_visual;
VIsual_active = save_visual_active;
VIsual_select = save_visual_select;
VIsual_reselect = save_visual_reselect;
VIsual_mode = save_visual_mode;
}
if (!win_valid(wp))
break;
}
}
}

#if defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_GTK) || defined(FEAT_TERM_POPUP_MENU) || defined(FEAT_GUI_HAIKU) || defined(FEAT_BEVAL_TIP) || defined(PROTO)





vimmenu_T *
gui_find_menu(char_u *path_name)
{
vimmenu_T *menu = NULL;
char_u *name;
char_u *saved_name;
char_u *p;

menu = *get_root_menu(path_name);

saved_name = vim_strsave(path_name);
if (saved_name == NULL)
return NULL;

name = saved_name;
while (*name)
{

p = menu_name_skip(name);

while (menu != NULL)
{
if (menu_name_equal(name, menu))
{
if (menu->children == NULL)
{

if (*p == NUL)
emsg(_("E336: Menu path must lead to a sub-menu"));
else
emsg(_(e_notsubmenu));
menu = NULL;
goto theend;
}
if (*p == NUL) 
goto theend;
break;
}
menu = menu->next;
}
if (menu == NULL) 
break;


menu = menu->children;
name = p;
}

if (menu == NULL)
emsg(_("E337: Menu not found - check menu names"));
theend:
vim_free(saved_name);
return menu;
}
#endif

#if defined(FEAT_MULTI_LANG)




typedef struct
{
char_u *from; 
char_u *from_noamp; 
char_u *to; 
} menutrans_T;

static garray_T menutrans_ga = {0, 0, 0, 0, NULL};
#endif






void
ex_menutranslate(exarg_T *eap UNUSED)
{
#if defined(FEAT_MULTI_LANG)
char_u *arg = eap->arg;
menutrans_T *tp;
int i;
char_u *from, *from_noamp, *to;

if (menutrans_ga.ga_itemsize == 0)
ga_init2(&menutrans_ga, (int)sizeof(menutrans_T), 5);




if (STRNCMP(arg, "clear", 5) == 0 && ends_excmd(*skipwhite(arg + 5)))
{
tp = (menutrans_T *)menutrans_ga.ga_data;
for (i = 0; i < menutrans_ga.ga_len; ++i)
{
vim_free(tp[i].from);
vim_free(tp[i].from_noamp);
vim_free(tp[i].to);
}
ga_clear(&menutrans_ga);
#if defined(FEAT_EVAL)

del_menutrans_vars();
#endif
}
else
{

from = arg;
arg = menu_skip_part(arg);
to = skipwhite(arg);
*arg = NUL;
arg = menu_skip_part(to);
if (arg == to)
emsg(_(e_invarg));
else
{
if (ga_grow(&menutrans_ga, 1) == OK)
{
tp = (menutrans_T *)menutrans_ga.ga_data;
from = vim_strsave(from);
if (from != NULL)
{
from_noamp = menu_text(from, NULL, NULL);
to = vim_strnsave(to, (int)(arg - to));
if (from_noamp != NULL && to != NULL)
{
menu_translate_tab_and_shift(from);
menu_translate_tab_and_shift(to);
menu_unescape_name(from);
menu_unescape_name(to);
tp[menutrans_ga.ga_len].from = from;
tp[menutrans_ga.ga_len].from_noamp = from_noamp;
tp[menutrans_ga.ga_len].to = to;
++menutrans_ga.ga_len;
}
else
{
vim_free(from);
vim_free(from_noamp);
vim_free(to);
}
}
}
}
}
#endif
}

#if defined(FEAT_MULTI_LANG) || defined(FEAT_TOOLBAR)



static char_u *
menu_skip_part(char_u *p)
{
while (*p != NUL && *p != '.' && !VIM_ISWHITE(*p))
{
if ((*p == '\\' || *p == Ctrl_V) && p[1] != NUL)
++p;
++p;
}
return p;
}
#endif

#if defined(FEAT_MULTI_LANG)




static char_u *
menutrans_lookup(char_u *name, int len)
{
menutrans_T *tp = (menutrans_T *)menutrans_ga.ga_data;
int i;
char_u *dname;

for (i = 0; i < menutrans_ga.ga_len; ++i)
if (STRNICMP(name, tp[i].from, len) == 0 && tp[i].from[len] == NUL)
return tp[i].to;


i = name[len];
name[len] = NUL;
dname = menu_text(name, NULL, NULL);
name[len] = i;
if (dname != NULL)
{
for (i = 0; i < menutrans_ga.ga_len; ++i)
if (STRICMP(dname, tp[i].from_noamp) == 0)
{
vim_free(dname);
return tp[i].to;
}
vim_free(dname);
}

return NULL;
}




static void
menu_unescape_name(char_u *name)
{
char_u *p;

for (p = name; *p && *p != '.'; MB_PTR_ADV(p))
if (*p == '\\')
STRMOVE(p, p + 1);
}
#endif 





static char_u *
menu_translate_tab_and_shift(char_u *arg_start)
{
char_u *arg = arg_start;

while (*arg && !VIM_ISWHITE(*arg))
{
if ((*arg == '\\' || *arg == Ctrl_V) && arg[1] != NUL)
arg++;
else if (STRNICMP(arg, "<TAB>", 5) == 0)
{
*arg = TAB;
STRMOVE(arg + 1, arg + 5);
}
arg++;
}
if (*arg != NUL)
*arg++ = NUL;
arg = skipwhite(arg);

return arg;
}




static int
menuitem_getinfo(vimmenu_T *menu, int modes, dict_T *dict)
{
int status;

if (menu_is_tearoff(menu->dname)) 
return OK;

status = dict_add_string(dict, "name", menu->name);
if (status == OK)
status = dict_add_string(dict, "display", menu->dname);
if (status == OK && menu->actext != NULL)
status = dict_add_string(dict, "accel", menu->actext);
if (status == OK)
status = dict_add_number(dict, "priority", menu->priority);
if (status == OK)
status = dict_add_string(dict, "modes",
get_menu_mode_str(menu->modes));
#if defined(FEAT_TOOLBAR)
if (status == OK && menu->iconfile != NULL)
status = dict_add_string(dict, "icon", menu->iconfile);
if (status == OK && menu->iconidx >= 0)
status = dict_add_number(dict, "iconidx", menu->iconidx);
#endif
if (status == OK)
{
char_u buf[NUMBUFLEN];

if (has_mbyte)
buf[utf_char2bytes(menu->mnemonic, buf)] = NUL;
else
{
buf[0] = (char_u)menu->mnemonic;
buf[1] = NUL;
}
status = dict_add_string(dict, "shortcut", buf);
}
if (status == OK && menu->children == NULL)
{
int bit;


for (bit = 0; bit < MENU_MODES && !((1 << bit) & modes); bit++)
;
if (bit < MENU_MODES) 
{
if (menu->strings[bit] != NULL)
{
char_u *tofree = NULL;

status = dict_add_string(dict, "rhs",
*menu->strings[bit] == NUL
? (char_u *)"<Nop>"
: (tofree = str2special_save(
menu->strings[bit], FALSE)));
vim_free(tofree);
}
if (status == OK)
status = dict_add_bool(dict, "noremenu",
menu->noremap[bit] == REMAP_NONE);
if (status == OK)
status = dict_add_bool(dict, "script",
menu->noremap[bit] == REMAP_SCRIPT);
if (status == OK)
status = dict_add_bool(dict, "silent", menu->silent[bit]);
if (status == OK)
status = dict_add_bool(dict, "enabled",
((menu->enabled & (1 << bit)) != 0));
}
}


if (status == OK && menu->children != NULL)
{
list_T *l = list_alloc();
vimmenu_T *child;

if (l == NULL)
return FAIL;

dict_add_list(dict, "submenus", l);
child = menu->children;
while (child)
{
if (!menu_is_tearoff(child->dname)) 
list_append_string(l, child->dname, -1);
child = child->next;
}
}

return status;
}





void
f_menu_info(typval_T *argvars, typval_T *rettv)
{
char_u *menu_name;
char_u *which;
int modes;
char_u *saved_name;
char_u *name;
vimmenu_T *menu;
dict_T *retdict;

if (rettv_dict_alloc(rettv) != OK)
return;
retdict = rettv->vval.v_dict;

menu_name = tv_get_string_chk(&argvars[0]);
if (menu_name == NULL)
return;


if (argvars[1].v_type != VAR_UNKNOWN)
which = tv_get_string_chk(&argvars[1]);
else
which = (char_u *)""; 
if (which == NULL)
return;

modes = get_menu_cmd_modes(which, *which == '!', NULL, NULL);


menu = *get_root_menu(menu_name);
saved_name = vim_strsave(menu_name);
if (saved_name == NULL)
return;
if (*saved_name != NUL)
{
char_u *p;

name = saved_name;
while (*name)
{

p = menu_name_skip(name);
while (menu != NULL)
{
if (menu_name_equal(name, menu))
break;
menu = menu->next;
}
if (menu == NULL || *p == NUL)
break;
menu = menu->children;
name = p;
}
}
vim_free(saved_name);

if (menu == NULL) 
return;

if (menu->modes & modes)
menuitem_getinfo(menu, modes, retdict);
}

#endif 
