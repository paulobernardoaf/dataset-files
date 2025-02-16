

#if !defined(INCLUDE_HEAP_GLIBC_C)
#define INCLUDE_HEAP_GLIBC_C
#define HEAP32 1
#include "linux_heap_glibc.c"
#undef HEAP32
#endif

#undef GH
#undef GHT
#undef GHT_MAX

#if HEAP32
#define GH(x) x##_32
#define GHT ut32
#define GHT_MAX UT32_MAX
#else
#define GH(x) x##_64
#define GHT ut64
#define GHT_MAX UT64_MAX
#endif

static bool GH(is_tcache)(RCore *core) {
char *fp = NULL;
double v = 0;
if (r_config_get_i (core->config, "cfg.debug")) {
RDebugMap *map;
RListIter *iter;
r_debug_map_sync (core->dbg);
r_list_foreach (core->dbg->maps, iter, map) {
fp = strstr (map->name, "libc-");
if (fp) {
break;
}
}
} else {
int tcv = r_config_get_i (core->config, "dbg.glibc.tcache");
eprintf ("dbg.glibc.tcache = %i\n", tcv);
return tcv != 0;
}
if (fp) {
v = r_num_get_float (NULL, fp + 5);
}
return (v > 2.25);
}

static GHT GH(tcache_chunk_size)(RCore *core, GHT brk_start) {
GHT sz = 0;

GH(RHeapChunk) *cnk = R_NEW0 (GH(RHeapChunk));
if (!cnk) {
return sz;
}
r_io_read_at (core->io, brk_start, (ut8 *)cnk, sizeof (GH(RHeapChunk)));
sz = (cnk->size >> 3) << 3; 
return sz;
}

static void GH(update_arena_with_tc)(GH(RHeap_MallocState_tcache) *cmain_arena, MallocState *main_arena) {
int i = 0;
main_arena->mutex = cmain_arena->mutex;
main_arena->flags = cmain_arena->flags;
for (i = 0; i < BINMAPSIZE; i++) {
main_arena->binmap[i] = cmain_arena->binmap[i];
}
main_arena->have_fast_chunks = cmain_arena->have_fast_chunks;
main_arena->attached_threads = cmain_arena->attached_threads;
for (i = 0; i < NFASTBINS; i++) {
main_arena->GH(fastbinsY)[i] = cmain_arena->fastbinsY[i];
}
main_arena->GH(top) = cmain_arena->top;
main_arena->GH(last_remainder) = cmain_arena->last_remainder;
for (i = 0; i < NBINS * 2 - 2; i++) {
main_arena->GH(bins)[i] = cmain_arena->bins[i];
}
main_arena->GH(next) = cmain_arena->next;
main_arena->GH(next_free) = cmain_arena->next_free;
main_arena->GH(system_mem) = cmain_arena->system_mem;
main_arena->GH(max_system_mem) = cmain_arena->max_system_mem;
}

static void GH(update_arena_without_tc)(GH(RHeap_MallocState) *cmain_arena, MallocState *main_arena) {
int i = 0;
main_arena->mutex = cmain_arena->mutex;
main_arena->flags = cmain_arena->flags;
for (i = 0; i < BINMAPSIZE; i++) {
main_arena->binmap[i] = cmain_arena->binmap[i];
}
main_arena->attached_threads = 1;
for (i = 0; i < NFASTBINS; i++) {
main_arena->GH(fastbinsY)[i] = cmain_arena->fastbinsY[i];
}
main_arena->GH(top) = cmain_arena->top;
main_arena->GH(last_remainder) = cmain_arena->last_remainder;
for (i = 0; i < NBINS * 2 - 2; i++) {
main_arena->GH(bins)[i] = cmain_arena->bins[i];
}
main_arena->GH(next) = cmain_arena->next;
main_arena->GH(next_free) = cmain_arena->next_free;
main_arena->GH(system_mem) = cmain_arena->system_mem;
main_arena->GH(max_system_mem) = cmain_arena->max_system_mem;
}

static bool GH(update_main_arena)(RCore *core, GHT m_arena, MallocState *main_arena) {
const int tcache = r_config_get_i (core->config, "dbg.glibc.tcache");
if (tcache) {
GH(RHeap_MallocState_tcache) *cmain_arena = R_NEW0 (GH(RHeap_MallocState_tcache));
if (!cmain_arena) {
return false;
}
(void)r_io_read_at (core->io, m_arena, (ut8 *)cmain_arena, sizeof (GH(RHeap_MallocState_tcache)));
GH(update_arena_with_tc)(cmain_arena,main_arena);
} else {
GH(RHeap_MallocState) *cmain_arena = R_NEW0 (GH(RHeap_MallocState));
if (!cmain_arena) {
return false;
}
(void)r_io_read_at (core->io, m_arena, (ut8 *)cmain_arena, sizeof (GH(RHeap_MallocState)));
GH(update_arena_without_tc)(cmain_arena,main_arena);
}
return true;
}

static void GH(get_brks)(RCore *core, GHT *brk_start, GHT *brk_end) {
if (r_config_get_i (core->config, "cfg.debug")) {
RListIter *iter;
RDebugMap *map;
r_debug_map_sync (core->dbg);
r_list_foreach (core->dbg->maps, iter, map) {
if (map->name) {
if (strstr (map->name, "[heap]")) {
*brk_start = map->addr;
*brk_end = map->addr_end;
break;
}
}
}
} else {
RIOMap *map;
SdbListIter *iter;
ls_foreach (core->io->maps, iter, map) {
if (map->name) {
if (strstr (map->name, "[heap]")) {
*brk_start = map->itv.addr;
*brk_end = map->itv.addr + map->itv.size;
break;
}
}
}
}
}

static void GH(print_arena_stats)(RCore *core, GHT m_arena, MallocState *main_arena, GHT global_max_fast, int format) {
int i, j, k, start;
GHT align = 12 * SZ + sizeof (int) * 2;
const int tcache = r_config_get_i (core->config, "dbg.glibc.tcache");
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

if (tcache) {
align = 16;
}

GHT apart[NSMALLBINS + 1] = { 0LL };
if (format == '*') {
for (i = 0; i < NBINS * 2 - 2; i += 2) {
GHT addr = m_arena + align + SZ * i - SZ * 2;
GHT bina = main_arena->GH(bins)[i];
r_cons_printf ("f chunk.%d.bin = 0x%"PFMT64x"\n", i, (ut64)addr);
r_cons_printf ("f chunk.%d.fd = 0x%"PFMT64x"\n", i, (ut64)bina);
bina = main_arena->GH(bins)[i + 1];
r_cons_printf ("f chunk.%d.bk = 0x%"PFMT64x"\n", i, (ut64)bina);
}
for (i = 0; i < BINMAPSIZE; i++) {
r_cons_printf ("f binmap.%d = 0x%"PFMT64x, i, (ut64)main_arena->binmap[i]);
}
{ 
char units[8];
r_num_units (units, sizeof (units), main_arena->GH(max_system_mem));
r_cons_printf ("f heap.maxmem = %s\n", units);

r_num_units (units, sizeof (units), main_arena->GH(system_mem));
r_cons_printf ("f heap.sysmem = %s\n", units);

r_num_units (units, sizeof (units), main_arena->GH(next_free));
r_cons_printf ("f heap.nextfree = %s\n", units);

r_num_units (units, sizeof (units), main_arena->GH(next));
r_cons_printf ("f heap.next= %s\n", units);
}
return;
}

PRINT_GA ("malloc_state @ ");
PRINTF_BA ("0x%"PFMT64x"\n\n", (ut64)m_arena);
PRINT_GA ("struct malloc_state main_arena {\n");
PRINT_GA (" mutex = ");
PRINTF_BA ("0x%08x\n", (ut32)main_arena->mutex);
PRINT_GA (" flags = ");
PRINTF_BA ("0x%08x\n", (ut32)main_arena->flags);
PRINT_GA (" fastbinsY = {\n");

for (i = 0, j = 1, k = SZ * 4; i < NFASTBINS; i++, j++, k += SZ * 2) {
if (FASTBIN_IDX_TO_SIZE (j) <= global_max_fast) {
PRINTF_YA (" Fastbin %02d\n", j);
} else {
PRINTF_RA (" Fastbin %02d\n", j);
}
PRINT_GA (" chunksize:");
PRINTF_BA (" == %04d ", k);
PRINTF_GA ("0x%"PFMT64x, (ut64)main_arena->GH(fastbinsY)[i]);
PRINT_GA (",\n");
}
PRINT_GA ("}\n");
PRINT_GA (" top = ");
PRINTF_BA ("0x%"PFMT64x, (ut64)main_arena->GH(top));
PRINT_GA (",\n");
PRINT_GA (" last_remainder = ");
PRINTF_BA ("0x%"PFMT64x, (ut64)main_arena->GH(last_remainder));
PRINT_GA (",\n");
PRINT_GA (" bins {\n");


start = SZ * 128;
for (i = start, k = 0, j = 0; j < NBINS - 2 && i < 1024 * 1024; i += 64) {
j = largebin_index (i);
if (j == k + NSMALLBINS + 1) {
apart[k++] = i;
}
}
for (i = 0, j = 1, k = SZ * 4; i < NBINS * 2 - 2; i += 2, j++) {
PRINTF_YA (" Bin %03d: ", j);
if (j == 1) {
PRINT_GA ("Unsorted Bin");
PRINT_GA (" [");
PRINT_GA (" chunksize:");
PRINT_BA (" undefined ");
} else if (j > 1 && j <= NSMALLBINS) {
if (j == 2) {
PRINT_GA (" ┌");
} else if (j == (NSMALLBINS / 2)) {
PRINT_GA (" Small Bins │");
} else if (j != 2 && j != (NSMALLBINS / 2) && j != NSMALLBINS) {
PRINT_GA (" │");
} else {
PRINT_GA (" └");
}
PRINT_GA (" chunksize:");
PRINTF_BA (" == %06d ", k);
if (j < NSMALLBINS) {
k += SZ * 2;
}
} else {
if (j == NSMALLBINS + 1) {
PRINT_GA (" ┌");
} else if (j == (NSMALLBINS / 2) * 3) {
PRINT_GA (" Large Bins │");
} else if (j != NSMALLBINS + 1 && j != (NSMALLBINS / 2) * 3 && j != NBINS - 1) {
PRINT_GA (" │");
} else {
PRINT_GA (" └");
}
PRINT_GA (" chunksize:");
if (j != NBINS - 1) {
PRINTF_BA (" >= %06d ", apart[j - NSMALLBINS - 1]);
} else {
PRINT_BA (" remaining ");
}
}
GHT bin = m_arena + align + SZ * i - SZ * 2;
PRINTF_GA ("0x%"PFMT64x"->fd = ", (ut64)bin);
PRINTF_BA ("0x%"PFMT64x, (ut64)main_arena->GH(bins)[i]);
PRINT_GA (", ");
PRINTF_GA ("0x%"PFMT64x"->bk = ", (ut64)bin);
PRINTF_BA ("0x%"PFMT64x, (ut64)main_arena->GH(bins)[i + 1] );
PRINT_GA (", ");
r_cons_newline ();
}

PRINT_GA (" }\n");
PRINT_GA (" binmap = {");

for (i = 0; i < BINMAPSIZE; i++) {
if (i) {
PRINT_GA (",");
}
PRINTF_BA ("0x%x", (ut32)main_arena->binmap[i]);
}
PRINT_GA ("}\n");
PRINT_GA (" next = ");
PRINTF_BA ("0x%"PFMT64x, (ut64)main_arena->GH(next));
PRINT_GA (",\n");
PRINT_GA (" next_free = ");
PRINTF_BA ("0x%"PFMT64x, (ut64)main_arena->GH(next_free));
PRINT_GA (",\n");
PRINT_GA (" system_mem = ");
PRINTF_BA ("0x%"PFMT64x, (ut64)main_arena->GH(system_mem));
PRINT_GA (",\n");
PRINT_GA (" max_system_mem = ");
PRINTF_BA ("0x%"PFMT64x, (ut64)main_arena->GH(max_system_mem));
PRINT_GA (",\n");
PRINT_GA ("}\n\n");
}

static bool GH(r_resolve_main_arena)(RCore *core, GHT *m_arena) {
if (!core || !core->dbg || !core->dbg->maps) {
return false;
}

GHT brk_start = GHT_MAX, brk_end = GHT_MAX;
GHT libc_addr_sta = GHT_MAX, libc_addr_end = 0;
GHT addr_srch = GHT_MAX, heap_sz = GHT_MAX;

if (r_config_get_i (core->config, "cfg.debug")) {
RListIter *iter;
RDebugMap *map;
r_debug_map_sync (core->dbg);
r_list_foreach (core->dbg->maps, iter, map) {
if (strstr (map->name, "/libc-") && map->perm == 6) {
libc_addr_sta = map->addr;
libc_addr_end = map->addr_end;
break;
}
}
} else {
RIOMap *map;
SdbListIter *iter;
ls_foreach (core->io->maps, iter, map) {
if (map->name && strstr (map->name, "arena")) {
libc_addr_sta = map->itv.addr;
libc_addr_end = map->itv.addr + map->itv.size;
break;
}
}
}

if (libc_addr_sta == GHT_MAX || libc_addr_end == GHT_MAX) {
if (r_config_get_i (core->config, "cfg.debug")) {
eprintf ("Warning: Can't find glibc mapped in memory (see dm)\n");
} else {
eprintf ("Warning: Can't find arena mapped in memory (see om)\n");
}
return false;
}

GH(get_brks) (core, &brk_start, &brk_end);
if (brk_start == GHT_MAX || brk_end == GHT_MAX) {
eprintf ("No Heap section\n");
return false;
}

addr_srch = libc_addr_sta;
heap_sz = brk_end - brk_start;
MallocState *ta = R_NEW0 (MallocState);
if (!ta) {
return false;
}
while (addr_srch < libc_addr_end) {
GH (update_main_arena) (core, addr_srch, ta);
if ( ta->GH(top) > brk_start && ta->GH(top) < brk_end &&
ta->GH(system_mem) == heap_sz) {

*m_arena = addr_srch;
free (ta);
return true;
}
addr_srch += sizeof (GHT);
}
eprintf ("Warning: Can't find main_arena in mapped memory\n");
free (ta);
return false;
}

void GH(print_heap_chunk)(RCore *core) {
GH(RHeapChunk) *cnk = R_NEW0 (GH(RHeapChunk));
GHT chunk = core->offset;
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

if (!cnk) {
return;
}

(void) r_io_read_at (core->io, chunk, (ut8 *)cnk, sizeof (*cnk));

PRINT_GA ("struct malloc_chunk @ ");
PRINTF_BA ("0x%"PFMT64x, (ut64)chunk);
PRINT_GA (" {\n prev_size = ");
PRINTF_BA ("0x%"PFMT64x, (ut64)cnk->prev_size);
PRINT_GA (",\n size = ");
PRINTF_BA ("0x%"PFMT64x, (ut64)cnk->size & ~(NON_MAIN_ARENA | IS_MMAPPED | PREV_INUSE));
PRINT_GA(",\n flags: |N:");
PRINTF_BA("%1d", (cnk->size & NON_MAIN_ARENA ) >> 2);
PRINT_GA(" |M:");
PRINTF_BA("%1d", (cnk->size & IS_MMAPPED) >> 1);
PRINT_GA(" |P:");
PRINTF_BA("%1d", cnk->size & PREV_INUSE);

PRINT_GA (",\n fd = ");
PRINTF_BA ("0x%"PFMT64x, (ut64)cnk->fd);

PRINT_GA (",\n bk = ");
PRINTF_BA ("0x%"PFMT64x, (ut64)cnk->bk);

if (cnk->size > SZ * 128) {
PRINT_GA (",\n fd-nextsize = ");
PRINTF_BA ("0x%"PFMT64x, (ut64) cnk->fd_nextsize);
PRINT_GA (",\n bk-nextsize = ");
PRINTF_BA ("0x%"PFMT64x, (ut64) cnk->bk_nextsize);
}

PRINT_GA (",\n}\n");
GHT size = ((cnk->size >> 3) << 3) - SZ * 2;
if (size > SZ * 128) {
PRINT_GA ("chunk too big to be displayed\n");
size = SZ * 128;
}

char *data = calloc (1, size);
if (data) {
r_io_read_at (core->io, chunk + SZ * 2, (ut8 *)data, size);
PRINT_GA ("chunk data = \n");
r_print_hexdump (core->print, chunk + SZ * 2, (ut8 *)data, size, SZ * 8, SZ, 1);
free (data);
}
free (cnk);
}

static bool GH(is_arena)(RCore *core, GHT m_arena, GHT m_state) {
if (m_arena == m_state) {
return true;
}
MallocState *ta = R_NEW0 (MallocState);
if (!ta) {
return false;
}
if (!GH(update_main_arena) (core, m_arena, ta)) {
free (ta);
return false;
}
if (ta->GH(next) == m_state) {
free (ta);
return true;
}
while (ta->GH(next) != GHT_MAX && ta->GH(next) != m_arena) {
if (!GH(update_main_arena) (core, ta->GH(next), ta)) {
free (ta);
return false;
}
if (ta->GH(next) == m_state) {
free (ta);
return true;
}
}
free (ta);
return false;
}

static int GH(print_double_linked_list_bin_simple)(RCore *core, GHT bin, MallocState *main_arena, GHT brk_start) {
GHT next = GHT_MAX;
int ret = 1;
GH(RHeapChunk) *cnk = R_NEW0 (GH(RHeapChunk));
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

if (!cnk) {
return -1;
}

r_io_read_at (core->io, bin, (ut8 *)cnk, sizeof (GH(RHeapChunk)));

PRINTF_GA (" 0x%"PFMT64x, (ut64)bin);
if (cnk->fd != bin) {
ret = 0;
}
while (cnk->fd != bin) {
PRINTF_BA ("->fd = 0x%"PFMT64x, (ut64)cnk->fd);
next = cnk->fd;
if (next < brk_start || next > main_arena->GH(top)) {
PRINT_RA ("Double linked list corrupted\n");
free (cnk);
return -1;
}
r_io_read_at (core->io, next, (ut8 *)cnk, sizeof (GH(RHeapChunk)));
}

PRINTF_GA ("->fd = 0x%"PFMT64x, (ut64)cnk->fd);
next = cnk->fd;

if (next != bin) {
PRINT_RA ("Double linked list corrupted\n");
free (cnk);
return -1;
}
(void)r_io_read_at (core->io, next, (ut8 *)cnk, sizeof (GH(RHeapChunk)));
PRINTF_GA ("\n 0x%"PFMT64x, (ut64)bin);

while (cnk->bk != bin) {
PRINTF_BA ("->bk = 0x%"PFMT64x, (ut64) cnk->bk);
next = cnk->bk;
if (next < brk_start || next > main_arena->GH(top)) {
PRINT_RA ("Double linked list corrupted.\n");
free (cnk);
return -1;
}
(void)r_io_read_at (core->io, next, (ut8 *)cnk, sizeof (GH(RHeapChunk)));
}

PRINTF_GA ("->bk = 0x%"PFMT64x, (ut64)cnk->bk);
free (cnk);
return ret;
}

static int GH(print_double_linked_list_bin_graph)(RCore *core, GHT bin, MallocState *main_arena, GHT brk_start) {
RAGraph *g = r_agraph_new (r_cons_canvas_new (1, 1));
GHT next = GHT_MAX;
char title[256], chunk[256];
RANode *bin_node = NULL, *prev_node = NULL, *next_node = NULL;
GH(RHeapChunk) *cnk = R_NEW0 (GH(RHeapChunk));
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

if (!cnk || !g) {
free (cnk);
r_agraph_free (g);
return -1;
}
g->can->color = r_config_get_i (core->config, "scr.color");

(void)r_io_read_at (core->io, bin, (ut8 *)cnk, sizeof (GH(RHeapChunk)));
snprintf (title, sizeof (title) - 1, "bin @ 0x%"PFMT64x"\n", (ut64)bin);
snprintf (chunk, sizeof (chunk) - 1, "fd: 0x%"PFMT64x"\nbk: 0x%"PFMT64x"\n",
(ut64)cnk->fd, (ut64)cnk->bk);
bin_node = r_agraph_add_node (g, title, chunk);
prev_node = bin_node;

while (cnk->bk != bin) {
next = cnk->bk;
if (next < brk_start || next > main_arena->GH(top)) {
PRINT_RA ("Double linked list corrupted\n");
free (cnk);
free (g);
return -1;
}

r_io_read_at (core->io, next, (ut8 *)cnk, sizeof (GH(RHeapChunk)));
snprintf (title, sizeof (title) - 1, "Chunk @ 0x%"PFMT64x"\n", (ut64)next);
snprintf (chunk, sizeof (chunk) - 1, "fd: 0x%"PFMT64x"\nbk: 0x%"PFMT64x"\n",
(ut64)cnk->fd, (ut64)cnk->bk);
next_node = r_agraph_add_node (g, title, chunk);
r_agraph_add_edge (g, prev_node, next_node);
r_agraph_add_edge (g, next_node, prev_node);
prev_node = next_node;
}

r_agraph_add_edge (g, prev_node, bin_node);
r_agraph_add_edge (g, bin_node, prev_node);
r_agraph_print (g);

free (cnk);
r_agraph_free (g);
return 0;
}

static int GH(print_double_linked_list_bin)(RCore *core, MallocState *main_arena, GHT m_arena, GHT offset, GHT num_bin, int graph) {
if (!core || !core->dbg || !core->dbg->maps) {
return -1;
}
int ret = 0;
GHT brk_start = GHT_MAX, brk_end = GHT_MAX, initial_brk = GHT_MAX;
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

if (num_bin > 126) {
return -1;
}
GHT bin = main_arena->GH(bins)[num_bin];

if (!bin) {
return -1;
}

GH(get_brks) (core, &brk_start, &brk_end);
if (brk_start == GHT_MAX || brk_end == GHT_MAX) {
eprintf ("No Heap section\n");
return -1;
}

const int tcache = r_config_get_i (core->config, "dbg.glibc.tcache");
if (tcache) {
const int fc_offset = r_config_get_i (core->config, "dbg.glibc.fc_offset");
bin = m_arena + offset + SZ * num_bin * 2 + 10 * SZ;
initial_brk = ( (brk_start >> 12) << 12 ) + fc_offset;
} else {
bin = m_arena + offset + SZ * num_bin * 2 - SZ * 2;
initial_brk = (brk_start >> 12) << 12;
}

switch (num_bin) {
case 0:
PRINT_GA (" double linked list unsorted bin {\n");
break;
case 1 ... NSMALLBINS - 1:
PRINT_GA (" double linked list small bin {\n");
break;
case NSMALLBINS ... NBINS - 2:
PRINT_GA (" double linked list large bin {\n");
break;
}

if (!graph || graph == 1) {
ret = GH(print_double_linked_list_bin_simple)(core, bin, main_arena, initial_brk);
} else {
ret = GH(print_double_linked_list_bin_graph)(core, bin, main_arena, initial_brk);
}
PRINT_GA ("\n }\n");
return ret;
}

static void GH(print_heap_bin)(RCore *core, GHT m_arena, MallocState *main_arena, const char *input) {
int i, j = 2;
GHT num_bin = GHT_MAX;
GHT offset;
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

const int tcache = r_config_get_i (core->config, "dbg.glibc.tcache");
if (tcache) {
offset = 16;
} else {
offset = 12 * SZ + sizeof (int) * 2;
}

switch (input[0]) {
case '\0': 
PRINT_YA ("Bins {\n");
for (i = 0; i < NBINS - 1; i++) {
PRINTF_YA (" Bin %03d:\n", i + 1);
GH(print_double_linked_list_bin) (core, main_arena, m_arena, offset, i, 0);
}
PRINT_YA ("\n}\n");
break;
case ' ': 
j--; 

case 'g': 
num_bin = r_num_get (NULL, input + j) - 1;
if (num_bin > NBINS - 2) {
eprintf ("Error: 0 < bin <= %d\n", NBINS - 1);
break;
}
PRINTF_YA (" Bin %03d:\n", num_bin + 1);
GH(print_double_linked_list_bin) (core, main_arena, m_arena, offset, num_bin, j);
break;
}
}

static int GH(print_single_linked_list_bin)(RCore *core, MallocState *main_arena, GHT m_arena, GHT offset, GHT bin_num) {
if (!core || !core->dbg || !core->dbg->maps) {
return -1;
}
GHT next = GHT_MAX, brk_start = GHT_MAX, brk_end = GHT_MAX;
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

GH(RHeapChunk) *cnk = R_NEW0 (GH(RHeapChunk));
if (!cnk) {
return 0;
}

if (!GH(update_main_arena) (core, m_arena, main_arena)) {
free (cnk);
return 0;
}

GHT bin = main_arena->GH(fastbinsY)[bin_num];
if (!bin) {
return -1;
}

bin = m_arena + offset + SZ * bin_num;
r_io_read_at (core->io, bin, (ut8 *)&next, SZ);

GH(get_brks) (core, &brk_start, &brk_end);
if (brk_start == GHT_MAX || brk_end == GHT_MAX) {
eprintf ("No Heap section\n");
free (cnk);
return 0;
}

PRINTF_GA (" fastbin %d @ ", bin_num + 1);
PRINTF_GA ("0x%"PFMT64x" {\n ", (ut64)bin);

GHT size = main_arena->GH(top) - brk_start;

GHT next_root = next, next_tmp = next, double_free = GHT_MAX;
while (next && next >= brk_start && next < main_arena->GH(top)) {
PRINTF_BA ("0x%"PFMT64x, (ut64)next);
while (double_free == GHT_MAX && next_tmp && next_tmp >= brk_start && next_tmp <= main_arena->GH(top)) {
r_io_read_at (core->io, next_tmp, (ut8 *)cnk, sizeof (GH(RHeapChunk)));
next_tmp = cnk->fd;
if (cnk->prev_size > size || ((cnk->size >> 3) << 3) > size) {
break;
}
if (next_root == next_tmp) {
double_free = next_root;
break;
}
}
r_io_read_at (core->io, next, (ut8 *)cnk, sizeof (GH(RHeapChunk)));
next = cnk->fd;
PRINTF_BA ("%s", next ? "->fd = " : "");
if (cnk->prev_size > size || ((cnk->size >> 3) << 3) > size) {
PRINTF_RA (" 0x%"PFMT64x, (ut64)next);
PRINT_RA (" Linked list corrupted\n");
PRINT_GA ("\n }\n");
free (cnk);
return -1;
}

next_root = next_tmp = next;
if (double_free == next) {
PRINTF_RA ("0x%"PFMT64x, (ut64)next);
PRINT_RA (" Double free detected\n");
PRINT_GA ("\n }\n");
free (cnk);
return -1;
}
}

if (next && (next < brk_start || next >= main_arena->GH(top))) {
PRINTF_RA ("0x%"PFMT64x, (ut64)next);
PRINT_RA (" Linked list corrupted\n");
PRINT_GA ("\n }\n");
free (cnk);
return -1;
}

PRINT_GA ("\n }\n");
free (cnk);
return 0;
}

void GH(print_heap_fastbin)(RCore *core, GHT m_arena, MallocState *main_arena, GHT global_max_fast, const char *input) {
int i;
GHT num_bin = GHT_MAX, offset = sizeof (int) * 2;
const int tcache = r_config_get_i (core->config, "dbg.glibc.tcache");
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

if (tcache) {
offset = 16;
}

switch (input[0]) {
case '\0': 
if (core->offset != core->prompt_offset) {
m_arena = core->offset;
}
PRINT_YA ("fastbinY {\n");
for (i = 1; i <= NFASTBINS; i++) {
if (FASTBIN_IDX_TO_SIZE(i) <= global_max_fast) {
PRINTF_YA (" Fastbin %02d\n", i);
} else {
PRINTF_RA (" Fastbin %02d\n", i);
}
if (GH(print_single_linked_list_bin) (core, main_arena, m_arena, offset, i - 1)) {
PRINT_GA (" Empty bin");
PRINT_BA (" 0x0\n");
}
}
PRINT_YA ("}\n");
break;
case ' ': 
num_bin = r_num_get (NULL, input) - 1;
if (num_bin >= NFASTBINS) {
eprintf ("Error: 0 < bin <= %d\n", NFASTBINS);
break;
}
if (GH(print_single_linked_list_bin)(core, main_arena, m_arena, offset, num_bin)) {
PRINT_GA (" Empty bin");
PRINT_BA (" 0x0\n");
}
break;
}
}

static void GH(print_tcache_instance)(RCore *core, GHT m_arena, MallocState *main_arena) {
if (!core || !core->dbg || !core->dbg->maps) {
return;
}
const int tcache = r_config_get_i (core->config, "dbg.glibc.tcache");
if (!tcache) {
return;
}
GHT brk_start = GHT_MAX, brk_end = GHT_MAX, tcache_fd = GHT_MAX, initial_brk = GHT_MAX ;
GH(get_brks) (core, &brk_start, &brk_end);
GHT tcache_tmp = GHT_MAX, tcache_start = GHT_MAX;
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

tcache_start = ((brk_start >> 12) << 12) + GH(HDR_SZ);
GHT fc_offset = GH(tcache_chunk_size) (core, brk_start);
initial_brk = brk_start + fc_offset;
if (brk_start == GHT_MAX || brk_end == GHT_MAX || initial_brk == GHT_MAX) {
eprintf ("No heap section\n");
return;
}

GH(RHeapTcache) *tcache_heap = R_NEW0 (GH(RHeapTcache));
if (!tcache_heap) {
return;
}
(void)r_io_read_at (core->io, tcache_start, (ut8 *)tcache_heap, sizeof (GH(RHeapTcache)));

PRINT_GA("Tcache main arena @");
PRINTF_BA (" 0x%"PFMT64x"\n", (ut64)m_arena);
int i;
for (i = 0; i < TCACHE_MAX_BINS; i++) {
if (tcache_heap->counts[i] > 0) {
PRINT_GA("bin :");
PRINTF_BA("%2d",i);
PRINT_GA(", items :");
PRINTF_BA("%2d",tcache_heap->counts[i]);
PRINT_GA(", fd :");

PRINTF_BA("0x%"PFMT64x, (GHT)(tcache_heap->entries[i]) - GH(HDR_SZ));
if (tcache_heap->counts[i] > 1) {
tcache_fd = (GHT)tcache_heap->entries[i];
int n;
for(n=1; n < tcache_heap->counts[i]; n++) {
(void)r_io_read_at (core->io, tcache_fd, (ut8 *)&tcache_tmp, sizeof (ut64));
PRINTF_BA("->0x%"PFMT64x, tcache_tmp - TC_HDR_SZ);
tcache_fd = tcache_tmp;
}
}
PRINT_BA ("\n");
}
}

if (main_arena->GH(next) != m_arena) {
GHT mmap_start = GHT_MAX, tcache_start = GHT_MAX;
MallocState *ta = R_NEW0 (MallocState);
if (!ta) {
free (tcache_heap);
return;
}
ta->GH(next) = main_arena->GH(next);
while (GH(is_arena) (core, m_arena, ta->GH(next)) && ta->GH(next) != m_arena) {
PRINT_YA ("Tcache thread arena @ ");
PRINTF_BA (" 0x%"PFMT64x, (ut64)ta->GH(next));
mmap_start = ((ta->GH(next) >> 16) << 16);
tcache_start = mmap_start + sizeof (GH(RHeapInfo)) + sizeof(GH(RHeap_MallocState_tcache)) + GH(MMAP_ALIGN);

if (!GH(update_main_arena) (core, ta->GH(next), ta)) {
free (tcache_heap);
free (ta);
return;
}

if (ta->attached_threads) {
PRINT_BA ("\n");
(void)r_io_read_at (core->io, tcache_start, (ut8 *)tcache_heap, sizeof (GH(RHeapTcache)));
int i;

for (i = 0; i < TCACHE_MAX_BINS; i++) {
if (tcache_heap->counts[i] > 0) {
PRINT_GA ("bin :");
PRINTF_BA ("%2d",i);
PRINT_GA (", items :");
PRINTF_BA ("%2d",tcache_heap->counts[i]);
PRINT_GA (", fd :");
PRINTF_BA ("0x%"PFMT64x, (GHT)tcache_heap->entries[i] - GH(HDR_SZ));
if (tcache_heap->counts[i] > 1) {
tcache_fd = (GHT)tcache_heap->entries[i];
int n;
for ( n = 1; n < tcache_heap->counts[i]; n++) {
(void)r_io_read_at (core->io, tcache_fd, (ut8 *)&tcache_tmp, sizeof (GHT));
PRINTF_BA ("->0x%"PFMT64x, tcache_tmp - GH(HDR_SZ));
tcache_fd = tcache_tmp;
}
}
PRINT_BA ("\n");
}
}
} else {
PRINT_GA (" free\n");
}
}
free (tcache_heap);
}
}

static void GH(print_heap_segment)(RCore *core, MallocState *main_arena,
GHT m_arena, GHT m_state, GHT global_max_fast, int format_out) {

if (!core || !core->dbg || !core->dbg->maps) {
return;
}

int w, h;
GHT brk_start = GHT_MAX, brk_end = GHT_MAX, size_tmp, min_size = SZ * 4;
GHT tcache_fd = GHT_MAX, tcache_tmp = GHT_MAX;
GHT initial_brk = GHT_MAX, tcache_initial_brk = GHT_MAX;

const int tcache = r_config_get_i (core->config, "dbg.glibc.tcache");
const int offset = r_config_get_i (core->config, "dbg.glibc.fc_offset");
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

if (m_arena == m_state) {
GH(get_brks) (core, &brk_start, &brk_end);
if (tcache) {
GHT fc_offset = GH(tcache_chunk_size) (core, brk_start);
initial_brk = ((brk_start >> 12) << 12) + fc_offset;
} else {
initial_brk = (brk_start >> 12) << 12;
}
} else {
brk_start = ((m_state >> 16) << 16) ;
brk_end = brk_start + main_arena->GH(system_mem);
if (tcache) {
tcache_initial_brk = brk_start + sizeof (GH(RHeapInfo)) + sizeof (GH(RHeap_MallocState_tcache)) + GH(MMAP_ALIGN);
initial_brk = tcache_initial_brk + offset;
} else {
initial_brk = brk_start + sizeof (GH(RHeapInfo)) + sizeof (GH(RHeap_MallocState)) + MMAP_OFFSET;
}
}

if (brk_start == GHT_MAX || brk_end == GHT_MAX || initial_brk == GHT_MAX) {
eprintf ("No Heap section\n");
return;
}

GHT next_chunk = initial_brk, prev_chunk = next_chunk;
GH(RHeapChunk) *cnk = R_NEW0 (GH(RHeapChunk));
if (!cnk) {
return;
}
GH(RHeapChunk) *cnk_next = R_NEW0 (GH(RHeapChunk));
if (!cnk_next) {
free (cnk);
return;
}

RConfigHold *hc = r_config_hold_new (core->config);
if (!hc) {
free (cnk);
free (cnk_next);
return;
}

w = r_cons_get_size (&h);
RConsCanvas *can = r_cons_canvas_new (w, h);
if (!can) {
free (cnk);
free (cnk_next);
r_config_hold_free (hc);
return;
}

RAGraph *g = r_agraph_new (can);
if (!g) {
free (cnk);
free (cnk_next);
r_cons_canvas_free (can);
r_config_hold_restore (hc);
r_config_hold_free (hc);
return;
}

RANode *top = R_EMPTY, *chunk_node = R_EMPTY, *prev_node = R_EMPTY;
char *top_title, *top_data, *node_title, *node_data;
bool first_node = true;

top_data = r_str_newf ("");
top_title = r_str_newf ("");

(void)r_io_read_at (core->io, next_chunk, (ut8 *)cnk, sizeof (GH(RHeapChunk)));
size_tmp = (cnk->size >> 3) << 3;
ut64 prev_chunk_addr;
ut64 prev_chunk_size;

switch (format_out) {
case 'j':
r_cons_printf ("{\"chunks\":[");
break;
case '*':
r_cons_printf ("fs+heap.allocated\n");
break;
case 'g':
can->linemode = r_config_get_i (core->config, "graph.linemode");
can->color = r_config_get_i (core->config, "scr.color");
core->cons->use_utf8 = r_config_get_i (core->config, "scr.utf8");
g->layout = r_config_get_i (core->config, "graph.layout");
r_agraph_set_title (g, "Heap Layout");
top_title = r_str_newf ("Top chunk @ 0x%"PFMT64x"\n", (ut64)main_arena->GH(top));
}

const char *comma = "";
while (next_chunk && next_chunk >= brk_start && next_chunk < main_arena->GH(top)) {
if (size_tmp < min_size || next_chunk + size_tmp > main_arena->GH(top)) {
const char *status = "corrupted";
switch (format_out) {
case 'c':
PRINT_YA ("\n Malloc chunk @ ");
PRINTF_BA ("0x%"PFMT64x" ", (ut64)next_chunk);
PRINTF_RA ("[%s]\n",status);
PRINTF_RA (" size: 0x%"PFMT64x"\n fd: 0x%"PFMT64x", bk: 0x%"PFMT64x"\n",
(ut64)cnk->size, (ut64)cnk->fd, (ut64)cnk->bk);
break;
case 'j':
r_cons_printf ("%s{\"addr\":%"PFMT64d",\"size\":%"PFMT64d",\"status\":\"%s\",\"fd\":"PFMT64d",\"bk\":"PFMT64d"}",
comma, (ut64)next_chunk, (ut64)cnk->size, status, (ut64)cnk->fd, (ut64)cnk->bk);
comma = ",";
break;
case '*':
r_cons_printf ("fs heap.corrupted\n");
char *name = r_str_newf ("chunk.corrupted.%06x", ((prev_chunk>>4) & 0xffff));
r_cons_printf ("f %s %d 0x%"PFMT64x"\n", name, (int)cnk->size, prev_chunk);
free (name);
break;
case 'g':
node_title = r_str_newf (" Malloc chunk @ 0x%"PFMT64x" ", (ut64)prev_chunk);
node_data = r_str_newf ("[corrupted] size: 0x%"PFMT64x"\n fd: 0x%"PFMT64x", bk: 0x%"PFMT64x
"\nHeap graph could not be recovered\n", (ut64)cnk->size, (ut64)cnk->fd, (ut64)cnk->bk);
r_agraph_add_node (g, node_title, node_data);
if (first_node) {
first_node = false;
}
break;
}
break;
}

prev_chunk_addr = (ut64)prev_chunk;
prev_chunk_size = (((ut64)cnk->size) >> 3) << 3;

bool fastbin = size_tmp >= SZ * 4 && size_tmp <= global_max_fast;
bool is_free = false, double_free = false;

if (fastbin) {
int i = (size_tmp / (SZ * 2)) - 2;
GHT idx = (GHT)main_arena->GH(fastbinsY)[i];
(void)r_io_read_at (core->io, idx, (ut8 *)cnk, sizeof (GH(RHeapChunk)));
GHT next = cnk->fd;
if (prev_chunk == idx && idx && !next) {
is_free = true;
}
while (next && next >= brk_start && next < main_arena->GH(top)) {
if (prev_chunk == idx || prev_chunk == next || idx == next) {
is_free = true;
if (idx == next) {
double_free = true;
break;
}
(void)r_io_read_at (core->io, next, (ut8 *)cnk_next, sizeof (GH(RHeapChunk)));
GHT next_node = cnk_next->fd;

while (next_node && next_node >= brk_start && next_node < main_arena->GH(top)) {
if (prev_chunk == next_node) {
double_free = true;
break;
}
(void)r_io_read_at (core->io, next_node, (ut8 *)cnk_next, sizeof (GH(RHeapChunk)));
next_node = cnk_next->fd;
}
if (double_free) {
break;
}
}
(void)r_io_read_at (core->io, next, (ut8 *)cnk, sizeof (GH(RHeapChunk)));
next = cnk->fd;
}
if (double_free) {
PRINT_RA (" Double free in simple-linked list detected ");
break;
}
prev_chunk_size = ((i + 1) * GH(HDR_SZ)) + GH(HDR_SZ);
}

if (tcache) {
GH(RHeapTcache) *tcache_heap = R_NEW0 (GH(RHeapTcache));
if (!tcache_heap) {
r_cons_canvas_free (can);
r_config_hold_restore (hc);
r_config_hold_free (hc);
free (g);
free (cnk);
free (cnk_next);
return;
}

(void)r_io_read_at (core->io, tcache_initial_brk, (ut8 *)tcache_heap, sizeof (GH(RHeapTcache)));
int i;
for (i=0; i < TCACHE_MAX_BINS; i++) {
if (tcache_heap->counts[i] > 0) {
if ((GHT)tcache_heap->entries[i] - SZ * 2 == prev_chunk) {
is_free = true;
prev_chunk_size = ((i + 1) * TC_HDR_SZ + GH(TC_SZ));
break;
}
if (tcache_heap->counts[i] > 1) {
tcache_fd = (GHT)tcache_heap->entries[i];
int n;
for (n = 1; n < tcache_heap->counts[i]; n++) {
(void)r_io_read_at (core->io, tcache_fd, (ut8*)&tcache_tmp, sizeof (GHT));
if (tcache_tmp - SZ * 2 == prev_chunk) {
is_free = true;
prev_chunk_size = ((i + 1) * TC_HDR_SZ + GH(TC_SZ));
break;
}
tcache_fd = (ut64)tcache_tmp;
}
}
}
}
free (tcache_heap);
}

next_chunk += size_tmp;
prev_chunk = next_chunk;
r_io_read_at (core->io, next_chunk, (ut8 *)cnk, sizeof (GH(RHeapChunk)));
size_tmp = (cnk->size >> 3) << 3;

const char *status = "allocated";
if (fastbin) {
if (is_free) {
status = "free";
}
}
if (!(cnk->size & 1)) {
status = "free";
}
if (tcache) {
if (is_free) {
status = "free";
}
}

switch (format_out) {
case 'c':
PRINT_YA ("\n Malloc chunk @ ");
PRINTF_BA ("0x%"PFMT64x" ", prev_chunk_addr);
PRINT_GA ("[size: ");
PRINTF_BA ("0x%"PFMT64x, prev_chunk_size);
PRINTF_GA ("][%s]",status);
break;
case 'j':
r_cons_printf ("%s{\"addr\":0x%"PFMT64x",\"size\":0x%"PFMT64x",\"status\":\"%s\"}",
comma, prev_chunk_addr, prev_chunk_size, status);
comma = ",";
break;
case '*':
r_cons_printf ("fs heap.%s\n", status);
char *name = r_str_newf ("chunk.%06x", ((prev_chunk_addr>>4) & 0xffff));
r_cons_printf ("f %s %d 0x%"PFMT64x"\n", name, (int)prev_chunk_size, prev_chunk_addr);
free (name);
break;
case 'g':
node_title = r_str_newf (" Malloc chunk @ 0x%"PFMT64x" ", (ut64)prev_chunk_addr);
node_data = r_str_newf ("size: 0x%"PFMT64x" status: %s\n", (ut64)prev_chunk_size, status);
chunk_node = r_agraph_add_node (g, node_title, node_data);
if (first_node) {
first_node = false;
} else {
r_agraph_add_edge (g, prev_node, chunk_node);
}
prev_node = chunk_node;
break;
}
}

switch (format_out) {
case 'c':
PRINT_YA ("\n Top chunk @ ");
PRINTF_BA ("0x%"PFMT64x, (ut64)main_arena->GH(top));
PRINT_GA (" - [brk_start: ");
PRINTF_BA ("0x%"PFMT64x, (ut64)brk_start);
PRINT_GA (", brk_end: ");
PRINTF_BA ("0x%"PFMT64x, (ut64)brk_end);
PRINT_GA ("]\n");
break;
case 'j':
r_cons_printf ("],");
r_cons_printf ("\"top\":0x%"PFMT64x",", (ut64)main_arena->GH(top));
r_cons_printf ("\"brk\":0x%"PFMT64x",", (ut64)brk_start);
r_cons_printf ("\"end\":0x%"PFMT64x, (ut64)brk_end);
r_cons_printf ("}\n");
break;
case '*':
r_cons_printf ("fs-\n");
r_cons_printf ("f heap.top = 0x%08"PFMT64x"\n", (ut64)main_arena->GH(top));
r_cons_printf ("f heap.brk = 0x%08"PFMT64x"\n", (ut64)brk_start);
r_cons_printf ("f heap.end = 0x%08"PFMT64x"\n", (ut64)brk_end);
break;
case 'g':
top = r_agraph_add_node (g, top_title, top_data);
if (!first_node) {
r_agraph_add_edge (g, prev_node, top);
free (node_data);
free (node_title);
}
r_agraph_print (g);
r_cons_canvas_free (can);
r_config_hold_restore (hc);
r_config_hold_free (hc);
break;
}

r_cons_printf ("\n");
free (g);
free (top_data);
free (top_title);
free (cnk);
free (cnk_next);
}

void GH(print_malloc_states)( RCore *core, GHT m_arena, MallocState *main_arena) {
MallocState *ta = R_NEW0 (MallocState);
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

if (!ta) {
return;
}
PRINT_YA ("main_arena @ ");
PRINTF_BA ("0x%"PFMT64x"\n", (ut64)m_arena);
if (main_arena->GH(next) != m_arena) {
ta->GH(next) = main_arena->GH(next);
while (GH(is_arena) (core, m_arena, ta->GH(next)) && ta->GH(next) != m_arena) {
PRINT_YA ("thread arena @ ");
PRINTF_BA ("0x%"PFMT64x, (ut64)ta->GH(next));
if (!GH(update_main_arena) (core, ta->GH(next), ta)) {
free (ta);
return;
}
if (ta->attached_threads) {
PRINT_BA ("\n");
} else {
PRINT_GA (" free\n");
}
}
}
free(ta);
}

void GH(print_inst_minfo)(GH(RHeapInfo) *heap_info, GHT hinfo) {
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

PRINT_YA ("malloc_info @ ");
PRINTF_BA ("0x%"PFMT64x, (ut64)hinfo);
PRINT_YA (" {\n ar_ptr = " );
PRINTF_BA ("0x%"PFMT64x"\n", (ut64)heap_info->ar_ptr);
PRINT_YA (" prev = ");
PRINTF_BA ("0x%"PFMT64x"\n", (ut64)heap_info->prev);
PRINT_YA (" size = ");
PRINTF_BA ("0x%"PFMT64x"\n", (ut64)heap_info->size);
PRINT_YA (" mprotect_size = ");
PRINTF_BA ("0x%"PFMT64x"\n", (ut64)heap_info->mprotect_size);
PRINT_YA ("}\n\n");
}

void GH(print_malloc_info)(RCore *core, GHT m_state, GHT malloc_state) {
GHT h_info;
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

if (malloc_state == m_state) {
PRINT_RA ("main_arena does not have an instance of malloc_info\n");
} else if (GH(is_arena) (core, malloc_state, m_state)) {

h_info = (malloc_state >> 16) << 16;
GH(RHeapInfo) *heap_info = R_NEW0 (GH(RHeapInfo));
if (!heap_info) {
return;
}
r_io_read_at (core->io, h_info, (ut8*)heap_info, sizeof (GH(RHeapInfo)));
GH(print_inst_minfo) (heap_info, h_info);
MallocState *ms = R_NEW0 (MallocState);
if (!ms) {
free (heap_info);
return;
}

while (heap_info->prev != 0x0 && heap_info->prev != GHT_MAX) {
if (!GH(update_main_arena) (core, malloc_state, ms)) {
free (ms);
free (heap_info);
return;
}
if ((ms->GH(top) >> 16) << 16 != h_info) {
h_info = (ms->GH(top) >> 16) << 16;
r_io_read_at (core->io, h_info, (ut8*)heap_info, sizeof (GH(RHeapInfo)));
GH(print_inst_minfo) (heap_info, h_info);
}
}
free (heap_info);
free (ms);
} else {
PRINT_RA ("This address is not part of the arenas\n");
}
}

static const char* GH(help_msg)[] = {
"Usage:", " dmh", " #Memory map heap",
"dmh", "", "List chunks in heap segment",
"dmh", " @[malloc_state]", "List heap chunks of a particular arena",
"dmha", "", "List all malloc_state instances in application",
"dmhb", " @[malloc_state]", "Display all parsed Double linked list of main_arena's or a particular arena bins instance",
"dmhb", " [bin_num|bin_num:malloc_state]", "Display parsed double linked list of bins instance from a particular arena",
"dmhbg"," [bin_num]", "Display double linked list graph of main_arena's bin [Under developemnt]",
"dmhc", " @[chunk_addr]", "Display malloc_chunk struct for a given malloc chunk",
"dmhf", " @[malloc_state]", "Display all parsed fastbins of main_arena's or a particular arena fastbinY instance",
"dmhf", " [fastbin_num|fastbin_num:malloc_state]", "Display parsed single linked list in fastbinY instance from a particular arena",
"dmhg", "", "Display heap graph of heap segment",
"dmhg", " [malloc_state]", "Display heap graph of a particular arena",
"dmhi", " @[malloc_state]", "Display heap_info structure/structures for a given arena",
"dmhm", "", "List all elements of struct malloc_state of main thread (main_arena)",
"dmhm", " @[malloc_state]", "List all malloc_state instance of a particular arena",
"dmht", "", "Display all parsed thread cache bins of all arena's tcache instance",
"dmh?", "", "Show map heap help",
NULL
};

static int GH(cmd_dbg_map_heap_glibc)(RCore *core, const char *input) {
static GHT m_arena = GHT_MAX, m_state = GHT_MAX;
RConsPrintablePalette *pal = &r_cons_singleton ()->context->pal;

GHT global_max_fast = (64 * SZ / 4);

MallocState *main_arena = R_NEW0 (MallocState);
if (!main_arena) {
return false;
}

r_config_set_i (core->config, "dbg.glibc.tcache", GH(is_tcache) (core));

int format = 'c';
bool get_state = false;

switch (input[0]) {
case ' ' : 
m_state = r_num_get (NULL, input);
get_state = true;
case '\0': 
if (GH(r_resolve_main_arena) (core, &m_arena)) {

if (core->offset != core->prompt_offset) {
m_state = core->offset;
} else {
if (!get_state) {
m_state = m_arena;
}
}
if (GH(is_arena) (core, m_arena, m_state)) {
if (!GH(update_main_arena) (core, m_state, main_arena)) {
break;
}
GH(print_heap_segment) (core, main_arena, m_arena, m_state, global_max_fast, format);
break;
} else {
PRINT_RA ("This address is not part of the arenas\n");
break;
}
}
break;
case 'a': 
if (GH(r_resolve_main_arena) (core, &m_arena)) {
if (!GH(update_main_arena) (core, m_arena, main_arena)) {
break;
}
GH(print_malloc_states) (core, m_arena, main_arena);
}
break;
case 'i': 
if (GH(r_resolve_main_arena) (core, &m_arena)) {
if (!GH(update_main_arena) (core, m_arena, main_arena)) {
break;
}
input += 1;
if (!strcmp (input, "\0")) {
if (core->offset != core->prompt_offset) {
m_state = core->offset;
}
} else {
m_state = r_num_get (NULL, input);
}
GH(print_malloc_info) (core, m_arena, m_state);
}
break;
case 'm': 
if (GH(r_resolve_main_arena) (core, &m_arena)) {

switch (input[1]) {
case '*':
format = '*';
input += 1;
break;
case 'j':
format = 'j';
input += 1;
break;
}
input += 1;
if (!strcmp (input, "\0")) {
if (core->offset != core->prompt_offset) {
m_arena = core->offset;
if (!GH(update_main_arena) (core, m_arena, main_arena)) {
break;
}
} else {
if (!GH(update_main_arena) (core, m_arena, main_arena)) {
break;
}
}
} else {
m_arena = r_num_get (NULL, input);
if (!GH(update_main_arena) (core, m_arena, main_arena)) {
break;
}
}
GH(print_arena_stats) (core, m_arena, main_arena, global_max_fast, format);
}
break;
case 'b': 
if (GH(r_resolve_main_arena) (core, &m_arena)) {
char *m_state_str, *dup = strdup (input + 1);
if (*dup) {
strtok (dup, ":");
m_state_str = strtok (NULL, ":");
m_state = r_num_get (NULL, m_state_str);
if (!m_state) {
m_state = m_arena;
}
} else {
if (core->offset != core->prompt_offset) {
m_state = core->offset;
} else {
m_state = m_arena;
}
}
if (GH(is_arena) (core, m_arena, m_state)) {
if (!GH(update_main_arena) (core, m_state, main_arena)) {
free (dup);
break;
}
GH(print_heap_bin) (core, m_state, main_arena, dup);
} else {
PRINT_RA ("This address is not part of the arenas\n");
free (dup);
break;
}
free (dup);
}
break;
case 'c': 
if (GH(r_resolve_main_arena)(core, &m_arena)) {
GH(print_heap_chunk) (core);
}
break;
case 'f': 
if (GH(r_resolve_main_arena) (core, &m_arena)) {
char *m_state_str, *dup = strdup (input + 1);
if (*dup) {
strtok (dup, ":");
m_state_str = strtok (NULL, ":");
m_state = r_num_get (NULL, m_state_str);
if (!m_state) {
m_state = m_arena;
}
} else {
if (core->offset != core->prompt_offset) {
m_state = core->offset;
} else {
m_state = m_arena;
}
}
if (GH(is_arena) (core, m_arena, m_state)) {
if (!GH(update_main_arena) (core, m_state, main_arena)) {
free (dup);
break;
}
GH(print_heap_fastbin) (core, m_state, main_arena, global_max_fast, dup);
} else {
PRINT_RA ("This address is not part of the arenas\n");
free (dup);
break;
}
free (dup);
}
break;
case 'g': 
if (input[0] == 'g') {
format = 'g';
}
case '*': 
if (input[0] == '*') {
format = '*';
}
case 'j': 
if (input[0] == 'j') {
format = 'j';
}
if (GH(r_resolve_main_arena) (core, &m_arena)) {

input += 1;
if (!strcmp (input, "\0")) {
if (core->offset != core->prompt_offset) {
m_state = core->offset;
get_state = true;
}
} else {
m_state = r_num_get (NULL, input);
get_state = true;
}
if (!get_state) {
m_state = m_arena;
}
if (GH(is_arena) (core, m_arena, m_state)) {
if (!GH(update_main_arena) (core, m_state, main_arena)) {
break;
}
GH(print_heap_segment) (core, main_arena, m_arena, m_state, global_max_fast, format);
} else {
PRINT_RA ("This address is not part of the arenas\n");
}
}
break;
case 't':
if (GH(r_resolve_main_arena) (core, &m_arena)) {
if (!GH(update_main_arena) (core, m_arena, main_arena)) {
break;
}
GH(print_tcache_instance) (core, m_arena, main_arena);
}
break;
case '?':
r_core_cmd_help (core, GH(help_msg));
break;
}
free (main_arena);
return true;
}
