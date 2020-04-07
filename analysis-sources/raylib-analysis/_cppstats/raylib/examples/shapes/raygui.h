#define RAYGUI_VERSION "2.5-dev"
#if !defined(RAYGUI_STANDALONE)
#include "raylib.h"
#endif
#if defined(RAYGUI_IMPLEMENTATION)
#if defined(_WIN32) && defined(BUILD_LIBTYPE_SHARED)
#define RAYGUIDEF __declspec(dllexport) extern 
#elif defined(_WIN32) && defined(USE_LIBTYPE_SHARED)
#define RAYGUIDEF __declspec(dllimport) 
#else
#if defined(__cplusplus)
#define RAYGUIDEF extern "C" 
#else
#define RAYGUIDEF extern 
#endif
#endif
#elif defined(RAYGUI_STATIC)
#define RAYGUIDEF static 
#endif
#include <stdlib.h> 
#define TEXTEDIT_CURSOR_BLINK_FRAMES 20 
#define NUM_CONTROLS 16 
#define NUM_PROPS_DEFAULT 16 
#define NUM_PROPS_EXTENDED 8 
#if defined(RAYGUI_STANDALONE)
#if !defined(__cplusplus)
#if !defined(true)
typedef enum { false, true } bool;
#endif
#endif
typedef struct Vector2 {
float x;
float y;
} Vector2;
typedef struct Vector3 {
float x;
float y;
float z;
} Vector3;
typedef struct Color {
unsigned char r;
unsigned char g;
unsigned char b;
unsigned char a;
} Color;
typedef struct Rectangle {
int x;
int y;
int width;
int height;
} Rectangle;
typedef struct Texture2D Texture2D;
typedef struct Font Font;
#endif
#if defined(RAYGUI_TEXTBOX_EXTENDED)
typedef struct GuiTextBoxState {
int cursor; 
int start; 
int index; 
int select; 
} GuiTextBoxState;
#endif
typedef enum {
GUI_STATE_NORMAL = 0,
GUI_STATE_FOCUSED,
GUI_STATE_PRESSED,
GUI_STATE_DISABLED,
} GuiControlState;
typedef enum {
GUI_TEXT_ALIGN_LEFT = 0,
GUI_TEXT_ALIGN_CENTER,
GUI_TEXT_ALIGN_RIGHT,
} GuiTextAlignment;
typedef enum {
DEFAULT = 0,
LABEL, 
BUTTON, 
TOGGLE, 
SLIDER, 
PROGRESSBAR,
CHECKBOX,
COMBOBOX,
DROPDOWNBOX,
TEXTBOX, 
VALUEBOX,
SPINNER,
LISTVIEW,
COLORPICKER,
SCROLLBAR,
RESERVED
} GuiControl;
typedef enum {
BORDER_COLOR_NORMAL = 0,
BASE_COLOR_NORMAL,
TEXT_COLOR_NORMAL,
BORDER_COLOR_FOCUSED,
BASE_COLOR_FOCUSED,
TEXT_COLOR_FOCUSED,
BORDER_COLOR_PRESSED,
BASE_COLOR_PRESSED,
TEXT_COLOR_PRESSED,
BORDER_COLOR_DISABLED,
BASE_COLOR_DISABLED,
TEXT_COLOR_DISABLED,
BORDER_WIDTH,
INNER_PADDING,
TEXT_ALIGNMENT,
RESERVED02
} GuiControlProperty;
typedef enum {
TEXT_SIZE = 16,
TEXT_SPACING,
LINE_COLOR,
BACKGROUND_COLOR,
} GuiDefaultProperty;
typedef enum {
GROUP_PADDING = 16,
} GuiToggleProperty;
typedef enum {
SLIDER_WIDTH = 16,
TEXT_PADDING
} GuiSliderProperty;
typedef enum {
CHECK_TEXT_PADDING = 16
} GuiCheckBoxProperty;
typedef enum {
SELECTOR_WIDTH = 16,
SELECTOR_PADDING
} GuiComboBoxProperty;
typedef enum {
ARROW_RIGHT_PADDING = 16,
} GuiDropdownBoxProperty;
typedef enum {
MULTILINE_PADDING = 16,
COLOR_SELECTED_FG,
COLOR_SELECTED_BG
} GuiTextBoxProperty;
typedef enum {
SELECT_BUTTON_WIDTH = 16,
SELECT_BUTTON_PADDING,
SELECT_BUTTON_BORDER_WIDTH
} GuiSpinnerProperty;
typedef enum {
ARROWS_SIZE = 16,
SLIDER_PADDING,
SLIDER_SIZE,
SCROLL_SPEED,
ARROWS_VISIBLE
} GuiScrollBarProperty;
typedef enum {
SCROLLBAR_LEFT_SIDE = 0,
SCROLLBAR_RIGHT_SIDE
} GuiScrollBarSide;
typedef enum {
ELEMENTS_HEIGHT = 16,
ELEMENTS_PADDING,
SCROLLBAR_WIDTH,
SCROLLBAR_SIDE, 
} GuiListViewProperty;
typedef enum {
COLOR_SELECTOR_SIZE = 16,
BAR_WIDTH, 
BAR_PADDING, 
BAR_SELECTOR_HEIGHT, 
BAR_SELECTOR_PADDING 
} GuiColorPickerProperty;
RAYGUIDEF void GuiEnable(void); 
RAYGUIDEF void GuiDisable(void); 
RAYGUIDEF void GuiLock(void); 
RAYGUIDEF void GuiUnlock(void); 
RAYGUIDEF void GuiState(int state); 
RAYGUIDEF void GuiFont(Font font); 
RAYGUIDEF void GuiFade(float alpha); 
RAYGUIDEF void GuiSetStyle(int control, int property, int value); 
RAYGUIDEF int GuiGetStyle(int control, int property); 
#if defined(RAYGUI_TEXTBOX_EXTENDED)
RAYGUIDEF void GuiTextBoxSetActive(Rectangle bounds); 
RAYGUIDEF Rectangle GuiTextBoxGetActive(void); 
RAYGUIDEF void GuiTextBoxSetCursor(int cursor); 
RAYGUIDEF int GuiTextBoxGetCursor(void); 
RAYGUIDEF void GuiTextBoxSetSelection(int start, int length); 
RAYGUIDEF Vector2 GuiTextBoxGetSelection(void); 
RAYGUIDEF bool GuiTextBoxIsActive(Rectangle bounds); 
RAYGUIDEF GuiTextBoxState GuiTextBoxGetState(void); 
RAYGUIDEF void GuiTextBoxSetState(GuiTextBoxState state); 
RAYGUIDEF void GuiTextBoxSelectAll(const char *text); 
RAYGUIDEF void GuiTextBoxCopy(const char *text); 
RAYGUIDEF void GuiTextBoxPaste(char *text, int textSize); 
RAYGUIDEF void GuiTextBoxCut(char *text); 
RAYGUIDEF int GuiTextBoxDelete(char *text, int length, bool before); 
RAYGUIDEF int GuiTextBoxGetByteIndex(const char *text, int start, int from, int to); 
#endif
RAYGUIDEF bool GuiWindowBox(Rectangle bounds, const char *text); 
RAYGUIDEF void GuiGroupBox(Rectangle bounds, const char *text); 
RAYGUIDEF void GuiLine(Rectangle bounds, const char *text); 
RAYGUIDEF void GuiPanel(Rectangle bounds); 
RAYGUIDEF Rectangle GuiScrollPanel(Rectangle bounds, Rectangle content, Vector2 *scroll); 
RAYGUIDEF void GuiLabel(Rectangle bounds, const char *text); 
RAYGUIDEF bool GuiButton(Rectangle bounds, const char *text); 
RAYGUIDEF bool GuiLabelButton(Rectangle bounds, const char *text); 
RAYGUIDEF bool GuiImageButton(Rectangle bounds, Texture2D texture); 
RAYGUIDEF bool GuiImageButtonEx(Rectangle bounds, Texture2D texture, Rectangle texSource, const char *text); 
RAYGUIDEF bool GuiToggle(Rectangle bounds, const char *text, bool active); 
RAYGUIDEF int GuiToggleGroup(Rectangle bounds, const char *text, int active); 
RAYGUIDEF bool GuiCheckBox(Rectangle bounds, const char *text, bool checked); 
RAYGUIDEF int GuiComboBox(Rectangle bounds, const char *text, int active); 
RAYGUIDEF bool GuiDropdownBox(Rectangle bounds, const char *text, int *active, bool editMode); 
RAYGUIDEF bool GuiSpinner(Rectangle bounds, int *value, int minValue, int maxValue, bool editMode); 
RAYGUIDEF bool GuiValueBox(Rectangle bounds, int *value, int minValue, int maxValue, bool editMode); 
RAYGUIDEF bool GuiTextBox(Rectangle bounds, char *text, int textSize, bool editMode); 
RAYGUIDEF bool GuiTextBoxMulti(Rectangle bounds, char *text, int textSize, bool editMode); 
RAYGUIDEF float GuiSlider(Rectangle bounds, const char *text, float value, float minValue, float maxValue, bool showValue); 
RAYGUIDEF float GuiSliderBar(Rectangle bounds, const char *text, float value, float minValue, float maxValue, bool showValue); 
RAYGUIDEF float GuiProgressBar(Rectangle bounds, const char *text, float value, float minValue, float maxValue, bool showValue); 
RAYGUIDEF void GuiStatusBar(Rectangle bounds, const char *text); 
RAYGUIDEF void GuiDummyRec(Rectangle bounds, const char *text); 
RAYGUIDEF int GuiScrollBar(Rectangle bounds, int value, int minValue, int maxValue); 
RAYGUIDEF Vector2 GuiGrid(Rectangle bounds, float spacing, int subdivs); 
RAYGUIDEF bool GuiListView(Rectangle bounds, const char *text, int *active, int *scrollIndex, bool editMode); 
RAYGUIDEF bool GuiListViewEx(Rectangle bounds, const char **text, int count, int *enabled, int *active, int *focus, int *scrollIndex, bool editMode); 
RAYGUIDEF int GuiMessageBox(Rectangle bounds, const char *windowTitle, const char *message, const char *buttons); 
RAYGUIDEF int GuiTextInputBox(Rectangle bounds, const char *windowTitle, const char *message, char *text, const char *buttons); 
RAYGUIDEF Color GuiColorPicker(Rectangle bounds, Color color); 
RAYGUIDEF void GuiLoadStyle(const char *fileName); 
RAYGUIDEF void GuiLoadStyleProps(const int *props, int count); 
RAYGUIDEF void GuiLoadStyleDefault(void); 
RAYGUIDEF void GuiUpdateStyleComplete(void); 
RAYGUIDEF const char *GuiIconText(int iconId, const char *text); 
#if defined(RAYGUI_IMPLEMENTATION)
#if defined(RAYGUI_RICONS_SUPPORT)
#if defined(RAYGUI_STANDALONE)
#define RICONS_STANDALONE
#endif
#define RICONS_IMPLEMENTATION
#include "ricons.h" 
#endif
#include <stdio.h> 
#include <string.h> 
#if defined(RAYGUI_STANDALONE)
#include <stdarg.h> 
#endif
#if defined(__cplusplus)
#define RAYGUI_CLITERAL(name) name
#else
#define RAYGUI_CLITERAL(name) (name)
#endif
typedef enum { BORDER = 0, BASE, TEXT, OTHER } GuiPropertyElement;
static GuiControlState guiState = GUI_STATE_NORMAL;
static Font guiFont = { 0 }; 
static bool guiLocked = false;
static float guiAlpha = 1.0f;
static unsigned int guiStyle[NUM_CONTROLS*(NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED)] = { 0 };
static bool guiStyleLoaded = false;
#if defined(RAYGUI_TEXTBOX_EXTENDED)
static Rectangle guiTextBoxActive = { 0 }; 
static GuiTextBoxState guiTextBoxState = { .cursor = -1, .start = 0, .index = 0, .select = -1 }; 
#endif
#if defined(RAYGUI_STANDALONE)
#define KEY_RIGHT 262
#define KEY_LEFT 263
#define KEY_DOWN 264
#define KEY_UP 265
#define KEY_BACKSPACE 259
#define KEY_ENTER 257
#define MOUSE_LEFT_BUTTON 0
static Vector2 GetMousePosition(void);
static int GetMouseWheelMove(void);
static bool IsMouseButtonDown(int button);
static bool IsMouseButtonPressed(int button);
static bool IsMouseButtonReleased(int button);
static bool IsKeyDown(int key);
static bool IsKeyPressed(int key);
static int GetKeyPressed(void); 
static void DrawRectangle(int x, int y, int width, int height, Color color);
static void DrawRectangleGradientEx(Rectangle rec, Color col1, Color col2, Color col3, Color col4); 
static void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color); 
static void DrawTextureRec(Texture2D texture, Rectangle sourceRec, Vector2 position, Color tint); 
static Font GetFontDefault(void); 
static Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing); 
static void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); 
static Font LoadFontEx(const char *fileName, int fontSize, int *fontChars, int charsCount); 
static Color GetColor(int hexValue); 
static int ColorToInt(Color color); 
static Color Fade(Color color, float alpha); 
static bool CheckCollisionPointRec(Vector2 point, Rectangle rec); 
static const char *TextFormat(const char *text, ...); 
static void DrawRectangleRec(Rectangle rec, Color color); 
static void DrawRectangleLinesEx(Rectangle rec, int lineThick, Color color); 
static void DrawRectangleGradientV(int posX, int posY, int width, int height, Color color1, Color color2); 
#endif 
static bool GuiListElement(Rectangle bounds, const char *text, bool active, bool editMode);
static Vector3 ConvertHSVtoRGB(Vector3 hsv); 
static Vector3 ConvertRGBtoHSV(Vector3 rgb); 
static int GetTextWidth(const char *text) 
{
Vector2 size = { 0 };
if ((text != NULL) && (text[0] != '\0')) size = MeasureTextEx(guiFont, text, GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING));
return (int)size.x;
}
static Rectangle GetTextBounds(int control, Rectangle bounds)
{
Rectangle textBounds = { 0 };
textBounds.x = bounds.x + GuiGetStyle(control, BORDER_WIDTH) + GuiGetStyle(control, INNER_PADDING);
textBounds.y = bounds.y + GuiGetStyle(control, BORDER_WIDTH) + GuiGetStyle(control, INNER_PADDING);
textBounds.width = bounds.width - 2*(GuiGetStyle(control, BORDER_WIDTH) + GuiGetStyle(control, INNER_PADDING));
textBounds.height = bounds.height - 2*(GuiGetStyle(control, BORDER_WIDTH) + GuiGetStyle(control, INNER_PADDING));
switch (control)
{
case COMBOBOX: bounds.width -= (GuiGetStyle(control, SELECTOR_WIDTH) + GuiGetStyle(control, SELECTOR_PADDING)); break;
case CHECKBOX: bounds.x += (bounds.width + GuiGetStyle(control, CHECK_TEXT_PADDING)); break;
default: break;
}
return textBounds;
}
static const char *GetTextIcon(const char *text, int *iconId)
{
#if defined(RAYGUI_RICONS_SUPPORT)
if (text[0] == '#') 
{
char iconValue[4] = { 0 };
int i = 1;
for (i = 1; i < 4; i++)
{
if ((text[i] != '#') && (text[i] != '\0')) iconValue[i - 1] = text[i];
else break;
}
iconValue[3] = '\0';
*iconId = atoi(iconValue);
if (*iconId > 0) text += (i + 1);
}
#endif
return text;
}
static void GuiDrawText(const char *text, Rectangle bounds, int alignment, Color tint)
{
#define VALIGN_OFFSET(h) ((int)h%2) 
if ((text != NULL) && (text[0] != '\0'))
{
int iconId = 0;
text = GetTextIcon(text, &iconId); 
#define ICON_TEXT_PADDING 4
Vector2 position = { bounds.x, bounds.y };
int textWidth = GetTextWidth(text);
int textHeight = GuiGetStyle(DEFAULT, TEXT_SIZE);
#if defined(RAYGUI_RICONS_SUPPORT)
if (iconId > 0)
{
textWidth += RICONS_SIZE;
if ((text != NULL) && (text[0] != '\0')) textWidth += ICON_TEXT_PADDING;
}
#endif
switch (alignment)
{
case GUI_TEXT_ALIGN_LEFT:
{
position.x = bounds.x;
position.y = bounds.y + bounds.height/2 - textHeight/2 + VALIGN_OFFSET(bounds.height);
} break;
case GUI_TEXT_ALIGN_CENTER:
{
position.x = bounds.x + bounds.width/2 - textWidth/2;
position.y = bounds.y + bounds.height/2 - textHeight/2 + VALIGN_OFFSET(bounds.height);
} break;
case GUI_TEXT_ALIGN_RIGHT:
{
position.x = bounds.x + bounds.width - textWidth;
position.y = bounds.y + bounds.height/2 - textHeight/2 + VALIGN_OFFSET(bounds.height);
} break;
default: break;
}
#if defined(RAYGUI_RICONS_SUPPORT)
#define ICON_TEXT_PADDING 4
if (iconId > 0)
{
DrawIcon(iconId, RAYGUI_CLITERAL(Vector2){ position.x, bounds.y + bounds.height/2 - RICONS_SIZE/2 + VALIGN_OFFSET(bounds.height) }, 1, tint);
position.x += (RICONS_SIZE + ICON_TEXT_PADDING);
}
#endif
DrawTextEx(guiFont, text, position, GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), tint);
}
}
static const char **GuiTextSplit(const char *text, int *count, int *textRow);
RAYGUIDEF void GuiEnable(void) { guiState = GUI_STATE_NORMAL; }
RAYGUIDEF void GuiDisable(void) { guiState = GUI_STATE_DISABLED; }
RAYGUIDEF void GuiLock(void) { guiLocked = true; }
RAYGUIDEF void GuiUnlock(void) { guiLocked = false; }
RAYGUIDEF void GuiState(int state) { guiState = (GuiControlState)state; }
RAYGUIDEF void GuiFont(Font font)
{
if (font.texture.id > 0)
{
guiFont = font;
GuiSetStyle(DEFAULT, TEXT_SIZE, font.baseSize);
}
}
RAYGUIDEF void GuiFade(float alpha)
{
if (alpha < 0.0f) alpha = 0.0f;
else if (alpha > 1.0f) alpha = 1.0f;
guiAlpha = alpha;
}
RAYGUIDEF void GuiSetStyle(int control, int property, int value)
{
if (!guiStyleLoaded) GuiLoadStyleDefault();
guiStyle[control*(NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED) + property] = value;
}
RAYGUIDEF int GuiGetStyle(int control, int property)
{
if (!guiStyleLoaded) GuiLoadStyleDefault();
return guiStyle[control*(NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED) + property];
}
#if defined(RAYGUI_TEXTBOX_EXTENDED)
RAYGUIDEF void GuiTextBoxSetActive(Rectangle bounds) 
{
guiTextBoxActive = bounds;
guiTextBoxState = (GuiTextBoxState){ .cursor = -1, .start = 0, .index = 0, .select = -1 };
}
RAYGUIDEF Rectangle GuiTextBoxGetActive(void) { return guiTextBoxActive; }
RAYGUIDEF void GuiTextBoxSetCursor(int cursor) 
{ 
guiTextBoxState.cursor = (cursor < 0) ? -1 : cursor;
guiTextBoxState.start = -1; 
}
RAYGUIDEF int GuiTextBoxGetCursor(void) { return guiTextBoxState.cursor; }
RAYGUIDEF void GuiTextBoxSetSelection(int start, int length) 
{
if(start < 0) start = 0;
if(length < 0) length = 0;
GuiTextBoxSetCursor(start + length);
guiTextBoxState.select = start;
}
RAYGUIDEF Vector2 GuiTextBoxGetSelection(void)
{
if(guiTextBoxState.select == -1 || guiTextBoxState.select == guiTextBoxState.cursor) 
return RAYGUI_CLITERAL(Vector2){ 0 };
else if(guiTextBoxState.cursor > guiTextBoxState.select) 
return RAYGUI_CLITERAL(Vector2){ guiTextBoxState.select, guiTextBoxState.cursor - guiTextBoxState.select };
return RAYGUI_CLITERAL(Vector2){ guiTextBoxState.cursor, guiTextBoxState.select - guiTextBoxState.cursor };
}
RAYGUIDEF bool GuiTextBoxIsActive(Rectangle bounds)
{
return (bounds.x == guiTextBoxActive.x && bounds.y == guiTextBoxActive.y && 
bounds.width == guiTextBoxActive.width && bounds.height == guiTextBoxActive.height);
}
RAYGUIDEF GuiTextBoxState GuiTextBoxGetState(void) { return guiTextBoxState; }
RAYGUIDEF void GuiTextBoxSetState(GuiTextBoxState state) 
{ 
guiTextBoxState = state;
}
#endif
RAYGUIDEF bool GuiWindowBox(Rectangle bounds, const char *text)
{
#define WINDOW_CLOSE_BUTTON_PADDING 2
#define WINDOW_STATUSBAR_HEIGHT 24
GuiControlState state = guiState;
bool clicked = false;
Rectangle statusBar = { bounds.x, bounds.y, bounds.width, WINDOW_STATUSBAR_HEIGHT };
if (bounds.height < WINDOW_STATUSBAR_HEIGHT*2) bounds.height = WINDOW_STATUSBAR_HEIGHT*2;
Rectangle buttonRec = { statusBar.x + statusBar.width - GuiGetStyle(DEFAULT, BORDER_WIDTH) - WINDOW_CLOSE_BUTTON_PADDING - 20,
statusBar.y + GuiGetStyle(DEFAULT, BORDER_WIDTH) + WINDOW_CLOSE_BUTTON_PADDING, 18, 18 };
DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DEFAULT, BORDER + (state*3))), guiAlpha));
DrawRectangleRec(RAYGUI_CLITERAL(Rectangle){ bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH),
bounds.width - GuiGetStyle(DEFAULT, BORDER_WIDTH)*2, bounds.height - GuiGetStyle(DEFAULT, BORDER_WIDTH)*2 },
Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), guiAlpha));
int defaultPadding = GuiGetStyle(DEFAULT, INNER_PADDING);
int defaultTextAlign = GuiGetStyle(DEFAULT, TEXT_ALIGNMENT);
GuiSetStyle(DEFAULT, INNER_PADDING, 8);
GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
GuiStatusBar(statusBar, text);
GuiSetStyle(DEFAULT, INNER_PADDING, defaultPadding);
GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, defaultTextAlign);
int tempBorderWidth = GuiGetStyle(BUTTON, BORDER_WIDTH);
int tempTextAlignment = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
#if defined(RAYGUI_RICONS_SUPPORT)
clicked = GuiButton(buttonRec, GuiIconText(RICON_CROSS_SMALL, NULL));
#else
clicked = GuiButton(buttonRec, "x");
#endif
GuiSetStyle(BUTTON, BORDER_WIDTH, tempBorderWidth);
GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlignment);
return clicked;
}
RAYGUIDEF void GuiGroupBox(Rectangle bounds, const char *text)
{
#define GROUPBOX_LINE_THICK 1
#define GROUPBOX_TEXT_PADDING 10
#define GROUPBOX_PADDING 2
GuiControlState state = guiState;
DrawRectangle(bounds.x, bounds.y, GROUPBOX_LINE_THICK, bounds.height, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha));
DrawRectangle(bounds.x, bounds.y + bounds.height - 1, bounds.width, GROUPBOX_LINE_THICK, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha));
DrawRectangle(bounds.x + bounds.width - 1, bounds.y, GROUPBOX_LINE_THICK, bounds.height, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha));
GuiLine(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y, bounds.width, 1 }, text);
}
RAYGUIDEF void GuiLine(Rectangle bounds, const char *text)
{
#define LINE_THICK 1
#define LINE_TEXT_PADDING 10
#define LINE_TEXT_SPACING 2
GuiControlState state = guiState;
Color color = Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha);
if (text == NULL) DrawRectangle(bounds.x, bounds.y + bounds.height/2, bounds.width, 1, color);
else
{
Rectangle textBounds = { 0 };
textBounds.width = GetTextWidth(text) + 2*LINE_TEXT_SPACING; 
textBounds.height = GuiGetStyle(DEFAULT, TEXT_SIZE);
textBounds.x = bounds.x + LINE_TEXT_PADDING + LINE_TEXT_SPACING;
textBounds.y = bounds.y - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;
DrawRectangle(bounds.x, bounds.y, LINE_TEXT_PADDING, 1, color);
GuiLabel(textBounds, text);
DrawRectangle(bounds.x + textBounds.width + LINE_TEXT_PADDING + 2*LINE_TEXT_SPACING, bounds.y, bounds.width - (textBounds.width + LINE_TEXT_PADDING + 2*LINE_TEXT_SPACING), 1, color);
}
}
RAYGUIDEF void GuiPanel(Rectangle bounds)
{
#define PANEL_BORDER_WIDTH 1
GuiControlState state = guiState;
DrawRectangleRec(bounds, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BASE_COLOR_DISABLED : BACKGROUND_COLOR)), guiAlpha));
DrawRectangleLinesEx(bounds, PANEL_BORDER_WIDTH, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED: LINE_COLOR)), guiAlpha));
}
RAYGUIDEF Rectangle GuiScrollPanel(Rectangle bounds, Rectangle content, Vector2 *scroll)
{
GuiControlState state = guiState;
Vector2 scrollPos = { 0.0f, 0.0f };
if (scroll != NULL) scrollPos = *scroll;
bool hasHorizontalScrollBar = (content.width > bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH))? true : false;
bool hasVerticalScrollBar = (content.height > bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH))? true : false;
if (!hasHorizontalScrollBar) hasHorizontalScrollBar = (hasVerticalScrollBar && (content.width > (bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH))))? true : false;
if (!hasVerticalScrollBar) hasVerticalScrollBar = (hasHorizontalScrollBar && (content.height > (bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH))))? true : false;
const int horizontalScrollBarWidth = hasHorizontalScrollBar? GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH) : 0;
const int verticalScrollBarWidth = hasVerticalScrollBar? GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH) : 0;
const Rectangle horizontalScrollBar = { (float)((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE) ? (float)bounds.x + verticalScrollBarWidth : (float)bounds.x) + GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)bounds.y + bounds.height - horizontalScrollBarWidth - GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)bounds.width - verticalScrollBarWidth - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)horizontalScrollBarWidth };
const Rectangle verticalScrollBar = { (float)((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE) ? (float)bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH) : (float)bounds.x + bounds.width - verticalScrollBarWidth - GuiGetStyle(DEFAULT, BORDER_WIDTH)), (float)bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)verticalScrollBarWidth, (float)bounds.height - horizontalScrollBarWidth - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) };
Rectangle view = (GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)?
RAYGUI_CLITERAL(Rectangle){ bounds.x + verticalScrollBarWidth + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth, bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth } :
RAYGUI_CLITERAL(Rectangle){ bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth, bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth };
if (view.width > content.width) view.width = content.width;
if (view.height > content.height) view.height = content.height;
const int horizontalMin = hasHorizontalScrollBar? ((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? -verticalScrollBarWidth : 0) - GuiGetStyle(DEFAULT, BORDER_WIDTH) : ((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? -verticalScrollBarWidth : 0) - GuiGetStyle(DEFAULT, BORDER_WIDTH);
const int horizontalMax = hasHorizontalScrollBar? content.width - bounds.width + verticalScrollBarWidth + GuiGetStyle(DEFAULT, BORDER_WIDTH) - ((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? verticalScrollBarWidth : 0) : -GuiGetStyle(DEFAULT, BORDER_WIDTH);
const int verticalMin = hasVerticalScrollBar? -GuiGetStyle(DEFAULT, BORDER_WIDTH) : -GuiGetStyle(DEFAULT, BORDER_WIDTH);
const int verticalMax = hasVerticalScrollBar? content.height - bounds.height + horizontalScrollBarWidth + GuiGetStyle(DEFAULT, BORDER_WIDTH) : -GuiGetStyle(DEFAULT, BORDER_WIDTH);
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
else state = GUI_STATE_FOCUSED;
if (hasHorizontalScrollBar)
{
if (IsKeyDown(KEY_RIGHT)) scrollPos.x -= GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
if (IsKeyDown(KEY_LEFT)) scrollPos.x += GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
}
if (hasVerticalScrollBar)
{
if (IsKeyDown(KEY_DOWN)) scrollPos.y -= GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
if (IsKeyDown(KEY_UP)) scrollPos.y += GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
}
scrollPos.y += GetMouseWheelMove()*20;
}
}
if (scrollPos.x > -horizontalMin) scrollPos.x = -horizontalMin;
if (scrollPos.x < -horizontalMax) scrollPos.x = -horizontalMax;
if (scrollPos.y > -verticalMin) scrollPos.y = -verticalMin;
if (scrollPos.y < -verticalMax) scrollPos.y = -verticalMax;
DrawRectangleRec(bounds, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); 
const int slider = GuiGetStyle(SCROLLBAR, SLIDER_SIZE);
if (hasHorizontalScrollBar)
{
GuiSetStyle(SCROLLBAR, SLIDER_SIZE, ((bounds.width - 2 * GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth)/content.width)*(bounds.width - 2 * GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth));
scrollPos.x = -GuiScrollBar(horizontalScrollBar, -scrollPos.x, horizontalMin, horizontalMax);
}
if (hasVerticalScrollBar)
{
GuiSetStyle(SCROLLBAR, SLIDER_SIZE, ((bounds.height - 2 * GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth)/content.height)* (bounds.height - 2 * GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth));
scrollPos.y = -GuiScrollBar(verticalScrollBar, -scrollPos.y, verticalMin, verticalMax);
}
if (hasHorizontalScrollBar && hasVerticalScrollBar)
{
DrawRectangle(horizontalScrollBar.x + horizontalScrollBar.width + 2,
verticalScrollBar.y + verticalScrollBar.height + 2,
horizontalScrollBarWidth - 4, verticalScrollBarWidth - 4,
Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT + (state * 3))), guiAlpha));
}
GuiSetStyle(SCROLLBAR, SLIDER_SIZE, slider);
DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, (float)BORDER + (state*3))), guiAlpha));
if (scroll != NULL) *scroll = scrollPos;
return view;
}
RAYGUIDEF void GuiLabel(Rectangle bounds, const char *text)
{
GuiControlState state = guiState;
GuiDrawText(text, GetTextBounds(LABEL, bounds), GuiGetStyle(LABEL, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LABEL, (state == GUI_STATE_DISABLED)? TEXT_COLOR_DISABLED : TEXT_COLOR_NORMAL)), guiAlpha));
}
RAYGUIDEF bool GuiButton(Rectangle bounds, const char *text)
{
GuiControlState state = guiState;
bool pressed = false;
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
else state = GUI_STATE_FOCUSED;
if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) pressed = true;
}
}
DrawRectangleLinesEx(bounds, GuiGetStyle(BUTTON, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(BUTTON, BORDER + (state*3))), guiAlpha));
DrawRectangle(bounds.x + GuiGetStyle(BUTTON, BORDER_WIDTH), bounds.y + GuiGetStyle(BUTTON, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(BUTTON, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(BUTTON, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(BUTTON, BASE + (state*3))), guiAlpha));
GuiDrawText(text, GetTextBounds(BUTTON, bounds), GuiGetStyle(BUTTON, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(BUTTON, TEXT + (state*3))), guiAlpha));
return pressed;
}
RAYGUIDEF bool GuiLabelButton(Rectangle bounds, const char *text)
{
GuiControlState state = guiState;
bool pressed = false;
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
else state = GUI_STATE_FOCUSED;
if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) pressed = true;
}
}
GuiDrawText(text, GetTextBounds(LABEL, bounds), GuiGetStyle(LABEL, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state*3))), guiAlpha));
return pressed;
}
RAYGUIDEF bool GuiImageButton(Rectangle bounds, Texture2D texture)
{
return GuiImageButtonEx(bounds, texture, RAYGUI_CLITERAL(Rectangle){ 0, 0, (float)texture.width, (float)texture.height }, NULL);
}
RAYGUIDEF bool GuiImageButtonEx(Rectangle bounds, Texture2D texture, Rectangle texSource, const char *text)
{
GuiControlState state = guiState;
bool clicked = false;
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) clicked = true;
else state = GUI_STATE_FOCUSED;
}
}
DrawRectangleLinesEx(bounds, GuiGetStyle(BUTTON, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(BUTTON, BORDER + (state*3))), guiAlpha));
DrawRectangle(bounds.x + GuiGetStyle(BUTTON, BORDER_WIDTH), bounds.y + GuiGetStyle(BUTTON, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(BUTTON, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(BUTTON, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(BUTTON, BASE + (state*3))), guiAlpha));
if (text != NULL) GuiDrawText(text, GetTextBounds(BUTTON, bounds), GuiGetStyle(BUTTON, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(BUTTON, TEXT + (state*3))), guiAlpha));
if (texture.id > 0) DrawTextureRec(texture, texSource, RAYGUI_CLITERAL(Vector2){ bounds.x + bounds.width/2 - (texSource.width + GuiGetStyle(BUTTON, INNER_PADDING)/2)/2, bounds.y + bounds.height/2 - texSource.height/2 }, Fade(GetColor(GuiGetStyle(BUTTON, TEXT + (state*3))), guiAlpha));
return clicked;
}
RAYGUIDEF bool GuiToggle(Rectangle bounds, const char *text, bool active)
{
GuiControlState state = guiState;
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
{
state = GUI_STATE_NORMAL;
active = !active;
}
else state = GUI_STATE_FOCUSED;
}
}
if (state == GUI_STATE_NORMAL)
{
DrawRectangleLinesEx(bounds, GuiGetStyle(TOGGLE, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TOGGLE, (active? BORDER_COLOR_PRESSED : (BORDER + state*3)))), guiAlpha));
DrawRectangle(bounds.x + GuiGetStyle(TOGGLE, BORDER_WIDTH), bounds.y + GuiGetStyle(TOGGLE, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TOGGLE, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TOGGLE, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TOGGLE, (active? BASE_COLOR_PRESSED : (BASE + state*3)))), guiAlpha));
GuiDrawText(text, GetTextBounds(TOGGLE, bounds), GuiGetStyle(TOGGLE, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TOGGLE, (active? TEXT_COLOR_PRESSED : (TEXT + state*3)))), guiAlpha));
}
else
{
DrawRectangleLinesEx(bounds, GuiGetStyle(TOGGLE, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TOGGLE, BORDER + state*3)), guiAlpha));
DrawRectangle(bounds.x + GuiGetStyle(TOGGLE, BORDER_WIDTH), bounds.y + GuiGetStyle(TOGGLE, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TOGGLE, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TOGGLE, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TOGGLE, BASE + state*3)), guiAlpha));
GuiDrawText(text, GetTextBounds(TOGGLE, bounds), GuiGetStyle(TOGGLE, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TOGGLE, TEXT + state*3)), guiAlpha));
}
return active;
}
RAYGUIDEF int GuiToggleGroup(Rectangle bounds, const char *text, int active)
{
float initBoundsX = bounds.x;
int rows[64] = { 0 };
int elementsCount = 0;
const char **elementsPtrs = GuiTextSplit(text, &elementsCount, rows);
int prevRow = rows[0];
for (int i = 0; i < elementsCount; i++)
{
if (prevRow != rows[i])
{
bounds.x = initBoundsX;
bounds.y += (bounds.height + GuiGetStyle(TOGGLE, GROUP_PADDING));
prevRow = rows[i];
}
if (i == active) GuiToggle(bounds, elementsPtrs[i], true);
else if (GuiToggle(bounds, elementsPtrs[i], false) == true) active = i;
bounds.x += (bounds.width + GuiGetStyle(TOGGLE, GROUP_PADDING));
}
return active;
}
RAYGUIDEF bool GuiCheckBox(Rectangle bounds, const char *text, bool checked)
{
GuiControlState state = guiState;
Rectangle textBounds = { 0 };
textBounds.x = bounds.x + bounds.width + GuiGetStyle(CHECKBOX, CHECK_TEXT_PADDING);
textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;
textBounds.width = GetTextWidth(text); 
textBounds.height = GuiGetStyle(DEFAULT, TEXT_SIZE);
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y, bounds.width + textBounds.width + GuiGetStyle(CHECKBOX, CHECK_TEXT_PADDING), bounds.height }))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
else state = GUI_STATE_FOCUSED;
if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) checked = !checked;
}
}
DrawRectangleLinesEx(bounds, GuiGetStyle(CHECKBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(CHECKBOX, BORDER + (state*3))), guiAlpha));
if (checked) DrawRectangle(bounds.x + GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, INNER_PADDING),
bounds.y + GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, INNER_PADDING),
bounds.width - 2*(GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, INNER_PADDING)),
bounds.height - 2*(GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, INNER_PADDING)),
Fade(GetColor(GuiGetStyle(CHECKBOX, TEXT + state*3)), guiAlpha));
GuiDrawText(text, textBounds, GUI_TEXT_ALIGN_LEFT, Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state*3))), guiAlpha));
return checked;
}
RAYGUIDEF int GuiComboBox(Rectangle bounds, const char *text, int active)
{
GuiControlState state = guiState;
bounds.width -= (GuiGetStyle(COMBOBOX, SELECTOR_WIDTH) + GuiGetStyle(COMBOBOX, SELECTOR_PADDING));
Rectangle selector = { (float)bounds.x + bounds.width + GuiGetStyle(COMBOBOX, SELECTOR_PADDING),
(float)bounds.y, (float)GuiGetStyle(COMBOBOX, SELECTOR_WIDTH), (float)bounds.height };
int elementsCount = 0;
const char **elementsPtrs = GuiTextSplit(text, &elementsCount, NULL);
if (active < 0) active = 0;
else if (active > elementsCount - 1) active = elementsCount - 1;
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds) ||
CheckCollisionPointRec(mousePoint, selector))
{
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
active += 1;
if (active >= elementsCount) active = 0;
}
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
else state = GUI_STATE_FOCUSED;
}
}
DrawRectangleLinesEx(bounds, GuiGetStyle(COMBOBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(COMBOBOX, BORDER + (state*3))), guiAlpha));
DrawRectangle(bounds.x + GuiGetStyle(COMBOBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(COMBOBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(COMBOBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(COMBOBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(COMBOBOX, BASE + (state*3))), guiAlpha));
GuiDrawText(elementsPtrs[active], GetTextBounds(COMBOBOX, bounds), GuiGetStyle(COMBOBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(COMBOBOX, TEXT + (state*3))), guiAlpha));
int tempBorderWidth = GuiGetStyle(BUTTON, BORDER_WIDTH);
int tempTextAlign = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
GuiButton(selector, TextFormat("%i/%i", active + 1, elementsCount));
GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlign);
GuiSetStyle(BUTTON, BORDER_WIDTH, tempBorderWidth);
return active;
}
RAYGUIDEF bool GuiDropdownBox(Rectangle bounds, const char *text, int *active, bool editMode)
{
GuiControlState state = guiState;
int elementsCount = 0;
const char **elementsPtrs = GuiTextSplit(text, &elementsCount, NULL);
bool pressed = false;
int auxActive = *active;
Rectangle closeBounds = bounds;
Rectangle openBounds = bounds;
openBounds.height *= (elementsCount + 1);
if (guiLocked && editMode) guiLocked = false;
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (editMode) state = GUI_STATE_PRESSED;
if (!editMode)
{
if (CheckCollisionPointRec(mousePoint, closeBounds))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;
else state = GUI_STATE_FOCUSED;
}
}
else
{
if (CheckCollisionPointRec(mousePoint, closeBounds))
{
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;
}
else if (!CheckCollisionPointRec(mousePoint, openBounds))
{
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) pressed = true;
}
}
}
int tempTextAlign = GuiGetStyle(DEFAULT, TEXT_ALIGNMENT);
GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT));
switch (state)
{
case GUI_STATE_NORMAL:
{
DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_NORMAL)), guiAlpha));
DrawRectangleLinesEx(bounds, GuiGetStyle(DROPDOWNBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_NORMAL)), guiAlpha));
GuiListElement(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y, bounds.width, bounds.height }, elementsPtrs[auxActive], false, false);
} break;
case GUI_STATE_FOCUSED:
{
GuiListElement(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y, bounds.width, bounds.height }, elementsPtrs[auxActive], false, editMode);
} break;
case GUI_STATE_PRESSED:
{
if (!editMode) GuiListElement(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y, bounds.width, bounds.height }, elementsPtrs[auxActive], true, true);
if (editMode)
{
GuiPanel(openBounds);
GuiListElement(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y, bounds.width, bounds.height }, elementsPtrs[auxActive], true, true);
for (int i = 0; i < elementsCount; i++)
{
if (i == auxActive && editMode)
{
if (GuiListElement(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y + bounds.height*(i + 1) + GuiGetStyle(DROPDOWNBOX, INNER_PADDING),
bounds.width, bounds.height - GuiGetStyle(DROPDOWNBOX, INNER_PADDING) },
elementsPtrs[i], true, true) == false) pressed = true;
}
else
{
if (GuiListElement(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y + bounds.height*(i+1) + GuiGetStyle(DROPDOWNBOX, INNER_PADDING),
bounds.width, bounds.height - GuiGetStyle(DROPDOWNBOX, INNER_PADDING) },
elementsPtrs[i], false, true))
{
auxActive = i;
pressed = true;
}
}
}
}
} break;
case GUI_STATE_DISABLED:
{
DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_DISABLED)), guiAlpha));
DrawRectangleLinesEx(bounds, GuiGetStyle(DROPDOWNBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_DISABLED)), guiAlpha));
GuiListElement(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y, bounds.width, bounds.height }, elementsPtrs[auxActive], false, false);
} break;
default: break;
}
GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, tempTextAlign);
DrawTriangle(RAYGUI_CLITERAL(Vector2){ bounds.x + bounds.width - GuiGetStyle(DROPDOWNBOX, ARROW_RIGHT_PADDING), bounds.y + bounds.height/2 - 2 },
RAYGUI_CLITERAL(Vector2){ bounds.x + bounds.width - GuiGetStyle(DROPDOWNBOX, ARROW_RIGHT_PADDING) + 5, bounds.y + bounds.height/2 - 2 + 5 },
RAYGUI_CLITERAL(Vector2){ bounds.x + bounds.width - GuiGetStyle(DROPDOWNBOX, ARROW_RIGHT_PADDING) + 10, bounds.y + bounds.height/2 - 2 },
Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + (state*3))), guiAlpha));
*active = auxActive;
return pressed;
}
#if defined(RAYGUI_TEXTBOX_EXTENDED)
RAYGUIDEF bool GuiSpinner(Rectangle bounds, int *value, int minValue, int maxValue, bool editMode)
{
#define GUI_SPINNER_HOLD_SPEED 0.2f 
static float timer = 0.0f;
int tempValue = *value;
const float time = GetTime(); 
bool pressed = false, active = GuiTextBoxIsActive(bounds);
Rectangle spinner = { bounds.x + GuiGetStyle(SPINNER, SELECT_BUTTON_WIDTH) + GuiGetStyle(SPINNER, SELECT_BUTTON_PADDING), bounds.y,
bounds.width - 2*(GuiGetStyle(SPINNER, SELECT_BUTTON_WIDTH) + GuiGetStyle(SPINNER, SELECT_BUTTON_PADDING)), bounds.height };
Rectangle leftButtonBound = { bounds.x, bounds.y, GuiGetStyle(SPINNER, SELECT_BUTTON_WIDTH), bounds.height };
Rectangle rightButtonBound = { bounds.x + bounds.width - GuiGetStyle(SPINNER, SELECT_BUTTON_WIDTH), bounds.y, GuiGetStyle(SPINNER, SELECT_BUTTON_WIDTH), bounds.height };
Vector2 mouse = GetMousePosition();
if (tempValue < minValue) tempValue = minValue;
if (tempValue > maxValue) tempValue = maxValue;
if (editMode) 
{
if (!active)
{
if ((IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonDown(MOUSE_LEFT_BUTTON)) && 
CheckCollisionPointRec(mouse, bounds))
{
GuiTextBoxSetActive(bounds);
active = true;
}
}
}
if ((CheckCollisionPointRec(mouse, leftButtonBound) || CheckCollisionPointRec(mouse, rightButtonBound)) && 
IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
timer = time;
}
if (GuiTextBoxIsActive(bounds)) guiTextBoxActive = spinner; 
pressed = GuiValueBox(spinner, &tempValue, minValue, maxValue, editMode);
if (GuiTextBoxIsActive(spinner)) guiTextBoxActive = bounds; 
int tempBorderWidth = GuiGetStyle(BUTTON, BORDER_WIDTH);
GuiSetStyle(BUTTON, BORDER_WIDTH, GuiGetStyle(SPINNER, BORDER_WIDTH));
int tempTextAlign = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
char *icon = "<";
#if defined(RAYGUI_RICONS_SUPPORT)
icon = (char *)GuiIconText(RICON_ARROW_LEFT_FILL, NULL);
#endif
if (GuiButton(leftButtonBound, icon) || 
(IsMouseButtonDown(MOUSE_LEFT_BUTTON) && 
CheckCollisionPointRec(mouse, leftButtonBound) && 
(time - timer) > GUI_SPINNER_HOLD_SPEED))
{
tempValue--;
}
icon = ">";
#if defined(RAYGUI_RICONS_SUPPORT)
icon = (char *)GuiIconText(RICON_ARROW_RIGHT_FILL, NULL);
#endif 
if (GuiButton(rightButtonBound, icon) || 
(IsMouseButtonDown(MOUSE_LEFT_BUTTON) && 
CheckCollisionPointRec(mouse, rightButtonBound) && 
(time - timer) > GUI_SPINNER_HOLD_SPEED))
{
tempValue++;
}
GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlign);
GuiSetStyle(BUTTON, BORDER_WIDTH, tempBorderWidth);
if (tempValue < minValue) tempValue = minValue;
if (tempValue > maxValue) tempValue = maxValue;
if (active && (((time - timer) > GUI_SPINNER_HOLD_SPEED) || (timer == 0.0f) || (timer > time))) timer = time;
*value = tempValue;
return pressed;
}
RAYGUIDEF bool GuiValueBox(Rectangle bounds, int *value, int minValue, int maxValue, bool editMode)
{
#define VALUEBOX_MAX_CHARS 32
char text[VALUEBOX_MAX_CHARS + 1] = { 0 };
sprintf(text, "%i", *value);
bool pressed = GuiTextBox(bounds, text, VALUEBOX_MAX_CHARS, editMode);
*value = atoi(text);
if (*value > maxValue) *value = maxValue;
else if (*value < minValue) *value = minValue;
return pressed;
}
enum {
GUI_MEASURE_MODE_CURSOR_END = 0xA,
GUI_MEASURE_MODE_CURSOR_POS,
GUI_MEASURE_MODE_CURSOR_COORDS,
};
static int GuiMeasureTextBox(const char *text, int length, Rectangle rec, int *pos, int mode)
{
const Font font = guiFont;
const float fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
const float spacing = GuiGetStyle(DEFAULT, TEXT_SPACING);
int textOffsetX = 0; 
float scaleFactor = 0.0f;
int letter = 0; 
int index = 0; 
scaleFactor = fontSize/font.baseSize;
int i = 0, k = 0;
int glyphWidth = 0;
for (i = 0; i < length; i++, k++)
{
glyphWidth = 0;
int next = 1;
letter = GetNextCodepoint(&text[i], &next);
if (letter == 0x3f) next = 1; 
index = GetGlyphIndex(font, letter);
i += next - 1;
if (letter != '\n')
{
glyphWidth = (font.chars[index].advanceX == 0)?
(int)(font.chars[index].rec.width*scaleFactor + spacing):
(int)(font.chars[index].advanceX*scaleFactor + spacing);
if ((textOffsetX + glyphWidth + 1) >= rec.width) break;
if ((mode == GUI_MEASURE_MODE_CURSOR_POS) && (*pos == k)) break;
else if (mode == GUI_MEASURE_MODE_CURSOR_COORDS)
{
Rectangle grec = {rec.x + textOffsetX - 1, rec.y, glyphWidth, (font.baseSize + font.baseSize/2)*scaleFactor - 1 };
Vector2 mouse = GetMousePosition();
if (CheckCollisionPointRec(mouse, grec)) 
{
if (mouse.x > (grec.x + glyphWidth/2)) 
{
textOffsetX += glyphWidth;
k++;
}
break;
}
}
}
else break;
textOffsetX += glyphWidth;
}
*pos = k;
return (rec.x + textOffsetX - 1);
}
static int GetPrevCodepoint(const char *text, const char *start, int *prev) 
{
int c = 0x3f;
char *p = (char *)text;
*prev = 1;
for (int i = 0; (p >= start) && (i < 4); p--, i++)
{
if ((((unsigned char)*p) >> 6) != 2)
{
c = GetNextCodepoint(p, prev);
break;
}
}
return c;
}
static int GuiMeasureTextBoxRev(const char *text, int length, Rectangle rec, int *pos)
{
const Font font = guiFont;
const float fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
const float spacing = GuiGetStyle(DEFAULT, TEXT_SPACING);
int textOffsetX = 0; 
float scaleFactor = 0.0f;
int letter = 0; 
int index = 0; 
scaleFactor = fontSize/font.baseSize;
int i = 0, k = 0;
int glyphWidth = 0, prev = 1;
for (i = length; i >= 0; i--, k++)
{
glyphWidth = 0;
letter = GetPrevCodepoint(&text[i], &text[0], &prev);
if (letter == 0x3f) prev = 1; 
index = GetGlyphIndex(font, letter);
i -= prev - 1;
if (letter != '\n')
{
glyphWidth = (font.chars[index].advanceX == 0)?
(int)(font.chars[index].rec.width*scaleFactor + spacing):
(int)(font.chars[index].advanceX*scaleFactor + spacing);
if ((textOffsetX + glyphWidth + 1) >= rec.width) break;
}
else break;
textOffsetX += glyphWidth;
}
*pos = k;
return (i + prev);
}
static inline int GuiTextBoxGetCursorCoordinates(const char *text, int length, Rectangle rec, int pos)
{
return GuiMeasureTextBox(text, length, rec, &pos, GUI_MEASURE_MODE_CURSOR_POS);
}
static inline int GuiTextBoxGetCursorFromMouse(const char *text, int length, Rectangle rec, int* pos)
{
return GuiMeasureTextBox(text, length, rec, pos, GUI_MEASURE_MODE_CURSOR_COORDS);
}
static inline int GuiTextBoxMaxCharacters(const char *text, int length, Rectangle rec)
{
int pos = -1;
GuiMeasureTextBox(text, length, rec, &pos, GUI_MEASURE_MODE_CURSOR_END);
return pos;
}
static inline unsigned int GuiCountCodepointsUntilNewline(const char *text) 
{
unsigned int len = 0;
char *ptr = (char*)&text[0];
while ((*ptr != '\0') && (*ptr != '\n'))
{
int next = 0;
int letter = GetNextCodepoint(ptr, &next);
if (letter == 0x3f) ptr += 1;
else ptr += next;
++len;
}
return len;
}
static inline void MoveTextBoxCursorRight(const char* text, int length, Rectangle textRec) 
{
int count = GuiCountCodepointsUntilNewline(text);
if (guiTextBoxState.cursor < count ) guiTextBoxState.cursor++;
const int max = GuiTextBoxMaxCharacters(&text[guiTextBoxState.index], length - guiTextBoxState.index, textRec);
if ((guiTextBoxState.cursor - guiTextBoxState.start) > max) 
{
const int cidx = GuiTextBoxGetByteIndex(text, guiTextBoxState.index, guiTextBoxState.start, guiTextBoxState.cursor);
int pos = 0;
guiTextBoxState.index = GuiMeasureTextBoxRev(text, cidx - 1, textRec, &pos);
guiTextBoxState.start = guiTextBoxState.cursor - pos;
}
}
static inline void MoveTextBoxCursorLeft(const char* text) 
{
if (guiTextBoxState.cursor > 0) guiTextBoxState.cursor--;
if (guiTextBoxState.cursor < guiTextBoxState.start)
{
int prev = 0;
int letter = GetPrevCodepoint(&text[guiTextBoxState.index - 1], text, &prev);
if (letter == 0x3f) prev = 1;
guiTextBoxState.start--;
guiTextBoxState.index -= prev;
}
}
RAYGUIDEF int GuiTextBoxGetByteIndex(const char *text, int start, int from, int to)
{
int i = start, k = from;
while ((text[i] != '\0') && (k < to)) 
{
int j = 0;
int letter = GetNextCodepoint(&text[i], &j);
if (letter == 0x3f) j = 1;
i += j;
++k;
}
return i;
}
RAYGUIDEF int GuiTextBoxDelete(char *text, int length, bool before)
{
if ((guiTextBoxState.cursor != -1) && (text != NULL))
{
int startIdx = 0, endIdx = 0;
if ((guiTextBoxState.select != -1) && (guiTextBoxState.select != guiTextBoxState.cursor))
{
int start = guiTextBoxState.cursor;
int end = guiTextBoxState.select;
if (guiTextBoxState.cursor > guiTextBoxState.select) 
{
start = guiTextBoxState.select;
end = guiTextBoxState.cursor;
}
startIdx = GuiTextBoxGetByteIndex(text, 0, 0, start);
endIdx = GuiTextBoxGetByteIndex(text, 0, 0, end);
guiTextBoxState.cursor = start; 
if (guiTextBoxState.select < guiTextBoxState.start) guiTextBoxState.start = -1; 
}
else
{
if (before)
{
if (guiTextBoxState.cursor != 0)
{
endIdx = GuiTextBoxGetByteIndex(text, 0, 0, guiTextBoxState.cursor);
guiTextBoxState.cursor--;
startIdx = GuiTextBoxGetByteIndex(text, 0, 0, guiTextBoxState.cursor);
if (guiTextBoxState.cursor < guiTextBoxState.start) guiTextBoxState.start = -1; 
}
}
else
{
if (guiTextBoxState.cursor + 1 <= GuiCountCodepointsUntilNewline(text))
{
startIdx = GuiTextBoxGetByteIndex(text, 0, 0, guiTextBoxState.cursor);
endIdx = GuiTextBoxGetByteIndex(text, 0, 0, guiTextBoxState.cursor+1);
}
}
}
memmove(&text[startIdx], &text[endIdx], length - endIdx);
text[length - (endIdx - startIdx)] = '\0';
guiTextBoxState.select = -1; 
return (endIdx - startIdx);
}
return 0;
}
RAYGUIDEF void GuiTextBoxSelectAll(const char *text)
{
guiTextBoxState.cursor = GuiCountCodepointsUntilNewline(text);
if (guiTextBoxState.cursor > 0)
{
guiTextBoxState.select = 0;
guiTextBoxState.start = -1; 
}
else guiTextBoxState.select = -1;
}
RAYGUIDEF void GuiTextBoxCopy(const char *text)
{
if ((text != NULL) &&
(guiTextBoxState.select != -1) && 
(guiTextBoxState.cursor != -1) && 
(guiTextBoxState.select != guiTextBoxState.cursor))
{
int start = guiTextBoxState.cursor;
int end = guiTextBoxState.select;
if (guiTextBoxState.cursor > guiTextBoxState.select) 
{
start = guiTextBoxState.select;
end = guiTextBoxState.cursor;
}
start = GuiTextBoxGetByteIndex(text, 0, 0, start);
end = GuiTextBoxGetByteIndex(text, 0, 0, end);
const char *clipText = TextSubtext(text, start, end - start);
SetClipboardText(clipText);
}
}
RAYGUIDEF void GuiTextBoxPaste(char *text, int textSize)
{
const char *clipText = GetClipboardText(); 
int length = strlen(text);
if ((text != NULL) && (clipText != NULL) && (guiTextBoxState.cursor != -1))
{
if ((guiTextBoxState.select != -1) && (guiTextBoxState.select != guiTextBoxState.cursor))
{
length -= GuiTextBoxDelete(text, length, true);
}
int clipLen = strlen(clipText); 
int size = ((length + clipLen) <= textSize) ? clipLen : textSize - length;
int startIdx = GuiTextBoxGetByteIndex(text, 0, 0, guiTextBoxState.cursor);
int endIdx = startIdx + size;
memmove(&text[endIdx], &text[startIdx], length - startIdx);
text[length + size] = '\0'; 
memcpy(&text[startIdx], clipText, size);
guiTextBoxState.cursor = 0;
for (int i = 0; i < (startIdx + size); guiTextBoxState.cursor++)
{
int next = 0;
int letter = GetNextCodepoint(&text[i], &next);
if (letter != 0x3f) i += next;
else i += 1;
}
guiTextBoxState.start = -1; 
}
}
RAYGUIDEF void GuiTextBoxCut(char* text)
{
if ((text != NULL) && 
(guiTextBoxState.select != -1) &&
(guiTextBoxState.cursor != -1) &&
(guiTextBoxState.select != guiTextBoxState.cursor))
{
int start = guiTextBoxState.cursor, end = guiTextBoxState.select;
if (guiTextBoxState.cursor > guiTextBoxState.select) 
{
start = guiTextBoxState.select;
end = guiTextBoxState.cursor;
}
int startIdx = GuiTextBoxGetByteIndex(text, 0, 0, start);
int endIdx = GuiTextBoxGetByteIndex(text, 0, 0, end);
const char *clipText = TextSubtext(text, startIdx, endIdx - startIdx);
SetClipboardText(clipText);
int len = strlen(text);
memmove(&text[startIdx], &text[endIdx], len - endIdx);
text[len - (endIdx - startIdx)] = '\0';
guiTextBoxState.cursor = start; 
if (guiTextBoxState.select < guiTextBoxState.start) guiTextBoxState.start = -1; 
guiTextBoxState.select = -1; 
}
}
static int EncodeCodepoint(unsigned int c, char out[5])
{
int len = 0;
if (c <= 0x7f)
{
out[0] = (char)c;
len = 1;
}
else if (c <= 0x7ff)
{
out[0] = (char)(((c >> 6) & 0x1f) | 0xc0);
out[1] = (char)((c & 0x3f) | 0x80);
len = 2;
}
else if (c <= 0xffff)
{
out[0] = (char)(((c >> 12) & 0x0f) | 0xe0);
out[1] = (char)(((c >> 6) & 0x3f) | 0x80);
out[2] = (char)((c & 0x3f) | 0x80);
len = 3;
}
else if (c <= 0x10ffff)
{
out[0] = (char)(((c >> 18) & 0x07) | 0xf0);
out[1] = (char)(((c >> 12) & 0x3f) | 0x80);
out[2] = (char)(((c >> 6) & 0x3f) | 0x80);
out[3] = (char)((c & 0x3f) | 0x80);
len = 4;
}
out[len] = 0;
return len;
}
RAYGUIDEF bool GuiTextBox(Rectangle bounds, char *text, int textSize, bool editMode)
{
#define GUI_TEXTBOX_CURSOR_SPEED_MODIFIER 5
static int framesCounter = 0; 
GuiControlState state = guiState;
bool pressed = false;
int length = strlen(text);
if (length > textSize) 
{
text[textSize] = '\0';
length = textSize;
}
if ((bounds.width - 2*GuiGetStyle(TEXTBOX, INNER_PADDING)) < GuiGetStyle(DEFAULT, TEXT_SIZE))
{
bounds.width = GuiGetStyle(DEFAULT, TEXT_SIZE) + 2*GuiGetStyle(TEXTBOX, INNER_PADDING);
}
int verticalPadding = (bounds.height - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH) - GuiGetStyle(DEFAULT, TEXT_SIZE))/2;
if (verticalPadding < 0) 
{
bounds.height = 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(DEFAULT, TEXT_SIZE);
verticalPadding = 0;
}
Rectangle textRec = { bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(TEXTBOX, INNER_PADDING), 
bounds.y + verticalPadding + GuiGetStyle(TEXTBOX, BORDER_WIDTH), 
bounds.width - 2*(GuiGetStyle(TEXTBOX, INNER_PADDING) + GuiGetStyle(TEXTBOX, BORDER_WIDTH)), 
GuiGetStyle(DEFAULT, TEXT_SIZE) };
Vector2 cursorPos = { textRec.x, textRec.y }; 
bool active = GuiTextBoxIsActive(bounds); 
int selStart = 0, selLength = 0, textStartIndex = 0;
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
const Vector2 mousePoint = GetMousePosition();
if (editMode)
{
if (!active)
{
if (CheckCollisionPointRec(mousePoint, bounds) && 
(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))) 
{
active = true;
GuiTextBoxSetActive(bounds);
}
}
else if (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
{
GuiTextBoxSetActive(RAYGUI_CLITERAL(Rectangle){0,0,-1,-1}); 
active = false;
}
if (active)
{
state = GUI_STATE_PRESSED;
framesCounter++;
if (guiTextBoxState.cursor > length) guiTextBoxState.cursor = -1;
if (guiTextBoxState.select > length) guiTextBoxState.select = -1;
if (guiTextBoxState.start > length) guiTextBoxState.start = -1;
if (guiTextBoxState.cursor == -1)
{
guiTextBoxState.cursor = GuiTextBoxMaxCharacters(text, length, textRec);
}
if (guiTextBoxState.start == -1) 
{
if (guiTextBoxState.cursor == 0)
{
guiTextBoxState.start = guiTextBoxState.index = 0; 
}
else
{
int pos = 0;
int len = GuiTextBoxGetByteIndex(text, 0, 0, guiTextBoxState.cursor);
guiTextBoxState.index = GuiMeasureTextBoxRev(text, len, textRec, &pos);
guiTextBoxState.start = guiTextBoxState.cursor - pos + 1;
}
}
if (IsKeyPressed(KEY_RIGHT) || 
(IsKeyDown(KEY_RIGHT) && (framesCounter%GUI_TEXTBOX_CURSOR_SPEED_MODIFIER == 0)))
{
if (IsKeyDown(KEY_LEFT_SHIFT))
{
if (guiTextBoxState.select == -1) guiTextBoxState.select = guiTextBoxState.cursor; 
MoveTextBoxCursorRight(text, length, textRec);
}
else
{
if (guiTextBoxState.select != -1 && guiTextBoxState.select != guiTextBoxState.cursor)
{
if (guiTextBoxState.cursor < guiTextBoxState.select)
{
guiTextBoxState.cursor = guiTextBoxState.select - 1;
MoveTextBoxCursorRight(text, length, textRec);
}
}
else
{
MoveTextBoxCursorRight(text, length, textRec);
}
guiTextBoxState.select = -1;
}
framesCounter = 0;
}
else if (IsKeyPressed(KEY_LEFT) || (IsKeyDown(KEY_LEFT) && (framesCounter%GUI_TEXTBOX_CURSOR_SPEED_MODIFIER == 0)))
{
if (IsKeyDown(KEY_LEFT_SHIFT))
{
if (guiTextBoxState.select == -1) guiTextBoxState.select = guiTextBoxState.cursor; 
MoveTextBoxCursorLeft(text);
}
else
{
if ((guiTextBoxState.select != -1) && (guiTextBoxState.select != guiTextBoxState.cursor))
{
if (guiTextBoxState.cursor > guiTextBoxState.select)
{
guiTextBoxState.cursor = guiTextBoxState.select;
if (guiTextBoxState.start > guiTextBoxState.cursor)
{
guiTextBoxState.start = guiTextBoxState.cursor;
guiTextBoxState.index = GuiTextBoxGetByteIndex(text, 0, 0, guiTextBoxState.start); 
}
}
}
else
{
MoveTextBoxCursorLeft(text);
}
guiTextBoxState.select = -1;
}
framesCounter = 0;
}
else if (IsKeyPressed(KEY_BACKSPACE) || (IsKeyDown(KEY_BACKSPACE) && (framesCounter%GUI_TEXTBOX_CURSOR_SPEED_MODIFIER) == 0))
{
GuiTextBoxDelete(text, length, true);
}
else if (IsKeyPressed(KEY_DELETE) || (IsKeyDown(KEY_DELETE) && (framesCounter%GUI_TEXTBOX_CURSOR_SPEED_MODIFIER) == 0))
{
GuiTextBoxDelete(text, length, false);
}
else if (IsKeyPressed(KEY_HOME))
{
if (IsKeyDown(KEY_LEFT_SHIFT)) 
{
if ((guiTextBoxState.select > guiTextBoxState.cursor) || 
((guiTextBoxState.select == -1) && (guiTextBoxState.cursor != 0)))
{
guiTextBoxState.select = guiTextBoxState.cursor;
}
}
else guiTextBoxState.select = -1; 
guiTextBoxState.cursor = guiTextBoxState.start = guiTextBoxState.index = 0;
framesCounter = 0;
}
else if (IsKeyPressed(KEY_END))
{
int max = GuiCountCodepointsUntilNewline(text);
if (IsKeyDown(KEY_LEFT_SHIFT)) 
{
if ((guiTextBoxState.select == -1) && (guiTextBoxState.cursor != max))
{ 
guiTextBoxState.select = guiTextBoxState.cursor;
}
}
else guiTextBoxState.select = -1; 
int pos = 0;
guiTextBoxState.cursor = max;
int len = GuiTextBoxGetByteIndex(text, 0, 0, guiTextBoxState.cursor);
guiTextBoxState.index = GuiMeasureTextBoxRev(text, len, textRec, &pos);
guiTextBoxState.start = guiTextBoxState.cursor - pos + 1;
}
else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A))
{
GuiTextBoxSelectAll(text);
}
else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C))
{
GuiTextBoxCopy(text);
}
else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_X))
{
GuiTextBoxCut(text);
}
else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V))
{
GuiTextBoxPaste(text, textSize);
}
else if (IsKeyPressed(KEY_ENTER))
{
pressed = true;
}
else
{
int key = GetKeyPressed();
if ((key >= 32) && ((guiTextBoxState.cursor + 1) < textSize))
{
if ((guiTextBoxState.select != -1) && (guiTextBoxState.select != guiTextBoxState.cursor))
{
GuiTextBoxDelete(text, length, true);
}
char out[5] = {0};
int sz = EncodeCodepoint(key, &out[0]);
if (sz != 0)
{
int startIdx = GuiTextBoxGetByteIndex(text, 0, 0, guiTextBoxState.cursor);
int endIdx = startIdx + sz;
if (endIdx <= textSize && length < textSize - 1)
{
guiTextBoxState.cursor++;
guiTextBoxState.select = -1;
memmove(&text[endIdx], &text[startIdx], length - startIdx);
memcpy(&text[startIdx], &out[0], sz);
length += sz;
text[length] = '\0';
if (guiTextBoxState.start != -1)
{
const int max = GuiTextBoxMaxCharacters(&text[guiTextBoxState.index], length - guiTextBoxState.index, textRec);
if ((guiTextBoxState.cursor - guiTextBoxState.start) > max) guiTextBoxState.start = -1;
}
}
}
}
}
if (CheckCollisionPointRec(mousePoint, bounds))
{
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
if (CheckCollisionPointRec(mousePoint, textRec))
{
GuiTextBoxGetCursorFromMouse(&text[guiTextBoxState.index], length - guiTextBoxState.index, textRec, &guiTextBoxState.cursor);
guiTextBoxState.cursor += guiTextBoxState.start;
guiTextBoxState.select = -1;
}
else
{
if (mousePoint.x <= bounds.x+bounds.width/2) guiTextBoxState.cursor = 0 + guiTextBoxState.start;
else guiTextBoxState.cursor = guiTextBoxState.start + GuiTextBoxMaxCharacters(&text[guiTextBoxState.index], length - guiTextBoxState.index, textRec);
guiTextBoxState.select = -1;
}
}
else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
{
int cursor = guiTextBoxState.cursor - guiTextBoxState.start;
bool move = false;
if (CheckCollisionPointRec(mousePoint, textRec))
{
GuiTextBoxGetCursorFromMouse(&text[guiTextBoxState.index], length - guiTextBoxState.index, textRec, &cursor);
}
else
{
move = true;
if (mousePoint.x > bounds.x+bounds.width/2) 
{
cursor = GuiTextBoxMaxCharacters(&text[guiTextBoxState.index], length - guiTextBoxState.index, textRec);
}
}
guiTextBoxState.cursor = cursor + guiTextBoxState.start;
if (guiTextBoxState.select == -1)
{
guiTextBoxState.select = guiTextBoxState.cursor;
}
if ((framesCounter%GUI_TEXTBOX_CURSOR_SPEED_MODIFIER) == 0 && move)
{
if (cursor == 0) MoveTextBoxCursorLeft(text);
else if (cursor == GuiTextBoxMaxCharacters(&text[guiTextBoxState.index], length - guiTextBoxState.index, textRec))
{
MoveTextBoxCursorRight(text, length, textRec);
}
}
}
}
cursorPos.x = GuiTextBoxGetCursorCoordinates(&text[guiTextBoxState.index], length - guiTextBoxState.index, textRec, guiTextBoxState.cursor - guiTextBoxState.start);
textStartIndex = guiTextBoxState.index;
if (guiTextBoxState.select == -1)
{
selStart = guiTextBoxState.cursor;
selLength = 0;
}
else if (guiTextBoxState.cursor > guiTextBoxState.select) 
{
selStart = guiTextBoxState.select;
selLength = guiTextBoxState.cursor - guiTextBoxState.select;
}
else 
{
selStart = guiTextBoxState.cursor;
selLength = guiTextBoxState.select - guiTextBoxState.cursor;
}
if (guiTextBoxState.start > selStart) 
{
selLength -= guiTextBoxState.start - selStart;
selStart = 0;
}
else selStart = selStart - guiTextBoxState.start;
}
else state = GUI_STATE_FOCUSED;
}
else
{
if (CheckCollisionPointRec(mousePoint, bounds))
{
state = GUI_STATE_FOCUSED;
if (IsMouseButtonPressed(0)) pressed = true;
}
if (active && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C))
{
int select = guiTextBoxState.select;
int cursor = guiTextBoxState.cursor;
int start = guiTextBoxState.start;
if (guiTextBoxState.select == -1 || guiTextBoxState.select == guiTextBoxState.cursor)
{
GuiTextBoxSelectAll(text);
}
GuiTextBoxCopy(text);
guiTextBoxState.select = select;
guiTextBoxState.cursor = cursor;
guiTextBoxState.start = start;
}
}
}
DrawRectangleLinesEx(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha));
if (state == GUI_STATE_PRESSED)
{
DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_FOCUSED)), guiAlpha));
if (editMode && active && ((framesCounter/TEXTEDIT_CURSOR_BLINK_FRAMES)%2 == 0) && selLength == 0) 
{
DrawRectangle(cursorPos.x, cursorPos.y, 1, GuiGetStyle(DEFAULT, TEXT_SIZE), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER_COLOR_PRESSED)), guiAlpha));
}
}
else if (state == GUI_STATE_DISABLED)
{
DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_DISABLED)), guiAlpha));
}
DrawTextRecEx(guiFont, &text[textStartIndex], textRec, GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), false, Fade(GetColor(GuiGetStyle(TEXTBOX, TEXT + (state*3))), guiAlpha), selStart, selLength, GetColor(GuiGetStyle(TEXTBOX, COLOR_SELECTED_FG)), GetColor(GuiGetStyle(TEXTBOX, COLOR_SELECTED_BG)));
return pressed;
}
#else 
RAYGUIDEF bool GuiSpinner(Rectangle bounds, int *value, int minValue, int maxValue, bool editMode)
{
bool pressed = false;
int tempValue = *value;
Rectangle spinner = { bounds.x + GuiGetStyle(SPINNER, SELECT_BUTTON_WIDTH) + GuiGetStyle(SPINNER, SELECT_BUTTON_PADDING), bounds.y,
bounds.width - 2*(GuiGetStyle(SPINNER, SELECT_BUTTON_WIDTH) + GuiGetStyle(SPINNER, SELECT_BUTTON_PADDING)), bounds.height };
Rectangle leftButtonBound = { (float)bounds.x, (float)bounds.y, (float)GuiGetStyle(SPINNER, SELECT_BUTTON_WIDTH), (float)bounds.height };
Rectangle rightButtonBound = { (float)bounds.x + bounds.width - GuiGetStyle(SPINNER, SELECT_BUTTON_WIDTH), (float)bounds.y, (float)GuiGetStyle(SPINNER, SELECT_BUTTON_WIDTH), (float)bounds.height };
if (!editMode)
{
if (tempValue < minValue) tempValue = minValue;
if (tempValue > maxValue) tempValue = maxValue;
}
pressed = GuiValueBox(spinner, &tempValue, minValue, maxValue, editMode);
int tempBorderWidth = GuiGetStyle(BUTTON, BORDER_WIDTH);
GuiSetStyle(BUTTON, BORDER_WIDTH, GuiGetStyle(SPINNER, BORDER_WIDTH));
int tempTextAlign = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
#if defined(RAYGUI_RICONS_SUPPORT)
if (GuiButton(leftButtonBound, GuiIconText(RICON_ARROW_LEFT_FILL, NULL))) tempValue--;
if (GuiButton(rightButtonBound, GuiIconText(RICON_ARROW_RIGHT_FILL, NULL))) tempValue++;
#else
if (GuiButton(leftButtonBound, "<")) tempValue--;
if (GuiButton(rightButtonBound, ">")) tempValue++;
#endif
GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlign);
GuiSetStyle(BUTTON, BORDER_WIDTH, tempBorderWidth);
*value = tempValue;
return pressed;
}
RAYGUIDEF bool GuiValueBox(Rectangle bounds, int *value, int minValue, int maxValue, bool editMode)
{
#define VALUEBOX_MAX_CHARS 32
static int framesCounter = 0; 
GuiControlState state = guiState;
bool pressed = false;
char text[VALUEBOX_MAX_CHARS + 1] = "\0";
sprintf(text, "%i", *value);
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
bool valueHasChanged = false;
if (editMode)
{
state = GUI_STATE_PRESSED;
framesCounter++;
int keyCount = strlen(text);
if (keyCount < VALUEBOX_MAX_CHARS)
{
int maxWidth = (bounds.width - (GuiGetStyle(VALUEBOX, INNER_PADDING)*2));
if (GetTextWidth(text) < maxWidth)
{
int key = GetKeyPressed();
if ((key >= 48) && (key <= 57))
{
text[keyCount] = (char)key;
keyCount++;
valueHasChanged = true;
}
}
}
if (keyCount > 0)
{
if (IsKeyPressed(KEY_BACKSPACE))
{
keyCount--;
text[keyCount] = '\0';
framesCounter = 0;
if (keyCount < 0) keyCount = 0;
valueHasChanged = true;
}
else if (IsKeyDown(KEY_BACKSPACE))
{
if ((framesCounter > TEXTEDIT_CURSOR_BLINK_FRAMES) && (framesCounter%2) == 0) keyCount--;
text[keyCount] = '\0';
if (keyCount < 0) keyCount = 0;
valueHasChanged = true;
}
}
if (valueHasChanged) *value = atoi(text);
}
else
{
if (*value > maxValue) *value = maxValue;
else if (*value < minValue) *value = minValue;
}
if (!editMode)
{
if (CheckCollisionPointRec(mousePoint, bounds))
{
state = GUI_STATE_FOCUSED;
if (IsMouseButtonPressed(0)) pressed = true;
}
}
else
{
if (IsKeyPressed(KEY_ENTER) || (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(0))) pressed = true;
}
if (pressed) framesCounter = 0;
}
DrawRectangleLinesEx(bounds, GuiGetStyle(VALUEBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER + (state*3))), guiAlpha));
if (state == GUI_STATE_PRESSED)
{
DrawRectangle(bounds.x + GuiGetStyle(VALUEBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(VALUEBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(VALUEBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(VALUEBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_PRESSED)), guiAlpha));
if (editMode && ((framesCounter/20)%2 == 0)) DrawRectangle(bounds.x + GetTextWidth(text)/2 + bounds.width/2 + 2, bounds.y + GuiGetStyle(VALUEBOX, INNER_PADDING), 1, bounds.height - GuiGetStyle(VALUEBOX, INNER_PADDING)*2, Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER_COLOR_PRESSED)), guiAlpha));
}
else if (state == GUI_STATE_DISABLED)
{
DrawRectangle(bounds.x + GuiGetStyle(VALUEBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(VALUEBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(VALUEBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(VALUEBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_DISABLED)), guiAlpha));
}
GuiDrawText(text, GetTextBounds(VALUEBOX, bounds), GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(VALUEBOX, TEXT + (state*3))), guiAlpha));
return pressed;
}
RAYGUIDEF bool GuiTextBox(Rectangle bounds, char *text, int textSize, bool editMode)
{
static int framesCounter = 0; 
GuiControlState state = guiState;
bool pressed = false;
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (editMode)
{
state = GUI_STATE_PRESSED;
framesCounter++;
int key = GetKeyPressed();
int keyCount = strlen(text);
if (keyCount < (textSize - 1))
{
int maxWidth = (bounds.width - (GuiGetStyle(DEFAULT, INNER_PADDING)*2));
if (GetTextWidth(text) < (maxWidth - GuiGetStyle(DEFAULT, TEXT_SIZE)))
{
if (((key >= 32) && (key <= 125)) ||
((key >= 128) && (key < 255)))
{
text[keyCount] = (char)key;
keyCount++;
text[keyCount] = '\0';
}
}
}
if (keyCount > 0)
{
if (IsKeyPressed(KEY_BACKSPACE))
{
keyCount--;
text[keyCount] = '\0';
framesCounter = 0;
if (keyCount < 0) keyCount = 0;
}
else if (IsKeyDown(KEY_BACKSPACE))
{
if ((framesCounter > TEXTEDIT_CURSOR_BLINK_FRAMES) && (framesCounter%2) == 0) keyCount--;
text[keyCount] = '\0';
if (keyCount < 0) keyCount = 0;
}
}
}
if (!editMode)
{
if (CheckCollisionPointRec(mousePoint, bounds))
{
state = GUI_STATE_FOCUSED;
if (IsMouseButtonPressed(0)) pressed = true;
}
}
else
{
if (IsKeyPressed(KEY_ENTER) || (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(0))) pressed = true;
}
if (pressed) framesCounter = 0;
}
DrawRectangleLinesEx(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha));
if (state == GUI_STATE_PRESSED)
{
DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_PRESSED)), guiAlpha));
if (editMode && ((framesCounter/20)%2 == 0)) DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, INNER_PADDING) + GetTextWidth(text) + 2 + bounds.width/2*GuiGetStyle(TEXTBOX, TEXT_ALIGNMENT), bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE), 1, GuiGetStyle(DEFAULT, TEXT_SIZE)*2, Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER_COLOR_PRESSED)), guiAlpha));
}
else if (state == GUI_STATE_DISABLED)
{
DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_DISABLED)), guiAlpha));
}
GuiDrawText(text, GetTextBounds(TEXTBOX, bounds), GuiGetStyle(TEXTBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TEXTBOX, TEXT + (state*3))), guiAlpha));
return pressed;
}
#endif
RAYGUIDEF bool GuiTextBoxMulti(Rectangle bounds, char *text, int textSize, bool editMode)
{
static int framesCounter = 0; 
GuiControlState state = guiState;
bool pressed = false;
bool textHasChange = false;
int currentLine = 0;
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (editMode)
{
state = GUI_STATE_PRESSED;
framesCounter++;
int keyCount = strlen(text);
int maxWidth = (bounds.width - (GuiGetStyle(TEXTBOX, INNER_PADDING)*2));
int maxHeight = (bounds.height - (GuiGetStyle(TEXTBOX, INNER_PADDING)*2));
if (keyCount < (textSize - 1))
{
int key = GetKeyPressed();
if (MeasureTextEx(guiFont, text, GuiGetStyle(DEFAULT, TEXT_SIZE), 1).y < (maxHeight - GuiGetStyle(DEFAULT, TEXT_SIZE)))
{
if (IsKeyPressed(KEY_ENTER))
{
text[keyCount] = '\n';
keyCount++;
}
else if (((key >= 32) && (key <= 125)) ||
((key >= 128) && (key < 255)))
{
text[keyCount] = (char)key;
keyCount++;
textHasChange = true;
}
}
else if (GetTextWidth(strrchr(text, '\n')) < (maxWidth - GuiGetStyle(DEFAULT, TEXT_SIZE)))
{
if (((key >= 32) && (key <= 125)) ||
((key >= 128) && (key < 255)))
{
text[keyCount] = (char)key;
keyCount++;
textHasChange = true;
}
}
}
if (keyCount > 0)
{
if (IsKeyPressed(KEY_BACKSPACE))
{
keyCount--;
text[keyCount] = '\0';
framesCounter = 0;
if (keyCount < 0) keyCount = 0;
textHasChange = true;
}
else if (IsKeyDown(KEY_BACKSPACE))
{
if ((framesCounter > TEXTEDIT_CURSOR_BLINK_FRAMES) && (framesCounter%2) == 0) keyCount--;
text[keyCount] = '\0';
if (keyCount < 0) keyCount = 0;
textHasChange = true;
}
}
if (textHasChange)
{
textHasChange = false;
char *lastLine = strrchr(text, '\n');
int maxWidth = (bounds.width - (GuiGetStyle(TEXTBOX, INNER_PADDING)*2));
if (lastLine != NULL)
{
if (GetTextWidth(lastLine) > maxWidth)
{
int firstIndex = lastLine - text;
char *lastSpace = strrchr(lastLine, 32);
if (lastSpace != NULL)
{
int secondIndex = lastSpace - lastLine;
text[firstIndex + secondIndex] = '\n';
}
else
{
int len = (lastLine != NULL)? strlen(lastLine) : 0;
char lastChar = lastLine[len - 1];
lastLine[len - 1] = '\n';
lastLine[len] = lastChar;
lastLine[len + 1] = '\0';
keyCount++;
}
}
}
else
{
if (GetTextWidth(text) > maxWidth)
{
char *lastSpace = strrchr(text, 32);
if (lastSpace != NULL)
{
int index = lastSpace - text;
text[index] = '\n';
}
else
{
int len = (lastLine != NULL)? strlen(lastLine) : 0;
char lastChar = lastLine[len - 1];
lastLine[len - 1] = '\n';
lastLine[len] = lastChar;
lastLine[len + 1] = '\0';
keyCount++;
}
}
}
}
for (int i = 0; i < keyCount; i++)
{
if (text[i] == '\n') currentLine++;
}
}
if (!editMode)
{
if (CheckCollisionPointRec(mousePoint, bounds))
{
state = GUI_STATE_FOCUSED;
if (IsMouseButtonPressed(0)) pressed = true;
}
}
else
{
if (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(0)) pressed = true;
}
if (pressed) framesCounter = 0;
}
DrawRectangleLinesEx(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha));
if (state == GUI_STATE_PRESSED)
{
DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_PRESSED)), guiAlpha));
if (editMode)
{
if ((framesCounter/20)%2 == 0)
{
char *line = NULL;
if (currentLine > 0) line = strrchr(text, '\n');
else line = text;
DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(TEXTBOX, INNER_PADDING) + GetTextWidth(line),
bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(TEXTBOX, INNER_PADDING)/2 + ((GuiGetStyle(DEFAULT, TEXT_SIZE) + GuiGetStyle(TEXTBOX, INNER_PADDING))*currentLine),
1, GuiGetStyle(DEFAULT, TEXT_SIZE) + GuiGetStyle(TEXTBOX, INNER_PADDING), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER_COLOR_FOCUSED)), guiAlpha));
}
}
}
else if (state == GUI_STATE_DISABLED)
{
DrawRectangle(bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_DISABLED)), guiAlpha));
}
GuiDrawText(text, GetTextBounds(TEXTBOX, bounds), GuiGetStyle(TEXTBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TEXTBOX, TEXT + (state*3))), guiAlpha));
return pressed;
}
RAYGUIDEF float GuiSliderPro(Rectangle bounds, const char *text, float value, float minValue, float maxValue, int sliderWidth, bool showValue)
{
GuiControlState state = guiState;
int sliderValue = (int)(((value - minValue)/(maxValue - minValue))*(bounds.width - 2*GuiGetStyle(SLIDER, BORDER_WIDTH)));
Rectangle slider = { bounds.x, bounds.y + GuiGetStyle(SLIDER, BORDER_WIDTH) + GuiGetStyle(SLIDER, INNER_PADDING),
0, bounds.height - 2*GuiGetStyle(SLIDER, BORDER_WIDTH) - 2*GuiGetStyle(SLIDER, INNER_PADDING) };
if (sliderWidth > 0) 
{
slider.x += (sliderValue - sliderWidth/2);
slider.width = sliderWidth;
}
else if (sliderWidth == 0) 
{
slider.x += GuiGetStyle(SLIDER, BORDER_WIDTH);
slider.width = sliderValue;
}
Rectangle textBounds = { 0 };
textBounds.width = GetTextWidth(text); 
textBounds.height = GuiGetStyle(DEFAULT, TEXT_SIZE);
textBounds.x = bounds.x - textBounds.width - GuiGetStyle(SLIDER, TEXT_PADDING);
textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
{
state = GUI_STATE_PRESSED;
value = ((maxValue - minValue)*(mousePoint.x - (float)(bounds.x + sliderWidth/2)))/(float)(bounds.width - sliderWidth) + minValue;
if (sliderWidth > 0) slider.x = mousePoint.x - slider.width/2; 
else if (sliderWidth == 0) slider.width = sliderValue; 
}
else state = GUI_STATE_FOCUSED;
}
if (value > maxValue) value = maxValue;
else if (value < minValue) value = minValue;
}
if (sliderWidth > 0) 
{
if (slider.x <= (bounds.x + GuiGetStyle(SLIDER, BORDER_WIDTH))) slider.x = bounds.x + GuiGetStyle(SLIDER, BORDER_WIDTH);
else if ((slider.x + slider.width) >= (bounds.x + bounds.width)) slider.x = bounds.x + bounds.width - slider.width - GuiGetStyle(SLIDER, BORDER_WIDTH);
}
else if (sliderWidth == 0) 
{
if (slider.width > bounds.width) slider.width = bounds.width - 2*GuiGetStyle(SLIDER, BORDER_WIDTH);
}
DrawRectangleLinesEx(bounds, GuiGetStyle(SLIDER, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(SLIDER, BORDER + (state*3))), guiAlpha));
DrawRectangle(bounds.x + GuiGetStyle(SLIDER, BORDER_WIDTH), bounds.y + GuiGetStyle(SLIDER, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(SLIDER, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(SLIDER, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(SLIDER, (state != GUI_STATE_DISABLED)? BASE_COLOR_NORMAL : BASE_COLOR_DISABLED)), guiAlpha));
if ((state == GUI_STATE_NORMAL) || (state == GUI_STATE_PRESSED)) DrawRectangleRec(slider, Fade(GetColor(GuiGetStyle(SLIDER, BASE_COLOR_PRESSED)), guiAlpha));
else if (state == GUI_STATE_FOCUSED) DrawRectangleRec(slider, Fade(GetColor(GuiGetStyle(SLIDER, TEXT_COLOR_FOCUSED)), guiAlpha));
GuiDrawText(text, textBounds, GuiGetStyle(SLIDER, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(SLIDER, TEXT + (state*3))), guiAlpha));
if (showValue) GuiDrawText(TextFormat("%.02f", value), RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + bounds.width + GuiGetStyle(SLIDER, TEXT_PADDING),
(float)bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2 + GuiGetStyle(SLIDER, INNER_PADDING),
(float)GuiGetStyle(DEFAULT, TEXT_SIZE), (float)GuiGetStyle(DEFAULT, TEXT_SIZE) }, GUI_TEXT_ALIGN_LEFT,
Fade(GetColor(GuiGetStyle(SLIDER, TEXT + (state*3))), guiAlpha));
return value;
}
RAYGUIDEF float GuiSlider(Rectangle bounds, const char *text, float value, float minValue, float maxValue, bool showValue)
{
return GuiSliderPro(bounds, text, value, minValue, maxValue, GuiGetStyle(SLIDER, SLIDER_WIDTH), showValue);
}
RAYGUIDEF float GuiSliderBar(Rectangle bounds, const char *text, float value, float minValue, float maxValue, bool showValue)
{
return GuiSliderPro(bounds, text, value, minValue, maxValue, 0, showValue);
}
RAYGUIDEF float GuiProgressBar(Rectangle bounds, const char *text, float value, float minValue, float maxValue, bool showValue)
{
GuiControlState state = guiState;
Rectangle progress = { bounds.x + GuiGetStyle(PROGRESSBAR, BORDER_WIDTH),
bounds.y + GuiGetStyle(PROGRESSBAR, BORDER_WIDTH) + GuiGetStyle(PROGRESSBAR, INNER_PADDING), 0,
bounds.height - 2*GuiGetStyle(PROGRESSBAR, BORDER_WIDTH) - 2*GuiGetStyle(PROGRESSBAR, INNER_PADDING) };
if (state != GUI_STATE_DISABLED) progress.width = (int)(value/(maxValue - minValue)*(float)(bounds.width - 2*GuiGetStyle(PROGRESSBAR, BORDER_WIDTH)));
if (showValue) GuiLabel(RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + bounds.width + GuiGetStyle(SLIDER, TEXT_PADDING), (float)bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2 + GuiGetStyle(SLIDER, INNER_PADDING), (float)GuiGetStyle(DEFAULT, TEXT_SIZE), (float)GuiGetStyle(DEFAULT, TEXT_SIZE) }, TextFormat("%.02f", value));
DrawRectangleLinesEx(bounds, GuiGetStyle(PROGRESSBAR, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(PROGRESSBAR, BORDER + (state*3))), guiAlpha));
if ((state == GUI_STATE_NORMAL) || (state == GUI_STATE_PRESSED)) DrawRectangleRec(progress, Fade(GetColor(GuiGetStyle(PROGRESSBAR, BASE_COLOR_PRESSED)), guiAlpha));
else if (state == GUI_STATE_FOCUSED) DrawRectangleRec(progress, Fade(GetColor(GuiGetStyle(PROGRESSBAR, TEXT_COLOR_FOCUSED)), guiAlpha));
return value;
}
RAYGUIDEF void GuiStatusBar(Rectangle bounds, const char *text)
{
GuiControlState state = guiState;
DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DEFAULT, (state != GUI_STATE_DISABLED)? BORDER_COLOR_NORMAL : BORDER_COLOR_DISABLED)), guiAlpha));
DrawRectangleRec(RAYGUI_CLITERAL(Rectangle){ bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.width - GuiGetStyle(DEFAULT, BORDER_WIDTH)*2, bounds.height - GuiGetStyle(DEFAULT, BORDER_WIDTH)*2 }, Fade(GetColor(GuiGetStyle(DEFAULT, (state != GUI_STATE_DISABLED)? BASE_COLOR_NORMAL : BASE_COLOR_DISABLED)), guiAlpha));
GuiDrawText(text, GetTextBounds(DEFAULT, bounds), GuiGetStyle(DEFAULT, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(DEFAULT, (state != GUI_STATE_DISABLED)? TEXT_COLOR_NORMAL : TEXT_COLOR_DISABLED)), guiAlpha));
}
RAYGUIDEF void GuiDummyRec(Rectangle bounds, const char *text)
{
GuiControlState state = guiState;
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
else state = GUI_STATE_FOCUSED;
}
}
DrawRectangleRec(bounds, Fade(GetColor(GuiGetStyle(DEFAULT, (state != GUI_STATE_DISABLED)? BASE_COLOR_NORMAL : BASE_COLOR_DISABLED)), guiAlpha));
GuiDrawText(text, GetTextBounds(DEFAULT, bounds), GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(BUTTON, (state != GUI_STATE_DISABLED)? TEXT_COLOR_NORMAL : TEXT_COLOR_DISABLED)), guiAlpha));
}
RAYGUIDEF int GuiScrollBar(Rectangle bounds, int value, int minValue, int maxValue)
{
GuiControlState state = guiState;
bool isVertical = (bounds.width > bounds.height)? false : true;
const int spinnerSize = GuiGetStyle(SCROLLBAR, ARROWS_VISIBLE)? (isVertical? bounds.width - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH) : bounds.height - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH)) : 0;
Rectangle arrowUpLeft = { 0 };
Rectangle arrowDownRight = { 0 };
Rectangle scrollbar = { 0 };
Rectangle slider = { 0 };
if (value > maxValue) value = maxValue;
if (value < minValue) value = minValue;
const int range = maxValue - minValue;
int sliderSize = GuiGetStyle(SCROLLBAR, SLIDER_SIZE);
arrowUpLeft = RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)spinnerSize, (float)spinnerSize };
if (isVertical)
{
arrowDownRight = RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)bounds.y + bounds.height - spinnerSize - GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)spinnerSize, (float)spinnerSize};
scrollbar = RAYGUI_CLITERAL(Rectangle){ bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, INNER_PADDING), arrowUpLeft.y + arrowUpLeft.height, bounds.width - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, INNER_PADDING)), bounds.height - arrowUpLeft.height - arrowDownRight.height - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH) };
sliderSize = (sliderSize >= scrollbar.height)? (scrollbar.height - 2) : sliderSize; 
slider = RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SLIDER_PADDING), (float)scrollbar.y + (int)(((float)(value - minValue)/range)*(scrollbar.height - sliderSize)), (float)bounds.width - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SLIDER_PADDING)), (float)sliderSize };
}
else
{
arrowDownRight = RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + bounds.width - spinnerSize - GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)spinnerSize, (float)spinnerSize};
scrollbar = RAYGUI_CLITERAL(Rectangle){ arrowUpLeft.x + arrowUpLeft.width, bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, INNER_PADDING), bounds.width - arrowUpLeft.width - arrowDownRight.width - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH), bounds.height - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, INNER_PADDING))};
sliderSize = (sliderSize >= scrollbar.width)? (scrollbar.width - 2) : sliderSize; 
slider = RAYGUI_CLITERAL(Rectangle){ (float)scrollbar.x + (int)(((float)(value - minValue)/range)*(scrollbar.width - sliderSize)), (float)bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SLIDER_PADDING), (float)sliderSize, (float)bounds.height - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SLIDER_PADDING)) };
}
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds))
{
state = GUI_STATE_FOCUSED;
int wheel = GetMouseWheelMove();
if (wheel != 0) value += wheel;
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
if (CheckCollisionPointRec(mousePoint, arrowUpLeft)) value -= range/GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
else if (CheckCollisionPointRec(mousePoint, arrowDownRight)) value += range/GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
state = GUI_STATE_PRESSED;
}
else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
{
if (!isVertical)
{
Rectangle scrollArea = { arrowUpLeft.x + arrowUpLeft.width, arrowUpLeft.y, scrollbar.width, bounds.height - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH)};
if (CheckCollisionPointRec(mousePoint, scrollArea)) value = ((float)(mousePoint.x - scrollArea.x - slider.width/2)*range)/(scrollArea.width - slider.width) + minValue;
}
else
{
Rectangle scrollArea = { arrowUpLeft.x, arrowUpLeft.y+arrowUpLeft.height, bounds.width - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH), scrollbar.height};
if (CheckCollisionPointRec(mousePoint, scrollArea)) value = ((float)(mousePoint.y - scrollArea.y - slider.height/2)*range)/(scrollArea.height - slider.height) + minValue;
}
}
}
if (value > maxValue) value = maxValue;
if (value < minValue) value = minValue;
}
DrawRectangleRec(bounds, Fade(GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_DISABLED)), guiAlpha)); 
DrawRectangleRec(scrollbar, Fade(GetColor(GuiGetStyle(BUTTON, BASE_COLOR_NORMAL)), guiAlpha)); 
DrawRectangleLinesEx(bounds, GuiGetStyle(SCROLLBAR, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER + state*3)), guiAlpha));
DrawRectangleRec(slider, Fade(GetColor(GuiGetStyle(SLIDER, BORDER + state*3)), guiAlpha)); 
const int padding = (spinnerSize - GuiGetStyle(SCROLLBAR, ARROWS_SIZE))/2;
const Vector2 lineCoords[] =
{
{ arrowUpLeft.x + padding, arrowUpLeft.y + spinnerSize/2 },
{ arrowUpLeft.x + spinnerSize - padding, arrowUpLeft.y + padding },
{ arrowUpLeft.x + spinnerSize - padding, arrowUpLeft.y + spinnerSize - padding },
{ arrowDownRight.x + padding, arrowDownRight.y + padding },
{ arrowDownRight.x + spinnerSize - padding, arrowDownRight.y + spinnerSize/2 },
{ arrowDownRight.x + padding, arrowDownRight.y + spinnerSize - padding },
{ arrowUpLeft.x + spinnerSize/2, arrowUpLeft.y + padding },
{ arrowUpLeft.x + padding, arrowUpLeft.y + spinnerSize - padding },
{ arrowUpLeft.x + spinnerSize - padding, arrowUpLeft.y + spinnerSize - padding },
{ arrowDownRight.x + padding, arrowDownRight.y + padding },
{ arrowDownRight.x + spinnerSize/2, arrowDownRight.y + spinnerSize - padding },
{ arrowDownRight.x + spinnerSize - padding, arrowDownRight.y + padding }
};
Color lineColor = Fade(GetColor(GuiGetStyle(BUTTON, TEXT + state*3)), guiAlpha);
if (GuiGetStyle(SCROLLBAR, ARROWS_VISIBLE))
{
if (isVertical)
{
DrawTriangle(lineCoords[6], lineCoords[7], lineCoords[8], lineColor);
DrawTriangle(lineCoords[9], lineCoords[10], lineCoords[11], lineColor);
}
else
{
DrawTriangle(lineCoords[2], lineCoords[1], lineCoords[0], lineColor);
DrawTriangle(lineCoords[5], lineCoords[4], lineCoords[3], lineColor);
}
}
return value;
}
static bool GuiListElement(Rectangle bounds, const char *text, bool active, bool editMode)
{
GuiControlState state = guiState;
if (!guiLocked && editMode) state = GUI_STATE_NORMAL;
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds))
{
if (!active)
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
else state = GUI_STATE_FOCUSED;
}
if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) active = !active;
}
}
switch (state)
{
case GUI_STATE_NORMAL:
{
if (active)
{
DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_PRESSED)), guiAlpha));
DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_PRESSED)), guiAlpha));
}
} break;
case GUI_STATE_FOCUSED:
{
DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_FOCUSED)), guiAlpha));
DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_FOCUSED)), guiAlpha));
} break;
case GUI_STATE_PRESSED:
{
DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_PRESSED)), guiAlpha));
DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_PRESSED)), guiAlpha));
} break;
case GUI_STATE_DISABLED:
{
if (active)
{
DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_DISABLED)), guiAlpha));
DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_NORMAL)), guiAlpha));
}
} break;
default: break;
}
if (state == GUI_STATE_NORMAL) GuiDrawText(text, GetTextBounds(DEFAULT, bounds), GuiGetStyle(DEFAULT, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, active? TEXT_COLOR_PRESSED : TEXT_COLOR_NORMAL)), guiAlpha));
else if (state == GUI_STATE_DISABLED) GuiDrawText(text, GetTextBounds(DEFAULT, bounds), GuiGetStyle(DEFAULT, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, active? TEXT_COLOR_NORMAL : TEXT_COLOR_DISABLED)), guiAlpha));
else GuiDrawText(text, GetTextBounds(DEFAULT, bounds), GuiGetStyle(DEFAULT, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT + state*3)), guiAlpha));
return active;
}
RAYGUIDEF bool GuiListView(Rectangle bounds, const char *text, int *active, int *scrollIndex, bool editMode)
{
bool result = 0;
int count = 0;
const char **textList = GuiTextSplit(text, &count, NULL);
result = GuiListViewEx(bounds, textList, count, NULL, active, NULL, scrollIndex, editMode);
return result;
}
RAYGUIDEF bool GuiListViewEx(Rectangle bounds, const char **text, int count, int *enabled, int *active, int *focus, int *scrollIndex, bool editMode)
{
GuiControlState state = guiState;
bool pressed = false;
int focusElement = -1;
int startIndex = (scrollIndex == NULL)? 0 : *scrollIndex;
bool useScrollBar = true;
bool pressedKey = false;
int visibleElements = bounds.height/(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING));
if ((startIndex < 0) || (startIndex > count - visibleElements)) startIndex = 0;
int endIndex = startIndex + visibleElements;
int auxActive = *active;
float barHeight = bounds.height;
float minBarHeight = 10;
if (visibleElements >= count)
{
useScrollBar = false;
startIndex = 0;
endIndex = count;
}
int posX = bounds.x + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING);
int elementWidth = bounds.width - 2*GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) - GuiGetStyle(DEFAULT, BORDER_WIDTH);
if (useScrollBar)
{
posX = GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE? posX + GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH) : posX;
elementWidth = bounds.width - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH) - 2*GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) - GuiGetStyle(DEFAULT, BORDER_WIDTH);
}
Rectangle scrollBarRect = { (float)bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH), (float)bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) };
if (GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_RIGHT_SIDE) scrollBarRect.x = posX + elementWidth + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING);
Rectangle viewArea = { (float)posX, (float)bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)elementWidth, (float)bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) };
if ((state != GUI_STATE_DISABLED) && !guiLocked) 
{
Vector2 mousePoint = GetMousePosition();
if (editMode)
{
state = GUI_STATE_PRESSED;
if (IsKeyPressed(KEY_UP))
{
if (auxActive > 0)
{
auxActive--;
if ((useScrollBar) && (auxActive < startIndex)) startIndex--;
}
pressedKey = true;
}
else if (IsKeyPressed(KEY_DOWN))
{
if (auxActive < count - 1)
{
auxActive++;
if ((useScrollBar) && (auxActive >= endIndex)) startIndex++;
}
pressedKey = true;
}
if (useScrollBar)
{
endIndex = startIndex + visibleElements;
if (CheckCollisionPointRec(mousePoint, viewArea))
{
int wheel = GetMouseWheelMove();
if (wheel < 0 && endIndex < count) startIndex -= wheel;
else if (wheel > 0 && startIndex > 0) startIndex -= wheel;
}
if (pressedKey)
{
pressedKey = false;
if ((auxActive < startIndex) || (auxActive >= endIndex)) startIndex = auxActive;
}
if (startIndex < 0) startIndex = 0;
else if (startIndex > (count - (endIndex - startIndex)))
{
startIndex = count - (endIndex - startIndex);
}
endIndex = startIndex + visibleElements;
if (endIndex > count) endIndex = count;
}
}
if (!editMode)
{
if (CheckCollisionPointRec(mousePoint, viewArea))
{
state = GUI_STATE_FOCUSED;
if (IsMouseButtonPressed(0)) pressed = true;
startIndex -= GetMouseWheelMove();
if (startIndex < 0) startIndex = 0;
else if (startIndex > (count - (endIndex - startIndex)))
{
startIndex = count - (endIndex - startIndex);
}
pressed = true;
}
}
else
{
if (!CheckCollisionPointRec(mousePoint, viewArea))
{
if (IsMouseButtonPressed(0) || (GetMouseWheelMove() != 0)) pressed = true;
}
}
for (int i = startIndex; i < endIndex; i++)
{
if (CheckCollisionPointRec(mousePoint, RAYGUI_CLITERAL(Rectangle){ (float)posX, (float)bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), (float)elementWidth, (float)GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }))
{
focusElement = i;
}
}
}
const int slider = GuiGetStyle(SCROLLBAR, SLIDER_SIZE); 
if (useScrollBar)
{
float percentVisible = (endIndex - startIndex)*100/count;
barHeight *= percentVisible/100;
if (barHeight < minBarHeight) barHeight = minBarHeight;
else if (barHeight > bounds.height) barHeight = bounds.height;
GuiSetStyle(SCROLLBAR, SLIDER_SIZE, barHeight); 
}
DrawRectangleRec(bounds, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); 
if (useScrollBar)
{
const int scrollSpeed = GuiGetStyle(SCROLLBAR, SCROLL_SPEED); 
GuiSetStyle(SCROLLBAR, SCROLL_SPEED, count - visibleElements); 
int index = scrollIndex != NULL? *scrollIndex : startIndex;
index = GuiScrollBar(scrollBarRect, index, 0, count - visibleElements);
GuiSetStyle(SCROLLBAR, SCROLL_SPEED, scrollSpeed); 
GuiSetStyle(SCROLLBAR, SLIDER_SIZE, slider); 
if (scrollIndex != NULL && CheckCollisionPointRec(GetMousePosition(), scrollBarRect) && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
{
startIndex = index;
if (startIndex < 0) startIndex = 0;
if (startIndex > (count - (endIndex - startIndex)))
{
startIndex = count - (endIndex - startIndex);
}
endIndex = startIndex + visibleElements;
if (endIndex > count) endIndex = count;
}
}
DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER + state*3)), guiAlpha));
switch (state)
{
case GUI_STATE_NORMAL:
{
for (int i = startIndex; i < endIndex; i++)
{
if ((enabled != NULL) && (enabled[i] == 0))
{
GuiDisable();
GuiListElement(RAYGUI_CLITERAL(Rectangle){ (float)posX, (float)bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), (float)elementWidth, (float)GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, false);
GuiEnable();
}
else if (i == auxActive)
{
GuiDisable();
GuiListElement(RAYGUI_CLITERAL(Rectangle){ (float)posX, (float)bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), (float)elementWidth, (float)GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], true, false);
GuiEnable();
}
else GuiListElement(RAYGUI_CLITERAL(Rectangle){ (float)posX, (float)bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), (float)elementWidth, (float)GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, false);
}
} break;
case GUI_STATE_FOCUSED:
{
for (int i = startIndex; i < endIndex; i++)
{
if ((enabled != NULL) && (enabled[i] == 0))
{
GuiDisable();
GuiListElement(RAYGUI_CLITERAL(Rectangle){ (float)posX, (float)bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), (float)elementWidth, (float)GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, false);
GuiEnable();
}
else if (i == auxActive) GuiListElement(RAYGUI_CLITERAL(Rectangle){ (float)posX, (float)bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), (float)elementWidth, (float)GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], true, false);
else GuiListElement(RAYGUI_CLITERAL(Rectangle){ (float)posX, (float)bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), (float)elementWidth, (float)GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, false);
}
} break;
case GUI_STATE_PRESSED:
{
for (int i = startIndex; i < endIndex; i++)
{
if ((enabled != NULL) && (enabled[i] == 0))
{
GuiDisable();
GuiListElement(RAYGUI_CLITERAL(Rectangle){ (float)posX, (float)bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), (float)elementWidth, (float)GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, false);
GuiEnable();
}
else if ((i == auxActive) && editMode)
{
if (GuiListElement(RAYGUI_CLITERAL(Rectangle){ (float)posX, (float)bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), (float)elementWidth, (float)GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], true, true) == false) auxActive = -1;
}
else
{
if (GuiListElement(RAYGUI_CLITERAL(Rectangle){ (float)posX, (float)bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), (float)elementWidth, (float)GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, true) == true) auxActive = i;
}
}
} break;
case GUI_STATE_DISABLED:
{
for (int i = startIndex; i < endIndex; i++)
{
if (i == auxActive) GuiListElement(RAYGUI_CLITERAL(Rectangle){ (float)posX, (float)bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), (float)elementWidth, (float)GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], true, false);
else GuiListElement(RAYGUI_CLITERAL(Rectangle){ (float)posX, (float)bounds.y + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + (i - startIndex)*(GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) + GuiGetStyle(LISTVIEW, ELEMENTS_PADDING)), (float)elementWidth, (float)GuiGetStyle(LISTVIEW, ELEMENTS_HEIGHT) }, text[i], false, false);
}
} break;
default: break;
}
if (scrollIndex != NULL) *scrollIndex = startIndex;
if (focus != NULL) *focus = focusElement;
*active = auxActive;
return pressed;
}
RAYGUIDEF Color GuiColorPanelEx(Rectangle bounds, Color color, float hue)
{
GuiControlState state = guiState;
Vector2 pickerSelector = { 0 };
Vector3 vcolor = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };
Vector3 hsv = ConvertRGBtoHSV(vcolor);
pickerSelector.x = bounds.x + (float)hsv.y*bounds.width; 
pickerSelector.y = bounds.y + (1.0f - (float)hsv.z)*bounds.height; 
Vector3 maxHue = { hue >= 0.0f ? hue : hsv.x, 1.0f, 1.0f };
Vector3 rgbHue = ConvertHSVtoRGB(maxHue);
Color maxHueCol = { (unsigned char)(255.0f*rgbHue.x),
(unsigned char)(255.0f*rgbHue.y),
(unsigned char)(255.0f*rgbHue.z), 255 };
const Color colWhite = { 255, 255, 255, 255 };
const Color colBlack = { 0, 0, 0, 255 };
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
{
state = GUI_STATE_PRESSED;
pickerSelector = mousePoint;
Vector2 colorPick = { pickerSelector.x - bounds.x, pickerSelector.y - bounds.y };
colorPick.x /= (float)bounds.width; 
colorPick.y /= (float)bounds.height; 
hsv.y = colorPick.x;
hsv.z = 1.0f - colorPick.y;
Vector3 rgb = ConvertHSVtoRGB(hsv);
color = RAYGUI_CLITERAL(Color){ (unsigned char)(255.0f*rgb.x),
(unsigned char)(255.0f*rgb.y),
(unsigned char)(255.0f*rgb.z),
(unsigned char)(255.0f*(float)color.a/255.0f) };
}
else state = GUI_STATE_FOCUSED;
}
}
if (state != GUI_STATE_DISABLED)
{
DrawRectangleGradientEx(bounds, Fade(colWhite, guiAlpha), Fade(colWhite, guiAlpha), Fade(maxHueCol, guiAlpha), Fade(maxHueCol, guiAlpha));
DrawRectangleGradientEx(bounds, Fade(colBlack, 0), Fade(colBlack, guiAlpha), Fade(colBlack, guiAlpha), Fade(colBlack, 0));
DrawRectangle(pickerSelector.x - GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE)/2, pickerSelector.y - GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE)/2, GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE), GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE), Fade(colWhite, guiAlpha));
}
else
{
DrawRectangleGradientEx(bounds, Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), guiAlpha), Fade(Fade(colBlack, 0.6f), guiAlpha), Fade(Fade(colBlack, 0.6f), guiAlpha), Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), 0.6f), guiAlpha));
}
DrawRectangleLinesEx(bounds, 1, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha));
return color;
}
RAYGUIDEF Color GuiColorPanel(Rectangle bounds, Color color)
{
return GuiColorPanelEx(bounds, color, -1.0f);
}
RAYGUIDEF float GuiColorBarAlpha(Rectangle bounds, float alpha)
{
#define COLORBARALPHA_CHECKED_SIZE 10
GuiControlState state = guiState;
Rectangle selector = { (float)bounds.x + alpha*bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (float)bounds.y - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (float)GuiGetStyle(COLORPICKER, BAR_SELECTOR_HEIGHT), (float)bounds.height + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)*2 };
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds) ||
CheckCollisionPointRec(mousePoint, selector))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
{
state = GUI_STATE_PRESSED;
selector.x = mousePoint.x - selector.width/2;
alpha = (mousePoint.x - bounds.x)/bounds.width;
if (alpha <= 0.0f) alpha = 0.0f;
if (alpha >= 1.0f) alpha = 1.0f;
}
else state = GUI_STATE_FOCUSED;
}
}
if (state != GUI_STATE_DISABLED)
{
int checksX = bounds.width/COLORBARALPHA_CHECKED_SIZE;
int checksY = bounds.height/COLORBARALPHA_CHECKED_SIZE;
for (int x = 0; x < checksX; x++)
{
for (int y = 0; y < checksY; y++)
{
DrawRectangle(bounds.x + x*COLORBARALPHA_CHECKED_SIZE, 
bounds.y + y*COLORBARALPHA_CHECKED_SIZE, 
COLORBARALPHA_CHECKED_SIZE, COLORBARALPHA_CHECKED_SIZE, 
((x + y)%2)? Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), 0.4f), guiAlpha) : 
Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.4f), guiAlpha));
}
}
DrawRectangleGradientEx(bounds, RAYGUI_CLITERAL(Color){ 255, 255, 255, 0 }, RAYGUI_CLITERAL(Color){ 255, 255, 255, 0 }, Fade(RAYGUI_CLITERAL(Color){ 0, 0, 0, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color){ 0, 0, 0, 255 }, guiAlpha));
}
else DrawRectangleGradientEx(bounds, Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), guiAlpha), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), guiAlpha));
DrawRectangleLinesEx(bounds, 1, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha));
DrawRectangleRec(selector, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha));
return alpha;
}
RAYGUIDEF float GuiColorBarHue(Rectangle bounds, float hue)
{
GuiControlState state = guiState;
Rectangle selector = { (float)bounds.x - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (float)bounds.y + hue/360.0f*bounds.height - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (float)bounds.width + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)*2, (float)GuiGetStyle(COLORPICKER, BAR_SELECTOR_HEIGHT) };
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds) ||
CheckCollisionPointRec(mousePoint, selector))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
{
state = GUI_STATE_PRESSED;
selector.y = mousePoint.y - selector.height/2;
hue = (mousePoint.y - bounds.y)*360/bounds.height;
if (hue <= 0.0f) hue = 0.0f;
if (hue >= 359.0f) hue = 359.0f;
}
else state = GUI_STATE_FOCUSED;
}
}
if (state != GUI_STATE_DISABLED)
{
DrawRectangleGradientV(bounds.x + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.y + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (int)bounds.height/6, Fade(RAYGUI_CLITERAL(Color){ 255,0,0,255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color){ 255,255,0,255 }, guiAlpha));
DrawRectangleGradientV(bounds.x + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.y + (int)bounds.height/6 + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (int)bounds.height/6, Fade(RAYGUI_CLITERAL(Color){ 255,255,0,255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color){ 0,255,0,255 }, guiAlpha));
DrawRectangleGradientV(bounds.x + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.y + 2*((int)bounds.height/6) + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (int)bounds.height/6, Fade(RAYGUI_CLITERAL(Color){ 0,255,0,255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color){ 0,255,255,255 }, guiAlpha));
DrawRectangleGradientV(bounds.x + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.y + 3*((int)bounds.height/6) + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (int)bounds.height/6, Fade(RAYGUI_CLITERAL(Color){ 0,255,255,255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color){ 0,0,255,255 }, guiAlpha));
DrawRectangleGradientV(bounds.x + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.y + 4*((int)bounds.height/6) + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (int)bounds.height/6, Fade(RAYGUI_CLITERAL(Color){ 0,0,255,255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color){ 255,0,255,255 }, guiAlpha));
DrawRectangleGradientV(bounds.x + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.y + 5*((int)bounds.height/6) + GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING)/2, bounds.width - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), (int)bounds.height/6 - GuiGetStyle(COLORPICKER, BAR_SELECTOR_PADDING), Fade(RAYGUI_CLITERAL(Color){ 255,0,255,255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color){ 255,0,0,255 }, guiAlpha));
}
else DrawRectangleGradientV(bounds.x, bounds.y, bounds.width, bounds.height, Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), guiAlpha), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), guiAlpha));
DrawRectangleLinesEx(bounds, 1, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha));
DrawRectangleRec(selector, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha));
return hue;
}
RAYGUIDEF Color GuiColorPicker(Rectangle bounds, Color color)
{
color = GuiColorPanel(bounds, color);
Rectangle boundsHue = { (float)bounds.x + bounds.width + GuiGetStyle(COLORPICKER, BAR_PADDING), (float)bounds.y, (float)GuiGetStyle(COLORPICKER, BAR_WIDTH), (float)bounds.height };
Vector3 hsv = ConvertRGBtoHSV(RAYGUI_CLITERAL(Vector3){ color.r/255.0f, color.g/255.0f, color.b/255.0f });
hsv.x = GuiColorBarHue(boundsHue, hsv.x);
Vector3 rgb = ConvertHSVtoRGB(hsv);
color = RAYGUI_CLITERAL(Color){ (unsigned char)(rgb.x*255.0f), (unsigned char)(rgb.y*255.0f), (unsigned char)(rgb.z*255.0f), color.a };
return color;
}
RAYGUIDEF int GuiMessageBox(Rectangle bounds, const char *windowTitle, const char *message, const char *buttons)
{
#define MESSAGEBOX_BUTTON_HEIGHT 24
#define MESSAGEBOX_BUTTON_PADDING 10
int clicked = -1; 
int buttonsCount = 0;
const char **buttonsText = GuiTextSplit(buttons, &buttonsCount, NULL);
Vector2 textSize = MeasureTextEx(guiFont, message, GuiGetStyle(DEFAULT, TEXT_SIZE), 1);
Rectangle textBounds = { 0 };
textBounds.x = bounds.x + bounds.width/2 - textSize.x/2;
textBounds.y = bounds.y + WINDOW_STATUSBAR_HEIGHT + (bounds.height - WINDOW_STATUSBAR_HEIGHT)/4 - textSize.y/2;
textBounds.width = textSize.x;
textBounds.height = textSize.y;
Rectangle buttonBounds = { 0 };
buttonBounds.x = bounds.x + MESSAGEBOX_BUTTON_PADDING;
buttonBounds.y = bounds.y + bounds.height/2 + bounds.height/4 - MESSAGEBOX_BUTTON_HEIGHT/2;
buttonBounds.width = (bounds.width - MESSAGEBOX_BUTTON_PADDING*(buttonsCount + 1))/buttonsCount;
buttonBounds.height = MESSAGEBOX_BUTTON_HEIGHT;
if (GuiWindowBox(bounds, windowTitle)) clicked = 0;
int prevTextAlignment = GuiGetStyle(LABEL, TEXT_ALIGNMENT);
GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
GuiLabel(textBounds, message);
GuiSetStyle(LABEL, TEXT_ALIGNMENT, prevTextAlignment);
prevTextAlignment = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
for (int i = 0; i < buttonsCount; i++)
{
if (GuiButton(buttonBounds, buttonsText[i])) clicked = i + 1;
buttonBounds.x += (buttonBounds.width + MESSAGEBOX_BUTTON_PADDING);
}
GuiSetStyle(BUTTON, TEXT_ALIGNMENT, prevTextAlignment);
return clicked;
}
RAYGUIDEF int GuiTextInputBox(Rectangle bounds, const char *windowTitle, const char *message, char *text, const char *buttons)
{
int btnIndex = -1;
return btnIndex;
}
RAYGUIDEF Vector2 GuiGrid(Rectangle bounds, float spacing, int subdivs)
{
#define GRID_COLOR_ALPHA 0.15f 
GuiControlState state = guiState;
Vector2 mousePoint = GetMousePosition();
Vector2 currentCell = { -1, -1 };
int linesV = ((int)(bounds.width/spacing) + 1)*subdivs;
int linesH = ((int)(bounds.height/spacing) + 1)*subdivs;
if ((state != GUI_STATE_DISABLED) && !guiLocked)
{
if (CheckCollisionPointRec(mousePoint, bounds))
{
currentCell.x = (int)((mousePoint.x - bounds.x)/spacing);
currentCell.y = (int)((mousePoint.y - bounds.y)/spacing);
}
}
switch (state)
{
case GUI_STATE_NORMAL:
{
for (int i = 0; i < linesV; i++)
{
DrawRectangleRec(RAYGUI_CLITERAL(Rectangle){ bounds.x + spacing*i, bounds.y, 1, bounds.height }, ((i%subdivs) == 0)? Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA*4) : Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA));
}
for (int i = 0; i < linesH; i++)
{
DrawRectangleRec(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y + spacing*i, bounds.width, 1 }, ((i%subdivs) == 0)? Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA*4) : Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA));
}
} break;
default: break;
}
return currentCell;
}
RAYGUIDEF void GuiLoadStyle(const char *fileName)
{
bool tryBinary = false;
FILE *rgsFile = fopen(fileName, "rt");
if (rgsFile != NULL)
{
char buffer[256] = { 0 };
fgets(buffer, 256, rgsFile);
if (buffer[0] == '#')
{
int controlId = 0;
int propertyId = 0;
int propertyValue = 0;
while (!feof(rgsFile))
{
switch (buffer[0])
{
case 'p':
{
sscanf(buffer, "p %d %d 0x%x", &controlId, &propertyId, &propertyValue);
if (controlId == 0) 
{
GuiSetStyle(0, propertyId, propertyValue);
if (propertyId < NUM_PROPS_DEFAULT) for (int i = 1; i < NUM_CONTROLS; i++) GuiSetStyle(i, propertyId, propertyValue);
}
else GuiSetStyle(controlId, propertyId, propertyValue);
} break;
case 'f':
{
int fontSize = 0;
int fontSpacing = 0;
char fontFileName[256] = { 0 };
sscanf(buffer, "f %d %d %[^\n]s", &fontSize, &fontSpacing, fontFileName);
Font font = LoadFontEx(FormatText("%s/%s", GetDirectoryPath(fileName), fontFileName), fontSize, NULL, 0);
if ((font.texture.id > 0) && (font.charsCount > 0))
{
GuiFont(font);
GuiSetStyle(DEFAULT, TEXT_SIZE, fontSize);
GuiSetStyle(DEFAULT, TEXT_SPACING, fontSpacing);
}
} break;
default: break;
}
fgets(buffer, 256, rgsFile);
}
}
else tryBinary = true;
fclose(rgsFile);
}
else return;
if (tryBinary)
{
rgsFile = fopen(fileName, "rb");
if (rgsFile == NULL) return;
char signature[5] = "";
short version = 0;
short reserved = 0;
int propertiesCount = 0;
fread(signature, 1, 4, rgsFile);
fread(&version, 1, sizeof(short), rgsFile);
fread(&reserved, 1, sizeof(short), rgsFile);
fread(&propertiesCount, 1, sizeof(int), rgsFile);
if ((signature[0] == 'r') &&
(signature[1] == 'G') &&
(signature[2] == 'S') &&
(signature[3] == ' '))
{
short controlId = 0;
short propertyId = 0;
int propertyValue = 0;
for (int i = 0; i < propertiesCount; i++)
{
fread(&controlId, 1, sizeof(short), rgsFile);
fread(&propertyId, 1, sizeof(short), rgsFile);
fread(&propertyValue, 1, sizeof(int), rgsFile);
if (controlId == 0) 
{
GuiSetStyle(0, (int)propertyId, propertyValue);
if (propertyId < NUM_PROPS_DEFAULT) for (int i = 1; i < NUM_CONTROLS; i++) GuiSetStyle(i, (int)propertyId, propertyValue);
}
else GuiSetStyle((int)controlId, (int)propertyId, propertyValue);
}
#if !defined(RAYGUI_STANDALONE)
int fontDataSize = 0;
fread(&fontDataSize, 1, sizeof(int), rgsFile);
if (fontDataSize > 0)
{
Font font = { 0 };
int fontType = 0; 
Rectangle whiteRec = { 0 };
fread(&font.baseSize, 1, sizeof(int), rgsFile);
fread(&font.charsCount, 1, sizeof(int), rgsFile);
fread(&fontType, 1, sizeof(int), rgsFile);
fread(&whiteRec, 1, sizeof(Rectangle), rgsFile);
int fontImageSize = 0;
fread(&fontImageSize, 1, sizeof(int), rgsFile);
if (fontImageSize > 0)
{
Image imFont = { 0 };
imFont.mipmaps = 1;
fread(&imFont.width, 1, sizeof(int), rgsFile);
fread(&imFont.height, 1, sizeof(int), rgsFile);
fread(&imFont.format, 1, sizeof(int), rgsFile);
imFont.data = (unsigned char *)malloc(fontImageSize);
fread(imFont.data, 1, fontImageSize, rgsFile);
font.texture = LoadTextureFromImage(imFont);
UnloadImage(imFont);
}
font.chars = (CharInfo *)calloc(font.charsCount, sizeof(CharInfo));
for (int i = 0; i < font.charsCount; i++)
{
fread(&font.recs[i], 1, sizeof(Rectangle), rgsFile);
fread(&font.chars[i].value, 1, sizeof(int), rgsFile);
fread(&font.chars[i].offsetX, 1, sizeof(int), rgsFile);
fread(&font.chars[i].offsetY, 1, sizeof(int), rgsFile);
fread(&font.chars[i].advanceX, 1, sizeof(int), rgsFile);
}
GuiFont(font);
if ((whiteRec.width != 0) && (whiteRec.height != 0)) SetShapesTexture(font.texture, whiteRec);
}
#endif
}
fclose(rgsFile);
}
}
RAYGUIDEF void GuiLoadStyleProps(const int *props, int count)
{
int completeSets = count/(NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED);
int uncompleteSetProps = count%(NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED);
for (int i = 0; i < completeSets; i++)
{
for (int j = 0; j < (NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED); j++) GuiSetStyle(i, j, props[i]);
}
for (int k = 0; k < uncompleteSetProps; k++) GuiSetStyle(completeSets, k, props[completeSets*(NUM_PROPS_DEFAULT + NUM_PROPS_EXTENDED) + k]);
}
RAYGUIDEF void GuiLoadStyleDefault(void)
{
guiStyleLoaded = true;
GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0x838383ff);
GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, 0xc9c9c9ff);
GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x686868ff);
GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, 0x5bb2d9ff);
GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, 0xc9effeff);
GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, 0x6c9bbcff);
GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, 0x0492c7ff);
GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, 0x97e8ffff);
GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, 0x368bafff);
GuiSetStyle(DEFAULT, BORDER_COLOR_DISABLED, 0xb5c1c2ff);
GuiSetStyle(DEFAULT, BASE_COLOR_DISABLED, 0xe6e9e9ff);
GuiSetStyle(DEFAULT, TEXT_COLOR_DISABLED, 0xaeb7b8ff);
GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);
GuiSetStyle(DEFAULT, INNER_PADDING, 1);
GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
for (int i = 1; i < NUM_CONTROLS; i++)
{
for (int j = 0; j < NUM_PROPS_DEFAULT; j++) GuiSetStyle(i, j, GuiGetStyle(DEFAULT, j));
}
guiFont = GetFontDefault(); 
GuiSetStyle(DEFAULT, TEXT_SIZE, 10);
GuiSetStyle(DEFAULT, TEXT_SPACING, 1);
GuiSetStyle(DEFAULT, LINE_COLOR, 0x90abb5ff); 
GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0xf5f5f5ff); 
GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
GuiSetStyle(BUTTON, INNER_PADDING, 4);
GuiSetStyle(TOGGLE, GROUP_PADDING, 2);
GuiSetStyle(SLIDER, SLIDER_WIDTH, 15);
GuiSetStyle(SLIDER, TEXT_PADDING, 5);
GuiSetStyle(CHECKBOX, CHECK_TEXT_PADDING, 5);
GuiSetStyle(COMBOBOX, SELECTOR_WIDTH, 30);
GuiSetStyle(COMBOBOX, SELECTOR_PADDING, 2);
GuiSetStyle(DROPDOWNBOX, ARROW_RIGHT_PADDING, 16);
GuiSetStyle(TEXTBOX, INNER_PADDING, 4);
GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
GuiSetStyle(TEXTBOX, MULTILINE_PADDING, 5);
GuiSetStyle(TEXTBOX, COLOR_SELECTED_FG, 0xf0fffeff);
GuiSetStyle(TEXTBOX, COLOR_SELECTED_BG, 0x839affe0);
GuiSetStyle(VALUEBOX, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
GuiSetStyle(SPINNER, SELECT_BUTTON_WIDTH, 20);
GuiSetStyle(SPINNER, SELECT_BUTTON_PADDING, 2);
GuiSetStyle(SPINNER, SELECT_BUTTON_BORDER_WIDTH, 1);
GuiSetStyle(SCROLLBAR, BORDER_WIDTH, 0);
GuiSetStyle(SCROLLBAR, ARROWS_VISIBLE, 0);
GuiSetStyle(SCROLLBAR, INNER_PADDING, 0);
GuiSetStyle(SCROLLBAR, ARROWS_SIZE, 6);
GuiSetStyle(SCROLLBAR, SLIDER_PADDING, 0);
GuiSetStyle(SCROLLBAR, SLIDER_SIZE, 16);
GuiSetStyle(SCROLLBAR, SCROLL_SPEED, 10);
GuiSetStyle(LISTVIEW, ELEMENTS_HEIGHT, 0x1e);
GuiSetStyle(LISTVIEW, ELEMENTS_PADDING, 2);
GuiSetStyle(LISTVIEW, SCROLLBAR_WIDTH, 10);
GuiSetStyle(LISTVIEW, SCROLLBAR_SIDE, SCROLLBAR_RIGHT_SIDE);
GuiSetStyle(COLORPICKER, COLOR_SELECTOR_SIZE, 6);
GuiSetStyle(COLORPICKER, BAR_WIDTH, 0x14);
GuiSetStyle(COLORPICKER, BAR_PADDING, 0xa);
GuiSetStyle(COLORPICKER, BAR_SELECTOR_HEIGHT, 6);
GuiSetStyle(COLORPICKER, BAR_SELECTOR_PADDING, 2);
}
RAYGUIDEF void GuiUpdateStyleComplete(void)
{
for (int i = 1; i < NUM_CONTROLS; i++)
{
for (int j = 0; j < NUM_PROPS_DEFAULT; j++) GuiSetStyle(i, j, GuiGetStyle(DEFAULT, j));
}
}
RAYGUIDEF const char *GuiIconText(int iconId, const char *text)
{
static char buffer[1024] = { 0 };
memset(buffer, 0, 1024);
sprintf(buffer, "#%03i#", iconId);
if (text != NULL)
{
for (int i = 5; i < 1024; i++)
{
buffer[i] = text[i - 5];
if (text[i - 5] == '\0') break;
}
}
return buffer;
}
static const char **GuiTextSplit(const char *text, int *count, int *textRow)
{
#define MAX_TEXT_BUFFER_LENGTH 1024
#define MAX_SUBSTRINGS_COUNT 64
static const char *result[MAX_SUBSTRINGS_COUNT] = { NULL };
static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };
memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);
result[0] = buffer;
int counter = 1;
if (textRow != NULL) textRow[0] = 0;
for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
{
buffer[i] = text[i];
if (buffer[i] == '\0') break;
else if ((buffer[i] == ';') || (buffer[i] == '\n'))
{
result[counter] = buffer + i + 1;
if (textRow != NULL)
{
if (buffer[i] == '\n') textRow[counter] = textRow[counter - 1] + 1;
else textRow[counter] = textRow[counter - 1];
}
buffer[i] = '\0'; 
counter++;
if (counter == MAX_SUBSTRINGS_COUNT) break;
}
}
*count = counter;
return result;
}
static Vector3 ConvertRGBtoHSV(Vector3 rgb)
{
Vector3 hsv = { 0.0f };
float min = 0.0f;
float max = 0.0f;
float delta = 0.0f;
min = (rgb.x < rgb.y)? rgb.x : rgb.y;
min = (min < rgb.z)? min : rgb.z;
max = (rgb.x > rgb.y)? rgb.x : rgb.y;
max = (max > rgb.z)? max : rgb.z;
hsv.z = max; 
delta = max - min;
if (delta < 0.00001f)
{
hsv.y = 0.0f;
hsv.x = 0.0f; 
return hsv;
}
if (max > 0.0f)
{
hsv.y = (delta/max); 
}
else
{
hsv.y = 0.0f;
hsv.x = 0.0f; 
return hsv;
}
if (rgb.x >= max) hsv.x = (rgb.y - rgb.z)/delta; 
else
{
if (rgb.y >= max) hsv.x = 2.0f + (rgb.z - rgb.x)/delta; 
else hsv.x = 4.0f + (rgb.x - rgb.y)/delta; 
}
hsv.x *= 60.0f; 
if (hsv.x < 0.0f) hsv.x += 360.0f;
return hsv;
}
static Vector3 ConvertHSVtoRGB(Vector3 hsv)
{
Vector3 rgb = { 0.0f };
float hh = 0.0f, p = 0.0f, q = 0.0f, t = 0.0f, ff = 0.0f;
long i = 0;
if (hsv.y <= 0.0f)
{
rgb.x = hsv.z;
rgb.y = hsv.z;
rgb.z = hsv.z;
return rgb;
}
hh = hsv.x;
if (hh >= 360.0f) hh = 0.0f;
hh /= 60.0f;
i = (long)hh;
ff = hh - i;
p = hsv.z*(1.0f - hsv.y);
q = hsv.z*(1.0f - (hsv.y*ff));
t = hsv.z*(1.0f - (hsv.y*(1.0f - ff)));
switch (i)
{
case 0:
{
rgb.x = hsv.z;
rgb.y = t;
rgb.z = p;
} break;
case 1:
{
rgb.x = q;
rgb.y = hsv.z;
rgb.z = p;
} break;
case 2:
{
rgb.x = p;
rgb.y = hsv.z;
rgb.z = t;
} break;
case 3:
{
rgb.x = p;
rgb.y = q;
rgb.z = hsv.z;
} break;
case 4:
{
rgb.x = t;
rgb.y = p;
rgb.z = hsv.z;
} break;
case 5:
default:
{
rgb.x = hsv.z;
rgb.y = p;
rgb.z = q;
} break;
}
return rgb;
}
#if defined(RAYGUI_STANDALONE)
static Color GetColor(int hexValue)
{
Color color;
color.r = (unsigned char)(hexValue >> 24) & 0xFF;
color.g = (unsigned char)(hexValue >> 16) & 0xFF;
color.b = (unsigned char)(hexValue >> 8) & 0xFF;
color.a = (unsigned char)hexValue & 0xFF;
return color;
}
static int ColorToInt(Color color)
{
return (((int)color.r << 24) | ((int)color.g << 16) | ((int)color.b << 8) | (int)color.a);
}
static bool CheckCollisionPointRec(Vector2 point, Rectangle rec)
{
bool collision = false;
if ((point.x >= rec.x) && (point.x <= (rec.x + rec.width)) &&
(point.y >= rec.y) && (point.y <= (rec.y + rec.height))) collision = true;
return collision;
}
static Color Fade(Color color, float alpha)
{
if (alpha < 0.0f) alpha = 0.0f;
else if (alpha > 1.0f) alpha = 1.0f;
return RAYGUI_CLITERAL(Color){ color.r, color.g, color.b, (unsigned char)(255.0f*alpha) };
}
static const char *TextFormat(const char *text, ...)
{
#define MAX_FORMATTEXT_LENGTH 64
static char buffer[MAX_FORMATTEXT_LENGTH];
va_list args;
va_start(args, text);
vsprintf(buffer, text, args);
va_end(args);
return buffer;
}
static void DrawRectangleRec(Rectangle rec, Color color)
{ 
DrawRectangle(rec.x, rec.y, rec.width, rec.height, color);
}
static void DrawRectangleLinesEx(Rectangle rec, int lineThick, Color color)
{
DrawRectangle(rec.x, rec.y, rec.width, lineThick, color);
DrawRectangle(rec.x, rec.y + lineThick, lineThick, rec.height - 2*lineThick, color);
DrawRectangle(rec.x + rec.width - lineThick, rec.y + lineThick, lineThick, rec.height - 2*lineThick, color);
DrawRectangle(rec.x, rec.y + rec.height - lineThick, rec.width, lineThick, color);
}
static void DrawRectangleGradientV(int posX, int posY, int width, int height, Color color1, Color color2)
{
Rectangle bounds = { (float)posX, (float)posY, (float)width, (float)height };
DrawRectangleGradientEx(bounds, color1, color2, color2, color1);
}
#endif 
#endif
