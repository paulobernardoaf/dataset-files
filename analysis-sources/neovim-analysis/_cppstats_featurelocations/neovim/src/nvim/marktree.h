#if !defined(NVIM_MARKTREE_H)
#define NVIM_MARKTREE_H

#include <stdint.h>
#include "nvim/map.h"
#include "nvim/garray.h"

#define MT_MAX_DEPTH 20
#define MT_BRANCH_FACTOR 10

typedef struct {
int32_t row;
int32_t col;
} mtpos_t;

typedef struct {
int32_t row;
int32_t col;
uint64_t id;
bool right_gravity;
} mtmark_t;

typedef struct mtnode_s mtnode_t;
typedef struct {
int oldcol;
int i;
} iterstate_t;

typedef struct {
mtpos_t pos;
int lvl;
mtnode_t *node;
int i;
iterstate_t s[MT_MAX_DEPTH];
} MarkTreeIter;






typedef struct {
mtpos_t pos;
uint64_t id;
} mtkey_t;

struct mtnode_s {
int32_t n;
int32_t level;


mtnode_t *parent;
mtkey_t key[2 * MT_BRANCH_FACTOR - 1];
mtnode_t *ptr[];
};



typedef struct {
mtnode_t *root;
size_t n_keys, n_nodes;
uint64_t next_id;


PMap(uint64_t) *id2node;
} MarkTree;


#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "marktree.h.generated.h"
#endif

#define MARKTREE_PAIRED_FLAG (((uint64_t)1) << 1)
#define MARKTREE_END_FLAG (((uint64_t)1) << 0)

#endif 
