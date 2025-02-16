


















#if !defined(GRUB_PARSER_HEADER)
#define GRUB_PARSER_HEADER 1

#include <grub/types.h>
#include <grub/err.h>
#include <grub/handler.h>
#include <grub/reader.h>


typedef enum
{
GRUB_PARSER_STATE_TEXT = 1,
GRUB_PARSER_STATE_ESC,
GRUB_PARSER_STATE_QUOTE,
GRUB_PARSER_STATE_DQUOTE,
GRUB_PARSER_STATE_VAR,
GRUB_PARSER_STATE_VARNAME,
GRUB_PARSER_STATE_VARNAME2,
GRUB_PARSER_STATE_QVAR,
GRUB_PARSER_STATE_QVARNAME,
GRUB_PARSER_STATE_QVARNAME2
} grub_parser_state_t;


struct grub_parser_state_transition
{

grub_parser_state_t from_state;


grub_parser_state_t to_state;


char input;


int keep_value;
};


grub_parser_state_t grub_parser_cmdline_state (grub_parser_state_t state,
char c, char *result);

grub_err_t grub_parser_split_cmdline (const char *cmdline,
grub_reader_getline_t getline,
void *closure, int *argc, char ***argv);

struct grub_parser
{

struct grub_parser *next;


const char *name;


grub_err_t (*init) (void);


grub_err_t (*fini) (void);

grub_err_t (*parse_line) (char *line, grub_reader_getline_t getline,
void *closure);
};
typedef struct grub_parser *grub_parser_t;

extern struct grub_handler_class grub_parser_class;
grub_err_t grub_parser_execute (char *source);

#define grub_parser_register(name, parser) grub_parser_register_internal (parser); GRUB_MODATTR ("handler", "parser." name);



static inline void
grub_parser_register_internal (grub_parser_t parser)
{
grub_handler_register (&grub_parser_class, GRUB_AS_HANDLER (parser));
}

static inline void
grub_parser_unregister (grub_parser_t parser)
{
grub_handler_unregister (&grub_parser_class, GRUB_AS_HANDLER (parser));
}

static inline grub_parser_t
grub_parser_get_current (void)
{
return (grub_parser_t) grub_parser_class.cur_handler;
}

static inline grub_err_t
grub_parser_set_current (grub_parser_t parser)
{
return grub_handler_set_current (&grub_parser_class,
GRUB_AS_HANDLER (parser));
}

void grub_register_rescue_parser (void);

#endif 
