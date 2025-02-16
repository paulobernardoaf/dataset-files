

#include <r_anal.h>

#define DB anal->sdb_fcns


#define LABELS sdb_fmt ("fcn.%"PFMT64x".labels", fcn->addr)

#define ADDRLABEL(x,y) sdb_fmt ("0x%"PFMT64x"/%s", x,y)

#define LABEL(x) sdb_fmt ("fcn.%"PFMT64x".label.%s", fcn->addr, x)

#define ADDR(x) sdb_fmt ("fcn.%"PFMT64x".label.0x%"PFMT64x, fcn->addr,x)


R_API ut64 r_anal_fcn_label_get (RAnal *anal, RAnalFunction *fcn, const char *name) {
return (anal && fcn)? sdb_num_get (DB, LABEL (name), NULL): UT64_MAX;
}

R_API const char *r_anal_fcn_label_at (RAnal *anal, RAnalFunction *fcn, ut64 addr) {
return (anal && fcn)? sdb_const_get (DB, ADDR (addr), NULL): NULL;
}

R_API int r_anal_fcn_label_set (RAnal *anal, RAnalFunction *fcn, const char *name, ut64 addr) {
if (!anal || !fcn) {
return false;
}
if (sdb_add (DB, ADDR (addr), name, 0)) {
if (sdb_num_add (DB, LABEL (name), addr, 0)) {
sdb_array_add (DB, LABELS, ADDRLABEL (addr, name), 0);
return true;
} else {
sdb_unset (DB, ADDR (addr), 0);
}
} else {
eprintf ("Cannot add\n");
}
return false;
}

R_API int r_anal_fcn_label_del (RAnal *anal, RAnalFunction *fcn, const char *name, ut64 addr) {
if (!anal || !fcn || !name) {
return false;
}
sdb_array_remove (DB, LABELS, ADDRLABEL (addr, name), 0);
sdb_unset (DB, LABEL (name), 0);
sdb_unset (DB, ADDR (addr), 0);
return true;
}

R_API int r_anal_fcn_labels(RAnal *anal, RAnalFunction *fcn, int rad) {
if (!anal) {
return 0;
}
if (fcn) {
char *cur, *token;
char *str = sdb_get (DB, LABELS, 0);
sdb_aforeach (cur, str) {
struct {
ut64 addr;
char *name;
} loc;
token = strchr (cur, '/');
if (!token) {
break;
}
*token = ',';
sdb_fmt_tobin (cur, "qz", &loc);
switch (rad) {
case '*':
case 1:
anal->cb_printf ("f.%s@0x%08"PFMT64x"\n",
loc.name, loc.addr);
break;
case 'j':
eprintf ("TODO\n");
break;
default:
anal->cb_printf ("0x%08"PFMT64x" %s [%s + %"PFMT64d"]\n",
loc.addr,
loc.name, fcn->name,
loc.addr - fcn->addr, loc.addr);
}
*token = '/';
sdb_fmt_free (&loc, "qz");
sdb_aforeach_next (cur);
}
free (str);
} else {
RAnalFunction *f;
RListIter *iter;
r_list_foreach (anal->fcns, iter, f) {
r_anal_fcn_labels (anal, f, rad);
}
}
return true;
}
