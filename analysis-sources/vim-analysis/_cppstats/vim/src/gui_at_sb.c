#include "vim.h"
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/XawInit.h>
#include "gui_at_sb.h"
#include <X11/Xmu/Drawing.h>
static char defaultTranslations[] =
"<Btn1Down>: NotifyScroll()\n\
<Btn2Down>: MoveThumb() NotifyThumb()\n\
<Btn3Down>: NotifyScroll()\n\
<Btn4Down>: ScrollOneLineUp()\n\
Shift<Btn4Down>: ScrollPageUp()\n\
<Btn5Down>: ScrollOneLineDown()\n\
Shift<Btn5Down>: ScrollPageDown()\n\
<Btn1Motion>: HandleThumb()\n\
<Btn3Motion>: HandleThumb()\n\
<Btn2Motion>: MoveThumb() NotifyThumb()\n\
<BtnUp>: EndScroll()";
static float floatZero = 0.0;
#define Offset(field) XtOffsetOf(ScrollbarRec, field)
static XtResource resources[] =
{
{XtNlength, XtCLength, XtRDimension, sizeof(Dimension),
Offset(scrollbar.length), XtRImmediate, (XtPointer) 1},
{XtNthickness, XtCThickness, XtRDimension, sizeof(Dimension),
Offset(scrollbar.thickness), XtRImmediate, (XtPointer) 14},
{XtNorientation, XtCOrientation, XtROrientation, sizeof(XtOrientation),
Offset(scrollbar.orientation), XtRImmediate, (XtPointer) XtorientVertical},
{XtNscrollProc, XtCCallback, XtRCallback, sizeof(XtPointer),
Offset(scrollbar.scrollProc), XtRCallback, NULL},
{XtNthumbProc, XtCCallback, XtRCallback, sizeof(XtPointer),
Offset(scrollbar.thumbProc), XtRCallback, NULL},
{XtNjumpProc, XtCCallback, XtRCallback, sizeof(XtPointer),
Offset(scrollbar.jumpProc), XtRCallback, NULL},
{XtNthumb, XtCThumb, XtRBitmap, sizeof(Pixmap),
Offset(scrollbar.thumb), XtRImmediate, (XtPointer) XtUnspecifiedPixmap},
{XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
Offset(scrollbar.foreground), XtRString, XtDefaultForeground},
{XtNshown, XtCShown, XtRFloat, sizeof(float),
Offset(scrollbar.shown), XtRFloat, (XtPointer)&floatZero},
{XtNtopOfThumb, XtCTopOfThumb, XtRFloat, sizeof(float),
Offset(scrollbar.top), XtRFloat, (XtPointer)&floatZero},
{XtNmaxOfThumb, XtCMaxOfThumb, XtRFloat, sizeof(float),
Offset(scrollbar.max), XtRFloat, (XtPointer)&floatZero},
{XtNminimumThumb, XtCMinimumThumb, XtRDimension, sizeof(Dimension),
Offset(scrollbar.min_thumb), XtRImmediate, (XtPointer) 7},
{XtNshadowWidth, XtCShadowWidth, XtRDimension, sizeof(Dimension),
Offset(scrollbar.shadow_width), XtRImmediate, (XtPointer) 1},
{XtNtopShadowPixel, XtCTopShadowPixel, XtRPixel, sizeof(Pixel),
Offset(scrollbar.top_shadow_pixel), XtRString, XtDefaultBackground},
{XtNbottomShadowPixel, XtCBottomShadowPixel, XtRPixel, sizeof(Pixel),
Offset(scrollbar.bot_shadow_pixel), XtRString, XtDefaultForeground},
{XtNlimitThumb, XtCLimitThumb, XtRBool, sizeof(Bool),
Offset(scrollbar.limit_thumb), XtRImmediate, (XtPointer)0}
};
#undef Offset
static void ClassInitialize(void);
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Destroy(Widget);
static void Realize(Widget, Mask *, XSetWindowAttributes *);
static void Resize(Widget);
static void Redisplay(Widget, XEvent *, Region);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static void HandleThumb(Widget, XEvent *, String *, Cardinal *);
static void MoveThumb(Widget, XEvent *, String *, Cardinal *);
static void NotifyThumb(Widget, XEvent *, String *, Cardinal *);
static void NotifyScroll(Widget, XEvent *, String *, Cardinal *);
static void EndScroll(Widget, XEvent *, String *, Cardinal *);
static void ScrollOneLineUp(Widget, XEvent *, String *, Cardinal *);
static void ScrollOneLineDown(Widget, XEvent *, String *, Cardinal *);
static void ScrollPageUp(Widget, XEvent *, String *, Cardinal *);
static void ScrollPageDown(Widget, XEvent *, String *, Cardinal *);
static void ScrollSome(Widget w, XEvent *event, int call_data);
static void _Xaw3dDrawShadows(Widget, XEvent *, Region, int);
static void AllocTopShadowGC(Widget);
static void AllocBotShadowGC(Widget);
static XtActionsRec actions[] =
{
{"HandleThumb", HandleThumb},
{"MoveThumb", MoveThumb},
{"NotifyThumb", NotifyThumb},
{"NotifyScroll", NotifyScroll},
{"EndScroll", EndScroll},
{"ScrollOneLineUp", ScrollOneLineUp},
{"ScrollOneLineDown", ScrollOneLineDown},
{"ScrollPageUp", ScrollPageUp},
{"ScrollPageDown", ScrollPageDown}
};
ScrollbarClassRec vim_scrollbarClassRec =
{
{ 
(WidgetClass) &simpleClassRec,
"Scrollbar",
sizeof(ScrollbarRec),
ClassInitialize,
NULL,
FALSE,
Initialize,
NULL,
Realize,
actions,
XtNumber(actions),
resources,
XtNumber(resources),
NULLQUARK,
TRUE,
TRUE,
TRUE,
FALSE,
Destroy,
Resize,
Redisplay,
SetValues,
NULL,
XtInheritSetValuesAlmost,
NULL,
NULL,
XtVersion,
NULL,
defaultTranslations,
XtInheritQueryGeometry,
XtInheritDisplayAccelerator,
NULL
},
{ 
XtInheritChangeSensitive,
#if !defined(OLDXAW)
NULL
#endif
},
{ 
0
}
};
WidgetClass vim_scrollbarWidgetClass = (WidgetClass)&vim_scrollbarClassRec;
#define NoButton -1
#define PICKLENGTH(widget, x, y) ((widget->scrollbar.orientation == XtorientHorizontal) ? (x) : (y))
#define AT_MIN(x,y) ((x) < (y) ? (x) : (y))
#define AT_MAX(x,y) ((x) > (y) ? (x) : (y))
#define LINE_DELAY 300
#define PAGE_DELAY 300
#define LINE_REPEAT 50
#define PAGE_REPEAT 250
static void
ClassInitialize(void)
{
XawInitializeWidgetSet();
XtAddConverter( XtRString, XtROrientation, XmuCvtStringToOrientation,
(XtConvertArgList)NULL, (Cardinal)0 );
}
#define MARGIN(sbw) (sbw)->scrollbar.thickness + (sbw)->scrollbar.shadow_width
static void
FillArea(
ScrollbarWidget sbw,
Position top,
Position bottom,
int fill,
int draw_shadow)
{
int tlen = bottom - top; 
int sw, margin, floor;
int lx, ly, lw, lh;
if (bottom <= 0 || bottom <= top)
return;
sw = sbw->scrollbar.shadow_width;
if (sw < 0)
sw = 0;
margin = MARGIN (sbw);
floor = sbw->scrollbar.length - margin + 2;
if (sbw->scrollbar.orientation == XtorientHorizontal)
{
lx = ((top < margin) ? margin : top);
ly = sw;
lw = (((top + tlen) > floor) ? floor - top : tlen);
lh = sbw->core.height - 2 * sw;
}
else
{
lx = sw;
ly = ((top < margin) ? margin : top);
lw = sbw->core.width - 2 * sw;
lh = (((top + tlen) > floor) ? floor - top : tlen);
}
if (lh <= 0 || lw <= 0)
return;
if (draw_shadow)
{
if (!(sbw->scrollbar.orientation == XtorientHorizontal))
{
XDrawLine (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.top_shadow_GC,
lx, ly, lx + lw - 1, ly);
XDrawLine (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.bot_shadow_GC,
lx, ly + lh - 1, lx + lw - 1, ly + lh - 1);
}
else
{
XDrawLine (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.top_shadow_GC,
lx, ly, lx, ly + lh - 1);
XDrawLine (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.bot_shadow_GC,
lx + lw - 1, ly, lx + lw - 1, ly + lh - 1);
}
return;
}
if (fill)
{
XFillRectangle(XtDisplay((Widget) sbw), XtWindow((Widget) sbw),
sbw->scrollbar.gc,
lx, ly, (unsigned int) lw, (unsigned int) lh);
if (!(sbw->scrollbar.orientation == XtorientHorizontal))
{
XDrawLine(XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.top_shadow_GC,
lx, ly, lx, ly + lh - 1);
XDrawLine(XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.bot_shadow_GC,
lx + lw - 1, ly, lx + lw - 1, ly + lh - 1);
}
else
{
XDrawLine(XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.top_shadow_GC,
lx, ly, lx + lw - 1, ly);
XDrawLine(XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.bot_shadow_GC,
lx, ly + lh - 1, lx + lw - 1, ly + lh - 1);
}
}
else
{
XClearArea(XtDisplay((Widget) sbw), XtWindow((Widget) sbw),
lx, ly, (unsigned int) lw, (unsigned int) lh, FALSE);
}
}
static void
PaintThumb(ScrollbarWidget sbw)
{
Position oldtop, oldbot, newtop, newbot;
Dimension margin, tzl;
margin = MARGIN (sbw);
tzl = sbw->scrollbar.length - 2 * margin;
newtop = margin + (int)(tzl * sbw->scrollbar.top);
newbot = newtop + (int)(tzl * sbw->scrollbar.shown) + 1;
if (newbot < newtop + (int)sbw->scrollbar.min_thumb)
newbot = newtop + sbw->scrollbar.min_thumb;
oldtop = sbw->scrollbar.topLoc;
oldbot = oldtop + sbw->scrollbar.shownLength;
sbw->scrollbar.topLoc = newtop;
sbw->scrollbar.shownLength = newbot - newtop;
if (XtIsRealized ((Widget) sbw))
{
if (newtop < oldtop)
FillArea(sbw, newtop, AT_MIN(newbot, oldtop+1),1,0);
if (newtop > oldtop)
FillArea(sbw, oldtop, AT_MIN(newtop, oldbot ),0,0);
if (newbot < oldbot)
FillArea(sbw, AT_MAX(newbot, oldtop), oldbot, 0,0);
if (newbot > oldbot)
FillArea(sbw, AT_MAX(newtop, oldbot-1), newbot, 1,0);
FillArea(sbw, newtop, newbot, 0, 1);
}
}
static void
PaintArrows(ScrollbarWidget sbw)
{
XPoint point[6];
Dimension thickness = sbw->scrollbar.thickness - 1;
Dimension size;
Dimension off;
if (XtIsRealized((Widget) sbw))
{
if ((int)thickness * 2 > (int)sbw->scrollbar.length)
{
size = sbw->scrollbar.length / 2;
off = (int)(thickness - size) / 2;
}
else
{
size = thickness;
off = 0;
}
point[0].x = off + sbw->scrollbar.shadow_width;
point[0].y = size;
point[1].x = thickness - off - sbw->scrollbar.shadow_width;
point[1].y = size;
point[2].x = thickness / 2;
point[2].y = sbw->scrollbar.shadow_width;
point[3].x = off + sbw->scrollbar.shadow_width;
point[3].y = sbw->scrollbar.length - size;
point[4].x = thickness - off - sbw->scrollbar.shadow_width;
point[4].y = sbw->scrollbar.length - size;
point[5].x = thickness / 2;
point[5].y = sbw->scrollbar.length - sbw->scrollbar.shadow_width - 1;
if (sbw->scrollbar.orientation == XtorientHorizontal)
{
int n;
int swap;
for (n = 0; n < 6; n++)
{
swap = point[n].x;
point[n].x = point[n].y;
point[n].y = swap;
}
}
XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.gc,
point, 3,
Convex, CoordModeOrigin);
XDrawLines (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.bot_shadow_GC,
point, 3,
CoordModeOrigin);
XDrawLine (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.top_shadow_GC,
point[0].x, point[0].y,
point[2].x, point[2].y);
XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.gc,
point+3, 3,
Convex, CoordModeOrigin);
XDrawLine (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.top_shadow_GC,
point[3].x, point[3].y,
point[4].x, point[4].y);
XDrawLine (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.top_shadow_GC,
point[3].x, point[3].y,
point[5].x, point[5].y);
XDrawLine (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
sbw->scrollbar.bot_shadow_GC,
point[4].x, point[4].y,
point[5].x, point[5].y);
}
}
static void
Destroy(Widget w)
{
ScrollbarWidget sbw = (ScrollbarWidget) w;
if (sbw->scrollbar.timer_id != (XtIntervalId) 0)
XtRemoveTimeOut (sbw->scrollbar.timer_id);
XtReleaseGC(w, sbw->scrollbar.gc);
XtReleaseGC(w, sbw->scrollbar.top_shadow_GC);
XtReleaseGC(w, sbw->scrollbar.bot_shadow_GC);
}
static void
CreateGC(Widget w)
{
ScrollbarWidget sbw = (ScrollbarWidget) w;
XGCValues gcValues;
XtGCMask mask;
unsigned int depth = 1;
if (sbw->scrollbar.thumb == XtUnspecifiedPixmap)
{
sbw->scrollbar.thumb = XmuCreateStippledPixmap (XtScreen(w),
(Pixel) 1, (Pixel) 0, depth);
}
else if (sbw->scrollbar.thumb != None)
{
Window root;
int x, y;
unsigned int width, height, bw;
if (XGetGeometry (XtDisplay(w), sbw->scrollbar.thumb, &root, &x, &y,
&width, &height, &bw, &depth) == 0)
emsg(_("Scrollbar Widget: Could not get geometry of thumb pixmap."));
}
gcValues.foreground = sbw->scrollbar.foreground;
gcValues.background = sbw->core.background_pixel;
mask = GCForeground | GCBackground;
if (sbw->scrollbar.thumb != None)
{
gcValues.fill_style = FillSolid;
mask |= GCFillStyle;
}
sbw->scrollbar.gc = XtGetGC (w, mask, &gcValues);
}
static void
SetDimensions(ScrollbarWidget sbw)
{
if (sbw->scrollbar.orientation == XtorientVertical)
{
sbw->scrollbar.length = sbw->core.height;
sbw->scrollbar.thickness = sbw->core.width;
}
else
{
sbw->scrollbar.length = sbw->core.width;
sbw->scrollbar.thickness = sbw->core.height;
}
}
static void
Initialize(
Widget request UNUSED, 
Widget new, 
ArgList args UNUSED,
Cardinal *num_args UNUSED)
{
ScrollbarWidget sbw = (ScrollbarWidget) new;
CreateGC(new);
AllocTopShadowGC(new);
AllocBotShadowGC(new);
if (sbw->core.width == 0)
sbw->core.width = (sbw->scrollbar.orientation == XtorientVertical)
? sbw->scrollbar.thickness : sbw->scrollbar.length;
if (sbw->core.height == 0)
sbw->core.height = (sbw->scrollbar.orientation == XtorientHorizontal)
? sbw->scrollbar.thickness : sbw->scrollbar.length;
SetDimensions(sbw);
sbw->scrollbar.scroll_mode = SMODE_NONE;
sbw->scrollbar.timer_id = (XtIntervalId)0;
sbw->scrollbar.topLoc = 0;
sbw->scrollbar.shownLength = sbw->scrollbar.min_thumb;
}
static void
Realize(
Widget w,
Mask *valueMask,
XSetWindowAttributes *attributes)
{
(*vim_scrollbarWidgetClass->core_class.superclass->core_class.realize)
(w, valueMask, attributes);
}
static Boolean
SetValues(
Widget current, 
Widget request UNUSED, 
Widget desired, 
ArgList args UNUSED,
Cardinal *num_args UNUSED)
{
ScrollbarWidget sbw = (ScrollbarWidget) current;
ScrollbarWidget dsbw = (ScrollbarWidget) desired;
Boolean redraw = FALSE;
if (dsbw->scrollbar.top < 0.0 || dsbw->scrollbar.top > 1.0)
dsbw->scrollbar.top = sbw->scrollbar.top;
if (dsbw->scrollbar.shown < 0.0 || dsbw->scrollbar.shown > 1.0)
dsbw->scrollbar.shown = sbw->scrollbar.shown;
if (XtIsRealized(desired))
{
if (sbw->scrollbar.foreground != dsbw->scrollbar.foreground ||
sbw->core.background_pixel != dsbw->core.background_pixel ||
sbw->scrollbar.thumb != dsbw->scrollbar.thumb)
{
XtReleaseGC(desired, sbw->scrollbar.gc);
CreateGC (desired);
redraw = TRUE;
}
if (sbw->scrollbar.top != dsbw->scrollbar.top ||
sbw->scrollbar.shown != dsbw->scrollbar.shown)
redraw = TRUE;
}
return redraw;
}
static void
Resize(Widget w)
{
SetDimensions ((ScrollbarWidget) w);
Redisplay(w, (XEvent*) NULL, (Region)NULL);
}
static void
Redisplay(Widget w, XEvent *event, Region region)
{
ScrollbarWidget sbw = (ScrollbarWidget) w;
int x, y;
unsigned int width, height;
_Xaw3dDrawShadows(w, event, region, FALSE);
if (sbw->scrollbar.orientation == XtorientHorizontal)
{
x = sbw->scrollbar.topLoc;
y = 1;
width = sbw->scrollbar.shownLength;
height = sbw->core.height - 2;
}
else
{
x = 1;
y = sbw->scrollbar.topLoc;
width = sbw->core.width - 2;
height = sbw->scrollbar.shownLength;
}
if (region == NULL ||
XRectInRegion (region, x, y, width, height) != RectangleOut)
{
sbw->scrollbar.topLoc = -(sbw->scrollbar.length + 1);
PaintThumb (sbw);
}
PaintArrows(sbw);
}
static Boolean
CompareEvents(XEvent *oldEvent, XEvent *newEvent)
{
#define Check(field) do { if (newEvent->field != oldEvent->field) return False; } while (0)
Check(xany.display);
Check(xany.type);
Check(xany.window);
switch (newEvent->type)
{
case MotionNotify:
Check(xmotion.state);
break;
case ButtonPress:
case ButtonRelease:
Check(xbutton.state);
Check(xbutton.button);
break;
case KeyPress:
case KeyRelease:
Check(xkey.state);
Check(xkey.keycode);
break;
case EnterNotify:
case LeaveNotify:
Check(xcrossing.mode);
Check(xcrossing.detail);
Check(xcrossing.state);
break;
}
#undef Check
return True;
}
struct EventData
{
XEvent *oldEvent;
int count;
};
static Bool
PeekNotifyEvent(Display *dpy, XEvent *event, char *args)
{
struct EventData *eventData = (struct EventData*)args;
return ((++eventData->count == QLength(dpy)) 
|| CompareEvents(event, eventData->oldEvent));
}
static Boolean
LookAhead(Widget w, XEvent *event)
{
XEvent newEvent;
struct EventData eventData;
if (QLength (XtDisplay (w)) == 0)
return False;
eventData.count = 0;
eventData.oldEvent = event;
XPeekIfEvent (XtDisplay (w), &newEvent, PeekNotifyEvent, (char*)&eventData);
return CompareEvents (event, &newEvent);
}
static void
ExtractPosition(
XEvent *event,
Position *x, 
Position *y, 
unsigned int *state) 
{
switch (event->type)
{
case MotionNotify:
*x = event->xmotion.x;
*y = event->xmotion.y;
if (state != NULL)
*state = event->xmotion.state;
break;
case ButtonPress:
case ButtonRelease:
*x = event->xbutton.x;
*y = event->xbutton.y;
if (state != NULL)
*state = event->xbutton.state;
break;
case KeyPress:
case KeyRelease:
*x = event->xkey.x;
*y = event->xkey.y;
if (state != NULL)
*state = event->xkey.state;
break;
case EnterNotify:
case LeaveNotify:
*x = event->xcrossing.x;
*y = event->xcrossing.y;
if (state != NULL)
*state = event->xcrossing.state;
break;
default:
*x = 0; *y = 0;
if (state != NULL)
*state = 0;
}
}
static void
HandleThumb(
Widget w,
XEvent *event,
String *params,
Cardinal *num_params)
{
Position x, y, loc;
ScrollbarWidget sbw = (ScrollbarWidget) w;
ExtractPosition(event, &x, &y, (unsigned int *)NULL);
loc = PICKLENGTH(sbw, x, y);
if (sbw->scrollbar.scroll_mode == SMODE_CONT ||
(loc >= sbw->scrollbar.topLoc &&
loc <= sbw->scrollbar.topLoc + (int)sbw->scrollbar.shownLength))
{
XtCallActionProc(w, "MoveThumb", event, params, *num_params);
XtCallActionProc(w, "NotifyThumb", event, params, *num_params);
}
}
static void
RepeatNotify(XtPointer client_data, XtIntervalId *idp UNUSED)
{
ScrollbarWidget sbw = (ScrollbarWidget) client_data;
int call_data;
char mode = sbw->scrollbar.scroll_mode;
unsigned long rep;
if (mode == SMODE_NONE || mode == SMODE_CONT)
{
sbw->scrollbar.timer_id = (XtIntervalId)0;
return;
}
if (mode == SMODE_LINE_DOWN || mode == SMODE_LINE_UP)
{
call_data = ONE_LINE_DATA;
rep = LINE_REPEAT;
}
else
{
call_data = ONE_PAGE_DATA;
rep = PAGE_REPEAT;
}
if (mode == SMODE_PAGE_UP || mode == SMODE_LINE_UP)
call_data = -call_data;
XtCallCallbacks((Widget)sbw, XtNscrollProc, (XtPointer)(long_u)call_data);
sbw->scrollbar.timer_id =
XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)sbw),
rep,
RepeatNotify,
client_data);
}
static float
FloatInRange(float num, float small, float big)
{
return (num < small) ? small : ((num > big) ? big : num);
}
static void
ScrollOneLineUp(
Widget w,
XEvent *event,
String *params UNUSED,
Cardinal *num_params UNUSED)
{
ScrollSome(w, event, -ONE_LINE_DATA);
}
static void
ScrollOneLineDown(
Widget w,
XEvent *event,
String *params UNUSED,
Cardinal *num_params UNUSED)
{
ScrollSome(w, event, ONE_LINE_DATA);
}
static void
ScrollPageDown(
Widget w,
XEvent *event,
String *params UNUSED,
Cardinal *num_params UNUSED)
{
ScrollSome(w, event, ONE_PAGE_DATA);
}
static void
ScrollPageUp(
Widget w,
XEvent *event,
String *params UNUSED,
Cardinal *num_params UNUSED)
{
ScrollSome(w, event, -ONE_PAGE_DATA);
}
static void
ScrollSome(
Widget w,
XEvent *event,
int call_data)
{
ScrollbarWidget sbw = (ScrollbarWidget) w;
if (sbw->scrollbar.scroll_mode == SMODE_CONT) 
return;
if (LookAhead(w, event))
return;
sbw->scrollbar.scroll_mode = SMODE_LINE_UP;
XtCallCallbacks(w, XtNscrollProc, (XtPointer)(long_u)call_data);
}
static void
NotifyScroll(
Widget w,
XEvent *event,
String *params UNUSED,
Cardinal *num_params UNUSED)
{
ScrollbarWidget sbw = (ScrollbarWidget) w;
Position x, y, loc;
Dimension arrow_size;
unsigned long delay = 0;
int call_data = 0;
unsigned int state;
if (sbw->scrollbar.scroll_mode == SMODE_CONT) 
return;
if (LookAhead (w, event))
return;
ExtractPosition(event, &x, &y, &state);
loc = PICKLENGTH(sbw, x, y);
if ((int)sbw->scrollbar.thickness * 2 > (int)sbw->scrollbar.length)
arrow_size = sbw->scrollbar.length / 2;
else
arrow_size = sbw->scrollbar.thickness;
if (state & ControlMask)
{
if (loc > sbw->scrollbar.topLoc + (Position)sbw->scrollbar.shownLength)
call_data = END_PAGE_DATA;
else
call_data = -END_PAGE_DATA;
sbw->scrollbar.scroll_mode = SMODE_NONE;
}
else if (loc < (Position)arrow_size)
{
call_data = -ONE_LINE_DATA;
sbw->scrollbar.scroll_mode = SMODE_LINE_UP;
delay = LINE_DELAY;
}
else if (loc > (Position)(sbw->scrollbar.length - arrow_size))
{
call_data = ONE_LINE_DATA;
sbw->scrollbar.scroll_mode = SMODE_LINE_DOWN;
delay = LINE_DELAY;
}
else if (loc < sbw->scrollbar.topLoc)
{
call_data = -ONE_PAGE_DATA;
sbw->scrollbar.scroll_mode = SMODE_PAGE_UP;
delay = PAGE_DELAY;
}
else if (loc > sbw->scrollbar.topLoc + (Position)sbw->scrollbar.shownLength)
{
call_data = ONE_PAGE_DATA;
sbw->scrollbar.scroll_mode = SMODE_PAGE_DOWN;
delay = PAGE_DELAY;
}
if (call_data)
XtCallCallbacks(w, XtNscrollProc, (XtPointer)(long_u)call_data);
if (delay)
sbw->scrollbar.timer_id =
XtAppAddTimeOut(XtWidgetToApplicationContext(w),
delay, RepeatNotify, (XtPointer)w);
}
static void
EndScroll(
Widget w,
XEvent *event UNUSED,
String *params UNUSED,
Cardinal *num_params UNUSED)
{
ScrollbarWidget sbw = (ScrollbarWidget) w;
sbw->scrollbar.scroll_mode = SMODE_NONE;
}
static float
FractionLoc(ScrollbarWidget sbw, int x, int y)
{
int margin;
float height, width;
margin = MARGIN(sbw);
x -= margin;
y -= margin;
height = (float)sbw->core.height - 2 * margin;
width = (float)sbw->core.width - 2 * margin;
return PICKLENGTH(sbw, x / width, y / height);
}
static void
MoveThumb(
Widget w,
XEvent *event,
String *params UNUSED,
Cardinal *num_params UNUSED)
{
ScrollbarWidget sbw = (ScrollbarWidget)w;
Position x, y;
float top;
char old_mode = sbw->scrollbar.scroll_mode;
sbw->scrollbar.scroll_mode = SMODE_CONT; 
if (LookAhead(w, event))
return;
if (!event->xmotion.same_screen)
return;
ExtractPosition(event, &x, &y, (unsigned int *)NULL);
top = FractionLoc(sbw, x, y);
if (old_mode != SMODE_CONT) 
{
if (event->xbutton.button == Button2)
sbw->scrollbar.scroll_off = sbw->scrollbar.shown / 2.;
else
sbw->scrollbar.scroll_off = top - sbw->scrollbar.top;
}
top -= sbw->scrollbar.scroll_off;
if (sbw->scrollbar.limit_thumb)
top = FloatInRange(top, 0.0,
sbw->scrollbar.max - sbw->scrollbar.shown + 0.000001);
else
top = FloatInRange(top, 0.0, sbw->scrollbar.max);
sbw->scrollbar.top = top;
PaintThumb(sbw);
XFlush(XtDisplay(w)); 
}
static void
NotifyThumb(
Widget w,
XEvent *event,
String *params UNUSED,
Cardinal *num_params UNUSED)
{
ScrollbarWidget sbw = (ScrollbarWidget)w;
union {
XtPointer xtp;
float xtf;
} xtpf;
if (LookAhead(w, event))
return;
xtpf.xtf = sbw->scrollbar.top;
XtCallCallbacks(w, XtNthumbProc, xtpf.xtp);
XtCallCallbacks(w, XtNjumpProc, (XtPointer)&sbw->scrollbar.top);
}
static void
AllocTopShadowGC(Widget w)
{
ScrollbarWidget sbw = (ScrollbarWidget) w;
XtGCMask valuemask;
XGCValues myXGCV;
valuemask = GCForeground;
myXGCV.foreground = sbw->scrollbar.top_shadow_pixel;
sbw->scrollbar.top_shadow_GC = XtGetGC(w, valuemask, &myXGCV);
}
static void
AllocBotShadowGC(Widget w)
{
ScrollbarWidget sbw = (ScrollbarWidget) w;
XtGCMask valuemask;
XGCValues myXGCV;
valuemask = GCForeground;
myXGCV.foreground = sbw->scrollbar.bot_shadow_pixel;
sbw->scrollbar.bot_shadow_GC = XtGetGC(w, valuemask, &myXGCV);
}
static void
_Xaw3dDrawShadows(
Widget gw,
XEvent *event UNUSED,
Region region,
int out)
{
XPoint pt[6];
ScrollbarWidget sbw = (ScrollbarWidget) gw;
Dimension s = sbw->scrollbar.shadow_width;
if (s > 0 && XtIsRealized(gw))
{
Dimension h = sbw->core.height;
Dimension w = sbw->core.width;
Dimension wms = w - s;
Dimension hms = h - s;
Display *dpy = XtDisplay (gw);
Window win = XtWindow (gw);
GC top, bot;
if (out)
{
top = sbw->scrollbar.top_shadow_GC;
bot = sbw->scrollbar.bot_shadow_GC;
}
else
{
top = sbw->scrollbar.bot_shadow_GC;
bot = sbw->scrollbar.top_shadow_GC;
}
if ((region == NULL) ||
(XRectInRegion (region, 0, 0, w, s) != RectangleOut) ||
(XRectInRegion (region, 0, 0, s, h) != RectangleOut))
{
pt[0].x = 0; pt[0].y = h;
pt[1].x = pt[1].y = 0;
pt[2].x = w; pt[2].y = 0;
pt[3].x = wms; pt[3].y = s;
pt[4].x = pt[4].y = s;
pt[5].x = s; pt[5].y = hms;
XFillPolygon (dpy, win, top, pt, 6, Complex, CoordModeOrigin);
}
if ((region == NULL) ||
(XRectInRegion (region, 0, hms, w, s) != RectangleOut) ||
(XRectInRegion (region, wms, 0, s, h) != RectangleOut))
{
pt[0].x = 0; pt[0].y = h;
pt[1].x = w; pt[1].y = h;
pt[2].x = w; pt[2].y = 0;
pt[3].x = wms; pt[3].y = s;
pt[4].x = wms; pt[4].y = hms;
pt[5].x = s; pt[5].y = hms;
XFillPolygon (dpy, win, bot, pt, 6, Complex, CoordModeOrigin);
}
}
}
void
vim_XawScrollbarSetThumb(Widget w, double top, double shown, double max)
{
ScrollbarWidget sbw = (ScrollbarWidget) w;
if (sbw->scrollbar.scroll_mode == SMODE_CONT) 
return;
sbw->scrollbar.max = (max > 1.0) ? 1.0 :
(max >= 0.0) ? max : sbw->scrollbar.max;
sbw->scrollbar.top = (top > sbw->scrollbar.max) ? sbw->scrollbar.max :
(top >= 0.0) ? top : sbw->scrollbar.top;
sbw->scrollbar.shown = (shown > 1.0) ? 1.0 :
(shown >= 0.0) ? shown : sbw->scrollbar.shown;
PaintThumb(sbw);
}
