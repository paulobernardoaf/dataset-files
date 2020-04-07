#include <Devices.h> 
#include "vim.h"
#define USE_CARBONIZED
#define USE_AEVENT 
#undef USE_OFFSETED_WINDOW 
#if defined(FEAT_CW_EDITOR) && !defined(USE_AEVENT)
#define USE_AEVENT 
#endif
#define VIMSCRAPFLAVOR 'VIM!'
#define SCRAPTEXTFLAVOR kScrapFlavorTypeUnicode
static EventHandlerUPP mouseWheelHandlerUPP = NULL;
SInt32 gMacSystemVersion;
#if defined(MACOS_CONVERT)
#define USE_CARBONKEYHANDLER
static int im_is_active = FALSE;
#if 0
static int im_start_row = 0;
static int im_start_col = 0;
#endif
#define NR_ELEMS(x) (sizeof(x) / sizeof(x[0]))
static TSMDocumentID gTSMDocument;
static void im_on_window_switch(int active);
static EventHandlerUPP keyEventHandlerUPP = NULL;
static EventHandlerUPP winEventHandlerUPP = NULL;
static pascal OSStatus gui_mac_handle_window_activate(
EventHandlerCallRef nextHandler, EventRef theEvent, void *data);
static pascal OSStatus gui_mac_handle_text_input(
EventHandlerCallRef nextHandler, EventRef theEvent, void *data);
static pascal OSStatus gui_mac_update_input_area(
EventHandlerCallRef nextHandler, EventRef theEvent);
static pascal OSStatus gui_mac_unicode_key_event(
EventHandlerCallRef nextHandler, EventRef theEvent);
#endif
#include <Menus.h>
#include <Resources.h>
#include <Processes.h>
#if defined(USE_AEVENT)
#include <AppleEvents.h>
#include <AERegistry.h>
#endif
#include <Gestalt.h>
#if UNIVERSAL_INTERFACES_VERSION >= 0x0330
#include <ControlDefinitions.h>
#include <Navigation.h> 
#endif
#if 0
#include <MacHelp.h>
#endif
#define kNothing 0
#define kCreateEmpty 2 
#define kCreateRect 2
#define kDestroy 3
#define topLeft(r) (((Point*)&(r))[0])
#define botRight(r) (((Point*)&(r))[1])
static long lastMouseTick = 0;
static RgnHandle cursorRgn;
static RgnHandle dragRgn;
static Rect dragRect;
static short dragRectEnbl;
static short dragRectControl;
static int allow_scrollbar = FALSE;
static short clickIsPopup;
ControlActionUPP gScrollAction;
ControlActionUPP gScrollDrag;
static ControlHandle dragged_sb = NULL;
static short *gDialogHotKeys;
static struct
{
FMFontFamily family;
FMFontSize size;
FMFontStyle style;
Boolean isPanelVisible;
} gFontPanelInfo = { 0, 0, 0, false };
#if defined(MACOS_CONVERT)
#define USE_ATSUI_DRAWING
int p_macatsui_last;
ATSUStyle gFontStyle;
ATSUStyle gWideFontStyle;
Boolean gIsFontFallbackSet;
UInt32 useAntialias_cached = 0x0;
#endif
#define RGB(r,g,b) ((r) << 16) + ((g) << 8) + (b)
#define Red(c) ((c & 0x00FF0000) >> 16)
#define Green(c) ((c & 0x0000FF00) >> 8)
#define Blue(c) ((c & 0x000000FF) >> 0)
#define vk_Esc 0x35 
#define vk_F1 0x7A 
#define vk_F2 0x78 
#define vk_F3 0x63 
#define vk_F4 0x76 
#define vk_F5 0x60 
#define vk_F6 0x61 
#define vk_F7 0x62 
#define vk_F8 0x64
#define vk_F9 0x65
#define vk_F10 0x6D
#define vk_F11 0x67
#define vk_F12 0x6F
#define vk_F13 0x69
#define vk_F14 0x6B
#define vk_F15 0x71
#define vk_Clr 0x47 
#define vk_Enter 0x4C 
#define vk_Space 0x31 
#define vk_Tab 0x30 
#define vk_Return 0x24 
#define vk_Delete 0X08 
#define vk_Help 0x72 
#define vk_Home 0x73 
#define vk_PageUp 0x74 
#define vk_FwdDelete 0x75 
#define vk_End 0x77 
#define vk_PageDown 0x79 
#define vk_Up 0x7E 
#define vk_Down 0x7D 
#define vk_Left 0x7B 
#define vk_Right 0x7C 
#define vk_Undo vk_F1
#define vk_Cut vk_F2
#define vk_Copy vk_F3
#define vk_Paste vk_F4
#define vk_PrintScreen vk_F13
#define vk_SCrollLock vk_F14
#define vk_Pause vk_F15
#define vk_NumLock vk_Clr
#define vk_Insert vk_Help
#define KeySym char
static struct
{
KeySym key_sym;
char_u vim_code0;
char_u vim_code1;
} special_keys[] =
{
{vk_Up, 'k', 'u'},
{vk_Down, 'k', 'd'},
{vk_Left, 'k', 'l'},
{vk_Right, 'k', 'r'},
{vk_F1, 'k', '1'},
{vk_F2, 'k', '2'},
{vk_F3, 'k', '3'},
{vk_F4, 'k', '4'},
{vk_F5, 'k', '5'},
{vk_F6, 'k', '6'},
{vk_F7, 'k', '7'},
{vk_F8, 'k', '8'},
{vk_F9, 'k', '9'},
{vk_F10, 'k', ';'},
{vk_F11, 'F', '1'},
{vk_F12, 'F', '2'},
{vk_F13, 'F', '3'},
{vk_F14, 'F', '4'},
{vk_F15, 'F', '5'},
{vk_Insert, 'k', 'I'},
{vk_FwdDelete, 'k', 'D'},
{vk_Home, 'k', 'h'},
{vk_End, '@', '7'},
{vk_PageUp, 'k', 'P'},
{vk_PageDown, 'k', 'N'},
{(KeySym)0, 0, 0}
};
#if defined(USE_AEVENT)
OSErr HandleUnusedParms(const AppleEvent *theAEvent);
#endif
#if defined(FEAT_GUI_TABLINE)
static void initialise_tabline(void);
static WindowRef drawer = NULL; 
#endif
#if defined(USE_ATSUI_DRAWING)
static void gui_mac_set_font_attributes(GuiFont font);
#endif
char_u *
C2Pascal_save(char_u *Cstring)
{
char_u *PascalString;
int len;
if (Cstring == NULL)
return NULL;
len = STRLEN(Cstring);
if (len > 255) 
len = 255;
PascalString = alloc(len + 1);
if (PascalString != NULL)
{
mch_memmove(PascalString + 1, Cstring, len);
PascalString[0] = len;
}
return PascalString;
}
char_u *
C2Pascal_save_and_remove_backslash(char_u *Cstring)
{
char_u *PascalString;
int len;
char_u *p, *c;
len = STRLEN(Cstring);
if (len > 255) 
len = 255;
PascalString = alloc(len + 1);
if (PascalString != NULL)
{
for (c = Cstring, p = PascalString+1, len = 0; (*c != 0) && (len < 255); c++)
{
if ((*c == '\\') && (c[1] != 0))
c++;
*p = *c;
p++;
len++;
}
PascalString[0] = len;
}
return PascalString;
}
int_u
EventModifiers2VimMouseModifiers(EventModifiers macModifiers)
{
int_u vimModifiers = 0x00;
if (macModifiers & (shiftKey | rightShiftKey))
vimModifiers |= MOUSE_SHIFT;
if (macModifiers & (controlKey | rightControlKey))
vimModifiers |= MOUSE_CTRL;
if (macModifiers & (optionKey | rightOptionKey))
vimModifiers |= MOUSE_ALT;
#if 0
if (macModifiers & (cmdKey)) 
vimModifiers |= MOUSE_CMD;
#endif
return (vimModifiers);
}
static int_u
EventModifiers2VimModifiers(EventModifiers macModifiers)
{
int_u vimModifiers = 0x00;
if (macModifiers & (shiftKey | rightShiftKey))
vimModifiers |= MOD_MASK_SHIFT;
if (macModifiers & (controlKey | rightControlKey))
vimModifiers |= MOD_MASK_CTRL;
if (macModifiers & (optionKey | rightOptionKey))
vimModifiers |= MOD_MASK_ALT;
#if defined(USE_CMD_KEY)
if (macModifiers & (cmdKey)) 
vimModifiers |= MOD_MASK_CMD;
#endif
return (vimModifiers);
}
static int
points_to_pixels(char_u *str, char_u **end, int vertical)
{
int pixels;
int points = 0;
int divisor = 0;
while (*str)
{
if (*str == '.' && divisor == 0)
{
divisor = 1;
continue;
}
if (!isdigit(*str))
break;
points *= 10;
points += *str - '0';
divisor *= 10;
++str;
}
if (divisor == 0)
divisor = 1;
pixels = points/divisor;
*end = str;
return pixels;
}
#if defined(MACOS_CONVERT)
static CFStringRef
menu_title_removing_mnemonic(vimmenu_T *menu)
{
CFStringRef name;
size_t menuTitleLen;
CFIndex displayLen;
CFRange mnemonicStart;
CFRange mnemonicEnd;
CFMutableStringRef cleanedName;
menuTitleLen = STRLEN(menu->dname);
name = (CFStringRef) mac_enc_to_cfstring(menu->dname, menuTitleLen);
if (name)
{
mnemonicStart = CFStringFind(name, CFSTR("("), kCFCompareBackwards);
displayLen = CFStringGetLength(name);
if (mnemonicStart.location != kCFNotFound
&& (mnemonicStart.location + 2) < displayLen
&& CFStringGetCharacterAtIndex(name,
mnemonicStart.location + 1) == (UniChar)menu->mnemonic)
{
if (CFStringFindWithOptions(name, CFSTR(")"),
CFRangeMake(mnemonicStart.location + 1,
displayLen - mnemonicStart.location - 1),
kCFCompareBackwards, &mnemonicEnd) &&
(mnemonicStart.location + 2) == mnemonicEnd.location)
{
cleanedName = CFStringCreateMutableCopy(NULL, 0, name);
if (cleanedName)
{
CFStringDelete(cleanedName,
CFRangeMake(mnemonicStart.location,
mnemonicEnd.location + 1 -
mnemonicStart.location));
CFRelease(name);
name = cleanedName;
}
}
}
}
return name;
}
#endif
char_u **
new_fnames_from_AEDesc(AEDesc *theList, long *numFiles, OSErr *error)
{
char_u **fnames = NULL;
OSErr newError;
long fileCount;
FSSpec fileToOpen;
long actualSize;
AEKeyword dummyKeyword;
DescType dummyType;
*error = AECountItems(theList, numFiles);
if (*error)
return fnames;
fnames = ALLOC_MULT(char_u *, *numFiles);
for (fileCount = 0; fileCount < *numFiles; fileCount++)
fnames[fileCount] = NULL;
for (fileCount = 1; fileCount <= *numFiles; fileCount++)
{
newError = AEGetNthPtr(theList, fileCount, typeFSS,
&dummyKeyword, &dummyType,
(Ptr) &fileToOpen, sizeof(FSSpec), &actualSize);
if (newError)
{
return fnames;
}
fnames[fileCount - 1] = FullPathFromFSSpec_save(fileToOpen);
}
return (fnames);
}
#if defined(FEAT_CW_EDITOR)
typedef struct WindowSearch WindowSearch;
struct WindowSearch 
{
FSSpec theFile; 
long *theDate; 
};
pascal OSErr
Handle_KAHL_SRCH_AE(
const AppleEvent *theAEvent,
AppleEvent *theReply,
long refCon)
{
OSErr error = noErr;
buf_T *buf;
int foundFile = false;
DescType typeCode;
WindowSearch SearchData;
Size actualSize;
error = AEGetParamPtr(theAEvent, keyDirectObject, typeChar, &typeCode, (Ptr) &SearchData, sizeof(WindowSearch), &actualSize);
if (error)
return error;
error = HandleUnusedParms(theAEvent);
if (error)
return error;
FOR_ALL_BUFFERS(buf)
if (buf->b_ml.ml_mfp != NULL
&& SearchData.theFile.parID == buf->b_FSSpec.parID
&& SearchData.theFile.name[0] == buf->b_FSSpec.name[0]
&& STRNCMP(SearchData.theFile.name, buf->b_FSSpec.name, buf->b_FSSpec.name[0] + 1) == 0)
{
foundFile = true;
break;
}
if (foundFile == false)
*SearchData.theDate = fnfErr;
else
*SearchData.theDate = buf->b_mtime;
return error;
};
typedef struct ModificationInfo ModificationInfo;
struct ModificationInfo 
{
FSSpec theFile; 
long theDate; 
short saved; 
};
pascal OSErr
Handle_KAHL_MOD_AE(
const AppleEvent *theAEvent,
AppleEvent *theReply,
long refCon)
{
OSErr error = noErr;
AEDescList replyList;
long numFiles;
ModificationInfo theFile;
buf_T *buf;
theFile.saved = 0;
error = HandleUnusedParms(theAEvent);
if (error)
return error;
error = AECreateList(nil, 0, false, &replyList);
if (error)
return error;
#if 0
error = AECountItems(&replyList, &numFiles);
#endif
numFiles = 0;
FOR_ALL_BUFFERS(buf)
if (buf->b_ml.ml_mfp != NULL)
{
theFile.theFile = buf->b_FSSpec;
theFile.theDate = buf->b_mtime;
error = AEPutPtr(&replyList, numFiles, typeChar, (Ptr) &theFile, sizeof(theFile));
};
#if 0
error = AECountItems(&replyList, &numFiles);
#endif
error = AEPutParamDesc(theReply, keyDirectObject, &replyList);
if (replyList.dataHandle)
AEDisposeDesc(&replyList);
return error;
};
typedef struct CW_GetText CW_GetText;
struct CW_GetText 
{
FSSpec theFile; 
Handle theText; 
long *unused; 
long *theDate; 
};
pascal OSErr
Handle_KAHL_GTTX_AE(
const AppleEvent *theAEvent,
AppleEvent *theReply,
long refCon)
{
OSErr error = noErr;
buf_T *buf;
int foundFile = false;
DescType typeCode;
CW_GetText GetTextData;
Size actualSize;
char_u *line;
char_u *fullbuffer = NULL;
long linesize;
long lineStart;
long BufferSize;
long lineno;
error = AEGetParamPtr(theAEvent, keyDirectObject, typeChar, &typeCode, (Ptr) &GetTextData, sizeof(GetTextData), &actualSize);
if (error)
return error;
FOR_ALL_BUFFERS(buf)
if (buf->b_ml.ml_mfp != NULL)
if (GetTextData.theFile.parID == buf->b_FSSpec.parID)
{
foundFile = true;
break;
}
if (foundFile)
{
BufferSize = 0; 
for (lineno = 0; lineno <= buf->b_ml.ml_line_count; lineno++)
{
line = ml_get_buf(buf, (linenr_T) lineno, FALSE);
linesize = STRLEN(line) + 1;
lineStart = BufferSize;
BufferSize += linesize;
SetHandleSize(GetTextData.theText, BufferSize);
if (GetHandleSize(GetTextData.theText) != BufferSize)
{
break; 
}
else
{
HLock(GetTextData.theText);
fullbuffer = (char_u *) *GetTextData.theText;
STRCPY((char_u *)(fullbuffer + lineStart), line);
fullbuffer[BufferSize-1] = '\r';
HUnlock(GetTextData.theText);
}
}
if (fullbuffer != NULL)
{
HLock(GetTextData.theText);
fullbuffer[BufferSize-1] = 0;
HUnlock(GetTextData.theText);
}
if (foundFile == false)
*GetTextData.theDate = fnfErr;
else
*GetTextData.theDate = buf->b_mtime;
}
error = HandleUnusedParms(theAEvent);
return error;
}
pascal OSErr
FindProcessBySignature(
const OSType targetType,
const OSType targetCreator,
ProcessSerialNumberPtr psnPtr)
{
OSErr anErr = noErr;
Boolean lookingForProcess = true;
ProcessInfoRec infoRec;
infoRec.processInfoLength = sizeof(ProcessInfoRec);
infoRec.processName = nil;
infoRec.processAppSpec = nil;
psnPtr->lowLongOfPSN = kNoProcess;
psnPtr->highLongOfPSN = kNoProcess;
while (lookingForProcess)
{
anErr = GetNextProcess(psnPtr);
if (anErr != noErr)
lookingForProcess = false;
else
{
anErr = GetProcessInformation(psnPtr, &infoRec);
if ((anErr == noErr)
&& (infoRec.processType == targetType)
&& (infoRec.processSignature == targetCreator))
lookingForProcess = false;
}
}
return anErr;
}
void
Send_KAHL_MOD_AE(buf_T *buf)
{
OSErr anErr = noErr;
AEDesc targetAppDesc = { typeNull, nil };
ProcessSerialNumber psn = { kNoProcess, kNoProcess };
AppleEvent theReply = { typeNull, nil };
AESendMode sendMode;
AppleEvent theEvent = {typeNull, nil };
AEIdleUPP idleProcUPP = nil;
ModificationInfo ModData;
anErr = FindProcessBySignature('APPL', 'CWIE', &psn);
if (anErr == noErr)
{
anErr = AECreateDesc(typeProcessSerialNumber, &psn,
sizeof(ProcessSerialNumber), &targetAppDesc);
if (anErr == noErr)
{
anErr = AECreateAppleEvent( 'KAHL', 'MOD ', &targetAppDesc,
kAutoGenerateReturnID, kAnyTransactionID, &theEvent);
}
AEDisposeDesc(&targetAppDesc);
ModData.theFile = buf->b_FSSpec;
ModData.theDate = buf->b_mtime;
if (anErr == noErr)
anErr = AEPutParamPtr(&theEvent, keyDirectObject, typeChar, &ModData, sizeof(ModData));
if (idleProcUPP == nil)
sendMode = kAENoReply;
else
sendMode = kAEWaitReply;
if (anErr == noErr)
anErr = AESend(&theEvent, &theReply, sendMode, kAENormalPriority, kNoTimeOut, idleProcUPP, nil);
if (anErr == noErr && sendMode == kAEWaitReply)
{
}
(void) AEDisposeDesc(&theReply);
}
}
#endif 
#if defined(USE_AEVENT)
OSErr
HandleUnusedParms(const AppleEvent *theAEvent)
{
OSErr error;
long actualSize;
DescType dummyType;
AEKeyword missedKeyword;
error = AEGetAttributePtr(theAEvent, keyMissedKeywordAttr,
typeKeyword, &dummyType,
(Ptr)&missedKeyword, sizeof(missedKeyword),
&actualSize);
if (error == errAEDescNotFound)
{
error = noErr;
}
else
{
#if 0
error = errAEEventNotHandled;
#endif
}
return error;
}
typedef struct SelectionRange SelectionRange;
struct SelectionRange 
{
short unused1; 
short lineNum; 
long startRange; 
long endRange; 
long unused2; 
long theDate; 
};
static long drop_numFiles;
static short drop_gotPosition;
static SelectionRange drop_thePosition;
static void
drop_callback(void *cookie UNUSED)
{
if ((drop_numFiles == 1) & (drop_gotPosition))
{
if (drop_thePosition.lineNum >= 0)
{
lnum = drop_thePosition.lineNum + 1;
if (lnum < 1L)
lnum = 1L;
else if (lnum > curbuf->b_ml.ml_line_count)
lnum = curbuf->b_ml.ml_line_count;
curwin->w_cursor.lnum = lnum;
curwin->w_cursor.col = 0;
}
else
goto_byte(drop_thePosition.startRange + 1);
}
update_screen(NOT_VALID);
if (drop_gotPosition)
{
VIsual_active = TRUE;
VIsual_select = FALSE;
VIsual = curwin->w_cursor;
if (drop_thePosition.lineNum < 0)
{
VIsual_mode = 'v';
goto_byte(drop_thePosition.endRange);
}
else
{
VIsual_mode = 'V';
VIsual.col = 0;
}
}
}
pascal OSErr
HandleODocAE(const AppleEvent *theAEvent, AppleEvent *theReply, long refCon)
{
OSErr error = noErr;
AEDesc theList;
DescType typeCode;
long numFiles;
char_u **fnames;
Size actualSize;
SelectionRange thePosition;
short gotPosition = false;
long lnum;
error = AEGetParamDesc(theAEvent, keyDirectObject, typeAEList, &theList);
if (error)
return error;
error = AEGetParamPtr(theAEvent, keyAEPosition, typeChar, &typeCode, (Ptr) &thePosition, sizeof(SelectionRange), &actualSize);
if (error == noErr)
gotPosition = true;
if (error == errAEDescNotFound)
error = noErr;
if (error)
return error;
reset_VIsual();
fnames = new_fnames_from_AEDesc(&theList, &numFiles, &error);
if (error)
{
vim_free(fnames);
return (error);
}
if (starting > 0)
{
int i;
char_u *p;
int fnum = -1;
for (i = 0 ; i < numFiles; i++)
{
if (ga_grow(&global_alist.al_ga, 1) == FAIL
|| (p = vim_strsave(fnames[i])) == NULL)
mch_exit(2);
else
alist_add(&global_alist, p, 2);
if (fnum == -1)
fnum = GARGLIST[GARGCOUNT - 1].ae_fnum;
}
if (curbuf->b_fnum != fnum)
{
char_u cmd[30];
vim_snprintf((char *)cmd, 30, "silent %dbuffer", fnum);
do_cmdline_cmd(cmd);
}
if (GARGCOUNT > 0
&& vim_chdirfile(alist_name(&GARGLIST[0]), "drop") == OK)
shorten_fnames(TRUE);
goto finished;
}
drop_numFiles = numFiles;
drop_gotPosition = gotPosition;
drop_thePosition = thePosition;
handle_drop(numFiles, fnames, FALSE, drop_callback, NULL);
setcursor();
out_flush();
PostEvent(mouseUp, 0);
finished:
AEDisposeDesc(&theList); 
error = HandleUnusedParms(theAEvent);
return error;
}
pascal OSErr
Handle_aevt_oapp_AE(
const AppleEvent *theAEvent,
AppleEvent *theReply,
long refCon)
{
OSErr error = noErr;
error = HandleUnusedParms(theAEvent);
return error;
}
pascal OSErr
Handle_aevt_quit_AE(
const AppleEvent *theAEvent,
AppleEvent *theReply,
long refCon)
{
OSErr error = noErr;
error = HandleUnusedParms(theAEvent);
if (error)
return error;
do_cmdline_cmd((char_u *)"confirm qa");
return error;
}
pascal OSErr
Handle_aevt_pdoc_AE(
const AppleEvent *theAEvent,
AppleEvent *theReply,
long refCon)
{
OSErr error = noErr;
error = HandleUnusedParms(theAEvent);
return error;
}
pascal OSErr
Handle_unknown_AE(
const AppleEvent *theAEvent,
AppleEvent *theReply,
long refCon)
{
OSErr error = noErr;
error = HandleUnusedParms(theAEvent);
return error;
}
OSErr
InstallAEHandlers(void)
{
OSErr error;
error = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
NewAEEventHandlerUPP(Handle_aevt_oapp_AE), 0, false);
if (error)
return error;
error = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
NewAEEventHandlerUPP(Handle_aevt_quit_AE), 0, false);
if (error)
return error;
error = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
NewAEEventHandlerUPP(HandleODocAE), 0, false);
if (error)
return error;
error = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,
NewAEEventHandlerUPP(Handle_aevt_pdoc_AE), 0, false);
#if 0
error = AEInstallEventHandler(kAECoreSuite, kAEClone,
NewAEEventHandlerUPP(Handle_unknown_AE), nil, false);
error = AEInstallEventHandler(kAECoreSuite, kAEClose,
NewAEEventHandlerUPP(Handle_unknown_AE), nil, false);
error = AEInstallEventHandler(kAECoreSuite, kAECountElements,
NewAEEventHandlerUPP(Handle_unknown_AE), nil, false);
error = AEInstallEventHandler(kAECoreSuite, kAECreateElement,
NewAEEventHandlerUPP(Handle_unknown_AE), nil, false);
error = AEInstallEventHandler(kAECoreSuite, kAEDelete,
NewAEEventHandlerUPP(Handle_unknown_AE), nil, false);
error = AEInstallEventHandler(kAECoreSuite, kAEDoObjectsExist,
NewAEEventHandlerUPP(Handle_unknown_AE), nil, false);
error = AEInstallEventHandler(kAECoreSuite, kAEGetData,
NewAEEventHandlerUPP(Handle_unknown_AE), kAEGetData, false);
error = AEInstallEventHandler(kAECoreSuite, kAEGetDataSize,
NewAEEventHandlerUPP(Handle_unknown_AE), kAEGetDataSize, false);
error = AEInstallEventHandler(kAECoreSuite, kAEGetClassInfo,
NewAEEventHandlerUPP(Handle_unknown_AE), nil, false);
error = AEInstallEventHandler(kAECoreSuite, kAEGetEventInfo,
NewAEEventHandlerUPP(Handle_unknown_AE), nil, false);
error = AEInstallEventHandler(kAECoreSuite, kAEMove,
NewAEEventHandlerUPP(Handle_unknown_AE), nil, false);
error = AEInstallEventHandler(kAECoreSuite, kAESave,
NewAEEventHandlerUPP(Handle_unknown_AE), nil, false);
error = AEInstallEventHandler(kAECoreSuite, kAESetData,
NewAEEventHandlerUPP(Handle_unknown_AE), nil, false);
#endif
#if defined(FEAT_CW_EDITOR)
error = AEInstallEventHandler('KAHL', 'GTTX',
NewAEEventHandlerUPP(Handle_KAHL_GTTX_AE), 0, false);
if (error)
return error;
error = AEInstallEventHandler('KAHL', 'SRCH',
NewAEEventHandlerUPP(Handle_KAHL_SRCH_AE), 0, false);
if (error)
return error;
error = AEInstallEventHandler('KAHL', 'MOD ',
NewAEEventHandlerUPP(Handle_KAHL_MOD_AE), 0, false);
#endif
return error;
}
#endif 
static OSStatus
FontPanelHandler(
EventHandlerCallRef inHandlerCallRef,
EventRef inEvent,
void *inUserData)
{
if (GetEventKind(inEvent) == kEventFontPanelClosed)
{
gFontPanelInfo.isPanelVisible = false;
return noErr;
}
if (GetEventKind(inEvent) == kEventFontSelection)
{
OSStatus status;
FMFontFamily newFamily;
FMFontSize newSize;
FMFontStyle newStyle;
status = GetEventParameter(inEvent, kEventParamFMFontFamily,
typeFMFontFamily, NULL,
sizeof(FMFontFamily), NULL,
&newFamily);
if (status == noErr)
gFontPanelInfo.family = newFamily;
status = GetEventParameter(inEvent, kEventParamFMFontSize,
typeFMFontSize, NULL, sizeof(FMFontSize), NULL, &newSize);
if (status == noErr)
gFontPanelInfo.size = newSize;
status = GetEventParameter(inEvent, kEventParamFMFontStyle,
typeFMFontStyle, NULL, sizeof(FMFontStyle), NULL, &newStyle);
if (status == noErr)
gFontPanelInfo.style = newStyle;
}
return noErr;
}
static void
InstallFontPanelHandler(void)
{
EventTypeSpec eventTypes[2];
EventHandlerUPP handlerUPP;
eventTypes[0].eventClass = kEventClassFont;
eventTypes[0].eventKind = kEventFontSelection;
eventTypes[1].eventClass = kEventClassFont;
eventTypes[1].eventKind = kEventFontPanelClosed;
handlerUPP = NewEventHandlerUPP(FontPanelHandler);
InstallApplicationEventHandler(handlerUPP, 2, eventTypes,
NULL, NULL);
}
#define FONT_STYLE_BUFFER_SIZE 32
static void
GetFontPanelSelection(char_u *outName)
{
Str255 buf;
ByteCount fontNameLen = 0;
ATSUFontID fid;
char_u styleString[FONT_STYLE_BUFFER_SIZE];
if (!outName)
return;
if (FMGetFontFamilyName(gFontPanelInfo.family, buf) == noErr)
{
if (FMGetFontFromFontFamilyInstance(gFontPanelInfo.family,
gFontPanelInfo.style, &fid, NULL) != noErr)
return;
if (ATSUFindFontName(fid, kFontFullName, kFontMacintoshPlatform,
kFontNoScriptCode, kFontNoLanguageCode,
255, (char *)outName, &fontNameLen, NULL) != noErr)
return;
vim_snprintf((char *)styleString, FONT_STYLE_BUFFER_SIZE, ":h%d",
gFontPanelInfo.size
);
if ((fontNameLen + STRLEN(styleString)) < 255)
STRCPY(outName + fontNameLen, styleString);
}
else
{
*outName = NUL;
}
}
short 
gui_mac_get_menu_item_index(vimmenu_T *pMenu)
{
short index;
short itemIndex = -1;
vimmenu_T *pBrother;
if (pMenu->parent)
{
pBrother = pMenu->parent->children;
index = 1;
while ((pBrother) && (itemIndex == -1))
{
if (pBrother == pMenu)
itemIndex = index;
index++;
pBrother = pBrother->next;
}
}
return itemIndex;
}
static vimmenu_T *
gui_mac_get_vim_menu(short menuID, short itemIndex, vimmenu_T *pMenu)
{
short index;
vimmenu_T *pChildMenu;
vimmenu_T *pElder = pMenu->parent;
if ((pElder) && (pElder->submenu_id == menuID))
{
for (index = 1; (index != itemIndex) && (pMenu != NULL); index++)
pMenu = pMenu->next;
}
else
{
for (; pMenu != NULL; pMenu = pMenu->next)
{
if (pMenu->children != NULL)
{
pChildMenu = gui_mac_get_vim_menu
(menuID, itemIndex, pMenu->children);
if (pChildMenu)
{
pMenu = pChildMenu;
break;
}
}
}
}
return pMenu;
}
pascal
void
gui_mac_drag_thumb(ControlHandle theControl, short partCode)
{
scrollbar_T *sb;
int value, dragging;
ControlHandle theControlToUse;
int dont_scroll_save = dont_scroll;
theControlToUse = dragged_sb;
sb = gui_find_scrollbar((long) GetControlReference(theControlToUse));
if (sb == NULL)
return;
value = GetControl32BitValue(theControlToUse);
dragging = (partCode != 0);
dont_scroll = !allow_scrollbar;
gui_drag_scrollbar(sb, value, dragging);
dont_scroll = dont_scroll_save;
}
pascal
void
gui_mac_scroll_action(ControlHandle theControl, short partCode)
{
scrollbar_T *sb, *sb_info;
long data;
long value;
int page;
int dragging = FALSE;
int dont_scroll_save = dont_scroll;
sb = gui_find_scrollbar((long)GetControlReference(theControl));
if (sb == NULL)
return;
if (sb->wp != NULL) 
{
sb_info = &sb->wp->w_scrollbars[0];
if (sb_info->size > 5)
page = sb_info->size - 2; 
else
page = sb_info->size;
}
else 
{
sb_info = sb;
page = curwin->w_width - 5;
}
switch (partCode)
{
case kControlUpButtonPart: data = -1; break;
case kControlDownButtonPart: data = 1; break;
case kControlPageDownPart: data = page; break;
case kControlPageUpPart: data = -page; break;
default: data = 0; break;
}
value = sb_info->value + data;
dont_scroll = !allow_scrollbar;
gui_drag_scrollbar(sb, value, dragging);
dont_scroll = dont_scroll_save;
out_flush();
gui_mch_set_scrollbar_thumb(sb, value, sb_info->size, sb_info->max);
#if 0
if (sb_info->wp != NULL)
{
win_T *wp;
int sb_num;
sb_num = 0;
for (wp = firstwin; wp != sb->wp && wp != NULL; wp = W_NEXT(wp))
sb_num++;
if (wp != NULL)
{
current_scrollbar = sb_num;
scrollbar_value = value;
gui_do_scroll();
gui_mch_set_scrollbar_thumb(sb, value, sb_info->size, sb_info->max);
}
}
#endif
}
void
gui_mac_doInContentClick(EventRecord *theEvent, WindowPtr whichWindow)
{
Point thePoint;
int_u vimModifiers;
short thePortion;
ControlHandle theControl;
int vimMouseButton;
short dblClick;
thePoint = theEvent->where;
GlobalToLocal(&thePoint);
SelectWindow(whichWindow);
thePortion = FindControl(thePoint, whichWindow, &theControl);
if (theControl != NUL)
{
if (thePortion != kControlIndicatorPart)
{
dragged_sb = theControl;
TrackControl(theControl, thePoint, gScrollAction);
dragged_sb = NULL;
}
else
{
dragged_sb = theControl;
#if 1
TrackControl(theControl, thePoint, gScrollDrag);
#else
TrackControl(theControl, thePoint, NULL);
#endif
gui_mac_drag_thumb(theControl, 0); 
dragged_sb = NULL;
}
}
else
{
vimModifiers = EventModifiers2VimMouseModifiers(theEvent->modifiers);
vimMouseButton = MOUSE_LEFT;
clickIsPopup = FALSE;
if (mouse_model_popup() && IsShowContextualMenuClick(theEvent))
{
vimMouseButton = MOUSE_RIGHT;
vimModifiers &= ~MOUSE_CTRL;
clickIsPopup = TRUE;
}
dblClick = ((theEvent->when - lastMouseTick) < GetDblTime());
gui_send_mouse_event(vimMouseButton, thePoint.h,
thePoint.v, dblClick, vimModifiers);
#if 0
if (vimMouseButton == MOUSE_LEFT)
#endif
{
SetRect(&dragRect, FILL_X(X_2_COL(thePoint.h)),
FILL_Y(Y_2_ROW(thePoint.v)),
FILL_X(X_2_COL(thePoint.h)+1),
FILL_Y(Y_2_ROW(thePoint.v)+1));
dragRectEnbl = TRUE;
dragRectControl = kCreateRect;
}
}
}
void
gui_mac_doInDragClick(Point where, WindowPtr whichWindow)
{
Rect movingLimits;
Rect *movingLimitsPtr = &movingLimits;
movingLimitsPtr = GetRegionBounds(GetGrayRgn(), &movingLimits);
DragWindow(whichWindow, where, movingLimitsPtr);
}
void
gui_mac_doInGrowClick(Point where, WindowPtr whichWindow)
{
long newSize;
unsigned short newWidth;
unsigned short newHeight;
Rect resizeLimits;
Rect *resizeLimitsPtr = &resizeLimits;
Rect NewContentRect;
resizeLimitsPtr = GetRegionBounds(GetGrayRgn(), &resizeLimits);
resizeLimits.top = 100;
resizeLimits.left = 100;
newSize = ResizeWindow(whichWindow, where, &resizeLimits, &NewContentRect);
newWidth = NewContentRect.right - NewContentRect.left;
newHeight = NewContentRect.bottom - NewContentRect.top;
gui_resize_shell(newWidth, newHeight);
gui_mch_set_bg_color(gui.back_pixel);
gui_set_shellsize(TRUE, FALSE, RESIZE_BOTH);
}
static void
gui_mac_doInZoomClick(EventRecord *theEvent, WindowPtr whichWindow)
{
Rect r;
Point p;
short thePart;
p.h = Columns * gui.char_width + 2 * gui.border_offset;
if (gui.which_scrollbars[SBAR_LEFT])
p.h += gui.scrollbar_width;
if (gui.which_scrollbars[SBAR_RIGHT])
p.h += gui.scrollbar_width;
p.v = 15 * 1024;
thePart = IsWindowInStandardState(whichWindow, &p, &r)
? inZoomIn : inZoomOut;
if (!TrackBox(whichWindow, theEvent->where, thePart))
return;
p.h = r.right - r.left;
p.v = r.bottom - r.top - 2 * gui.border_offset;
if (gui.which_scrollbars[SBAR_BOTTOM])
p.v -= gui.scrollbar_height;
p.v -= p.v % gui.char_height;
p.v += 2 * gui.border_width;
if (gui.which_scrollbars[SBAR_BOTTOM])
p.v += gui.scrollbar_height;
ZoomWindowIdeal(whichWindow, thePart, &p);
GetWindowBounds(whichWindow, kWindowContentRgn, &r);
gui_resize_shell(r.right - r.left, r.bottom - r.top);
gui_mch_set_bg_color(gui.back_pixel);
gui_set_shellsize(TRUE, FALSE, RESIZE_BOTH);
}
void
gui_mac_doUpdateEvent(EventRecord *event)
{
WindowPtr whichWindow;
GrafPtr savePort;
RgnHandle updateRgn;
Rect updateRect;
Rect *updateRectPtr;
Rect rc;
Rect growRect;
RgnHandle saveRgn;
updateRgn = NewRgn();
if (updateRgn == NULL)
return;
whichWindow = (WindowPtr) event->message;
GetPort(&savePort);
SetPortWindowPort(whichWindow);
BeginUpdate(whichWindow);
GetPortVisibleRegion(GetWindowPort(whichWindow), updateRgn);
#if 0
GetWindowRegion(whichWindow, kWindowUpdateRgn, updateRgn);
#endif
HLock((Handle) updateRgn);
updateRectPtr = GetRegionBounds(updateRgn, &updateRect);
#if 0
GlobalToLocal(&topLeft(updateRect)); 
GlobalToLocal(&botRight(updateRect));
#endif
gui_redraw(updateRectPtr->left, updateRectPtr->top,
updateRectPtr->right - updateRectPtr->left,
updateRectPtr->bottom - updateRectPtr->top);
gui_mch_set_bg_color(gui.back_pixel);
if (updateRectPtr->left < FILL_X(0))
{
SetRect(&rc, 0, 0, FILL_X(0), FILL_Y(Rows));
EraseRect(&rc);
}
if (updateRectPtr->top < FILL_Y(0))
{
SetRect(&rc, 0, 0, FILL_X(Columns), FILL_Y(0));
EraseRect(&rc);
}
if (updateRectPtr->right > FILL_X(Columns))
{
SetRect(&rc, FILL_X(Columns), 0,
FILL_X(Columns) + gui.border_offset, FILL_Y(Rows));
EraseRect(&rc);
}
if (updateRectPtr->bottom > FILL_Y(Rows))
{
SetRect(&rc, 0, FILL_Y(Rows), FILL_X(Columns) + gui.border_offset,
FILL_Y(Rows) + gui.border_offset);
EraseRect(&rc);
}
HUnlock((Handle) updateRgn);
DisposeRgn(updateRgn);
DrawControls(whichWindow);
saveRgn = NewRgn();
GetWindowBounds(whichWindow, kWindowGrowRgn, &growRect);
GetClip(saveRgn);
ClipRect(&growRect);
DrawGrowIcon(whichWindow);
SetClip(saveRgn);
DisposeRgn(saveRgn);
EndUpdate(whichWindow);
SetPort(savePort);
}
void
gui_mac_doActivateEvent(EventRecord *event)
{
WindowPtr whichWindow;
whichWindow = (WindowPtr) event->message;
if (whichWindow == gui.VimWindow)
{
ControlRef rootControl;
GetRootControl(gui.VimWindow, &rootControl);
if ((event->modifiers) & activeFlag)
ActivateControl(rootControl);
else
DeactivateControl(rootControl);
}
gui_focus_change((event->modifiers) & activeFlag);
}
void
gui_mac_doSuspendEvent(EventRecord *event)
{
if (event->message & 1)
gui_focus_change(TRUE);
else
gui_focus_change(FALSE);
}
#if defined(USE_CARBONKEYHANDLER)
static pascal OSStatus
gui_mac_handle_window_activate(
EventHandlerCallRef nextHandler,
EventRef theEvent,
void *data)
{
UInt32 eventClass = GetEventClass(theEvent);
UInt32 eventKind = GetEventKind(theEvent);
if (eventClass == kEventClassWindow)
{
switch (eventKind)
{
case kEventWindowActivated:
im_on_window_switch(TRUE);
return noErr;
case kEventWindowDeactivated:
im_on_window_switch(FALSE);
return noErr;
}
}
return eventNotHandledErr;
}
static pascal OSStatus
gui_mac_handle_text_input(
EventHandlerCallRef nextHandler,
EventRef theEvent,
void *data)
{
UInt32 eventClass = GetEventClass(theEvent);
UInt32 eventKind = GetEventKind(theEvent);
if (eventClass != kEventClassTextInput)
return eventNotHandledErr;
if ((kEventTextInputUpdateActiveInputArea != eventKind) &&
(kEventTextInputUnicodeForKeyEvent != eventKind) &&
(kEventTextInputOffsetToPos != eventKind) &&
(kEventTextInputPosToOffset != eventKind) &&
(kEventTextInputGetSelectedText != eventKind))
return eventNotHandledErr;
switch (eventKind)
{
case kEventTextInputUpdateActiveInputArea:
return gui_mac_update_input_area(nextHandler, theEvent);
case kEventTextInputUnicodeForKeyEvent:
return gui_mac_unicode_key_event(nextHandler, theEvent);
case kEventTextInputOffsetToPos:
case kEventTextInputPosToOffset:
case kEventTextInputGetSelectedText:
break;
}
return eventNotHandledErr;
}
static pascal
OSStatus gui_mac_update_input_area(
EventHandlerCallRef nextHandler,
EventRef theEvent)
{
return eventNotHandledErr;
}
static int dialog_busy = FALSE; 
#define INLINE_KEY_BUFFER_SIZE 80
static pascal OSStatus
gui_mac_unicode_key_event(
EventHandlerCallRef nextHandler,
EventRef theEvent)
{
OSStatus err = -1;
UInt32 actualSize;
UniChar *text;
char_u result[INLINE_KEY_BUFFER_SIZE];
short len = 0;
UInt32 key_sym;
char charcode;
int key_char;
UInt32 modifiers, vimModifiers;
size_t encLen;
char_u *to = NULL;
Boolean isSpecial = FALSE;
int i;
EventRef keyEvent;
if (p_mh)
ObscureCursor();
if (dialog_busy)
return eventNotHandledErr;
if (noErr != GetEventParameter(theEvent, kEventParamTextInputSendText,
typeUnicodeText, NULL, 0, &actualSize, NULL))
return eventNotHandledErr;
text = alloc(actualSize);
if (!text)
return eventNotHandledErr;
err = GetEventParameter(theEvent, kEventParamTextInputSendText,
typeUnicodeText, NULL, actualSize, NULL, text);
require_noerr(err, done);
err = GetEventParameter(theEvent, kEventParamTextInputSendKeyboardEvent,
typeEventRef, NULL, sizeof(EventRef), NULL, &keyEvent);
require_noerr(err, done);
err = GetEventParameter(keyEvent, kEventParamKeyModifiers,
typeUInt32, NULL, sizeof(UInt32), NULL, &modifiers);
require_noerr(err, done);
err = GetEventParameter(keyEvent, kEventParamKeyCode,
typeUInt32, NULL, sizeof(UInt32), NULL, &key_sym);
require_noerr(err, done);
err = GetEventParameter(keyEvent, kEventParamKeyMacCharCodes,
typeChar, NULL, sizeof(char), NULL, &charcode);
require_noerr(err, done);
#if !defined(USE_CMD_KEY)
if (modifiers & cmdKey)
goto done; 
#endif
key_char = charcode;
vimModifiers = EventModifiers2VimModifiers(modifiers);
if (actualSize <= sizeof(UniChar) &&
((text[0] < 0x20) || (text[0] == 0x7f)))
{
for (i = 0; special_keys[i].key_sym != (KeySym)0; ++i)
if (special_keys[i].key_sym == key_sym)
{
key_char = TO_SPECIAL(special_keys[i].vim_code0,
special_keys[i].vim_code1);
key_char = simplify_key(key_char,
(int *)&vimModifiers);
isSpecial = TRUE;
break;
}
}
if (((modifiers & controlKey) && key_char == 'c') ||
((modifiers & cmdKey) && key_char == '.'))
got_int = TRUE;
if (!isSpecial)
{
if (key_char < 0x100 && !isalpha(key_char) && isprint(key_char))
vimModifiers &= ~MOD_MASK_SHIFT;
if (key_char < 0x20)
vimModifiers &= ~MOD_MASK_CTRL;
if (!IS_SPECIAL(key_char))
{
key_char = simplify_key(key_char, (int *)&vimModifiers);
key_char = extract_modifiers(key_char, (int *)&vimModifiers,
TRUE, NULL);
if (key_char == CSI)
key_char = K_CSI;
if (IS_SPECIAL(key_char))
isSpecial = TRUE;
}
}
if (vimModifiers)
{
result[len++] = CSI;
result[len++] = KS_MODIFIER;
result[len++] = vimModifiers;
}
if (isSpecial && IS_SPECIAL(key_char))
{
result[len++] = CSI;
result[len++] = K_SECOND(key_char);
result[len++] = K_THIRD(key_char);
}
else
{
encLen = actualSize;
to = mac_utf16_to_enc(text, actualSize, &encLen);
if (to)
{
for (i = 0; i < encLen && len < (INLINE_KEY_BUFFER_SIZE-1); ++i)
{
result[len++] = to[i];
if (to[i] == CSI)
{
result[len++] = KS_EXTRA;
result[len++] = (int)KE_CSI;
}
}
vim_free(to);
}
}
add_to_input_buf(result, len);
err = noErr;
done:
vim_free(text);
if (err == noErr)
{
PostEvent(keyUp, 0);
return noErr;
}
return eventNotHandledErr;
}
#else
void
gui_mac_doKeyEvent(EventRecord *theEvent)
{
long menu;
unsigned char string[20];
short num, i;
short len = 0;
KeySym key_sym;
int key_char;
int modifiers;
int simplify = FALSE;
if (p_mh)
ObscureCursor();
key_sym = ((theEvent->message & keyCodeMask) >> 8);
key_char = theEvent->message & charCodeMask;
num = 1;
if (theEvent->modifiers & controlKey)
{
if (key_char == Ctrl_C && ctrl_c_interrupts)
got_int = TRUE;
else if ((theEvent->modifiers & ~(controlKey|shiftKey)) == 0
&& (key_char == '2' || key_char == '6'))
{
if (key_char == '2')
key_char = Ctrl_AT;
else
key_char = Ctrl_HAT;
theEvent->modifiers = 0;
}
}
if (theEvent->modifiers & cmdKey)
if (key_char == '.')
got_int = TRUE;
if (theEvent->modifiers & cmdKey)
if ((theEvent->modifiers & (~(cmdKey | btnState | alphaLock))) == 0)
{
menu = MenuKey(key_char);
if (HiWord(menu))
{
gui_mac_handle_menu(menu);
return;
}
}
modifiers = EventModifiers2VimModifiers(theEvent->modifiers);
#if 0
if (!(theEvent->modifiers & (cmdKey | controlKey | rightControlKey)))
#endif
{
if ((key_char < 0x20) || (key_char == 0x7f))
for (i = 0; special_keys[i].key_sym != (KeySym)0; i++)
if (special_keys[i].key_sym == key_sym)
{
#if 0
if (special_keys[i].vim_code1 == NUL)
key_char = special_keys[i].vim_code0;
else
#endif
key_char = TO_SPECIAL(special_keys[i].vim_code0,
special_keys[i].vim_code1);
simplify = TRUE;
break;
}
}
if (simplify || key_char == TAB || key_char == ' ')
key_char = simplify_key(key_char, &modifiers);
if (!IS_SPECIAL(key_char)
&& key_sym != vk_Space
&& key_sym != vk_Tab
&& key_sym != vk_Return
&& key_sym != vk_Enter
&& key_sym != vk_Esc)
{
#if 1
if ((modifiers == MOD_MASK_SHIFT)
|| (modifiers == MOD_MASK_CTRL)
|| (modifiers == MOD_MASK_ALT))
modifiers = 0;
#else
if (modifiers & MOD_MASK_CTRL)
modifiers = modifiers & ~MOD_MASK_CTRL;
if (modifiers & MOD_MASK_ALT)
modifiers = modifiers & ~MOD_MASK_ALT;
if (modifiers & MOD_MASK_SHIFT)
modifiers = modifiers & ~MOD_MASK_SHIFT;
#endif
}
if (modifiers)
{
string[len++] = CSI;
string[len++] = KS_MODIFIER;
string[len++] = modifiers;
}
if (IS_SPECIAL(key_char))
{
string[len++] = CSI;
string[len++] = K_SECOND(key_char);
string[len++] = K_THIRD(key_char);
}
else
{
if (input_conv.vc_type != CONV_NONE && key_char > 0)
{
char_u from[2], *to;
int l;
from[0] = key_char;
from[1] = NUL;
l = 1;
to = string_convert(&input_conv, from, &l);
if (to != NULL)
{
for (i = 0; i < l && len < 19; i++)
{
if (to[i] == CSI)
{
string[len++] = KS_EXTRA;
string[len++] = KE_CSI;
}
else
string[len++] = to[i];
}
vim_free(to);
}
else
string[len++] = key_char;
}
else
string[len++] = key_char;
}
if (len == 1 && string[0] == CSI)
{
string[ len++ ] = KS_EXTRA;
string[ len++ ] = KE_CSI;
}
add_to_input_buf(string, len);
}
#endif
void
gui_mac_doMouseDownEvent(EventRecord *theEvent)
{
short thePart;
WindowPtr whichWindow;
thePart = FindWindow(theEvent->where, &whichWindow);
#if defined(FEAT_GUI_TABLINE)
if (whichWindow == drawer)
return;
#endif
switch (thePart)
{
case (inDesk):
break;
case (inMenuBar):
gui_mac_handle_menu(MenuSelect(theEvent->where));
break;
case (inContent):
gui_mac_doInContentClick(theEvent, whichWindow);
break;
case (inDrag):
gui_mac_doInDragClick(theEvent->where, whichWindow);
break;
case (inGrow):
gui_mac_doInGrowClick(theEvent->where, whichWindow);
break;
case (inGoAway):
if (TrackGoAway(whichWindow, theEvent->where))
gui_shell_closed();
break;
case (inZoomIn):
case (inZoomOut):
gui_mac_doInZoomClick(theEvent, whichWindow);
break;
}
}
void
gui_mac_doMouseMovedEvent(EventRecord *event)
{
Point thePoint;
int_u vimModifiers;
thePoint = event->where;
GlobalToLocal(&thePoint);
vimModifiers = EventModifiers2VimMouseModifiers(event->modifiers);
if (!Button())
gui_mouse_moved(thePoint.h, thePoint.v);
else
if (!clickIsPopup)
gui_send_mouse_event(MOUSE_DRAG, thePoint.h,
thePoint.v, FALSE, vimModifiers);
SetRect(&dragRect, FILL_X(X_2_COL(thePoint.h)),
FILL_Y(Y_2_ROW(thePoint.v)),
FILL_X(X_2_COL(thePoint.h)+1),
FILL_Y(Y_2_ROW(thePoint.v)+1));
if (dragRectEnbl)
dragRectControl = kCreateRect;
}
void
gui_mac_doMouseUpEvent(EventRecord *theEvent)
{
Point thePoint;
int_u vimModifiers;
lastMouseTick = theEvent->when;
dragRectEnbl = FALSE;
dragRectControl = kCreateEmpty;
thePoint = theEvent->where;
GlobalToLocal(&thePoint);
vimModifiers = EventModifiers2VimMouseModifiers(theEvent->modifiers);
if (clickIsPopup)
{
vimModifiers &= ~MOUSE_CTRL;
clickIsPopup = FALSE;
}
gui_send_mouse_event(MOUSE_RELEASE, thePoint.h, thePoint.v, FALSE, vimModifiers);
}
static pascal OSStatus
gui_mac_mouse_wheel(EventHandlerCallRef nextHandler, EventRef theEvent,
void *data)
{
Point point;
Rect bounds;
UInt32 mod;
SInt32 delta;
int_u vim_mod;
EventMouseWheelAxis axis;
if (noErr == GetEventParameter(theEvent, kEventParamMouseWheelAxis,
typeMouseWheelAxis, NULL, sizeof(axis), NULL, &axis)
&& axis != kEventMouseWheelAxisY)
goto bail; 
if (noErr != GetEventParameter(theEvent, kEventParamMouseWheelDelta,
typeSInt32, NULL, sizeof(SInt32), NULL, &delta))
goto bail;
if (noErr != GetEventParameter(theEvent, kEventParamMouseLocation,
typeQDPoint, NULL, sizeof(Point), NULL, &point))
goto bail;
if (noErr != GetEventParameter(theEvent, kEventParamKeyModifiers,
typeUInt32, NULL, sizeof(UInt32), NULL, &mod))
goto bail;
vim_mod = 0;
if (mod & shiftKey)
vim_mod |= MOUSE_SHIFT;
if (mod & controlKey)
vim_mod |= MOUSE_CTRL;
if (mod & optionKey)
vim_mod |= MOUSE_ALT;
if (noErr == GetWindowBounds(gui.VimWindow, kWindowContentRgn, &bounds))
{
point.h -= bounds.left;
point.v -= bounds.top;
}
gui_send_mouse_event((delta > 0) ? MOUSE_4 : MOUSE_5,
point.h, point.v, FALSE, vim_mod);
PostEvent(keyUp, 0);
return noErr;
bail:
return CallNextEventHandler(nextHandler, theEvent);
}
void
gui_mch_mousehide(int hide)
{
}
#if 0
void
gui_mac_handle_contextual_menu(EventRecord *event)
{
OSStatus status = ContextualMenuSelect(CntxMenu, event->where, false, kCMHelpItemNoHelp, "", NULL, &CntxType, &CntxMenuID, &CntxMenuItem);
if (status != noErr)
return;
if (CntxType == kCMMenuItemSelected)
{
gui_mac_handle_menu((CntxMenuID << 16) + CntxMenuItem);
}
else if (CntxMenuID == kCMShowHelpSelected)
{
}
}
#endif
void
gui_mac_handle_menu(long menuChoice)
{
short menu = HiWord(menuChoice);
short item = LoWord(menuChoice);
vimmenu_T *theVimMenu = root_menu;
if (menu == 256) 
{
if (item == 1)
gui_mch_beep(); 
}
else if (item != 0)
{
theVimMenu = gui_mac_get_vim_menu(menu, item, root_menu);
if (theVimMenu)
gui_menu_cb(theVimMenu);
}
HiliteMenu(0);
}
void
gui_mac_handle_event(EventRecord *event)
{
OSErr error;
if (IsShowContextualMenuClick(event))
{
#if 0
gui_mac_handle_contextual_menu(event);
#else
gui_mac_doMouseDownEvent(event);
#endif
return;
}
switch (event->what)
{
#if !defined(USE_CARBONKEYHANDLER)
case (keyDown):
case (autoKey):
gui_mac_doKeyEvent(event);
break;
#endif
case (keyUp):
break;
case (mouseDown):
gui_mac_doMouseDownEvent(event);
break;
case (mouseUp):
gui_mac_doMouseUpEvent(event);
break;
case (updateEvt):
gui_mac_doUpdateEvent(event);
break;
case (diskEvt):
break;
case (activateEvt):
gui_mac_doActivateEvent(event);
break;
case (osEvt):
switch ((event->message >> 24) & 0xFF)
{
case (0xFA): 
gui_mac_doMouseMovedEvent(event);
break;
case (0x01): 
gui_mac_doSuspendEvent(event);
break;
}
break;
#if defined(USE_AEVENT)
case (kHighLevelEvent):
error = AEProcessAppleEvent(event); 
break;
#endif
}
}
GuiFont
gui_mac_find_font(char_u *font_name)
{
char_u c;
char_u *p;
char_u pFontName[256];
Str255 systemFontname;
short font_id;
short size=9;
GuiFont font;
#if 0
char_u *fontNamePtr;
#endif
for (p = font_name; ((*p != 0) && (*p != ':')); p++)
;
c = *p;
*p = 0;
#if 1
STRCPY(&pFontName[1], font_name);
pFontName[0] = STRLEN(font_name);
*p = c;
char_u fontName[256];
char_u *styleStart = vim_strchr(font_name, ':');
size_t fontNameLen = styleStart ? styleStart - font_name : STRLEN(fontName);
vim_strncpy(fontName, font_name, fontNameLen);
ATSUFontID fontRef;
FMFontStyle fontStyle;
font_id = 0;
if (ATSUFindFontFromName(&pFontName[1], pFontName[0], kFontFullName,
kFontMacintoshPlatform, kFontNoScriptCode, kFontNoLanguageCode,
&fontRef) == noErr)
{
if (FMGetFontFamilyInstanceFromFont(fontRef, &font_id, &fontStyle) != noErr)
font_id = 0;
}
if (font_id == 0)
{
int i, changed = FALSE;
for (i = pFontName[0]; i > 0; --i)
{
if (pFontName[i] == '_')
{
pFontName[i] = ' ';
changed = TRUE;
}
}
if (changed)
if (ATSUFindFontFromName(&pFontName[1], pFontName[0],
kFontFullName, kFontNoPlatformCode, kFontNoScriptCode,
kFontNoLanguageCode, &fontRef) == noErr)
{
if (FMGetFontFamilyInstanceFromFont(fontRef, &font_id, &fontStyle) != noErr)
font_id = 0;
}
}
#else
fontNamePtr = C2Pascal_save_and_remove_backslash(font_name);
GetFNum(fontNamePtr, &font_id);
#endif
if (font_id == 0)
{
if (FMGetFontFamilyName(systemFont, systemFontname) != noErr)
return NOFONT;
if (!EqualString(pFontName, systemFontname, false, false))
return NOFONT;
}
if (*p == ':')
{
p++;
while (*p)
{
switch (*p++)
{
case 'h':
size = points_to_pixels(p, &p, TRUE);
break;
}
while (*p == ':')
p++;
}
}
if (size < 1)
size = 1; 
font = (size << 16) + ((long) font_id & 0xFFFF);
return font;
}
void
gui_mch_prepare(int *argc, char **argv)
{
#if defined(USE_EXE_NAME)
FSSpec applDir;
#if !defined(USE_FIND_BUNDLE_PATH)
short applVRefNum;
long applDirID;
Str255 volName;
#else
ProcessSerialNumber psn;
FSRef applFSRef;
#endif
#endif
#if 0
InitCursor();
RegisterAppearanceClient();
#if defined(USE_AEVENT)
(void) InstallAEHandlers();
#endif
pomme = NewMenu(256, "\p\024"); 
AppendMenu(pomme, "\pAbout VIM");
InsertMenu(pomme, 0);
DrawMenuBar();
#if !defined(USE_OFFSETED_WINDOW)
SetRect(&windRect, 10, 48, 10+80*7 + 16, 48+24*11);
#else
SetRect(&windRect, 300, 40, 300+80*7 + 16, 40+24*11);
#endif
CreateNewWindow(kDocumentWindowClass,
kWindowResizableAttribute | kWindowCollapseBoxAttribute,
&windRect, &gui.VimWindow);
SetPortWindowPort(gui.VimWindow);
gui.char_width = 7;
gui.char_height = 11;
gui.char_ascent = 6;
gui.num_rows = 24;
gui.num_cols = 80;
gui.in_focus = TRUE; 
gScrollAction = NewControlActionUPP(gui_mac_scroll_action);
gScrollDrag = NewControlActionUPP(gui_mac_drag_thumb);
dragRectEnbl = FALSE;
dragRgn = NULL;
dragRectControl = kCreateEmpty;
cursorRgn = NewRgn();
#endif
#if defined(USE_EXE_NAME)
#if !defined(USE_FIND_BUNDLE_PATH)
HGetVol(volName, &applVRefNum, &applDirID);
FSMakeFSSpec(applVRefNum, applDirID, "\p", &applDir);
#else
(void)GetCurrentProcess(&psn);
(void)GetProcessBundleLocation(&psn, &applFSRef);
(void)FSGetCatalogInfo(&applFSRef, kFSCatInfoNone, NULL, NULL, &applDir, NULL);
#endif
exe_name = FullPathFromFSSpec_save(applDir);
#endif
}
#if !defined(ALWAYS_USE_GUI)
int
gui_mch_init_check(void)
{
if (disallow_gui) 
return FAIL;
return OK;
}
#endif
static OSErr
receiveHandler(WindowRef theWindow, void *handlerRefCon, DragRef theDrag)
{
int x, y;
int_u modifiers;
char_u **fnames = NULL;
int count;
int i, j;
{
Point point;
SInt16 mouseUpModifiers;
UInt16 countItem;
GetDragMouse(theDrag, &point, NULL);
GlobalToLocal(&point);
x = point.h;
y = point.v;
GetDragModifiers(theDrag, NULL, NULL, &mouseUpModifiers);
modifiers = EventModifiers2VimMouseModifiers(mouseUpModifiers);
CountDragItems(theDrag, &countItem);
count = countItem;
}
fnames = ALLOC_MULT(char_u *, count);
if (fnames == NULL)
return dragNotAcceptedErr;
for (i = j = 0; i < count; ++i)
{
DragItemRef item;
OSErr err;
Size size;
FlavorType type = flavorTypeHFS;
HFSFlavor hfsFlavor;
fnames[i] = NULL;
GetDragItemReferenceNumber(theDrag, i + 1, &item);
err = GetFlavorDataSize(theDrag, item, type, &size);
if (err != noErr || size > sizeof(hfsFlavor))
continue;
err = GetFlavorData(theDrag, item, type, &hfsFlavor, &size, 0);
if (err != noErr)
continue;
fnames[j++] = FullPathFromFSSpec_save(hfsFlavor.fileSpec);
}
count = j;
gui_handle_drop(x, y, modifiers, fnames, count);
PostEvent(mouseUp, 0);
return noErr;
}
int
gui_mch_init(void)
{
Rect windRect;
MenuHandle pomme;
EventHandlerRef mouseWheelHandlerRef;
EventTypeSpec eventTypeSpec;
ControlRef rootControl;
if (Gestalt(gestaltSystemVersion, &gMacSystemVersion) != noErr)
gMacSystemVersion = 0x1000; 
#if 1
InitCursor();
RegisterAppearanceClient();
#if defined(USE_AEVENT)
(void) InstallAEHandlers();
#endif
pomme = NewMenu(256, "\p\024"); 
AppendMenu(pomme, "\pAbout VIM");
InsertMenu(pomme, 0);
DrawMenuBar();
#if !defined(USE_OFFSETED_WINDOW)
SetRect(&windRect, 10, 48, 10+80*7 + 16, 48+24*11);
#else
SetRect(&windRect, 300, 40, 300+80*7 + 16, 40+24*11);
#endif
gui.VimWindow = NewCWindow(nil, &windRect, "\pgVim on Macintosh", true,
zoomDocProc,
(WindowPtr)-1L, true, 0);
CreateRootControl(gui.VimWindow, &rootControl);
InstallReceiveHandler((DragReceiveHandlerUPP)receiveHandler,
gui.VimWindow, NULL);
SetPortWindowPort(gui.VimWindow);
gui.char_width = 7;
gui.char_height = 11;
gui.char_ascent = 6;
gui.num_rows = 24;
gui.num_cols = 80;
gui.in_focus = TRUE; 
gScrollAction = NewControlActionUPP(gui_mac_scroll_action);
gScrollDrag = NewControlActionUPP(gui_mac_drag_thumb);
(void)InstallFontPanelHandler();
dragRectEnbl = FALSE;
dragRgn = NULL;
dragRectControl = kCreateEmpty;
cursorRgn = NewRgn();
#endif
display_errors();
gui.norm_pixel = 0x00000000;
gui.back_pixel = 0x00FFFFFF;
set_normal_colors();
gui_check_colors();
gui.def_norm_pixel = gui.norm_pixel;
gui.def_back_pixel = gui.back_pixel;
highlight_gui_started();
#if defined(FEAT_MENU)
gui.menu_height = 0;
#endif
gui.scrollbar_height = gui.scrollbar_width = 15; 
gui.border_offset = gui.border_width = 2;
vim_setenv((char_u *)"QDTEXT_MINSIZE", (char_u *)"1");
eventTypeSpec.eventClass = kEventClassMouse;
eventTypeSpec.eventKind = kEventMouseWheelMoved;
mouseWheelHandlerUPP = NewEventHandlerUPP(gui_mac_mouse_wheel);
if (noErr != InstallApplicationEventHandler(mouseWheelHandlerUPP, 1,
&eventTypeSpec, NULL, &mouseWheelHandlerRef))
{
mouseWheelHandlerRef = NULL;
DisposeEventHandlerUPP(mouseWheelHandlerUPP);
mouseWheelHandlerUPP = NULL;
}
#if defined(USE_CARBONKEYHANDLER)
InterfaceTypeList supportedServices = { kUnicodeDocument };
NewTSMDocument(1, supportedServices, &gTSMDocument, 0);
UseInputWindow(gTSMDocument, TRUE);
EventTypeSpec textEventTypes[] = {
{ kEventClassTextInput, kEventTextInputUpdateActiveInputArea },
{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
{ kEventClassTextInput, kEventTextInputPosToOffset },
{ kEventClassTextInput, kEventTextInputOffsetToPos },
};
keyEventHandlerUPP = NewEventHandlerUPP(gui_mac_handle_text_input);
if (noErr != InstallApplicationEventHandler(keyEventHandlerUPP,
NR_ELEMS(textEventTypes),
textEventTypes, NULL, NULL))
{
DisposeEventHandlerUPP(keyEventHandlerUPP);
keyEventHandlerUPP = NULL;
}
EventTypeSpec windowEventTypes[] = {
{ kEventClassWindow, kEventWindowActivated },
{ kEventClassWindow, kEventWindowDeactivated },
};
winEventHandlerUPP = NewEventHandlerUPP(gui_mac_handle_window_activate);
if (noErr != InstallWindowEventHandler(gui.VimWindow,
winEventHandlerUPP,
NR_ELEMS(windowEventTypes),
windowEventTypes, NULL, NULL))
{
DisposeEventHandlerUPP(winEventHandlerUPP);
winEventHandlerUPP = NULL;
}
#endif
#if defined(FEAT_GUI_TABLINE)
initialise_tabline();
#endif
return OK;
}
void
gui_mch_new_colors(void)
{
}
int
gui_mch_open(void)
{
ShowWindow(gui.VimWindow);
if (gui_win_x != -1 && gui_win_y != -1)
gui_mch_set_winpos(gui_win_x, gui_win_y);
{
ProcessSerialNumber psn;
if (GetCurrentProcess(&psn) == noErr)
SetFrontProcess(&psn);
}
return OK;
}
#if defined(USE_ATSUI_DRAWING)
static void
gui_mac_dispose_atsui_style(void)
{
if (p_macatsui && gFontStyle)
ATSUDisposeStyle(gFontStyle);
if (p_macatsui && gWideFontStyle)
ATSUDisposeStyle(gWideFontStyle);
}
#endif
void
gui_mch_exit(int rc)
{
DisposeRgn(cursorRgn);
#if defined(USE_CARBONKEYHANDLER)
if (keyEventHandlerUPP)
DisposeEventHandlerUPP(keyEventHandlerUPP);
#endif
if (mouseWheelHandlerUPP != NULL)
DisposeEventHandlerUPP(mouseWheelHandlerUPP);
#if defined(USE_ATSUI_DRAWING)
gui_mac_dispose_atsui_style();
#endif
#if defined(USE_CARBONKEYHANDLER)
FixTSMDocument(gTSMDocument);
DeactivateTSMDocument(gTSMDocument);
DeleteTSMDocument(gTSMDocument);
#endif
exit(rc);
}
int
gui_mch_get_winpos(int *x, int *y)
{
Rect bounds;
OSStatus status;
status = GetWindowBounds(gui.VimWindow, kWindowStructureRgn, &bounds);
if (status != noErr)
return FAIL;
*x = bounds.left;
*y = bounds.top;
return OK;
}
void
gui_mch_set_winpos(int x, int y)
{
MoveWindowStructure(gui.VimWindow, x, y);
}
void
gui_mch_set_shellsize(
int width,
int height,
int min_width,
int min_height,
int base_width,
int base_height,
int direction)
{
CGrafPtr VimPort;
Rect VimBound;
if (gui.which_scrollbars[SBAR_LEFT])
{
VimPort = GetWindowPort(gui.VimWindow);
GetPortBounds(VimPort, &VimBound);
VimBound.left = -gui.scrollbar_width; 
SetPortBounds(VimPort, &VimBound);
}
else
{
VimPort = GetWindowPort(gui.VimWindow);
GetPortBounds(VimPort, &VimBound);
VimBound.left = 0;
SetPortBounds(VimPort, &VimBound);
}
SizeWindow(gui.VimWindow, width, height, TRUE);
gui_resize_shell(width, height);
}
void
gui_mch_get_screen_dimensions(int *screen_w, int *screen_h)
{
GDHandle dominantDevice = GetMainDevice();
Rect screenRect = (**dominantDevice).gdRect;
*screen_w = screenRect.right - 10;
*screen_h = screenRect.bottom - 40;
}
static GuiFont
gui_mac_select_font(char_u *font_name)
{
GuiFont selected_font = NOFONT;
OSStatus status;
FontSelectionQDStyle curr_font;
curr_font.instance.fontFamily = gui.norm_font & 0xFFFF;
curr_font.size = (gui.norm_font >> 16);
curr_font.instance.fontStyle = 0;
curr_font.hasColor = false;
curr_font.version = 0; 
status = SetFontInfoForSelection(kFontSelectionQDType,
1, &curr_font, NULL);
gFontPanelInfo.family = curr_font.instance.fontFamily;
gFontPanelInfo.style = curr_font.instance.fontStyle;
gFontPanelInfo.size = curr_font.size;
status = FPShowHideFontPanel();
if (status == noErr)
{
gFontPanelInfo.isPanelVisible = true;
while (gFontPanelInfo.isPanelVisible)
{
EventRecord e;
WaitNextEvent(everyEvent, &e, 20, NULL);
}
GetFontPanelSelection(font_name);
selected_font = gui_mac_find_font(font_name);
}
return selected_font;
}
#if defined(USE_ATSUI_DRAWING)
static void
gui_mac_create_atsui_style(void)
{
if (p_macatsui && gFontStyle == NULL)
{
if (ATSUCreateStyle(&gFontStyle) != noErr)
gFontStyle = NULL;
}
if (p_macatsui && gWideFontStyle == NULL)
{
if (ATSUCreateStyle(&gWideFontStyle) != noErr)
gWideFontStyle = NULL;
}
p_macatsui_last = p_macatsui;
}
#endif
int
gui_mch_init_font(char_u *font_name, int fontset)
{
Str255 suggestedFont = "\pMonaco";
int suggestedSize = 10;
FontInfo font_info;
short font_id;
GuiFont font;
char_u used_font_name[512];
#if defined(USE_ATSUI_DRAWING)
gui_mac_create_atsui_style();
#endif
if (font_name == NULL)
{
GetFNum(suggestedFont, &font_id);
if (font_id == 0)
{
font_id = GetAppFont();
STRCPY(used_font_name, "default");
}
else
STRCPY(used_font_name, "Monaco");
font = (suggestedSize << 16) + ((long) font_id & 0xFFFF);
}
else if (STRCMP(font_name, "*") == 0)
{
char_u *new_p_guifont;
font = gui_mac_select_font(used_font_name);
if (font == NOFONT)
return FAIL;
new_p_guifont = alloc(STRLEN(used_font_name) + 1);
if (new_p_guifont != NULL)
{
STRCPY(new_p_guifont, used_font_name);
vim_free(p_guifont);
p_guifont = new_p_guifont;
for ( ; *new_p_guifont; ++new_p_guifont)
{
if (*new_p_guifont == ' ')
*new_p_guifont = '_';
}
}
}
else
{
font = gui_mac_find_font(font_name);
vim_strncpy(used_font_name, font_name, sizeof(used_font_name) - 1);
if (font == NOFONT)
return FAIL;
}
gui.norm_font = font;
hl_set_font_name(used_font_name);
TextSize(font >> 16);
TextFont(font & 0xFFFF);
GetFontInfo(&font_info);
gui.char_ascent = font_info.ascent;
gui.char_width = CharWidth('_');
gui.char_height = font_info.ascent + font_info.descent + p_linespace;
#if defined(USE_ATSUI_DRAWING)
if (p_macatsui && gFontStyle)
gui_mac_set_font_attributes(font);
#endif
return OK;
}
int
gui_mch_adjust_charheight(void)
{
FontInfo font_info;
GetFontInfo(&font_info);
gui.char_height = font_info.ascent + font_info.descent + p_linespace;
gui.char_ascent = font_info.ascent + p_linespace / 2;
return OK;
}
GuiFont
gui_mch_get_font(char_u *name, int giveErrorIfMissing)
{
GuiFont font;
font = gui_mac_find_font(name);
if (font == NOFONT)
{
if (giveErrorIfMissing)
semsg(_(e_font), name);
return NOFONT;
}
return font;
}
#if defined(FEAT_EVAL) || defined(PROTO)
char_u *
gui_mch_get_fontname(GuiFont font, char_u *name)
{
if (name == NULL)
return NULL;
return vim_strsave(name);
}
#endif
#if defined(USE_ATSUI_DRAWING)
static void
gui_mac_set_font_attributes(GuiFont font)
{
ATSUFontID fontID;
Fixed fontSize;
Fixed fontWidth;
fontID = font & 0xFFFF;
fontSize = Long2Fix(font >> 16);
fontWidth = Long2Fix(gui.char_width);
ATSUAttributeTag attribTags[] =
{
kATSUFontTag, kATSUSizeTag, kATSUImposeWidthTag,
kATSUMaxATSUITagValue + 1
};
ByteCount attribSizes[] =
{
sizeof(ATSUFontID), sizeof(Fixed), sizeof(fontWidth),
sizeof(font)
};
ATSUAttributeValuePtr attribValues[] =
{
&fontID, &fontSize, &fontWidth, &font
};
if (FMGetFontFromFontFamilyInstance(fontID, 0, &fontID, NULL) == noErr)
{
if (ATSUSetAttributes(gFontStyle,
(sizeof attribTags) / sizeof(ATSUAttributeTag),
attribTags, attribSizes, attribValues) != noErr)
{
#if !defined(NDEBUG)
fprintf(stderr, "couldn't set font style\n");
#endif
ATSUDisposeStyle(gFontStyle);
gFontStyle = NULL;
}
if (has_mbyte)
{
fontWidth = Long2Fix(gui.char_width * 2);
if (ATSUSetAttributes(gWideFontStyle,
(sizeof attribTags) / sizeof(ATSUAttributeTag),
attribTags, attribSizes, attribValues) != noErr)
{
ATSUDisposeStyle(gWideFontStyle);
gWideFontStyle = NULL;
}
}
}
}
#endif
void
gui_mch_set_font(GuiFont font)
{
#if defined(USE_ATSUI_DRAWING)
GuiFont currFont;
ByteCount actualFontByteCount;
if (p_macatsui && gFontStyle)
{
if (ATSUGetAttribute(gFontStyle, kATSUMaxATSUITagValue + 1,
sizeof(font), &currFont, &actualFontByteCount) == noErr
&& actualFontByteCount == (sizeof font))
{
if (currFont == font)
return;
}
gui_mac_set_font_attributes(font);
}
if (p_macatsui && !gIsFontFallbackSet)
{
#if 0
ATSUAttributeTag fallbackTags[] = { kATSULineFontFallbacksTag };
ByteCount fallbackSizes[] = { sizeof(ATSUFontFallbacks) };
ATSUCreateFontFallbacks(&gFontFallbacks);
ATSUSetObjFontFallbacks(gFontFallbacks, );
#endif
if (gui.wide_font)
{
ATSUFontID fallbackFonts;
gIsFontFallbackSet = TRUE;
if (FMGetFontFromFontFamilyInstance(
(gui.wide_font & 0xFFFF),
0,
&fallbackFonts,
NULL) == noErr)
{
ATSUSetFontFallbacks((sizeof fallbackFonts)/sizeof(ATSUFontID),
&fallbackFonts,
kATSUSequentialFallbacksPreferred);
}
}
}
#endif
TextSize(font >> 16);
TextFont(font & 0xFFFF);
}
void
gui_mch_free_font(GuiFont font)
{
}
guicolor_T
gui_mch_get_color(char_u *name)
{
RGBColor MacColor;
if (STRICMP(name, "hilite") == 0)
{
LMGetHiliteRGB(&MacColor);
return (RGB(MacColor.red >> 8, MacColor.green >> 8, MacColor.blue >> 8));
}
return gui_get_color_cmn(name);
}
guicolor_T
gui_mch_get_rgb_color(int r, int g, int b)
{
return gui_get_rgb_color_cmn(r, g, b);
}
void
gui_mch_set_fg_color(guicolor_T color)
{
RGBColor TheColor;
TheColor.red = Red(color) * 0x0101;
TheColor.green = Green(color) * 0x0101;
TheColor.blue = Blue(color) * 0x0101;
RGBForeColor(&TheColor);
}
void
gui_mch_set_bg_color(guicolor_T color)
{
RGBColor TheColor;
TheColor.red = Red(color) * 0x0101;
TheColor.green = Green(color) * 0x0101;
TheColor.blue = Blue(color) * 0x0101;
RGBBackColor(&TheColor);
}
RGBColor specialColor;
void
gui_mch_set_sp_color(guicolor_T color)
{
specialColor.red = Red(color) * 0x0101;
specialColor.green = Green(color) * 0x0101;
specialColor.blue = Blue(color) * 0x0101;
}
static void
draw_undercurl(int flags, int row, int col, int cells)
{
int x;
int offset;
const static int val[8] = {1, 0, 0, 0, 1, 2, 2, 2 };
int y = FILL_Y(row + 1) - 1;
RGBForeColor(&specialColor);
offset = val[FILL_X(col) % 8];
MoveTo(FILL_X(col), y - offset);
for (x = FILL_X(col); x < FILL_X(col + cells); ++x)
{
offset = val[x % 8];
LineTo(x, y - offset);
}
}
static void
draw_string_QD(int row, int col, char_u *s, int len, int flags)
{
char_u *tofree = NULL;
if (output_conv.vc_type != CONV_NONE)
{
tofree = string_convert(&output_conv, s, &len);
if (tofree != NULL)
s = tofree;
}
if (gMacSystemVersion >= 0x1020)
{
UInt32 qd_flags = (p_antialias ?
kQDUseCGTextRendering | kQDUseCGTextMetrics : 0);
QDSwapTextFlags(qd_flags);
}
if (((gMacSystemVersion >= 0x1020 && p_antialias) || p_linespace != 0)
&& !(flags & DRAW_TRANSP))
{
Rect rc;
rc.left = FILL_X(col);
rc.top = FILL_Y(row);
if (has_mbyte)
{
rc.right = FILL_X(col + mb_string2cells(s, len));
}
else
rc.right = FILL_X(col + len) + (col + len == Columns);
rc.bottom = FILL_Y(row + 1);
EraseRect(&rc);
}
if (gMacSystemVersion >= 0x1020 && p_antialias)
{
StyleParameter face;
face = normal;
if (flags & DRAW_BOLD)
face |= bold;
if (flags & DRAW_UNDERL)
face |= underline;
TextFace(face);
TextMode(srcOr);
MoveTo(TEXT_X(col), TEXT_Y(row));
DrawText((char*)s, 0, len);
}
else
{
TextMode(srcCopy);
TextFace(normal);
if (flags & DRAW_TRANSP)
TextMode(srcOr);
MoveTo(TEXT_X(col), TEXT_Y(row));
DrawText((char *)s, 0, len);
if (flags & DRAW_BOLD)
{
TextMode(srcOr);
MoveTo(TEXT_X(col) + 1, TEXT_Y(row));
DrawText((char *)s, 0, len);
}
if (flags & DRAW_UNDERL)
{
MoveTo(FILL_X(col), FILL_Y(row + 1) - 1);
LineTo(FILL_X(col + len) - 1, FILL_Y(row + 1) - 1);
}
if (flags & DRAW_STRIKE)
{
MoveTo(FILL_X(col), FILL_Y(row + 1) - gui.char_height/2);
LineTo(FILL_X(col + len) - 1, FILL_Y(row + 1) - gui.char_height/2);
}
}
if (flags & DRAW_UNDERC)
draw_undercurl(flags, row, col, len);
vim_free(tofree);
}
#if defined(USE_ATSUI_DRAWING)
static void
draw_string_ATSUI(int row, int col, char_u *s, int len, int flags)
{
UniCharCount utf16_len;
UniChar *tofree = mac_enc_to_utf16(s, len, (size_t *)&utf16_len);
utf16_len /= sizeof(UniChar);
#if defined(MAC_ATSUI_DEBUG)
fprintf(stderr, "row = %d, col = %d, len = %d: '%c'\n",
row, col, len, len == 1 ? s[0] : ' ');
#endif
if ((flags & DRAW_TRANSP) == 0)
{
Rect rc;
rc.left = FILL_X(col);
rc.top = FILL_Y(row);
if (has_mbyte)
{
rc.right = FILL_X(col + mb_string2cells(s, len));
}
else
rc.right = FILL_X(col + len) + (col + len == Columns);
rc.bottom = FILL_Y(row + 1);
EraseRect(&rc);
}
{
TextMode(srcCopy);
TextFace(normal);
if (flags & DRAW_TRANSP)
TextMode(srcOr);
MoveTo(TEXT_X(col), TEXT_Y(row));
if (gFontStyle && flags & DRAW_BOLD)
{
Boolean attValue = true;
ATSUAttributeTag attribTags[] = { kATSUQDBoldfaceTag };
ByteCount attribSizes[] = { sizeof(Boolean) };
ATSUAttributeValuePtr attribValues[] = { &attValue };
ATSUSetAttributes(gFontStyle, 1, attribTags, attribSizes, attribValues);
}
UInt32 useAntialias = p_antialias ? kATSStyleApplyAntiAliasing
: kATSStyleNoAntiAliasing;
if (useAntialias != useAntialias_cached)
{
ATSUAttributeTag attribTags[] = { kATSUStyleRenderingOptionsTag };
ByteCount attribSizes[] = { sizeof(UInt32) };
ATSUAttributeValuePtr attribValues[] = { &useAntialias };
if (gFontStyle)
ATSUSetAttributes(gFontStyle, 1, attribTags,
attribSizes, attribValues);
if (gWideFontStyle)
ATSUSetAttributes(gWideFontStyle, 1, attribTags,
attribSizes, attribValues);
useAntialias_cached = useAntialias;
}
if (has_mbyte)
{
int n, width_in_cell, last_width_in_cell;
UniCharArrayOffset offset = 0;
UniCharCount yet_to_draw = 0;
ATSUTextLayout textLayout;
ATSUStyle textStyle;
last_width_in_cell = 1;
ATSUCreateTextLayout(&textLayout);
ATSUSetTextPointerLocation(textLayout, tofree,
kATSUFromTextBeginning,
kATSUToTextEnd, utf16_len);
for (n = 0; n < len; n += MB_BYTE2LEN(s[n]))
{
width_in_cell = (*mb_ptr2cells)(s + n);
if (width_in_cell != last_width_in_cell)
{
#if defined(MAC_ATSUI_DEBUG)
fprintf(stderr, "\tn = %2d, (%d-%d), offset = %d, yet_to_draw = %d\n",
n, last_width_in_cell, width_in_cell, offset, yet_to_draw);
#endif
textStyle = last_width_in_cell > 1 ? gWideFontStyle
: gFontStyle;
ATSUSetRunStyle(textLayout, textStyle, offset, yet_to_draw);
offset += yet_to_draw;
yet_to_draw = 0;
last_width_in_cell = width_in_cell;
}
yet_to_draw++;
}
if (yet_to_draw)
{
#if defined(MAC_ATSUI_DEBUG)
fprintf(stderr, "\tn = %2d, (%d-%d), offset = %d, yet_to_draw = %d\n",
n, last_width_in_cell, width_in_cell, offset, yet_to_draw);
#endif
textStyle = width_in_cell > 1 ? gWideFontStyle : gFontStyle;
ATSUSetRunStyle(textLayout, textStyle, offset, kATSUToTextEnd);
}
ATSUSetTransientFontMatching(textLayout, TRUE);
ATSUDrawText(textLayout,
kATSUFromTextBeginning, kATSUToTextEnd,
kATSUUseGrafPortPenLoc, kATSUUseGrafPortPenLoc);
ATSUDisposeTextLayout(textLayout);
}
else
{
ATSUTextLayout textLayout;
if (ATSUCreateTextLayoutWithTextPtr(tofree,
kATSUFromTextBeginning, kATSUToTextEnd,
utf16_len,
(gFontStyle ? 1 : 0), &utf16_len,
(gFontStyle ? &gFontStyle : NULL),
&textLayout) == noErr)
{
ATSUSetTransientFontMatching(textLayout, TRUE);
ATSUDrawText(textLayout,
kATSUFromTextBeginning, kATSUToTextEnd,
kATSUUseGrafPortPenLoc, kATSUUseGrafPortPenLoc);
ATSUDisposeTextLayout(textLayout);
}
}
if (gFontStyle && flags & DRAW_BOLD)
{
Boolean attValue = false;
ATSUAttributeTag attribTags[] = { kATSUQDBoldfaceTag };
ByteCount attribSizes[] = { sizeof(Boolean) };
ATSUAttributeValuePtr attribValues[] = { &attValue };
ATSUSetAttributes(gFontStyle, 1, attribTags, attribSizes,
attribValues);
}
}
if (flags & DRAW_UNDERC)
draw_undercurl(flags, row, col, len);
vim_free(tofree);
}
#endif
void
gui_mch_draw_string(int row, int col, char_u *s, int len, int flags)
{
#if defined(USE_ATSUI_DRAWING)
if (p_macatsui == 0 && p_macatsui_last != 0)
gui_mac_dispose_atsui_style();
else if (p_macatsui != 0 && p_macatsui_last == 0)
gui_mac_create_atsui_style();
if (p_macatsui)
draw_string_ATSUI(row, col, s, len, flags);
else
#endif
draw_string_QD(row, col, s, len, flags);
}
int
gui_mch_haskey(char_u *name)
{
int i;
for (i = 0; special_keys[i].key_sym != (KeySym)0; i++)
if (name[0] == special_keys[i].vim_code0 &&
name[1] == special_keys[i].vim_code1)
return OK;
return FAIL;
}
void
gui_mch_beep(void)
{
SysBeep(1); 
}
void
gui_mch_flash(int msec)
{
Rect rc;
rc.left = 0;
rc.top = 0;
rc.right = gui.num_cols * gui.char_width;
rc.bottom = gui.num_rows * gui.char_height;
InvertRect(&rc);
ui_delay((long)msec, TRUE); 
InvertRect(&rc);
}
void
gui_mch_invert_rectangle(int r, int c, int nr, int nc)
{
Rect rc;
rc.left = FILL_X(c);
rc.top = FILL_Y(r);
rc.right = rc.left + nc * gui.char_width;
rc.bottom = rc.top + nr * gui.char_height;
InvertRect(&rc);
}
void
gui_mch_iconify(void)
{
}
#if defined(FEAT_EVAL) || defined(PROTO)
void
gui_mch_set_foreground(void)
{
}
#endif
void
gui_mch_draw_hollow_cursor(guicolor_T color)
{
Rect rc;
rc.left = FILL_X(gui.col);
rc.top = FILL_Y(gui.row);
rc.right = rc.left + gui.char_width;
if (mb_lefthalve(gui.row, gui.col))
rc.right += gui.char_width;
rc.bottom = rc.top + gui.char_height;
gui_mch_set_fg_color(color);
FrameRect(&rc);
}
void
gui_mch_draw_part_cursor(int w, int h, guicolor_T color)
{
Rect rc;
#if defined(FEAT_RIGHTLEFT)
if (CURSOR_BAR_RIGHT)
rc.left = FILL_X(gui.col + 1) - w;
else
#endif
rc.left = FILL_X(gui.col);
rc.top = FILL_Y(gui.row) + gui.char_height - h;
rc.right = rc.left + w;
rc.bottom = rc.top + h;
gui_mch_set_fg_color(color);
FrameRect(&rc);
}
void
gui_mch_update(void)
{
EventRecord theEvent;
if (EventAvail(everyEvent, &theEvent))
if (theEvent.what != nullEvent)
gui_mch_wait_for_chars(0);
}
pascal
Boolean
WaitNextEventWrp(EventMask eventMask, EventRecord *theEvent, UInt32 sleep, RgnHandle mouseRgn)
{
if (((long) sleep) < -1)
sleep = 32767;
return WaitNextEvent(eventMask, theEvent, sleep, mouseRgn);
}
int
gui_mch_wait_for_chars(int wtime)
{
EventMask mask = (everyEvent);
EventRecord event;
long entryTick;
long currentTick;
long sleeppyTick;
if (dragged_sb != NULL)
return FAIL;
entryTick = TickCount();
allow_scrollbar = TRUE;
do
{
#if 0
if (dragRectControl == kCreateEmpty)
{
dragRgn = NULL;
dragRectControl = kNothing;
}
else
#endif
if (dragRectControl == kCreateRect)
{
dragRgn = cursorRgn;
RectRgn(dragRgn, &dragRect);
dragRectControl = kNothing;
}
if (wtime > -1)
sleeppyTick = 60 * wtime / 1000;
else
sleeppyTick = 32767;
if (WaitNextEventWrp(mask, &event, sleeppyTick, dragRgn))
{
gui_mac_handle_event(&event);
if (input_available())
{
allow_scrollbar = FALSE;
return OK;
}
}
currentTick = TickCount();
}
while ((wtime == -1) || ((currentTick - entryTick) < 60*wtime/1000));
allow_scrollbar = FALSE;
return FAIL;
}
void
gui_mch_flush(void)
{
}
void
gui_mch_clear_block(int row1, int col1, int row2, int col2)
{
Rect rc;
rc.left = FILL_X(col1);
rc.top = FILL_Y(row1);
rc.right = FILL_X(col2 + 1) + (col2 == Columns - 1);
rc.bottom = FILL_Y(row2 + 1);
gui_mch_set_bg_color(gui.back_pixel);
EraseRect(&rc);
}
void
gui_mch_clear_all(void)
{
Rect rc;
rc.left = 0;
rc.top = 0;
rc.right = Columns * gui.char_width + 2 * gui.border_width;
rc.bottom = Rows * gui.char_height + 2 * gui.border_width;
gui_mch_set_bg_color(gui.back_pixel);
EraseRect(&rc);
}
void
gui_mch_delete_lines(int row, int num_lines)
{
Rect rc;
rc.left = FILL_X(gui.scroll_region_left);
rc.right = FILL_X(gui.scroll_region_right + 1);
rc.top = FILL_Y(row);
rc.bottom = FILL_Y(gui.scroll_region_bot + 1);
gui_mch_set_bg_color(gui.back_pixel);
ScrollRect(&rc, 0, -num_lines * gui.char_height, (RgnHandle) nil);
gui_clear_block(gui.scroll_region_bot - num_lines + 1,
gui.scroll_region_left,
gui.scroll_region_bot, gui.scroll_region_right);
}
void
gui_mch_insert_lines(int row, int num_lines)
{
Rect rc;
rc.left = FILL_X(gui.scroll_region_left);
rc.right = FILL_X(gui.scroll_region_right + 1);
rc.top = FILL_Y(row);
rc.bottom = FILL_Y(gui.scroll_region_bot + 1);
gui_mch_set_bg_color(gui.back_pixel);
ScrollRect(&rc, 0, gui.char_height * num_lines, (RgnHandle) nil);
if (gui.cursor_row >= gui.row
&& gui.cursor_col >= gui.scroll_region_left
&& gui.cursor_col <= gui.scroll_region_right)
{
if (gui.cursor_row <= gui.scroll_region_bot - num_lines)
gui.cursor_row += num_lines;
else if (gui.cursor_row <= gui.scroll_region_bot)
gui.cursor_is_valid = FALSE;
}
gui_clear_block(row, gui.scroll_region_left,
row + num_lines - 1, gui.scroll_region_right);
}
void
clip_mch_request_selection(Clipboard_T *cbd)
{
Handle textOfClip;
int flavor = 0;
Size scrapSize;
ScrapFlavorFlags scrapFlags;
ScrapRef scrap = nil;
OSStatus error;
int type;
char *searchCR;
char_u *tempclip;
error = GetCurrentScrap(&scrap);
if (error != noErr)
return;
error = GetScrapFlavorFlags(scrap, VIMSCRAPFLAVOR, &scrapFlags);
if (error == noErr)
{
error = GetScrapFlavorSize(scrap, VIMSCRAPFLAVOR, &scrapSize);
if (error == noErr && scrapSize > 1)
flavor = 1;
}
if (flavor == 0)
{
error = GetScrapFlavorFlags(scrap, SCRAPTEXTFLAVOR, &scrapFlags);
if (error != noErr)
return;
error = GetScrapFlavorSize(scrap, SCRAPTEXTFLAVOR, &scrapSize);
if (error != noErr)
return;
}
ReserveMem(scrapSize);
textOfClip = NewHandle(scrapSize);
HLock(textOfClip);
error = GetScrapFlavorData(scrap,
flavor ? VIMSCRAPFLAVOR : SCRAPTEXTFLAVOR,
&scrapSize, *textOfClip);
scrapSize -= flavor;
if (flavor)
type = **textOfClip;
else
type = MAUTO;
tempclip = alloc(scrapSize + 1);
mch_memmove(tempclip, *textOfClip + flavor, scrapSize);
tempclip[scrapSize] = 0;
#if defined(MACOS_CONVERT)
{
size_t encLen = 0;
char_u *to = mac_utf16_to_enc((UniChar *)tempclip, scrapSize, &encLen);
if (to != NULL)
{
scrapSize = encLen;
vim_free(tempclip);
tempclip = to;
}
}
#endif
searchCR = (char *)tempclip;
while (searchCR != NULL)
{
searchCR = strchr(searchCR, '\r');
if (searchCR != NULL)
*searchCR = '\n';
}
clip_yank_selection(type, tempclip, scrapSize, cbd);
vim_free(tempclip);
HUnlock(textOfClip);
DisposeHandle(textOfClip);
}
void
clip_mch_lose_selection(Clipboard_T *cbd)
{
}
int
clip_mch_own_selection(Clipboard_T *cbd)
{
return OK;
}
void
clip_mch_set_selection(Clipboard_T *cbd)
{
Handle textOfClip;
long scrapSize;
int type;
ScrapRef scrap;
char_u *str = NULL;
if (!cbd->owned)
return;
clip_get_selection(cbd);
cbd->owned = FALSE;
type = clip_convert_selection(&str, (long_u *)&scrapSize, cbd);
#if defined(MACOS_CONVERT)
size_t utf16_len = 0;
UniChar *to = mac_enc_to_utf16(str, scrapSize, &utf16_len);
if (to)
{
scrapSize = utf16_len;
vim_free(str);
str = (char_u *)to;
}
#endif
if (type >= 0)
{
ClearCurrentScrap();
textOfClip = NewHandle(scrapSize + 1);
HLock(textOfClip);
**textOfClip = type;
mch_memmove(*textOfClip + 1, str, scrapSize);
GetCurrentScrap(&scrap);
PutScrapFlavor(scrap, SCRAPTEXTFLAVOR, kScrapFlavorMaskNone,
scrapSize, *textOfClip + 1);
PutScrapFlavor(scrap, VIMSCRAPFLAVOR, kScrapFlavorMaskNone,
scrapSize + 1, *textOfClip);
HUnlock(textOfClip);
DisposeHandle(textOfClip);
}
vim_free(str);
}
void
gui_mch_set_text_area_pos(int x, int y, int w, int h)
{
Rect VimBound;
GetWindowBounds(gui.VimWindow, kWindowGlobalPortRgn, &VimBound);
if (gui.which_scrollbars[SBAR_LEFT])
VimBound.left = -gui.scrollbar_width + 1;
else
VimBound.left = 0;
SetWindowBounds(gui.VimWindow, kWindowGlobalPortRgn, &VimBound);
ShowWindow(gui.VimWindow);
}
void
gui_mch_enable_menu(int flag)
{
}
void
gui_mch_set_menu_pos(int x, int y, int w, int h)
{
}
void
gui_mch_add_menu(vimmenu_T *menu, int idx)
{
static long next_avail_id = 128;
long menu_after_me = 0; 
CFStringRef name;
short index;
vimmenu_T *parent = menu->parent;
vimmenu_T *brother = menu->next;
if ((parent != NULL && parent->submenu_id == 0))
return;
if (next_avail_id == 1024)
return;
while ((brother != NULL) && (!menu_is_menubar(brother->name)))
brother = brother->next;
if ((parent == NULL) && (brother != NULL))
menu_after_me = brother->submenu_id;
if (!menu_is_menubar(menu->name))
menu_after_me = hierMenu;
#if defined(MACOS_CONVERT)
name = menu_title_removing_mnemonic(menu);
#else
name = C2Pascal_save(menu->dname);
#endif
if (name == NULL)
return;
{
menu->submenu_id = next_avail_id;
if (CreateNewMenu(menu->submenu_id, 0, (MenuRef *)&menu->submenu_handle) == noErr)
SetMenuTitleWithCFString((MenuRef)menu->submenu_handle, name);
next_avail_id++;
}
if (parent == NULL)
{
InsertMenu(menu->submenu_handle, menu_after_me); 
#if 1
DrawMenuBar();
#endif
}
else
{
index = gui_mac_get_menu_item_index(menu);
InsertMenuItem(parent->submenu_handle, "\p ", idx); 
SetMenuItemTextWithCFString(parent->submenu_handle, idx+1, name);
SetItemCmd(parent->submenu_handle, idx+1, 0x1B);
SetItemMark(parent->submenu_handle, idx+1, menu->submenu_id);
InsertMenu(menu->submenu_handle, hierMenu);
}
CFRelease(name);
#if 0
DrawMenuBar();
#endif
}
void
gui_mch_add_menu_item(vimmenu_T *menu, int idx)
{
CFStringRef name;
vimmenu_T *parent = menu->parent;
int menu_inserted;
if (parent->submenu_id == 0)
return;
#if defined(MACOS_CONVERT)
name = menu_title_removing_mnemonic(menu);
#else
name = C2Pascal_save(menu->dname);
#endif
menu->submenu_id = 0;
menu->submenu_handle = NULL;
menu_inserted = 0;
if (menu->actext)
{
int key = 0;
int modifiers = 0;
char_u *p_actext;
p_actext = menu->actext;
key = find_special_key(&p_actext, &modifiers, FALSE, FALSE, FALSE,
TRUE, NULL);
if (*p_actext != 0)
key = 0; 
if (key > 0 && key < 32)
{
modifiers |= MOD_MASK_CTRL;
key += '@';
}
else if (key >= 'A' && key <= 'Z')
modifiers |= MOD_MASK_SHIFT;
else if (key >= 'a' && key <= 'z')
key += 'A' - 'a';
if (key >= 0x22 && vim_isprintc_strict(key))
{
int valid = 1;
char_u mac_mods = kMenuNoModifiers;
if (modifiers & MOD_MASK_SHIFT)
mac_mods |= kMenuShiftModifier;
if (modifiers & MOD_MASK_CTRL)
mac_mods |= kMenuControlModifier;
if (!(modifiers & MOD_MASK_CMD))
mac_mods |= kMenuNoCommandModifier;
if (modifiers & MOD_MASK_ALT || modifiers & MOD_MASK_MULTI_CLICK)
valid = 0; 
if (valid)
{
char_u item_txt[10];
item_txt[0] = 3; item_txt[1] = ' '; item_txt[2] = '/';
item_txt[3] = key;
InsertMenuItem(parent->submenu_handle, item_txt, idx);
SetMenuItemModifiers(parent->submenu_handle, idx+1, mac_mods);
menu_inserted = 1;
}
}
}
if (!menu_inserted)
InsertMenuItem(parent->submenu_handle, "\p ", idx); 
SetMenuItemTextWithCFString(parent->submenu_handle, idx+1, name);
#if 0
DrawMenuBar();
#endif
CFRelease(name);
}
void
gui_mch_toggle_tearoffs(int enable)
{
}
void
gui_mch_destroy_menu(vimmenu_T *menu)
{
short index = gui_mac_get_menu_item_index(menu);
if (index > 0)
{
if (menu->parent)
{
{
DeleteMenuItem(menu->parent->submenu_handle, index);
if (menu->submenu_id != 0)
{
DeleteMenu(menu->submenu_id);
DisposeMenu(menu->submenu_handle);
}
}
}
#if defined(DEBUG_MAC_MENU)
else
{
printf("gmdm 2\n");
}
#endif
}
else
{
{
DeleteMenu(menu->submenu_id);
DisposeMenu(menu->submenu_handle);
}
}
DrawMenuBar();
}
void
gui_mch_menu_grey(vimmenu_T *menu, int grey)
{
short index = gui_mac_get_menu_item_index(menu);
if (grey)
{
if (menu->children)
DisableMenuItem(menu->submenu_handle, index);
if (menu->parent)
if (menu->parent->submenu_handle)
DisableMenuItem(menu->parent->submenu_handle, index);
}
else
{
if (menu->children)
EnableMenuItem(menu->submenu_handle, index);
if (menu->parent)
if (menu->parent->submenu_handle)
EnableMenuItem(menu->parent->submenu_handle, index);
}
}
void
gui_mch_menu_hidden(vimmenu_T *menu, int hidden)
{
gui_mch_menu_grey(menu, hidden);
}
void
gui_mch_draw_menubar(void)
{
DrawMenuBar();
}
void
gui_mch_enable_scrollbar(
scrollbar_T *sb,
int flag)
{
if (flag)
ShowControl(sb->id);
else
HideControl(sb->id);
#if defined(DEBUG_MAC_SB)
printf("enb_sb (%x) %x\n",sb->id, flag);
#endif
}
void
gui_mch_set_scrollbar_thumb(
scrollbar_T *sb,
long val,
long size,
long max)
{
SetControl32BitMaximum (sb->id, max);
SetControl32BitMinimum (sb->id, 0);
SetControl32BitValue (sb->id, val);
SetControlViewSize (sb->id, size);
#if defined(DEBUG_MAC_SB)
printf("thumb_sb (%x) %lx, %lx,%lx\n",sb->id, val, size, max);
#endif
}
void
gui_mch_set_scrollbar_pos(
scrollbar_T *sb,
int x,
int y,
int w,
int h)
{
gui_mch_set_bg_color(gui.back_pixel);
#if 0
if (gui.which_scrollbars[SBAR_LEFT])
{
MoveControl(sb->id, x-16, y);
SizeControl(sb->id, w + 1, h);
}
else
{
MoveControl(sb->id, x, y);
SizeControl(sb->id, w + 1, h);
}
#endif
if (sb == &gui.bottom_sbar)
h += 1;
else
w += 1;
if (gui.which_scrollbars[SBAR_LEFT])
x -= 15;
MoveControl(sb->id, x, y);
SizeControl(sb->id, w, h);
#if defined(DEBUG_MAC_SB)
printf("size_sb (%x) %x, %x, %x, %x\n",sb->id, x, y, w, h);
#endif
}
void
gui_mch_create_scrollbar(
scrollbar_T *sb,
int orient) 
{
Rect bounds;
bounds.top = -16;
bounds.bottom = -10;
bounds.right = -10;
bounds.left = -16;
sb->id = NewControl(gui.VimWindow,
&bounds,
"\pScrollBar",
TRUE,
0, 
0, 
0, 
kControlScrollBarLiveProc,
(long) sb->ident);
#if defined(DEBUG_MAC_SB)
printf("create_sb (%x) %x\n",sb->id, orient);
#endif
}
void
gui_mch_destroy_scrollbar(scrollbar_T *sb)
{
gui_mch_set_bg_color(gui.back_pixel);
DisposeControl(sb->id);
#if defined(DEBUG_MAC_SB)
printf("dest_sb (%x) \n",sb->id);
#endif
}
int
gui_mch_is_blinking(void)
{
return FALSE;
}
int
gui_mch_is_blink_off(void)
{
return FALSE;
}
void
gui_mch_set_blinking(long wait, long on, long off)
{
#if 0
blink_waittime = wait;
blink_ontime = on;
blink_offtime = off;
#endif
}
void
gui_mch_stop_blink(int may_call_gui_update_cursor)
{
if (may_call_gui_update_cursor)
gui_update_cursor(TRUE, FALSE);
#if 0
gui_w32_rm_blink_timer();
if (blink_state == BLINK_OFF)
gui_update_cursor(TRUE, FALSE);
blink_state = BLINK_NONE;
#endif
}
void
gui_mch_start_blink(void)
{
gui_update_cursor(TRUE, FALSE);
#if 0
if (blink_waittime && blink_ontime && blink_offtime)
{
blink_timer = SetTimer(NULL, 0, (UINT)blink_waittime,
(TIMERPROC)_OnBlinkTimer);
blink_state = BLINK_ON;
gui_update_cursor(TRUE, FALSE);
}
#endif
}
guicolor_T
gui_mch_get_rgb(guicolor_T pixel)
{
return (guicolor_T)((Red(pixel) << 16) + (Green(pixel) << 8) + Blue(pixel));
}
#if defined(FEAT_BROWSE)
char_u *
gui_mch_browse(
int saving,
char_u *title,
char_u *dflt,
char_u *ext,
char_u *initdir,
char_u *filter)
{
NavReplyRecord reply;
char_u *fname = NULL;
char_u **fnames = NULL;
long numFiles;
NavDialogOptions navOptions;
OSErr error;
NavGetDefaultDialogOptions(&navOptions);
navOptions.dialogOptionFlags = kNavAllowInvisibleFiles
| kNavDontAutoTranslate
| kNavDontAddTranslateItems
| kNavAllowStationery;
(void) C2PascalString(title, &navOptions.message);
(void) C2PascalString(dflt, &navOptions.savedFileName);
if (saving)
{
NavPutFile(NULL, &reply, &navOptions, NULL, 'TEXT', 'VIM!', NULL);
if (!reply.validRecord)
return NULL;
}
else
{
NavGetFile(NULL, &reply, &navOptions, NULL, NULL, NULL, NULL, NULL);
if (!reply.validRecord)
return NULL;
}
fnames = new_fnames_from_AEDesc(&reply.selection, &numFiles, &error);
NavDisposeReply(&reply);
if (fnames)
{
fname = fnames[0];
vim_free(fnames);
}
return fname;
}
#endif 
#if defined(FEAT_GUI_DIALOG)
typedef struct
{
short idx;
short width; 
Rect box;
} vgmDlgItm; 
#define MoveRectTo(r,x,y) OffsetRect(r,x-r->left,y-r->top)
static void
macMoveDialogItem(
DialogRef theDialog,
short itemNumber,
short X,
short Y,
Rect *inBox)
{
#if 0 
MoveDialogItem(theDialog, itemNumber, X, Y);
if (inBox != nil)
GetDialogItem(theDialog, itemNumber, &itemType, &itemHandle, inBox);
#else
short itemType;
Handle itemHandle;
Rect localBox;
Rect *itemBox = &localBox;
if (inBox != nil)
itemBox = inBox;
GetDialogItem(theDialog, itemNumber, &itemType, &itemHandle, itemBox);
OffsetRect(itemBox, -itemBox->left, -itemBox->top);
OffsetRect(itemBox, X, Y);
if (1) 
MoveControl((ControlRef) itemHandle, X, Y);
SetDialogItem(theDialog, itemNumber, itemType, itemHandle, itemBox);
#endif
}
static void
macSizeDialogItem(
DialogRef theDialog,
short itemNumber,
short width,
short height)
{
short itemType;
Handle itemHandle;
Rect itemBox;
GetDialogItem(theDialog, itemNumber, &itemType, &itemHandle, &itemBox);
if (width == 0)
width = itemBox.right - itemBox.left;
if (height == 0)
height = itemBox.bottom - itemBox.top;
#if 0 
SizeDialogItem(theDialog, itemNumber, width, height); 
#else
itemBox.right = itemBox.left + width;
itemBox.bottom = itemBox.top + height;
if (itemType & kControlDialogItem)
SizeControl((ControlRef) itemHandle, width, height);
SetDialogItem(theDialog, itemNumber, itemType, itemHandle, &itemBox);
#endif
}
static void
macSetDialogItemText(
DialogRef theDialog,
short itemNumber,
Str255 itemName)
{
short itemType;
Handle itemHandle;
Rect itemBox;
GetDialogItem(theDialog, itemNumber, &itemType, &itemHandle, &itemBox);
if (itemType & kControlDialogItem)
SetControlTitle((ControlRef) itemHandle, itemName);
else
SetDialogItemText(itemHandle, itemName);
}
static pascal Boolean
DialogHotkeyFilterProc (
DialogRef theDialog,
EventRecord *event,
DialogItemIndex *itemHit)
{
char_u keyHit;
if (event->what == keyDown || event->what == autoKey)
{
keyHit = (event->message & charCodeMask);
if (gDialogHotKeys && gDialogHotKeys[keyHit])
{
#if defined(DEBUG_MAC_DIALOG_HOTKEYS)
printf("user pressed hotkey '%c' --> item %d\n", keyHit, gDialogHotKeys[keyHit]);
#endif
*itemHit = gDialogHotKeys[keyHit];
event->what = kEventControlSimulateHit;
return true; 
}
return StdFilterProc(theDialog, event, itemHit);
}
return false; 
}
int
gui_mch_dialog(
int type,
char_u *title,
char_u *message,
char_u *buttons,
int dfltbutton,
char_u *textfield,
int ex_cmd)
{
Handle buttonDITL;
Handle iconDITL;
Handle inputDITL;
Handle messageDITL;
Handle itemHandle;
Handle iconHandle;
DialogPtr theDialog;
char_u len;
char_u PascalTitle[256]; 
char_u name[256];
GrafPtr oldPort;
short itemHit;
char_u *buttonChar;
short hotKeys[256]; 
char_u aHotKey;
Rect box;
short button;
short lastButton;
short itemType;
short useIcon;
short width;
short totalButtonWidth = 0; 
short widestButton = 0;
short dfltButtonEdge = 20; 
short dfltElementSpacing = 13; 
short dfltIconSideSpace = 23; 
short maximumWidth = 400; 
short maxButtonWidth = 175; 
short vertical;
short dialogHeight;
short messageLines = 3;
FontInfo textFontInfo;
vgmDlgItm iconItm;
vgmDlgItm messageItm;
vgmDlgItm inputItm;
vgmDlgItm buttonItm;
WindowRef theWindow;
ModalFilterUPP dialogUPP;
vertical = (vim_strchr(p_go, GO_VERTICAL) != NULL);
theDialog = GetNewDialog(129, nil, (WindowRef) -1);
theWindow = GetDialogWindow(theDialog);
HideWindow(theWindow);
GetPort(&oldPort);
SetPortDialogPort(theDialog);
GetFontInfo(&textFontInfo);
if (title != NULL)
{
(void) C2PascalString(title, &PascalTitle);
SetWTitle(theWindow, PascalTitle);
}
buttonDITL = GetResource('DITL', 130);
buttonChar = buttons;
button = 0;
vim_memset(hotKeys, 0, sizeof(hotKeys));
for (;*buttonChar != 0;)
{
button++;
len = 0;
for (;((*buttonChar != DLG_BUTTON_SEP) && (*buttonChar != 0) && (len < 255)); buttonChar++)
{
if (*buttonChar != DLG_HOTKEY_CHAR)
name[++len] = *buttonChar;
else
{
aHotKey = (char_u)*(buttonChar+1);
if (aHotKey >= 'A' && aHotKey <= 'Z')
aHotKey = (char_u)((int)aHotKey + (int)'a' - (int)'A');
hotKeys[aHotKey] = button;
#if defined(DEBUG_MAC_DIALOG_HOTKEYS)
printf("###hotKey for button %d is '%c'\n", button, aHotKey);
#endif
}
}
if (*buttonChar != 0)
buttonChar++;
name[0] = len;
AppendDITL(theDialog, buttonDITL, overlayDITL); 
macSetDialogItemText(theDialog, button, name);
width = StringWidth(name) + 2 * dfltButtonEdge;
if (width > maxButtonWidth)
width = maxButtonWidth;
macSizeDialogItem(theDialog, button, width, 0);
totalButtonWidth += width;
if (width > widestButton)
widestButton = width;
}
ReleaseResource(buttonDITL);
lastButton = button;
iconItm.idx = lastButton + 1;
iconDITL = GetResource('DITL', 131);
switch (type)
{
case VIM_GENERIC:
case VIM_INFO:
case VIM_QUESTION: useIcon = kNoteIcon; break;
case VIM_WARNING: useIcon = kCautionIcon; break;
case VIM_ERROR: useIcon = kStopIcon; break;
default: useIcon = kStopIcon;
}
AppendDITL(theDialog, iconDITL, overlayDITL);
ReleaseResource(iconDITL);
GetDialogItem(theDialog, iconItm.idx, &itemType, &itemHandle, &box);
iconHandle = GetIcon(useIcon);
SetDialogItem(theDialog, iconItm.idx, itemType, iconHandle, &box);
messageItm.idx = lastButton + 2;
messageDITL = GetResource('DITL', 132);
AppendDITL(theDialog, messageDITL, overlayDITL);
ReleaseResource(messageDITL);
GetDialogItem(theDialog, messageItm.idx, &itemType, &itemHandle, &box);
(void) C2PascalString(message, &name);
SetDialogItemText(itemHandle, name);
messageItm.width = StringWidth(name);
if (textfield != NULL)
{
inputItm.idx = lastButton + 3;
inputDITL = GetResource('DITL', 132);
AppendDITL(theDialog, inputDITL, overlayDITL);
ReleaseResource(inputDITL);
GetDialogItem(theDialog, inputItm.idx, &itemType, &itemHandle, &box);
(void) C2PascalString(textfield, &name);
SetDialogItemText(itemHandle, name);
inputItm.width = StringWidth(name);
gDialogHotKeys = NULL;
}
else
gDialogHotKeys = (short *)&hotKeys;
SetDialogDefaultItem(theDialog, dfltbutton);
SetDialogCancelItem(theDialog, 0);
if (totalButtonWidth > maximumWidth)
vertical = TRUE;
macMoveDialogItem(theDialog, iconItm.idx, dfltIconSideSpace, dfltElementSpacing, &box);
iconItm.box.right = box.right;
iconItm.box.bottom = box.bottom;
messageItm.box.left = iconItm.box.right + dfltIconSideSpace;
macSizeDialogItem(theDialog, messageItm.idx, 0, messageLines * (textFontInfo.ascent + textFontInfo.descent));
macMoveDialogItem(theDialog, messageItm.idx, messageItm.box.left, dfltElementSpacing, &messageItm.box);
if (textfield != NULL)
{
inputItm.box.left = messageItm.box.left;
inputItm.box.top = messageItm.box.bottom + dfltElementSpacing;
macSizeDialogItem(theDialog, inputItm.idx, 0, textFontInfo.ascent + textFontInfo.descent);
macMoveDialogItem(theDialog, inputItm.idx, inputItm.box.left, inputItm.box.top, &inputItm.box);
GetDialogItem(theDialog, inputItm.idx, &itemType, &itemHandle, &inputItm.box);
SetDialogItem(theDialog, inputItm.idx, kEditTextDialogItem, itemHandle, &inputItm.box);
SelectDialogItemText(theDialog, inputItm.idx, 0, 32767);
}
if (textfield != NULL)
{
buttonItm.box.left = inputItm.box.left;
buttonItm.box.top = inputItm.box.bottom + dfltElementSpacing;
}
else
{
buttonItm.box.left = messageItm.box.left;
buttonItm.box.top = messageItm.box.bottom + dfltElementSpacing;
}
for (button=1; button <= lastButton; button++)
{
macMoveDialogItem(theDialog, button, buttonItm.box.left, buttonItm.box.top, &box);
if (vertical)
{
macSizeDialogItem(theDialog, button, widestButton, 0);
GetDialogItem(theDialog, button, &itemType, &itemHandle, &box);
}
if (vertical)
buttonItm.box.top = box.bottom + dfltElementSpacing;
else
buttonItm.box.left = box.right + dfltElementSpacing;
}
dialogHeight = box.bottom + dfltElementSpacing;
SizeWindow(theWindow, maximumWidth, dialogHeight, TRUE);
AutoSizeDialog(theDialog);
ShowWindow(theWindow);
SelectWindow(theWindow);
#if 0
GetPort(&oldPort);
SetPortDialogPort(theDialog);
#endif
#if defined(USE_CARBONKEYHANDLER)
dialog_busy = TRUE;
#endif
dialogUPP = NewModalFilterUPP(DialogHotkeyFilterProc);
do
ModalDialog(dialogUPP, &itemHit);
while ((itemHit < 1) || (itemHit > lastButton));
#if defined(USE_CARBONKEYHANDLER)
dialog_busy = FALSE;
#endif
if (textfield != NULL)
{
GetDialogItem(theDialog, inputItm.idx, &itemType, &itemHandle, &box);
GetDialogItemText(itemHandle, (char_u *) &name);
#if IOSIZE < 256
if (name[0] > IOSIZE)
name[0] = IOSIZE - 1;
#endif
vim_strncpy(textfield, &name[1], name[0]);
}
SetPort(oldPort);
DisposeRoutineDescriptor(dialogUPP);
DisposeDialog(theDialog);
return itemHit;
}
#endif 
#if defined(USE_MCH_ERRMSG)
void
display_errors(void)
{
char *p;
char_u pError[256];
if (error_ga.ga_data == NULL)
return;
for (p = (char *)error_ga.ga_data; *p; ++p)
if (!isspace(*p))
{
if (STRLEN(p) > 255)
pError[0] = 255;
else
pError[0] = STRLEN(p);
STRNCPY(&pError[1], p, pError[0]);
ParamText(pError, nil, nil, nil);
Alert(128, nil);
break;
}
ga_clear(&error_ga);
}
#endif
void
gui_mch_getmouse(int *x, int *y)
{
Point where;
GetMouse(&where);
*x = where.h;
*y = where.v;
}
void
gui_mch_setmouse(int x, int y)
{
#if 0
CursorDevicePtr myMouse;
Point where;
if ( NGetTrapAddress(_CursorDeviceDispatch, ToolTrap)
!= NGetTrapAddress(_Unimplemented, ToolTrap))
{
myMouse = nil;
do
{
CursorDeviceNextDevice(&myMouse);
}
while ((myMouse != nil) && (myMouse->cntButtons != 1));
CursorDeviceMoveTo(myMouse, x, y);
}
else
{
where.h = x;
where.v = y;
*(Point *)RawMouse = where;
*(Point *)MTemp = where;
*(Ptr) CrsrNew = 0xFFFF;
}
#endif
}
void
gui_mch_show_popupmenu(vimmenu_T *menu)
{
MenuHandle CntxMenu;
Point where;
OSStatus status;
UInt32 CntxType;
SInt16 CntxMenuID;
UInt16 CntxMenuItem;
Str255 HelpName = "";
GrafPtr savePort;
GetPort(&savePort); 
GetMouse(&where);
LocalToGlobal(&where); 
CntxMenu = menu->submenu_handle;
status = ContextualMenuSelect(CntxMenu, where, false, kCMHelpItemRemoveHelp,
HelpName, NULL, &CntxType, &CntxMenuID, &CntxMenuItem);
if (status == noErr)
{
if (CntxType == kCMMenuItemSelected)
{
gui_mac_handle_menu((CntxMenuID << 16) + CntxMenuItem);
}
else if (CntxMenuID == kCMShowHelpSelected)
{
}
}
SetPort(savePort); 
}
#if defined(FEAT_CW_EDITOR) || defined(PROTO)
void
mch_post_buffer_write(buf_T *buf)
{
GetFSSpecFromPath(buf->b_ffname, &buf->b_FSSpec);
Send_KAHL_MOD_AE(buf);
}
#endif
#if defined(FEAT_TITLE)
void
gui_mch_settitle(char_u *title, char_u *icon)
{
#if defined(MACOS_CONVERT)
CFStringRef windowTitle;
size_t windowTitleLen;
#else
char_u *pascalTitle;
#endif
if (title == NULL) 
return;
#if defined(MACOS_CONVERT)
windowTitleLen = STRLEN(title);
windowTitle = (CFStringRef)mac_enc_to_cfstring(title, windowTitleLen);
if (windowTitle)
{
SetWindowTitleWithCFString(gui.VimWindow, windowTitle);
CFRelease(windowTitle);
}
#else
pascalTitle = C2Pascal_save(title);
if (pascalTitle != NULL)
{
SetWTitle(gui.VimWindow, pascalTitle);
vim_free(pascalTitle);
}
#endif
}
#endif
int
C2PascalString(char_u *CString, Str255 *PascalString)
{
char_u *PascalPtr = (char_u *) PascalString;
int len;
int i;
PascalPtr[0] = 0;
if (CString == NULL)
return 0;
len = STRLEN(CString);
if (len > 255)
len = 255;
for (i = 0; i < len; i++)
PascalPtr[i+1] = CString[i];
PascalPtr[0] = len;
return 0;
}
int
GetFSSpecFromPath(char_u *file, FSSpec *fileFSSpec)
{
Str255 filePascal;
CInfoPBRec myCPB;
OSErr err;
(void) C2PascalString(file, &filePascal);
myCPB.dirInfo.ioNamePtr = filePascal;
myCPB.dirInfo.ioVRefNum = 0;
myCPB.dirInfo.ioFDirIndex = 0;
myCPB.dirInfo.ioDrDirID = 0;
err= PBGetCatInfo(&myCPB, false);
FSMakeFSSpec(0, 0, filePascal, fileFSSpec);
return 0;
}
char_u *FullPathFromFSSpec_save(FSSpec file)
{
CInfoPBRec theCPB;
char_u fname[256];
char_u *filenamePtr = fname;
OSErr error;
int folder = 1;
#if defined(USE_UNIXFILENAME)
SInt16 dfltVol_vRefNum;
SInt32 dfltVol_dirID;
FSRef refFile;
OSStatus status;
UInt32 pathSize = 256;
char_u pathname[256];
char_u *path = pathname;
#else
Str255 directoryName;
char_u temporary[255];
char_u *temporaryPtr = temporary;
#endif
#if defined(USE_UNIXFILENAME)
error=HGetVol(NULL, &dfltVol_vRefNum, &dfltVol_dirID);
if (error)
return NULL;
#endif
vim_strncpy(filenamePtr, &file.name[1], file.name[0]);
theCPB.dirInfo.ioFDirIndex = 0;
theCPB.dirInfo.ioNamePtr = file.name;
theCPB.dirInfo.ioVRefNum = file.vRefNum;
theCPB.dirInfo.ioDrDirID = file.parID;
error = PBGetCatInfo(&theCPB, false);
if ((error) && (error != fnfErr))
return NULL;
if (((theCPB.hFileInfo.ioFlAttrib & ioDirMask) == 0) || (error))
folder = 0; 
else
folder = 1;
#if defined(USE_UNIXFILENAME)
if (error == fnfErr)
{
FSSpec dirSpec;
FSRef dirRef;
Str255 emptyFilename = "\p";
error = FSMakeFSSpec(theCPB.dirInfo.ioVRefNum,
theCPB.dirInfo.ioDrDirID, emptyFilename, &dirSpec);
if (error)
return NULL;
error = FSpMakeFSRef(&dirSpec, &dirRef);
if (error)
return NULL;
status = FSRefMakePath(&dirRef, (UInt8*)path, pathSize);
if (status)
return NULL;
STRCAT(path, "/");
STRCAT(path, filenamePtr);
}
else
{
error=FSpMakeFSRef(&file, &refFile);
if (error)
return NULL;
status=FSRefMakePath(&refFile, (UInt8 *) path, pathSize);
if (status)
return NULL;
}
if (folder)
STRCAT(path, "/");
return (vim_strsave(path));
#else
theCPB.dirInfo.ioNamePtr = directoryName;
theCPB.dirInfo.ioDrParID = file.parID;
theCPB.dirInfo.ioDrDirID = file.parID;
if ((TRUE) && (file.parID != fsRtDirID ))
do
{
theCPB.dirInfo.ioFDirIndex = -1;
theCPB.dirInfo.ioVRefNum = file.vRefNum;
theCPB.dirInfo.ioDrDirID = theCPB.dirInfo.ioDrParID;
error = PBGetCatInfo(&theCPB,false);
if (error)
return NULL;
STRCPY(temporaryPtr, filenamePtr);
vim_strncpy(filenamePtr, &directoryName[1], directoryName[0]);
STRCAT(filenamePtr, ":");
STRCAT(filenamePtr, temporaryPtr);
}
#if 1 
while ((theCPB.dirInfo.ioDrParID != fsRtDirID)
);
#else
while (theCPB.dirInfo.ioDrDirID != fsRtDirID);
#endif
theCPB.dirInfo.ioFDirIndex = -1;
theCPB.dirInfo.ioVRefNum = file.vRefNum;
theCPB.dirInfo.ioDrDirID = theCPB.dirInfo.ioDrParID;
error = PBGetCatInfo(&theCPB,false);
if (error)
return NULL;
#if defined(USE_UNIXFILENAME)
if (file.vRefNum != dfltVol_vRefNum)
#endif
{
STRCPY(temporaryPtr, filenamePtr);
vim_strncpy(filenamePtr, &directoryName[1], directoryName[0]);
STRCAT(filenamePtr, ":");
STRCAT(filenamePtr, temporaryPtr);
#if defined(USE_UNIXFILENAME)
STRCPY(temporaryPtr, filenamePtr);
filenamePtr[0] = 0; 
STRCAT(filenamePtr, "Volumes:");
STRCAT(filenamePtr, temporaryPtr);
#endif
}
if (folder)
STRCAT(fname, ":");
#if defined(USE_UNIXFILENAME)
STRCPY(&temporaryPtr[1], filenamePtr);
temporaryPtr[0] = '/';
STRCPY(filenamePtr, temporaryPtr);
{
char *p;
for (p = fname; *p; p++)
if (*p == ':')
*p = '/';
}
#endif
return (vim_strsave(fname));
#endif
}
#if defined(USE_CARBONKEYHANDLER) || defined(PROTO)
void
im_set_position(int row, int col)
{
#if 0
im_start_row = row;
im_start_col = col;
#endif
}
static ScriptLanguageRecord gTSLWindow;
static ScriptLanguageRecord gTSLInsert;
static ScriptLanguageRecord gTSLDefault = { 0, 0 };
static Component gTSCWindow;
static Component gTSCInsert;
static Component gTSCDefault;
static int im_initialized = 0;
static void
im_on_window_switch(int active)
{
ScriptLanguageRecord *slptr = NULL;
OSStatus err;
if (! gui.in_use)
return;
if (im_initialized == 0)
{
im_initialized = 1;
GetDefaultInputMethodOfClass(&gTSCDefault, &gTSLDefault,
kKeyboardInputMethodClass);
}
if (active == TRUE)
{
im_is_active = TRUE;
ActivateTSMDocument(gTSMDocument);
slptr = &gTSLWindow;
if (slptr)
{
err = SetDefaultInputMethodOfClass(gTSCWindow, slptr,
kKeyboardInputMethodClass);
if (err == noErr)
err = SetTextServiceLanguage(slptr);
if (err == noErr)
KeyScript(slptr->fScript | smKeyForceKeyScriptMask);
}
}
else
{
err = GetTextServiceLanguage(&gTSLWindow);
if (err == noErr)
slptr = &gTSLWindow;
if (slptr)
GetDefaultInputMethodOfClass(&gTSCWindow, slptr,
kKeyboardInputMethodClass);
im_is_active = FALSE;
DeactivateTSMDocument(gTSMDocument);
}
}
void
im_set_active(int active)
{
ScriptLanguageRecord *slptr = NULL;
OSStatus err;
if (!gui.in_use)
return;
if (im_initialized == 0)
{
im_initialized = 1;
GetDefaultInputMethodOfClass(&gTSCDefault, &gTSLDefault,
kKeyboardInputMethodClass);
}
if (active == TRUE)
{
im_is_active = TRUE;
ActivateTSMDocument(gTSMDocument);
slptr = &gTSLInsert;
if (slptr)
{
err = SetDefaultInputMethodOfClass(gTSCInsert, slptr,
kKeyboardInputMethodClass);
if (err == noErr)
err = SetTextServiceLanguage(slptr);
if (err == noErr)
KeyScript(slptr->fScript | smKeyForceKeyScriptMask);
}
}
else
{
err = GetTextServiceLanguage(&gTSLInsert);
if (err == noErr)
slptr = &gTSLInsert;
if (slptr)
GetDefaultInputMethodOfClass(&gTSCInsert, slptr,
kKeyboardInputMethodClass);
SetDefaultInputMethodOfClass(gTSCDefault, &gTSLDefault,
kKeyboardInputMethodClass);
SetTextServiceLanguage(&gTSLDefault);
im_is_active = FALSE;
DeactivateTSMDocument(gTSMDocument);
}
}
int
im_get_status(void)
{
if (! gui.in_use)
return 0;
return im_is_active;
}
#endif
#if defined(FEAT_GUI_TABLINE) || defined(PROTO)
static MenuRef contextMenu = NULL;
enum
{
kTabContextMenuId = 42
};
static CFStringRef
getTabLabel(tabpage_T *page)
{
get_tabline_label(page, FALSE);
#if defined(MACOS_CONVERT)
return (CFStringRef)mac_enc_to_cfstring(NameBuff, STRLEN(NameBuff));
#else
return CFStringCreateWithCString(kCFAllocatorDefault, (char *)NameBuff,
kCFStringEncodingMacRoman);
#endif
}
#define DRAWER_SIZE 150
#define DRAWER_INSET 16
static ControlRef dataBrowser = NULL;
static CFStringRef *tabLabels = NULL;
static int tabLabelsSize = 0;
enum
{
kTabsColumn = 'Tabs'
};
static int
getTabCount(void)
{
tabpage_T *tp;
int numTabs = 0;
FOR_ALL_TABPAGES(tp)
++numTabs;
return numTabs;
}
static OSStatus
dbItemDataCallback(ControlRef browser,
DataBrowserItemID itemID,
DataBrowserPropertyID property ,
DataBrowserItemDataRef itemData,
Boolean changeValue)
{
OSStatus status = noErr;
if (!changeValue)
{
CFStringRef str;
assert(itemID - 1 >= 0 && itemID - 1 < tabLabelsSize);
str = tabLabels[itemID - 1];
status = SetDataBrowserItemDataText(itemData, str);
}
else
status = errDataBrowserPropertyNotSupported;
return status;
}
static void
dbItemNotificationCallback(ControlRef browser,
DataBrowserItemID item,
DataBrowserItemNotification message)
{
switch (message)
{
case kDataBrowserItemSelected:
send_tabline_event(item);
break;
}
}
static void
dbGetContextualMenuCallback(ControlRef browser,
MenuRef *menu,
UInt32 *helpType,
CFStringRef *helpItemString,
AEDesc *selection)
{
*helpType = kCMHelpItemRemoveHelp; 
*helpItemString = NULL;
*menu = contextMenu;
}
static void
dbSelectContextualMenuCallback(ControlRef browser,
MenuRef menu,
UInt32 selectionType,
SInt16 menuID,
MenuItemIndex menuItem)
{
if (selectionType == kCMMenuItemSelected)
{
MenuCommand command;
GetMenuItemCommandID(menu, menuItem, &command);
Handle items = NewHandle(0);
if (items != NULL)
{
int numItems;
GetDataBrowserItems(browser, kDataBrowserNoItem, false,
kDataBrowserItemIsSelected, items);
numItems = GetHandleSize(items) / sizeof(DataBrowserItemID);
if (numItems > 0)
{
int idx;
DataBrowserItemID *itemsPtr;
HLock(items);
itemsPtr = (DataBrowserItemID *)*items;
idx = itemsPtr[0];
HUnlock(items);
send_tabline_menu_event(idx, command);
}
DisposeHandle(items);
}
}
}
static OSStatus
dbFocusCallback(EventHandlerCallRef handler, EventRef event, void *data)
{
assert(GetEventClass(event) == kEventClassControl
&& GetEventKind(event) == kEventControlSetFocusPart);
return paramErr;
}
static OSStatus
drawerCallback(EventHandlerCallRef handler, EventRef event, void *data)
{
switch (GetEventKind(event))
{
case kEventWindowBoundsChanged: 
{
UInt32 attribs;
GetEventParameter(event, kEventParamAttributes, typeUInt32,
NULL, sizeof(attribs), NULL, &attribs);
if (attribs & kWindowBoundsChangeSizeChanged) 
{
Rect r;
GetWindowBounds(drawer, kWindowContentRgn, &r);
SetRect(&r, 0, 0, r.right - r.left, r.bottom - r.top);
SetControlBounds(dataBrowser, &r);
SetDataBrowserTableViewNamedColumnWidth(dataBrowser,
kTabsColumn, r.right);
}
}
break;
}
return eventNotHandledErr;
}
#import <mach-o/dyld.h>
enum { kMyDataBrowserAttributeListViewAlternatingRowColors = (1 << 1) };
static OSStatus
myDataBrowserChangeAttributes(ControlRef inDataBrowser,
OptionBits inAttributesToSet,
OptionBits inAttributesToClear)
{
long osVersion;
char *symbolName;
NSSymbol symbol = NULL;
OSStatus (*dataBrowserChangeAttributes)(ControlRef inDataBrowser,
OptionBits inAttributesToSet, OptionBits inAttributesToClear);
Gestalt(gestaltSystemVersion, &osVersion);
if (osVersion < 0x1040) 
return noErr;
symbolName = "_DataBrowserChangeAttributes";
if (!NSIsSymbolNameDefined(symbolName)
|| (symbol = NSLookupAndBindSymbol(symbolName)) == NULL)
return noErr;
dataBrowserChangeAttributes = NSAddressOfSymbol(symbol);
if (dataBrowserChangeAttributes == NULL)
return noErr; 
return dataBrowserChangeAttributes(inDataBrowser,
inAttributesToSet, inAttributesToClear);
}
static void
initialise_tabline(void)
{
Rect drawerRect = { 0, 0, 0, DRAWER_SIZE };
DataBrowserCallbacks dbCallbacks;
EventTypeSpec focusEvent = {kEventClassControl, kEventControlSetFocusPart};
EventTypeSpec resizeEvent = {kEventClassWindow, kEventWindowBoundsChanged};
DataBrowserListViewColumnDesc colDesc;
CreateNewWindow(kDrawerWindowClass,
kWindowStandardHandlerAttribute
| kWindowCompositingAttribute
| kWindowResizableAttribute
| kWindowLiveResizeAttribute,
&drawerRect, &drawer);
SetThemeWindowBackground(drawer, kThemeBrushDrawerBackground, true);
SetDrawerParent(drawer, gui.VimWindow);
SetDrawerOffsets(drawer, kWindowOffsetUnchanged, DRAWER_INSET);
CreateDataBrowserControl(drawer, &drawerRect, kDataBrowserListView,
&dataBrowser);
dbCallbacks.version = kDataBrowserLatestCallbacks;
InitDataBrowserCallbacks(&dbCallbacks);
dbCallbacks.u.v1.itemDataCallback =
NewDataBrowserItemDataUPP(dbItemDataCallback);
dbCallbacks.u.v1.itemNotificationCallback =
NewDataBrowserItemNotificationUPP(dbItemNotificationCallback);
dbCallbacks.u.v1.getContextualMenuCallback =
NewDataBrowserGetContextualMenuUPP(dbGetContextualMenuCallback);
dbCallbacks.u.v1.selectContextualMenuCallback =
NewDataBrowserSelectContextualMenuUPP(dbSelectContextualMenuCallback);
SetDataBrowserCallbacks(dataBrowser, &dbCallbacks);
SetDataBrowserListViewHeaderBtnHeight(dataBrowser, 0); 
SetDataBrowserHasScrollBars(dataBrowser, false, true); 
SetDataBrowserSelectionFlags(dataBrowser,
kDataBrowserSelectOnlyOne | kDataBrowserNeverEmptySelectionSet);
SetDataBrowserTableViewHiliteStyle(dataBrowser,
kDataBrowserTableViewFillHilite);
Boolean b = false;
SetControlData(dataBrowser, kControlEntireControl,
kControlDataBrowserIncludesFrameAndFocusTag, sizeof(b), &b);
myDataBrowserChangeAttributes(dataBrowser,
kMyDataBrowserAttributeListViewAlternatingRowColors, 0);
InstallControlEventHandler(dataBrowser, dbFocusCallback, 1, &focusEvent,
NULL, NULL);
InstallWindowEventHandler(drawer, drawerCallback, 1, &resizeEvent,
NULL, NULL);
colDesc.propertyDesc.propertyID = kTabsColumn;
colDesc.propertyDesc.propertyType = kDataBrowserTextType;
colDesc.propertyDesc.propertyFlags = kDataBrowserDefaultPropertyFlags;
colDesc.headerBtnDesc.version = kDataBrowserListViewLatestHeaderDesc;
colDesc.headerBtnDesc.minimumWidth = 100;
colDesc.headerBtnDesc.maximumWidth = 150;
colDesc.headerBtnDesc.titleOffset = 0;
colDesc.headerBtnDesc.titleString = CFSTR("Tabs");
colDesc.headerBtnDesc.initialOrder = kDataBrowserOrderIncreasing;
colDesc.headerBtnDesc.btnFontStyle.flags = 0; 
colDesc.headerBtnDesc.btnContentInfo.contentType = kControlContentTextOnly;
AddDataBrowserListViewColumn(dataBrowser, &colDesc, 0);
CreateNewMenu(kTabContextMenuId, 0, &contextMenu);
AppendMenuItemTextWithCFString(contextMenu, CFSTR("Close Tab"), 0,
TABLINE_MENU_CLOSE, NULL);
AppendMenuItemTextWithCFString(contextMenu, CFSTR("New Tab"), 0,
TABLINE_MENU_NEW, NULL);
AppendMenuItemTextWithCFString(contextMenu, CFSTR("Open Tab..."), 0,
TABLINE_MENU_OPEN, NULL);
}
void
gui_mch_show_tabline(int showit)
{
if (showit == 0)
CloseDrawer(drawer, true);
else
OpenDrawer(drawer, kWindowEdgeRight, true);
}
int
gui_mch_showing_tabline(void)
{
WindowDrawerState state = GetDrawerState(drawer);
return state == kWindowDrawerOpen || state == kWindowDrawerOpening;
}
void
gui_mch_update_tabline(void)
{
tabpage_T *tp;
int numTabs = getTabCount();
int nr = 1;
int curtabidx = 1;
if (tabLabels != NULL)
{
int i;
for (i = 0; i < tabLabelsSize; ++i)
CFRelease(tabLabels[i]);
free(tabLabels);
}
tabLabels = (CFStringRef *)malloc(numTabs * sizeof(CFStringRef));
tabLabelsSize = numTabs;
for (tp = first_tabpage; tp != NULL; tp = tp->tp_next, ++nr)
{
if (tp == curtab)
curtabidx = nr;
tabLabels[nr-1] = getTabLabel(tp);
}
RemoveDataBrowserItems(dataBrowser, kDataBrowserNoItem, 0, NULL,
kDataBrowserItemNoProperty);
AddDataBrowserItems(dataBrowser, kDataBrowserNoItem, numTabs, NULL,
kDataBrowserItemNoProperty);
DataBrowserItemID item = curtabidx;
SetDataBrowserSelectedItems(dataBrowser, 1, &item, kDataBrowserItemsAssign);
}
void
gui_mch_set_curtab(int nr)
{
DataBrowserItemID item = nr;
SetDataBrowserSelectedItems(dataBrowser, 1, &item, kDataBrowserItemsAssign);
RevealDataBrowserItem(dataBrowser, item, kTabsColumn,
kDataBrowserRevealOnly);
}
#endif 
