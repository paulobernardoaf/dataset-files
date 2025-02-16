#include <stdio.h>
#include <string.h>
#if defined(HAVE_SELECT)
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include <sys/poll.h>
#endif
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
char * sendToVim(Display *dpy, char *name, char *cmd, int asKeys, int *code);
#if defined(MAIN)
main(int argc, char **argv)
{
char *res;
int code;
if (argc == 4)
{
if ((res = sendToVim(XOpenDisplay(NULL), argv[2], argv[3],
argv[1][0] != 'e', &code)) != NULL)
{
if (code)
printf("Error code returned: %d\n", code);
puts(res);
}
exit(0);
}
else
fprintf(stderr, "Usage: %s {k|e} <server> <command>", argv[0]);
exit(1);
}
#endif
#define MAX_PROP_WORDS 100000
static int x_error_check(Display *dpy, XErrorEvent *error_event);
static int AppendPropCarefully(Display *display,
Window window, Atom property, char *value, int length);
static Window LookupName(Display *dpy, char *name,
int delete, char **loose);
static int SendInit(Display *dpy);
static char *SendEventProc(Display *dpy, XEvent *eventPtr,
int expect, int *code);
static int IsSerialName(char *name);
static Atom registryProperty = None;
static Atom commProperty = None;
static Window commWindow = None;
static int got_x_error = FALSE;
char *
sendToVim(
Display *dpy, 
char *name, 
char *cmd, 
int asKeys, 
int *code) 
{
Window w;
Atom *plist;
XErrorHandler old_handler;
#define STATIC_SPACE 500
char *property, staticSpace[STATIC_SPACE];
int length;
int res;
static int serial = 0; 
XEvent event;
XPropertyEvent *e = (XPropertyEvent *)&event;
time_t start;
char *result;
char *loosename = NULL;
if (commProperty == None && dpy != NULL)
{
if (SendInit(dpy) < 0)
return NULL;
}
old_handler = XSetErrorHandler(x_error_check);
while (TRUE)
{
got_x_error = FALSE;
w = LookupName(dpy, name, 0, &loosename);
if (w != None)
{
plist = XListProperties(dpy, w, &res);
XSync(dpy, False);
if (plist != NULL)
XFree(plist);
if (got_x_error)
{
LookupName(dpy, loosename ? loosename : name,
TRUE, NULL);
continue;
}
}
break;
}
if (w == None)
{
fprintf(stderr, "no registered server named %s\n", name);
return NULL;
}
else if (loosename != NULL)
name = loosename;
length = strlen(name) + strlen(cmd) + 10;
if (length <= STATIC_SPACE)
property = staticSpace;
else
property = (char *) malloc((unsigned) length);
serial++;
sprintf(property, "%c%c%c-n %s%c-s %s",
0, asKeys ? 'k' : 'c', 0, name, 0, cmd);
if (name == loosename)
free(loosename);
if (!asKeys)
{
sprintf(property + length, "%c-r %x %d", 0, (uint) commWindow, serial);
length += strlen(property + length + 1) + 1;
}
res = AppendPropCarefully(dpy, w, commProperty, property, length + 1);
if (length > STATIC_SPACE)
free(property);
if (res < 0)
{
fprintf(stderr, "Failed to send command to the destination program\n");
return NULL;
}
if (asKeys) 
return NULL;
#define SEND_MSEC_POLL 50
time(&start);
while ((time((time_t *) 0) - start) < 60)
{
#if !defined(HAVE_SELECT)
struct pollfd fds;
fds.fd = ConnectionNumber(dpy);
fds.events = POLLIN;
if (poll(&fds, 1, SEND_MSEC_POLL) < 0)
break;
#else
fd_set fds;
struct timeval tv;
tv.tv_sec = 0;
tv.tv_usec = SEND_MSEC_POLL * 1000;
FD_ZERO(&fds);
FD_SET(ConnectionNumber(dpy), &fds);
if (select(ConnectionNumber(dpy) + 1, &fds, NULL, NULL, &tv) < 0)
break;
#endif
while (XEventsQueued(dpy, QueuedAfterReading) > 0)
{
XNextEvent(dpy, &event);
if (event.type == PropertyNotify && e->window == commWindow)
if ((result = SendEventProc(dpy, &event, serial, code)) != NULL)
return result;
}
}
return NULL;
}
static int
SendInit(Display *dpy)
{
XErrorHandler old_handler;
old_handler = XSetErrorHandler(x_error_check);
got_x_error = FALSE;
commProperty = XInternAtom(dpy, "Comm", False);
registryProperty = XInternAtom(dpy, "VimRegistry", False);
if (commWindow == None)
{
commWindow =
XCreateSimpleWindow(dpy, XDefaultRootWindow(dpy),
getpid(), 0, 10, 10, 0,
WhitePixel(dpy, DefaultScreen(dpy)),
WhitePixel(dpy, DefaultScreen(dpy)));
XSelectInput(dpy, commWindow, PropertyChangeMask);
}
XSync(dpy, False);
(void) XSetErrorHandler(old_handler);
return got_x_error ? -1 : 0;
}
static Window
LookupName(
Display *dpy, 
char *name, 
int delete, 
char **loose) 
{
unsigned char *regProp, *entry;
unsigned char *p;
int result, actualFormat;
unsigned long numItems, bytesAfter;
Atom actualType;
Window returnValue;
regProp = NULL;
result = XGetWindowProperty(dpy, RootWindow(dpy, 0), registryProperty, 0,
MAX_PROP_WORDS, False, XA_STRING, &actualType,
&actualFormat, &numItems, &bytesAfter,
&regProp);
if (actualType == None)
return 0;
if ((result != Success) || (actualFormat != 8) || (actualType != XA_STRING))
{
if (regProp != NULL)
XFree(regProp);
XDeleteProperty(dpy, RootWindow(dpy, 0), registryProperty);
return 0;
}
returnValue = None;
entry = NULL; 
for (p = regProp; (p - regProp) < numItems; )
{
entry = p;
while ((*p != 0) && (!isspace(*p)))
p++;
if ((*p != 0) && (strcasecmp(name, p + 1) == 0))
{
sscanf(entry, "%x", (uint*) &returnValue);
break;
}
while (*p != 0)
p++;
p++;
}
if (loose != NULL && returnValue == None && !IsSerialName(name))
{
for (p = regProp; (p - regProp) < numItems; )
{
entry = p;
while ((*p != 0) && (!isspace(*p)))
p++;
if ((*p != 0) && IsSerialName(p + 1)
&& (strncmp(name, p + 1, strlen(name)) == 0))
{
sscanf(entry, "%x", (uint*) &returnValue);
*loose = strdup(p + 1);
break;
}
while (*p != 0)
p++;
p++;
}
}
if ((delete) && (returnValue != None))
{
int count;
while (*p != 0)
p++;
p++;
count = numItems - (p-regProp);
if (count > 0)
memcpy(entry, p, count);
XChangeProperty(dpy, RootWindow(dpy, 0), registryProperty, XA_STRING,
8, PropModeReplace, regProp,
(int) (numItems - (p-entry)));
XSync(dpy, False);
}
XFree(regProp);
return returnValue;
}
static char *
SendEventProc(
Display *dpy,
XEvent *eventPtr, 
int expected, 
int *code) 
{
unsigned char *propInfo;
unsigned char *p;
int result, actualFormat;
int retCode;
unsigned long numItems, bytesAfter;
Atom actualType;
if ((eventPtr->xproperty.atom != commProperty)
|| (eventPtr->xproperty.state != PropertyNewValue))
{
return;
}
propInfo = NULL;
result = XGetWindowProperty(dpy, commWindow, commProperty, 0,
MAX_PROP_WORDS, True, XA_STRING, &actualType,
&actualFormat, &numItems, &bytesAfter,
&propInfo);
if ((result != Success) || (actualType != XA_STRING)
|| (actualFormat != 8))
{
if (propInfo != NULL)
{
XFree(propInfo);
}
return;
}
for (p = propInfo; (p - propInfo) < numItems; )
{
if (*p == 0)
{
p++;
continue;
}
if ((*p == 'r') && (p[1] == 0))
{
int serial, gotSerial;
char *res;
p += 2;
gotSerial = 0;
res = "";
retCode = 0;
while (((p-propInfo) < numItems) && (*p == '-'))
{
switch (p[1])
{
case 'r':
if (p[2] == ' ')
res = p + 3;
break;
case 's':
if (sscanf(p + 2, " %d", &serial) == 1)
gotSerial = 1;
break;
case 'c':
if (sscanf(p + 2, " %d", &retCode) != 1)
retCode = 0;
break;
}
while (*p != 0)
p++;
p++;
}
if (!gotSerial)
continue;
if (code != NULL)
*code = retCode;
return serial == expected ? strdup(res) : NULL;
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
char *value, 
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
x_error_check(Display *dpy, XErrorEvent *error_event)
{
got_x_error = TRUE;
return 0;
}
static int
IsSerialName(char *str)
{
int len = strlen(str);
return (len > 1 && isdigit(str[len - 1]));
}
