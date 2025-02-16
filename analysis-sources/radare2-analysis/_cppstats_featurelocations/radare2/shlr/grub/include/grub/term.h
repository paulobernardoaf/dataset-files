

















#include <grub/list.h>

#if !defined(GRUB_TERM_HEADER)
#define GRUB_TERM_HEADER 1


#define GRUB_TERM_LEFT 2 
#define GRUB_TERM_RIGHT 6 
#define GRUB_TERM_UP 0xfa
#define GRUB_TERM_DOWN 0xfb
#define GRUB_TERM_HOME 1 
#define GRUB_TERM_END 5 
#define GRUB_TERM_DC 4 
#define GRUB_TERM_IC 24 
#define GRUB_TERM_PPAGE 7
#define GRUB_TERM_NPAGE 3
#define GRUB_TERM_ESC '\e'
#define GRUB_TERM_TAB '\t'
#define GRUB_TERM_BACKSPACE 8

#define GRUB_TERM_F1 0xf0
#define GRUB_TERM_F2 0xf1
#define GRUB_TERM_F3 0xf2
#define GRUB_TERM_F4 0xf3
#define GRUB_TERM_F5 0xf4
#define GRUB_TERM_F6 0xf5
#define GRUB_TERM_F7 0xf6
#define GRUB_TERM_F8 0xf7
#define GRUB_TERM_F9 0xf8
#define GRUB_TERM_F10 0xf9

#define GRUB_TERM_CTRL_A 1
#define GRUB_TERM_CTRL_B 2
#define GRUB_TERM_CTRL_C 3
#define GRUB_TERM_CTRL_D 4
#define GRUB_TERM_CTRL_E 5
#define GRUB_TERM_CTRL_F 6
#define GRUB_TERM_CTRL_G 7
#define GRUB_TERM_CTRL_H 8
#define GRUB_TERM_CTRL_I 9
#define GRUB_TERM_CTRL_J 10
#define GRUB_TERM_CTRL_K 11
#define GRUB_TERM_CTRL_L 12
#define GRUB_TERM_CTRL_M 13
#define GRUB_TERM_CTRL_N 14
#define GRUB_TERM_CTRL_O 15
#define GRUB_TERM_CTRL_P 16
#define GRUB_TERM_CTRL_Q 17
#define GRUB_TERM_CTRL_R 18
#define GRUB_TERM_CTRL_S 19
#define GRUB_TERM_CTRL_T 20
#define GRUB_TERM_CTRL_U 21
#define GRUB_TERM_CTRL_V 22
#define GRUB_TERM_CTRL_W 23
#define GRUB_TERM_CTRL_X 24
#define GRUB_TERM_CTRL_Y 25
#define GRUB_TERM_CTRL_Z 26

#if !defined(ASM_FILE)

#include <grub/err.h>
#include <grub/symbol.h>
#include <grub/types.h>



typedef enum
{


GRUB_TERM_COLOR_STANDARD,

GRUB_TERM_COLOR_NORMAL,

GRUB_TERM_COLOR_HIGHLIGHT
}
grub_term_color_state;











#define GRUB_TERM_NO_ECHO (1 << 0)

#define GRUB_TERM_NO_EDIT (1 << 1)

#define GRUB_TERM_DUMB (1 << 2)



#define GRUB_TERM_STATUS_SHIFT (1 << 0)
#define GRUB_TERM_STATUS_CTRL (1 << 1)
#define GRUB_TERM_STATUS_ALT (1 << 2)



#define GRUB_TERM_DISP_LEFT 0x2190
#define GRUB_TERM_DISP_UP 0x2191
#define GRUB_TERM_DISP_RIGHT 0x2192
#define GRUB_TERM_DISP_DOWN 0x2193
#define GRUB_TERM_DISP_HLINE 0x2501
#define GRUB_TERM_DISP_VLINE 0x2503
#define GRUB_TERM_DISP_UL 0x250F
#define GRUB_TERM_DISP_UR 0x2513
#define GRUB_TERM_DISP_LL 0x2517
#define GRUB_TERM_DISP_LR 0x251B
#define GRUB_TERM_DISP_DHLINE 0x2550
#define GRUB_TERM_DISP_DVLINE 0x2551
#define GRUB_TERM_DISP_DUL 0x2554
#define GRUB_TERM_DISP_DUR 0x2557
#define GRUB_TERM_DISP_DLL 0x255A
#define GRUB_TERM_DISP_DLR 0x255D





#define GRUB_TERM_INFO_HEIGHT 1


#define GRUB_TERM_MARGIN 1


#define GRUB_TERM_SCROLL_WIDTH 1


#define GRUB_TERM_TOP_BORDER_Y (GRUB_TERM_MARGIN + GRUB_TERM_INFO_HEIGHT + GRUB_TERM_MARGIN)



#define GRUB_TERM_LEFT_BORDER_X GRUB_TERM_MARGIN


#define GRUB_TERM_MESSAGE_HEIGHT 8


#define GRUB_TERM_FIRST_ENTRY_Y (GRUB_TERM_TOP_BORDER_Y + 1)

struct grub_term_input
{

struct grub_term_input *next;


const char *name;


grub_err_t (*init) (void);


grub_err_t (*fini) (void);


int (*checkkey) (void);


int (*getkey) (void);


int (*getkeystatus) (void);
};
typedef struct grub_term_input *grub_term_input_t;

struct grub_term_output
{

struct grub_term_output *next;


const char *name;


grub_err_t (*init) (void);


grub_err_t (*fini) (void);


void (*putchar) (grub_uint32_t c);



grub_ssize_t (*getcharwidth) (grub_uint32_t c);


grub_uint16_t (*getwh) (void);


grub_uint16_t (*getxy) (void);


void (*gotoxy) (grub_uint8_t x, grub_uint8_t y);


void (*cls) (void);


void (*setcolorstate) (grub_term_color_state state);



void (*setcolor) (grub_uint8_t normal_color, grub_uint8_t highlight_color);



void (*getcolor) (grub_uint8_t *normal_color, grub_uint8_t *highlight_color);


void (*setcursor) (int on);


void (*refresh) (void);


grub_uint32_t flags;
};
typedef struct grub_term_output *grub_term_output_t;

extern struct grub_term_output *grub_term_outputs_disabled;
extern struct grub_term_input *grub_term_inputs_disabled;
extern struct grub_term_output *grub_term_outputs;
extern struct grub_term_input *grub_term_inputs;

#define grub_term_register_input(name, term) grub_term_register_input_internal (term); GRUB_MODATTR ("terminal", "i" name);



#define grub_term_register_output(name, term) grub_term_register_output_internal (term); GRUB_MODATTR ("terminal", "o" name);



static inline void
grub_term_register_input_internal (grub_term_input_t term)
{
if (grub_term_inputs)
grub_list_push (GRUB_AS_LIST_P (&grub_term_inputs_disabled),
GRUB_AS_LIST (term));
else
{

if (! term->init || term->init () == GRUB_ERR_NONE)
grub_list_push (GRUB_AS_LIST_P (&grub_term_inputs), GRUB_AS_LIST (term));
}
}

static inline void
grub_term_register_output_internal (grub_term_output_t term)
{
if (grub_term_outputs)
grub_list_push (GRUB_AS_LIST_P (&grub_term_outputs_disabled),
GRUB_AS_LIST (term));
else
{

if (! term->init || term->init () == GRUB_ERR_NONE)
grub_list_push (GRUB_AS_LIST_P (&grub_term_outputs),
GRUB_AS_LIST (term));
}
}

static inline void
grub_term_unregister_input (grub_term_input_t term)
{
grub_list_remove (GRUB_AS_LIST_P (&grub_term_inputs), GRUB_AS_LIST (term));
grub_list_remove (GRUB_AS_LIST_P (&grub_term_inputs_disabled),
GRUB_AS_LIST (term));
}

static inline void
grub_term_unregister_output (grub_term_output_t term)
{
grub_list_remove (GRUB_AS_LIST_P (&grub_term_outputs), GRUB_AS_LIST (term));
grub_list_remove (GRUB_AS_LIST_P (&(grub_term_outputs_disabled)),
GRUB_AS_LIST (term));
}

#define FOR_ACTIVE_TERM_INPUTS(var) for (var = grub_term_inputs; var; var = var->next)
#define FOR_DISABLED_TERM_INPUTS(var) for (var = grub_term_inputs_disabled; var; var = var->next)
#define FOR_ACTIVE_TERM_OUTPUTS(var) for (var = grub_term_outputs; var; var = var->next)
#define FOR_DISABLED_TERM_OUTPUTS(var) for (var = grub_term_outputs_disabled; var; var = var->next)

void grub_putchar (int c);
void grub_putcode (grub_uint32_t code,
struct grub_term_output *term);
int grub_getkey (void);
int grub_checkkey (void);
int grub_getkeystatus (void);
void grub_cls (void);
void grub_setcolorstate (grub_term_color_state state);
void grub_refresh (void);
void grub_puts_terminal (const char *str, struct grub_term_output *term);
grub_uint16_t *grub_term_save_pos (void);
void grub_term_restore_pos (grub_uint16_t *pos);

static inline unsigned grub_term_width (struct grub_term_output *term)
{
return ((term->getwh()&0xFF00)>>8);
}

static inline unsigned grub_term_height (struct grub_term_output *term)
{
return (term->getwh()&0xFF);
}


static inline unsigned
grub_term_border_width (struct grub_term_output *term)
{
return grub_term_width (term) - GRUB_TERM_MARGIN * 3 - GRUB_TERM_SCROLL_WIDTH;
}



static inline int
grub_term_entry_width (struct grub_term_output *term)
{
return grub_term_border_width (term) - 2 - GRUB_TERM_MARGIN * 2 - 1;
}



static inline unsigned
grub_term_border_height (struct grub_term_output *term)
{
return grub_term_height (term) - GRUB_TERM_TOP_BORDER_Y
- GRUB_TERM_MESSAGE_HEIGHT;
}


static inline int
grub_term_num_entries (struct grub_term_output *term)
{
return grub_term_border_height (term) - 2;
}

static inline int
grub_term_cursor_x (struct grub_term_output *term)
{
return (GRUB_TERM_LEFT_BORDER_X + grub_term_border_width (term)
- GRUB_TERM_MARGIN - 1);
}

static inline grub_uint16_t
grub_term_getxy (struct grub_term_output *term)
{
return term->getxy ();
}

static inline void
grub_term_refresh (struct grub_term_output *term)
{
if (term->refresh)
term->refresh ();
}

static inline void
grub_term_gotoxy (struct grub_term_output *term, grub_uint8_t x, grub_uint8_t y)
{
term->gotoxy (x, y);
}

static inline void
grub_term_setcolorstate (struct grub_term_output *term,
grub_term_color_state state)
{
if (term->setcolorstate)
term->setcolorstate (state);
}



static inline void
grub_term_setcolor (struct grub_term_output *term,
grub_uint8_t normal_color, grub_uint8_t highlight_color)
{
if (term->setcolor)
term->setcolor (normal_color, highlight_color);
}


static inline void
grub_term_setcursor (struct grub_term_output *term, int on)
{
if (term->setcursor)
term->setcursor (on);
}

static inline void
grub_term_cls (struct grub_term_output *term)
{
if (term->cls)
(term->cls) ();
else
{
grub_putcode ('\n', term);
grub_term_refresh (term);
}
}

static inline grub_ssize_t
grub_term_getcharwidth (struct grub_term_output *term, grub_uint32_t c)
{
if (term->getcharwidth)
return term->getcharwidth (c);
else
return 1;
}

static inline void
grub_term_getcolor (struct grub_term_output *term,
grub_uint8_t *normal_color, grub_uint8_t *highlight_color)
{
if (term->getcolor)
term->getcolor (normal_color, highlight_color);
else
{
*normal_color = 0x07;
*highlight_color = 0x07;
}
}

extern void (*grub_newline_hook) (void);

struct grub_term_autoload
{
struct grub_term_autoload *next;
char *name;
char *modname;
};

extern struct grub_term_autoload *grub_term_input_autoload;
extern struct grub_term_autoload *grub_term_output_autoload;

static inline void
grub_print_spaces (struct grub_term_output *term, int number_spaces)
{
while (--number_spaces >= 0)
grub_putcode (' ', term);
}



#define GRUB_TERM_ASCII_CHAR(c) ((c) & 0xff)

#endif 

#endif 
