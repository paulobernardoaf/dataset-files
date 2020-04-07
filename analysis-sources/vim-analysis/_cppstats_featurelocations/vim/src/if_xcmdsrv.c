












#include "vim.h"
#include "version.h"

#if defined(FEAT_CLIENTSERVER) || defined(PROTO)

#if defined(FEAT_X11)
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#endif










































typedef struct PendingCommand
{
int serial; 
int code; 
char_u *result; 

struct PendingCommand *nextPtr;


} PendingCommand;

static PendingCommand *pendingCommands = NULL;



















































































#define MAX_PROP_WORDS 100000

struct ServerReply
{
Window id;
garray_T strings;
};
static garray_T serverReply = { 0, 0, 0, 0, 0 };
enum ServerReplyOp { SROP_Find, SROP_Add, SROP_Delete };

typedef int (*EndCond)(void *);

struct x_cmdqueue
{
char_u *propInfo;
long_u len;
struct x_cmdqueue *next;
struct x_cmdqueue *prev;
};

typedef struct x_cmdqueue x_queue_T;


static x_queue_T head = {NULL, 0, NULL, NULL};





static Window LookupName(Display *dpy, char_u *name, int delete, char_u **loose);
static int SendInit(Display *dpy);
static int DoRegisterName(Display *dpy, char_u *name);
static void DeleteAnyLingerer(Display *dpy, Window w);
static int GetRegProp(Display *dpy, char_u **regPropp, long_u *numItemsp, int domsg);
static int WaitForPend(void *p);
static int WindowValid(Display *dpy, Window w);
static void ServerWait(Display *dpy, Window w, EndCond endCond, void *endData, int localLoop, int seconds);
static int AppendPropCarefully(Display *display, Window window, Atom property, char_u *value, int length);
static int x_error_check(Display *dpy, XErrorEvent *error_event);
static int IsSerialName(char_u *name);
static void save_in_queue(char_u *buf, long_u len);
static void server_parse_message(Display *dpy, char_u *propInfo, long_u numItems);


static Atom registryProperty = None;
static Atom vimProperty = None;
static int got_x_error = FALSE;

static char_u *empty_prop = (char_u *)""; 





int
serverRegisterName(
Display *dpy, 
char_u *name) 
{
int i;
int res;
char_u *p = NULL;

res = DoRegisterName(dpy, name);
if (res < 0)
{
i = 1;
do
{
if (res < -1 || i >= 1000)
{
msg_attr(_("Unable to register a command server name"),
HL_ATTR(HLF_W));
return FAIL;
}
if (p == NULL)
p = alloc(STRLEN(name) + 10);
if (p == NULL)
{
res = -10;
continue;
}
sprintf((char *)p, "%s%d", name, i++);
res = DoRegisterName(dpy, p);
}
while (res < 0)
;
vim_free(p);
}
return OK;
}

static int
DoRegisterName(Display *dpy, char_u *name)
{
Window w;
XErrorHandler old_handler;
#define MAX_NAME_LENGTH 100
char_u propInfo[MAX_NAME_LENGTH + 20];

if (commProperty == None)
{
if (SendInit(dpy) < 0)
return -2;
}








XGrabServer(dpy);
w = LookupName(dpy, name, FALSE, NULL);
if (w != (Window)0)
{
Status status;
int dummyInt;
unsigned int dummyUns;
Window dummyWin;








old_handler = XSetErrorHandler(x_error_check);
status = XGetGeometry(dpy, w, &dummyWin, &dummyInt, &dummyInt,
&dummyUns, &dummyUns, &dummyUns, &dummyUns);
(void)XSetErrorHandler(old_handler);
if (status != Success && w != commWindow)
{
XUngrabServer(dpy);
XFlush(dpy);
return -1;
}
(void)LookupName(dpy, name, TRUE, NULL);
}
sprintf((char *)propInfo, "%x %.*s", (int_u)commWindow,
MAX_NAME_LENGTH, name);
old_handler = XSetErrorHandler(x_error_check);
got_x_error = FALSE;
XChangeProperty(dpy, RootWindow(dpy, 0), registryProperty, XA_STRING, 8,
PropModeAppend, propInfo, STRLEN(propInfo) + 1);
XUngrabServer(dpy);
XSync(dpy, False);
(void)XSetErrorHandler(old_handler);

if (!got_x_error)
{
#if defined(FEAT_EVAL)
set_vim_var_string(VV_SEND_SERVER, name, -1);
#endif
serverName = vim_strsave(name);
#if defined(FEAT_TITLE)
need_maketitle = TRUE;
#endif
return 0;
}
return -2;
}

#if defined(FEAT_GUI) || defined(PROTO)




void
serverChangeRegisteredWindow(
Display *dpy, 
Window newwin) 
{
char_u propInfo[MAX_NAME_LENGTH + 20];

commWindow = newwin;



if (SendInit(dpy) < 0)
return;



XGrabServer(dpy);
DeleteAnyLingerer(dpy, newwin);
if (serverName != NULL)
{

(void)LookupName(dpy, serverName, TRUE, NULL);
sprintf((char *)propInfo, "%x %.*s",
(int_u)newwin, MAX_NAME_LENGTH, serverName);
XChangeProperty(dpy, RootWindow(dpy, 0), registryProperty, XA_STRING, 8,
PropModeAppend, (char_u *)propInfo,
STRLEN(propInfo) + 1);
}
XUngrabServer(dpy);
}
#endif





int
serverSendToVim(
Display *dpy, 
char_u *name, 
char_u *cmd, 
char_u **result, 
Window *server, 
Bool asExpr, 
int timeout, 
Bool localLoop, 
int silent) 
{
Window w;
char_u *property;
int length;
int res;
static int serial = 0; 


PendingCommand pending;
char_u *loosename = NULL;

if (result != NULL)
*result = NULL;
if (name == NULL || *name == NUL)
name = (char_u *)"GVIM"; 

if (commProperty == None && dpy != NULL)
{
if (SendInit(dpy) < 0)
return -1;
}


if (dpy == NULL || (serverName != NULL && STRICMP(name, serverName) == 0))
return sendToLocalVim(cmd, asExpr, result);









while (TRUE)
{
w = LookupName(dpy, name, FALSE, &loosename);

if (w != None)
{
if (!WindowValid(dpy, w))
{
LookupName(dpy, loosename ? loosename : name,
TRUE, NULL);
vim_free(loosename);
continue;
}
}
break;
}
if (w == None)
{
if (!silent)
semsg(_(e_noserver), name);
return -1;
}
else if (loosename != NULL)
name = loosename;
if (server != NULL)
*server = w;






length = STRLEN(name) + STRLEN(p_enc) + STRLEN(cmd) + 14;
property = alloc(length + 30);

sprintf((char *)property, "%c%c%c-n %s%c-E %s%c-s %s",
0, asExpr ? 'c' : 'k', 0, name, 0, p_enc, 0, cmd);
if (name == loosename)
vim_free(loosename);

serial++;
sprintf((char *)property + length, "%c-r %x %d",
0, (int_u)commWindow, serial);

length += STRLEN(property + length + 1) + 1;

res = AppendPropCarefully(dpy, w, commProperty, property, length + 1);
vim_free(property);
if (res < 0)
{
emsg(_("E248: Failed to send command to the destination program"));
return -1;
}

if (!asExpr) 
return 0;






pending.serial = serial;
pending.code = 0;
pending.result = NULL;
pending.nextPtr = pendingCommands;
pendingCommands = &pending;

ServerWait(dpy, w, WaitForPend, &pending, localLoop,
timeout > 0 ? timeout : 600);





if (pendingCommands == &pending)
pendingCommands = pending.nextPtr;
else
{
PendingCommand *pcPtr;

for (pcPtr = pendingCommands; pcPtr != NULL; pcPtr = pcPtr->nextPtr)
if (pcPtr->nextPtr == &pending)
{
pcPtr->nextPtr = pending.nextPtr;
break;
}
}
if (result != NULL)
*result = pending.result;
else
vim_free(pending.result);

return pending.code == 0 ? 0 : -1;
}

static int
WaitForPend(void *p)
{
PendingCommand *pending = (PendingCommand *) p;
return pending->result != NULL;
}




static int
WindowValid(Display *dpy, Window w)
{
XErrorHandler old_handler;
Atom *plist;
int numProp;
int i;

old_handler = XSetErrorHandler(x_error_check);
got_x_error = 0;
plist = XListProperties(dpy, w, &numProp);
XSync(dpy, False);
XSetErrorHandler(old_handler);
if (plist == NULL || got_x_error)
return FALSE;

for (i = 0; i < numProp; i++)
if (plist[i] == vimProperty)
{
XFree(plist);
return TRUE;
}
XFree(plist);
return FALSE;
}




static void
ServerWait(
Display *dpy,
Window w,
EndCond endCond,
void *endData,
int localLoop,
int seconds)
{
time_t start;
time_t now;
XEvent event;

#define UI_MSEC_DELAY 53
#define SEND_MSEC_POLL 500
#if !defined(HAVE_SELECT)
struct pollfd fds;

fds.fd = ConnectionNumber(dpy);
fds.events = POLLIN;
#else
fd_set fds;
struct timeval tv;

tv.tv_sec = 0;
tv.tv_usec = SEND_MSEC_POLL * 1000;
FD_ZERO(&fds);
FD_SET(ConnectionNumber(dpy), &fds);
#endif

time(&start);
while (TRUE)
{
while (XCheckWindowEvent(dpy, commWindow, PropertyChangeMask, &event))
serverEventProc(dpy, &event, 1);
server_parse_messages();

if (endCond(endData) != 0)
break;
if (!WindowValid(dpy, w))
break;
time(&now);
if (seconds >= 0 && (now - start) >= seconds)
break;

#if defined(FEAT_TIMERS)
check_due_timer();
#endif


if (localLoop)
{
#if !defined(HAVE_SELECT)
if (poll(&fds, 1, SEND_MSEC_POLL) < 0)
break;
#else
if (select(FD_SETSIZE, &fds, NULL, NULL, &tv) < 0)
break;
#endif
}
else
{
if (got_int)
break;
ui_delay((long)UI_MSEC_DELAY, TRUE);
ui_breakcheck();
}
}
}








char_u *
serverGetVimNames(Display *dpy)
{
char_u *regProp;
char_u *entry;
char_u *p;
long_u numItems;
int_u w;
garray_T ga;

if (registryProperty == None)
{
if (SendInit(dpy) < 0)
return NULL;
}




if (GetRegProp(dpy, &regProp, &numItems, TRUE) == FAIL)
return NULL;




ga_init2(&ga, 1, 100);
for (p = regProp; (long_u)(p - regProp) < numItems; p++)
{
entry = p;
while (*p != 0 && !isspace(*p))
p++;
if (*p != 0)
{
w = None;
sscanf((char *)entry, "%x", &w);
if (WindowValid(dpy, (Window)w))
{
ga_concat(&ga, p + 1);
ga_concat(&ga, (char_u *)"\n");
}
while (*p != 0)
p++;
}
}
if (regProp != empty_prop)
XFree(regProp);
ga_append(&ga, NUL);
return ga.ga_data;
}




static struct ServerReply *
ServerReplyFind(Window w, enum ServerReplyOp op)
{
struct ServerReply *p;
struct ServerReply e;
int i;

p = (struct ServerReply *) serverReply.ga_data;
for (i = 0; i < serverReply.ga_len; i++, p++)
if (p->id == w)
break;
if (i >= serverReply.ga_len)
p = NULL;

if (p == NULL && op == SROP_Add)
{
if (serverReply.ga_growsize == 0)
ga_init2(&serverReply, sizeof(struct ServerReply), 1);
if (ga_grow(&serverReply, 1) == OK)
{
p = ((struct ServerReply *) serverReply.ga_data)
+ serverReply.ga_len;
e.id = w;
ga_init2(&e.strings, 1, 100);
mch_memmove(p, &e, sizeof(e));
serverReply.ga_len++;
}
}
else if (p != NULL && op == SROP_Delete)
{
ga_clear(&p->strings);
mch_memmove(p, p + 1, (serverReply.ga_len - i - 1) * sizeof(*p));
serverReply.ga_len--;
}

return p;
}





Window
serverStrToWin(char_u *str)
{
unsigned id = None;

sscanf((char *)str, "0x%x", &id);
if (id == None)
semsg(_("E573: Invalid server id used: %s"), str);

return (Window)id;
}





int
serverSendReply(char_u *name, char_u *str)
{
char_u *property;
int length;
int res;
Display *dpy = X_DISPLAY;
Window win = serverStrToWin(name);

if (commProperty == None)
{
if (SendInit(dpy) < 0)
return -2;
}
if (!WindowValid(dpy, win))
return -1;

length = STRLEN(p_enc) + STRLEN(str) + 14;
if ((property = alloc(length + 30)) != NULL)
{
sprintf((char *)property, "%cn%c-E %s%c-n %s%c-w %x",
0, 0, p_enc, 0, str, 0, (unsigned int)commWindow);

length += STRLEN(property + length);
res = AppendPropCarefully(dpy, win, commProperty, property, length + 1);
vim_free(property);
return res;
}
return -1;
}

static int
WaitForReply(void *p)
{
Window *w = (Window *) p;

return ServerReplyFind(*w, SROP_Find) != NULL;
}







int
serverReadReply(
Display *dpy,
Window win,
char_u **str,
int localLoop,
int timeout)
{
int len;
char_u *s;
struct ServerReply *p;

ServerWait(dpy, win, WaitForReply, &win, localLoop,
timeout > 0 ? timeout : -1);

if ((p = ServerReplyFind(win, SROP_Find)) != NULL && p->strings.ga_len > 0)
{
*str = vim_strsave(p->strings.ga_data);
len = STRLEN(*str) + 1;
if (len < p->strings.ga_len)
{
s = (char_u *) p->strings.ga_data;
mch_memmove(s, s + len, p->strings.ga_len - len);
p->strings.ga_len -= len;
}
else
{

ga_clear(&p->strings);
ServerReplyFind(win, SROP_Delete);
}
return 0;
}
return -1;
}





int
serverPeekReply(Display *dpy, Window win, char_u **str)
{
struct ServerReply *p;

if ((p = ServerReplyFind(win, SROP_Find)) != NULL && p->strings.ga_len > 0)
{
if (str != NULL)
*str = p->strings.ga_data;
return 1;
}
if (!WindowValid(dpy, win))
return -1;
return 0;
}






static int
SendInit(Display *dpy)
{
XErrorHandler old_handler;





old_handler = XSetErrorHandler(x_error_check);
got_x_error = FALSE;

if (commProperty == None)
commProperty = XInternAtom(dpy, "Comm", False);
if (vimProperty == None)
vimProperty = XInternAtom(dpy, "Vim", False);
if (registryProperty == None)
registryProperty = XInternAtom(dpy, "VimRegistry", False);

if (commWindow == None)
{
commWindow = XCreateSimpleWindow(dpy, XDefaultRootWindow(dpy),
getpid(), 0, 10, 10, 0,
WhitePixel(dpy, DefaultScreen(dpy)),
WhitePixel(dpy, DefaultScreen(dpy)));
XSelectInput(dpy, commWindow, PropertyChangeMask);


XGrabServer(dpy);
DeleteAnyLingerer(dpy, commWindow);
XUngrabServer(dpy);
}


XChangeProperty(dpy, commWindow, vimProperty, XA_STRING,
8, PropModeReplace, (char_u *)VIM_VERSION_SHORT,
(int)STRLEN(VIM_VERSION_SHORT) + 1);

XSync(dpy, False);
(void)XSetErrorHandler(old_handler);

return got_x_error ? -1 : 0;
}













static Window
LookupName(
Display *dpy, 
char_u *name, 
int delete, 
char_u **loose) 

{
char_u *regProp, *entry;
char_u *p;
long_u numItems;
int_u returnValue;




if (GetRegProp(dpy, &regProp, &numItems, FALSE) == FAIL)
return 0;




returnValue = (int_u)None;
entry = NULL; 
for (p = regProp; (long_u)(p - regProp) < numItems; )
{
entry = p;
while (*p != 0 && !isspace(*p))
p++;
if (*p != 0 && STRICMP(name, p + 1) == 0)
{
sscanf((char *)entry, "%x", &returnValue);
break;
}
while (*p != 0)
p++;
p++;
}

if (loose != NULL && returnValue == (int_u)None && !IsSerialName(name))
{
for (p = regProp; (long_u)(p - regProp) < numItems; )
{
entry = p;
while (*p != 0 && !isspace(*p))
p++;
if (*p != 0 && IsSerialName(p + 1)
&& STRNICMP(name, p + 1, STRLEN(name)) == 0)
{
sscanf((char *)entry, "%x", &returnValue);
*loose = vim_strsave(p + 1);
break;
}
while (*p != 0)
p++;
p++;
}
}






if (delete && returnValue != (int_u)None)
{
int count;

while (*p != 0)
p++;
p++;
count = numItems - (p - regProp);
if (count > 0)
mch_memmove(entry, p, count);
XChangeProperty(dpy, RootWindow(dpy, 0), registryProperty, XA_STRING,
8, PropModeReplace, regProp,
(int)(numItems - (p - entry)));
XSync(dpy, False);
}

if (regProp != empty_prop)
XFree(regProp);
return (Window)returnValue;
}










static void
DeleteAnyLingerer(
Display *dpy, 
Window win) 
{
char_u *regProp, *entry = NULL;
char_u *p;
long_u numItems;
int_u wwin;




if (GetRegProp(dpy, &regProp, &numItems, FALSE) == FAIL)
return;


for (p = regProp; (long_u)(p - regProp) < numItems; )
{
if (*p != 0)
{
sscanf((char *)p, "%x", &wwin);
if ((Window)wwin == win)
{
int lastHalf;


entry = p;
while (*p != 0)
p++;
p++;
lastHalf = numItems - (p - regProp);
if (lastHalf > 0)
mch_memmove(entry, p, lastHalf);
numItems = (entry - regProp) + lastHalf;
p = entry;
continue;
}
}
while (*p != 0)
p++;
p++;
}

if (entry != NULL)
{
XChangeProperty(dpy, RootWindow(dpy, 0), registryProperty,
XA_STRING, 8, PropModeReplace, regProp,
(int)(p - regProp));
XSync(dpy, False);
}

if (regProp != empty_prop)
XFree(regProp);
}







static int
GetRegProp(
Display *dpy,
char_u **regPropp,
long_u *numItemsp,
int domsg) 
{
int result, actualFormat;
long_u bytesAfter;
Atom actualType;
XErrorHandler old_handler;

*regPropp = NULL;
old_handler = XSetErrorHandler(x_error_check);
got_x_error = FALSE;

result = XGetWindowProperty(dpy, RootWindow(dpy, 0), registryProperty, 0L,
(long)MAX_PROP_WORDS, False,
XA_STRING, &actualType,
&actualFormat, numItemsp, &bytesAfter,
regPropp);

XSync(dpy, FALSE);
(void)XSetErrorHandler(old_handler);
if (got_x_error)
return FAIL;

if (actualType == None)
{

*numItemsp = 0;
*regPropp = empty_prop;
return OK;
}


if (result != Success || actualFormat != 8 || actualType != XA_STRING)
{
if (*regPropp != NULL)
XFree(*regPropp);
XDeleteProperty(dpy, RootWindow(dpy, 0), registryProperty);
if (domsg)
emsg(_("E251: VIM instance registry property is badly formed. Deleted!"));
return FAIL;
}
return OK;
}










void
serverEventProc(
Display *dpy,
XEvent *eventPtr, 
int immediate) 
{
char_u *propInfo;
int result, actualFormat;
long_u numItems, bytesAfter;
Atom actualType;

if (eventPtr != NULL)
{
if (eventPtr->xproperty.atom != commProperty
|| eventPtr->xproperty.state != PropertyNewValue)
return;
}




propInfo = NULL;
result = XGetWindowProperty(dpy, commWindow, commProperty, 0L,
(long)MAX_PROP_WORDS, True,
XA_STRING, &actualType,
&actualFormat, &numItems, &bytesAfter,
&propInfo);


if (result != Success || actualType != XA_STRING || actualFormat != 8)
{
if (propInfo != NULL)
XFree(propInfo);
return;
}
if (immediate)
server_parse_message(dpy, propInfo, numItems);
else
save_in_queue(propInfo, numItems);
}





static void
save_in_queue(char_u *propInfo, long_u len)
{
x_queue_T *node;

node = ALLOC_ONE(x_queue_T);
if (node == NULL)
return; 
node->propInfo = propInfo;
node->len = len;

if (head.next == NULL) 
{
head.next = &head;
head.prev = &head;
}


node->next = &head;
node->prev = head.prev;
head.prev->next = node;
head.prev = node;
}




void
server_parse_messages(void)
{
x_queue_T *node;

if (!X_DISPLAY)
return; 
while (head.next != NULL && head.next != &head)
{
node = head.next;
head.next = node->next;
node->next->prev = node->prev;
server_parse_message(X_DISPLAY, node->propInfo, node->len);
vim_free(node);
}
}





int
server_waiting(void)
{
return head.next != NULL && head.next != &head;
}






static void
server_parse_message(
Display *dpy,
char_u *propInfo, 
long_u numItems) 
{
char_u *p;
int code;
char_u *tofree;






for (p = propInfo; (long_u)(p - propInfo) < numItems; )
{






if (*p == 0)
{
p++;
continue;
}

if ((*p == 'c' || *p == 'k') && (p[1] == 0))
{
Window resWindow;
char_u *name, *script, *serial, *end;
Bool asKeys = *p == 'k';
char_u *enc;







p += 2;
name = NULL;
resWindow = None;
serial = (char_u *)"";
script = NULL;
enc = NULL;
while ((long_u)(p - propInfo) < numItems && *p == '-')
{
switch (p[1])
{
case 'r':
end = skipwhite(p + 2);
resWindow = 0;
while (vim_isxdigit(*end))
{
resWindow = 16 * resWindow + (long_u)hex2nr(*end);
++end;
}
if (end == p + 2 || *end != ' ')
resWindow = None;
else
{
p = serial = end + 1;
clientWindow = resWindow; 
}
break;
case 'n':
if (p[2] == ' ')
name = p + 3;
break;
case 's':
if (p[2] == ' ')
script = p + 3;
break;
case 'E':
if (p[2] == ' ')
enc = p + 3;
break;
}
while (*p != 0)
p++;
p++;
}

if (script == NULL || name == NULL)
continue;

if (serverName != NULL && STRICMP(name, serverName) == 0)
{
script = serverConvert(enc, script, &tofree);
if (asKeys)
server_to_input_buf(script);
else
{
char_u *res;

res = eval_client_expr_to_string(script);
if (resWindow != None)
{
garray_T reply;


ga_init2(&reply, 1, 100);
(void)ga_grow(&reply, 50 + STRLEN(p_enc));
sprintf(reply.ga_data, "%cr%c-E %s%c-s %s%c-r ",
0, 0, p_enc, 0, serial, 0);
reply.ga_len = 14 + STRLEN(p_enc) + STRLEN(serial);


if (res != NULL)
ga_concat(&reply, res);
else
{
ga_concat(&reply, (char_u *)_(e_invexprmsg));
ga_append(&reply, 0);
ga_concat(&reply, (char_u *)"-c 1");
}
ga_append(&reply, NUL);
(void)AppendPropCarefully(dpy, resWindow, commProperty,
reply.ga_data, reply.ga_len);
ga_clear(&reply);
}
vim_free(res);
}
vim_free(tofree);
}
}
else if (*p == 'r' && p[1] == 0)
{
int serial, gotSerial;
char_u *res;
PendingCommand *pcPtr;
char_u *enc;






p += 2;
gotSerial = 0;
res = (char_u *)"";
code = 0;
enc = NULL;
while ((long_u)(p - propInfo) < numItems && *p == '-')
{
switch (p[1])
{
case 'r':
if (p[2] == ' ')
res = p + 3;
break;
case 'E':
if (p[2] == ' ')
enc = p + 3;
break;
case 's':
if (sscanf((char *)p + 2, " %d", &serial) == 1)
gotSerial = 1;
break;
case 'c':
if (sscanf((char *)p + 2, " %d", &code) != 1)
code = 0;
break;
}
while (*p != 0)
p++;
p++;
}

if (!gotSerial)
continue;





for (pcPtr = pendingCommands; pcPtr != NULL; pcPtr = pcPtr->nextPtr)
{
if (serial != pcPtr->serial || pcPtr->result != NULL)
continue;

pcPtr->code = code;
res = serverConvert(enc, res, &tofree);
if (tofree == NULL)
res = vim_strsave(res);
pcPtr->result = res;
break;
}
}
else if (*p == 'n' && p[1] == 0)
{
Window win = 0;
unsigned int u;
int gotWindow;
char_u *str;
struct ServerReply *r;
char_u *enc;





p += 2;
gotWindow = 0;
str = (char_u *)"";
enc = NULL;
while ((long_u)(p - propInfo) < numItems && *p == '-')
{
switch (p[1])
{
case 'n':
if (p[2] == ' ')
str = p + 3;
break;
case 'E':
if (p[2] == ' ')
enc = p + 3;
break;
case 'w':
if (sscanf((char *)p + 2, " %x", &u) == 1)
{
win = u;
gotWindow = 1;
}
break;
}
while (*p != 0)
p++;
p++;
}

if (!gotWindow)
continue;
str = serverConvert(enc, str, &tofree);
if ((r = ServerReplyFind(win, SROP_Add)) != NULL)
{
ga_concat(&(r->strings), str);
ga_append(&(r->strings), NUL);
}
{
char_u winstr[30];

sprintf((char *)winstr, "0x%x", (unsigned int)win);
apply_autocmds(EVENT_REMOTEREPLY, winstr, str, TRUE, curbuf);
}
vim_free(tofree);
}
else
{






while (*p != 0)
p++;
p++;
}
}
XFree(propInfo);
}







static int
AppendPropCarefully(
Display *dpy, 
Window window, 
Atom property, 
char_u *value, 
int length) 
{
XErrorHandler old_handler;

old_handler = XSetErrorHandler(x_error_check);
got_x_error = FALSE;
XChangeProperty(dpy, window, property, XA_STRING, 8,
PropModeAppend, value, length);
XSync(dpy, False);
(void) XSetErrorHandler(old_handler);
return got_x_error ? -1 : 0;
}





static int
x_error_check(Display *dpy UNUSED, XErrorEvent *error_event UNUSED)
{
got_x_error = TRUE;
return 0;
}





static int
IsSerialName(char_u *str)
{
int len = STRLEN(str);

return (len > 1 && vim_isdigit(str[len - 1]));
}
#endif 
