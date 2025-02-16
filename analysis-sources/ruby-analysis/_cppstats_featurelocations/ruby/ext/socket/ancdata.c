#include "rubysocket.h"

#include <time.h>

int rsock_cmsg_cloexec_state = -1; 
static VALUE sym_wait_readable, sym_wait_writable;

#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
static VALUE rb_cAncillaryData;

static VALUE
constant_to_sym(int constant, ID (*intern_const)(int))
{
ID name = intern_const(constant);
if (name) {
return ID2SYM(name);
}

return INT2NUM(constant);
}

static VALUE
ip_cmsg_type_to_sym(int level, int cmsg_type)
{
switch (level) {
case SOL_SOCKET:
return constant_to_sym(cmsg_type, rsock_intern_scm_optname);
case IPPROTO_IP:
return constant_to_sym(cmsg_type, rsock_intern_ip_optname);
#if defined(IPPROTO_IPV6)
case IPPROTO_IPV6:
return constant_to_sym(cmsg_type, rsock_intern_ipv6_optname);
#endif
case IPPROTO_TCP:
return constant_to_sym(cmsg_type, rsock_intern_tcp_optname);
case IPPROTO_UDP:
return constant_to_sym(cmsg_type, rsock_intern_udp_optname);
default:
return INT2NUM(cmsg_type);
}
}

































static VALUE
ancillary_initialize(VALUE self, VALUE vfamily, VALUE vlevel, VALUE vtype, VALUE data)
{
int family = rsock_family_arg(vfamily);
int level = rsock_level_arg(family, vlevel);
int type = rsock_cmsg_type_arg(family, level, vtype);
StringValue(data);
rb_ivar_set(self, rb_intern("family"), INT2NUM(family));
rb_ivar_set(self, rb_intern("level"), INT2NUM(level));
rb_ivar_set(self, rb_intern("type"), INT2NUM(type));
rb_ivar_set(self, rb_intern("data"), data);
return self;
}

static VALUE
ancdata_new(int family, int level, int type, VALUE data)
{
NEWOBJ_OF(obj, struct RObject, rb_cAncillaryData, T_OBJECT);
StringValue(data);
ancillary_initialize((VALUE)obj, INT2NUM(family), INT2NUM(level), INT2NUM(type), data);
return (VALUE)obj;
}

static int
ancillary_family(VALUE self)
{
VALUE v = rb_attr_get(self, rb_intern("family"));
return NUM2INT(v);
}










static VALUE
ancillary_family_m(VALUE self)
{
return INT2NUM(ancillary_family(self));
}

static int
ancillary_level(VALUE self)
{
VALUE v = rb_attr_get(self, rb_intern("level"));
return NUM2INT(v);
}










static VALUE
ancillary_level_m(VALUE self)
{
return INT2NUM(ancillary_level(self));
}

static int
ancillary_type(VALUE self)
{
VALUE v = rb_attr_get(self, rb_intern("type"));
return NUM2INT(v);
}










static VALUE
ancillary_type_m(VALUE self)
{
return INT2NUM(ancillary_type(self));
}










static VALUE
ancillary_data(VALUE self)
{
VALUE v = rb_attr_get(self, rb_intern("data"));
StringValue(v);
return v;
}

#if defined(SCM_RIGHTS)









static VALUE
ancillary_s_unix_rights(int argc, VALUE *argv, VALUE klass)
{
VALUE result, str, ary;
int i;

ary = rb_ary_new();

for (i = 0 ; i < argc; i++) {
VALUE obj = argv[i];
if (!RB_TYPE_P(obj, T_FILE)) {
rb_raise(rb_eTypeError, "IO expected");
}
rb_ary_push(ary, obj);
}

str = rb_str_buf_new(sizeof(int) * argc);

for (i = 0 ; i < argc; i++) {
VALUE obj = RARRAY_AREF(ary, i);
rb_io_t *fptr;
int fd;
GetOpenFile(obj, fptr);
fd = fptr->fd;
rb_str_buf_cat(str, (char *)&fd, sizeof(int));
}

result = ancdata_new(AF_UNIX, SOL_SOCKET, SCM_RIGHTS, str);
rb_ivar_set(result, rb_intern("unix_rights"), ary);
return result;
}
#else
#define ancillary_s_unix_rights rb_f_notimplement
#endif

#if defined(SCM_RIGHTS)






























static VALUE
ancillary_unix_rights(VALUE self)
{
int level, type;

level = ancillary_level(self);
type = ancillary_type(self);

if (level != SOL_SOCKET || type != SCM_RIGHTS)
rb_raise(rb_eTypeError, "SCM_RIGHTS ancillary data expected");

return rb_attr_get(self, rb_intern("unix_rights"));
}
#else
#define ancillary_unix_rights rb_f_notimplement
#endif

#if defined(SCM_TIMESTAMP) || defined(SCM_TIMESTAMPNS) || defined(SCM_BINTIME)

























static VALUE
ancillary_timestamp(VALUE self)
{
int level, type;
VALUE data;
VALUE result = Qnil;

level = ancillary_level(self);
type = ancillary_type(self);
data = ancillary_data(self);

#if defined(SCM_TIMESTAMP)
if (level == SOL_SOCKET && type == SCM_TIMESTAMP &&
RSTRING_LEN(data) == sizeof(struct timeval)) {
struct timeval tv;
memcpy((char*)&tv, RSTRING_PTR(data), sizeof(tv));
result = rb_time_new(tv.tv_sec, tv.tv_usec);
}
#endif

#if defined(SCM_TIMESTAMPNS)
if (level == SOL_SOCKET && type == SCM_TIMESTAMPNS &&
RSTRING_LEN(data) == sizeof(struct timespec)) {
struct timespec ts;
memcpy((char*)&ts, RSTRING_PTR(data), sizeof(ts));
result = rb_time_nano_new(ts.tv_sec, ts.tv_nsec);
}
#endif

#define add(x,y) (rb_funcall((x), '+', 1, (y)))
#define mul(x,y) (rb_funcall((x), '*', 1, (y)))
#define quo(x,y) (rb_funcall((x), rb_intern("quo"), 1, (y)))

#if defined(SCM_BINTIME)
if (level == SOL_SOCKET && type == SCM_BINTIME &&
RSTRING_LEN(data) == sizeof(struct bintime)) {
struct bintime bt;
VALUE d, timev;
memcpy((char*)&bt, RSTRING_PTR(data), sizeof(bt));
d = ULL2NUM(0x100000000ULL);
d = mul(d,d);
timev = add(TIMET2NUM(bt.sec), quo(ULL2NUM(bt.frac), d));
result = rb_time_num_new(timev, Qnil);
}
#endif

if (result == Qnil)
rb_raise(rb_eTypeError, "timestamp ancillary data expected");

return result;
}
#else
#define ancillary_timestamp rb_f_notimplement
#endif














static VALUE
ancillary_s_int(VALUE klass, VALUE vfamily, VALUE vlevel, VALUE vtype, VALUE integer)
{
int family = rsock_family_arg(vfamily);
int level = rsock_level_arg(family, vlevel);
int type = rsock_cmsg_type_arg(family, level, vtype);
int i = NUM2INT(integer);
return ancdata_new(family, level, type, rb_str_new((char*)&i, sizeof(i)));
}












static VALUE
ancillary_int(VALUE self)
{
VALUE data;
int i;
data = ancillary_data(self);
if (RSTRING_LEN(data) != sizeof(int))
rb_raise(rb_eTypeError, "size differ. expected as sizeof(int)=%d but %ld", (int)sizeof(int), (long)RSTRING_LEN(data));
memcpy((char*)&i, RSTRING_PTR(data), sizeof(int));
return INT2NUM(i);
}

#if defined(IPPROTO_IP) && defined(IP_PKTINFO) && defined(HAVE_STRUCT_IN_PKTINFO_IPI_SPEC_DST) 




















static VALUE
ancillary_s_ip_pktinfo(int argc, VALUE *argv, VALUE self)
{
VALUE v_addr, v_ifindex, v_spec_dst;
unsigned int ifindex;
struct sockaddr_in sa;
struct in_pktinfo pktinfo;

rb_scan_args(argc, argv, "21", &v_addr, &v_ifindex, &v_spec_dst);

SockAddrStringValue(v_addr);
ifindex = NUM2UINT(v_ifindex);
if (NIL_P(v_spec_dst))
v_spec_dst = v_addr;
else
SockAddrStringValue(v_spec_dst);

memset(&pktinfo, 0, sizeof(pktinfo));

memset(&sa, 0, sizeof(sa));
if (RSTRING_LEN(v_addr) != sizeof(sa))
rb_raise(rb_eArgError, "addr size different to AF_INET sockaddr");
memcpy(&sa, RSTRING_PTR(v_addr), sizeof(sa));
if (sa.sin_family != AF_INET)
rb_raise(rb_eArgError, "addr is not AF_INET sockaddr");
memcpy(&pktinfo.ipi_addr, &sa.sin_addr, sizeof(pktinfo.ipi_addr));

pktinfo.ipi_ifindex = ifindex;

memset(&sa, 0, sizeof(sa));
if (RSTRING_LEN(v_spec_dst) != sizeof(sa))
rb_raise(rb_eArgError, "spec_dat size different to AF_INET sockaddr");
memcpy(&sa, RSTRING_PTR(v_spec_dst), sizeof(sa));
if (sa.sin_family != AF_INET)
rb_raise(rb_eArgError, "spec_dst is not AF_INET sockaddr");
memcpy(&pktinfo.ipi_spec_dst, &sa.sin_addr, sizeof(pktinfo.ipi_spec_dst));

return ancdata_new(AF_INET, IPPROTO_IP, IP_PKTINFO, rb_str_new((char *)&pktinfo, sizeof(pktinfo)));
}
#else
#define ancillary_s_ip_pktinfo rb_f_notimplement
#endif

#if defined(IPPROTO_IP) && defined(IP_PKTINFO) && defined(HAVE_STRUCT_IN_PKTINFO_IPI_SPEC_DST) 



















static VALUE
ancillary_ip_pktinfo(VALUE self)
{
int level, type;
VALUE data;
struct in_pktinfo pktinfo;
struct sockaddr_in sa;
VALUE v_spec_dst, v_addr;

level = ancillary_level(self);
type = ancillary_type(self);
data = ancillary_data(self);

if (level != IPPROTO_IP || type != IP_PKTINFO ||
RSTRING_LEN(data) != sizeof(struct in_pktinfo)) {
rb_raise(rb_eTypeError, "IP_PKTINFO ancillary data expected");
}

memcpy(&pktinfo, RSTRING_PTR(data), sizeof(struct in_pktinfo));
memset(&sa, 0, sizeof(sa));

sa.sin_family = AF_INET;
memcpy(&sa.sin_addr, &pktinfo.ipi_addr, sizeof(sa.sin_addr));
v_addr = rsock_addrinfo_new((struct sockaddr *)&sa, sizeof(sa), PF_INET, 0, 0, Qnil, Qnil);

sa.sin_family = AF_INET;
memcpy(&sa.sin_addr, &pktinfo.ipi_spec_dst, sizeof(sa.sin_addr));
v_spec_dst = rsock_addrinfo_new((struct sockaddr *)&sa, sizeof(sa), PF_INET, 0, 0, Qnil, Qnil);

return rb_ary_new3(3, v_addr, UINT2NUM(pktinfo.ipi_ifindex), v_spec_dst);
}
#else
#define ancillary_ip_pktinfo rb_f_notimplement
#endif

#if defined(IPPROTO_IPV6) && defined(IPV6_PKTINFO) 














static VALUE
ancillary_s_ipv6_pktinfo(VALUE self, VALUE v_addr, VALUE v_ifindex)
{
unsigned int ifindex;
struct sockaddr_in6 sa;
struct in6_pktinfo pktinfo;

SockAddrStringValue(v_addr);
ifindex = NUM2UINT(v_ifindex);

memset(&pktinfo, 0, sizeof(pktinfo));

memset(&sa, 0, sizeof(sa));
if (RSTRING_LEN(v_addr) != sizeof(sa))
rb_raise(rb_eArgError, "addr size different to AF_INET6 sockaddr");
memcpy(&sa, RSTRING_PTR(v_addr), sizeof(sa));
if (sa.sin6_family != AF_INET6)
rb_raise(rb_eArgError, "addr is not AF_INET6 sockaddr");
memcpy(&pktinfo.ipi6_addr, &sa.sin6_addr, sizeof(pktinfo.ipi6_addr));

pktinfo.ipi6_ifindex = ifindex;

return ancdata_new(AF_INET6, IPPROTO_IPV6, IPV6_PKTINFO, rb_str_new((char *)&pktinfo, sizeof(pktinfo)));
}
#else
#define ancillary_s_ipv6_pktinfo rb_f_notimplement
#endif

#if defined(IPPROTO_IPV6) && defined(IPV6_PKTINFO) 
static void
extract_ipv6_pktinfo(VALUE self, struct in6_pktinfo *pktinfo_ptr, struct sockaddr_in6 *sa_ptr)
{
int level, type;
VALUE data;

level = ancillary_level(self);
type = ancillary_type(self);
data = ancillary_data(self);

if (level != IPPROTO_IPV6 || type != IPV6_PKTINFO ||
RSTRING_LEN(data) != sizeof(struct in6_pktinfo)) {
rb_raise(rb_eTypeError, "IPV6_PKTINFO ancillary data expected");
}

memcpy(pktinfo_ptr, RSTRING_PTR(data), sizeof(*pktinfo_ptr));

INIT_SOCKADDR((struct sockaddr *)sa_ptr, AF_INET6, sizeof(*sa_ptr));
memcpy(&sa_ptr->sin6_addr, &pktinfo_ptr->ipi6_addr, sizeof(sa_ptr->sin6_addr));
if (IN6_IS_ADDR_LINKLOCAL(&sa_ptr->sin6_addr))
sa_ptr->sin6_scope_id = pktinfo_ptr->ipi6_ifindex;
}
#endif

#if defined(IPPROTO_IPV6) && defined(IPV6_PKTINFO) 














static VALUE
ancillary_ipv6_pktinfo(VALUE self)
{
struct in6_pktinfo pktinfo;
struct sockaddr_in6 sa;
VALUE v_addr;

extract_ipv6_pktinfo(self, &pktinfo, &sa);
v_addr = rsock_addrinfo_new((struct sockaddr *)&sa, (socklen_t)sizeof(sa), PF_INET6, 0, 0, Qnil, Qnil);
return rb_ary_new3(2, v_addr, UINT2NUM(pktinfo.ipi6_ifindex));
}
#else
#define ancillary_ipv6_pktinfo rb_f_notimplement
#endif

#if defined(IPPROTO_IPV6) && defined(IPV6_PKTINFO) 














static VALUE
ancillary_ipv6_pktinfo_addr(VALUE self)
{
struct in6_pktinfo pktinfo;
struct sockaddr_in6 sa;
extract_ipv6_pktinfo(self, &pktinfo, &sa);
return rsock_addrinfo_new((struct sockaddr *)&sa, (socklen_t)sizeof(sa), PF_INET6, 0, 0, Qnil, Qnil);
}
#else
#define ancillary_ipv6_pktinfo_addr rb_f_notimplement
#endif

#if defined(IPPROTO_IPV6) && defined(IPV6_PKTINFO) 














static VALUE
ancillary_ipv6_pktinfo_ifindex(VALUE self)
{
struct in6_pktinfo pktinfo;
struct sockaddr_in6 sa;
extract_ipv6_pktinfo(self, &pktinfo, &sa);
return UINT2NUM(pktinfo.ipi6_ifindex);
}
#else
#define ancillary_ipv6_pktinfo_ifindex rb_f_notimplement
#endif

#if defined(SOL_SOCKET) && defined(SCM_RIGHTS) 
static int
anc_inspect_socket_rights(int level, int type, VALUE data, VALUE ret)
{
if (level == SOL_SOCKET && type == SCM_RIGHTS &&
0 < RSTRING_LEN(data) && (RSTRING_LEN(data) % sizeof(int) == 0)) {
long off;
for (off = 0; off < RSTRING_LEN(data); off += sizeof(int)) {
int fd;
memcpy((char*)&fd, RSTRING_PTR(data)+off, sizeof(int));
rb_str_catf(ret, " %d", fd);
}
return 1;
}
else {
return 0;
}
}
#endif

#if defined(SCM_CREDENTIALS) 
static int
anc_inspect_passcred_credentials(int level, int type, VALUE data, VALUE ret)
{
if (level == SOL_SOCKET && type == SCM_CREDENTIALS &&
RSTRING_LEN(data) == sizeof(struct ucred)) {
struct ucred cred;
memcpy(&cred, RSTRING_PTR(data), sizeof(struct ucred));
rb_str_catf(ret, " pid=%u uid=%u gid=%u", cred.pid, cred.uid, cred.gid);
rb_str_cat2(ret, " (ucred)");
return 1;
}
else {
return 0;
}
}
#endif

#if defined(SCM_CREDS)
#define INSPECT_SCM_CREDS
static int
anc_inspect_socket_creds(int level, int type, VALUE data, VALUE ret)
{
if (level != SOL_SOCKET && type != SCM_CREDS)
return 0;












#if defined(HAVE_TYPE_STRUCT_CMSGCRED) 
if (RSTRING_LEN(data) == sizeof(struct cmsgcred)) {
struct cmsgcred cred;
memcpy(&cred, RSTRING_PTR(data), sizeof(struct cmsgcred));
rb_str_catf(ret, " pid=%u", cred.cmcred_pid);
rb_str_catf(ret, " uid=%u", cred.cmcred_uid);
rb_str_catf(ret, " euid=%u", cred.cmcred_euid);
rb_str_catf(ret, " gid=%u", cred.cmcred_gid);
if (cred.cmcred_ngroups) {
int i;
const char *sep = " groups=";
for (i = 0; i < cred.cmcred_ngroups; i++) {
rb_str_catf(ret, "%s%u", sep, cred.cmcred_groups[i]);
sep = ",";
}
}
rb_str_cat2(ret, " (cmsgcred)");
return 1;
}
#endif
#if defined(HAVE_TYPE_STRUCT_SOCKCRED) 
if ((size_t)RSTRING_LEN(data) >= SOCKCREDSIZE(0)) {
struct sockcred cred0, *cred;
memcpy(&cred0, RSTRING_PTR(data), SOCKCREDSIZE(0));
if ((size_t)RSTRING_LEN(data) == SOCKCREDSIZE(cred0.sc_ngroups)) {
cred = (struct sockcred *)ALLOCA_N(char, SOCKCREDSIZE(cred0.sc_ngroups));
memcpy(cred, RSTRING_PTR(data), SOCKCREDSIZE(cred0.sc_ngroups));
rb_str_catf(ret, " uid=%u", cred->sc_uid);
rb_str_catf(ret, " euid=%u", cred->sc_euid);
rb_str_catf(ret, " gid=%u", cred->sc_gid);
rb_str_catf(ret, " egid=%u", cred->sc_egid);
if (cred0.sc_ngroups) {
int i;
const char *sep = " groups=";
for (i = 0; i < cred0.sc_ngroups; i++) {
rb_str_catf(ret, "%s%u", sep, cred->sc_groups[i]);
sep = ",";
}
}
rb_str_cat2(ret, " (sockcred)");
return 1;
}
}
#endif
return 0;
}
#endif

#if defined(IPPROTO_IP) && defined(IP_RECVDSTADDR) 
static int
anc_inspect_ip_recvdstaddr(int level, int type, VALUE data, VALUE ret)
{
if (level == IPPROTO_IP && type == IP_RECVDSTADDR &&
RSTRING_LEN(data) == sizeof(struct in_addr)) {
struct in_addr addr;
char addrbuf[INET_ADDRSTRLEN];
memcpy(&addr, RSTRING_PTR(data), sizeof(addr));
if (inet_ntop(AF_INET, &addr, addrbuf, (socklen_t)sizeof(addrbuf)) == NULL)
rb_str_cat2(ret, " invalid-address");
else
rb_str_catf(ret, " %s", addrbuf);
return 1;
}
else {
return 0;
}
}
#endif

#if defined(IPPROTO_IP) && defined(IP_PKTINFO) && defined(HAVE_STRUCT_IN_PKTINFO_IPI_SPEC_DST) 
static int
anc_inspect_ip_pktinfo(int level, int type, VALUE data, VALUE ret)
{
if (level == IPPROTO_IP && type == IP_PKTINFO &&
RSTRING_LEN(data) == sizeof(struct in_pktinfo)) {
struct in_pktinfo pktinfo;
char buf[INET_ADDRSTRLEN > IFNAMSIZ ? INET_ADDRSTRLEN : IFNAMSIZ];
memcpy(&pktinfo, RSTRING_PTR(data), sizeof(pktinfo));
if (inet_ntop(AF_INET, &pktinfo.ipi_addr, buf, sizeof(buf)) == NULL)
rb_str_cat2(ret, " invalid-address");
else
rb_str_catf(ret, " %s", buf);
if (if_indextoname(pktinfo.ipi_ifindex, buf) == NULL)
rb_str_catf(ret, " ifindex:%d", pktinfo.ipi_ifindex);
else
rb_str_catf(ret, " %s", buf);
if (inet_ntop(AF_INET, &pktinfo.ipi_spec_dst, buf, sizeof(buf)) == NULL)
rb_str_cat2(ret, " spec_dst:invalid-address");
else
rb_str_catf(ret, " spec_dst:%s", buf);
return 1;
}
else {
return 0;
}
}
#endif

#if defined(IPPROTO_IPV6) && defined(IPV6_PKTINFO) && defined(HAVE_TYPE_STRUCT_IN6_PKTINFO) 
static int
anc_inspect_ipv6_pktinfo(int level, int type, VALUE data, VALUE ret)
{
if (level == IPPROTO_IPV6 && type == IPV6_PKTINFO &&
RSTRING_LEN(data) == sizeof(struct in6_pktinfo)) {
struct in6_pktinfo *pktinfo = (struct in6_pktinfo *)RSTRING_PTR(data);
struct in6_addr addr;
unsigned int ifindex;
char addrbuf[INET6_ADDRSTRLEN], ifbuf[IFNAMSIZ];
memcpy(&addr, &pktinfo->ipi6_addr, sizeof(addr));
memcpy(&ifindex, &pktinfo->ipi6_ifindex, sizeof(ifindex));
if (inet_ntop(AF_INET6, &addr, addrbuf, (socklen_t)sizeof(addrbuf)) == NULL)
rb_str_cat2(ret, " invalid-address");
else
rb_str_catf(ret, " %s", addrbuf);
if (if_indextoname(ifindex, ifbuf) == NULL)
rb_str_catf(ret, " ifindex:%d", ifindex);
else
rb_str_catf(ret, " %s", ifbuf);
return 1;
}
else {
return 0;
}
}
#endif

#if defined(SCM_TIMESTAMP) 
static int
inspect_timeval_as_abstime(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(struct timeval)) {
struct timeval tv;
time_t time;
struct tm tm;
char buf[32];
memcpy((char*)&tv, RSTRING_PTR(data), sizeof(tv));
time = tv.tv_sec;
tm = *localtime(&time);
strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
rb_str_catf(ret, " %s.%06ld", buf, (long)tv.tv_usec);
return 1;
}
else {
return 0;
}
}
#endif

#if defined(SCM_TIMESTAMPNS) 
static int
inspect_timespec_as_abstime(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(struct timespec)) {
struct timespec ts;
struct tm tm;
char buf[32];
memcpy((char*)&ts, RSTRING_PTR(data), sizeof(ts));
tm = *localtime(&ts.tv_sec);
strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
rb_str_catf(ret, " %s.%09ld", buf, (long)ts.tv_nsec);
return 1;
}
else {
return 0;
}
}
#endif

#if defined(SCM_BINTIME) 
static int
inspect_bintime_as_abstime(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(struct bintime)) {
struct bintime bt;
struct tm tm;
uint64_t frac_h, frac_l;
uint64_t scale_h, scale_l;
uint64_t tmp1, tmp2;
uint64_t res_h, res_l;
char buf[32];
memcpy((char*)&bt, RSTRING_PTR(data), sizeof(bt));
tm = *localtime(&bt.sec);
strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);



frac_h = bt.frac >> 32;
frac_l = bt.frac & 0xffffffff;

scale_h = 0x8ac72304; 
scale_l = 0x89e80000;

res_h = frac_h * scale_h;
res_l = frac_l * scale_l;

tmp1 = frac_h * scale_l;
res_h += tmp1 >> 32;
tmp2 = res_l;
res_l += tmp1 & 0xffffffff;
if (res_l < tmp2) res_h++;

tmp1 = frac_l * scale_h;
res_h += tmp1 >> 32;
tmp2 = res_l;
res_l += tmp1 & 0xffffffff;
if (res_l < tmp2) res_h++;

rb_str_catf(ret, " %s.%019"PRIu64, buf, res_h);
return 1;
}
else {
return 0;
}
}
#endif










static VALUE
ancillary_inspect(VALUE self)
{
VALUE ret;
int family, level, type;
VALUE data;
ID family_id, level_id, type_id;
VALUE vtype;
int inspected;

family = ancillary_family(self);
level = ancillary_level(self);
type = ancillary_type(self);
data = ancillary_data(self);

ret = rb_sprintf("#<%s:", rb_obj_classname(self));

family_id = rsock_intern_family_noprefix(family);
if (family_id)
rb_str_catf(ret, " %s", rb_id2name(family_id));
else
rb_str_catf(ret, " family:%d", family);

if (level == SOL_SOCKET) {
rb_str_cat2(ret, " SOCKET");

type_id = rsock_intern_scm_optname(type);
if (type_id)
rb_str_catf(ret, " %s", rb_id2name(type_id));
else
rb_str_catf(ret, " cmsg_type:%d", type);
}
else if (IS_IP_FAMILY(family)) {
level_id = rsock_intern_iplevel(level);
if (level_id)
rb_str_catf(ret, " %s", rb_id2name(level_id));
else
rb_str_catf(ret, " cmsg_level:%d", level);

vtype = ip_cmsg_type_to_sym(level, type);
if (SYMBOL_P(vtype))
rb_str_catf(ret, " %"PRIsVALUE, rb_sym2str(vtype));
else
rb_str_catf(ret, " cmsg_type:%d", type);
}
else {
rb_str_catf(ret, " cmsg_level:%d", level);
rb_str_catf(ret, " cmsg_type:%d", type);
}

inspected = 0;

if (level == SOL_SOCKET)
family = AF_UNSPEC;

switch (family) {
case AF_UNSPEC:
switch (level) {
#if defined(SOL_SOCKET)
case SOL_SOCKET:
switch (type) {
#if defined(SCM_TIMESTAMP) 
case SCM_TIMESTAMP: inspected = inspect_timeval_as_abstime(level, type, data, ret); break;
#endif
#if defined(SCM_TIMESTAMPNS) 
case SCM_TIMESTAMPNS: inspected = inspect_timespec_as_abstime(level, type, data, ret); break;
#endif
#if defined(SCM_BINTIME) 
case SCM_BINTIME: inspected = inspect_bintime_as_abstime(level, type, data, ret); break;
#endif
#if defined(SCM_RIGHTS) 
case SCM_RIGHTS: inspected = anc_inspect_socket_rights(level, type, data, ret); break;
#endif
#if defined(SCM_CREDENTIALS) 
case SCM_CREDENTIALS: inspected = anc_inspect_passcred_credentials(level, type, data, ret); break;
#endif
#if defined(INSPECT_SCM_CREDS) 
case SCM_CREDS: inspected = anc_inspect_socket_creds(level, type, data, ret); break;
#endif
}
break;
#endif
}
break;

case AF_INET:
#if defined(INET6)
case AF_INET6:
#endif
switch (level) {
#if defined(IPPROTO_IP)
case IPPROTO_IP:
switch (type) {
#if defined(IP_RECVDSTADDR) 
case IP_RECVDSTADDR: inspected = anc_inspect_ip_recvdstaddr(level, type, data, ret); break;
#endif
#if defined(IP_PKTINFO) && defined(HAVE_STRUCT_IN_PKTINFO_IPI_SPEC_DST) 
case IP_PKTINFO: inspected = anc_inspect_ip_pktinfo(level, type, data, ret); break;
#endif
}
break;
#endif

#if defined(IPPROTO_IPV6)
case IPPROTO_IPV6:
switch (type) {
#if defined(IPV6_PKTINFO) && defined(HAVE_TYPE_STRUCT_IN6_PKTINFO) 
case IPV6_PKTINFO: inspected = anc_inspect_ipv6_pktinfo(level, type, data, ret); break;
#endif
}
break;
#endif
}
break;
}

if (!inspected) {
rb_str_cat2(ret, " ");
rb_str_append(ret, rb_str_dump(data));
}

rb_str_cat2(ret, ">");

return ret;
}













static VALUE
ancillary_cmsg_is_p(VALUE self, VALUE vlevel, VALUE vtype)
{
int family = ancillary_family(self);
int level = rsock_level_arg(family, vlevel);
int type = rsock_cmsg_type_arg(family, level, vtype);

if (ancillary_level(self) == level &&
ancillary_type(self) == type)
return Qtrue;
else
return Qfalse;
}

#endif

#if defined(HAVE_SENDMSG)
struct sendmsg_args_struct {
int fd;
int flags;
const struct msghdr *msg;
};

static void *
nogvl_sendmsg_func(void *ptr)
{
struct sendmsg_args_struct *args = ptr;
return (void *)(VALUE)sendmsg(args->fd, args->msg, args->flags);
}

static ssize_t
rb_sendmsg(int fd, const struct msghdr *msg, int flags)
{
struct sendmsg_args_struct args;
args.fd = fd;
args.msg = msg;
args.flags = flags;
return (ssize_t)rb_thread_call_without_gvl(nogvl_sendmsg_func, &args, RUBY_UBF_IO, 0);
}

static VALUE
bsock_sendmsg_internal(VALUE sock, VALUE data, VALUE vflags,
VALUE dest_sockaddr, VALUE controls, VALUE ex,
int nonblock)
{
rb_io_t *fptr;
struct msghdr mh;
struct iovec iov;
VALUE tmp;
int controls_num;
#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
VALUE controls_str = 0;
int family;
#endif
int flags;
ssize_t ss;

GetOpenFile(sock, fptr);
#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
family = rsock_getfamily(fptr);
#endif

StringValue(data);
tmp = rb_str_tmp_frozen_acquire(data);

if (!RB_TYPE_P(controls, T_ARRAY)) {
controls = rb_ary_new();
}
controls_num = RARRAY_LENINT(controls);

if (controls_num) {
#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
int i;
size_t last_pad = 0;
const VALUE *controls_ptr = RARRAY_CONST_PTR(controls);
#if defined(__NetBSD__)
int last_level = 0;
int last_type = 0;
#endif
controls_str = rb_str_tmp_new(0);
for (i = 0; i < controls_num; i++) {
VALUE elt = controls_ptr[i], v;
VALUE vlevel, vtype;
int level, type;
VALUE cdata;
long oldlen;
struct cmsghdr cmh;
char *cmsg;
size_t cspace;
v = rb_check_convert_type(elt, T_ARRAY, "Array", "to_ary");
if (!NIL_P(v)) {
elt = v;
if (RARRAY_LEN(elt) != 3)
rb_raise(rb_eArgError, "an element of controls should be 3-elements array");
vlevel = rb_ary_entry(elt, 0);
vtype = rb_ary_entry(elt, 1);
cdata = rb_ary_entry(elt, 2);
}
else {
vlevel = rb_funcall(elt, rb_intern("level"), 0);
vtype = rb_funcall(elt, rb_intern("type"), 0);
cdata = rb_funcall(elt, rb_intern("data"), 0);
}
level = rsock_level_arg(family, vlevel);
type = rsock_cmsg_type_arg(family, level, vtype);
StringValue(cdata);
oldlen = RSTRING_LEN(controls_str);
cspace = CMSG_SPACE(RSTRING_LEN(cdata));
rb_str_resize(controls_str, oldlen + cspace);
cmsg = RSTRING_PTR(controls_str)+oldlen;
memset((char *)cmsg, 0, cspace);
memset((char *)&cmh, 0, sizeof(cmh));
cmh.cmsg_level = level;
cmh.cmsg_type = type;
cmh.cmsg_len = (socklen_t)CMSG_LEN(RSTRING_LEN(cdata));
MEMCPY(cmsg, &cmh, char, sizeof(cmh));
MEMCPY(cmsg+((char*)CMSG_DATA(&cmh)-(char*)&cmh), RSTRING_PTR(cdata), char, RSTRING_LEN(cdata));
#if defined(__NetBSD__)
last_level = cmh.cmsg_level;
last_type = cmh.cmsg_type;
#endif
last_pad = cspace - cmh.cmsg_len;
}
if (last_pad) {




















#if defined(__NetBSD__)
if (last_level == SOL_SOCKET && last_type == SCM_RIGHTS)
rb_str_set_len(controls_str, RSTRING_LEN(controls_str)-last_pad);
#endif
}
RB_GC_GUARD(controls);
#else
rb_raise(rb_eNotImpError, "control message for sendmsg is unimplemented");
#endif
}

flags = NIL_P(vflags) ? 0 : NUM2INT(vflags);
#if defined(MSG_DONTWAIT)
if (nonblock)
flags |= MSG_DONTWAIT;
#endif

if (!NIL_P(dest_sockaddr))
SockAddrStringValue(dest_sockaddr);

rb_io_check_closed(fptr);

retry:
memset(&mh, 0, sizeof(mh));
if (!NIL_P(dest_sockaddr)) {
mh.msg_name = RSTRING_PTR(dest_sockaddr);
mh.msg_namelen = RSTRING_SOCKLEN(dest_sockaddr);
}
mh.msg_iovlen = 1;
mh.msg_iov = &iov;
iov.iov_base = RSTRING_PTR(tmp);
iov.iov_len = RSTRING_LEN(tmp);
#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
if (controls_str) {
mh.msg_control = RSTRING_PTR(controls_str);
mh.msg_controllen = RSTRING_SOCKLEN(controls_str);
}
#endif

rb_io_check_closed(fptr);
if (nonblock && !MSG_DONTWAIT_RELIABLE)
rb_io_set_nonblock(fptr);

ss = rb_sendmsg(fptr->fd, &mh, flags);

if (ss == -1) {
int e;
if (!nonblock && rb_io_wait_writable(fptr->fd)) {
rb_io_check_closed(fptr);
goto retry;
}
e = errno;
if (nonblock && (e == EWOULDBLOCK || e == EAGAIN)) {
if (ex == Qfalse) {
return sym_wait_writable;
}
rb_readwrite_syserr_fail(RB_IO_WAIT_WRITABLE, e,
"sendmsg(2) would block");
}
rb_syserr_fail(e, "sendmsg(2)");
}
#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
RB_GC_GUARD(controls_str);
#endif
rb_str_tmp_frozen_release(data, tmp);

return SSIZET2NUM(ss);
}
#endif

#if defined(HAVE_SENDMSG)
VALUE
rsock_bsock_sendmsg(VALUE sock, VALUE data, VALUE flags, VALUE dest_sockaddr,
VALUE controls)
{
return bsock_sendmsg_internal(sock, data, flags, dest_sockaddr, controls,
Qtrue, 0);
}
#endif

#if defined(HAVE_SENDMSG)
VALUE
rsock_bsock_sendmsg_nonblock(VALUE sock, VALUE data, VALUE flags,
VALUE dest_sockaddr, VALUE controls, VALUE ex)
{
return bsock_sendmsg_internal(sock, data, flags, dest_sockaddr,
controls, ex, 1);
}
#endif

#if defined(HAVE_RECVMSG)
struct recvmsg_args_struct {
int fd;
int flags;
struct msghdr *msg;
};

ssize_t
rsock_recvmsg(int socket, struct msghdr *message, int flags)
{
ssize_t ret;
socklen_t len0;
#if defined(MSG_CMSG_CLOEXEC)

flags |= MSG_CMSG_CLOEXEC;
#endif
len0 = message->msg_namelen;
ret = recvmsg(socket, message, flags);
if (ret != -1 && len0 < message->msg_namelen)
message->msg_namelen = len0;
return ret;
}

static void *
nogvl_recvmsg_func(void *ptr)
{
struct recvmsg_args_struct *args = ptr;
int flags = args->flags;
return (void *)rsock_recvmsg(args->fd, args->msg, flags);
}

static ssize_t
rb_recvmsg(int fd, struct msghdr *msg, int flags)
{
struct recvmsg_args_struct args;
args.fd = fd;
args.msg = msg;
args.flags = flags;
return (ssize_t)rb_thread_call_without_gvl(nogvl_recvmsg_func, &args, RUBY_UBF_IO, 0);
}

#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
static void
discard_cmsg(struct cmsghdr *cmh, char *msg_end, int msg_peek_p)
{
#if !defined(FD_PASSING_WORK_WITH_RECVMSG_MSG_PEEK)








if (msg_peek_p)
return;
#endif
if (cmh->cmsg_level == SOL_SOCKET && cmh->cmsg_type == SCM_RIGHTS) {
int *fdp = (int *)CMSG_DATA(cmh);
int *end = (int *)((char *)cmh + cmh->cmsg_len);
while ((char *)fdp + sizeof(int) <= (char *)end &&
(char *)fdp + sizeof(int) <= msg_end) {
rb_update_max_fd(*fdp);
close(*fdp);
fdp++;
}
}
}
#endif

void
rsock_discard_cmsg_resource(struct msghdr *mh, int msg_peek_p)
{
#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
struct cmsghdr *cmh;
char *msg_end;

if (mh->msg_controllen == 0)
return;

msg_end = (char *)mh->msg_control + mh->msg_controllen;

for (cmh = CMSG_FIRSTHDR(mh); cmh != NULL; cmh = CMSG_NXTHDR(mh, cmh)) {
discard_cmsg(cmh, msg_end, msg_peek_p);
}
#endif
}

#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
static void
make_io_for_unix_rights(VALUE ctl, struct cmsghdr *cmh, char *msg_end)
{
if (cmh->cmsg_level == SOL_SOCKET && cmh->cmsg_type == SCM_RIGHTS) {
int *fdp, *end;
VALUE ary = rb_ary_new();
rb_ivar_set(ctl, rb_intern("unix_rights"), ary);
fdp = (int *)CMSG_DATA(cmh);
end = (int *)((char *)cmh + cmh->cmsg_len);
while ((char *)fdp + sizeof(int) <= (char *)end &&
(char *)fdp + sizeof(int) <= msg_end) {
int fd = *fdp;
struct stat stbuf;
VALUE io;
if (fstat(fd, &stbuf) == -1)
rb_raise(rb_eSocket, "invalid fd in SCM_RIGHTS");
rb_update_max_fd(fd);
if (rsock_cmsg_cloexec_state < 0)
rsock_cmsg_cloexec_state = rsock_detect_cloexec(fd);
if (rsock_cmsg_cloexec_state == 0 || fd <= 2)
rb_maygvl_fd_fix_cloexec(fd);
if (S_ISSOCK(stbuf.st_mode))
io = rsock_init_sock(rb_obj_alloc(rb_cSocket), fd);
else
io = rb_io_fdopen(fd, O_RDWR, NULL);
ary = rb_attr_get(ctl, rb_intern("unix_rights"));
rb_ary_push(ary, io);
fdp++;
}
OBJ_FREEZE(ary);
}
}
#endif

static VALUE
bsock_recvmsg_internal(VALUE sock,
VALUE vmaxdatlen, VALUE vflags, VALUE vmaxctllen,
VALUE scm_rights, VALUE ex, int nonblock)
{
rb_io_t *fptr;
int grow_buffer;
size_t maxdatlen;
int flags, orig_flags;
struct msghdr mh;
struct iovec iov;
union_sockaddr namebuf;
char *datbuf;
VALUE dat_str = Qnil;
VALUE ret;
ssize_t ss;
int request_scm_rights;
#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
struct cmsghdr *cmh;
size_t maxctllen;
char *ctlbuf;
VALUE ctl_str = Qnil;
int family;
int gc_done = 0;
#endif

maxdatlen = NIL_P(vmaxdatlen) ? 4096 : NUM2SIZET(vmaxdatlen);
#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
maxctllen = NIL_P(vmaxctllen) ? 4096 : NUM2SIZET(vmaxctllen);
#else
if (!NIL_P(vmaxctllen))
rb_raise(rb_eArgError, "control message not supported");
#endif
flags = NUM2INT(vflags);
#if defined(MSG_DONTWAIT)
if (nonblock)
flags |= MSG_DONTWAIT;
#endif
orig_flags = flags;

grow_buffer = NIL_P(vmaxdatlen) || NIL_P(vmaxctllen);

request_scm_rights = 0;
if (RTEST(scm_rights))
request_scm_rights = 1;
#if !defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
if (request_scm_rights)
rb_raise(rb_eNotImpError, "control message for recvmsg is unimplemented");
#endif

GetOpenFile(sock, fptr);
if (rb_io_read_pending(fptr)) {
rb_raise(rb_eIOError, "recvmsg for buffered IO");
}

#if !defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
if (grow_buffer) {
int socktype;
socklen_t optlen = (socklen_t)sizeof(socktype);
if (getsockopt(fptr->fd, SOL_SOCKET, SO_TYPE, (void*)&socktype, &optlen) == -1) {
rb_sys_fail("getsockopt(SO_TYPE)");
}
if (socktype == SOCK_STREAM)
grow_buffer = 0;
}
#endif

retry:
if (NIL_P(dat_str))
dat_str = rb_str_tmp_new(maxdatlen);
else
rb_str_resize(dat_str, maxdatlen);
datbuf = RSTRING_PTR(dat_str);

#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
if (NIL_P(ctl_str))
ctl_str = rb_str_tmp_new(maxctllen);
else
rb_str_resize(ctl_str, maxctllen);
ctlbuf = RSTRING_PTR(ctl_str);
#endif

memset(&mh, 0, sizeof(mh));

memset(&namebuf, 0, sizeof(namebuf));
mh.msg_name = &namebuf.addr;
mh.msg_namelen = (socklen_t)sizeof(namebuf);

mh.msg_iov = &iov;
mh.msg_iovlen = 1;
iov.iov_base = datbuf;
iov.iov_len = maxdatlen;

#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
mh.msg_control = ctlbuf;
mh.msg_controllen = (socklen_t)maxctllen;
#endif

if (grow_buffer)
flags |= MSG_PEEK;

rb_io_check_closed(fptr);
if (nonblock && !MSG_DONTWAIT_RELIABLE)
rb_io_set_nonblock(fptr);

ss = rb_recvmsg(fptr->fd, &mh, flags);

if (ss == -1) {
int e;
if (!nonblock && rb_io_wait_readable(fptr->fd)) {
rb_io_check_closed(fptr);
goto retry;
}
e = errno;
if (nonblock && (e == EWOULDBLOCK || e == EAGAIN)) {
if (ex == Qfalse) {
return sym_wait_readable;
}
rb_readwrite_syserr_fail(RB_IO_WAIT_READABLE, e, "recvmsg(2) would block");
}
#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
if (!gc_done && (e == EMFILE || e == EMSGSIZE)) {






gc_and_retry:
rb_gc();
gc_done = 1;
goto retry;
}
#else
if (NIL_P(vmaxdatlen) && grow_buffer && e == EMSGSIZE)
ss = (ssize_t)iov.iov_len;
else
#endif
rb_syserr_fail(e, "recvmsg(2)");
}

if (grow_buffer) {
int grown = 0;
if (NIL_P(vmaxdatlen) && ss != -1 && ss == (ssize_t)iov.iov_len) {
if (SIZE_MAX/2 < maxdatlen)
rb_raise(rb_eArgError, "max data length too big");
maxdatlen *= 2;
grown = 1;
}
#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
if (NIL_P(vmaxctllen) && (mh.msg_flags & MSG_CTRUNC)) {
#define BIG_ENOUGH_SPACE 65536
if (BIG_ENOUGH_SPACE < maxctllen &&
(socklen_t)mh.msg_controllen < (socklen_t)(maxctllen - BIG_ENOUGH_SPACE)) {


if (!gc_done) {
rsock_discard_cmsg_resource(&mh, (flags & MSG_PEEK) != 0);
goto gc_and_retry;
}
}
else {
if (SIZE_MAX/2 < maxctllen)
rb_raise(rb_eArgError, "max control message length too big");
maxctllen *= 2;
grown = 1;
}
#undef BIG_ENOUGH_SPACE
}
#endif
if (grown) {
rsock_discard_cmsg_resource(&mh, (flags & MSG_PEEK) != 0);
goto retry;
}
else {
grow_buffer = 0;
if (flags != orig_flags) {
rsock_discard_cmsg_resource(&mh, (flags & MSG_PEEK) != 0);
flags = orig_flags;
goto retry;
}
}
}

if (NIL_P(dat_str))
dat_str = rb_str_new(datbuf, ss);
else {
rb_str_resize(dat_str, ss);
rb_obj_reveal(dat_str, rb_cString);
}

ret = rb_ary_new3(3, dat_str,
rsock_io_socket_addrinfo(sock, mh.msg_name, mh.msg_namelen),
#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
INT2NUM(mh.msg_flags)
#else
Qnil
#endif
);

#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)
family = rsock_getfamily(fptr);
if (mh.msg_controllen) {
char *msg_end = (char *)mh.msg_control + mh.msg_controllen;
for (cmh = CMSG_FIRSTHDR(&mh); cmh != NULL; cmh = CMSG_NXTHDR(&mh, cmh)) {
VALUE ctl;
char *ctl_end;
size_t clen;
if (cmh->cmsg_len == 0) {
rb_raise(rb_eTypeError, "invalid control message (cmsg_len == 0)");
}
ctl_end = (char*)cmh + cmh->cmsg_len;
clen = (ctl_end <= msg_end ? ctl_end : msg_end) - (char*)CMSG_DATA(cmh);
ctl = ancdata_new(family, cmh->cmsg_level, cmh->cmsg_type, rb_str_new((char*)CMSG_DATA(cmh), clen));
if (request_scm_rights)
make_io_for_unix_rights(ctl, cmh, msg_end);
else
discard_cmsg(cmh, msg_end, (flags & MSG_PEEK) != 0);
rb_ary_push(ret, ctl);
}
RB_GC_GUARD(ctl_str);
}
#endif

return ret;
}
#endif

#if defined(HAVE_RECVMSG)
VALUE
rsock_bsock_recvmsg(VALUE sock, VALUE dlen, VALUE flags, VALUE clen,
VALUE scm_rights)
{
VALUE ex = Qtrue;
return bsock_recvmsg_internal(sock, dlen, flags, clen, scm_rights, ex, 0);
}
#endif

#if defined(HAVE_RECVMSG)
VALUE
rsock_bsock_recvmsg_nonblock(VALUE sock, VALUE dlen, VALUE flags, VALUE clen,
VALUE scm_rights, VALUE ex)
{
return bsock_recvmsg_internal(sock, dlen, flags, clen, scm_rights, ex, 1);
}
#endif

void
rsock_init_ancdata(void)
{
#if defined(HAVE_STRUCT_MSGHDR_MSG_CONTROL)







rb_cAncillaryData = rb_define_class_under(rb_cSocket, "AncillaryData", rb_cObject);
rb_define_method(rb_cAncillaryData, "initialize", ancillary_initialize, 4);
rb_define_method(rb_cAncillaryData, "inspect", ancillary_inspect, 0);
rb_define_method(rb_cAncillaryData, "family", ancillary_family_m, 0);
rb_define_method(rb_cAncillaryData, "level", ancillary_level_m, 0);
rb_define_method(rb_cAncillaryData, "type", ancillary_type_m, 0);
rb_define_method(rb_cAncillaryData, "data", ancillary_data, 0);

rb_define_method(rb_cAncillaryData, "cmsg_is?", ancillary_cmsg_is_p, 2);

rb_define_singleton_method(rb_cAncillaryData, "int", ancillary_s_int, 4);
rb_define_method(rb_cAncillaryData, "int", ancillary_int, 0);

rb_define_singleton_method(rb_cAncillaryData, "unix_rights", ancillary_s_unix_rights, -1);
rb_define_method(rb_cAncillaryData, "unix_rights", ancillary_unix_rights, 0);

rb_define_method(rb_cAncillaryData, "timestamp", ancillary_timestamp, 0);

rb_define_singleton_method(rb_cAncillaryData, "ip_pktinfo", ancillary_s_ip_pktinfo, -1);
rb_define_method(rb_cAncillaryData, "ip_pktinfo", ancillary_ip_pktinfo, 0);

rb_define_singleton_method(rb_cAncillaryData, "ipv6_pktinfo", ancillary_s_ipv6_pktinfo, 2);
rb_define_method(rb_cAncillaryData, "ipv6_pktinfo", ancillary_ipv6_pktinfo, 0);
rb_define_method(rb_cAncillaryData, "ipv6_pktinfo_addr", ancillary_ipv6_pktinfo_addr, 0);
rb_define_method(rb_cAncillaryData, "ipv6_pktinfo_ifindex", ancillary_ipv6_pktinfo_ifindex, 0);
#endif
#undef rb_intern
sym_wait_readable = ID2SYM(rb_intern("wait_readable"));
sym_wait_writable = ID2SYM(rb_intern("wait_writable"));
}
