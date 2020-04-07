#include <X11/IntrinsicP.h>
#include <X11/Xaw/SimpleP.h>
#include <X11/Xmu/Converters.h>
#define XtCMinimumThumb "MinimumThumb"
#define XtCShown "Shown"
#define XtCTopOfThumb "TopOfThumb"
#define XtCMaxOfThumb "MaxOfThumb"
#define XtCShadowWidth "ShadowWidth"
#define XtCTopShadowPixel "TopShadowPixel"
#define XtCBottomShadowPixel "BottomShadowPixel"
#define XtCLimitThumb "LimitThumb"
#define XtNminimumThumb "minimumThumb"
#define XtNtopOfThumb "topOfThumb"
#define XtNmaxOfThumb "maxOfThumb"
#define XtNshadowWidth "shadowWidth"
#define XtNtopShadowPixel "topShadowPixel"
#define XtNbottomShadowPixel "bottomShadowPixel"
#define XtNlimitThumb "limitThumb"
typedef struct _ScrollbarRec *ScrollbarWidget;
typedef struct _ScrollbarClassRec *ScrollbarWidgetClass;
extern WidgetClass vim_scrollbarWidgetClass;
extern void vim_XawScrollbarSetThumb(Widget, double, double, double);
typedef struct
{
Pixel foreground; 
XtOrientation orientation; 
XtCallbackList scrollProc; 
XtCallbackList thumbProc; 
XtCallbackList jumpProc; 
Pixmap thumb; 
float top; 
float shown; 
float max; 
Dimension length; 
Dimension thickness; 
Dimension min_thumb; 
XtIntervalId timer_id; 
char scroll_mode; 
float scroll_off; 
GC gc; 
Position topLoc; 
Dimension shownLength; 
Dimension shadow_width;
Pixel top_shadow_pixel;
Pixel bot_shadow_pixel;
Bool limit_thumb; 
int top_shadow_contrast;
int bot_shadow_contrast;
GC top_shadow_GC;
GC bot_shadow_GC;
} ScrollbarPart;
#define SMODE_NONE 0
#define SMODE_CONT 1
#define SMODE_PAGE_UP 2
#define SMODE_PAGE_DOWN 3
#define SMODE_LINE_UP 4
#define SMODE_LINE_DOWN 5
#define ONE_LINE_DATA 1
#define ONE_PAGE_DATA 10
#define END_PAGE_DATA 9999
typedef struct _ScrollbarRec {
CorePart core;
SimplePart simple;
ScrollbarPart scrollbar;
} ScrollbarRec;
typedef struct {int empty;} ScrollbarClassPart;
typedef struct _ScrollbarClassRec {
CoreClassPart core_class;
SimpleClassPart simple_class;
ScrollbarClassPart scrollbar_class;
} ScrollbarClassRec;
extern ScrollbarClassRec vim_scrollbarClassRec;
