#include <r_core.h>
R_API RCoreItem *r_core_item_at (RCore *core, ut64 addr) {
RCoreItem *ci = R_NEW0 (RCoreItem);
ci->addr = addr;
RIOMap *map = r_io_map_get (core->io, addr);
if (map) {
ci->perm = map->perm;
if (map->perm & R_PERM_X) {
RAnalMetaItem *item = r_meta_find (core->anal, addr, R_META_TYPE_ANY, 0);
if (item) {
switch (item->type) {
case R_META_TYPE_DATA:
ci->type = "data";
ci->size = item->size;
ci->data = r_core_cmd_strf (core, "pdi 1 @e:asm.flags=0@e:asm.lines=0@e:scr.color=0@0x%08"PFMT64x, addr);
r_str_trim (ci->data);
break;
case R_META_TYPE_FORMAT:
ci->type = "format"; 
ci->size = item->size;
break;
case R_META_TYPE_STRING:
ci->type = "string";
ci->size = item->size;
break;
}
if (item->str) {
if (!ci->data) {
ci->data = strdup (item->str);
}
}
r_meta_item_free (item);
}
}
}
RAnalFunction *fcn = r_anal_get_fcn_in (core->anal, addr, 1);
if (fcn) {
ci->fcnname = strdup (fcn->name);
}
RBinObject *o = r_bin_cur_object (core->bin);
RBinSection *sec = r_bin_get_section_at (o, addr, core->io->va);
if (sec) {
ci->sectname = strdup (sec->name);
}
if (!ci->data) {
RAnalOp* op = r_core_anal_op (core, addr, R_ANAL_OP_MASK_ESIL | R_ANAL_OP_MASK_HINT);
if (op) {
if (!ci->data) {
if (op->mnemonic) {
ci->data = strdup (op->mnemonic);
} else {
ci->data = r_core_cmd_strf (core, "pi 1 @e:scr.color=0@0x%08"PFMT64x, addr);
r_str_trim (ci->data);
}
}
ci->size = op->size;
r_anal_op_free (op);
}
}
char *cmt = r_core_cmd_strf (core, "CC.@0x%08"PFMT64x, addr);
if (cmt) {
if (*cmt) {
ci->comment = strdup (cmt);
r_str_trim (ci->comment);
}
free (cmt);
}
if (!ci->type) {
ci->type = "code";
}
ci->next = ci->addr + ci->size;
char *prev = r_core_cmd_strf (core, "pd -1@e:asm.lines=0~[0]");
r_str_trim (prev);
ci->prev = r_num_get (NULL, prev);
free (prev);
return ci;
}
R_API void r_core_item_free (RCoreItem *ci) {
free (ci->data);
free (ci);
}
