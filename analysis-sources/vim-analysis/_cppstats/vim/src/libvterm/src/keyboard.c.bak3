#include "vterm_internal.h"

#include <stdio.h>

#include "utf8.h"

int vterm_is_modify_other_keys(VTerm *vt)
{
return vt->state->mode.modify_other_keys;
}


void vterm_keyboard_unichar(VTerm *vt, uint32_t c, VTermModifier mod)
{
int needs_CSIu;

if (vt->state->mode.modify_other_keys && mod != 0) {
vterm_push_output_sprintf_ctrl(vt, C1_CSI, "27;%d;%d~", mod+1, c);
return;
}



if(c != ' ')
mod &= ~VTERM_MOD_SHIFT;

if(mod == 0) {

char str[6];
int seqlen = fill_utf8(c, str);
vterm_push_output_bytes(vt, str, seqlen);
return;
}

switch(c) {

case 'i': case 'j': case 'm': case '[':
needs_CSIu = 1;
break;

case '\\': case ']': case '^': case '_':
needs_CSIu = 0;
break;

case ' ':
needs_CSIu = !!(mod & VTERM_MOD_SHIFT);
break;

default:
needs_CSIu = (c < 'a' || c > 'z');
}


if(needs_CSIu && (mod & ~VTERM_MOD_ALT)) {
vterm_push_output_sprintf_ctrl(vt, C1_CSI, "%d;%du", c, mod+1);
return;
}

if(mod & VTERM_MOD_CTRL)
c &= 0x1f;

vterm_push_output_sprintf(vt, "%s%c", mod & VTERM_MOD_ALT ? ESC_S : "", c);
}

typedef struct {
enum {
KEYCODE_NONE,
KEYCODE_LITERAL,
KEYCODE_TAB,
KEYCODE_ENTER,
KEYCODE_SS3,
KEYCODE_CSI,
KEYCODE_CSI_CURSOR,
KEYCODE_CSINUM,
KEYCODE_KEYPAD,
} type;
char literal;
int csinum;
} keycodes_s;


static keycodes_s keycodes[] = {
{ KEYCODE_NONE, 0, 0 }, 

{ KEYCODE_ENTER, '\r', 0 }, 
{ KEYCODE_TAB, '\t', 0 }, 
{ KEYCODE_LITERAL, '\x7f', 0 }, 
{ KEYCODE_LITERAL, '\x1b', 0 }, 

{ KEYCODE_CSI_CURSOR, 'A', 0 }, 
{ KEYCODE_CSI_CURSOR, 'B', 0 }, 
{ KEYCODE_CSI_CURSOR, 'D', 0 }, 
{ KEYCODE_CSI_CURSOR, 'C', 0 }, 

{ KEYCODE_CSINUM, '~', 2 }, 
{ KEYCODE_CSINUM, '~', 3 }, 
{ KEYCODE_CSI_CURSOR, 'H', 0 }, 
{ KEYCODE_CSI_CURSOR, 'F', 0 }, 
{ KEYCODE_CSINUM, '~', 5 }, 
{ KEYCODE_CSINUM, '~', 6 }, 
};

static keycodes_s keycodes_fn[] = {
{ KEYCODE_NONE, 0, 0 }, 
{ KEYCODE_CSI_CURSOR, 'P', 0 }, 
{ KEYCODE_CSI_CURSOR, 'Q', 0 }, 
{ KEYCODE_CSI_CURSOR, 'R', 0 }, 
{ KEYCODE_CSI_CURSOR, 'S', 0 }, 
{ KEYCODE_CSINUM, '~', 15 }, 
{ KEYCODE_CSINUM, '~', 17 }, 
{ KEYCODE_CSINUM, '~', 18 }, 
{ KEYCODE_CSINUM, '~', 19 }, 
{ KEYCODE_CSINUM, '~', 20 }, 
{ KEYCODE_CSINUM, '~', 21 }, 
{ KEYCODE_CSINUM, '~', 23 }, 
{ KEYCODE_CSINUM, '~', 24 }, 
};

static keycodes_s keycodes_kp[] = {
{ KEYCODE_KEYPAD, '0', 'p' }, 
{ KEYCODE_KEYPAD, '1', 'q' }, 
{ KEYCODE_KEYPAD, '2', 'r' }, 
{ KEYCODE_KEYPAD, '3', 's' }, 
{ KEYCODE_KEYPAD, '4', 't' }, 
{ KEYCODE_KEYPAD, '5', 'u' }, 
{ KEYCODE_KEYPAD, '6', 'v' }, 
{ KEYCODE_KEYPAD, '7', 'w' }, 
{ KEYCODE_KEYPAD, '8', 'x' }, 
{ KEYCODE_KEYPAD, '9', 'y' }, 
{ KEYCODE_KEYPAD, '*', 'j' }, 
{ KEYCODE_KEYPAD, '+', 'k' }, 
{ KEYCODE_KEYPAD, ',', 'l' }, 
{ KEYCODE_KEYPAD, '-', 'm' }, 
{ KEYCODE_KEYPAD, '.', 'n' }, 
{ KEYCODE_KEYPAD, '/', 'o' }, 
{ KEYCODE_KEYPAD, '\n', 'M' }, 
{ KEYCODE_KEYPAD, '=', 'X' }, 
};

void vterm_keyboard_key(VTerm *vt, VTermKey key, VTermModifier mod)
{
keycodes_s k;

if(key == VTERM_KEY_NONE)
return;

if(key < VTERM_KEY_FUNCTION_0) {
if(key >= sizeof(keycodes)/sizeof(keycodes[0]))
return;
k = keycodes[key];
}
else if(key >= VTERM_KEY_FUNCTION_0 && key <= VTERM_KEY_FUNCTION_MAX) {
if((key - VTERM_KEY_FUNCTION_0) >= sizeof(keycodes_fn)/sizeof(keycodes_fn[0]))
return;
k = keycodes_fn[key - VTERM_KEY_FUNCTION_0];
}
else if(key >= VTERM_KEY_KP_0) {
if((key - VTERM_KEY_KP_0) >= sizeof(keycodes_kp)/sizeof(keycodes_kp[0]))
return;
k = keycodes_kp[key - VTERM_KEY_KP_0];
}

switch(k.type) {
case KEYCODE_NONE:
break;

case KEYCODE_TAB:

if(mod == VTERM_MOD_SHIFT)
vterm_push_output_sprintf_ctrl(vt, C1_CSI, "Z");
else if(mod & VTERM_MOD_SHIFT)
vterm_push_output_sprintf_ctrl(vt, C1_CSI, "1;%dZ", mod+1);
else
goto case_LITERAL;
break;

case KEYCODE_ENTER:

if(vt->state->mode.newline)
vterm_push_output_sprintf(vt, "\r\n");
else
goto case_LITERAL;
break;

case KEYCODE_LITERAL: case_LITERAL:
if(mod & (VTERM_MOD_SHIFT|VTERM_MOD_CTRL))
vterm_push_output_sprintf_ctrl(vt, C1_CSI, "%d;%du", k.literal, mod+1);
else
vterm_push_output_sprintf(vt, mod & VTERM_MOD_ALT ? ESC_S "%c" : "%c", k.literal);
break;

case KEYCODE_SS3: case_SS3:
if(mod == 0)
vterm_push_output_sprintf_ctrl(vt, C1_SS3, "%c", k.literal);
else
goto case_CSI;
break;

case KEYCODE_CSI: case_CSI:
if(mod == 0)
vterm_push_output_sprintf_ctrl(vt, C1_CSI, "%c", k.literal);
else
vterm_push_output_sprintf_ctrl(vt, C1_CSI, "1;%d%c", mod + 1, k.literal);
break;

case KEYCODE_CSINUM:
if(mod == 0)
vterm_push_output_sprintf_ctrl(vt, C1_CSI, "%d%c", k.csinum, k.literal);
else
vterm_push_output_sprintf_ctrl(vt, C1_CSI, "%d;%d%c", k.csinum, mod + 1, k.literal);
break;

case KEYCODE_CSI_CURSOR:
if(vt->state->mode.cursor)
goto case_SS3;
else
goto case_CSI;

case KEYCODE_KEYPAD:
if(vt->state->mode.keypad) {
k.literal = k.csinum;
goto case_SS3;
}
else
goto case_LITERAL;
}
}

void vterm_keyboard_start_paste(VTerm *vt)
{
if(vt->state->mode.bracketpaste)
vterm_push_output_sprintf_ctrl(vt, C1_CSI, "200~");
}

void vterm_keyboard_end_paste(VTerm *vt)
{
if(vt->state->mode.bracketpaste)
vterm_push_output_sprintf_ctrl(vt, C1_CSI, "201~");
}
