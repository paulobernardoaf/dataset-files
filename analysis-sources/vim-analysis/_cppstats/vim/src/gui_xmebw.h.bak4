


















#if !defined(EnhancedB_H)
#define EnhancedB_H





#if !defined(XmNshift)
#define XmNshift "shift"
#endif
#if !defined(XmCShift)
#define XmCShift "Shift"
#endif

#if !defined(XmNlabelLocation)
#define XmNlabelLocation "labelLocation"
#endif
#if !defined(XmCLocation)
#define XmCLocation "Location"
#endif

#if !defined(XmNpixmapData)
#define XmNpixmapData "pixmapData"
#endif

#if !defined(XmNpixmapFile)
#define XmNpixmapFile "pixmapFile"
#endif




#if defined(HAVE_XM_JOINSIDET_H)
#include <Xm/JoinSideT.h>
#else
#define XmLEFT 1
#define XmRIGHT 2
#define XmTOP 3
#define XmBOTTOM 4
#endif

#define XmIsEnhancedButton(w) XtIsSubclass(w, xmEnhancedButtonWidgetClass)




extern Widget XgCreateEPushButtonWidget(Widget, char *, ArgList, Cardinal);

extern WidgetClass xmEnhancedButtonWidgetClass;
typedef struct _XmEnhancedButtonClassRec *XmEnhancedButtonWidgetClass;
typedef struct _XmEnhancedButtonRec *XmEnhancedButtonWidget;

#endif
