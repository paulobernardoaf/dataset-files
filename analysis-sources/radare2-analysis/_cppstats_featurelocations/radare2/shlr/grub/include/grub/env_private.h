

















#if !defined(GRUB_ENV_PRIVATE_HEADER)
#define GRUB_ENV_PRIVATE_HEADER 1

#include <grub/env.h>


#define HASHSZ 13


struct grub_env_context
{

struct grub_env_var *vars[HASHSZ];


struct grub_env_context *prev;
};


struct grub_env_sorted_var
{
struct grub_env_var *var;
struct grub_env_sorted_var *next;
};

extern struct grub_env_context *grub_current_context;

#endif 
