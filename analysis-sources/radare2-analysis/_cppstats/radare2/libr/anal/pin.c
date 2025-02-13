#include <r_anal.h>
typedef void (*RAnalEsilPin)(RAnal *a);
#if 0
static void pin_strlen(RAnal *a) {
eprintf ("esilpin: strlen\n");
}
static void pin_write(RAnal *a) {
eprintf ("esilpin: write\n");
}
#endif
#define DB a->sdb_pins
R_API void r_anal_pin_init(RAnal *a) {
sdb_free (DB);
DB = sdb_new0();
}
R_API void r_anal_pin_fini(RAnal *a) {
sdb_free (DB);
}
R_API void r_anal_pin (RAnal *a, ut64 addr, const char *name) {
char buf[64];
const char *key = sdb_itoa (addr, buf, 16);
sdb_set (DB, key, name, 0);
}
R_API void r_anal_pin_unset (RAnal *a, ut64 addr) {
char buf[64];
const char *key = sdb_itoa (addr, buf, 16);
sdb_unset (DB, key, 0);
}
R_API const char *r_anal_pin_call(RAnal *a, ut64 addr) {
char buf[64];
const char *key = sdb_itoa (addr, buf, 16);
if (key) {
return sdb_const_get (DB, key, NULL);
#if 0
const char *name;
if (name) {
RAnalEsilPin fcnptr = (RAnalEsilPin)
sdb_ptr_get (DB, name, NULL);
if (fcnptr) {
fcnptr (a);
return true;
}
}
#endif
}
return NULL;
}
static int cb_list(void *user, const char *k, const char *v) {
RAnal *a = (RAnal*)user;
if (*k == '0') {
a->cb_printf ("%s = %s\n", k, v);
} else {
a->cb_printf ("PIN %s\n", k);
}
return true;
}
R_API void r_anal_pin_list(RAnal *a) {
sdb_foreach (DB, cb_list, a);
}
