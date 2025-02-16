

#include <r_anal.h>
#include <r_parse.h>
#include <r_util.h>
#include <r_list.h>

#define JMPTBL_MAXSZ 512

static void apply_case(RAnal *anal, RAnalBlock *block, ut64 switch_addr, ut64 offset_sz, ut64 case_addr, ut64 id, ut64 case_addr_loc) {

r_meta_set_data_at (anal, case_addr_loc, offset_sz);
r_anal_hint_set_immbase (anal, case_addr_loc, 10);
r_anal_xrefs_set (anal, switch_addr, case_addr, R_ANAL_REF_TYPE_CODE);
if (block) {
r_anal_block_add_switch_case (block, switch_addr, case_addr);
}
if (anal->flb.set) {
char flagname[0x30];
snprintf (flagname, sizeof (flagname), "case.0x%"PFMT64x ".%d", (ut64)switch_addr, (int)id);
anal->flb.set (anal->flb.f, flagname, case_addr, 1);
}
}

static void apply_switch(RAnal *anal, ut64 switch_addr, ut64 jmptbl_addr, ut64 cases_count, ut64 default_case_addr) {
char tmp[0x30];
snprintf (tmp, sizeof (tmp), "switch table (%"PFMT64u" cases) at 0x%"PFMT64x, cases_count, jmptbl_addr);
r_meta_set_string (anal, R_META_TYPE_COMMENT, switch_addr, tmp);
if (anal->flb.set) {
snprintf (tmp, sizeof (tmp), "switch.0x%08"PFMT64x, switch_addr);
anal->flb.set (anal->flb.f, tmp, switch_addr, 1);
if (default_case_addr != UT64_MAX) {
snprintf (tmp, sizeof (tmp), "case.default.0x%"PFMT64x, switch_addr);
anal->flb.set (anal->flb.f, tmp, default_case_addr, 1);
}
}
}


R_API bool r_anal_jmptbl(RAnal *anal, RAnalFunction *fcn, RAnalBlock *block, ut64 jmpaddr, ut64 table, ut64 tablesize, ut64 default_addr) {
const int depth = 50;
return try_walkthrough_jmptbl (anal, fcn, block, depth, jmpaddr, table, table, tablesize, tablesize, default_addr, false);
}

R_API bool try_walkthrough_jmptbl(RAnal *anal, RAnalFunction *fcn, RAnalBlock *block, int depth, ut64 ip, ut64 jmptbl_loc, ut64 jmptbl_off, ut64 sz, int jmptbl_size, ut64 default_case, bool ret0) {
bool ret = ret0;

if (jmptbl_size == 0) {
jmptbl_size = JMPTBL_MAXSZ;
}
if (jmptbl_loc == UT64_MAX) {
if (anal->verbose) {
eprintf ("Warning: Invalid JumpTable location 0x%08"PFMT64x"\n", jmptbl_loc);
}
return false;
}
if (jmptbl_size < 1 || jmptbl_size > ST32_MAX) {
if (anal->verbose) {
eprintf ("Warning: Invalid JumpTable size at 0x%08"PFMT64x"\n", ip);
}
return false;
}
ut64 jmpptr, offs;
ut8 *jmptbl = calloc (jmptbl_size, sz);
if (!jmptbl) {
return false;
}
bool is_arm = anal->cur->arch && !strncmp (anal->cur->arch, "arm", 3);

anal->iob.read_at (anal->iob.io, jmptbl_loc, jmptbl, jmptbl_size * sz);
for (offs = 0; offs + sz - 1 < jmptbl_size * sz; offs += sz) {
switch (sz) {
case 1:
jmpptr = (ut64)(ut8)r_read_le8 (jmptbl + offs);
break;
case 2:
jmpptr = (ut64)r_read_le16 (jmptbl + offs);
break;
case 4:
jmpptr = r_read_le32 (jmptbl + offs);
break;
case 8:
jmpptr = r_read_le64 (jmptbl + offs);
break; 
default:
jmpptr = r_read_le64 (jmptbl + offs);
break;
}




if (jmpptr == 0 || jmpptr == UT32_MAX || jmpptr == UT64_MAX) {
break;
}
if (sz == 2 && is_arm) {
jmpptr = ip + 4 + (jmpptr * 2); 
} else if (sz == 1 && is_arm) {
jmpptr = ip + 4 + (jmpptr * 2); 
} else if (!anal->iob.is_valid_offset (anal->iob.io, jmpptr, 0)) {
st32 jmpdelta = (st32)jmpptr;

jmpptr = jmptbl_off + jmpdelta;
if (!anal->iob.is_valid_offset (anal->iob.io, jmpptr, 0)) {
break;
}
}
if (anal->limit) {
if (jmpptr < anal->limit->from || jmpptr > anal->limit->to) {
break;
}
}
apply_case (anal, block, ip, sz, jmpptr, offs / sz, jmptbl_loc + offs);
(void)r_anal_fcn_bb (anal, fcn, jmpptr, depth - 1);
}

if (offs > 0) {
if (default_case == 0) {
default_case = UT64_MAX;
}
apply_switch (anal, ip, jmptbl_loc, offs / sz, default_case);
}

free (jmptbl);
return ret;
}


R_API bool try_get_delta_jmptbl_info(RAnal *anal, RAnalFunction *fcn, ut64 jmp_addr, ut64 lea_addr, ut64 *table_size, ut64 *default_case) {
bool isValid = false;
bool foundCmp = false;
int i;

RAnalOp tmp_aop = {0};
if (lea_addr > jmp_addr) {
return false;
}
int search_sz = jmp_addr - lea_addr;
ut8 *buf = malloc (search_sz);
if (!buf) {
return false;
}

anal->iob.read_at (anal->iob.io, lea_addr, (ut8 *)buf, search_sz);

for (i = 0; i + 8 < search_sz; i++) {
int len = r_anal_op (anal, &tmp_aop, lea_addr + i, buf + i, search_sz - i, R_ANAL_OP_MASK_BASIC);
if (len < 1) {
len = 1;
}

if (foundCmp) {
if (tmp_aop.type != R_ANAL_OP_TYPE_CJMP) {
continue;
}

*default_case = tmp_aop.jump == tmp_aop.jump + len ? tmp_aop.fail : tmp_aop.jump;
break;
}

ut32 type = tmp_aop.type & R_ANAL_OP_TYPE_MASK;
if (type != R_ANAL_OP_TYPE_CMP) {
continue;
}






if (tmp_aop.val == UT64_MAX && tmp_aop.refptr == 0) {
isValid = true;
*table_size = 0;
} else if (tmp_aop.refptr == 0) {
isValid = tmp_aop.val < 0x200;
*table_size = tmp_aop.val + 1;
} else {
isValid = tmp_aop.refptr < 0x200;
*table_size = tmp_aop.refptr + 1;
}

foundCmp = true;
}
free (buf);
return isValid;
}


R_API int walkthrough_arm_jmptbl_style(RAnal *anal, RAnalFunction *fcn, RAnalBlock *block, int depth, ut64 ip, ut64 jmptbl_loc, ut64 sz, ut64 jmptbl_size, ut64 default_case, int ret0) {



















ut64 offs, jmpptr;
int ret = ret0;

if (jmptbl_size == 0) {
jmptbl_size = JMPTBL_MAXSZ;
}

for (offs = 0; offs + sz - 1 < jmptbl_size * sz; offs += sz) {
jmpptr = jmptbl_loc + offs;
apply_case (anal, block, ip, sz, jmpptr, offs / sz, jmptbl_loc + offs);
(void)r_anal_fcn_bb (anal, fcn, jmpptr, depth - 1);
}

if (offs > 0) {
if (default_case == 0 || default_case == UT32_MAX) {
default_case = UT64_MAX;
}
apply_switch (anal, ip, jmptbl_loc, offs / sz, default_case);
}
return ret;
}

R_API bool try_get_jmptbl_info(RAnal *anal, RAnalFunction *fcn, ut64 addr, RAnalBlock *my_bb, ut64 *table_size, ut64 *default_case) {
bool isValid = false;
int i;
RListIter *iter;
RAnalBlock *tmp_bb, *prev_bb;
prev_bb = 0;
if (!fcn->bbs) {
return false;
}


RBinSection *s = anal->binb.get_vsect_at (anal->binb.bin, addr);
if (s && s->name[0]) {
bool in_plt = strstr (s->name, ".plt") != NULL;
if (!in_plt && strstr (s->name, "_stubs") != NULL) {

in_plt = true;
}
if (in_plt) {
return false;
}
}


r_list_foreach (fcn->bbs, iter, tmp_bb) {
if (tmp_bb->jump == my_bb->addr || tmp_bb->fail == my_bb->addr) {
prev_bb = tmp_bb;
break;
}
}

if (!prev_bb || !prev_bb->jump || !prev_bb->fail) {
if (anal->verbose) {
eprintf ("Warning: [anal.jmp.tbl] Missing predecesessor cjmp bb at 0x%08"PFMT64x"\n", addr);
}
return false;
}


*default_case = prev_bb->jump == my_bb->addr ? prev_bb->fail : prev_bb->jump;

RAnalOp tmp_aop = {0};
ut8 *bb_buf = calloc (1, prev_bb->size);
if (!bb_buf) {
return false;
}

anal->iob.read_at (anal->iob.io, prev_bb->addr, (ut8 *) bb_buf, prev_bb->size);
isValid = false;

RAnalHint *hint = r_anal_hint_get (anal, addr);
if (hint) {
ut64 val = hint->val;
r_anal_hint_free (hint);
if (val != UT64_MAX) {
*table_size = val;
return true;
}
}

for (i = 0; i < prev_bb->op_pos_size; i++) {
ut64 prev_pos = prev_bb->op_pos[i];
ut64 op_addr = prev_bb->addr + prev_pos;
if (prev_pos >= prev_bb->size) {
continue;
}
int buflen = prev_bb->size - prev_pos;
int len = r_anal_op (anal, &tmp_aop, op_addr,
bb_buf + prev_pos, buflen,
R_ANAL_OP_MASK_BASIC | R_ANAL_OP_MASK_HINT);
ut32 type = tmp_aop.type & R_ANAL_OP_TYPE_MASK;
if (len < 1 || type != R_ANAL_OP_TYPE_CMP) {
r_anal_op_fini (&tmp_aop);
continue;
}






if (tmp_aop.val == UT64_MAX && tmp_aop.refptr == 0) {
isValid = true;
*table_size = 0;
} else if (tmp_aop.refptr == 0 || tmp_aop.val != UT64_MAX) {
isValid = tmp_aop.val < 0x200;
*table_size = tmp_aop.val + 1;
} else {
isValid = tmp_aop.refptr < 0x200;
*table_size = tmp_aop.refptr + 1;
}
r_anal_op_fini (&tmp_aop);

break;
}
free (bb_buf);




return isValid;
}
