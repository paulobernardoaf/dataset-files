#include <stdio.h>
#include <string.h>
#include <r_util.h>
#include <r_lib.h>
#include <stdlib.h>
#include <r_cons.h>
#if !defined(HAS_MAIN)
#define HAS_MAIN 0
#endif
#define IFDBG if(0)
struct Type {
const char *code;
const char *name;
};
static struct Type types[] = {
{ "Sb", "Bool" },
{ "SS", "String" },
{ "FS", "String" },
{ "GV", "mutableAddressor" }, 
{ "Ss", "generic" }, 
{ "S_", "Generic" }, 
{ "TF", "GenericSpec" }, 
{ "Ts", "String" }, 
{ "Sa", "Array" },
{ "Si", "Swift.Int" },
{ "Sf", "Float" },
{ "Sb", "Bool" },
{ "Su", "UInt" },
{ "SQ", "ImplicitlyUnwrappedOptional" },
{ "Sc", "UnicodeScalar" },
{ "Sd", "Double" },
{ "SS", "String" }, 
{ "Bi1", "Builtin.Int1" },
{ "Bp", "Builtin.RawPointer" },
{ "Bw", "Builtin.Word" }, 
{ NULL, NULL }
};
static struct Type metas [] = {
{ "FC", "ClassFunc" },
{ "S0_FT", "?" },
{ "RxC", ".." },
{ "S0", "self" },
{ "U__FQ_T_", "<A>(A)" },
{ "ToFC", "@objc class func" },
{ "ToF", "@objc func" },
{ NULL, NULL }
};
static struct Type flags [] = {
{ "s", "setter" },
{ "g", "getter" },
{ "m", "method" }, 
{ "d", "destructor" },
{ "D", "deallocator" },
{ "c", "constructor" },
{ "C", "allocator" },
{ NULL , NULL}
};
static const char *getnum(const char* n, int *num) {
if (num && *n) {
*num = atoi (n);
}
while (*n && *n>='0' && *n <='9') {
n++;
}
return n;
}
static const char *numpos(const char* n) {
while (*n && (*n < '0' || *n > '9')) {
n++;
}
return n;
}
static const char *getstring(const char *s, int len) {
static char buf[256] = {0};
if (len < 0 || len > sizeof (buf) - 2) {
return "";
}
strncpy (buf, s, len);
buf[len] = 0;
return buf;
}
static const char *resolve(struct Type *t, const char *foo, const char **bar) {
if (!t || !foo || !*foo) {
return NULL;
}
for (; t[0].code; t++) {
int len = strlen (t[0].code);
if (!strncmp (foo, t[0].code, len)) {
if (bar) {
*bar = t[0].name;
}
return foo + len;
}
}
return NULL;
}
static int have_swift_demangle = -1;
static char *swift_demangle_cmd(const char *s) {
static char *swift_demangle = NULL;
if (have_swift_demangle == -1) {
if (!swift_demangle) {
have_swift_demangle = 0;
swift_demangle = r_file_path ("swift-demangle");
if (!swift_demangle || !strcmp (swift_demangle, "swift-demangle")) {
char *xcrun = r_file_path ("xcrun");
if (xcrun) {
if (strcmp (xcrun, "xcrun")) {
free (swift_demangle);
swift_demangle = r_str_newf ("%s swift-demangle", xcrun);
have_swift_demangle = 1;
}
free (xcrun);
}
}
}
}
if (swift_demangle) {
if (strchr (s, '\'') || strchr (s, '\\')) {
return NULL;
}
char *res = r_sys_cmd_strf ("%s -compact '%s'",
swift_demangle, s);
if (res && !*res) {
free (res);
res = NULL;
}
r_str_trim (res);
return res;
}
return NULL;
}
static char *swift_demangle_lib(const char *s) {
#if __UNIX__
static bool haveSwiftCore = false;
static char *(*swift_demangle)(const char *sym, int symlen, void *out, int *outlen, int flags) = NULL;
if (!haveSwiftCore) {
void *lib = r_lib_dl_open ("/usr/lib/swift/libswiftCore.dylib");
if (lib) {
swift_demangle = r_lib_dl_sym (lib, "swift_demangle");
}
haveSwiftCore = true;
}
if (swift_demangle) {
return swift_demangle (s, strlen (s), NULL, NULL, 0);
}
#endif
return NULL;
}
R_API char *r_bin_demangle_swift(const char *s, bool syscmd) {
#define STRCAT_BOUNDS(x) if (((x) + 2 + strlen (out)) > sizeof (out)) break;
char out[1024];
int i, len, is_generic = 0;
int is_first = 1;
int is_last = 0;
int retmode = 0;
if (!strncmp (s, "imp.", 4)) {
s = s + 4;
}
if (!strncmp (s, "reloc.", 6)) {
s = s + 6;
}
if (*s != 'T' && strncmp (s, "_T", 2) && strncmp (s, "__T", 3)) {
if (strncmp (s, "$s", 2)) {
return NULL;
}
}
if (!strncmp (s, "__", 2)) {
s = s + 2;
}
#if 0
const char *element[] = {
"module", "class", "method", NULL
};
#endif
char *res = swift_demangle_lib (s);
if (res) {
return res;
}
const char *attr = NULL;
const char *attr2 = NULL;
const char *q, *p = s;
const char *q_end = p + strlen (p);
const char *q_start = p;
if (strchr (s, '\'') || strchr (s, ' ')) {
return NULL;
}
if (syscmd) {
res = swift_demangle_cmd (s);
if (res) {
return res;
}
}
out[0] = 0;
const char *tail = NULL;
if (p[0]) {
switch (p[1]) {
case 'W':
switch (p[2]) {
case 'a':
tail = "..protocol";
break;
}
break;
case 'F':
switch (p[2]) {
case 'e':
tail = "..extension";
p += 2;
break;
}
break;
case 'M':
switch (p[2]) {
case 'a':
tail = "..accessor.metadata";
break;
case 'e':
tail = "..override";
break;
case 'm':
tail = "..metaclass";
break;
case 'L':
tail = "..lazy.metadata";
break;
default:
tail = "..metadata";
break;
}
break;
case 'I': 
break;
}
}
if (tail) {
if (*p) {
p++;
}
} else {
if (*p && p[1]) {
p += 2;
}
}
q = getnum (p, NULL);
if (IS_DIGIT (*p) || *p == 'v' || *p == 'I' || *p == 'o' || *p == 'T' || *p == 'V' || *p == 'M' || *p == 'C' || *p == 'F' || *p == 'W') {
if (!strncmp (p+1, "SS", 2)) {
strcat (out, "Swift.String.init (");
p += 3;
}
if (!strncmp (p, "vdv", 3)) {
tail = "..field";
p += 3;
}
if (!strncmp (p, "oFC", 3)) {
tail = "..init.witnesstable";
p += 4;
}
#if 0
if (!strncmp (p+1, "C", 2)) {
strcat (out, "class ");
p += 3;
}
#endif
q = getnum (q, &len);
q = numpos (p);
for (i = 0, len = 1; len && q < q_end; q += len, i++) {
if (*q == 'P') {
q++;
}
q = getnum (q, &len);
if (!len) {
break;
}
const char *str = getstring (q, len);
if (len == 2 && !strcmp (str, "ee")) {
strcat (out, "Swift");
} else {
#if 0
printf ("%s %d %s\n", element[i],
len, getstring (q, len));
#endif
if (i && *out) {
strcat (out, ".");
}
STRCAT_BOUNDS (len);
len = R_MIN (len, strlen (q));
strcat (out, getstring (q, len));
}
}
if (q > q_end) {
return 0;
}
p = resolve (flags, q, &attr);
if (!p && ((*q == 'U') || (*q == 'R'))) {
p = resolve (metas, q, &attr);
if (attr && *q == 'R') {
attr = NULL;
q += 3;
} else {
}
}
if (attr) {
int len = 0;
const char *name;
resolve (types, q, &attr);
q = getnum (q + 1, &len);
name = getstring (q, len);
#if 0
if (name && *name) {
printf ("Field Name: %s\n", name);
}
#endif
if (len < strlen (q)) {
resolve (types, q + len, &attr2);
} else {
resolve (types, q, &attr2);
}
do {
if (name && *name) {
strcat (out, ".");
STRCAT_BOUNDS (strlen (name));
strcat (out, name);
}
if (attr && *attr) {
strcat (out, ".");
STRCAT_BOUNDS (strlen (attr));
strcat (out, attr);
}
if (attr2 && *attr2) {
strcat (out, "__");
STRCAT_BOUNDS (strlen (attr2));
strcat (out, attr2);
}
} while (0);
if (*q == '_') {
strcat (out, " -> ()");
}
} else {
for (i = 0; q && q < q_end && q >= q_start; i++) {
if (*q == 'f') {
q++;
}
switch (*q) {
case 's':
{
int n = 0;
const char *Q = getnum (q + 1, &n);
const char *res = getstring (Q, n);
if (res) {
strcat (out, res);
}
q = Q + n + 1;
continue;
}
break;
case 'u':
if (!strncmp (q, "uRxs", 4)) {
strcat (out, "..");
int n = 0 ;
const char *Q = getnum (q + 4, &n);
strcat (out, getstring (Q, n));
q = Q + n + 1;
continue;
}
break;
case 'S': 
if (q[1] == '1') {
q++;
}
switch (q[1]) {
case '0':
strcat (out, " (self) -> ()");
if (attr) {
strcat (out, attr);
}
q = p = q + 1;
attr = "";
break;
case 'S':
strcat (out, "__String");
break;
case '_':
if (q[0] && q[1] && q[2]) {
strcat (out, "..");
int n = 0;
const char *Q = getnum (q + 2, &n);
strcat (out, getstring (Q, n));
q = Q + n + 1;
continue;
}
break;
}
break;
case 'B':
case 'T':
case 'I':
p = resolve (types, q + 0, &attr); 
if (p && *p && IS_DIGIT (p[1])) {
p--;
}
break;
case 'F':
strcat (out, " ()");
p = resolve (types, (strlen (q) > 2)? q + 3: "", &attr); 
break;
case 'G':
q += 2;
if (!strncmp (q, "_V", 2)) {
q += 2;
}
p = resolve (types, q, &attr); 
break;
case 'V':
p = resolve (types, q + 1, &attr); 
break;
case '_':
p = resolve (types, q + 1, &attr); 
break;
default:
p = resolve (types, q, &attr); 
}
if (p) {
q = getnum (p, &len);
if (attr && !strcmp (attr, "generic")) {
is_generic = 1;
}
if (!len) {
if (retmode) {
if (q + 1 > q_end) {
if (attr) {
strcat (out, " -> ");
STRCAT_BOUNDS (strlen (attr));
strcat (out, attr);
}
break;
}
p = resolve (types, *q? q + 1: q, &attr); 
if (attr) {
strcat (out, " -> ");
STRCAT_BOUNDS (strlen (attr));
strcat (out, attr);
}
break;
}
retmode = 1;
len++;
}
if (len < 0 || len > 256) {
break;
}
if (len <= (q_end - q) && q[len]) {
const char *s = getstring (q, len);
if (s && *s) {
if (is_first) { 
strcat (out, is_generic?"<":"(");
is_first = 0;
}
is_last = q[len];
if (attr) {
STRCAT_BOUNDS (strlen (attr));
strcat (out, attr);
strcat (out, " ");
}
STRCAT_BOUNDS (strlen (s));
strcat (out, s);
if (is_last) {
strcat (out, is_generic?">":")");
is_first = (*s != '_');
if (is_generic && !is_first) {
break;
}
} else {
strcat (out, ", ");
}
} else {
if (attr) {
strcat (out, " -> ");
STRCAT_BOUNDS (strlen (attr));
strcat (out, attr);
}
}
} else {
if (attr) {
strcat (out, " -> ");
STRCAT_BOUNDS (strlen (attr));
strcat (out, attr);
}
}
q += len;
p = q;
} else {
if (q && *q) {
q++;
} else {
break;
}
char *n = strstr (q, "__");
if (n) {
q = n + 1;
} else {
n = strchr (q, '_');
if (n) {
q = n + 1;
} else {
break;
}
}
}
}
}
} else {
}
if (*out) {
if (tail) {
strcat (out, tail);
}
#if 1
char *p, *outstr = strdup (out);
p = outstr;
for (;;) {
p = strstr (p, ")(");
if (p) {
p[0] = '_';
p[1] = '_';
p+=2;
} else {
break;
}
}
return outstr;
#endif
}
return NULL;
}
#define MAXMAIN
#if HAS_MAIN
typedef struct {
const char *sym;
const char *dem;
} Test;
Test swift_tests[] = {
{
"_TWPu0_Rq_Ss14CollectionType_GVSs17MapCollectionViewq_q0__Ss23_CollectionDefaultsTypeSs_8",
"<generic _CollectionDefaultsType>"
},
{
"_TWPurGVSs15CollectionOfOneq__Ss14CollectionTypeSs_248",
"CollectionOfOne<generic CollectionType><generic S>"
},
{
"_TFSSCfT21_builtinStringLiteralBp8byteSizeBw7isASCIIBi1__SS"
,"Swift.String.init (_builtinStringLiteral(Builtin.RawPointer byteSize__Builtin.Word isASCII__Builtin.Int1 _) -> String"
},{
"_TFC10swifthello5Hellog5WorldSS" 
,"swifthello.Hello.World.getter__String"
},{
"_TFC10swifthello5Hellom5WorldSS" 
,"swifthello.Hello.World.method__String"
},{
"_TFC10swifthello5Hellos5WorldSS" 
,"swifthello.Hello.World.setter__String"
},{
"_TFSSCfMSSFT21_builtinStringLiteralBp8byteSizeBw7isASCIIBi1__SS"
,"Swift.String.init (_builtinStringLiteral(Builtin.RawPointer byteSize__Builtin.Word isASCII__Builtin.Int1 _) -> String"
},{
"_TF10swifthello3norFT_Si"
,"swifthello.nor () -> Swift.Int"
},{
"_TFSs7printlnU__FQ_T_"
,"println.<A>(A) -> ()"
},{
"_TFSsa6C_ARGVGVSs20UnsafeMutablePointerGS_VSs4Int8__"
,"C_ARGV<generic UnsafeMutablePointer><generic Int8>"
},{
"_TFC10FlappyBird9GameScene10resetScenefS0_FT_T_"
,"FlappyBird.GameScene.resetScene (self) -> (__ _) ()" 
},{
"__TFC4main8BarClass8sayHellofT_T_"
,"main.BarClass.sayHello"
},{
"__TFC4main4TostCfT_S0_"
,"main.Tost.allocator"
},{
"__TFC4main4TostD"
,"main.Tost.deallocator"
},{
"__TFC4main4TostcfT_S0_"
,"main.Tost.constructor"
},{
"__TF4main4moinFT_Si"
,"main.moin () -> Swift.Int"
},{
"__TFC4main4Tostg3msgSS"
,"main.Tost.msg.getter__String"
},{
"__TMC4main4Tost"
,"main.Tost..metadata"
},{
"__TMLC4main4Tost"
,"main.Tost..lazy.metadata"
},{
"__TMaC4main4Tost"
,"main.Tost..accessor.metadata"
},{
"__TMmC4main4Tost"
,"main.Tost..metaclass"
},{
"__TFV4main7Balanceg5widthSd"
,"main.Balance.width.getter__Double"
},{
"__TWoFC4main4TostCfT_S0_"
,"Tost.allocator..init.witnesstable"
},{
"__TMBi256_"
,"_..metadata"
},{
"__TWvdvC4main4Tost3msgSS"
,"main.Tost.msg__String..field"
},{
"__TIFC10Moscapsule10MQTTClient11unsubscribeFTSS17requestCompletionGSqFTOS_10MosqResultSi_T___T_A0_"
,"Moscapsule.MQTTClient.unsubscribe ()"
},{
"__TWaC4main8FooClassS_9FoodClassS_"
,"main.FooClass..FoodClass..protocol"
},{
"__TFe4mainRxCS_8FooClassxS_9FoodClassrS1_8sayHellofT_T_"
,"main..FooClass..FoodClass..sayHello..extension"
},{
"_TTSg5P____TFs27_allocateUninitializedArrayurFBwTGSax_Bp_",
"P____(GenericSpec F)_allocateUninitializedArray -> Builtin.RawPointer" 
},{
"_TIFs5printFTGSaP__9separatorSS10terminatorSS_T_A0_",
"print (__Array P)", 
},{
"__TZFsoi2eeuRxs9EquatablerFTGSqx_GSqx__Sb",
"Swift..Equatable () -> Bool"
},{
NULL, NULL
}};
int main(int argc, char **argv) {
int rc = 0;
char *ret;
if (argc > 1) {
ret = r_bin_demangle_swift (argv[1], 0);
if (ret) {
printf ("%s\n", ret);
free (ret);
}
} else {
int i = 0;
for (i = 0; swift_tests[i].sym; i++) {
Test *test = &swift_tests[i];
printf ("[>>] %s\n", test->sym);
ret = r_bin_demangle_swift (test->sym, 0);
if (ret) {
if (test->dem && !strcmp (ret, test->dem)) {
printf (Color_GREEN"[OK]"Color_RESET" %s\n", ret);
} else {
printf (Color_RED"[XX]"Color_RESET" %s\n", ret);
printf (Color_YELLOW"[MUSTBE]"Color_RESET" %s\n", test->dem);
rc = 1;
}
free (ret);
} else {
printf (Color_RED"[XX]"Color_RESET" \"(null)\"\n");
printf (Color_YELLOW"[MUSTBE]"Color_RESET" %s\n", test->dem);
rc = 1;
}
}
}
return rc;
}
#endif
