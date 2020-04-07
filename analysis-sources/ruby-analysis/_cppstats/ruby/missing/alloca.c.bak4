


























#if !defined(lint)
static char SCCSid[] = "@(#)alloca.c 1.1"; 
#endif

#include "ruby/config.h"
#define X3J11 1 

#if defined(C_ALLOCA)

#if defined(emacs)
#if defined(static)




#if !defined(STACK_DIRECTION)
you
lose
-- must know STACK_DIRECTION at compile-time
#endif 
#endif 
#endif 

#if defined(X3J11)
typedef void *pointer; 
#else
typedef char *pointer; 
#endif 

#if !defined(NULL)
#define NULL 0 
#endif

#define xmalloc ruby_xmalloc
#define xfree ruby_xfree

extern void xfree();
extern pointer xmalloc();











#if !defined(STACK_DIRECTION)
#define STACK_DIRECTION 0 
#endif

#if STACK_DIRECTION != 0

#define STACK_DIR STACK_DIRECTION 

#else 

static int stack_dir; 
#define STACK_DIR stack_dir

static void
find_stack_direction ()
{
static char *addr = NULL; 

auto char dummy; 

if (addr == NULL)
{ 
addr = &dummy;

find_stack_direction (); 
}
else 
if (&dummy > addr)
stack_dir = 1; 
else
stack_dir = -1; 
}

#endif 










#if !defined(ALIGN_SIZE)
#define ALIGN_SIZE sizeof(double)
#endif

typedef union hdr
{
char align[ALIGN_SIZE]; 
struct
{
union hdr *next; 
char *deep; 
} h;
} header;










static header *last_alloca_header = NULL; 

pointer
alloca (size) 
unsigned size; 
{
auto char probe; 
register char *depth = &probe;

#if STACK_DIRECTION == 0
if (STACK_DIR == 0) 
find_stack_direction ();
#endif



{
register header *hp; 

for (hp = last_alloca_header; hp != NULL;)
if (STACK_DIR > 0 && hp->h.deep > depth
|| STACK_DIR < 0 && hp->h.deep < depth)
{
register header *np = hp->h.next;

xfree ((pointer) hp); 

hp = np; 
}
else
break; 

last_alloca_header = hp; 
}

if (size == 0)
return NULL; 



{
register pointer new = xmalloc (sizeof (header) + size);


((header *)new)->h.next = last_alloca_header;
((header *)new)->h.deep = depth;

last_alloca_header = (header *)new;



return (pointer)((char *)new + sizeof(header));
}
}

#endif
