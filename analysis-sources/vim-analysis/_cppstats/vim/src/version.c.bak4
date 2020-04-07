








#include "vim.h"

#if defined(AMIGA)
#include <time.h> 
#endif














#include "version.h"

char *Version = VIM_VERSION_SHORT;
static char *mediumVersion = VIM_VERSION_MEDIUM;

#if defined(HAVE_DATE_TIME) || defined(PROTO)
#if (defined(VMS) && defined(VAXC)) || defined(PROTO)
char longVersion[sizeof(VIM_VERSION_LONG_DATE) + sizeof(__DATE__)
+ sizeof(__TIME__) + 3];

void
init_longVersion(void)
{




strcpy(longVersion, VIM_VERSION_LONG_DATE);
strcat(longVersion, __DATE__);
strcat(longVersion, " ");
strcat(longVersion, __TIME__);
strcat(longVersion, ")");
}

#else
void
init_longVersion(void)
{
char *date_time = __DATE__ " " __TIME__;
char *msg = _("%s (%s, compiled %s)");
size_t len = strlen(msg)
+ strlen(VIM_VERSION_LONG_ONLY)
+ strlen(VIM_VERSION_DATE_ONLY)
+ strlen(date_time);

longVersion = alloc(len);
if (longVersion == NULL)
longVersion = VIM_VERSION_LONG;
else
vim_snprintf(longVersion, len, msg,
VIM_VERSION_LONG_ONLY, VIM_VERSION_DATE_ONLY, date_time);
}
#endif
#else
char *longVersion = VIM_VERSION_LONG;

void
init_longVersion(void)
{

}
#endif

static char *(features[]) =
{
#if defined(HAVE_ACL)
"+acl",
#else
"-acl",
#endif
#if defined(AMIGA)
#if defined(FEAT_ARP)
"+ARP",
#else
"-ARP",
#endif
#endif
#if defined(FEAT_ARABIC)
"+arabic",
#else
"-arabic",
#endif
"+autocmd",
#if defined(FEAT_AUTOCHDIR)
"+autochdir",
#else
"-autochdir",
#endif
#if defined(FEAT_AUTOSERVERNAME)
"+autoservername",
#else
"-autoservername",
#endif
#if defined(FEAT_BEVAL_GUI)
"+balloon_eval",
#else
"-balloon_eval",
#endif
#if defined(FEAT_BEVAL_TERM)
"+balloon_eval_term",
#else
"-balloon_eval_term",
#endif
#if defined(FEAT_BROWSE)
"+browse",
#else
"-browse",
#endif
#if defined(NO_BUILTIN_TCAPS)
"-builtin_terms",
#endif
#if defined(SOME_BUILTIN_TCAPS)
"+builtin_terms",
#endif
#if defined(ALL_BUILTIN_TCAPS)
"++builtin_terms",
#endif
#if defined(FEAT_BYTEOFF)
"+byte_offset",
#else
"-byte_offset",
#endif
#if defined(FEAT_JOB_CHANNEL)
"+channel",
#else
"-channel",
#endif
#if defined(FEAT_CINDENT)
"+cindent",
#else
"-cindent",
#endif
#if defined(FEAT_CLIENTSERVER)
"+clientserver",
#else
"-clientserver",
#endif
#if defined(FEAT_CLIPBOARD)
"+clipboard",
#else
"-clipboard",
#endif
"+cmdline_compl",
"+cmdline_hist",
#if defined(FEAT_CMDL_INFO)
"+cmdline_info",
#else
"-cmdline_info",
#endif
"+comments",
#if defined(FEAT_CONCEAL)
"+conceal",
#else
"-conceal",
#endif
#if defined(FEAT_CRYPT)
"+cryptv",
#else
"-cryptv",
#endif
#if defined(FEAT_CSCOPE)
"+cscope",
#else
"-cscope",
#endif
"+cursorbind",
#if defined(CURSOR_SHAPE)
"+cursorshape",
#else
"-cursorshape",
#endif
#if defined(FEAT_CON_DIALOG) && defined(FEAT_GUI_DIALOG)
"+dialog_con_gui",
#else
#if defined(FEAT_CON_DIALOG)
"+dialog_con",
#else
#if defined(FEAT_GUI_DIALOG)
"+dialog_gui",
#else
"-dialog",
#endif
#endif
#endif
#if defined(FEAT_DIFF)
"+diff",
#else
"-diff",
#endif
#if defined(FEAT_DIGRAPHS)
"+digraphs",
#else
"-digraphs",
#endif
#if defined(FEAT_GUI_MSWIN)
#if defined(FEAT_DIRECTX)
"+directx",
#else
"-directx",
#endif
#endif
#if defined(FEAT_DND)
"+dnd",
#else
"-dnd",
#endif
#if defined(EBCDIC)
"+ebcdic",
#else
"-ebcdic",
#endif
#if defined(FEAT_EMACS_TAGS)
"+emacs_tags",
#else
"-emacs_tags",
#endif
#if defined(FEAT_EVAL)
"+eval",
#else
"-eval",
#endif
"+ex_extra",
#if defined(FEAT_SEARCH_EXTRA)
"+extra_search",
#else
"-extra_search",
#endif
"-farsi",
#if defined(FEAT_SEARCHPATH)
"+file_in_path",
#else
"-file_in_path",
#endif
#if defined(FEAT_FIND_ID)
"+find_in_path",
#else
"-find_in_path",
#endif
#if defined(FEAT_FLOAT)
"+float",
#else
"-float",
#endif
#if defined(FEAT_FOLDING)
"+folding",
#else
"-folding",
#endif
#if defined(FEAT_FOOTER)
"+footer",
#else
"-footer",
#endif

#if !defined(USE_SYSTEM) && defined(UNIX)
"+fork()",
#endif
#if defined(FEAT_GETTEXT)
#if defined(DYNAMIC_GETTEXT)
"+gettext/dyn",
#else
"+gettext",
#endif
#else
"-gettext",
#endif
"-hangul_input",
#if (defined(HAVE_ICONV_H) && defined(USE_ICONV)) || defined(DYNAMIC_ICONV)
#if defined(DYNAMIC_ICONV)
"+iconv/dyn",
#else
"+iconv",
#endif
#else
"-iconv",
#endif
"+insert_expand",
#if defined(FEAT_JOB_CHANNEL)
"+job",
#else
"-job",
#endif
#if defined(FEAT_JUMPLIST)
"+jumplist",
#else
"-jumplist",
#endif
#if defined(FEAT_KEYMAP)
"+keymap",
#else
"-keymap",
#endif
#if defined(FEAT_EVAL)
"+lambda",
#else
"-lambda",
#endif
#if defined(FEAT_LANGMAP)
"+langmap",
#else
"-langmap",
#endif
#if defined(FEAT_LIBCALL)
"+libcall",
#else
"-libcall",
#endif
#if defined(FEAT_LINEBREAK)
"+linebreak",
#else
"-linebreak",
#endif
#if defined(FEAT_LISP)
"+lispindent",
#else
"-lispindent",
#endif
"+listcmds",
"+localmap",
#if defined(FEAT_LUA)
#if defined(DYNAMIC_LUA)
"+lua/dyn",
#else
"+lua",
#endif
#else
"-lua",
#endif
#if defined(FEAT_MENU)
"+menu",
#else
"-menu",
#endif
#if defined(FEAT_SESSION)
"+mksession",
#else
"-mksession",
#endif
"+modify_fname",
"+mouse",
#if defined(FEAT_MOUSESHAPE)
"+mouseshape",
#else
"-mouseshape",
#endif

#if defined(UNIX) || defined(VMS)
#if defined(FEAT_MOUSE_DEC)
"+mouse_dec",
#else
"-mouse_dec",
#endif
#if defined(FEAT_MOUSE_GPM)
"+mouse_gpm",
#else
"-mouse_gpm",
#endif
#if defined(FEAT_MOUSE_JSB)
"+mouse_jsbterm",
#else
"-mouse_jsbterm",
#endif
#if defined(FEAT_MOUSE_NET)
"+mouse_netterm",
#else
"-mouse_netterm",
#endif
#endif

#if defined(__QNX__)
#if defined(FEAT_MOUSE_PTERM)
"+mouse_pterm",
#else
"-mouse_pterm",
#endif
#endif

#if defined(UNIX) || defined(VMS)
"+mouse_sgr",
#if defined(FEAT_SYSMOUSE)
"+mouse_sysmouse",
#else
"-mouse_sysmouse",
#endif
#if defined(FEAT_MOUSE_URXVT)
"+mouse_urxvt",
#else
"-mouse_urxvt",
#endif
"+mouse_xterm",
#endif

#if defined(FEAT_MBYTE_IME)
#if defined(DYNAMIC_IME)
"+multi_byte_ime/dyn",
#else
"+multi_byte_ime",
#endif
#else
"+multi_byte",
#endif
#if defined(FEAT_MULTI_LANG)
"+multi_lang",
#else
"-multi_lang",
#endif
#if defined(FEAT_MZSCHEME)
#if defined(DYNAMIC_MZSCHEME)
"+mzscheme/dyn",
#else
"+mzscheme",
#endif
#else
"-mzscheme",
#endif
#if defined(FEAT_NETBEANS_INTG)
"+netbeans_intg",
#else
"-netbeans_intg",
#endif
"+num64",
#if defined(FEAT_GUI_MSWIN)
#if defined(FEAT_OLE)
"+ole",
#else
"-ole",
#endif
#endif
#if defined(FEAT_EVAL)
"+packages",
#else
"-packages",
#endif
#if defined(FEAT_PATH_EXTRA)
"+path_extra",
#else
"-path_extra",
#endif
#if defined(FEAT_PERL)
#if defined(DYNAMIC_PERL)
"+perl/dyn",
#else
"+perl",
#endif
#else
"-perl",
#endif
#if defined(FEAT_PERSISTENT_UNDO)
"+persistent_undo",
#else
"-persistent_undo",
#endif
#if defined(FEAT_PROP_POPUP)
"+popupwin",
#else
"-popupwin",
#endif
#if defined(FEAT_PRINTER)
#if defined(FEAT_POSTSCRIPT)
"+postscript",
#else
"-postscript",
#endif
"+printer",
#else
"-printer",
#endif
#if defined(FEAT_PROFILE)
"+profile",
#else
"-profile",
#endif
#if defined(FEAT_PYTHON)
#if defined(DYNAMIC_PYTHON)
"+python/dyn",
#else
"+python",
#endif
#else
"-python",
#endif
#if defined(FEAT_PYTHON3)
#if defined(DYNAMIC_PYTHON3)
"+python3/dyn",
#else
"+python3",
#endif
#else
"-python3",
#endif
#if defined(FEAT_QUICKFIX)
"+quickfix",
#else
"-quickfix",
#endif
#if defined(FEAT_RELTIME)
"+reltime",
#else
"-reltime",
#endif
#if defined(FEAT_RIGHTLEFT)
"+rightleft",
#else
"-rightleft",
#endif
#if defined(FEAT_RUBY)
#if defined(DYNAMIC_RUBY)
"+ruby/dyn",
#else
"+ruby",
#endif
#else
"-ruby",
#endif
"+scrollbind",
#if defined(FEAT_SIGNS)
"+signs",
#else
"-signs",
#endif
#if defined(FEAT_SMARTINDENT)
"+smartindent",
#else
"-smartindent",
#endif
#if defined(FEAT_SOUND)
"+sound",
#else
"-sound",
#endif
#if defined(FEAT_SPELL)
"+spell",
#else
"-spell",
#endif
#if defined(STARTUPTIME)
"+startuptime",
#else
"-startuptime",
#endif
#if defined(FEAT_STL_OPT)
"+statusline",
#else
"-statusline",
#endif
"-sun_workshop",
#if defined(FEAT_SYN_HL)
"+syntax",
#else
"-syntax",
#endif

#if defined(USE_SYSTEM) && defined(UNIX)
"+system()",
#endif
#if defined(FEAT_TAG_BINS)
"+tag_binary",
#else
"-tag_binary",
#endif
"-tag_old_static",
"-tag_any_white",
#if defined(FEAT_TCL)
#if defined(DYNAMIC_TCL)
"+tcl/dyn",
#else
"+tcl",
#endif
#else
"-tcl",
#endif
#if defined(FEAT_TERMGUICOLORS)
"+termguicolors",
#else
"-termguicolors",
#endif
#if defined(FEAT_TERMINAL)
"+terminal",
#else
"-terminal",
#endif
#if defined(UNIX)

#if defined(TERMINFO)
"+terminfo",
#else
"-terminfo",
#endif
#endif
#if defined(FEAT_TERMRESPONSE)
"+termresponse",
#else
"-termresponse",
#endif
#if defined(FEAT_TEXTOBJ)
"+textobjects",
#else
"-textobjects",
#endif
#if defined(FEAT_PROP_POPUP)
"+textprop",
#else
"-textprop",
#endif
#if !defined(UNIX)

#if defined(HAVE_TGETENT)
"+tgetent",
#else
"-tgetent",
#endif
#endif
#if defined(FEAT_TIMERS)
"+timers",
#else
"-timers",
#endif
#if defined(FEAT_TITLE)
"+title",
#else
"-title",
#endif
#if defined(FEAT_TOOLBAR)
"+toolbar",
#else
"-toolbar",
#endif
"+user_commands",
#if defined(FEAT_VARTABS)
"+vartabs",
#else
"-vartabs",
#endif
"+vertsplit",
"+virtualedit",
"+visual",
"+visualextra",
#if defined(FEAT_VIMINFO)
"+viminfo",
#else
"-viminfo",
#endif
"+vreplace",
#if defined(MSWIN)
#if defined(FEAT_VTP)
"+vtp",
#else
"-vtp",
#endif
#endif
#if defined(FEAT_WILDIGN)
"+wildignore",
#else
"-wildignore",
#endif
#if defined(FEAT_WILDMENU)
"+wildmenu",
#else
"-wildmenu",
#endif
"+windows",
#if defined(FEAT_WRITEBACKUP)
"+writebackup",
#else
"-writebackup",
#endif
#if defined(UNIX) || defined(VMS)
#if defined(FEAT_X11)
"+X11",
#else
"-X11",
#endif
#endif
#if defined(FEAT_XFONTSET)
"+xfontset",
#else
"-xfontset",
#endif
#if defined(FEAT_XIM)
"+xim",
#else
"-xim",
#endif
#if defined(MSWIN)
#if defined(FEAT_XPM_W32)
"+xpm_w32",
#else
"-xpm_w32",
#endif
#else
#if defined(HAVE_XPM)
"+xpm",
#else
"-xpm",
#endif
#endif
#if defined(UNIX) || defined(VMS)
#if defined(USE_XSMP_INTERACT)
"+xsmp_interact",
#else
#if defined(USE_XSMP)
"+xsmp",
#else
"-xsmp",
#endif
#endif
#if defined(FEAT_XCLIPBOARD)
"+xterm_clipboard",
#else
"-xterm_clipboard",
#endif
#endif
#if defined(FEAT_XTERM_SAVE)
"+xterm_save",
#else
"-xterm_save",
#endif
NULL
};

static int included_patches[] =
{ 

505,

504,

503,

502,

501,

500,

499,

498,

497,

496,

495,

494,

493,

492,

491,

490,

489,

488,

487,

486,

485,

484,

483,

482,

481,

480,

479,

478,

477,

476,

475,

474,

473,

472,

471,

470,

469,

468,

467,

466,

465,

464,

463,

462,

461,

460,

459,

458,

457,

456,

455,

454,

453,

452,

451,

450,

449,

448,

447,

446,

445,

444,

443,

442,

441,

440,

439,

438,

437,

436,

435,

434,

433,

432,

431,

430,

429,

428,

427,

426,

425,

424,

423,

422,

421,

420,

419,

418,

417,

416,

415,

414,

413,

412,

411,

410,

409,

408,

407,

406,

405,

404,

403,

402,

401,

400,

399,

398,

397,

396,

395,

394,

393,

392,

391,

390,

389,

388,

387,

386,

385,

384,

383,

382,

381,

380,

379,

378,

377,

376,

375,

374,

373,

372,

371,

370,

369,

368,

367,

366,

365,

364,

363,

362,

361,

360,

359,

358,

357,

356,

355,

354,

353,

352,

351,

350,

349,

348,

347,

346,

345,

344,

343,

342,

341,

340,

339,

338,

337,

336,

335,

334,

333,

332,

331,

330,

329,

328,

327,

326,

325,

324,

323,

322,

321,

320,

319,

318,

317,

316,

315,

314,

313,

312,

311,

310,

309,

308,

307,

306,

305,

304,

303,

302,

301,

300,

299,

298,

297,

296,

295,

294,

293,

292,

291,

290,

289,

288,

287,

286,

285,

284,

283,

282,

281,

280,

279,

278,

277,

276,

275,

274,

273,

272,

271,

270,

269,

268,

267,

266,

265,

264,

263,

262,

261,

260,

259,

258,

257,

256,

255,

254,

253,

252,

251,

250,

249,

248,

247,

246,

245,

244,

243,

242,

241,

240,

239,

238,

237,

236,

235,

234,

233,

232,

231,

230,

229,

228,

227,

226,

225,

224,

223,

222,

221,

220,

219,

218,

217,

216,

215,

214,

213,

212,

211,

210,

209,

208,

207,

206,

205,

204,

203,

202,

201,

200,

199,

198,

197,

196,

195,

194,

193,

192,

191,

190,

189,

188,

187,

186,

185,

184,

183,

182,

181,

180,

179,

178,

177,

176,

175,

174,

173,

172,

171,

170,

169,

168,

167,

166,

165,

164,

163,

162,

161,

160,

159,

158,

157,

156,

155,

154,

153,

152,

151,

150,

149,

148,

147,

146,

145,

144,

143,

142,

141,

140,

139,

138,

137,

136,

135,

134,

133,

132,

131,

130,

129,

128,

127,

126,

125,

124,

123,

122,

121,

120,

119,

118,

117,

116,

115,

114,

113,

112,

111,

110,

109,

108,

107,

106,

105,

104,

103,

102,

101,

100,

99,

98,

97,

96,

95,

94,

93,

92,

91,

90,

89,

88,

87,

86,

85,

84,

83,

82,

81,

80,

79,

78,

77,

76,

75,

74,

73,

72,

71,

70,

69,

68,

67,

66,

65,

64,

63,

62,

61,

60,

59,

58,

57,

56,

55,

54,

53,

52,

51,

50,

49,

48,

47,

46,

45,

44,

43,

42,

41,

40,

39,

38,

37,

36,

35,

34,

33,

32,

31,

30,

29,

28,

27,

26,

25,

24,

23,

22,

21,

20,

19,

18,

17,

16,

15,

14,

13,

12,

11,

10,

9,

8,

7,

6,

5,

4,

3,

2,

1,

0
};








static char *(extra_patches[]) =
{ 

NULL
};

int
highest_patch(void)
{

return included_patches[0];
}

#if defined(FEAT_EVAL) || defined(PROTO)



int
has_patch(int n)
{
int i;

for (i = 0; included_patches[i] != 0; ++i)
if (included_patches[i] == n)
return TRUE;
return FALSE;
}
#endif

void
ex_version(exarg_T *eap)
{



if (*eap->arg == NUL)
{
msg_putchar('\n');
list_version();
}
}






static void
version_msg_wrap(char_u *s, int wrap)
{
int len = (int)vim_strsize(s) + (wrap ? 2 : 0);

if (!got_int && len < (int)Columns && msg_col + len >= (int)Columns
&& *s != '\n')
msg_putchar('\n');
if (!got_int)
{
if (wrap)
msg_puts("[");
msg_puts((char *)s);
if (wrap)
msg_puts("]");
}
}

static void
version_msg(char *s)
{
version_msg_wrap((char_u *)s, FALSE);
}




static void
list_features(void)
{
list_in_columns((char_u **)features, -1, -1);
}






void
list_in_columns(char_u **items, int size, int current)
{
int i;
int ncol;
int nrow;
int cur_row = 1;
int item_count = 0;
int width = 0;
#if defined(FEAT_SYN_HL)
int use_highlight = (items == (char_u **)features);
#endif



for (i = 0; size < 0 ? items[i] != NULL : i < size; ++i)
{
int l = (int)vim_strsize(items[i]) + (i == current ? 2 : 0);

if (l > width)
width = l;
++item_count;
}
width += 1;

if (Columns < width)
{

for (i = 0; i < item_count; ++i)
{
version_msg_wrap(items[i], i == current);
if (msg_col > 0 && i < item_count - 1)
msg_putchar('\n');
}
return;
}



ncol = (int) (Columns + 1) / width;
nrow = item_count / ncol + (item_count % ncol ? 1 : 0);


for (i = 0; !got_int && i < nrow * ncol; ++i)
{
int idx = (i / ncol) + (i % ncol) * nrow;

if (idx < item_count)
{
int last_col = (i + 1) % ncol == 0;

if (idx == current)
msg_putchar('[');
#if defined(FEAT_SYN_HL)
if (use_highlight && items[idx][0] == '-')
msg_puts_attr((char *)items[idx], HL_ATTR(HLF_W));
else
#endif
msg_puts((char *)items[idx]);
if (idx == current)
msg_putchar(']');
if (last_col)
{
if (msg_col > 0 && cur_row < nrow)
msg_putchar('\n');
++cur_row;
}
else
{
while (msg_col % width)
msg_putchar(' ');
}
}
else
{

if (msg_col > 0)
{
if (cur_row < nrow)
msg_putchar('\n');
++cur_row;
}
}
}
}

void
list_version(void)
{
int i;
int first;
char *s = "";





init_longVersion();
msg(longVersion);
#if defined(MSWIN)
#if defined(FEAT_GUI_MSWIN)
#if defined(VIMDLL)
#if defined(_WIN64)
msg_puts(_("\nMS-Windows 64-bit GUI/console version"));
#else
msg_puts(_("\nMS-Windows 32-bit GUI/console version"));
#endif
#else
#if defined(_WIN64)
msg_puts(_("\nMS-Windows 64-bit GUI version"));
#else
msg_puts(_("\nMS-Windows 32-bit GUI version"));
#endif
#endif
#if defined(FEAT_OLE)
msg_puts(_(" with OLE support"));
#endif
#else
#if defined(_WIN64)
msg_puts(_("\nMS-Windows 64-bit console version"));
#else
msg_puts(_("\nMS-Windows 32-bit console version"));
#endif
#endif
#endif
#if defined(MACOS_X)
#if defined(MACOS_X_DARWIN)
msg_puts(_("\nmacOS version"));
#else
msg_puts(_("\nmacOS version w/o darwin feat."));
#endif
#endif

#if defined(VMS)
msg_puts(_("\nOpenVMS version"));
#if defined(HAVE_PATHDEF)
if (*compiled_arch != NUL)
{
msg_puts(" - ");
msg_puts((char *)compiled_arch);
}
#endif

#endif



if (included_patches[0] != 0)
{
msg_puts(_("\nIncluded patches: "));
first = -1;

for (i = 0; included_patches[i] != 0; ++i)
;
while (--i >= 0)
{
if (first < 0)
first = included_patches[i];
if (i == 0 || included_patches[i - 1] != included_patches[i] + 1)
{
msg_puts(s);
s = ", ";
msg_outnum((long)first);
if (first != included_patches[i])
{
msg_puts("-");
msg_outnum((long)included_patches[i]);
}
first = -1;
}
}
}


if (extra_patches[0] != NULL)
{
msg_puts(_("\nExtra patches: "));
s = "";
for (i = 0; extra_patches[i] != NULL; ++i)
{
msg_puts(s);
s = ", ";
msg_puts(extra_patches[i]);
}
}

#if defined(MODIFIED_BY)
msg_puts("\n");
msg_puts(_("Modified by "));
msg_puts(MODIFIED_BY);
#endif

#if defined(HAVE_PATHDEF)
if (*compiled_user != NUL || *compiled_sys != NUL)
{
msg_puts(_("\nCompiled "));
if (*compiled_user != NUL)
{
msg_puts(_("by "));
msg_puts((char *)compiled_user);
}
if (*compiled_sys != NUL)
{
msg_puts("@");
msg_puts((char *)compiled_sys);
}
}
#endif

#if defined(FEAT_HUGE)
msg_puts(_("\nHuge version "));
#else
#if defined(FEAT_BIG)
msg_puts(_("\nBig version "));
#else
#if defined(FEAT_NORMAL)
msg_puts(_("\nNormal version "));
#else
#if defined(FEAT_SMALL)
msg_puts(_("\nSmall version "));
#else
msg_puts(_("\nTiny version "));
#endif
#endif
#endif
#endif
#if !defined(FEAT_GUI)
msg_puts(_("without GUI."));
#else
#if defined(FEAT_GUI_GTK)
#if defined(USE_GTK3)
msg_puts(_("with GTK3 GUI."));
#else
#if defined(FEAT_GUI_GNOME)
msg_puts(_("with GTK2-GNOME GUI."));
#else
msg_puts(_("with GTK2 GUI."));
#endif
#endif
#else
#if defined(FEAT_GUI_MOTIF)
msg_puts(_("with X11-Motif GUI."));
#else
#if defined(FEAT_GUI_ATHENA)
#if defined(FEAT_GUI_NEXTAW)
msg_puts(_("with X11-neXtaw GUI."));
#else
msg_puts(_("with X11-Athena GUI."));
#endif
#else
#if defined(FEAT_GUI_HAIKU)
msg_puts(_("with Haiku GUI."));
#else
#if defined(FEAT_GUI_PHOTON)
msg_puts(_("with Photon GUI."));
#else
#if defined(MSWIN)
msg_puts(_("with GUI."));
#else
#if defined(TARGET_API_MAC_CARBON) && TARGET_API_MAC_CARBON
msg_puts(_("with Carbon GUI."));
#else
#if defined(TARGET_API_MAC_OSX) && TARGET_API_MAC_OSX
msg_puts(_("with Cocoa GUI."));
#else
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
version_msg(_(" Features included (+) or not (-):\n"));

list_features();
if (msg_col > 0)
msg_putchar('\n');

#if defined(SYS_VIMRC_FILE)
version_msg(_(" system vimrc file: \""));
version_msg(SYS_VIMRC_FILE);
version_msg("\"\n");
#endif
#if defined(USR_VIMRC_FILE)
version_msg(_(" user vimrc file: \""));
version_msg(USR_VIMRC_FILE);
version_msg("\"\n");
#endif
#if defined(USR_VIMRC_FILE2)
version_msg(_(" 2nd user vimrc file: \""));
version_msg(USR_VIMRC_FILE2);
version_msg("\"\n");
#endif
#if defined(USR_VIMRC_FILE3)
version_msg(_(" 3rd user vimrc file: \""));
version_msg(USR_VIMRC_FILE3);
version_msg("\"\n");
#endif
#if defined(USR_EXRC_FILE)
version_msg(_(" user exrc file: \""));
version_msg(USR_EXRC_FILE);
version_msg("\"\n");
#endif
#if defined(USR_EXRC_FILE2)
version_msg(_(" 2nd user exrc file: \""));
version_msg(USR_EXRC_FILE2);
version_msg("\"\n");
#endif
#if defined(FEAT_GUI)
#if defined(SYS_GVIMRC_FILE)
version_msg(_(" system gvimrc file: \""));
version_msg(SYS_GVIMRC_FILE);
version_msg("\"\n");
#endif
version_msg(_(" user gvimrc file: \""));
version_msg(USR_GVIMRC_FILE);
version_msg("\"\n");
#if defined(USR_GVIMRC_FILE2)
version_msg(_("2nd user gvimrc file: \""));
version_msg(USR_GVIMRC_FILE2);
version_msg("\"\n");
#endif
#if defined(USR_GVIMRC_FILE3)
version_msg(_("3rd user gvimrc file: \""));
version_msg(USR_GVIMRC_FILE3);
version_msg("\"\n");
#endif
#endif
version_msg(_(" defaults file: \""));
version_msg(VIM_DEFAULTS_FILE);
version_msg("\"\n");
#if defined(FEAT_GUI)
#if defined(SYS_MENU_FILE)
version_msg(_(" system menu file: \""));
version_msg(SYS_MENU_FILE);
version_msg("\"\n");
#endif
#endif
#if defined(HAVE_PATHDEF)
if (*default_vim_dir != NUL)
{
version_msg(_(" fall-back for $VIM: \""));
version_msg((char *)default_vim_dir);
version_msg("\"\n");
}
if (*default_vimruntime_dir != NUL)
{
version_msg(_(" f-b for $VIMRUNTIME: \""));
version_msg((char *)default_vimruntime_dir);
version_msg("\"\n");
}
version_msg(_("Compilation: "));
version_msg((char *)all_cflags);
version_msg("\n");
#if defined(VMS)
if (*compiler_version != NUL)
{
version_msg(_("Compiler: "));
version_msg((char *)compiler_version);
version_msg("\n");
}
#endif
version_msg(_("Linking: "));
version_msg((char *)all_lflags);
#endif
#if defined(DEBUG)
version_msg("\n");
version_msg(_(" DEBUG BUILD"));
#endif
}

static void do_intro_line(int row, char_u *mesg, int add_version, int attr);




void
maybe_intro_message(void)
{
if (BUFEMPTY()
&& curbuf->b_fname == NULL
&& firstwin->w_next == NULL
&& vim_strchr(p_shm, SHM_INTRO) == NULL)
intro_message(FALSE);
}






void
intro_message(
int colon) 
{
int i;
int row;
int blanklines;
int sponsor;
char *p;
static char *(lines[]) =
{
N_("VIM - Vi IMproved"),
"",
N_("version "),
N_("by Bram Moolenaar et al."),
#if defined(MODIFIED_BY)
" ",
#endif
N_("Vim is open source and freely distributable"),
"",
N_("Help poor children in Uganda!"),
N_("type :help iccf<Enter> for information "),
"",
N_("type :q<Enter> to exit "),
N_("type :help<Enter> or <F1> for on-line help"),
N_("type :help version8<Enter> for version info"),
NULL,
"",
N_("Running in Vi compatible mode"),
N_("type :set nocp<Enter> for Vim defaults"),
N_("type :help cp-default<Enter> for info on this"),
};
#if defined(FEAT_GUI)
static char *(gui_lines[]) =
{
NULL,
NULL,
NULL,
NULL,
#if defined(MODIFIED_BY)
NULL,
#endif
NULL,
NULL,
NULL,
N_("menu Help->Orphans for information "),
NULL,
N_("Running modeless, typed text is inserted"),
N_("menu Edit->Global Settings->Toggle Insert Mode "),
N_(" for two modes "),
NULL,
NULL,
NULL,
N_("menu Edit->Global Settings->Toggle Vi Compatible"),
N_(" for Vim defaults "),
};
#endif


blanklines = (int)Rows - ((sizeof(lines) / sizeof(char *)) - 1);
if (!p_cp)
blanklines += 4; 


if (p_ls > 1)
blanklines -= Rows - topframe->fr_height;
if (blanklines < 0)
blanklines = 0;



sponsor = (int)time(NULL);
sponsor = ((sponsor & 2) == 0) - ((sponsor & 4) == 0);


row = blanklines / 2;
if ((row >= 2 && Columns >= 50) || colon)
{
for (i = 0; i < (int)(sizeof(lines) / sizeof(char *)); ++i)
{
p = lines[i];
#if defined(FEAT_GUI)
if (p_im && gui.in_use && gui_lines[i] != NULL)
p = gui_lines[i];
#endif
if (p == NULL)
{
if (!p_cp)
break;
continue;
}
if (sponsor != 0)
{
if (strstr(p, "children") != NULL)
p = sponsor < 0
? N_("Sponsor Vim development!")
: N_("Become a registered Vim user!");
else if (strstr(p, "iccf") != NULL)
p = sponsor < 0
? N_("type :help sponsor<Enter> for information ")
: N_("type :help register<Enter> for information ");
else if (strstr(p, "Orphans") != NULL)
p = N_("menu Help->Sponsor/Register for information ");
}
if (*p != NUL)
do_intro_line(row, (char_u *)_(p), i == 2, 0);
++row;
}
}


if (colon)
msg_row = row;
}

static void
do_intro_line(
int row,
char_u *mesg,
int add_version,
int attr)
{
char_u vers[20];
int col;
char_u *p;
int l;
int clen;
#if defined(MODIFIED_BY)
#define MODBY_LEN 150
char_u modby[MODBY_LEN];

if (*mesg == ' ')
{
vim_strncpy(modby, (char_u *)_("Modified by "), MODBY_LEN - 1);
l = (int)STRLEN(modby);
vim_strncpy(modby + l, (char_u *)MODIFIED_BY, MODBY_LEN - l - 1);
mesg = modby;
}
#endif


col = vim_strsize(mesg);
if (add_version)
{
STRCPY(vers, mediumVersion);
if (highest_patch())
{

if (isalpha((int)vers[3]))
{
int len = (isalpha((int)vers[4])) ? 5 : 4;
sprintf((char *)vers + len, ".%d%s", highest_patch(),
mediumVersion + len);
}
else
sprintf((char *)vers + 3, ".%d", highest_patch());
}
col += (int)STRLEN(vers);
}
col = (Columns - col) / 2;
if (col < 0)
col = 0;


for (p = mesg; *p != NUL; p += l)
{
clen = 0;
for (l = 0; p[l] != NUL
&& (l == 0 || (p[l] != '<' && p[l - 1] != '>')); ++l)
{
if (has_mbyte)
{
clen += ptr2cells(p + l);
l += (*mb_ptr2len)(p + l) - 1;
}
else
clen += byte2cells(p[l]);
}
screen_puts_len(p, l, row, col, *p == '<' ? HL_ATTR(HLF_8) : attr);
col += clen;
}


if (add_version)
screen_puts(vers, row, col, 0);
}




void
ex_intro(exarg_T *eap UNUSED)
{
screenclear();
intro_message(TRUE);
wait_return(TRUE);
}
