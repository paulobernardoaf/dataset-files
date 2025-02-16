#include "rubysocket.h"
VALUE rb_cSockOpt;
#define pack_var(v) rb_str_new((const char *)&(v), sizeof(v))
#define CAT(x,y) x##y
#define XCAT(x,y) CAT(x,y)
#if defined(__linux__) || defined(__GNU__) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__APPLE__) || defined(_WIN32) || defined(__CYGWIN__)
#define TYPE_IP_MULTICAST_LOOP int
#define TYPE_IP_MULTICAST_TTL int
#else
#define TYPE_IP_MULTICAST_LOOP byte
#define TYPE_IP_MULTICAST_TTL byte
#define USE_INSPECT_BYTE 1
#endif
#define check_size(len, size) ((len) == (size) ? (void)0 : rb_raise(rb_eTypeError, "size differ. expected as "#size"=%d but %ld", (int)size, (long)(len)))
static VALUE
sockopt_pack_byte(VALUE value)
{
char i = NUM2CHR(rb_to_int(value));
return pack_var(i);
}
static VALUE
sockopt_pack_int(VALUE value)
{
int i = NUM2INT(rb_to_int(value));
return pack_var(i);
}
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
optname_to_sym(int level, int optname)
{
switch (level) {
case SOL_SOCKET:
return constant_to_sym(optname, rsock_intern_so_optname);
case IPPROTO_IP:
return constant_to_sym(optname, rsock_intern_ip_optname);
#if defined(IPPROTO_IPV6)
case IPPROTO_IPV6:
return constant_to_sym(optname, rsock_intern_ipv6_optname);
#endif
case IPPROTO_TCP:
return constant_to_sym(optname, rsock_intern_tcp_optname);
case IPPROTO_UDP:
return constant_to_sym(optname, rsock_intern_udp_optname);
default:
return INT2NUM(optname);
}
}
static VALUE
sockopt_initialize(VALUE self, VALUE vfamily, VALUE vlevel, VALUE voptname, VALUE data)
{
int family = rsock_family_arg(vfamily);
int level = rsock_level_arg(family, vlevel);
int optname = rsock_optname_arg(family, level, voptname);
StringValue(data);
rb_ivar_set(self, rb_intern("family"), INT2NUM(family));
rb_ivar_set(self, rb_intern("level"), INT2NUM(level));
rb_ivar_set(self, rb_intern("optname"), INT2NUM(optname));
rb_ivar_set(self, rb_intern("data"), data);
return self;
}
VALUE
rsock_sockopt_new(int family, int level, int optname, VALUE data)
{
NEWOBJ_OF(obj, struct RObject, rb_cSockOpt, T_OBJECT);
StringValue(data);
sockopt_initialize((VALUE)obj, INT2NUM(family), INT2NUM(level), INT2NUM(optname), data);
return (VALUE)obj;
}
static VALUE
sockopt_family_m(VALUE self)
{
return rb_attr_get(self, rb_intern("family"));
}
static int
sockopt_level(VALUE self)
{
return NUM2INT(rb_attr_get(self, rb_intern("level")));
}
static VALUE
sockopt_level_m(VALUE self)
{
return INT2NUM(sockopt_level(self));
}
static int
sockopt_optname(VALUE self)
{
return NUM2INT(rb_attr_get(self, rb_intern("optname")));
}
static VALUE
sockopt_optname_m(VALUE self)
{
return INT2NUM(sockopt_optname(self));
}
static VALUE
sockopt_data(VALUE self)
{
VALUE v = rb_attr_get(self, rb_intern("data"));
StringValue(v);
return v;
}
static VALUE
sockopt_s_byte(VALUE klass, VALUE vfamily, VALUE vlevel, VALUE voptname, VALUE vint)
{
int family = rsock_family_arg(vfamily);
int level = rsock_level_arg(family, vlevel);
int optname = rsock_optname_arg(family, level, voptname);
return rsock_sockopt_new(family, level, optname, sockopt_pack_byte(vint));
}
static VALUE
sockopt_byte(VALUE self)
{
VALUE data = sockopt_data(self);
StringValue(data);
check_size(RSTRING_LEN(data), sizeof(char));
return CHR2FIX(*RSTRING_PTR(data));
}
static VALUE
sockopt_s_int(VALUE klass, VALUE vfamily, VALUE vlevel, VALUE voptname, VALUE vint)
{
int family = rsock_family_arg(vfamily);
int level = rsock_level_arg(family, vlevel);
int optname = rsock_optname_arg(family, level, voptname);
return rsock_sockopt_new(family, level, optname, sockopt_pack_int(vint));
}
static VALUE
sockopt_int(VALUE self)
{
int i;
VALUE data = sockopt_data(self);
StringValue(data);
check_size(RSTRING_LEN(data), sizeof(int));
memcpy((char*)&i, RSTRING_PTR(data), sizeof(int));
return INT2NUM(i);
}
static VALUE
sockopt_s_bool(VALUE klass, VALUE vfamily, VALUE vlevel, VALUE voptname, VALUE vbool)
{
int family = rsock_family_arg(vfamily);
int level = rsock_level_arg(family, vlevel);
int optname = rsock_optname_arg(family, level, voptname);
int i = RTEST(vbool) ? 1 : 0;
return rsock_sockopt_new(family, level, optname, pack_var(i));
}
static VALUE
sockopt_bool(VALUE self)
{
int i;
long len;
VALUE data = sockopt_data(self);
StringValue(data);
len = RSTRING_LEN(data);
if (len == 1) {
return *RSTRING_PTR(data) == 0 ? Qfalse : Qtrue;
}
check_size(len, sizeof(int));
memcpy((char*)&i, RSTRING_PTR(data), len);
return i == 0 ? Qfalse : Qtrue;
}
static VALUE
sockopt_s_linger(VALUE klass, VALUE vonoff, VALUE vsecs)
{
VALUE tmp;
struct linger l;
memset(&l, 0, sizeof(l));
if (!NIL_P(tmp = rb_check_to_integer(vonoff, "to_int")))
l.l_onoff = NUM2INT(tmp);
else
l.l_onoff = RTEST(vonoff) ? 1 : 0;
l.l_linger = NUM2INT(vsecs);
return rsock_sockopt_new(AF_UNSPEC, SOL_SOCKET, SO_LINGER, pack_var(l));
}
static VALUE
sockopt_linger(VALUE self)
{
int level = sockopt_level(self);
int optname = sockopt_optname(self);
VALUE data = sockopt_data(self);
struct linger l;
VALUE vonoff, vsecs;
if (level != SOL_SOCKET || optname != SO_LINGER)
rb_raise(rb_eTypeError, "linger socket option expected");
check_size(RSTRING_LEN(data), sizeof(struct linger));
memcpy((char*)&l, RSTRING_PTR(data), sizeof(struct linger));
switch (l.l_onoff) {
case 0: vonoff = Qfalse; break;
case 1: vonoff = Qtrue; break;
default: vonoff = INT2NUM(l.l_onoff); break;
}
vsecs = INT2NUM(l.l_linger);
return rb_assoc_new(vonoff, vsecs);
}
static VALUE
sockopt_s_ipv4_multicast_loop(VALUE klass, VALUE value)
{
#if defined(IPPROTO_IP) && defined(IP_MULTICAST_LOOP)
VALUE o = XCAT(sockopt_pack_,TYPE_IP_MULTICAST_LOOP)(value);
return rsock_sockopt_new(AF_INET, IPPROTO_IP, IP_MULTICAST_LOOP, o);
#else
#error IPPROTO_IP or IP_MULTICAST_LOOP is not implemented
#endif
}
static VALUE
sockopt_ipv4_multicast_loop(VALUE self)
{
int family = NUM2INT(sockopt_family_m(self));
int level = sockopt_level(self);
int optname = sockopt_optname(self);
#if defined(IPPROTO_IP) && defined(IP_MULTICAST_LOOP)
if (family == AF_INET && level == IPPROTO_IP && optname == IP_MULTICAST_LOOP) {
return XCAT(sockopt_,TYPE_IP_MULTICAST_LOOP)(self);
}
#endif
rb_raise(rb_eTypeError, "ipv4_multicast_loop socket option expected");
UNREACHABLE_RETURN(Qnil);
}
#define inspect_ipv4_multicast_loop(a,b,c,d) XCAT(inspect_,TYPE_IP_MULTICAST_LOOP)(a,b,c,d)
static VALUE
sockopt_s_ipv4_multicast_ttl(VALUE klass, VALUE value)
{
#if defined(IPPROTO_IP) && defined(IP_MULTICAST_TTL)
VALUE o = XCAT(sockopt_pack_,TYPE_IP_MULTICAST_TTL)(value);
return rsock_sockopt_new(AF_INET, IPPROTO_IP, IP_MULTICAST_TTL, o);
#else
#error IPPROTO_IP or IP_MULTICAST_TTL is not implemented
#endif
}
static VALUE
sockopt_ipv4_multicast_ttl(VALUE self)
{
int family = NUM2INT(sockopt_family_m(self));
int level = sockopt_level(self);
int optname = sockopt_optname(self);
#if defined(IPPROTO_IP) && defined(IP_MULTICAST_TTL)
if (family == AF_INET && level == IPPROTO_IP && optname == IP_MULTICAST_TTL) {
return XCAT(sockopt_,TYPE_IP_MULTICAST_TTL)(self);
}
#endif
rb_raise(rb_eTypeError, "ipv4_multicast_ttl socket option expected");
UNREACHABLE_RETURN(Qnil);
}
#define inspect_ipv4_multicast_ttl(a,b,c,d) XCAT(inspect_,TYPE_IP_MULTICAST_TTL)(a,b,c,d)
static int
inspect_int(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(int)) {
int i;
memcpy((char*)&i, RSTRING_PTR(data), sizeof(int));
rb_str_catf(ret, " %d", i);
return 1;
}
else {
return 0;
}
}
#if defined(USE_INSPECT_BYTE)
static int
inspect_byte(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(unsigned char)) {
rb_str_catf(ret, " %d", (unsigned char)*RSTRING_PTR(data));
return 1;
}
else {
return 0;
}
}
#endif
static int
inspect_errno(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(int)) {
int i;
char *err;
memcpy((char*)&i, RSTRING_PTR(data), sizeof(int));
err = strerror(i);
rb_str_catf(ret, " %s (%d)", err, i);
return 1;
}
else {
return 0;
}
}
#if defined(IPV6_MULTICAST_LOOP)
static int
inspect_uint(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(int)) {
unsigned int i;
memcpy((char*)&i, RSTRING_PTR(data), sizeof(unsigned int));
rb_str_catf(ret, " %u", i);
return 1;
}
else {
return 0;
}
}
#endif
#if defined(SOL_SOCKET) && defined(SO_LINGER) 
static int
inspect_linger(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(struct linger)) {
struct linger s;
memcpy((char*)&s, RSTRING_PTR(data), sizeof(s));
switch (s.l_onoff) {
case 0: rb_str_cat2(ret, " off"); break;
case 1: rb_str_cat2(ret, " on"); break;
default: rb_str_catf(ret, " on(%d)", s.l_onoff); break;
}
rb_str_catf(ret, " %dsec", s.l_linger);
return 1;
}
else {
return 0;
}
}
#endif
#if defined(SOL_SOCKET) && defined(SO_TYPE) 
static int
inspect_socktype(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(int)) {
int i;
ID id;
memcpy((char*)&i, RSTRING_PTR(data), sizeof(int));
id = rsock_intern_socktype(i);
if (id)
rb_str_catf(ret, " %s", rb_id2name(id));
else
rb_str_catf(ret, " %d", i);
return 1;
}
else {
return 0;
}
}
#endif
static int
inspect_timeval_as_interval(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(struct timeval)) {
struct timeval s;
memcpy((char*)&s, RSTRING_PTR(data), sizeof(s));
rb_str_catf(ret, " %ld.%06ldsec", (long)s.tv_sec, (long)s.tv_usec);
return 1;
}
else {
return 0;
}
}
#if !defined HAVE_INET_NTOP && ! defined _WIN32
const char *
inet_ntop(int af, const void *addr, char *numaddr, size_t numaddr_len)
{
#if defined(HAVE_INET_NTOA)
struct in_addr in;
memcpy(&in.s_addr, addr, sizeof(in.s_addr));
snprintf(numaddr, numaddr_len, "%s", inet_ntoa(in));
#else
unsigned long x = ntohl(*(unsigned long*)addr);
snprintf(numaddr, numaddr_len, "%d.%d.%d.%d",
(int) (x>>24) & 0xff, (int) (x>>16) & 0xff,
(int) (x>> 8) & 0xff, (int) (x>> 0) & 0xff);
#endif
return numaddr;
}
#endif
static int
rb_if_indextoname(const char *succ_prefix, const char *fail_prefix, unsigned int ifindex, char *buf, size_t len)
{
#if defined(HAVE_IF_INDEXTONAME)
char ifbuf[IFNAMSIZ];
if (if_indextoname(ifindex, ifbuf) == NULL)
return snprintf(buf, len, "%s%u", fail_prefix, ifindex);
else
return snprintf(buf, len, "%s%s", succ_prefix, ifbuf);
#else
#if !defined(IFNAMSIZ)
#define IFNAMSIZ (sizeof(unsigned int)*3+1)
#endif
return snprintf(buf, len, "%s%u", fail_prefix, ifindex);
#endif
}
#if defined(IPPROTO_IP) && defined(HAVE_TYPE_STRUCT_IP_MREQ) 
static int
inspect_ipv4_mreq(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(struct ip_mreq)) {
struct ip_mreq s;
char addrbuf[INET_ADDRSTRLEN];
memcpy((char*)&s, RSTRING_PTR(data), sizeof(s));
if (inet_ntop(AF_INET, &s.imr_multiaddr, addrbuf, (socklen_t)sizeof(addrbuf)) == NULL)
rb_str_cat2(ret, " invalid-address");
else
rb_str_catf(ret, " %s", addrbuf);
if (inet_ntop(AF_INET, &s.imr_interface, addrbuf, (socklen_t)sizeof(addrbuf)) == NULL)
rb_str_catf(ret, " invalid-address");
else
rb_str_catf(ret, " %s", addrbuf);
return 1;
}
else {
return 0;
}
}
#endif
#if defined(IPPROTO_IP) && defined(HAVE_TYPE_STRUCT_IP_MREQN) 
static int
inspect_ipv4_mreqn(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(struct ip_mreqn)) {
struct ip_mreqn s;
char addrbuf[INET_ADDRSTRLEN], ifbuf[32+IFNAMSIZ];
memcpy((char*)&s, RSTRING_PTR(data), sizeof(s));
if (inet_ntop(AF_INET, &s.imr_multiaddr, addrbuf, (socklen_t)sizeof(addrbuf)) == NULL)
rb_str_cat2(ret, " invalid-address");
else
rb_str_catf(ret, " %s", addrbuf);
if (inet_ntop(AF_INET, &s.imr_address, addrbuf, (socklen_t)sizeof(addrbuf)) == NULL)
rb_str_catf(ret, " invalid-address");
else
rb_str_catf(ret, " %s", addrbuf);
rb_if_indextoname(" ", " ifindex:", s.imr_ifindex, ifbuf, sizeof(ifbuf));
rb_str_cat2(ret, ifbuf);
return 1;
}
else {
return 0;
}
}
#endif
#if defined(IPPROTO_IP) && defined(HAVE_TYPE_STRUCT_IP_MREQ) 
static int
inspect_ipv4_add_drop_membership(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(struct ip_mreq))
return inspect_ipv4_mreq(level, optname, data, ret);
#if defined(HAVE_TYPE_STRUCT_IP_MREQN)
else if (RSTRING_LEN(data) == sizeof(struct ip_mreqn))
return inspect_ipv4_mreqn(level, optname, data, ret);
#endif
else
return 0;
}
#endif
#if defined(IPPROTO_IP) && defined(IP_MULTICAST_IF) && defined(HAVE_TYPE_STRUCT_IP_MREQN) 
static int
inspect_ipv4_multicast_if(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(struct in_addr)) {
struct in_addr s;
char addrbuf[INET_ADDRSTRLEN];
memcpy((char*)&s, RSTRING_PTR(data), sizeof(s));
if (inet_ntop(AF_INET, &s, addrbuf, (socklen_t)sizeof(addrbuf)) == NULL)
rb_str_cat2(ret, " invalid-address");
else
rb_str_catf(ret, " %s", addrbuf);
return 1;
}
else if (RSTRING_LEN(data) == sizeof(struct ip_mreqn)) {
return inspect_ipv4_mreqn(level, optname, data, ret);
}
else {
return 0;
}
}
#endif
#if defined(IPV6_MULTICAST_IF) 
static int
inspect_ipv6_multicast_if(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(int)) {
char ifbuf[32+IFNAMSIZ];
unsigned int ifindex;
memcpy((char*)&ifindex, RSTRING_PTR(data), sizeof(unsigned int));
rb_if_indextoname(" ", " ", ifindex, ifbuf, sizeof(ifbuf));
rb_str_cat2(ret, ifbuf);
return 1;
}
else {
return 0;
}
}
#endif
#if defined(IPPROTO_IPV6) && defined(HAVE_TYPE_STRUCT_IPV6_MREQ) 
static int
inspect_ipv6_mreq(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(struct ipv6_mreq)) {
struct ipv6_mreq s;
char addrbuf[INET6_ADDRSTRLEN], ifbuf[32+IFNAMSIZ];
memcpy((char*)&s, RSTRING_PTR(data), sizeof(s));
if (inet_ntop(AF_INET6, &s.ipv6mr_multiaddr, addrbuf, (socklen_t)sizeof(addrbuf)) == NULL)
rb_str_cat2(ret, " invalid-address");
else
rb_str_catf(ret, " %s", addrbuf);
rb_if_indextoname(" ", " interface:", s.ipv6mr_interface, ifbuf, sizeof(ifbuf));
rb_str_cat2(ret, ifbuf);
return 1;
}
else {
return 0;
}
}
#endif
#if defined(IPPROTO_TCP) && defined(TCP_INFO) && defined(HAVE_TYPE_STRUCT_TCP_INFO)
#if defined(__FreeBSD__)
#if !defined(HAVE_CONST_TCP_ESTABLISHED)
#define TCP_ESTABLISHED TCPS_ESTABLISHED
#endif
#if !defined(HAVE_CONST_TCP_SYN_SENT)
#define TCP_SYN_SENT TCPS_SYN_SENT
#endif
#if !defined(HAVE_CONST_TCP_SYN_RECV)
#define TCP_SYN_RECV TCPS_SYN_RECEIVED
#endif
#if !defined(HAVE_CONST_TCP_FIN_WAIT1)
#define TCP_FIN_WAIT1 TCPS_FIN_WAIT_1
#endif
#if !defined(HAVE_CONST_TCP_FIN_WAIT2)
#define TCP_FIN_WAIT2 TCPS_FIN_WAIT_2
#endif
#if !defined(HAVE_CONST_TCP_TIME_WAIT)
#define TCP_TIME_WAIT TCPS_TIME_WAIT
#endif
#if !defined(HAVE_CONST_TCP_CLOSE)
#define TCP_CLOSE TCPS_CLOSED
#endif
#if !defined(HAVE_CONST_TCP_CLOSE_WAIT)
#define TCP_CLOSE_WAIT TCPS_CLOSE_WAIT
#endif
#if !defined(HAVE_CONST_TCP_LAST_ACK)
#define TCP_LAST_ACK TCPS_LAST_ACK
#endif
#if !defined(HAVE_CONST_TCP_LISTEN)
#define TCP_LISTEN TCPS_LISTEN
#endif
#if !defined(HAVE_CONST_TCP_CLOSING)
#define TCP_CLOSING TCPS_CLOSING
#endif
#endif
#if defined(HAVE_CONST_TCP_ESTABLISHED) && !defined(TCP_ESTABLISHED)
#define TCP_ESTABLISHED TCP_ESTABLISHED
#endif
#if defined(HAVE_CONST_TCP_SYN_SENT) && !defined(TCP_SYN_SENT)
#define TCP_SYN_SENT TCP_SYN_SENT
#endif
#if defined(HAVE_CONST_TCP_SYN_RECV) && !defined(TCP_SYN_RECV)
#define TCP_SYN_RECV TCP_SYN_RECV
#endif
#if defined(HAVE_CONST_TCP_FIN_WAIT1) && !defined(TCP_FIN_WAIT1)
#define TCP_FIN_WAIT1 TCP_FIN_WAIT1
#endif
#if defined(HAVE_CONST_TCP_FIN_WAIT2) && !defined(TCP_FIN_WAIT2)
#define TCP_FIN_WAIT2 TCP_FIN_WAIT2
#endif
#if defined(HAVE_CONST_TCP_TIME_WAIT) && !defined(TCP_TIME_WAIT)
#define TCP_TIME_WAIT TCP_TIME_WAIT
#endif
#if defined(HAVE_CONST_TCP_CLOSE) && !defined(TCP_CLOSE)
#define TCP_CLOSE TCP_CLOSE
#endif
#if defined(HAVE_CONST_TCP_CLOSE_WAIT) && !defined(TCP_CLOSE_WAIT)
#define TCP_CLOSE_WAIT TCP_CLOSE_WAIT
#endif
#if defined(HAVE_CONST_TCP_LAST_ACK) && !defined(TCP_LAST_ACK)
#define TCP_LAST_ACK TCP_LAST_ACK
#endif
#if defined(HAVE_CONST_TCP_LISTEN) && !defined(TCP_LISTEN)
#define TCP_LISTEN TCP_LISTEN
#endif
#if defined(HAVE_CONST_TCP_CLOSING) && !defined(TCP_CLOSING)
#define TCP_CLOSING TCP_CLOSING
#endif
static void
inspect_tcpi_options(VALUE ret, uint8_t options)
{
int sep = '=';
rb_str_cat2(ret, " options");
#define INSPECT_TCPI_OPTION(optval, name) if (options & (optval)) { options &= ~(uint8_t)(optval); rb_str_catf(ret, "%c%s", sep, name); sep = ','; }
#if defined(TCPI_OPT_TIMESTAMPS)
INSPECT_TCPI_OPTION(TCPI_OPT_TIMESTAMPS, "TIMESTAMPS");
#endif
#if defined(TCPI_OPT_SACK)
INSPECT_TCPI_OPTION(TCPI_OPT_SACK, "SACK");
#endif
#if defined(TCPI_OPT_WSCALE)
INSPECT_TCPI_OPTION(TCPI_OPT_WSCALE, "WSCALE");
#endif
#if defined(TCPI_OPT_ECN)
INSPECT_TCPI_OPTION(TCPI_OPT_ECN, "ECN");
#endif
#if defined(TCPI_OPT_ECN_SEEN)
INSPECT_TCPI_OPTION(TCPI_OPT_ECN_SEEN, "ECN_SEEN");
#endif
#if defined(TCPI_OPT_SYN_DATA)
INSPECT_TCPI_OPTION(TCPI_OPT_SYN_DATA, "SYN_DATA");
#endif
#if defined(TCPI_OPT_TOE)
INSPECT_TCPI_OPTION(TCPI_OPT_TOE, "TOE");
#endif
#undef INSPECT_TCPI_OPTION
if (options || sep == '=') {
rb_str_catf(ret, "%c%u", sep, options);
}
}
static void
inspect_tcpi_usec(VALUE ret, const char *prefix, uint32_t t)
{
rb_str_catf(ret, "%s%u.%06us", prefix, t / 1000000, t % 1000000);
}
#if !defined __FreeBSD__ && ( defined HAVE_STRUCT_TCP_INFO_TCPI_LAST_DATA_SENT || defined HAVE_STRUCT_TCP_INFO_TCPI_LAST_DATA_RECV || defined HAVE_STRUCT_TCP_INFO_TCPI_LAST_ACK_SENT || defined HAVE_STRUCT_TCP_INFO_TCPI_LAST_ACK_RECV || 0)
static void
inspect_tcpi_msec(VALUE ret, const char *prefix, uint32_t t)
{
rb_str_catf(ret, "%s%u.%03us", prefix, t / 1000, t % 1000);
}
#endif
#if defined(__FreeBSD__)
#define inspect_tcpi_rto(ret, t) inspect_tcpi_usec(ret, " rto=", t)
#define inspect_tcpi_last_data_recv(ret, t) inspect_tcpi_usec(ret, " last_data_recv=", t)
#define inspect_tcpi_rtt(ret, t) inspect_tcpi_usec(ret, " rtt=", t)
#define inspect_tcpi_rttvar(ret, t) inspect_tcpi_usec(ret, " rttvar=", t)
#else
#define inspect_tcpi_rto(ret, t) inspect_tcpi_usec(ret, " rto=", t)
#define inspect_tcpi_ato(ret, t) inspect_tcpi_usec(ret, " ato=", t)
#define inspect_tcpi_last_data_sent(ret, t) inspect_tcpi_msec(ret, " last_data_sent=", t)
#define inspect_tcpi_last_data_recv(ret, t) inspect_tcpi_msec(ret, " last_data_recv=", t)
#define inspect_tcpi_last_ack_sent(ret, t) inspect_tcpi_msec(ret, " last_ack_sent=", t)
#define inspect_tcpi_last_ack_recv(ret, t) inspect_tcpi_msec(ret, " last_ack_recv=", t)
#define inspect_tcpi_rtt(ret, t) inspect_tcpi_usec(ret, " rtt=", t)
#define inspect_tcpi_rttvar(ret, t) inspect_tcpi_usec(ret, " rttvar=", t)
#define inspect_tcpi_rcv_rtt(ret, t) inspect_tcpi_usec(ret, " rcv_rtt=", t)
#endif
static int
inspect_tcp_info(int level, int optname, VALUE data, VALUE ret)
{
size_t actual_size = RSTRING_LEN(data);
if (sizeof(struct tcp_info) <= actual_size) {
struct tcp_info s;
memcpy((char*)&s, RSTRING_PTR(data), sizeof(s));
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_STATE)
switch (s.tcpi_state) {
#if defined(TCP_ESTABLISHED)
case TCP_ESTABLISHED: rb_str_cat_cstr(ret, " state=ESTABLISHED"); break;
#endif
#if defined(TCP_SYN_SENT)
case TCP_SYN_SENT: rb_str_cat_cstr(ret, " state=SYN_SENT"); break;
#endif
#if defined(TCP_SYN_RECV)
case TCP_SYN_RECV: rb_str_cat_cstr(ret, " state=SYN_RECV"); break;
#endif
#if defined(TCP_FIN_WAIT1)
case TCP_FIN_WAIT1: rb_str_cat_cstr(ret, " state=FIN_WAIT1"); break;
#endif
#if defined(TCP_FIN_WAIT2)
case TCP_FIN_WAIT2: rb_str_cat_cstr(ret, " state=FIN_WAIT2"); break;
#endif
#if defined(TCP_TIME_WAIT)
case TCP_TIME_WAIT: rb_str_cat_cstr(ret, " state=TIME_WAIT"); break;
#endif
#if defined(TCP_CLOSE)
case TCP_CLOSE: rb_str_cat_cstr(ret, " state=CLOSED"); break; 
#endif
#if defined(TCP_CLOSE_WAIT)
case TCP_CLOSE_WAIT: rb_str_cat_cstr(ret, " state=CLOSE_WAIT"); break;
#endif
#if defined(TCP_LAST_ACK)
case TCP_LAST_ACK: rb_str_cat_cstr(ret, " state=LAST_ACK"); break;
#endif
#if defined(TCP_LISTEN)
case TCP_LISTEN: rb_str_cat_cstr(ret, " state=LISTEN"); break;
#endif
#if defined(TCP_CLOSING)
case TCP_CLOSING: rb_str_cat_cstr(ret, " state=CLOSING"); break;
#endif
default: rb_str_catf(ret, " state=%u", s.tcpi_state); break;
}
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_CA_STATE)
switch (s.tcpi_ca_state) {
case TCP_CA_Open: rb_str_cat_cstr(ret, " ca_state=Open"); break;
case TCP_CA_Disorder: rb_str_cat_cstr(ret, " ca_state=Disorder"); break;
case TCP_CA_CWR: rb_str_cat_cstr(ret, " ca_state=CWR"); break;
case TCP_CA_Recovery: rb_str_cat_cstr(ret, " ca_state=Recovery"); break;
case TCP_CA_Loss: rb_str_cat_cstr(ret, " ca_state=Loss"); break;
default: rb_str_catf(ret, " ca_state=%u", s.tcpi_ca_state); break;
}
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_RETRANSMITS)
rb_str_catf(ret, " retransmits=%u", s.tcpi_retransmits);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_PROBES)
rb_str_catf(ret, " probes=%u", s.tcpi_probes);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_BACKOFF)
rb_str_catf(ret, " backoff=%u", s.tcpi_backoff);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_OPTIONS)
inspect_tcpi_options(ret, s.tcpi_options);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_SND_WSCALE)
rb_str_catf(ret, " snd_wscale=%u", s.tcpi_snd_wscale);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_RCV_WSCALE)
rb_str_catf(ret, " rcv_wscale=%u", s.tcpi_rcv_wscale);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_RTO)
inspect_tcpi_rto(ret, s.tcpi_rto);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_ATO)
inspect_tcpi_ato(ret, s.tcpi_ato);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_SND_MSS)
rb_str_catf(ret, " snd_mss=%u", s.tcpi_snd_mss);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_RCV_MSS)
rb_str_catf(ret, " rcv_mss=%u", s.tcpi_rcv_mss);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_UNACKED)
rb_str_catf(ret, " unacked=%u", s.tcpi_unacked);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_SACKED)
rb_str_catf(ret, " sacked=%u", s.tcpi_sacked);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_LOST)
rb_str_catf(ret, " lost=%u", s.tcpi_lost);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_RETRANS)
rb_str_catf(ret, " retrans=%u", s.tcpi_retrans);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_FACKETS)
rb_str_catf(ret, " fackets=%u", s.tcpi_fackets);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_LAST_DATA_SENT)
inspect_tcpi_last_data_sent(ret, s.tcpi_last_data_sent);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_LAST_ACK_SENT)
inspect_tcpi_last_ack_sent(ret, s.tcpi_last_ack_sent);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_LAST_DATA_RECV)
inspect_tcpi_last_data_recv(ret, s.tcpi_last_data_recv);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_LAST_ACK_RECV)
inspect_tcpi_last_ack_recv(ret, s.tcpi_last_ack_recv);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_PMTU)
rb_str_catf(ret, " pmtu=%u", s.tcpi_pmtu);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_RCV_SSTHRESH)
rb_str_catf(ret, " rcv_ssthresh=%u", s.tcpi_rcv_ssthresh);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_RTT)
inspect_tcpi_rtt(ret, s.tcpi_rtt);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_RTTVAR)
inspect_tcpi_rttvar(ret, s.tcpi_rttvar);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_SND_SSTHRESH)
rb_str_catf(ret, " snd_ssthresh=%u", s.tcpi_snd_ssthresh);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_SND_CWND)
rb_str_catf(ret, " snd_cwnd=%u", s.tcpi_snd_cwnd);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_ADVMSS)
rb_str_catf(ret, " advmss=%u", s.tcpi_advmss);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_REORDERING)
rb_str_catf(ret, " reordering=%u", s.tcpi_reordering);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_RCV_RTT)
inspect_tcpi_rcv_rtt(ret, s.tcpi_rcv_rtt);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_RCV_SPACE)
rb_str_catf(ret, " rcv_space=%u", s.tcpi_rcv_space);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_TOTAL_RETRANS)
rb_str_catf(ret, " total_retrans=%u", s.tcpi_total_retrans);
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_SND_WND)
rb_str_catf(ret, " snd_wnd=%u", s.tcpi_snd_wnd); 
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_SND_BWND)
rb_str_catf(ret, " snd_bwnd=%u", s.tcpi_snd_bwnd); 
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_SND_NXT)
rb_str_catf(ret, " snd_nxt=%u", s.tcpi_snd_nxt); 
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_RCV_NXT)
rb_str_catf(ret, " rcv_nxt=%u", s.tcpi_rcv_nxt); 
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_TOE_TID)
rb_str_catf(ret, " toe_tid=%u", s.tcpi_toe_tid); 
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_SND_REXMITPACK)
rb_str_catf(ret, " snd_rexmitpack=%u", s.tcpi_snd_rexmitpack); 
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_RCV_OOOPACK)
rb_str_catf(ret, " rcv_ooopack=%u", s.tcpi_rcv_ooopack); 
#endif
#if defined(HAVE_STRUCT_TCP_INFO_TCPI_SND_ZEROWIN)
rb_str_catf(ret, " snd_zerowin=%u", s.tcpi_snd_zerowin); 
#endif
if (sizeof(struct tcp_info) < actual_size)
rb_str_catf(ret, " (%u bytes too long)", (unsigned)(actual_size - sizeof(struct tcp_info)));
return 1;
}
else {
return 0;
}
}
#endif
#if defined(SOL_SOCKET) && defined(SO_PEERCRED) 
#if defined(__OpenBSD__)
#define RUBY_SOCK_PEERCRED struct sockpeercred
#else
#define RUBY_SOCK_PEERCRED struct ucred
#endif
static int
inspect_peercred(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(RUBY_SOCK_PEERCRED)) {
RUBY_SOCK_PEERCRED cred;
memcpy(&cred, RSTRING_PTR(data), sizeof(RUBY_SOCK_PEERCRED));
rb_str_catf(ret, " pid=%u euid=%u egid=%u",
(unsigned)cred.pid, (unsigned)cred.uid, (unsigned)cred.gid);
rb_str_cat2(ret, " (ucred)");
return 1;
}
else {
return 0;
}
}
#endif
#if defined(LOCAL_PEERCRED) 
static int
inspect_local_peercred(int level, int optname, VALUE data, VALUE ret)
{
if (RSTRING_LEN(data) == sizeof(struct xucred)) {
struct xucred cred;
memcpy(&cred, RSTRING_PTR(data), sizeof(struct xucred));
if (cred.cr_version != XUCRED_VERSION)
return 0;
rb_str_catf(ret, " version=%u", cred.cr_version);
rb_str_catf(ret, " euid=%u", cred.cr_uid);
if (cred.cr_ngroups) {
int i;
const char *sep = " groups=";
for (i = 0; i < cred.cr_ngroups; i++) {
rb_str_catf(ret, "%s%u", sep, cred.cr_groups[i]);
sep = ",";
}
}
rb_str_cat2(ret, " (xucred)");
return 1;
}
else {
return 0;
}
}
#endif
static VALUE
sockopt_inspect(VALUE self)
{
int family = NUM2INT(sockopt_family_m(self));
int level = NUM2INT(sockopt_level_m(self));
int optname = NUM2INT(sockopt_optname_m(self));
VALUE data = sockopt_data(self);
VALUE v, ret;
ID family_id, level_id, optname_id;
int inspected;
StringValue(data);
ret = rb_sprintf("#<%s:", rb_obj_classname(self));
family_id = rsock_intern_family_noprefix(family);
if (family_id)
rb_str_catf(ret, " %s", rb_id2name(family_id));
else
rb_str_catf(ret, " family:%d", family);
if (level == SOL_SOCKET) {
rb_str_cat2(ret, " SOCKET");
optname_id = rsock_intern_so_optname(optname);
if (optname_id)
rb_str_catf(ret, " %s", rb_id2name(optname_id));
else
rb_str_catf(ret, " optname:%d", optname);
}
#if defined(HAVE_SYS_UN_H)
else if (family == AF_UNIX) {
rb_str_catf(ret, " level:%d", level);
optname_id = rsock_intern_local_optname(optname);
if (optname_id)
rb_str_catf(ret, " %s", rb_id2name(optname_id));
else
rb_str_catf(ret, " optname:%d", optname);
}
#endif
else if (IS_IP_FAMILY(family)) {
level_id = rsock_intern_iplevel(level);
if (level_id)
rb_str_catf(ret, " %s", rb_id2name(level_id));
else
rb_str_catf(ret, " level:%d", level);
v = optname_to_sym(level, optname);
if (SYMBOL_P(v))
rb_str_catf(ret, " %"PRIsVALUE, rb_sym2str(v));
else
rb_str_catf(ret, " optname:%d", optname);
}
else {
rb_str_catf(ret, " level:%d", level);
rb_str_catf(ret, " optname:%d", optname);
}
inspected = 0;
if (level == SOL_SOCKET)
family = AF_UNSPEC;
switch (family) {
case AF_UNSPEC:
switch (level) {
case SOL_SOCKET:
switch (optname) {
#if defined(SO_DEBUG) 
case SO_DEBUG: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(SO_ERROR) 
case SO_ERROR: inspected = inspect_errno(level, optname, data, ret); break;
#endif
#if defined(SO_TYPE) 
case SO_TYPE: inspected = inspect_socktype(level, optname, data, ret); break;
#endif
#if defined(SO_ACCEPTCONN) 
case SO_ACCEPTCONN: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(SO_BROADCAST) 
case SO_BROADCAST: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(SO_REUSEADDR) 
case SO_REUSEADDR: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(SO_KEEPALIVE) 
case SO_KEEPALIVE: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(SO_OOBINLINE) 
case SO_OOBINLINE: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(SO_SNDBUF) 
case SO_SNDBUF: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(SO_RCVBUF) 
case SO_RCVBUF: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(SO_DONTROUTE) 
case SO_DONTROUTE: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(SO_RCVLOWAT) 
case SO_RCVLOWAT: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(SO_SNDLOWAT) 
case SO_SNDLOWAT: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(SO_LINGER) 
case SO_LINGER: inspected = inspect_linger(level, optname, data, ret); break;
#endif
#if defined(SO_RCVTIMEO) 
case SO_RCVTIMEO: inspected = inspect_timeval_as_interval(level, optname, data, ret); break;
#endif
#if defined(SO_SNDTIMEO) 
case SO_SNDTIMEO: inspected = inspect_timeval_as_interval(level, optname, data, ret); break;
#endif
#if defined(SO_PEERCRED) 
case SO_PEERCRED: inspected = inspect_peercred(level, optname, data, ret); break;
#endif
}
break;
}
break;
case AF_INET:
#if defined(INET6)
case AF_INET6:
#endif
switch (level) {
#if defined(IPPROTO_IP)
case IPPROTO_IP:
switch (optname) {
#if defined(IP_MULTICAST_IF) && defined(HAVE_TYPE_STRUCT_IP_MREQN) 
case IP_MULTICAST_IF: inspected = inspect_ipv4_multicast_if(level, optname, data, ret); break;
#endif
#if defined(IP_ADD_MEMBERSHIP) 
case IP_ADD_MEMBERSHIP: inspected = inspect_ipv4_add_drop_membership(level, optname, data, ret); break;
#endif
#if defined(IP_DROP_MEMBERSHIP) 
case IP_DROP_MEMBERSHIP: inspected = inspect_ipv4_add_drop_membership(level, optname, data, ret); break;
#endif
#if defined(IP_MULTICAST_LOOP) 
case IP_MULTICAST_LOOP: inspected = inspect_ipv4_multicast_loop(level, optname, data, ret); break;
#endif
#if defined(IP_MULTICAST_TTL) 
case IP_MULTICAST_TTL: inspected = inspect_ipv4_multicast_ttl(level, optname, data, ret); break;
#endif
}
break;
#endif
#if defined(IPPROTO_IPV6)
case IPPROTO_IPV6:
switch (optname) {
#if defined(IPV6_MULTICAST_HOPS) 
case IPV6_MULTICAST_HOPS: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(IPV6_MULTICAST_IF) 
case IPV6_MULTICAST_IF: inspected = inspect_ipv6_multicast_if(level, optname, data, ret); break;
#endif
#if defined(IPV6_MULTICAST_LOOP) 
case IPV6_MULTICAST_LOOP: inspected = inspect_uint(level, optname, data, ret); break;
#endif
#if defined(IPV6_JOIN_GROUP) 
case IPV6_JOIN_GROUP: inspected = inspect_ipv6_mreq(level, optname, data, ret); break;
#endif
#if defined(IPV6_LEAVE_GROUP) 
case IPV6_LEAVE_GROUP: inspected = inspect_ipv6_mreq(level, optname, data, ret); break;
#endif
#if defined(IPV6_UNICAST_HOPS) 
case IPV6_UNICAST_HOPS: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(IPV6_V6ONLY) 
case IPV6_V6ONLY: inspected = inspect_int(level, optname, data, ret); break;
#endif
}
break;
#endif
#if defined(IPPROTO_TCP)
case IPPROTO_TCP:
switch (optname) {
#if defined(TCP_NODELAY) 
case TCP_NODELAY: inspected = inspect_int(level, optname, data, ret); break;
#endif
#if defined(TCP_INFO) && defined(HAVE_TYPE_STRUCT_TCP_INFO) 
case TCP_INFO: inspected = inspect_tcp_info(level, optname, data, ret); break;
#endif
}
break;
#endif
}
break;
#if defined(HAVE_SYS_UN_H)
case AF_UNIX:
switch (level) {
case 0:
switch (optname) {
#if defined(LOCAL_PEERCRED)
case LOCAL_PEERCRED: inspected = inspect_local_peercred(level, optname, data, ret); break;
#endif
}
break;
}
break;
#endif
}
if (!inspected) {
rb_str_cat2(ret, " ");
rb_str_append(ret, rb_str_dump(data));
}
rb_str_cat2(ret, ">");
return ret;
}
static VALUE
sockopt_unpack(VALUE self, VALUE template)
{
return rb_funcall(sockopt_data(self), rb_intern("unpack"), 1, template);
}
void
rsock_init_sockopt(void)
{
rb_cSockOpt = rb_define_class_under(rb_cSocket, "Option", rb_cObject);
rb_define_method(rb_cSockOpt, "initialize", sockopt_initialize, 4);
rb_define_method(rb_cSockOpt, "family", sockopt_family_m, 0);
rb_define_method(rb_cSockOpt, "level", sockopt_level_m, 0);
rb_define_method(rb_cSockOpt, "optname", sockopt_optname_m, 0);
rb_define_method(rb_cSockOpt, "data", sockopt_data, 0);
rb_define_method(rb_cSockOpt, "inspect", sockopt_inspect, 0);
rb_define_singleton_method(rb_cSockOpt, "int", sockopt_s_int, 4);
rb_define_method(rb_cSockOpt, "int", sockopt_int, 0);
rb_define_singleton_method(rb_cSockOpt, "byte", sockopt_s_byte, 4);
rb_define_method(rb_cSockOpt, "byte", sockopt_byte, 0);
rb_define_singleton_method(rb_cSockOpt, "bool", sockopt_s_bool, 4);
rb_define_method(rb_cSockOpt, "bool", sockopt_bool, 0);
rb_define_singleton_method(rb_cSockOpt, "linger", sockopt_s_linger, 2);
rb_define_method(rb_cSockOpt, "linger", sockopt_linger, 0);
rb_define_singleton_method(rb_cSockOpt, "ipv4_multicast_ttl", sockopt_s_ipv4_multicast_ttl, 1);
rb_define_method(rb_cSockOpt, "ipv4_multicast_ttl", sockopt_ipv4_multicast_ttl, 0);
rb_define_singleton_method(rb_cSockOpt, "ipv4_multicast_loop", sockopt_s_ipv4_multicast_loop, 1);
rb_define_method(rb_cSockOpt, "ipv4_multicast_loop", sockopt_ipv4_multicast_loop, 0);
rb_define_method(rb_cSockOpt, "unpack", sockopt_unpack, 1);
rb_define_method(rb_cSockOpt, "to_s", sockopt_data, 0); 
}
