











#include "index.h"
#include "stream_flags_common.h"






#define INDEX_GROUP_SIZE 512



#define PREALLOC_MAX ((SIZE_MAX - sizeof(index_group)) / sizeof(index_record))



typedef struct index_tree_node_s index_tree_node;
struct index_tree_node_s {



lzma_vli uncompressed_base;


lzma_vli compressed_base;

index_tree_node *parent;
index_tree_node *left;
index_tree_node *right;
};



typedef struct {

index_tree_node *root;




index_tree_node *leftmost;



index_tree_node *rightmost;


uint32_t count;

} index_tree;


typedef struct {
lzma_vli uncompressed_sum;
lzma_vli unpadded_sum;
} index_record;


typedef struct {

index_tree_node node;


lzma_vli number_base;


size_t allocated;


size_t last;



















index_record records[];

} index_group;


typedef struct {

index_tree_node node;


uint32_t number;


lzma_vli block_number_base;






index_tree groups;


lzma_vli record_count;




lzma_vli index_list_size;





lzma_stream_flags stream_flags;



lzma_vli stream_padding;

} index_stream;


struct lzma_index_s {



index_tree streams;


lzma_vli uncompressed_size;


lzma_vli total_size;


lzma_vli record_count;







lzma_vli index_list_size;




size_t prealloc;





uint32_t checks;
};


static void
index_tree_init(index_tree *tree)
{
tree->root = NULL;
tree->leftmost = NULL;
tree->rightmost = NULL;
tree->count = 0;
return;
}



static void
index_tree_node_end(index_tree_node *node, lzma_allocator *allocator,
void (*free_func)(void *node, lzma_allocator *allocator))
{


if (node->left != NULL)
index_tree_node_end(node->left, allocator, free_func);

if (node->right != NULL)
index_tree_node_end(node->right, allocator, free_func);

if (free_func != NULL)
free_func(node, allocator);

lzma_free(node, allocator);
return;
}






static void
index_tree_end(index_tree *tree, lzma_allocator *allocator,
void (*free_func)(void *node, lzma_allocator *allocator))
{
if (tree->root != NULL)
index_tree_node_end(tree->root, allocator, free_func);

return;
}




static void
index_tree_append(index_tree *tree, index_tree_node *node)
{
node->parent = tree->rightmost;
node->left = NULL;
node->right = NULL;

++tree->count;


if (tree->root == NULL) {
tree->root = node;
tree->leftmost = node;
tree->rightmost = node;
return;
}


assert(tree->rightmost->uncompressed_base <= node->uncompressed_base);
assert(tree->rightmost->compressed_base < node->compressed_base);



tree->rightmost->right = node;
tree->rightmost = node;






uint32_t up = tree->count ^ (UINT32_C(1) << bsr32(tree->count));
if (up != 0) {

up = ctz32(tree->count) + 2;
do {
node = node->parent;
} while (--up > 0);


index_tree_node *pivot = node->right;

if (node->parent == NULL) {
tree->root = pivot;
} else {
assert(node->parent->right == node);
node->parent->right = pivot;
}

pivot->parent = node->parent;

node->right = pivot->left;
if (node->right != NULL)
node->right->parent = node;

pivot->left = node;
node->parent = pivot;
}

return;
}



static void *
index_tree_next(const index_tree_node *node)
{
if (node->right != NULL) {
node = node->right;
while (node->left != NULL)
node = node->left;

return (void *)(node);
}

while (node->parent != NULL && node->parent->right == node)
node = node->parent;

return (void *)(node->parent);
}





static void *
index_tree_locate(const index_tree *tree, lzma_vli target)
{
const index_tree_node *result = NULL;
const index_tree_node *node = tree->root;

assert(tree->leftmost == NULL
|| tree->leftmost->uncompressed_base == 0);



while (node != NULL) {
if (node->uncompressed_base > target) {
node = node->left;
} else {
result = node;
node = node->right;
}
}

return (void *)(result);
}



static index_stream *
index_stream_init(lzma_vli compressed_base, lzma_vli uncompressed_base,
lzma_vli stream_number, lzma_vli block_number_base,
lzma_allocator *allocator)
{
index_stream *s = lzma_alloc(sizeof(index_stream), allocator);
if (s == NULL)
return NULL;

s->node.uncompressed_base = uncompressed_base;
s->node.compressed_base = compressed_base;
s->node.parent = NULL;
s->node.left = NULL;
s->node.right = NULL;

s->number = stream_number;
s->block_number_base = block_number_base;

index_tree_init(&s->groups);

s->record_count = 0;
s->index_list_size = 0;
s->stream_flags.version = UINT32_MAX;
s->stream_padding = 0;

return s;
}



static void
index_stream_end(void *node, lzma_allocator *allocator)
{
index_stream *s = node;
index_tree_end(&s->groups, allocator, NULL);
return;
}


static lzma_index *
index_init_plain(lzma_allocator *allocator)
{
lzma_index *i = lzma_alloc(sizeof(lzma_index), allocator);
if (i != NULL) {
index_tree_init(&i->streams);
i->uncompressed_size = 0;
i->total_size = 0;
i->record_count = 0;
i->index_list_size = 0;
i->prealloc = INDEX_GROUP_SIZE;
i->checks = 0;
}

return i;
}


extern LZMA_API(lzma_index *)
lzma_index_init(lzma_allocator *allocator)
{
lzma_index *i = index_init_plain(allocator);
if (i == NULL)
return NULL;

index_stream *s = index_stream_init(0, 0, 1, 0, allocator);
if (s == NULL) {
lzma_free(i, allocator);
return NULL;
}

index_tree_append(&i->streams, &s->node);

return i;
}


extern LZMA_API(void)
lzma_index_end(lzma_index *i, lzma_allocator *allocator)
{


if (i != NULL) {
index_tree_end(&i->streams, allocator, &index_stream_end);
lzma_free(i, allocator);
}

return;
}


extern void
lzma_index_prealloc(lzma_index *i, lzma_vli records)
{
if (records > PREALLOC_MAX)
records = PREALLOC_MAX;

i->prealloc = (size_t)(records);
return;
}


extern LZMA_API(uint64_t)
lzma_index_memusage(lzma_vli streams, lzma_vli blocks)
{







const size_t alloc_overhead = 4 * sizeof(void *);




const size_t stream_base = sizeof(index_stream)
+ sizeof(index_group) + 2 * alloc_overhead;


const size_t group_base = sizeof(index_group)
+ INDEX_GROUP_SIZE * sizeof(index_record)
+ alloc_overhead;



const lzma_vli groups
= (blocks + INDEX_GROUP_SIZE - 1) / INDEX_GROUP_SIZE;


const uint64_t streams_mem = streams * stream_base;
const uint64_t groups_mem = groups * group_base;


const uint64_t index_base = sizeof(lzma_index) + alloc_overhead;




const uint64_t limit = UINT64_MAX - index_base;
if (streams == 0 || streams > UINT32_MAX || blocks > LZMA_VLI_MAX
|| streams > limit / stream_base
|| groups > limit / group_base
|| limit - streams_mem < groups_mem)
return UINT64_MAX;

return index_base + streams_mem + groups_mem;
}


extern LZMA_API(uint64_t)
lzma_index_memused(const lzma_index *i)
{
return lzma_index_memusage(i->streams.count, i->record_count);
}


extern LZMA_API(lzma_vli)
lzma_index_block_count(const lzma_index *i)
{
return i->record_count;
}


extern LZMA_API(lzma_vli)
lzma_index_stream_count(const lzma_index *i)
{
return i->streams.count;
}


extern LZMA_API(lzma_vli)
lzma_index_size(const lzma_index *i)
{
return index_size(i->record_count, i->index_list_size);
}


extern LZMA_API(lzma_vli)
lzma_index_total_size(const lzma_index *i)
{
return i->total_size;
}


extern LZMA_API(lzma_vli)
lzma_index_stream_size(const lzma_index *i)
{

return LZMA_STREAM_HEADER_SIZE + i->total_size
+ index_size(i->record_count, i->index_list_size)
+ LZMA_STREAM_HEADER_SIZE;
}


static lzma_vli
index_file_size(lzma_vli compressed_base, lzma_vli unpadded_sum,
lzma_vli record_count, lzma_vli index_list_size,
lzma_vli stream_padding)
{





lzma_vli file_size = compressed_base + 2 * LZMA_STREAM_HEADER_SIZE
+ stream_padding + vli_ceil4(unpadded_sum);
if (file_size > LZMA_VLI_MAX)
return LZMA_VLI_UNKNOWN;


file_size += index_size(record_count, index_list_size);
if (file_size > LZMA_VLI_MAX)
return LZMA_VLI_UNKNOWN;

return file_size;
}


extern LZMA_API(lzma_vli)
lzma_index_file_size(const lzma_index *i)
{
const index_stream *s = (const index_stream *)(i->streams.rightmost);
const index_group *g = (const index_group *)(s->groups.rightmost);
return index_file_size(s->node.compressed_base,
g == NULL ? 0 : g->records[g->last].unpadded_sum,
s->record_count, s->index_list_size,
s->stream_padding);
}


extern LZMA_API(lzma_vli)
lzma_index_uncompressed_size(const lzma_index *i)
{
return i->uncompressed_size;
}


extern LZMA_API(uint32_t)
lzma_index_checks(const lzma_index *i)
{
uint32_t checks = i->checks;


const index_stream *s = (const index_stream *)(i->streams.rightmost);
if (s->stream_flags.version != UINT32_MAX)
checks |= UINT32_C(1) << s->stream_flags.check;

return checks;
}


extern uint32_t
lzma_index_padding_size(const lzma_index *i)
{
return (LZMA_VLI_C(4) - index_size_unpadded(
i->record_count, i->index_list_size)) & 3;
}


extern LZMA_API(lzma_ret)
lzma_index_stream_flags(lzma_index *i, const lzma_stream_flags *stream_flags)
{
if (i == NULL || stream_flags == NULL)
return LZMA_PROG_ERROR;


return_if_error(lzma_stream_flags_compare(
stream_flags, stream_flags));

index_stream *s = (index_stream *)(i->streams.rightmost);
s->stream_flags = *stream_flags;

return LZMA_OK;
}


extern LZMA_API(lzma_ret)
lzma_index_stream_padding(lzma_index *i, lzma_vli stream_padding)
{
if (i == NULL || stream_padding > LZMA_VLI_MAX
|| (stream_padding & 3) != 0)
return LZMA_PROG_ERROR;

index_stream *s = (index_stream *)(i->streams.rightmost);


const lzma_vli old_stream_padding = s->stream_padding;
s->stream_padding = 0;
if (lzma_index_file_size(i) + stream_padding > LZMA_VLI_MAX) {
s->stream_padding = old_stream_padding;
return LZMA_DATA_ERROR;
}

s->stream_padding = stream_padding;
return LZMA_OK;
}


extern LZMA_API(lzma_ret)
lzma_index_append(lzma_index *i, lzma_allocator *allocator,
lzma_vli unpadded_size, lzma_vli uncompressed_size)
{

if (i == NULL || unpadded_size < UNPADDED_SIZE_MIN
|| unpadded_size > UNPADDED_SIZE_MAX
|| uncompressed_size > LZMA_VLI_MAX)
return LZMA_PROG_ERROR;

index_stream *s = (index_stream *)(i->streams.rightmost);
index_group *g = (index_group *)(s->groups.rightmost);

const lzma_vli compressed_base = g == NULL ? 0
: vli_ceil4(g->records[g->last].unpadded_sum);
const lzma_vli uncompressed_base = g == NULL ? 0
: g->records[g->last].uncompressed_sum;
const uint32_t index_list_size_add = lzma_vli_size(unpadded_size)
+ lzma_vli_size(uncompressed_size);


if (index_file_size(s->node.compressed_base,
compressed_base + unpadded_size, s->record_count + 1,
s->index_list_size + index_list_size_add,
s->stream_padding) == LZMA_VLI_UNKNOWN)
return LZMA_DATA_ERROR;



if (index_size(i->record_count + 1,
i->index_list_size + index_list_size_add)
> LZMA_BACKWARD_SIZE_MAX)
return LZMA_DATA_ERROR;

if (g != NULL && g->last + 1 < g->allocated) {

++g->last;
} else {

g = lzma_alloc(sizeof(index_group)
+ i->prealloc * sizeof(index_record),
allocator);
if (g == NULL)
return LZMA_MEM_ERROR;

g->last = 0;
g->allocated = i->prealloc;



i->prealloc = INDEX_GROUP_SIZE;


g->node.uncompressed_base = uncompressed_base;
g->node.compressed_base = compressed_base;
g->number_base = s->record_count + 1;


index_tree_append(&s->groups, &g->node);
}


g->records[g->last].uncompressed_sum
= uncompressed_base + uncompressed_size;
g->records[g->last].unpadded_sum
= compressed_base + unpadded_size;


++s->record_count;
s->index_list_size += index_list_size_add;

i->total_size += vli_ceil4(unpadded_size);
i->uncompressed_size += uncompressed_size;
++i->record_count;
i->index_list_size += index_list_size_add;

return LZMA_OK;
}



typedef struct {

lzma_vli uncompressed_size;


lzma_vli file_size;


lzma_vli block_number_add;




uint32_t stream_number_add;


index_tree *streams;

} index_cat_info;





static void
index_cat_helper(const index_cat_info *info, index_stream *this)
{
index_stream *left = (index_stream *)(this->node.left);
index_stream *right = (index_stream *)(this->node.right);

if (left != NULL)
index_cat_helper(info, left);

this->node.uncompressed_base += info->uncompressed_size;
this->node.compressed_base += info->file_size;
this->number += info->stream_number_add;
this->block_number_base += info->block_number_add;
index_tree_append(info->streams, &this->node);

if (right != NULL)
index_cat_helper(info, right);

return;
}


extern LZMA_API(lzma_ret)
lzma_index_cat(lzma_index *restrict dest, lzma_index *restrict src,
lzma_allocator *allocator)
{
const lzma_vli dest_file_size = lzma_index_file_size(dest);


if (dest_file_size + lzma_index_file_size(src) > LZMA_VLI_MAX
|| dest->uncompressed_size + src->uncompressed_size
> LZMA_VLI_MAX)
return LZMA_DATA_ERROR;







{
const lzma_vli dest_size = index_size_unpadded(
dest->record_count, dest->index_list_size);
const lzma_vli src_size = index_size_unpadded(
src->record_count, src->index_list_size);
if (vli_ceil4(dest_size + src_size) > LZMA_BACKWARD_SIZE_MAX)
return LZMA_DATA_ERROR;
}



{
index_stream *s = (index_stream *)(dest->streams.rightmost);
index_group *g = (index_group *)(s->groups.rightmost);
if (g != NULL && g->last + 1 < g->allocated) {
assert(g->node.left == NULL);
assert(g->node.right == NULL);

index_group *newg = lzma_alloc(sizeof(index_group)
+ (g->last + 1)
* sizeof(index_record),
allocator);
if (newg == NULL)
return LZMA_MEM_ERROR;

newg->node = g->node;
newg->allocated = g->last + 1;
newg->last = g->last;
newg->number_base = g->number_base;

memcpy(newg->records, g->records, newg->allocated
* sizeof(index_record));

if (g->node.parent != NULL) {
assert(g->node.parent->right == &g->node);
g->node.parent->right = &newg->node;
}

if (s->groups.leftmost == &g->node) {
assert(s->groups.root == &g->node);
s->groups.leftmost = &newg->node;
s->groups.root = &newg->node;
}

if (s->groups.rightmost == &g->node)
s->groups.rightmost = &newg->node;

lzma_free(g, allocator);
}
}



const index_cat_info info = {
.uncompressed_size = dest->uncompressed_size,
.file_size = dest_file_size,
.stream_number_add = dest->streams.count,
.block_number_add = dest->record_count,
.streams = &dest->streams,
};
index_cat_helper(&info, (index_stream *)(src->streams.root));


dest->uncompressed_size += src->uncompressed_size;
dest->total_size += src->total_size;
dest->record_count += src->record_count;
dest->index_list_size += src->index_list_size;
dest->checks = lzma_index_checks(dest) | src->checks;


lzma_free(src, allocator);

return LZMA_OK;
}



static index_stream *
index_dup_stream(const index_stream *src, lzma_allocator *allocator)
{

if (src->record_count > PREALLOC_MAX)
return NULL;


index_stream *dest = index_stream_init(src->node.compressed_base,
src->node.uncompressed_base, src->number,
src->block_number_base, allocator);



if (dest == NULL || src->groups.leftmost == NULL)
return dest;


dest->record_count = src->record_count;
dest->index_list_size = src->index_list_size;
dest->stream_flags = src->stream_flags;
dest->stream_padding = src->stream_padding;




index_group *destg = lzma_alloc(sizeof(index_group)
+ src->record_count * sizeof(index_record),
allocator);
if (destg == NULL) {
index_stream_end(dest, allocator);
return NULL;
}


destg->node.uncompressed_base = 0;
destg->node.compressed_base = 0;
destg->number_base = 1;
destg->allocated = src->record_count;
destg->last = src->record_count - 1;


const index_group *srcg = (const index_group *)(src->groups.leftmost);
size_t i = 0;
do {
memcpy(destg->records + i, srcg->records,
(srcg->last + 1) * sizeof(index_record));
i += srcg->last + 1;
srcg = index_tree_next(&srcg->node);
} while (srcg != NULL);

assert(i == destg->allocated);


index_tree_append(&dest->groups, &destg->node);

return dest;
}


extern LZMA_API(lzma_index *)
lzma_index_dup(const lzma_index *src, lzma_allocator *allocator)
{

lzma_index *dest = index_init_plain(allocator);
if (dest == NULL)
return NULL;


dest->uncompressed_size = src->uncompressed_size;
dest->total_size = src->total_size;
dest->record_count = src->record_count;
dest->index_list_size = src->index_list_size;


const index_stream *srcstream
= (const index_stream *)(src->streams.leftmost);
do {
index_stream *deststream = index_dup_stream(
srcstream, allocator);
if (deststream == NULL) {
lzma_index_end(dest, allocator);
return NULL;
}

index_tree_append(&dest->streams, &deststream->node);

srcstream = index_tree_next(&srcstream->node);
} while (srcstream != NULL);

return dest;
}



enum {
ITER_INDEX,
ITER_STREAM,
ITER_GROUP,
ITER_RECORD,
ITER_METHOD,
};



enum {
ITER_METHOD_NORMAL,
ITER_METHOD_NEXT,
ITER_METHOD_LEFTMOST,
};


static void
iter_set_info(lzma_index_iter *iter)
{
const lzma_index *i = iter->internal[ITER_INDEX].p;
const index_stream *stream = iter->internal[ITER_STREAM].p;
const index_group *group = iter->internal[ITER_GROUP].p;
const size_t record = iter->internal[ITER_RECORD].s;




if (group == NULL) {

assert(stream->groups.root == NULL);
iter->internal[ITER_METHOD].s = ITER_METHOD_LEFTMOST;

} else if (i->streams.rightmost != &stream->node
|| stream->groups.rightmost != &group->node) {

iter->internal[ITER_METHOD].s = ITER_METHOD_NORMAL;

} else if (stream->groups.leftmost != &group->node) {



assert(stream->groups.root != &group->node);
assert(group->node.parent->right == &group->node);
iter->internal[ITER_METHOD].s = ITER_METHOD_NEXT;
iter->internal[ITER_GROUP].p = group->node.parent;

} else {

assert(stream->groups.root == &group->node);
assert(group->node.parent == NULL);
iter->internal[ITER_METHOD].s = ITER_METHOD_LEFTMOST;
iter->internal[ITER_GROUP].p = NULL;
}

iter->stream.number = stream->number;
iter->stream.block_count = stream->record_count;
iter->stream.compressed_offset = stream->node.compressed_base;
iter->stream.uncompressed_offset = stream->node.uncompressed_base;



iter->stream.flags = stream->stream_flags.version == UINT32_MAX
? NULL : &stream->stream_flags;
iter->stream.padding = stream->stream_padding;

if (stream->groups.rightmost == NULL) {

iter->stream.compressed_size = index_size(0, 0)
+ 2 * LZMA_STREAM_HEADER_SIZE;
iter->stream.uncompressed_size = 0;
} else {
const index_group *g = (const index_group *)(
stream->groups.rightmost);


iter->stream.compressed_size = 2 * LZMA_STREAM_HEADER_SIZE
+ index_size(stream->record_count,
stream->index_list_size)
+ vli_ceil4(g->records[g->last].unpadded_sum);
iter->stream.uncompressed_size
= g->records[g->last].uncompressed_sum;
}

if (group != NULL) {
iter->block.number_in_stream = group->number_base + record;
iter->block.number_in_file = iter->block.number_in_stream
+ stream->block_number_base;

iter->block.compressed_stream_offset
= record == 0 ? group->node.compressed_base
: vli_ceil4(group->records[
record - 1].unpadded_sum);
iter->block.uncompressed_stream_offset
= record == 0 ? group->node.uncompressed_base
: group->records[record - 1].uncompressed_sum;

iter->block.uncompressed_size
= group->records[record].uncompressed_sum
- iter->block.uncompressed_stream_offset;
iter->block.unpadded_size
= group->records[record].unpadded_sum
- iter->block.compressed_stream_offset;
iter->block.total_size = vli_ceil4(iter->block.unpadded_size);

iter->block.compressed_stream_offset
+= LZMA_STREAM_HEADER_SIZE;

iter->block.compressed_file_offset
= iter->block.compressed_stream_offset
+ iter->stream.compressed_offset;
iter->block.uncompressed_file_offset
= iter->block.uncompressed_stream_offset
+ iter->stream.uncompressed_offset;
}

return;
}


extern LZMA_API(void)
lzma_index_iter_init(lzma_index_iter *iter, const lzma_index *i)
{
iter->internal[ITER_INDEX].p = i;
lzma_index_iter_rewind(iter);
return;
}


extern LZMA_API(void)
lzma_index_iter_rewind(lzma_index_iter *iter)
{
iter->internal[ITER_STREAM].p = NULL;
iter->internal[ITER_GROUP].p = NULL;
iter->internal[ITER_RECORD].s = 0;
iter->internal[ITER_METHOD].s = ITER_METHOD_NORMAL;
return;
}


extern LZMA_API(lzma_bool)
lzma_index_iter_next(lzma_index_iter *iter, lzma_index_iter_mode mode)
{

if ((unsigned int)(mode) > LZMA_INDEX_ITER_NONEMPTY_BLOCK)
return true;

const lzma_index *i = iter->internal[ITER_INDEX].p;
const index_stream *stream = iter->internal[ITER_STREAM].p;
const index_group *group = NULL;
size_t record = iter->internal[ITER_RECORD].s;




if (mode != LZMA_INDEX_ITER_STREAM) {


switch (iter->internal[ITER_METHOD].s) {
case ITER_METHOD_NORMAL:
group = iter->internal[ITER_GROUP].p;
break;

case ITER_METHOD_NEXT:
group = index_tree_next(iter->internal[ITER_GROUP].p);
break;

case ITER_METHOD_LEFTMOST:
group = (const index_group *)(
stream->groups.leftmost);
break;
}
}

again:
if (stream == NULL) {


stream = (const index_stream *)(i->streams.leftmost);
if (mode >= LZMA_INDEX_ITER_BLOCK) {



while (stream->groups.leftmost == NULL) {
stream = index_tree_next(&stream->node);
if (stream == NULL)
return true;
}
}


group = (const index_group *)(stream->groups.leftmost);
record = 0;

} else if (group != NULL && record < group->last) {

++record;

} else {


record = 0;



if (group != NULL)
group = index_tree_next(&group->node);

if (group == NULL) {



do {
stream = index_tree_next(&stream->node);
if (stream == NULL)
return true;
} while (mode >= LZMA_INDEX_ITER_BLOCK
&& stream->groups.leftmost == NULL);

group = (const index_group *)(
stream->groups.leftmost);
}
}

if (mode == LZMA_INDEX_ITER_NONEMPTY_BLOCK) {


if (record == 0) {
if (group->node.uncompressed_base
== group->records[0].uncompressed_sum)
goto again;
} else if (group->records[record - 1].uncompressed_sum
== group->records[record].uncompressed_sum) {
goto again;
}
}

iter->internal[ITER_STREAM].p = stream;
iter->internal[ITER_GROUP].p = group;
iter->internal[ITER_RECORD].s = record;

iter_set_info(iter);

return false;
}


extern LZMA_API(lzma_bool)
lzma_index_iter_locate(lzma_index_iter *iter, lzma_vli target)
{
const lzma_index *i = iter->internal[ITER_INDEX].p;


if (i->uncompressed_size <= target)
return true;


const index_stream *stream = index_tree_locate(&i->streams, target);
assert(stream != NULL);
target -= stream->node.uncompressed_base;


const index_group *group = index_tree_locate(&stream->groups, target);
assert(group != NULL);






size_t left = 0;
size_t right = group->last;

while (left < right) {
const size_t pos = left + (right - left) / 2;
if (group->records[pos].uncompressed_sum <= target)
left = pos + 1;
else
right = pos;
}

iter->internal[ITER_STREAM].p = stream;
iter->internal[ITER_GROUP].p = group;
iter->internal[ITER_RECORD].s = left;

iter_set_info(iter);

return false;
}
