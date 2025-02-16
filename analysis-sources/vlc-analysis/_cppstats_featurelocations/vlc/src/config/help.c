



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <limits.h>
#include <float.h>

#include <vlc_common.h>
#include <vlc_modules.h>
#include <vlc_plugin.h>
#include <vlc_charset.h>
#include "modules/modules.h"
#include "config/configuration.h"
#include "libvlc.h"

#if defined( _WIN32 )
#include <vlc_charset.h>
#define wcwidth(cp) ((void)(cp), 1) 
#else
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif

#if defined( _WIN32 ) && !VLC_WINSTORE_APP
static void ShowConsole (void);
static void PauseConsole (void);
#else
#define ShowConsole() (void)0
#define PauseConsole() (void)0
#endif

static void Help (vlc_object_t *, const char *);
static void Usage (vlc_object_t *, const char *);
static void Version (void);
static void ListModules (vlc_object_t *, bool);




static unsigned ConsoleWidth(void)
{
#if defined(TIOCGWINSZ)
struct winsize ws;

if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
return ws.ws_col;
#endif
#if defined(WIOCGETD)
struct uwdata uw;

if (ioctl(STDOUT_FILENO, WIOCGETD, &uw) == 0)
return uw.uw_height / uw.uw_vs;
#endif
#if defined (_WIN32) && !VLC_WINSTORE_APP
CONSOLE_SCREEN_BUFFER_INFO buf;

if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buf))
return buf.dwSize.X;
#endif
return 80;
}







bool config_PrintHelp (vlc_object_t *obj)
{
char *str;


if (var_InheritBool (obj, "help"))
{
Help (obj, "help");
return true;
}


if (var_InheritBool (obj, "version"))
{
Version();
return true;
}


str = var_InheritString (obj, "module");
if (str != NULL)
{
Help (obj, str);
free (str);
return true;
}


if (var_InheritBool (obj, "full-help"))
{
var_Create (obj, "help-verbose", VLC_VAR_BOOL);
var_SetBool (obj, "help-verbose", true);
Help (obj, "full-help");
return true;
}


if (var_InheritBool (obj, "longhelp"))
{
Help (obj, "longhelp");
return true;
}


if (var_InheritBool (obj, "list"))
{
ListModules (obj, false );
return true;
}

if (var_InheritBool (obj, "list-verbose"))
{
ListModules (obj, true);
return true;
}

return false;
}






static inline void print_help_on_full_help( void )
{
putchar('\n');
puts(_("To get exhaustive help, use '-H'."));
}

static const char vlc_usage[] = N_(
"Usage: %s [options] [stream] ...\n"
"You can specify multiple streams on the commandline.\n"
"They will be enqueued in the playlist.\n"
"The first item specified will be played first.\n"
"\n"
"Options-styles:\n"
" --option A global option that is set for the duration of the program.\n"
" -option A single letter version of a global --option.\n"
" :option An option that only applies to the stream directly before it\n"
" and that overrides previous settings.\n"
"\n"
"Stream MRL syntax:\n"
" [[access][/demux]://]URL[#[title][:chapter][-[title][:chapter]]]\n"
" [:option=value ...]\n"
"\n"
" Many of the global --options can also be used as MRL specific :options.\n"
" Multiple :option=value pairs can be specified.\n"
"\n"
"URL syntax:\n"
" file:///path/file Plain media file\n"
" http://host[:port]/file HTTP URL\n"
" ftp://host[:port]/file FTP URL\n"
" mms://host[:port]/file MMS URL\n"
" screen:// Screen capture\n"
" dvd://[device] DVD device\n"
" vcd://[device] VCD device\n"
" cdda://[device] Audio CD device\n"
" udp://[[<source address>]@[<bind address>][:<bind port>]]\n"
" UDP stream sent by a streaming server\n"
" vlc://pause:<seconds> Pause the playlist for a certain time\n"
" vlc://quit Special item to quit VLC\n"
"\n");

static void Help (vlc_object_t *p_this, char const *psz_help_name)
{
ShowConsole();

if( psz_help_name && !strcmp( psz_help_name, "help" ) )
{
printf(_(vlc_usage), "vlc");
Usage( p_this, "=core" );
print_help_on_full_help();
}
else if( psz_help_name && !strcmp( psz_help_name, "longhelp" ) )
{
printf(_(vlc_usage), "vlc");
Usage( p_this, NULL );
print_help_on_full_help();
}
else if( psz_help_name && !strcmp( psz_help_name, "full-help" ) )
{
printf(_(vlc_usage), "vlc");
Usage( p_this, NULL );
}
else if( psz_help_name )
{
Usage( p_this, psz_help_name );
}

PauseConsole();
}






#define COL(x) "\033[" #x ";1m"
#define RED COL(31)
#define GREEN COL(32)
#define YELLOW COL(33)
#define BLUE COL(34)
#define MAGENTA COL(35)
#define CYAN COL(36)
#define WHITE COL(0)
#define GRAY "\033[0m"
#define LINE_START 8
#define PADDING_SPACES 25

static void print_section(const module_t *m, const module_config_t **sect,
bool color, bool desc)
{
const module_config_t *item = *sect;

if (item == NULL)
return;
*sect = NULL;

printf(color ? RED" %s:\n"GRAY : " %s:\n",
module_gettext(m, item->psz_text));
if (desc && item->psz_longtext != NULL)
printf(color ? MAGENTA" %s\n"GRAY : " %s\n",
module_gettext(m, item->psz_longtext));
}

static void print_desc(const char *str, unsigned margin, bool color)
{
unsigned width = ConsoleWidth() - margin;

if (color)
fputs(BLUE, stdout);

const char *word = str;
int wordlen = 0, wordwidth = 0;
unsigned offset = 0;
bool newline = true;

while (str[0])
{
uint32_t cp;
size_t charlen = vlc_towc(str, &cp);
if (unlikely(charlen == (size_t)-1))
break;

int charwidth = wcwidth(cp);
if (charwidth < 0)
charwidth = 0;

str += charlen;

if (iswspace(cp))
{
if (!newline)
{
putchar(' '); 
charwidth = 1;
}
fwrite(word, 1, wordlen, stdout); 
word = str;
wordlen = 0;
wordwidth = 0;
newline = false;
}
else
{
wordlen += charlen;
wordwidth += charwidth;
}

offset += charwidth;
if (offset >= width)
{
if (newline)
{ 
fwrite(word, 1, wordlen - charlen, stdout);
word = str - charlen;
wordlen = charlen;
wordwidth = charwidth;
}
printf("\n%*s", margin, ""); 
offset = wordwidth;
newline = true;
}
}

if (!newline)
putchar(' ');
printf(color ? "%s\n"GRAY : "%s\n", word);
}

static int vlc_swidth(const char *str)
{
for (int total = 0;;)
{
uint32_t cp;
size_t charlen = vlc_towc(str, &cp);

if (charlen == 0)
return total;
if (charlen == (size_t)-1)
return -1;
str += charlen;

int w = wcwidth(cp);
if (w == -1)
return -1;
total += w;
}
}

static void print_item(const module_t *m, const module_config_t *item,
const module_config_t **section, bool color, bool desc)
{
#if !defined(_WIN32)
#define OPTION_VALUE_SEP " "
#else
#define OPTION_VALUE_SEP "="
#endif
const char *bra = OPTION_VALUE_SEP "<", *type, *ket = ">";
const char *prefix = NULL, *suffix = NULL;
char *typebuf = NULL;

switch (CONFIG_CLASS(item->i_type))
{
case 0: 
switch (item->i_type)
{
case CONFIG_HINT_CATEGORY:
case CONFIG_HINT_USAGE:
printf(color ? GREEN "\n %s\n" GRAY : "\n %s\n",
module_gettext(m, item->psz_text));

if (desc && item->psz_longtext != NULL)
printf(color ? CYAN " %s\n" GRAY : " %s\n",
module_gettext(m, item->psz_longtext));
break;

case CONFIG_SECTION:
*section = item;
break;
}
return;

case CONFIG_ITEM_STRING:
{
type = _("string");

char **ppsz_values, **ppsz_texts;

ssize_t i_count = config_GetPszChoices(item->psz_name, &ppsz_values, &ppsz_texts);

if (i_count > 0)
{
size_t len = 0;

for (size_t i = 0; i < (size_t)i_count; i++)
len += strlen(ppsz_values[i]) + 1;

typebuf = malloc(len);
if (typebuf == NULL)
goto end_string;

bra = OPTION_VALUE_SEP "{";
type = typebuf;
ket = "}";

*typebuf = 0;
for (size_t i = 0; i < (size_t)i_count; i++)
{
if (i > 0)
strcat(typebuf, ",");
strcat(typebuf, ppsz_values[i]);
}

end_string:
for (size_t i = 0; i < (size_t)i_count; i++)
{
free(ppsz_values[i]);
free(ppsz_texts[i]);
}
free(ppsz_values);
free(ppsz_texts);
}

break;
}
case CONFIG_ITEM_INTEGER:
{
type = _("integer");

int64_t *pi_values;
char **ppsz_texts;

ssize_t i_count = config_GetIntChoices(item->psz_name, &pi_values, &ppsz_texts);

if (i_count > 0)
{
size_t len = 0;

for (size_t i = 0; i < (size_t)i_count; i++)
len += strlen(ppsz_texts[i])
+ 4 * sizeof (int64_t) + 5;

typebuf = malloc(len);
if (typebuf == NULL)
goto end_integer;

bra = OPTION_VALUE_SEP "{";
type = typebuf;
ket = "}";

*typebuf = 0;
for (size_t i = 0; i < (size_t)i_count; i++)
{
if (i != 0)
strcat(typebuf, ", ");
sprintf(typebuf + strlen(typebuf), "%"PRIi64" (%s)",
pi_values[i],
ppsz_texts[i]);
}

end_integer:
for (size_t i = 0; i < (size_t)i_count; i++)
free(ppsz_texts[i]);
free(pi_values);
free(ppsz_texts);
}
else if (item->min.i != INT64_MIN || item->max.i != INT64_MAX )
{
if (asprintf(&typebuf, "%s [%"PRId64" .. %"PRId64"]",
type, item->min.i, item->max.i) >= 0)
type = typebuf;
else
typebuf = NULL;
}
break;
}
case CONFIG_ITEM_FLOAT:
type = _("float");
if (item->min.f != FLT_MIN || item->max.f != FLT_MAX)
{
if (asprintf(&typebuf, "%s [%f .. %f]", type,
item->min.f, item->max.f) >= 0)
type = typebuf;
else
typebuf = NULL;
}
break;

case CONFIG_ITEM_BOOL:
bra = type = ket = "";
prefix = ", --no-";
suffix = item->value.i ? _("(default enabled)")
: _("(default disabled)");
break;
default:
return;
}

print_section(m, section, color, desc);


char shortopt[4];
if (item->i_short != '\0')
sprintf(shortopt, "-%c,", item->i_short);
else
strcpy(shortopt, " ");

if (CONFIG_CLASS(item->i_type) == CONFIG_ITEM_BOOL)
printf(color ? WHITE" %s --%s" "%s%s%s%s%s "GRAY
: " %s --%s%s%s%s%s%s ", shortopt, item->psz_name,
prefix, item->psz_name, bra, type, ket);
else
printf(color ? WHITE" %s --%s"YELLOW"%s%s%s%s%s "GRAY
: " %s --%s%s%s%s%s%s ", shortopt, item->psz_name,
"", "", bra, type, ket);


int offset = PADDING_SPACES - strlen(item->psz_name)
- strlen(bra) - vlc_swidth(type) - strlen(ket) - 1;
if (CONFIG_CLASS(item->i_type) == CONFIG_ITEM_BOOL)
offset -= strlen(item->psz_name) + vlc_swidth(prefix);
if (offset < 0)
{
putchar('\n');
offset = PADDING_SPACES + LINE_START;
}

printf("%*s", offset, "");
print_desc(module_gettext(m, item->psz_text),
PADDING_SPACES + LINE_START, color);

if (suffix != NULL)
{
printf("%*s", PADDING_SPACES + LINE_START, "");
print_desc(suffix, PADDING_SPACES + LINE_START, color);
}

if (desc && (item->psz_longtext != NULL && item->psz_longtext[0]))
{ 
printf("%*s", LINE_START + 2, "");
print_desc(module_gettext(m, item->psz_longtext),
LINE_START + 2, false);
}

free(typebuf);
}

static bool module_match(const module_t *m, const char *pattern, bool strict)
{
if (pattern == NULL)
return true;

const char *objname = module_get_object(m);

if (strict ? (strcmp(objname, pattern) == 0)
: (strstr(objname, pattern) != NULL))
return true;

for (unsigned i = 0; i < m->i_shortcuts; i++)
{
const char *shortcut = m->pp_shortcuts[i];

if (strict ? (strcmp(shortcut, pattern) == 0)
: (strstr(shortcut, pattern) != NULL))
return true;
}
return false;
}

static bool plugin_show(const vlc_plugin_t *plugin)
{
for (size_t i = 0; i < plugin->conf.size; i++)
{
const module_config_t *item = plugin->conf.items + i;

if (!CONFIG_ITEM(item->i_type))
continue;
if (item->b_removed)
continue;
return true;
}
return false;
}

static void Usage (vlc_object_t *p_this, char const *psz_search)
{
bool found = false;
bool strict = false;
if (psz_search != NULL && psz_search[0] == '=')
{
strict = true;
psz_search++;
}

bool color = false;
#if !defined(_WIN32)
if (isatty(STDOUT_FILENO))
color = var_InheritBool(p_this, "color");
#endif

const bool desc = var_InheritBool(p_this, "help-verbose");


for (const vlc_plugin_t *p = vlc_plugins; p != NULL; p = p->next)
{
const module_t *m = p->module;
const module_config_t *section = NULL;
const char *objname = module_get_object(m);

if (p->conf.count == 0)
continue; 
if (!module_match(m, psz_search, strict))
continue;
found = true;

if (!plugin_show(p))
continue;


printf(color ? "\n " GREEN "%s" GRAY " (%s)\n" : "\n %s (%s)\n",
module_gettext(m, m->psz_longname), objname);
if (m->psz_help != NULL)
printf(color ? CYAN" %s\n"GRAY : " %s\n",
module_gettext(m, m->psz_help));


for (size_t j = 0; j < p->conf.size; j++)
{
const module_config_t *item = p->conf.items + j;

if (item->b_removed)
continue; 

print_item(m, item, &section, color, desc);
}
}

if (!found)
printf(color ? "\n" WHITE "%s" GRAY "\n" : "\n%s\n",
_("No matching module found. Use --list or "
"--list-verbose to list available modules."));
}







static void ListModules (vlc_object_t *p_this, bool b_verbose)
{
bool color = false;

ShowConsole();
#if !defined(_WIN32)
if (isatty(STDOUT_FILENO))
color = var_InheritBool(p_this, "color");
#else
(void) p_this;
#endif


size_t count;
module_t **list = module_list_get (&count);


for (size_t j = 0; j < count; j++)
{
module_t *p_parser = list[j];
const char *objname = module_get_object (p_parser);
printf(color ? GREEN" %-22s "WHITE"%s\n"GRAY : " %-22s %s\n",
objname, module_gettext(p_parser, p_parser->psz_longname));

if( b_verbose )
{
const char *const *pp_shortcuts = p_parser->pp_shortcuts;
for( unsigned i = 0; i < p_parser->i_shortcuts; i++ )
if( strcmp( pp_shortcuts[i], objname ) )
printf(color ? CYAN" s %s\n"GRAY : " s %s\n",
pp_shortcuts[i]);
if (p_parser->psz_capability != NULL)
printf(color ? MAGENTA" c %s (%d)\n"GRAY : " c %s (%d)\n",
p_parser->psz_capability, p_parser->i_score);
}
}
module_list_free (list);
PauseConsole();
}






static void Version( void )
{
ShowConsole();
printf(_("VLC version %s (%s)\n"), VERSION_MESSAGE, psz_vlc_changeset);
printf(_("Compiled by %s on %s (%s)\n"), VLC_CompileBy(),
VLC_CompileHost(), __DATE__" "__TIME__ );
printf(_("Compiler: %s\n"), VLC_Compiler());
fputs(LICENSE_MSG, stdout);
PauseConsole();
}

#if defined( _WIN32 ) && !VLC_WINSTORE_APP





static void ShowConsole( void )
{
if( getenv( "PWD" ) ) return; 

if( !AllocConsole() ) return;




SetConsoleOutputCP (GetACP ());
SetConsoleTitle (TEXT("VLC media player version ") TEXT(PACKAGE_VERSION));

freopen( "CONOUT$", "w", stderr );
freopen( "CONIN$", "r", stdin );

if( freopen( "vlc-help.txt", "wt", stdout ) != NULL )
{
fputs( "\xEF\xBB\xBF", stdout );
fprintf( stderr, _("\nDumped content to vlc-help.txt file.\n") );
}
else
freopen( "CONOUT$", "w", stdout );
}






static void PauseConsole( void )
{
if( getenv( "PWD" ) ) return; 

utf8_fprintf( stderr, _("\nPress the RETURN key to continue...\n") );
getchar();
fclose( stdout );
}
#endif
