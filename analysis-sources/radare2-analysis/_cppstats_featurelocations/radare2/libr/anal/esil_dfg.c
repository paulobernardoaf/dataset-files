


#include <r_anal.h>



typedef struct esil_dfg_reg_var_t {
ut32 from;
ut32 to;
RGraphNode *node;
} EsilDFGRegVar;

typedef struct r_anal_esil_dfg_filter_t {
RAnalEsilDFG *dfg;
RContRBTree *tree;
Sdb *results;
} RAnalEsilDFGFilter;



R_API RAnalEsilDFGNode *r_anal_esil_dfg_node_new(RAnalEsilDFG *edf, const char *c) {
RAnalEsilDFGNode *ret = R_NEW0 (RAnalEsilDFGNode);
ret->content = r_strbuf_new (c);
ret->idx = edf->idx++;
return ret;
}

static void _dfg_node_free (RAnalEsilDFGNode *free_me) {
if (free_me) {
r_strbuf_free (free_me->content);
free (free_me);
}
}

static int _rv_del_alloc_cmp (void *incoming, void *in, void *user) {
EsilDFGRegVar *rv_incoming = (EsilDFGRegVar *)incoming;
EsilDFGRegVar *rv_in = (EsilDFGRegVar *)in;
RAnalEsilDFG *dfg = (RAnalEsilDFG *)user;

if (dfg->malloc_failed) {
return -1;
}


if (rv_incoming->to < rv_in->from) {
return -1;
}
if (rv_in->to < rv_incoming->from) {
return 1;
}
if (rv_in->from == rv_incoming->from && rv_in->to == rv_incoming->to) {
return 0;
}














if (rv_in->from < rv_incoming->from && rv_incoming->to < rv_in->to) {
EsilDFGRegVar *rv = R_NEW (EsilDFGRegVar);
if (!rv) {
dfg->malloc_failed = true;
return -1;
}
rv[0] = rv_in[0];
rv_in->to = rv_incoming->from - 1;
rv->from = rv_incoming->to + 1;
dfg->insert = rv;
return 1;
}








if (rv_incoming->from < rv_in->from && rv_in->to < rv_incoming->to) {

EsilDFGRegVar *rv = R_NEW (EsilDFGRegVar);
if (!rv) {
dfg->malloc_failed = true;
return -1;
}
rv[0] = rv_incoming[0];
rv->to = rv_in->from - 1;
r_queue_enqueue (dfg->todo, rv);

rv = R_NEW (EsilDFGRegVar);
if (!rv) {
dfg->malloc_failed = true;
return -1;
}
rv[0] = rv_incoming[0];
rv->from = rv_in->to + 1;
r_queue_enqueue (dfg->todo, rv);
return 0;
}








if (rv_incoming->from == rv_in->from && rv_in->to < rv_incoming->to) {
EsilDFGRegVar *rv = R_NEW (EsilDFGRegVar);
if (!rv) {
dfg->malloc_failed = true;
return -1;
}
rv[0] = rv_incoming[0];
rv->from = rv_in->to + 1;
r_queue_enqueue (dfg->todo, rv);
return 0;
}






if (rv_incoming->from < rv_in->from && rv_in->to == rv_incoming->to) {
EsilDFGRegVar *rv = R_NEW (EsilDFGRegVar);
if (!rv) {
dfg->malloc_failed = true;
return -1;
}
rv[0] = rv_incoming[0];
rv->to = rv_in->from - 1;
r_queue_enqueue (dfg->todo, rv);
return 0;
}











if (rv_in->to <= rv_incoming->to) {
rv_in->to = rv_incoming->from - 1;
return 1;
}











rv_in->from = rv_incoming->to + 1;
return -1;
}

static int _rv_ins_cmp (void *incoming, void *in, void *user) {
EsilDFGRegVar *rv_incoming = (EsilDFGRegVar *)incoming;
EsilDFGRegVar *rv_in = (EsilDFGRegVar *)in;
return rv_incoming->from - rv_in->from;
}

static bool _edf_reg_set (RAnalEsilDFG *dfg, const char *reg, RGraphNode *node) {
r_return_val_if_fail (dfg && !dfg->malloc_failed && reg, false);
const ut32 _reg_strlen = 4 + strlen (reg);
char *_reg = R_NEWS0 (char, _reg_strlen + 1);
if (!_reg) {

return false;
}
strncat (_reg, "reg.", _reg_strlen);
strncat (_reg, reg, _reg_strlen);
if (!sdb_num_exists (dfg->regs, _reg)) {

free (_reg);
return false;
}
EsilDFGRegVar *rv = R_NEW0 (EsilDFGRegVar);
if (!rv) {
free (_reg);
return false;
}

const ut64 v = sdb_num_get (dfg->regs, _reg, NULL);
free (_reg);
rv->from = (v & (UT64_MAX ^ UT32_MAX)) >> 32;
rv->to = v & UT32_MAX;
r_queue_enqueue (dfg->todo, rv);
while (!r_queue_is_empty (dfg->todo) && !dfg->malloc_failed) {

rv = r_queue_dequeue (dfg->todo);
r_rbtree_cont_delete (dfg->reg_vars, rv, _rv_del_alloc_cmp, dfg);
if (dfg->insert && !dfg->malloc_failed) {
r_rbtree_cont_insert (dfg->reg_vars, dfg->insert, _rv_ins_cmp, NULL);
dfg->insert = NULL;
}
free (rv);
}
if (dfg->malloc_failed) {
while (!r_queue_is_empty (dfg->todo)) {
free (r_queue_dequeue (dfg->todo));
}
return false;
}
rv = R_NEW0 (EsilDFGRegVar);
rv->from = (v & (UT64_MAX ^ UT32_MAX)) >> 32;
rv->to = v & UT32_MAX;
rv->node = node;
r_rbtree_cont_insert (dfg->reg_vars, rv, _rv_ins_cmp, NULL);
return true;
}

static int _rv_find_cmp (void *incoming, void *in, void *user) {
EsilDFGRegVar *rv_incoming = (EsilDFGRegVar *)incoming;
EsilDFGRegVar *rv_in = (EsilDFGRegVar *)in;

RAnalEsilDFG *dfg = (RAnalEsilDFG *)user;
if (dfg->malloc_failed) {
return -1;
}


if (rv_incoming->to < rv_in->from) {
return -1;
}
if (rv_in->to < rv_incoming->from) {
return 1;
}





if (rv_in->from <= rv_incoming->from && rv_incoming->to <= rv_in->to) {
return 0;
}







if (rv_incoming->from < rv_in->from && rv_in->to < rv_incoming->to) {

EsilDFGRegVar *rv = R_NEW (EsilDFGRegVar);
if (!rv) {
dfg->malloc_failed = true;
return -1;
}
rv[0] = rv_incoming[0];
rv->to = rv_in->from - 1;
r_queue_enqueue (dfg->todo, rv);

rv = R_NEW (EsilDFGRegVar);
if (!rv) {
dfg->malloc_failed = true;
return -1;
}
rv[0] = rv_incoming[0];
rv->from = rv_in->to + 1;
r_queue_enqueue (dfg->todo, rv);
return 0;
}







if (rv_in->from <= rv_incoming->from && rv_in->to < rv_incoming->to) {
EsilDFGRegVar *rv = R_NEW (EsilDFGRegVar);
if (!rv) {
dfg->malloc_failed = true;
return -1;
}
rv[0] = rv_incoming[0];
rv->from = rv_in->to + 1;
r_queue_enqueue (dfg->todo, rv);
return 0;
}





EsilDFGRegVar *rv = R_NEW (EsilDFGRegVar);
if (!rv) {
dfg->malloc_failed = true;
return -1;
}
rv[0] = rv_incoming[0];
rv->to = rv_in->from - 1;
r_queue_enqueue (dfg->todo, rv);
return 0;
}

static RGraphNode *_edf_origin_reg_get(RAnalEsilDFG *dfg, const char *reg) {
r_return_val_if_fail (dfg && reg, NULL);
const ut32 _reg_strlen = 4 + strlen (reg);
char *_reg = R_NEWS0 (char, _reg_strlen + 1);
if (!_reg) {
return NULL;
}
strncat (_reg, "reg.", _reg_strlen);
strncat (_reg, reg, _reg_strlen);
if (!sdb_num_exists (dfg->regs, _reg)) {
free (_reg);
return NULL;
}
free (_reg);
const ut32 origin_reg_strlen = 4 + strlen (reg);
char *origin_reg = R_NEWS0 (char, origin_reg_strlen + 1);
if (!origin_reg) {
return NULL;
}
strncat (origin_reg, "ori.", origin_reg_strlen);
strncat (origin_reg, reg, origin_reg_strlen);
RGraphNode *origin_reg_node = sdb_ptr_get (dfg->regs, origin_reg, 0);
if (origin_reg_node) {
free (origin_reg);
return origin_reg_node;
}
RGraphNode *reg_node = r_graph_add_node (dfg->flow, r_anal_esil_dfg_node_new (dfg, reg));
RAnalEsilDFGNode *_origin_reg_node = r_anal_esil_dfg_node_new (dfg, reg);
r_strbuf_appendf (_origin_reg_node->content, ":var_%d", dfg->idx++);
_origin_reg_node->type = R_ANAL_ESIL_DFG_BLOCK_VAR;
origin_reg_node = r_graph_add_node (dfg->flow, _origin_reg_node);
r_graph_add_edge (dfg->flow, reg_node, origin_reg_node);
sdb_ptr_set (dfg->regs, origin_reg, origin_reg_node, 0);
free (origin_reg);
return origin_reg_node;
}


static RGraphNode *_edf_reg_get(RAnalEsilDFG *dfg, const char *reg) {
r_return_val_if_fail (dfg && reg, NULL);
const ut32 _reg_strlen = 4 + strlen (reg);
char *_reg = R_NEWS0 (char, _reg_strlen + 1);
if (!_reg) {
return NULL;
}
strncat (_reg, "reg.", _reg_strlen);
strncat (_reg, reg, _reg_strlen);
if (!sdb_num_exists (dfg->regs, _reg)) {
free (_reg);
return NULL;
}
EsilDFGRegVar *rv = R_NEW0 (EsilDFGRegVar);
if (!rv) {
free (_reg);
return NULL;
}
const ut64 v = sdb_num_get (dfg->regs, _reg, NULL);
free (_reg);
rv->from = (v & (UT64_MAX ^ UT32_MAX)) >> 32;
rv->to = v & UT32_MAX;
RQueue *parts = r_queue_new (8);
if (!parts) {
free (rv);
return NULL;
}
r_queue_enqueue (dfg->todo, rv);




RGraphNode *reg_node = NULL;
while (!r_queue_is_empty (dfg->todo)) {
rv = r_queue_dequeue (dfg->todo);
EsilDFGRegVar *part_rv = r_rbtree_cont_find (dfg->reg_vars, rv, _rv_find_cmp, dfg);
if (part_rv) {
r_queue_enqueue (parts, part_rv->node);
} else if (!reg_node) {
reg_node = _edf_origin_reg_get (dfg, reg);

part_rv = R_NEW (EsilDFGRegVar);
if (!part_rv) {
R_FREE (rv);
dfg->malloc_failed = true;
break;
}
part_rv[0] = rv[0];
part_rv->node = reg_node;
r_rbtree_cont_insert (dfg->reg_vars, part_rv, _rv_ins_cmp, NULL);

r_queue_enqueue (parts, reg_node);
} else {


part_rv = R_NEW (EsilDFGRegVar);
if (!part_rv) {
R_FREE (part_rv);
dfg->malloc_failed = true;
break;
}
part_rv[0] = rv[0];
part_rv->node = reg_node;
r_rbtree_cont_insert (dfg->reg_vars, part_rv, _rv_ins_cmp, NULL);
}
free (rv);
}
reg_node = NULL; 
if (dfg->malloc_failed) {
while (!r_queue_is_empty(dfg->todo)) {
free (r_queue_dequeue (dfg->todo));
goto beach;
}
}
switch (parts->size) {
case 0:
break;
case 1:
reg_node = r_queue_dequeue (parts);
break;
default:
{
RAnalEsilDFGNode *_reg_node = r_anal_esil_dfg_node_new (dfg, "merge to ");
if (!_reg_node) {
while (!r_queue_is_empty (dfg->todo)) {
free (r_queue_dequeue (dfg->todo));
}
dfg->malloc_failed = true;
goto beach;
}

r_strbuf_appendf (_reg_node->content, "%s:var_%d", reg, dfg->idx++);
reg_node = r_graph_add_node (dfg->flow, _reg_node);
if (!reg_node) {
_dfg_node_free (_reg_node);
while (!r_queue_is_empty (dfg->todo)) {
free (r_queue_dequeue (dfg->todo));
}
dfg->malloc_failed = true;
goto beach;
}
}
do {
r_graph_add_edge (dfg->flow, r_queue_dequeue(parts), reg_node);
} while (!r_queue_is_empty (parts));
break;
}
beach:
r_queue_free (parts);
return reg_node;
}


static bool _edf_var_set (RAnalEsilDFG *dfg, const char *var, RGraphNode *node) {
r_return_val_if_fail (dfg && var, false);
const ut32 _var_strlen = 4 + strlen (var);
char *_var = R_NEWS0 (char, _var_strlen + 1);
if (!_var) {
return false;
}
strncat (_var, "var.", _var_strlen);
strncat (_var, var, _var_strlen);
const bool ret = !sdb_ptr_set (dfg->regs, _var, node, 0);
free (_var);
return ret;
}


static RGraphNode *_edf_var_get (RAnalEsilDFG *dfg, const char *var) {
r_return_val_if_fail (dfg && var, NULL);
const ut32 _var_strlen = 4 + strlen (var);
char *_var = R_NEWS0 (char, _var_strlen + 1);
if (!_var) {
return NULL;
}
strncat (_var, "var.", _var_strlen);
strncat (_var, var, _var_strlen);
RGraphNode *ret = sdb_ptr_get (dfg->regs, _var, NULL);
free (_var);
return ret;
}

static bool edf_consume_2_set_reg(RAnalEsil *esil);
static bool edf_consume_2_push_1(RAnalEsil *esil);
static bool edf_consume_1_push_1(RAnalEsil *esil);
typedef void (*AddConstraintStringUseNewCB) (RStrBuf *result, const char *new_node_str);
static bool edf_use_new_push_1(RAnalEsil *esil, const char *op_string, AddConstraintStringUseNewCB cb);
typedef void (*AddConstraintStringConsume1UseOldNewCB) (RStrBuf *result, const char *consume_str, const char *old_node_str, const char *new_node_str);
static bool edf_consume_1_use_old_new_push_1(RAnalEsil *esil, const char *op_string, AddConstraintStringConsume1UseOldNewCB cb);

static bool edf_eq_weak(RAnalEsil *esil) {
RAnalEsilDFG *edf = (RAnalEsilDFG *)esil->user;
RGraphNode *o_old = edf->old; 
RGraphNode *o_new = edf->cur; 
if (!edf_consume_2_set_reg (esil)) {
return false;
}

edf->old = o_old ? o_old : NULL;
edf->cur = o_new ? o_new : NULL;
return true;
}

static void edf_zf_constraint(RStrBuf *result, const char *new_node_str) {
r_strbuf_appendf (result, ":(%s==0)", new_node_str);
}

static bool edf_zf(RAnalEsil *esil) {
return edf_use_new_push_1 (esil, "$z", edf_zf_constraint);
}

static void edf_pf_constraint(RStrBuf *result, const char *new_node_str) {
r_strbuf_appendf (result, ":parity_of(%s)", new_node_str);
}

static bool edf_pf(RAnalEsil *esil) {
return edf_use_new_push_1 (esil, "$p", edf_pf_constraint);
}

static void edf_cf_constraint(RStrBuf *result, const char *consume, const char *o, const char *n) {
r_strbuf_appendf (result, ":((%s&mask(%s&0x3f))<(%s&mask(%s&0x3f)))",
n, consume, o, consume);
}

static bool edf_cf(RAnalEsil *esil) {
return edf_consume_1_use_old_new_push_1 (esil, "$c", edf_cf_constraint);
}

static void edf_bf_constraint(RStrBuf *result, const char *consume, const char *o, const char *n) {
r_strbuf_appendf (result, ":((%s&mask((%s+0x3f)&0x3f))<(%s& mask((%s+0x3f)&0x3f)))",
o, consume, n, consume);
}

static bool edf_bf(RAnalEsil *esil) {
return edf_consume_1_use_old_new_push_1 (esil, "$b", edf_bf_constraint);
}

static bool _edf_consume_2_set_reg(RAnalEsil *esil, const bool use_origin) {
const char *op_string = esil->current_opstr;
RAnalEsilDFG *edf = (RAnalEsilDFG *)esil->user;
char *dst = r_anal_esil_pop (esil);
char *src = r_anal_esil_pop (esil);

if (!src || !dst) {
free (dst);
free (src);
return false;
}

int dst_type = r_anal_esil_get_parm_type (esil, dst);
if (dst_type == R_ANAL_ESIL_PARM_INVALID) {
free (dst);
free (src);
return false;
}

const int src_type = r_anal_esil_get_parm_type (esil, src);
RGraphNode *src_node = NULL;
if (src_type == R_ANAL_ESIL_PARM_REG) {
src_node = _edf_reg_get (edf, src);
} else if (src_type == R_ANAL_ESIL_PARM_NUM) {
RGraphNode *n_value = r_graph_add_node (edf->flow, r_anal_esil_dfg_node_new (edf, src));
RAnalEsilDFGNode *ec_node = r_anal_esil_dfg_node_new (edf, src);
ec_node->type = R_ANAL_ESIL_DFG_BLOCK_CONST;
r_strbuf_appendf (ec_node->content, ":const_%d", edf->idx++);
src_node = r_graph_add_node (edf->flow, ec_node);
r_graph_add_edge (edf->flow, n_value, src_node);
} else {
src_node = _edf_var_get (edf, src);
}

RGraphNode *dst_node = use_origin ? _edf_origin_reg_get (edf, dst) : _edf_reg_get (edf, dst);
RGraphNode *old_dst_node = dst_node;

if (!src_node || !dst_node) {
free (src);
free (dst);
return false;
}

RAnalEsilDFGNode *eop_node = r_anal_esil_dfg_node_new (edf, src);
r_strbuf_appendf (eop_node->content, ",%s,%s", dst, op_string);
eop_node->type = R_ANAL_ESIL_DFG_BLOCK_GENERATIVE;
free (src);

RGraphNode *op_node = r_graph_add_node (edf->flow, eop_node);
r_graph_add_edge (edf->flow, dst_node, op_node);
r_graph_add_edge (edf->flow, src_node, op_node);
edf->old = old_dst_node;
RAnalEsilDFGNode *result = r_anal_esil_dfg_node_new (edf, dst);
result->type = R_ANAL_ESIL_DFG_BLOCK_RESULT | R_ANAL_ESIL_DFG_BLOCK_VAR;

r_strbuf_appendf (result->content, ":var_%d", edf->idx++);
dst_node = r_graph_add_node (edf->flow, result);
r_graph_add_edge (edf->flow, op_node, dst_node);
_edf_reg_set (edf, dst, dst_node);
edf->cur = dst_node;
free (dst);
return true;
}

static bool edf_consume_2_use_set_reg(RAnalEsil *esil) {
return _edf_consume_2_set_reg (esil, false);
}

static bool edf_consume_2_set_reg(RAnalEsil *esil) {
return _edf_consume_2_set_reg (esil, true);
}

static bool edf_consume_2_push_1(RAnalEsil *esil) {
const char *op_string = esil->current_opstr;
RAnalEsilDFG *edf = (RAnalEsilDFG *)esil->user;
char *src[2] = { r_anal_esil_pop (esil), r_anal_esil_pop (esil) };

if (!src[0] || !src[1]) {
free (src[0]);
free (src[1]);
return false;
}
RAnalEsilDFGNode *eop_node = r_anal_esil_dfg_node_new (edf, src[1]);
r_strbuf_appendf (eop_node->content, ",%s,%s", src[0], op_string);
eop_node->type = R_ANAL_ESIL_DFG_BLOCK_RESULT | R_ANAL_ESIL_DFG_BLOCK_GENERATIVE;
RGraphNode *op_node = r_graph_add_node (edf->flow, eop_node);
RGraphNode *src_node[2];
ut32 i;
for (i = 0; i < 2; i++) {
const int src_type = r_anal_esil_get_parm_type (esil, src[i]);
if (src_type == R_ANAL_ESIL_PARM_REG) {
src_node[i] = _edf_reg_get (edf, src[i]);
} else if (src_type == R_ANAL_ESIL_PARM_NUM) {
RGraphNode *n_value = r_graph_add_node (edf->flow, r_anal_esil_dfg_node_new (edf, src[i]));
RAnalEsilDFGNode *ec_node = r_anal_esil_dfg_node_new (edf, src[i]);
ec_node->type = R_ANAL_ESIL_DFG_BLOCK_CONST;
r_strbuf_appendf (ec_node->content, ":const_%d", edf->idx++);
src_node[i] = r_graph_add_node (edf->flow, ec_node);
r_graph_add_edge (edf->flow, n_value, src_node[i]);
} else {
src_node[i] = _edf_var_get (edf, src[i]);
}
r_graph_add_edge (edf->flow, src_node[i], op_node);
}

free (src[0]);
free (src[1]);

RAnalEsilDFGNode *result = r_anal_esil_dfg_node_new (edf, "result_");
result->type = R_ANAL_ESIL_DFG_BLOCK_RESULT;
r_strbuf_appendf (result->content, "%d", edf->idx++);
RGraphNode *result_node = r_graph_add_node (edf->flow, result);
r_graph_add_edge (edf->flow, op_node, result_node);
_edf_var_set (edf, r_strbuf_get (result->content), result_node);
r_anal_esil_push (esil, r_strbuf_get (result->content));
return true;
}

static bool edf_consume_1_push_1(RAnalEsil *esil) {
const char *op_string = esil->current_opstr;
RAnalEsilDFG *edf = (RAnalEsilDFG *)esil->user;
char *src = r_anal_esil_pop (esil);
if (!src) {
return false;
}
RAnalEsilDFGNode *eop_node = r_anal_esil_dfg_node_new (edf, src);
r_strbuf_appendf (eop_node->content, ",%s", op_string);
eop_node->type = R_ANAL_ESIL_DFG_BLOCK_RESULT | R_ANAL_ESIL_DFG_BLOCK_GENERATIVE;
RGraphNode *op_node = r_graph_add_node (edf->flow, eop_node);
const int src_type = r_anal_esil_get_parm_type (esil, src);
RGraphNode *src_node = NULL;
if (src_type == R_ANAL_ESIL_PARM_REG) {
src_node = _edf_reg_get (edf, src);
} else if (src_type == R_ANAL_ESIL_PARM_NUM) {
RGraphNode *n_value = r_graph_add_node (edf->flow, r_anal_esil_dfg_node_new (edf, src));
RAnalEsilDFGNode *ec_node = r_anal_esil_dfg_node_new (edf, src);
ec_node->type = R_ANAL_ESIL_DFG_BLOCK_CONST;
r_strbuf_appendf (ec_node->content, ":const_%d", edf->idx++);
src_node = r_graph_add_node (edf->flow, ec_node);
r_graph_add_edge (edf->flow, n_value, src_node);
} else {
src_node = _edf_var_get (edf, src);
}

free (src);

r_graph_add_edge (edf->flow, src_node, op_node);

RAnalEsilDFGNode *result = r_anal_esil_dfg_node_new (edf, "result_");
result->type = R_ANAL_ESIL_DFG_BLOCK_RESULT; 
r_strbuf_appendf (result->content, "%d", edf->idx++);
RGraphNode *result_node = r_graph_add_node (edf->flow, result);
r_graph_add_edge (edf->flow, op_node, result_node);
_edf_var_set (edf, r_strbuf_get (result->content), result_node);
r_anal_esil_push (esil, r_strbuf_get (result->content));
return true;
}

static bool edf_consume_2_set_mem(RAnalEsil *esil) {
const char *op_string = esil->current_opstr;
RAnalEsilDFG *edf = (RAnalEsilDFG *)esil->user;
char *dst = r_anal_esil_pop (esil);
char *src = r_anal_esil_pop (esil);

if (!src || !dst) {
free (dst);
free (src);
return 0;
}

int dst_type = r_anal_esil_get_parm_type (esil, dst);

const int src_type = r_anal_esil_get_parm_type (esil, src);
RGraphNode *src_node = NULL;
if (src_type == R_ANAL_ESIL_PARM_REG) {
src_node = _edf_reg_get (edf, src);
} else if (src_type == R_ANAL_ESIL_PARM_NUM) {
RGraphNode *n_value = r_graph_add_node (edf->flow, r_anal_esil_dfg_node_new (edf, src));
RAnalEsilDFGNode *ec_node = r_anal_esil_dfg_node_new (edf, src);
ec_node->type = R_ANAL_ESIL_DFG_BLOCK_CONST;
r_strbuf_appendf (ec_node->content, ":const_%d", edf->idx++);
src_node = r_graph_add_node (edf->flow, ec_node);
r_graph_add_edge (edf->flow, n_value, src_node);
} else {
src_node = _edf_var_get (edf, src);
}

RGraphNode *dst_node = _edf_reg_get (edf, dst);
if (!dst_node) {
dst_node = _edf_var_get (edf, dst);
}

if (!dst_node) {
if (dst_type == R_ANAL_ESIL_PARM_REG) {
RGraphNode *n_reg = r_graph_add_node (edf->flow, r_anal_esil_dfg_node_new (edf, dst));
RAnalEsilDFGNode *ev_node = r_anal_esil_dfg_node_new (edf, dst);
ev_node->type = R_ANAL_ESIL_DFG_BLOCK_VAR | R_ANAL_ESIL_DFG_BLOCK_PTR;
r_strbuf_appendf (ev_node->content, ":var_ptr_%d", edf->idx++);
dst_node = r_graph_add_node (edf->flow, ev_node);

r_graph_add_edge (edf->flow, n_reg, dst_node);
}

}

if (!src_node || !dst_node) {
free (src);
free (dst);
return false;
}

RAnalEsilDFGNode *eop_node = r_anal_esil_dfg_node_new (edf, src);
r_strbuf_appendf (eop_node->content, ",%s,%s", dst, op_string);
eop_node->type = R_ANAL_ESIL_DFG_BLOCK_GENERATIVE;
free (src);

RGraphNode *op_node = r_graph_add_node (edf->flow, eop_node);
r_graph_add_edge (edf->flow, dst_node, op_node);
r_graph_add_edge (edf->flow, src_node, op_node);
RAnalEsilDFGNode *result = r_anal_esil_dfg_node_new (edf, dst);

result->type = R_ANAL_ESIL_DFG_BLOCK_VAR;
r_strbuf_appendf (result->content, ":var_mem_%d", edf->idx++);
dst_node = r_graph_add_node (edf->flow, result);
r_graph_add_edge (edf->flow, op_node, dst_node);
free (dst);
return true;
}

static bool edf_use_new_push_1(RAnalEsil *esil, const char *op_string, AddConstraintStringUseNewCB cb) {
RAnalEsilDFG *edf = (RAnalEsilDFG *)esil->user;
RGraphNode *op_node = r_graph_add_node (edf->flow, r_anal_esil_dfg_node_new (edf, op_string));
RGraphNode *latest_new = edf->cur;
if (!latest_new) {
return 0;
}
RAnalEsilDFGNode *result = r_anal_esil_dfg_node_new (edf, "result_");
result->type = R_ANAL_ESIL_DFG_BLOCK_RESULT; 
r_strbuf_appendf (result->content, "%d", edf->idx++);
if (cb) {
RAnalEsilDFGNode *e_new_node = (RAnalEsilDFGNode *)latest_new->data;
cb (result->content, r_strbuf_get (e_new_node->content));
}
RGraphNode *result_node = r_graph_add_node (edf->flow, result);
_edf_var_set (edf, r_strbuf_get (result->content), result_node);
r_graph_add_edge (edf->flow, latest_new, op_node);
r_graph_add_edge (edf->flow, op_node, result_node);
return r_anal_esil_push (esil, r_strbuf_get (result->content));
}

static bool edf_consume_1_use_old_new_push_1(RAnalEsil *esil, const char *op_string, AddConstraintStringConsume1UseOldNewCB cb) {
RAnalEsilDFG *edf = (RAnalEsilDFG *)esil->user;
char *src = r_anal_esil_pop (esil);

if (!src) {
return false;
}
RAnalEsilDFGNode *eop_node = r_anal_esil_dfg_node_new (edf, src);
#if 0
eop_node->type = R_ANAL_ESIL_DFG_BLOCK_GENERATIVE;
#endif
r_strbuf_appendf (eop_node->content, ",%s", op_string);
RGraphNode *op_node = r_graph_add_node (edf->flow, eop_node);
const int src_type = r_anal_esil_get_parm_type (esil, src);
RGraphNode *src_node = NULL;
if (src_type == R_ANAL_ESIL_PARM_REG) {
src_node = _edf_reg_get (edf, src);
} else if (src_type == R_ANAL_ESIL_PARM_NUM) {
RGraphNode *n_value = r_graph_add_node (edf->flow, r_anal_esil_dfg_node_new (edf, src));
RAnalEsilDFGNode *ec_node = r_anal_esil_dfg_node_new (edf, src);
ec_node->type = R_ANAL_ESIL_DFG_BLOCK_CONST;
r_strbuf_appendf (ec_node->content, ":const_%d", edf->idx++);
src_node = r_graph_add_node (edf->flow, ec_node);
r_graph_add_edge (edf->flow, n_value, src_node);
} else {
src_node = _edf_var_get (edf, src);
}
free (src);

r_graph_add_edge (edf->flow, src_node, op_node);

RGraphNode *latest_new = edf->cur;
RGraphNode *latest_old = edf->old;
RAnalEsilDFGNode *result = r_anal_esil_dfg_node_new (edf, "result_");
result->type = R_ANAL_ESIL_DFG_BLOCK_RESULT; 
r_strbuf_appendf (result->content, "%d", edf->idx++);
if (cb) {
RAnalEsilDFGNode *e_src_node = (RAnalEsilDFGNode *)src_node->data;
RAnalEsilDFGNode *e_new_node = (RAnalEsilDFGNode *)latest_new->data;
RAnalEsilDFGNode *e_old_node = (RAnalEsilDFGNode *)latest_old->data;
cb (result->content, r_strbuf_get (e_src_node->content),
r_strbuf_get (e_new_node->content), r_strbuf_get (e_old_node->content));
}
RGraphNode *result_node = r_graph_add_node (edf->flow, result);
_edf_var_set (edf, r_strbuf_get (result->content), result_node);
r_graph_add_edge (edf->flow, latest_new, op_node);
r_graph_add_edge (edf->flow, latest_old, op_node);
r_graph_add_edge (edf->flow, op_node, result_node);
return r_anal_esil_push (esil, r_strbuf_get (result->content));
}

R_API RAnalEsilDFG *r_anal_esil_dfg_new(RReg *regs) {
if (!regs) {
return NULL;
}
RAnalEsilDFG *dfg = R_NEW0 (RAnalEsilDFG);
if (!dfg) {
return NULL;
}
dfg->flow = r_graph_new ();
if (!dfg->flow) {
free (dfg);
return NULL;
}
dfg->regs = sdb_new0 ();
if (!dfg->regs) {
r_graph_free (dfg->flow);
free (dfg);
return NULL;
}

dfg->todo = r_queue_new (8);
if (!dfg->todo) {
sdb_free (dfg->regs);
r_graph_free (dfg->flow);
free (dfg);
return NULL;
}
dfg->reg_vars = r_rbtree_cont_newf (free);
if (!dfg->reg_vars) {
r_queue_free (dfg->todo);
sdb_free (dfg->regs);
r_graph_free (dfg->flow);
free (dfg);
return NULL;
}




RRegItem *ri;
RListIter *ator;
r_list_foreach (regs->allregs, ator, ri) {
const ut32 from = ri->offset;
const ut32 to = from + ri->size - 1; 
const ut64 v = to | (((ut64)from) << 32);
const ut32 reg_strlen = 4 + strlen (ri->name) + 1;
char *reg = R_NEWS0 (char, reg_strlen);
strncat (reg, "reg.", reg_strlen);
strncat (reg, ri->name, reg_strlen);
sdb_num_set (dfg->regs, reg, v, 0);
free (reg);
}
return dfg;
}

R_API void r_anal_esil_dfg_free(RAnalEsilDFG *dfg) {
if (dfg) {
if (dfg->flow) {
RGraphNode *n;
RListIter *iter;
r_list_foreach (r_graph_get_nodes (dfg->flow), iter, n) {
n->free = (RListFree)_dfg_node_free;
}
r_graph_free (dfg->flow);
}
sdb_free (dfg->regs);
r_rbtree_cont_free (dfg->reg_vars);
r_queue_free (dfg->todo);
free (dfg);
}
}

R_API RAnalEsilDFG *r_anal_esil_dfg_expr(RAnal *anal, RAnalEsilDFG *dfg, const char *expr) {
if (!expr) {
return NULL;
}
RAnalEsil *esil = r_anal_esil_new (4096, 0, 1);
if (!esil) {
return NULL;
}
esil->anal = anal;

RAnalEsilDFG *edf = dfg ? dfg : r_anal_esil_dfg_new (anal->reg);
if (!edf) {
r_anal_esil_free (esil);
return NULL;
}

r_anal_esil_set_op (esil, "=", edf_consume_2_set_reg, 0, 2, R_ANAL_ESIL_OP_TYPE_REG_WRITE);
r_anal_esil_set_op (esil, ":=", edf_eq_weak, 0, 2, R_ANAL_ESIL_OP_TYPE_REG_WRITE);
r_anal_esil_set_op (esil, "$z", edf_zf, 1, 0, R_ANAL_ESIL_OP_TYPE_UNKNOWN);
r_anal_esil_set_op (esil, "$p", edf_pf, 1, 0, R_ANAL_ESIL_OP_TYPE_UNKNOWN);
r_anal_esil_set_op (esil, "$c", edf_cf, 1, 1, R_ANAL_ESIL_OP_TYPE_UNKNOWN);
r_anal_esil_set_op (esil, "$b", edf_bf, 1, 1, R_ANAL_ESIL_OP_TYPE_UNKNOWN);
r_anal_esil_set_op (esil, "^=", edf_consume_2_use_set_reg, 0, 2, R_ANAL_ESIL_OP_TYPE_MATH | R_ANAL_ESIL_OP_TYPE_REG_WRITE);
r_anal_esil_set_op (esil, "-=", edf_consume_2_use_set_reg, 0, 2, R_ANAL_ESIL_OP_TYPE_MATH | R_ANAL_ESIL_OP_TYPE_REG_WRITE);
r_anal_esil_set_op (esil, "+=", edf_consume_2_use_set_reg, 0, 2, R_ANAL_ESIL_OP_TYPE_MATH | R_ANAL_ESIL_OP_TYPE_REG_WRITE);
r_anal_esil_set_op (esil, "*=", edf_consume_2_use_set_reg, 0, 2, R_ANAL_ESIL_OP_TYPE_MATH | R_ANAL_ESIL_OP_TYPE_REG_WRITE);
r_anal_esil_set_op (esil, "/=", edf_consume_2_use_set_reg, 0, 2, R_ANAL_ESIL_OP_TYPE_MATH | R_ANAL_ESIL_OP_TYPE_REG_WRITE);
r_anal_esil_set_op (esil, "&=", edf_consume_2_use_set_reg, 0, 2, R_ANAL_ESIL_OP_TYPE_MATH | R_ANAL_ESIL_OP_TYPE_REG_WRITE);
r_anal_esil_set_op (esil, "|=", edf_consume_2_use_set_reg, 0, 2, R_ANAL_ESIL_OP_TYPE_MATH | R_ANAL_ESIL_OP_TYPE_REG_WRITE);
r_anal_esil_set_op (esil, "^=", edf_consume_2_use_set_reg, 0, 2, R_ANAL_ESIL_OP_TYPE_MATH | R_ANAL_ESIL_OP_TYPE_REG_WRITE);
r_anal_esil_set_op (esil, "+", edf_consume_2_push_1, 1, 2, R_ANAL_ESIL_OP_TYPE_MATH);
r_anal_esil_set_op (esil, "-", edf_consume_2_push_1, 1, 2, R_ANAL_ESIL_OP_TYPE_MATH);
r_anal_esil_set_op (esil, "&", edf_consume_2_push_1, 1, 2, R_ANAL_ESIL_OP_TYPE_MATH);
r_anal_esil_set_op (esil, "|", edf_consume_2_push_1, 1, 2, R_ANAL_ESIL_OP_TYPE_MATH);
r_anal_esil_set_op (esil, "^", edf_consume_2_push_1, 1, 2, R_ANAL_ESIL_OP_TYPE_MATH);
r_anal_esil_set_op (esil, "%", edf_consume_2_push_1, 1, 2, R_ANAL_ESIL_OP_TYPE_MATH);
r_anal_esil_set_op (esil, "*", edf_consume_2_push_1, 1, 2, R_ANAL_ESIL_OP_TYPE_MATH);
r_anal_esil_set_op (esil, "/", edf_consume_2_push_1, 1, 2, R_ANAL_ESIL_OP_TYPE_MATH);
r_anal_esil_set_op (esil, ">>", edf_consume_2_push_1, 1, 2, R_ANAL_ESIL_OP_TYPE_MATH);
r_anal_esil_set_op (esil, "<<", edf_consume_2_push_1, 1, 2, R_ANAL_ESIL_OP_TYPE_MATH);
r_anal_esil_set_op (esil, ">>>", edf_consume_2_push_1, 1, 2, R_ANAL_ESIL_OP_TYPE_MATH);
r_anal_esil_set_op (esil, ">>>", edf_consume_2_push_1, 1, 2, R_ANAL_ESIL_OP_TYPE_MATH);
r_anal_esil_set_op (esil, "!", edf_consume_1_push_1, 1, 1, R_ANAL_ESIL_OP_TYPE_MATH);
r_anal_esil_set_op (esil, "[1]", edf_consume_1_push_1, 1, 1, R_ANAL_ESIL_OP_TYPE_MEM_READ);
r_anal_esil_set_op (esil, "[2]", edf_consume_1_push_1, 1, 1, R_ANAL_ESIL_OP_TYPE_MEM_READ);
r_anal_esil_set_op (esil, "[4]", edf_consume_1_push_1, 1, 1, R_ANAL_ESIL_OP_TYPE_MEM_READ);
r_anal_esil_set_op (esil, "[8]", edf_consume_1_push_1, 1, 1, R_ANAL_ESIL_OP_TYPE_MEM_READ);
r_anal_esil_set_op (esil, "[16]", edf_consume_1_push_1, 1, 1, R_ANAL_ESIL_OP_TYPE_MEM_READ);
r_anal_esil_set_op (esil, "=[1]", edf_consume_2_set_mem, 0, 2, R_ANAL_ESIL_OP_TYPE_MEM_WRITE);
r_anal_esil_set_op (esil, "=[2]", edf_consume_2_set_mem, 0, 2, R_ANAL_ESIL_OP_TYPE_MEM_WRITE);
r_anal_esil_set_op (esil, "=[4]", edf_consume_2_set_mem, 0, 2, R_ANAL_ESIL_OP_TYPE_MEM_WRITE);
r_anal_esil_set_op (esil, "=[8]", edf_consume_2_set_mem, 0, 2, R_ANAL_ESIL_OP_TYPE_MEM_WRITE);

esil->user = edf;

r_anal_esil_parse (esil, expr);
r_anal_esil_free (esil);
return edf;
}

static int _dfg_node_filter_insert_cmp(void *incoming, void *in, void *user) {
RAnalEsilDFGNode *incoming_node = (RAnalEsilDFGNode *)incoming;
RAnalEsilDFGNode *in_node = (RAnalEsilDFGNode *)in;
return incoming_node->idx - in_node->idx;
}

static void _dfg_rev_dfs_cb(RGraphNode *n, RGraphVisitor *vi) {
RAnalEsilDFGNode *node = (RAnalEsilDFGNode *)n->data;
RAnalEsilDFGFilter *filter = (RAnalEsilDFGFilter *)vi->data;
switch (node->type) {
case R_ANAL_ESIL_DFG_BLOCK_CONST:
case R_ANAL_ESIL_DFG_BLOCK_VAR:
case R_ANAL_ESIL_DFG_BLOCK_PTR:
break;
case R_ANAL_ESIL_DFG_BLOCK_GENERATIVE:
r_rbtree_cont_insert (filter->tree, node, _dfg_node_filter_insert_cmp, NULL);
break;
case R_ANAL_ESIL_DFG_BLOCK_RESULT: 
{
RGraphNode *previous = (RGraphNode *)r_list_get_top (n->in_nodes);
if (previous) {
sdb_ptr_set (filter->results, r_strbuf_get (node->content), previous, 0);
}
} break;
}
}

static char *condrets_strtok(char *str, const char tok) {
if (!str) {
return NULL;
}
ut32 i = 0;
while (1 == 1) {
if (!str[i]) {
break;
}
if (str[i] == tok) {
str[i] = '\0';
return &str[i + 1];
}
i++;
}
return NULL;
}

static RStrBuf *get_resolved_expr(RAnalEsilDFGFilter *filter, RAnalEsilDFGNode *node) {
char *expr = strdup (r_strbuf_get (node->content));
RStrBuf *res = r_strbuf_new ("");
if (!expr) { 
return res;
}
char *p, *q;

for (p = expr; (q = condrets_strtok (p, ',')); p = q) {
RGraphNode *gn = sdb_ptr_get (filter->results, p, 0);
if (!gn) {
r_strbuf_appendf (res, ",%s,", p);
} else {
RStrBuf *r = get_resolved_expr (filter, (RAnalEsilDFGNode *)gn->data);
r_strbuf_appendf (res, ",%s,", r_strbuf_get (r));
r_strbuf_free (r);
}
}
r_strbuf_appendf (res, "%s", p);
free (expr);
return res;
}

R_API RStrBuf *r_anal_esil_dfg_filter(RAnalEsilDFG *dfg, const char *reg) {
if (!dfg || !reg) {
return NULL;
}
RGraphNode *resolve_me = _edf_reg_get (dfg, reg);
if (!resolve_me) {
return NULL;
}


RAnalEsilDFGFilter filter = { dfg, r_rbtree_cont_new (), sdb_new0 () };
RStrBuf *filtered = r_strbuf_new ("");
RGraphVisitor vi = { _dfg_rev_dfs_cb, NULL, NULL, NULL, NULL, &filter };


r_graph_dfs_node_reverse (dfg->flow, resolve_me, &vi);

RBIter ator;
RAnalEsilDFGNode *node;
r_rbtree_cont_foreach (filter.tree, ator, node) {

RStrBuf *resolved = get_resolved_expr (&filter, node);
r_strbuf_append (filtered, r_strbuf_get (resolved));
r_strbuf_free (resolved);
}
{
char *sanitized = r_str_replace (r_str_replace (strdup (r_strbuf_get (filtered)), ",,", ",", 1), ",,", ",", 1);
r_strbuf_set (filtered, (sanitized[0] == ',') ? &sanitized[1] : sanitized);
free (sanitized);
}
r_rbtree_cont_free (filter.tree);
sdb_free (filter.results);
return filtered;
}

R_API RStrBuf *r_anal_esil_dfg_filter_expr(RAnal *anal, const char *expr, const char *reg) {
if (!reg) {
return NULL;
}
RAnalEsilDFG *dfg = r_anal_esil_dfg_expr (anal, NULL, expr);
if (!dfg) {
return NULL;
}
RStrBuf *filtered = r_anal_esil_dfg_filter (dfg, reg);
r_anal_esil_dfg_free (dfg);
return filtered;
}
