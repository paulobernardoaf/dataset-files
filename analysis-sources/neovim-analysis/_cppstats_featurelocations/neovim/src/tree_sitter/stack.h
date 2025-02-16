#if !defined(TREE_SITTER_PARSE_STACK_H_)
#define TREE_SITTER_PARSE_STACK_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "./array.h"
#include "./subtree.h"
#include "./error_costs.h"
#include <stdio.h>

typedef struct Stack Stack;

typedef unsigned StackVersion;
#define STACK_VERSION_NONE ((StackVersion)-1)

typedef struct {
SubtreeArray subtrees;
StackVersion version;
} StackSlice;
typedef Array(StackSlice) StackSliceArray;

typedef struct {
Length position;
unsigned depth;
TSStateId state;
} StackSummaryEntry;
typedef Array(StackSummaryEntry) StackSummary;


Stack *ts_stack_new(SubtreePool *);


void ts_stack_delete(Stack *);


uint32_t ts_stack_version_count(const Stack *);



TSStateId ts_stack_state(const Stack *, StackVersion);


Subtree ts_stack_last_external_token(const Stack *, StackVersion);


void ts_stack_set_last_external_token(Stack *, StackVersion, Subtree );


Length ts_stack_position(const Stack *, StackVersion);






void ts_stack_push(Stack *, StackVersion, Subtree , bool, TSStateId);






StackSliceArray ts_stack_pop_count(Stack *, StackVersion, uint32_t count);


SubtreeArray ts_stack_pop_error(Stack *, StackVersion);


StackSliceArray ts_stack_pop_pending(Stack *, StackVersion);


StackSliceArray ts_stack_pop_all(Stack *, StackVersion);



unsigned ts_stack_node_count_since_error(const Stack *, StackVersion);

int ts_stack_dynamic_precedence(Stack *, StackVersion);

bool ts_stack_has_advanced_since_error(const Stack *, StackVersion);



void ts_stack_record_summary(Stack *, StackVersion, unsigned max_depth);



StackSummary *ts_stack_get_summary(Stack *, StackVersion);


unsigned ts_stack_error_cost(const Stack *, StackVersion version);



bool ts_stack_merge(Stack *, StackVersion, StackVersion);


bool ts_stack_can_merge(Stack *, StackVersion, StackVersion);

TSSymbol ts_stack_resume(Stack *, StackVersion);

void ts_stack_pause(Stack *, StackVersion, TSSymbol);

void ts_stack_halt(Stack *, StackVersion);

bool ts_stack_is_active(const Stack *, StackVersion);

bool ts_stack_is_paused(const Stack *, StackVersion);

bool ts_stack_is_halted(const Stack *, StackVersion);

void ts_stack_renumber_version(Stack *, StackVersion, StackVersion);

void ts_stack_swap_versions(Stack *, StackVersion, StackVersion);

StackVersion ts_stack_copy_version(Stack *, StackVersion);


void ts_stack_remove_version(Stack *, StackVersion);

void ts_stack_clear(Stack *);

bool ts_stack_print_dot_graph(Stack *, const TSLanguage *, FILE *);

typedef void (*StackIterateCallback)(void *, TSStateId, uint32_t);

void ts_stack_iterate(Stack *, StackVersion, StackIterateCallback, void *);

#if defined(__cplusplus)
}
#endif

#endif 
