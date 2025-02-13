#include "allegro5/internal/aintern_aatree.h"
struct ALLEGRO_CONFIG_ENTRY {
bool is_comment;
ALLEGRO_USTR *key; 
ALLEGRO_USTR *value;
ALLEGRO_CONFIG_ENTRY *prev, *next;
};
struct ALLEGRO_CONFIG_SECTION {
ALLEGRO_USTR *name;
ALLEGRO_CONFIG_ENTRY *head;
ALLEGRO_CONFIG_ENTRY *last;
_AL_AATREE *tree;
ALLEGRO_CONFIG_SECTION *prev, *next;
};
struct ALLEGRO_CONFIG {
ALLEGRO_CONFIG_SECTION *head;
ALLEGRO_CONFIG_SECTION *last;
_AL_AATREE *tree;
};
