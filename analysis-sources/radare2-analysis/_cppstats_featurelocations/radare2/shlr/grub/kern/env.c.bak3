


















#include <grub/env.h>
#include <grub/env_private.h>
#include <grub/misc.h>
#include <grub/mm.h>

GRUB_EXPORT(grub_env_set);
GRUB_EXPORT(grub_env_get);
GRUB_EXPORT(grub_env_unset);
GRUB_EXPORT(grub_env_iterate);
GRUB_EXPORT(grub_env_find);
GRUB_EXPORT(grub_register_variable_hook);
GRUB_EXPORT(grub_current_menu);
GRUB_EXPORT(grub_current_context);

static struct menu_pointer initial_menu;
struct menu_pointer *grub_current_menu = &initial_menu;


static struct grub_env_context initial_context;


struct grub_env_context *grub_current_context = &initial_context;


static unsigned int
grub_env_hashval (const char *s)
{
unsigned int i = 0;


while (*s)
i += 5 * *(s++);

return i % HASHSZ;
}

struct grub_env_var *
grub_env_find (const char *name)
{
struct grub_env_var *var;
int idx = grub_env_hashval (name);


for (var = grub_current_context->vars[idx]; var; var = var->next)
if (grub_strcmp (var->name, name) == 0)
return var;

return 0;
}

static void
grub_env_insert (struct grub_env_context *context,
struct grub_env_var *var)
{
int idx = grub_env_hashval (var->name);


var->prevp = &context->vars[idx];
var->next = context->vars[idx];
if (var->next)
var->next->prevp = &(var->next);
context->vars[idx] = var;
}

static void
grub_env_remove (struct grub_env_var *var)
{

*var->prevp = var->next;
if (var->next)
var->next->prevp = var->prevp;
}

grub_err_t
grub_env_set (const char *name, const char *val)
{
struct grub_env_var *var;


var = grub_env_find (name);
if (var)
{
char *old = var->value;

if (var->write_hook)
var->value = var->write_hook (var, val);
else
var->value = grub_strdup (val);

if (! var->value)
{
var->value = old;
return grub_errno;
}

grub_free (old);
return GRUB_ERR_NONE;
}


var = grub_zalloc (sizeof (*var));
if (! var)
return grub_errno;


var->global = 0;

var->name = grub_strdup (name);
if (! var->name)
goto fail;

var->value = grub_strdup (val);
if (! var->value)
goto fail;

grub_env_insert (grub_current_context, var);

return GRUB_ERR_NONE;

fail:
grub_free (var->name);
grub_free (var->value);
grub_free (var);

return grub_errno;
}

char *
grub_env_get (const char *name)
{
struct grub_env_var *var;

var = grub_env_find (name);
if (! var)
return 0;

if (var->read_hook)
return var->read_hook (var, var->value);

return var->value;
}

void
grub_env_unset (const char *name)
{
struct grub_env_var *var;

var = grub_env_find (name);
if (! var)
return;

if (var->read_hook || var->write_hook)
{
grub_env_set (name, "");
return;
}

grub_env_remove (var);

grub_free (var->name);
grub_free (var->value);
grub_free (var);
}

void
grub_env_iterate (int (*func) (struct grub_env_var *var, void *closure),
void *closure)
{
struct grub_env_sorted_var *sorted_list = 0;
struct grub_env_sorted_var *sorted_var;
int i;


for (i = 0; i < HASHSZ; i++)
{
struct grub_env_var *var;

for (var = grub_current_context->vars[i]; var; var = var->next)
{
struct grub_env_sorted_var *p, **q;

sorted_var = grub_malloc (sizeof (*sorted_var));
if (! sorted_var)
goto fail;

sorted_var->var = var;

for (q = &sorted_list, p = *q; p; q = &((*q)->next), p = *q)
{
if (grub_strcmp (p->var->name, var->name) > 0)
break;
}

sorted_var->next = *q;
*q = sorted_var;
}
}


for (sorted_var = sorted_list; sorted_var; sorted_var = sorted_var->next)
if (func (sorted_var->var, closure))
break;

fail:


for (sorted_var = sorted_list; sorted_var; )
{
struct grub_env_sorted_var *tmp = sorted_var->next;

grub_free (sorted_var);
sorted_var = tmp;
}
}

grub_err_t
grub_register_variable_hook (const char *name,
grub_env_read_hook_t read_hook,
grub_env_write_hook_t write_hook)
{
struct grub_env_var *var = grub_env_find (name);

if (! var)
{
if (grub_env_set (name, "") != GRUB_ERR_NONE)
return grub_errno;

var = grub_env_find (name);

}

if (var) {
var->read_hook = read_hook;
var->write_hook = write_hook;
}

return GRUB_ERR_NONE;
}
