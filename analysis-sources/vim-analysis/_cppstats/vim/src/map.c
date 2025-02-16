#include "vim.h"
static mapblock_T *first_abbr = NULL; 
static mapblock_T *(maphash[256]);
static int maphash_valid = FALSE;
#define MAP_HASH(mode, c1) (((mode) & (NORMAL + VISUAL + SELECTMODE + OP_PENDING + TERMINAL)) ? (c1) : ((c1) ^ 0x80))
mapblock_T *
get_maphash_list(int state, int c)
{
return maphash[MAP_HASH(state, c)];
}
mapblock_T *
get_buf_maphash_list(int state, int c)
{
return curbuf->b_maphash[MAP_HASH(state, c)];
}
int
is_maphash_valid(void)
{
return maphash_valid;
}
static void
validate_maphash(void)
{
if (!maphash_valid)
{
vim_memset(maphash, 0, sizeof(maphash));
maphash_valid = TRUE;
}
}
static void
map_free(mapblock_T **mpp)
{
mapblock_T *mp;
mp = *mpp;
vim_free(mp->m_keys);
vim_free(mp->m_str);
vim_free(mp->m_orig_str);
*mpp = mp->m_next;
vim_free(mp);
}
static char_u *
map_mode_to_chars(int mode)
{
garray_T mapmode;
ga_init2(&mapmode, 1, 7);
if ((mode & (INSERT + CMDLINE)) == INSERT + CMDLINE)
ga_append(&mapmode, '!'); 
else if (mode & INSERT)
ga_append(&mapmode, 'i'); 
else if (mode & LANGMAP)
ga_append(&mapmode, 'l'); 
else if (mode & CMDLINE)
ga_append(&mapmode, 'c'); 
else if ((mode & (NORMAL + VISUAL + SELECTMODE + OP_PENDING))
== NORMAL + VISUAL + SELECTMODE + OP_PENDING)
ga_append(&mapmode, ' '); 
else
{
if (mode & NORMAL)
ga_append(&mapmode, 'n'); 
if (mode & OP_PENDING)
ga_append(&mapmode, 'o'); 
if (mode & TERMINAL)
ga_append(&mapmode, 't'); 
if ((mode & (VISUAL + SELECTMODE)) == VISUAL + SELECTMODE)
ga_append(&mapmode, 'v'); 
else
{
if (mode & VISUAL)
ga_append(&mapmode, 'x'); 
if (mode & SELECTMODE)
ga_append(&mapmode, 's'); 
}
}
ga_append(&mapmode, NUL);
return (char_u *)mapmode.ga_data;
}
static void
showmap(
mapblock_T *mp,
int local) 
{
int len = 1;
char_u *mapchars;
if (message_filtered(mp->m_keys) && message_filtered(mp->m_str))
return;
if (msg_didout || msg_silent != 0)
{
msg_putchar('\n');
if (got_int) 
return;
}
mapchars = map_mode_to_chars(mp->m_mode);
if (mapchars != NULL)
{
msg_puts((char *)mapchars);
len = (int)STRLEN(mapchars);
vim_free(mapchars);
}
while (++len <= 3)
msg_putchar(' ');
len = msg_outtrans_special(mp->m_keys, TRUE, 0);
do
{
msg_putchar(' '); 
++len;
} while (len < 12);
if (mp->m_noremap == REMAP_NONE)
msg_puts_attr("*", HL_ATTR(HLF_8));
else if (mp->m_noremap == REMAP_SCRIPT)
msg_puts_attr("&", HL_ATTR(HLF_8));
else
msg_putchar(' ');
if (local)
msg_putchar('@');
else
msg_putchar(' ');
if (*mp->m_str == NUL)
msg_puts_attr("<Nop>", HL_ATTR(HLF_8));
else
{
char_u *s = vim_strsave(mp->m_str);
if (s != NULL)
{
vim_unescape_csi(s);
msg_outtrans_special(s, FALSE, 0);
vim_free(s);
}
}
#if defined(FEAT_EVAL)
if (p_verbose > 0)
last_set_msg(mp->m_script_ctx);
#endif
out_flush(); 
}
int
do_map(
int maptype,
char_u *arg,
int mode,
int abbrev) 
{
char_u *keys;
mapblock_T *mp, **mpp;
char_u *rhs;
char_u *p;
int n;
int len = 0; 
int hasarg;
int haskey;
int do_print;
int keyround;
char_u *keys_buf = NULL;
char_u *alt_keys_buf = NULL;
char_u *arg_buf = NULL;
int retval = 0;
int do_backslash;
mapblock_T **abbr_table;
mapblock_T **map_table;
int unique = FALSE;
int nowait = FALSE;
int silent = FALSE;
int special = FALSE;
#if defined(FEAT_EVAL)
int expr = FALSE;
#endif
int did_simplify = FALSE;
int noremap;
char_u *orig_rhs;
keys = arg;
map_table = maphash;
abbr_table = &first_abbr;
if (maptype == 2)
noremap = REMAP_NONE;
else
noremap = REMAP_YES;
for (;;)
{
if (STRNCMP(keys, "<buffer>", 8) == 0)
{
keys = skipwhite(keys + 8);
map_table = curbuf->b_maphash;
abbr_table = &curbuf->b_first_abbr;
continue;
}
if (STRNCMP(keys, "<nowait>", 8) == 0)
{
keys = skipwhite(keys + 8);
nowait = TRUE;
continue;
}
if (STRNCMP(keys, "<silent>", 8) == 0)
{
keys = skipwhite(keys + 8);
silent = TRUE;
continue;
}
if (STRNCMP(keys, "<special>", 9) == 0)
{
keys = skipwhite(keys + 9);
special = TRUE;
continue;
}
#if defined(FEAT_EVAL)
if (STRNCMP(keys, "<script>", 8) == 0)
{
keys = skipwhite(keys + 8);
noremap = REMAP_SCRIPT;
continue;
}
if (STRNCMP(keys, "<expr>", 6) == 0)
{
keys = skipwhite(keys + 6);
expr = TRUE;
continue;
}
#endif
if (STRNCMP(keys, "<unique>", 8) == 0)
{
keys = skipwhite(keys + 8);
unique = TRUE;
continue;
}
break;
}
validate_maphash();
p = keys;
do_backslash = (vim_strchr(p_cpo, CPO_BSLASH) == NULL);
while (*p && (maptype == 1 || !VIM_ISWHITE(*p)))
{
if ((p[0] == Ctrl_V || (do_backslash && p[0] == '\\')) &&
p[1] != NUL)
++p; 
++p;
}
if (*p != NUL)
*p++ = NUL;
p = skipwhite(p);
rhs = p;
hasarg = (*rhs != NUL);
haskey = (*keys != NUL);
do_print = !haskey || (maptype != 1 && !hasarg);
if (maptype == 1 && !haskey)
{
retval = 1;
goto theend;
}
if (haskey)
{
char_u *new_keys;
int flags = REPTERM_FROM_PART | REPTERM_DO_LT;
if (special)
flags |= REPTERM_SPECIAL;
new_keys = replace_termcodes(keys, &keys_buf, flags, &did_simplify);
if (did_simplify)
(void)replace_termcodes(keys, &alt_keys_buf,
flags | REPTERM_NO_SIMPLIFY, NULL);
keys = new_keys;
}
orig_rhs = rhs;
if (hasarg)
{
if (STRICMP(rhs, "<nop>") == 0) 
rhs = (char_u *)"";
else
rhs = replace_termcodes(rhs, &arg_buf,
REPTERM_DO_LT | (special ? REPTERM_SPECIAL : 0), NULL);
}
for (keyround = 1; keyround <= 2; ++keyround)
{
int did_it = FALSE;
int did_local = FALSE;
int round;
int hash;
int new_hash;
if (keyround == 2)
{
if (alt_keys_buf == NULL)
break;
keys = alt_keys_buf;
}
else if (alt_keys_buf != NULL && do_print)
keys = alt_keys_buf;
if (haskey)
{
len = (int)STRLEN(keys);
if (len > MAXMAPLEN) 
{
retval = 1;
goto theend;
}
if (abbrev && maptype != 1)
{
if (has_mbyte)
{
int first, last;
int same = -1;
first = vim_iswordp(keys);
last = first;
p = keys + (*mb_ptr2len)(keys);
n = 1;
while (p < keys + len)
{
++n; 
last = vim_iswordp(p); 
if (same == -1 && last != first)
same = n - 1; 
p += (*mb_ptr2len)(p);
}
if (last && n > 2 && same >= 0 && same < n - 1)
{
retval = 1;
goto theend;
}
}
else if (vim_iswordc(keys[len - 1]))
for (n = 0; n < len - 2; ++n)
if (vim_iswordc(keys[n]) != vim_iswordc(keys[len - 2]))
{
retval = 1;
goto theend;
}
for (n = 0; n < len; ++n)
if (VIM_ISWHITE(keys[n]))
{
retval = 1;
goto theend;
}
}
}
if (haskey && hasarg && abbrev) 
no_abbr = FALSE; 
if (do_print)
msg_start();
if (map_table == curbuf->b_maphash && haskey && hasarg && maptype != 1)
{
for (hash = 0; hash < 256 && !got_int; ++hash)
{
if (abbrev)
{
if (hash != 0) 
break;
mp = first_abbr;
}
else
mp = maphash[hash];
for ( ; mp != NULL && !got_int; mp = mp->m_next)
{
if ((mp->m_mode & mode) != 0
&& mp->m_keylen == len
&& unique
&& STRNCMP(mp->m_keys, keys, (size_t)len) == 0)
{
if (abbrev)
semsg(_(
"E224: global abbreviation already exists for %s"),
mp->m_keys);
else
semsg(_(
"E225: global mapping already exists for %s"),
mp->m_keys);
retval = 5;
goto theend;
}
}
}
}
if (map_table != curbuf->b_maphash && !hasarg && maptype != 1)
{
for (hash = 0; hash < 256 && !got_int; ++hash)
{
if (abbrev)
{
if (hash != 0) 
break;
mp = curbuf->b_first_abbr;
}
else
mp = curbuf->b_maphash[hash];
for ( ; mp != NULL && !got_int; mp = mp->m_next)
{
if (!mp->m_simplified && (mp->m_mode & mode) != 0)
{
if (!haskey) 
{
showmap(mp, TRUE);
did_local = TRUE;
}
else
{
n = mp->m_keylen;
if (STRNCMP(mp->m_keys, keys,
(size_t)(n < len ? n : len)) == 0)
{
showmap(mp, TRUE);
did_local = TRUE;
}
}
}
}
}
}
for (round = 0; (round == 0 || maptype == 1) && round <= 1
&& !did_it && !got_int; ++round)
{
for (hash = 0; hash < 256 && !got_int; ++hash)
{
if (abbrev)
{
if (hash > 0) 
break;
mpp = abbr_table;
}
else
mpp = &(map_table[hash]);
for (mp = *mpp; mp != NULL && !got_int; mp = *mpp)
{
if ((mp->m_mode & mode) == 0)
{
mpp = &(mp->m_next);
continue;
}
if (!haskey) 
{
if (!mp->m_simplified)
{
showmap(mp, map_table != maphash);
did_it = TRUE;
}
}
else 
{
if (round) 
{
n = (int)STRLEN(mp->m_str);
p = mp->m_str;
}
else
{
n = mp->m_keylen;
p = mp->m_keys;
}
if (STRNCMP(p, keys, (size_t)(n < len ? n : len)) == 0)
{
if (maptype == 1)
{
if (n != len && (!abbrev || round || n > len
|| *skipwhite(keys + n) != NUL))
{
mpp = &(mp->m_next);
continue;
}
mp->m_mode &= ~mode;
did_it = TRUE; 
}
else if (!hasarg) 
{
if (!mp->m_simplified)
{
showmap(mp, map_table != maphash);
did_it = TRUE;
}
}
else if (n != len) 
{
mpp = &(mp->m_next);
continue;
}
else if (unique)
{
if (abbrev)
semsg(_(
"E226: abbreviation already exists for %s"),
p);
else
semsg(_(
"E227: mapping already exists for %s"),
p);
retval = 5;
goto theend;
}
else
{
mp->m_mode &= ~mode; 
if (mp->m_mode == 0 && !did_it) 
{
char_u *newstr = vim_strsave(rhs);
if (newstr == NULL)
{
retval = 4; 
goto theend;
}
vim_free(mp->m_str);
mp->m_str = newstr;
vim_free(mp->m_orig_str);
mp->m_orig_str = vim_strsave(orig_rhs);
mp->m_noremap = noremap;
mp->m_nowait = nowait;
mp->m_silent = silent;
mp->m_mode = mode;
mp->m_simplified =
did_simplify && keyround == 1;
#if defined(FEAT_EVAL)
mp->m_expr = expr;
mp->m_script_ctx = current_sctx;
mp->m_script_ctx.sc_lnum += SOURCING_LNUM;
#endif
did_it = TRUE;
}
}
if (mp->m_mode == 0) 
{
map_free(mpp);
continue; 
}
new_hash = MAP_HASH(mp->m_mode, mp->m_keys[0]);
if (!abbrev && new_hash != hash)
{
*mpp = mp->m_next;
mp->m_next = map_table[new_hash];
map_table[new_hash] = mp;
continue; 
}
}
}
mpp = &(mp->m_next);
}
}
}
if (maptype == 1)
{
if (!did_it)
retval = 2; 
else if (*keys == Ctrl_C)
{
if (map_table == curbuf->b_maphash)
curbuf->b_mapped_ctrl_c &= ~mode;
else
mapped_ctrl_c &= ~mode;
}
continue;
}
if (!haskey || !hasarg)
{
if (!did_it && !did_local)
{
if (abbrev)
msg(_("No abbreviation found"));
else
msg(_("No mapping found"));
}
goto theend; 
}
if (did_it)
continue; 
mp = ALLOC_ONE(mapblock_T);
if (mp == NULL)
{
retval = 4; 
goto theend;
}
if (*keys == Ctrl_C)
{
if (map_table == curbuf->b_maphash)
curbuf->b_mapped_ctrl_c |= mode;
else
mapped_ctrl_c |= mode;
}
mp->m_keys = vim_strsave(keys);
mp->m_str = vim_strsave(rhs);
mp->m_orig_str = vim_strsave(orig_rhs);
if (mp->m_keys == NULL || mp->m_str == NULL)
{
vim_free(mp->m_keys);
vim_free(mp->m_str);
vim_free(mp->m_orig_str);
vim_free(mp);
retval = 4; 
goto theend;
}
mp->m_keylen = (int)STRLEN(mp->m_keys);
mp->m_noremap = noremap;
mp->m_nowait = nowait;
mp->m_silent = silent;
mp->m_mode = mode;
mp->m_simplified = did_simplify && keyround == 1;
#if defined(FEAT_EVAL)
mp->m_expr = expr;
mp->m_script_ctx = current_sctx;
mp->m_script_ctx.sc_lnum += SOURCING_LNUM;
#endif
if (abbrev)
{
mp->m_next = *abbr_table;
*abbr_table = mp;
}
else
{
n = MAP_HASH(mp->m_mode, mp->m_keys[0]);
mp->m_next = map_table[n];
map_table[n] = mp;
}
}
theend:
vim_free(keys_buf);
vim_free(alt_keys_buf);
vim_free(arg_buf);
return retval;
}
static int
get_map_mode(char_u **cmdp, int forceit)
{
char_u *p;
int modec;
int mode;
p = *cmdp;
modec = *p++;
if (modec == 'i')
mode = INSERT; 
else if (modec == 'l')
mode = LANGMAP; 
else if (modec == 'c')
mode = CMDLINE; 
else if (modec == 'n' && *p != 'o') 
mode = NORMAL; 
else if (modec == 'v')
mode = VISUAL + SELECTMODE; 
else if (modec == 'x')
mode = VISUAL; 
else if (modec == 's')
mode = SELECTMODE; 
else if (modec == 'o')
mode = OP_PENDING; 
else if (modec == 't')
mode = TERMINAL; 
else
{
--p;
if (forceit)
mode = INSERT + CMDLINE; 
else
mode = VISUAL + SELECTMODE + NORMAL + OP_PENDING;
}
*cmdp = p;
return mode;
}
static void
map_clear(
char_u *cmdp,
char_u *arg UNUSED,
int forceit,
int abbr)
{
int mode;
int local;
local = (STRCMP(arg, "<buffer>") == 0);
if (!local && *arg != NUL)
{
emsg(_(e_invarg));
return;
}
mode = get_map_mode(&cmdp, forceit);
map_clear_int(curbuf, mode, local, abbr);
}
void
map_clear_int(
buf_T *buf, 
int mode, 
int local, 
int abbr) 
{
mapblock_T *mp, **mpp;
int hash;
int new_hash;
validate_maphash();
for (hash = 0; hash < 256; ++hash)
{
if (abbr)
{
if (hash > 0) 
break;
if (local)
mpp = &buf->b_first_abbr;
else
mpp = &first_abbr;
}
else
{
if (local)
mpp = &buf->b_maphash[hash];
else
mpp = &maphash[hash];
}
while (*mpp != NULL)
{
mp = *mpp;
if (mp->m_mode & mode)
{
mp->m_mode &= ~mode;
if (mp->m_mode == 0) 
{
map_free(mpp);
continue;
}
new_hash = MAP_HASH(mp->m_mode, mp->m_keys[0]);
if (!abbr && new_hash != hash)
{
*mpp = mp->m_next;
if (local)
{
mp->m_next = buf->b_maphash[new_hash];
buf->b_maphash[new_hash] = mp;
}
else
{
mp->m_next = maphash[new_hash];
maphash[new_hash] = mp;
}
continue; 
}
}
mpp = &(mp->m_next);
}
}
}
#if defined(FEAT_EVAL) || defined(PROTO)
int
mode_str2flags(char_u *modechars)
{
int mode = 0;
if (vim_strchr(modechars, 'n') != NULL)
mode |= NORMAL;
if (vim_strchr(modechars, 'v') != NULL)
mode |= VISUAL + SELECTMODE;
if (vim_strchr(modechars, 'x') != NULL)
mode |= VISUAL;
if (vim_strchr(modechars, 's') != NULL)
mode |= SELECTMODE;
if (vim_strchr(modechars, 'o') != NULL)
mode |= OP_PENDING;
if (vim_strchr(modechars, 'i') != NULL)
mode |= INSERT;
if (vim_strchr(modechars, 'l') != NULL)
mode |= LANGMAP;
if (vim_strchr(modechars, 'c') != NULL)
mode |= CMDLINE;
return mode;
}
int
map_to_exists(char_u *str, char_u *modechars, int abbr)
{
char_u *rhs;
char_u *buf;
int retval;
rhs = replace_termcodes(str, &buf, REPTERM_DO_LT, NULL);
retval = map_to_exists_mode(rhs, mode_str2flags(modechars), abbr);
vim_free(buf);
return retval;
}
#endif
int
map_to_exists_mode(char_u *rhs, int mode, int abbr)
{
mapblock_T *mp;
int hash;
int exp_buffer = FALSE;
validate_maphash();
for (;;)
{
for (hash = 0; hash < 256; ++hash)
{
if (abbr)
{
if (hash > 0) 
break;
if (exp_buffer)
mp = curbuf->b_first_abbr;
else
mp = first_abbr;
}
else if (exp_buffer)
mp = curbuf->b_maphash[hash];
else
mp = maphash[hash];
for (; mp; mp = mp->m_next)
{
if ((mp->m_mode & mode)
&& strstr((char *)mp->m_str, (char *)rhs) != NULL)
return TRUE;
}
}
if (exp_buffer)
break;
exp_buffer = TRUE;
}
return FALSE;
}
static int expand_mapmodes = 0;
static int expand_isabbrev = 0;
static int expand_buffer = FALSE;
static char_u *
translate_mapping(char_u *str)
{
garray_T ga;
int c;
int modifiers;
int cpo_bslash;
int cpo_special;
ga_init(&ga);
ga.ga_itemsize = 1;
ga.ga_growsize = 40;
cpo_bslash = (vim_strchr(p_cpo, CPO_BSLASH) != NULL);
cpo_special = (vim_strchr(p_cpo, CPO_SPECI) != NULL);
for (; *str; ++str)
{
c = *str;
if (c == K_SPECIAL && str[1] != NUL && str[2] != NUL)
{
modifiers = 0;
if (str[1] == KS_MODIFIER)
{
str++;
modifiers = *++str;
c = *++str;
}
if (c == K_SPECIAL && str[1] != NUL && str[2] != NUL)
{
if (cpo_special)
{
ga_clear(&ga);
return NULL;
}
c = TO_SPECIAL(str[1], str[2]);
if (c == K_ZERO) 
c = NUL;
str += 2;
}
if (IS_SPECIAL(c) || modifiers) 
{
if (cpo_special)
{
ga_clear(&ga);
return NULL;
}
ga_concat(&ga, get_special_key_name(c, modifiers));
continue; 
}
}
if (c == ' ' || c == '\t' || c == Ctrl_J || c == Ctrl_V
|| (c == '<' && !cpo_special) || (c == '\\' && !cpo_bslash))
ga_append(&ga, cpo_bslash ? Ctrl_V : '\\');
if (c)
ga_append(&ga, c);
}
ga_append(&ga, NUL);
return (char_u *)(ga.ga_data);
}
char_u *
set_context_in_map_cmd(
expand_T *xp,
char_u *cmd,
char_u *arg,
int forceit, 
int isabbrev, 
int isunmap, 
cmdidx_T cmdidx)
{
if (forceit && cmdidx != CMD_map && cmdidx != CMD_unmap)
xp->xp_context = EXPAND_NOTHING;
else
{
if (isunmap)
expand_mapmodes = get_map_mode(&cmd, forceit || isabbrev);
else
{
expand_mapmodes = INSERT + CMDLINE;
if (!isabbrev)
expand_mapmodes += VISUAL + SELECTMODE + NORMAL + OP_PENDING;
}
expand_isabbrev = isabbrev;
xp->xp_context = EXPAND_MAPPINGS;
expand_buffer = FALSE;
for (;;)
{
if (STRNCMP(arg, "<buffer>", 8) == 0)
{
expand_buffer = TRUE;
arg = skipwhite(arg + 8);
continue;
}
if (STRNCMP(arg, "<unique>", 8) == 0)
{
arg = skipwhite(arg + 8);
continue;
}
if (STRNCMP(arg, "<nowait>", 8) == 0)
{
arg = skipwhite(arg + 8);
continue;
}
if (STRNCMP(arg, "<silent>", 8) == 0)
{
arg = skipwhite(arg + 8);
continue;
}
if (STRNCMP(arg, "<special>", 9) == 0)
{
arg = skipwhite(arg + 9);
continue;
}
#if defined(FEAT_EVAL)
if (STRNCMP(arg, "<script>", 8) == 0)
{
arg = skipwhite(arg + 8);
continue;
}
if (STRNCMP(arg, "<expr>", 6) == 0)
{
arg = skipwhite(arg + 6);
continue;
}
#endif
break;
}
xp->xp_pattern = arg;
}
return NULL;
}
int
ExpandMappings(
regmatch_T *regmatch,
int *num_file,
char_u ***file)
{
mapblock_T *mp;
int hash;
int count;
int round;
char_u *p;
int i;
validate_maphash();
*num_file = 0; 
*file = NULL;
for (round = 1; round <= 2; ++round)
{
count = 0;
for (i = 0; i < 7; ++i)
{
if (i == 0)
p = (char_u *)"<silent>";
else if (i == 1)
p = (char_u *)"<unique>";
#if defined(FEAT_EVAL)
else if (i == 2)
p = (char_u *)"<script>";
else if (i == 3)
p = (char_u *)"<expr>";
#endif
else if (i == 4 && !expand_buffer)
p = (char_u *)"<buffer>";
else if (i == 5)
p = (char_u *)"<nowait>";
else if (i == 6)
p = (char_u *)"<special>";
else
continue;
if (vim_regexec(regmatch, p, (colnr_T)0))
{
if (round == 1)
++count;
else
(*file)[count++] = vim_strsave(p);
}
}
for (hash = 0; hash < 256; ++hash)
{
if (expand_isabbrev)
{
if (hash > 0) 
break; 
mp = first_abbr;
}
else if (expand_buffer)
mp = curbuf->b_maphash[hash];
else
mp = maphash[hash];
for (; mp; mp = mp->m_next)
{
if (mp->m_mode & expand_mapmodes)
{
p = translate_mapping(mp->m_keys);
if (p != NULL && vim_regexec(regmatch, p, (colnr_T)0))
{
if (round == 1)
++count;
else
{
(*file)[count++] = p;
p = NULL;
}
}
vim_free(p);
}
} 
} 
if (count == 0) 
break; 
if (round == 1)
{
*file = ALLOC_MULT(char_u *, count);
if (*file == NULL)
return FAIL;
}
} 
if (count > 1)
{
char_u **ptr1;
char_u **ptr2;
char_u **ptr3;
sort_strings(*file, count);
ptr1 = *file;
ptr2 = ptr1 + 1;
ptr3 = ptr1 + count;
while (ptr2 < ptr3)
{
if (STRCMP(*ptr1, *ptr2))
*++ptr1 = *ptr2++;
else
{
vim_free(*ptr2++);
count--;
}
}
}
*num_file = count;
return (count == 0 ? FAIL : OK);
}
int
check_abbr(
int c,
char_u *ptr,
int col,
int mincol)
{
int len;
int scol; 
int j;
char_u *s;
char_u tb[MB_MAXBYTES + 4];
mapblock_T *mp;
mapblock_T *mp2;
int clen = 0; 
int is_id = TRUE;
int vim_abbr;
if (typebuf.tb_no_abbr_cnt) 
return FALSE;
if (noremap_keys() && c != Ctrl_RSB)
return FALSE;
if (col == 0) 
return FALSE;
if (has_mbyte)
{
char_u *p;
p = mb_prevptr(ptr, ptr + col);
if (!vim_iswordp(p))
vim_abbr = TRUE; 
else
{
vim_abbr = FALSE; 
if (p > ptr)
is_id = vim_iswordp(mb_prevptr(ptr, p));
}
clen = 1;
while (p > ptr + mincol)
{
p = mb_prevptr(ptr, p);
if (vim_isspace(*p) || (!vim_abbr && is_id != vim_iswordp(p)))
{
p += (*mb_ptr2len)(p);
break;
}
++clen;
}
scol = (int)(p - ptr);
}
else
{
if (!vim_iswordc(ptr[col - 1]))
vim_abbr = TRUE; 
else
{
vim_abbr = FALSE; 
if (col > 1)
is_id = vim_iswordc(ptr[col - 2]);
}
for (scol = col - 1; scol > 0 && !vim_isspace(ptr[scol - 1])
&& (vim_abbr || is_id == vim_iswordc(ptr[scol - 1])); --scol)
;
}
if (scol < mincol)
scol = mincol;
if (scol < col) 
{
ptr += scol;
len = col - scol;
mp = curbuf->b_first_abbr;
mp2 = first_abbr;
if (mp == NULL)
{
mp = mp2;
mp2 = NULL;
}
for ( ; mp; mp->m_next == NULL
? (mp = mp2, mp2 = NULL) : (mp = mp->m_next))
{
int qlen = mp->m_keylen;
char_u *q = mp->m_keys;
int match;
if (vim_strbyte(mp->m_keys, K_SPECIAL) != NULL)
{
char_u *qe = vim_strsave(mp->m_keys);
if (qe != NULL)
{
q = qe;
vim_unescape_csi(q);
qlen = (int)STRLEN(q);
}
}
match = (mp->m_mode & State)
&& qlen == len
&& !STRNCMP(q, ptr, (size_t)len);
if (q != mp->m_keys)
vim_free(q);
if (match)
break;
}
if (mp != NULL)
{
j = 0;
if (c != Ctrl_RSB)
{
if (IS_SPECIAL(c) || c == K_SPECIAL)
{
tb[j++] = K_SPECIAL;
tb[j++] = K_SECOND(c);
tb[j++] = K_THIRD(c);
}
else
{
if (c < ABBR_OFF && (c < ' ' || c > '~'))
tb[j++] = Ctrl_V; 
if (has_mbyte)
{
if (c >= ABBR_OFF)
c -= ABBR_OFF;
j += (*mb_char2bytes)(c, tb + j);
}
else
tb[j++] = c;
}
tb[j] = NUL;
(void)ins_typebuf(tb, 1, 0, TRUE, mp->m_silent);
}
#if defined(FEAT_EVAL)
if (mp->m_expr)
s = eval_map_expr(mp->m_str, c);
else
#endif
s = mp->m_str;
if (s != NULL)
{
(void)ins_typebuf(s, mp->m_noremap, 0, TRUE, mp->m_silent);
typebuf.tb_no_abbr_cnt += (int)STRLEN(s) + j + 1;
#if defined(FEAT_EVAL)
if (mp->m_expr)
vim_free(s);
#endif
}
tb[0] = Ctrl_H;
tb[1] = NUL;
if (has_mbyte)
len = clen; 
while (len-- > 0) 
(void)ins_typebuf(tb, 1, 0, TRUE, mp->m_silent);
return TRUE;
}
}
return FALSE;
}
#if defined(FEAT_EVAL)
char_u *
eval_map_expr(
char_u *str,
int c) 
{
char_u *res;
char_u *p;
char_u *expr;
pos_T save_cursor;
int save_msg_col;
int save_msg_row;
expr = vim_strsave(str);
if (expr == NULL)
return NULL;
vim_unescape_csi(expr);
++textlock;
++ex_normal_lock;
set_vim_var_char(c); 
save_cursor = curwin->w_cursor;
save_msg_col = msg_col;
save_msg_row = msg_row;
p = eval_to_string(expr, NULL, FALSE);
--textlock;
--ex_normal_lock;
curwin->w_cursor = save_cursor;
msg_col = save_msg_col;
msg_row = save_msg_row;
vim_free(expr);
if (p == NULL)
return NULL;
res = vim_strsave_escape_csi(p);
vim_free(p);
return res;
}
#endif
char_u *
vim_strsave_escape_csi(
char_u *p)
{
char_u *res;
char_u *s, *d;
res = alloc(STRLEN(p) * 4 + 1);
if (res != NULL)
{
d = res;
for (s = p; *s != NUL; )
{
if (s[0] == K_SPECIAL && s[1] != NUL && s[2] != NUL)
{
*d++ = *s++;
*d++ = *s++;
*d++ = *s++;
}
else
{
d = add_char2buf(PTR2CHAR(s), d);
s += MB_CPTR2LEN(s);
}
}
*d = NUL;
}
return res;
}
void
vim_unescape_csi(char_u *p)
{
char_u *s = p, *d = p;
while (*s != NUL)
{
if (s[0] == K_SPECIAL && s[1] == KS_SPECIAL && s[2] == KE_FILLER)
{
*d++ = K_SPECIAL;
s += 3;
}
else if ((s[0] == K_SPECIAL || s[0] == CSI)
&& s[1] == KS_EXTRA && s[2] == (int)KE_CSI)
{
*d++ = CSI;
s += 3;
}
else
*d++ = *s++;
}
*d = NUL;
}
int
makemap(
FILE *fd,
buf_T *buf) 
{
mapblock_T *mp;
char_u c1, c2, c3;
char_u *p;
char *cmd;
int abbr;
int hash;
int did_cpo = FALSE;
int i;
validate_maphash();
for (abbr = 0; abbr < 2; ++abbr)
for (hash = 0; hash < 256; ++hash)
{
if (abbr)
{
if (hash > 0) 
break;
if (buf != NULL)
mp = buf->b_first_abbr;
else
mp = first_abbr;
}
else
{
if (buf != NULL)
mp = buf->b_maphash[hash];
else
mp = maphash[hash];
}
for ( ; mp; mp = mp->m_next)
{
if (mp->m_noremap == REMAP_SCRIPT)
continue;
for (p = mp->m_str; *p != NUL; ++p)
if (p[0] == K_SPECIAL && p[1] == KS_EXTRA
&& p[2] == (int)KE_SNR)
break;
if (*p != NUL)
continue;
c1 = NUL;
c2 = NUL;
c3 = NUL;
if (abbr)
cmd = "abbr";
else
cmd = "map";
switch (mp->m_mode)
{
case NORMAL + VISUAL + SELECTMODE + OP_PENDING:
break;
case NORMAL:
c1 = 'n';
break;
case VISUAL:
c1 = 'x';
break;
case SELECTMODE:
c1 = 's';
break;
case OP_PENDING:
c1 = 'o';
break;
case NORMAL + VISUAL:
c1 = 'n';
c2 = 'x';
break;
case NORMAL + SELECTMODE:
c1 = 'n';
c2 = 's';
break;
case NORMAL + OP_PENDING:
c1 = 'n';
c2 = 'o';
break;
case VISUAL + SELECTMODE:
c1 = 'v';
break;
case VISUAL + OP_PENDING:
c1 = 'x';
c2 = 'o';
break;
case SELECTMODE + OP_PENDING:
c1 = 's';
c2 = 'o';
break;
case NORMAL + VISUAL + SELECTMODE:
c1 = 'n';
c2 = 'v';
break;
case NORMAL + VISUAL + OP_PENDING:
c1 = 'n';
c2 = 'x';
c3 = 'o';
break;
case NORMAL + SELECTMODE + OP_PENDING:
c1 = 'n';
c2 = 's';
c3 = 'o';
break;
case VISUAL + SELECTMODE + OP_PENDING:
c1 = 'v';
c2 = 'o';
break;
case CMDLINE + INSERT:
if (!abbr)
cmd = "map!";
break;
case CMDLINE:
c1 = 'c';
break;
case INSERT:
c1 = 'i';
break;
case LANGMAP:
c1 = 'l';
break;
case TERMINAL:
c1 = 't';
break;
default:
iemsg(_("E228: makemap: Illegal mode"));
return FAIL;
}
do 
{
if (!did_cpo)
{
if (*mp->m_str == NUL) 
did_cpo = TRUE;
else
for (i = 0; i < 2; ++i)
for (p = (i ? mp->m_str : mp->m_keys); *p; ++p)
if (*p == K_SPECIAL || *p == NL)
did_cpo = TRUE;
if (did_cpo)
{
if (fprintf(fd, "let s:cpo_save=&cpo") < 0
|| put_eol(fd) < 0
|| fprintf(fd, "set cpo&vim") < 0
|| put_eol(fd) < 0)
return FAIL;
}
}
if (c1 && putc(c1, fd) < 0)
return FAIL;
if (mp->m_noremap != REMAP_YES && fprintf(fd, "nore") < 0)
return FAIL;
if (fputs(cmd, fd) < 0)
return FAIL;
if (buf != NULL && fputs(" <buffer>", fd) < 0)
return FAIL;
if (mp->m_nowait && fputs(" <nowait>", fd) < 0)
return FAIL;
if (mp->m_silent && fputs(" <silent>", fd) < 0)
return FAIL;
#if defined(FEAT_EVAL)
if (mp->m_noremap == REMAP_SCRIPT
&& fputs("<script>", fd) < 0)
return FAIL;
if (mp->m_expr && fputs(" <expr>", fd) < 0)
return FAIL;
#endif
if ( putc(' ', fd) < 0
|| put_escstr(fd, mp->m_keys, 0) == FAIL
|| putc(' ', fd) < 0
|| put_escstr(fd, mp->m_str, 1) == FAIL
|| put_eol(fd) < 0)
return FAIL;
c1 = c2;
c2 = c3;
c3 = NUL;
} while (c1 != NUL);
}
}
if (did_cpo)
if (fprintf(fd, "let &cpo=s:cpo_save") < 0
|| put_eol(fd) < 0
|| fprintf(fd, "unlet s:cpo_save") < 0
|| put_eol(fd) < 0)
return FAIL;
return OK;
}
int
put_escstr(FILE *fd, char_u *strstart, int what)
{
char_u *str = strstart;
int c;
int modifiers;
if (*str == NUL && what == 1)
{
if (fprintf(fd, "<Nop>") < 0)
return FAIL;
return OK;
}
for ( ; *str != NUL; ++str)
{
char_u *p;
p = mb_unescape(&str);
if (p != NULL)
{
while (*p != NUL)
if (fputc(*p++, fd) < 0)
return FAIL;
--str;
continue;
}
c = *str;
if (c == K_SPECIAL && what != 2)
{
modifiers = 0x0;
if (str[1] == KS_MODIFIER)
{
modifiers = str[2];
str += 3;
c = *str;
}
if (c == K_SPECIAL)
{
c = TO_SPECIAL(str[1], str[2]);
str += 2;
}
if (IS_SPECIAL(c) || modifiers) 
{
if (fputs((char *)get_special_key_name(c, modifiers), fd) < 0)
return FAIL;
continue;
}
}
if (c == NL)
{
if (what == 2)
{
if (fprintf(fd, IF_EB("\\\026\n", "\\" CTRL_V_STR "\n")) < 0)
return FAIL;
}
else
{
if (fprintf(fd, "<NL>") < 0)
return FAIL;
}
continue;
}
if (what == 2 && (VIM_ISWHITE(c) || c == '"' || c == '\\'))
{
if (putc('\\', fd) < 0)
return FAIL;
}
else if (c < ' ' || c > '~' || c == '|'
|| (what == 0 && c == ' ')
|| (what == 1 && str == strstart && c == ' ')
|| (what != 2 && c == '<'))
{
if (putc(Ctrl_V, fd) < 0)
return FAIL;
}
if (putc(c, fd) < 0)
return FAIL;
}
return OK;
}
void
check_map_keycodes(void)
{
mapblock_T *mp;
char_u *p;
int i;
char_u buf[3];
int abbr;
int hash;
buf_T *bp;
ESTACK_CHECK_DECLARATION
validate_maphash();
estack_push(ETYPE_INTERNAL, (char_u *)"mappings", 0);
ESTACK_CHECK_SETUP
for (bp = firstbuf; ; bp = bp->b_next)
{
for (abbr = 0; abbr <= 1; ++abbr)
for (hash = 0; hash < 256; ++hash)
{
if (abbr)
{
if (hash) 
break;
if (bp != NULL)
mp = bp->b_first_abbr;
else
mp = first_abbr;
}
else
{
if (bp != NULL)
mp = bp->b_maphash[hash];
else
mp = maphash[hash];
}
for ( ; mp != NULL; mp = mp->m_next)
{
for (i = 0; i <= 1; ++i) 
{
if (i == 0)
p = mp->m_keys; 
else
p = mp->m_str; 
while (*p)
{
if (*p == K_SPECIAL)
{
++p;
if (*p < 128) 
{
buf[0] = p[0];
buf[1] = p[1];
buf[2] = NUL;
(void)add_termcap_entry(buf, FALSE);
}
++p;
}
++p;
}
}
}
}
if (bp == NULL)
break;
}
ESTACK_CHECK_NOW
estack_pop();
}
#if defined(FEAT_EVAL) || defined(PROTO)
char_u *
check_map(
char_u *keys,
int mode,
int exact, 
int ign_mod, 
int abbr, 
mapblock_T **mp_ptr, 
int *local_ptr) 
{
int hash;
int len, minlen;
mapblock_T *mp;
char_u *s;
int local;
validate_maphash();
len = (int)STRLEN(keys);
for (local = 1; local >= 0; --local)
for (hash = 0; hash < 256; ++hash)
{
if (abbr)
{
if (hash > 0) 
break;
if (local)
mp = curbuf->b_first_abbr;
else
mp = first_abbr;
}
else if (local)
mp = curbuf->b_maphash[hash];
else
mp = maphash[hash];
for ( ; mp != NULL; mp = mp->m_next)
{
if ((mp->m_mode & mode) && (!exact || mp->m_keylen == len))
{
if (len > mp->m_keylen)
minlen = mp->m_keylen;
else
minlen = len;
s = mp->m_keys;
if (ign_mod && s[0] == K_SPECIAL && s[1] == KS_MODIFIER
&& s[2] != NUL)
{
s += 3;
if (len > mp->m_keylen - 3)
minlen = mp->m_keylen - 3;
}
if (STRNCMP(s, keys, minlen) == 0)
{
if (mp_ptr != NULL)
*mp_ptr = mp;
if (local_ptr != NULL)
*local_ptr = local;
return mp->m_str;
}
}
}
}
return NULL;
}
void
get_maparg(typval_T *argvars, typval_T *rettv, int exact)
{
char_u *keys;
char_u *which;
char_u buf[NUMBUFLEN];
char_u *keys_buf = NULL;
char_u *rhs;
int mode;
int abbr = FALSE;
int get_dict = FALSE;
mapblock_T *mp;
int buffer_local;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
keys = tv_get_string(&argvars[0]);
if (*keys == NUL)
return;
if (argvars[1].v_type != VAR_UNKNOWN)
{
which = tv_get_string_buf_chk(&argvars[1], buf);
if (argvars[2].v_type != VAR_UNKNOWN)
{
abbr = (int)tv_get_number(&argvars[2]);
if (argvars[3].v_type != VAR_UNKNOWN)
get_dict = (int)tv_get_number(&argvars[3]);
}
}
else
which = (char_u *)"";
if (which == NULL)
return;
mode = get_map_mode(&which, 0);
keys = replace_termcodes(keys, &keys_buf,
REPTERM_FROM_PART | REPTERM_DO_LT, NULL);
rhs = check_map(keys, mode, exact, FALSE, abbr, &mp, &buffer_local);
vim_free(keys_buf);
if (!get_dict)
{
if (rhs != NULL)
{
if (*rhs == NUL)
rettv->vval.v_string = vim_strsave((char_u *)"<Nop>");
else
rettv->vval.v_string = str2special_save(rhs, FALSE);
}
}
else if (rettv_dict_alloc(rettv) != FAIL && rhs != NULL)
{
char_u *lhs = str2special_save(mp->m_keys, TRUE);
char_u *mapmode = map_mode_to_chars(mp->m_mode);
dict_T *dict = rettv->vval.v_dict;
dict_add_string(dict, "lhs", lhs);
dict_add_string(dict, "rhs", mp->m_orig_str);
dict_add_number(dict, "noremap", mp->m_noremap ? 1L : 0L);
dict_add_number(dict, "script", mp->m_noremap == REMAP_SCRIPT
? 1L : 0L);
dict_add_number(dict, "expr", mp->m_expr ? 1L : 0L);
dict_add_number(dict, "silent", mp->m_silent ? 1L : 0L);
dict_add_number(dict, "sid", (long)mp->m_script_ctx.sc_sid);
dict_add_number(dict, "lnum", (long)mp->m_script_ctx.sc_lnum);
dict_add_number(dict, "buffer", (long)buffer_local);
dict_add_number(dict, "nowait", mp->m_nowait ? 1L : 0L);
dict_add_string(dict, "mode", mapmode);
vim_free(lhs);
vim_free(mapmode);
}
}
#endif
#if defined(MSWIN) || defined(MACOS_X)
#define VIS_SEL (VISUAL+SELECTMODE) 
struct initmap
{
char_u *arg;
int mode;
};
#if defined(FEAT_GUI_MSWIN)
static struct initmap initmappings[] =
{
{(char_u *)"<S-Insert> \"*P", NORMAL},
{(char_u *)"<S-Insert> \"-d\"*P", VIS_SEL},
{(char_u *)"<S-Insert> <C-R><C-O>*", INSERT+CMDLINE},
{(char_u *)"<C-Insert> \"*y", VIS_SEL},
{(char_u *)"<S-Del> \"*d", VIS_SEL},
{(char_u *)"<C-Del> \"*d", VIS_SEL},
{(char_u *)"<C-X> \"*d", VIS_SEL},
};
#endif
#if defined(MSWIN) && (!defined(FEAT_GUI) || defined(VIMDLL))
static struct initmap cinitmappings[] =
{
{(char_u *)"\316w <C-Home>", NORMAL+VIS_SEL},
{(char_u *)"\316w <C-Home>", INSERT+CMDLINE},
{(char_u *)"\316u <C-End>", NORMAL+VIS_SEL},
{(char_u *)"\316u <C-End>", INSERT+CMDLINE},
#if defined(FEAT_CLIPBOARD)
{(char_u *)"\316\324 \"*P", NORMAL}, 
{(char_u *)"\316\324 \"-d\"*P", VIS_SEL}, 
{(char_u *)"\316\324 \022\017*", INSERT}, 
{(char_u *)"\316\325 \"*y", VIS_SEL}, 
{(char_u *)"\316\327 \"*d", VIS_SEL}, 
{(char_u *)"\316\330 \"*d", VIS_SEL}, 
{(char_u *)"\030 \"*d", VIS_SEL}, 
#else
{(char_u *)"\316\324 P", NORMAL}, 
{(char_u *)"\316\324 \"-dP", VIS_SEL}, 
{(char_u *)"\316\324 \022\017\"", INSERT}, 
{(char_u *)"\316\325 y", VIS_SEL}, 
{(char_u *)"\316\327 d", VIS_SEL}, 
{(char_u *)"\316\330 d", VIS_SEL}, 
#endif
};
#endif
#if defined(MACOS_X)
static struct initmap initmappings[] =
{
{(char_u *)"<D-v> \"*P", NORMAL},
{(char_u *)"<D-v> \"-d\"*P", VIS_SEL},
{(char_u *)"<D-v> <C-R>*", INSERT+CMDLINE},
{(char_u *)"<D-c> \"*y", VIS_SEL},
{(char_u *)"<D-x> \"*d", VIS_SEL},
{(char_u *)"<Backspace> \"-d", VIS_SEL},
};
#endif
#undef VIS_SEL
#endif
void
init_mappings(void)
{
#if defined(MSWIN) || defined(MACOS_X)
int i;
#if defined(MSWIN) && (!defined(FEAT_GUI_MSWIN) || defined(VIMDLL))
#if defined(VIMDLL)
if (!gui.starting)
#endif
{
for (i = 0;
i < (int)(sizeof(cinitmappings) / sizeof(struct initmap)); ++i)
add_map(cinitmappings[i].arg, cinitmappings[i].mode);
}
#endif
#if defined(FEAT_GUI_MSWIN) || defined(MACOS_X)
for (i = 0; i < (int)(sizeof(initmappings) / sizeof(struct initmap)); ++i)
add_map(initmappings[i].arg, initmappings[i].mode);
#endif
#endif
}
#if defined(MSWIN) || defined(FEAT_CMDWIN) || defined(MACOS_X) || defined(PROTO)
void
add_map(char_u *map, int mode)
{
char_u *s;
char_u *cpo_save = p_cpo;
p_cpo = (char_u *)""; 
s = vim_strsave(map);
if (s != NULL)
{
(void)do_map(0, s, mode, FALSE);
vim_free(s);
}
p_cpo = cpo_save;
}
#endif
#if defined(FEAT_LANGMAP) || defined(PROTO)
typedef struct
{
int from;
int to;
} langmap_entry_T;
static garray_T langmap_mapga;
static void
langmap_set_entry(int from, int to)
{
langmap_entry_T *entries = (langmap_entry_T *)(langmap_mapga.ga_data);
int a = 0;
int b = langmap_mapga.ga_len;
while (a != b)
{
int i = (a + b) / 2;
int d = entries[i].from - from;
if (d == 0)
{
entries[i].to = to;
return;
}
if (d < 0)
a = i + 1;
else
b = i;
}
if (ga_grow(&langmap_mapga, 1) != OK)
return; 
entries = (langmap_entry_T *)(langmap_mapga.ga_data) + a;
mch_memmove(entries + 1, entries,
(langmap_mapga.ga_len - a) * sizeof(langmap_entry_T));
++langmap_mapga.ga_len;
entries[0].from = from;
entries[0].to = to;
}
int
langmap_adjust_mb(int c)
{
langmap_entry_T *entries = (langmap_entry_T *)(langmap_mapga.ga_data);
int a = 0;
int b = langmap_mapga.ga_len;
while (a != b)
{
int i = (a + b) / 2;
int d = entries[i].from - c;
if (d == 0)
return entries[i].to; 
if (d < 0)
a = i + 1;
else
b = i;
}
return c; 
}
void
langmap_init(void)
{
int i;
for (i = 0; i < 256; i++)
langmap_mapchar[i] = i; 
ga_init2(&langmap_mapga, sizeof(langmap_entry_T), 8);
}
void
langmap_set(void)
{
char_u *p;
char_u *p2;
int from, to;
ga_clear(&langmap_mapga); 
langmap_init(); 
for (p = p_langmap; p[0] != NUL; )
{
for (p2 = p; p2[0] != NUL && p2[0] != ',' && p2[0] != ';';
MB_PTR_ADV(p2))
{
if (p2[0] == '\\' && p2[1] != NUL)
++p2;
}
if (p2[0] == ';')
++p2; 
else
p2 = NULL; 
while (p[0])
{
if (p[0] == ',')
{
++p;
break;
}
if (p[0] == '\\' && p[1] != NUL)
++p;
from = (*mb_ptr2char)(p);
to = NUL;
if (p2 == NULL)
{
MB_PTR_ADV(p);
if (p[0] != ',')
{
if (p[0] == '\\')
++p;
to = (*mb_ptr2char)(p);
}
}
else
{
if (p2[0] != ',')
{
if (p2[0] == '\\')
++p2;
to = (*mb_ptr2char)(p2);
}
}
if (to == NUL)
{
semsg(_("E357: 'langmap': Matching character missing for %s"),
transchar(from));
return;
}
if (from >= 256)
langmap_set_entry(from, to);
else
langmap_mapchar[from & 255] = to;
MB_PTR_ADV(p);
if (p2 != NULL)
{
MB_PTR_ADV(p2);
if (*p == ';')
{
p = p2;
if (p[0] != NUL)
{
if (p[0] != ',')
{
semsg(_("E358: 'langmap': Extra characters after semicolon: %s"), p);
return;
}
++p;
}
break;
}
}
}
}
}
#endif
static void
do_exmap(exarg_T *eap, int isabbrev)
{
int mode;
char_u *cmdp;
cmdp = eap->cmd;
mode = get_map_mode(&cmdp, eap->forceit || isabbrev);
switch (do_map((*cmdp == 'n') ? 2 : (*cmdp == 'u'),
eap->arg, mode, isabbrev))
{
case 1: emsg(_(e_invarg));
break;
case 2: emsg((isabbrev ? _(e_noabbr) : _(e_nomap)));
break;
}
}
void
ex_abbreviate(exarg_T *eap)
{
do_exmap(eap, TRUE); 
}
void
ex_map(exarg_T *eap)
{
if (secure)
{
secure = 2;
msg_outtrans(eap->cmd);
msg_putchar('\n');
}
do_exmap(eap, FALSE);
}
void
ex_unmap(exarg_T *eap)
{
do_exmap(eap, FALSE);
}
void
ex_mapclear(exarg_T *eap)
{
map_clear(eap->cmd, eap->arg, eap->forceit, FALSE);
}
void
ex_abclear(exarg_T *eap)
{
map_clear(eap->cmd, eap->arg, TRUE, TRUE);
}
