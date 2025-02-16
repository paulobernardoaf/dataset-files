#include "curl_setup.h"
#if !defined(CURL_DISABLE_POP3)
#if defined(HAVE_NETINET_IN_H)
#include <netinet/in.h>
#endif
#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h>
#endif
#if defined(HAVE_UTSNAME_H)
#include <sys/utsname.h>
#endif
#if defined(HAVE_NETDB_H)
#include <netdb.h>
#endif
#if defined(__VMS)
#include <in.h>
#include <inet.h>
#endif
#if (defined(NETWARE) && defined(__NOVELL_LIBC__))
#undef in_addr_t
#define in_addr_t unsigned long
#endif
#include <curl/curl.h>
#include "urldata.h"
#include "sendf.h"
#include "hostip.h"
#include "progress.h"
#include "transfer.h"
#include "escape.h"
#include "http.h" 
#include "socks.h"
#include "pop3.h"
#include "strtoofft.h"
#include "strcase.h"
#include "vtls/vtls.h"
#include "connect.h"
#include "strerror.h"
#include "select.h"
#include "multiif.h"
#include "url.h"
#include "curl_sasl.h"
#include "curl_md5.h"
#include "warnless.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
static CURLcode pop3_regular_transfer(struct connectdata *conn, bool *done);
static CURLcode pop3_do(struct connectdata *conn, bool *done);
static CURLcode pop3_done(struct connectdata *conn, CURLcode status,
bool premature);
static CURLcode pop3_connect(struct connectdata *conn, bool *done);
static CURLcode pop3_disconnect(struct connectdata *conn, bool dead);
static CURLcode pop3_multi_statemach(struct connectdata *conn, bool *done);
static int pop3_getsock(struct connectdata *conn, curl_socket_t *socks);
static CURLcode pop3_doing(struct connectdata *conn, bool *dophase_done);
static CURLcode pop3_setup_connection(struct connectdata *conn);
static CURLcode pop3_parse_url_options(struct connectdata *conn);
static CURLcode pop3_parse_url_path(struct connectdata *conn);
static CURLcode pop3_parse_custom_request(struct connectdata *conn);
static CURLcode pop3_perform_auth(struct connectdata *conn, const char *mech,
const char *initresp);
static CURLcode pop3_continue_auth(struct connectdata *conn, const char *resp);
static void pop3_get_message(char *buffer, char **outptr);
const struct Curl_handler Curl_handler_pop3 = {
"POP3", 
pop3_setup_connection, 
pop3_do, 
pop3_done, 
ZERO_NULL, 
pop3_connect, 
pop3_multi_statemach, 
pop3_doing, 
pop3_getsock, 
pop3_getsock, 
ZERO_NULL, 
ZERO_NULL, 
pop3_disconnect, 
ZERO_NULL, 
ZERO_NULL, 
PORT_POP3, 
CURLPROTO_POP3, 
PROTOPT_CLOSEACTION | PROTOPT_NOURLQUERY | 
PROTOPT_URLOPTIONS
};
#if defined(USE_SSL)
const struct Curl_handler Curl_handler_pop3s = {
"POP3S", 
pop3_setup_connection, 
pop3_do, 
pop3_done, 
ZERO_NULL, 
pop3_connect, 
pop3_multi_statemach, 
pop3_doing, 
pop3_getsock, 
pop3_getsock, 
ZERO_NULL, 
ZERO_NULL, 
pop3_disconnect, 
ZERO_NULL, 
ZERO_NULL, 
PORT_POP3S, 
CURLPROTO_POP3S, 
PROTOPT_CLOSEACTION | PROTOPT_SSL
| PROTOPT_NOURLQUERY | PROTOPT_URLOPTIONS 
};
#endif
static const struct SASLproto saslpop3 = {
"pop", 
'*', 
'+', 
255 - 8, 
pop3_perform_auth, 
pop3_continue_auth, 
pop3_get_message 
};
#if defined(USE_SSL)
static void pop3_to_pop3s(struct connectdata *conn)
{
conn->handler = &Curl_handler_pop3s;
conn->tls_upgraded = TRUE;
}
#else
#define pop3_to_pop3s(x) Curl_nop_stmt
#endif
static bool pop3_endofresp(struct connectdata *conn, char *line, size_t len,
int *resp)
{
struct pop3_conn *pop3c = &conn->proto.pop3c;
if(len >= 4 && !memcmp("-ERR", line, 4)) {
*resp = '-';
return TRUE;
}
if(pop3c->state == POP3_CAPA) {
if(len >= 1 && line[0] == '.')
*resp = '+';
else
*resp = '*';
return TRUE;
}
if(len >= 3 && !memcmp("+OK", line, 3)) {
*resp = '+';
return TRUE;
}
if(len >= 1 && line[0] == '+') {
*resp = '*';
return TRUE;
}
return FALSE; 
}
static void pop3_get_message(char *buffer, char **outptr)
{
size_t len = strlen(buffer);
char *message = NULL;
if(len > 2) {
len -= 2;
for(message = buffer + 2; *message == ' ' || *message == '\t';
message++, len--)
;
for(; len--;)
if(message[len] != '\r' && message[len] != '\n' && message[len] != ' ' &&
message[len] != '\t')
break;
if(++len) {
message[len] = '\0';
}
}
else
message = &buffer[len];
*outptr = message;
}
static void state(struct connectdata *conn, pop3state newstate)
{
struct pop3_conn *pop3c = &conn->proto.pop3c;
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
static const char * const names[] = {
"STOP",
"SERVERGREET",
"CAPA",
"STARTTLS",
"UPGRADETLS",
"AUTH",
"APOP",
"USER",
"PASS",
"COMMAND",
"QUIT",
};
if(pop3c->state != newstate)
infof(conn->data, "POP3 %p state change from %s to %s\n",
(void *)pop3c, names[pop3c->state], names[newstate]);
#endif
pop3c->state = newstate;
}
static CURLcode pop3_perform_capa(struct connectdata *conn)
{
CURLcode result = CURLE_OK;
struct pop3_conn *pop3c = &conn->proto.pop3c;
pop3c->sasl.authmechs = SASL_AUTH_NONE; 
pop3c->sasl.authused = SASL_AUTH_NONE; 
pop3c->tls_supported = FALSE; 
result = Curl_pp_sendf(&pop3c->pp, "%s", "CAPA");
if(!result)
state(conn, POP3_CAPA);
return result;
}
static CURLcode pop3_perform_starttls(struct connectdata *conn)
{
CURLcode result = Curl_pp_sendf(&conn->proto.pop3c.pp, "%s", "STLS");
if(!result)
state(conn, POP3_STARTTLS);
return result;
}
static CURLcode pop3_perform_upgrade_tls(struct connectdata *conn)
{
struct pop3_conn *pop3c = &conn->proto.pop3c;
CURLcode result = Curl_ssl_connect_nonblocking(conn, FIRSTSOCKET,
&pop3c->ssldone);
if(!result) {
if(pop3c->state != POP3_UPGRADETLS)
state(conn, POP3_UPGRADETLS);
if(pop3c->ssldone) {
pop3_to_pop3s(conn);
result = pop3_perform_capa(conn);
}
}
return result;
}
static CURLcode pop3_perform_user(struct connectdata *conn)
{
CURLcode result = CURLE_OK;
if(!conn->bits.user_passwd) {
state(conn, POP3_STOP);
return result;
}
result = Curl_pp_sendf(&conn->proto.pop3c.pp, "USER %s",
conn->user ? conn->user : "");
if(!result)
state(conn, POP3_USER);
return result;
}
#if !defined(CURL_DISABLE_CRYPTO_AUTH)
static CURLcode pop3_perform_apop(struct connectdata *conn)
{
CURLcode result = CURLE_OK;
struct pop3_conn *pop3c = &conn->proto.pop3c;
size_t i;
MD5_context *ctxt;
unsigned char digest[MD5_DIGEST_LEN];
char secret[2 * MD5_DIGEST_LEN + 1];
if(!conn->bits.user_passwd) {
state(conn, POP3_STOP);
return result;
}
ctxt = Curl_MD5_init(Curl_DIGEST_MD5);
if(!ctxt)
return CURLE_OUT_OF_MEMORY;
Curl_MD5_update(ctxt, (const unsigned char *) pop3c->apoptimestamp,
curlx_uztoui(strlen(pop3c->apoptimestamp)));
Curl_MD5_update(ctxt, (const unsigned char *) conn->passwd,
curlx_uztoui(strlen(conn->passwd)));
Curl_MD5_final(ctxt, digest);
for(i = 0; i < MD5_DIGEST_LEN; i++)
msnprintf(&secret[2 * i], 3, "%02x", digest[i]);
result = Curl_pp_sendf(&pop3c->pp, "APOP %s %s", conn->user, secret);
if(!result)
state(conn, POP3_APOP);
return result;
}
#endif
static CURLcode pop3_perform_auth(struct connectdata *conn,
const char *mech,
const char *initresp)
{
CURLcode result = CURLE_OK;
struct pop3_conn *pop3c = &conn->proto.pop3c;
if(initresp) { 
result = Curl_pp_sendf(&pop3c->pp, "AUTH %s %s", mech, initresp);
}
else {
result = Curl_pp_sendf(&pop3c->pp, "AUTH %s", mech);
}
return result;
}
static CURLcode pop3_continue_auth(struct connectdata *conn,
const char *resp)
{
struct pop3_conn *pop3c = &conn->proto.pop3c;
return Curl_pp_sendf(&pop3c->pp, "%s", resp);
}
static CURLcode pop3_perform_authentication(struct connectdata *conn)
{
CURLcode result = CURLE_OK;
struct pop3_conn *pop3c = &conn->proto.pop3c;
saslprogress progress = SASL_IDLE;
if(!Curl_sasl_can_authenticate(&pop3c->sasl, conn)) {
state(conn, POP3_STOP);
return result;
}
if(pop3c->authtypes & pop3c->preftype & POP3_TYPE_SASL) {
result = Curl_sasl_start(&pop3c->sasl, conn, FALSE, &progress);
if(!result)
if(progress == SASL_INPROGRESS)
state(conn, POP3_AUTH);
}
if(!result && progress == SASL_IDLE) {
#if !defined(CURL_DISABLE_CRYPTO_AUTH)
if(pop3c->authtypes & pop3c->preftype & POP3_TYPE_APOP)
result = pop3_perform_apop(conn);
else
#endif
if(pop3c->authtypes & pop3c->preftype & POP3_TYPE_CLEARTEXT)
result = pop3_perform_user(conn);
else {
infof(conn->data, "No known authentication mechanisms supported!\n");
result = CURLE_LOGIN_DENIED;
}
}
return result;
}
static CURLcode pop3_perform_command(struct connectdata *conn)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
struct POP3 *pop3 = data->req.protop;
const char *command = NULL;
if(pop3->id[0] == '\0' || conn->data->set.ftp_list_only) {
command = "LIST";
if(pop3->id[0] != '\0')
pop3->transfer = FTPTRANSFER_INFO;
}
else
command = "RETR";
if(pop3->id[0] != '\0')
result = Curl_pp_sendf(&conn->proto.pop3c.pp, "%s %s",
(pop3->custom && pop3->custom[0] != '\0' ?
pop3->custom : command), pop3->id);
else
result = Curl_pp_sendf(&conn->proto.pop3c.pp, "%s",
(pop3->custom && pop3->custom[0] != '\0' ?
pop3->custom : command));
if(!result)
state(conn, POP3_COMMAND);
return result;
}
static CURLcode pop3_perform_quit(struct connectdata *conn)
{
CURLcode result = Curl_pp_sendf(&conn->proto.pop3c.pp, "%s", "QUIT");
if(!result)
state(conn, POP3_QUIT);
return result;
}
static CURLcode pop3_state_servergreet_resp(struct connectdata *conn,
int pop3code,
pop3state instate)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
struct pop3_conn *pop3c = &conn->proto.pop3c;
const char *line = data->state.buffer;
size_t len = strlen(line);
(void)instate; 
if(pop3code != '+') {
failf(data, "Got unexpected pop3-server response");
result = CURLE_WEIRD_SERVER_REPLY;
}
else {
if(len >= 4 && line[len - 2] == '>') {
size_t i;
for(i = 3; i < len - 2; ++i) {
if(line[i] == '<') {
size_t timestamplen = len - 1 - i;
char *at;
if(!timestamplen)
break;
pop3c->apoptimestamp = (char *)calloc(1, timestamplen + 1);
if(!pop3c->apoptimestamp)
break;
memcpy(pop3c->apoptimestamp, line + i, timestamplen);
pop3c->apoptimestamp[timestamplen] = '\0';
at = strchr(pop3c->apoptimestamp, '@');
if(!at)
Curl_safefree(pop3c->apoptimestamp);
else
pop3c->authtypes |= POP3_TYPE_APOP;
break;
}
}
}
result = pop3_perform_capa(conn);
}
return result;
}
static CURLcode pop3_state_capa_resp(struct connectdata *conn, int pop3code,
pop3state instate)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
struct pop3_conn *pop3c = &conn->proto.pop3c;
const char *line = data->state.buffer;
size_t len = strlen(line);
(void)instate; 
if(pop3code == '*') {
if(len >= 4 && !memcmp(line, "STLS", 4))
pop3c->tls_supported = TRUE;
else if(len >= 4 && !memcmp(line, "USER", 4))
pop3c->authtypes |= POP3_TYPE_CLEARTEXT;
else if(len >= 5 && !memcmp(line, "SASL ", 5)) {
pop3c->authtypes |= POP3_TYPE_SASL;
line += 5;
len -= 5;
for(;;) {
size_t llen;
size_t wordlen;
unsigned int mechbit;
while(len &&
(*line == ' ' || *line == '\t' ||
*line == '\r' || *line == '\n')) {
line++;
len--;
}
if(!len)
break;
for(wordlen = 0; wordlen < len && line[wordlen] != ' ' &&
line[wordlen] != '\t' && line[wordlen] != '\r' &&
line[wordlen] != '\n';)
wordlen++;
mechbit = Curl_sasl_decode_mech(line, wordlen, &llen);
if(mechbit && llen == wordlen)
pop3c->sasl.authmechs |= mechbit;
line += wordlen;
len -= wordlen;
}
}
}
else if(pop3code == '+') {
if(data->set.use_ssl && !conn->ssl[FIRSTSOCKET].use) {
if(pop3c->tls_supported)
result = pop3_perform_starttls(conn);
else if(data->set.use_ssl == CURLUSESSL_TRY)
result = pop3_perform_authentication(conn);
else {
failf(data, "STLS not supported.");
result = CURLE_USE_SSL_FAILED;
}
}
else
result = pop3_perform_authentication(conn);
}
else {
pop3c->authtypes |= POP3_TYPE_CLEARTEXT;
result = pop3_perform_authentication(conn);
}
return result;
}
static CURLcode pop3_state_starttls_resp(struct connectdata *conn,
int pop3code,
pop3state instate)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
(void)instate; 
if(pop3code != '+') {
if(data->set.use_ssl != CURLUSESSL_TRY) {
failf(data, "STARTTLS denied");
result = CURLE_USE_SSL_FAILED;
}
else
result = pop3_perform_authentication(conn);
}
else
result = pop3_perform_upgrade_tls(conn);
return result;
}
static CURLcode pop3_state_auth_resp(struct connectdata *conn,
int pop3code,
pop3state instate)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
struct pop3_conn *pop3c = &conn->proto.pop3c;
saslprogress progress;
(void)instate; 
result = Curl_sasl_continue(&pop3c->sasl, conn, pop3code, &progress);
if(!result)
switch(progress) {
case SASL_DONE:
state(conn, POP3_STOP); 
break;
case SASL_IDLE: 
#if !defined(CURL_DISABLE_CRYPTO_AUTH)
if(pop3c->authtypes & pop3c->preftype & POP3_TYPE_APOP)
result = pop3_perform_apop(conn);
else
#endif
if(pop3c->authtypes & pop3c->preftype & POP3_TYPE_CLEARTEXT)
result = pop3_perform_user(conn);
else {
failf(data, "Authentication cancelled");
result = CURLE_LOGIN_DENIED;
}
break;
default:
break;
}
return result;
}
#if !defined(CURL_DISABLE_CRYPTO_AUTH)
static CURLcode pop3_state_apop_resp(struct connectdata *conn, int pop3code,
pop3state instate)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
(void)instate; 
if(pop3code != '+') {
failf(data, "Authentication failed: %d", pop3code);
result = CURLE_LOGIN_DENIED;
}
else
state(conn, POP3_STOP);
return result;
}
#endif
static CURLcode pop3_state_user_resp(struct connectdata *conn, int pop3code,
pop3state instate)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
(void)instate; 
if(pop3code != '+') {
failf(data, "Access denied. %c", pop3code);
result = CURLE_LOGIN_DENIED;
}
else
result = Curl_pp_sendf(&conn->proto.pop3c.pp, "PASS %s",
conn->passwd ? conn->passwd : "");
if(!result)
state(conn, POP3_PASS);
return result;
}
static CURLcode pop3_state_pass_resp(struct connectdata *conn, int pop3code,
pop3state instate)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
(void)instate; 
if(pop3code != '+') {
failf(data, "Access denied. %c", pop3code);
result = CURLE_LOGIN_DENIED;
}
else
state(conn, POP3_STOP);
return result;
}
static CURLcode pop3_state_command_resp(struct connectdata *conn,
int pop3code,
pop3state instate)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
struct POP3 *pop3 = data->req.protop;
struct pop3_conn *pop3c = &conn->proto.pop3c;
struct pingpong *pp = &pop3c->pp;
(void)instate; 
if(pop3code != '+') {
state(conn, POP3_STOP);
return CURLE_RECV_ERROR;
}
pop3c->eob = 2;
pop3c->strip = 2;
if(pop3->transfer == FTPTRANSFER_BODY) {
Curl_setup_transfer(data, FIRSTSOCKET, -1, FALSE, -1);
if(pp->cache) {
if(!data->set.opt_no_body) {
result = Curl_pop3_write(conn, pp->cache, pp->cache_size);
if(result)
return result;
}
Curl_safefree(pp->cache);
pp->cache_size = 0;
}
}
state(conn, POP3_STOP);
return result;
}
static CURLcode pop3_statemach_act(struct connectdata *conn)
{
CURLcode result = CURLE_OK;
curl_socket_t sock = conn->sock[FIRSTSOCKET];
int pop3code;
struct pop3_conn *pop3c = &conn->proto.pop3c;
struct pingpong *pp = &pop3c->pp;
size_t nread = 0;
if(pop3c->state == POP3_UPGRADETLS)
return pop3_perform_upgrade_tls(conn);
if(pp->sendleft)
return Curl_pp_flushsend(pp);
do {
result = Curl_pp_readresp(sock, pp, &pop3code, &nread);
if(result)
return result;
if(!pop3code)
break;
switch(pop3c->state) {
case POP3_SERVERGREET:
result = pop3_state_servergreet_resp(conn, pop3code, pop3c->state);
break;
case POP3_CAPA:
result = pop3_state_capa_resp(conn, pop3code, pop3c->state);
break;
case POP3_STARTTLS:
result = pop3_state_starttls_resp(conn, pop3code, pop3c->state);
break;
case POP3_AUTH:
result = pop3_state_auth_resp(conn, pop3code, pop3c->state);
break;
#if !defined(CURL_DISABLE_CRYPTO_AUTH)
case POP3_APOP:
result = pop3_state_apop_resp(conn, pop3code, pop3c->state);
break;
#endif
case POP3_USER:
result = pop3_state_user_resp(conn, pop3code, pop3c->state);
break;
case POP3_PASS:
result = pop3_state_pass_resp(conn, pop3code, pop3c->state);
break;
case POP3_COMMAND:
result = pop3_state_command_resp(conn, pop3code, pop3c->state);
break;
case POP3_QUIT:
default:
state(conn, POP3_STOP);
break;
}
} while(!result && pop3c->state != POP3_STOP && Curl_pp_moredata(pp));
return result;
}
static CURLcode pop3_multi_statemach(struct connectdata *conn, bool *done)
{
CURLcode result = CURLE_OK;
struct pop3_conn *pop3c = &conn->proto.pop3c;
if((conn->handler->flags & PROTOPT_SSL) && !pop3c->ssldone) {
result = Curl_ssl_connect_nonblocking(conn, FIRSTSOCKET, &pop3c->ssldone);
if(result || !pop3c->ssldone)
return result;
}
result = Curl_pp_statemach(&pop3c->pp, FALSE, FALSE);
*done = (pop3c->state == POP3_STOP) ? TRUE : FALSE;
return result;
}
static CURLcode pop3_block_statemach(struct connectdata *conn,
bool disconnecting)
{
CURLcode result = CURLE_OK;
struct pop3_conn *pop3c = &conn->proto.pop3c;
while(pop3c->state != POP3_STOP && !result)
result = Curl_pp_statemach(&pop3c->pp, TRUE, disconnecting);
return result;
}
static CURLcode pop3_init(struct connectdata *conn)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
struct POP3 *pop3;
pop3 = data->req.protop = calloc(sizeof(struct POP3), 1);
if(!pop3)
result = CURLE_OUT_OF_MEMORY;
return result;
}
static int pop3_getsock(struct connectdata *conn, curl_socket_t *socks)
{
return Curl_pp_getsock(&conn->proto.pop3c.pp, socks);
}
static CURLcode pop3_connect(struct connectdata *conn, bool *done)
{
CURLcode result = CURLE_OK;
struct pop3_conn *pop3c = &conn->proto.pop3c;
struct pingpong *pp = &pop3c->pp;
*done = FALSE; 
connkeep(conn, "POP3 default");
pp->response_time = RESP_TIMEOUT;
pp->statemach_act = pop3_statemach_act;
pp->endofresp = pop3_endofresp;
pp->conn = conn;
pop3c->preftype = POP3_TYPE_ANY;
Curl_sasl_init(&pop3c->sasl, &saslpop3);
Curl_pp_init(pp);
result = pop3_parse_url_options(conn);
if(result)
return result;
state(conn, POP3_SERVERGREET);
result = pop3_multi_statemach(conn, done);
return result;
}
static CURLcode pop3_done(struct connectdata *conn, CURLcode status,
bool premature)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
struct POP3 *pop3 = data->req.protop;
(void)premature;
if(!pop3)
return CURLE_OK;
if(status) {
connclose(conn, "POP3 done with bad status");
result = status; 
}
Curl_safefree(pop3->id);
Curl_safefree(pop3->custom);
pop3->transfer = FTPTRANSFER_BODY;
return result;
}
static CURLcode pop3_perform(struct connectdata *conn, bool *connected,
bool *dophase_done)
{
CURLcode result = CURLE_OK;
struct POP3 *pop3 = conn->data->req.protop;
DEBUGF(infof(conn->data, "DO phase starts\n"));
if(conn->data->set.opt_no_body) {
pop3->transfer = FTPTRANSFER_INFO;
}
*dophase_done = FALSE; 
result = pop3_perform_command(conn);
if(result)
return result;
result = pop3_multi_statemach(conn, dophase_done);
*connected = conn->bits.tcpconnect[FIRSTSOCKET];
if(*dophase_done)
DEBUGF(infof(conn->data, "DO phase is complete\n"));
return result;
}
static CURLcode pop3_do(struct connectdata *conn, bool *done)
{
CURLcode result = CURLE_OK;
*done = FALSE; 
result = pop3_parse_url_path(conn);
if(result)
return result;
result = pop3_parse_custom_request(conn);
if(result)
return result;
result = pop3_regular_transfer(conn, done);
return result;
}
static CURLcode pop3_disconnect(struct connectdata *conn, bool dead_connection)
{
struct pop3_conn *pop3c = &conn->proto.pop3c;
if(!dead_connection && pop3c->pp.conn && pop3c->pp.conn->bits.protoconnstart)
if(!pop3_perform_quit(conn))
(void)pop3_block_statemach(conn, TRUE); 
Curl_pp_disconnect(&pop3c->pp);
Curl_sasl_cleanup(conn, pop3c->sasl.authused);
Curl_safefree(pop3c->apoptimestamp);
return CURLE_OK;
}
static CURLcode pop3_dophase_done(struct connectdata *conn, bool connected)
{
(void)conn;
(void)connected;
return CURLE_OK;
}
static CURLcode pop3_doing(struct connectdata *conn, bool *dophase_done)
{
CURLcode result = pop3_multi_statemach(conn, dophase_done);
if(result)
DEBUGF(infof(conn->data, "DO phase failed\n"));
else if(*dophase_done) {
result = pop3_dophase_done(conn, FALSE );
DEBUGF(infof(conn->data, "DO phase is complete\n"));
}
return result;
}
static CURLcode pop3_regular_transfer(struct connectdata *conn,
bool *dophase_done)
{
CURLcode result = CURLE_OK;
bool connected = FALSE;
struct Curl_easy *data = conn->data;
data->req.size = -1;
Curl_pgrsSetUploadCounter(data, 0);
Curl_pgrsSetDownloadCounter(data, 0);
Curl_pgrsSetUploadSize(data, -1);
Curl_pgrsSetDownloadSize(data, -1);
result = pop3_perform(conn, &connected, dophase_done);
if(!result && *dophase_done)
result = pop3_dophase_done(conn, connected);
return result;
}
static CURLcode pop3_setup_connection(struct connectdata *conn)
{
CURLcode result = pop3_init(conn);
if(result)
return result;
conn->tls_upgraded = FALSE;
return CURLE_OK;
}
static CURLcode pop3_parse_url_options(struct connectdata *conn)
{
CURLcode result = CURLE_OK;
struct pop3_conn *pop3c = &conn->proto.pop3c;
const char *ptr = conn->options;
pop3c->sasl.resetprefs = TRUE;
while(!result && ptr && *ptr) {
const char *key = ptr;
const char *value;
while(*ptr && *ptr != '=')
ptr++;
value = ptr + 1;
while(*ptr && *ptr != ';')
ptr++;
if(strncasecompare(key, "AUTH=", 5)) {
result = Curl_sasl_parse_url_auth_option(&pop3c->sasl,
value, ptr - value);
if(result && strncasecompare(value, "+APOP", ptr - value)) {
pop3c->preftype = POP3_TYPE_APOP;
pop3c->sasl.prefmech = SASL_AUTH_NONE;
result = CURLE_OK;
}
}
else
result = CURLE_URL_MALFORMAT;
if(*ptr == ';')
ptr++;
}
if(pop3c->preftype != POP3_TYPE_APOP)
switch(pop3c->sasl.prefmech) {
case SASL_AUTH_NONE:
pop3c->preftype = POP3_TYPE_NONE;
break;
case SASL_AUTH_DEFAULT:
pop3c->preftype = POP3_TYPE_ANY;
break;
default:
pop3c->preftype = POP3_TYPE_SASL;
break;
}
return result;
}
static CURLcode pop3_parse_url_path(struct connectdata *conn)
{
struct Curl_easy *data = conn->data;
struct POP3 *pop3 = data->req.protop;
const char *path = &data->state.up.path[1]; 
return Curl_urldecode(data, path, 0, &pop3->id, NULL, TRUE);
}
static CURLcode pop3_parse_custom_request(struct connectdata *conn)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
struct POP3 *pop3 = data->req.protop;
const char *custom = data->set.str[STRING_CUSTOMREQUEST];
if(custom)
result = Curl_urldecode(data, custom, 0, &pop3->custom, NULL, TRUE);
return result;
}
CURLcode Curl_pop3_write(struct connectdata *conn, char *str, size_t nread)
{
CURLcode result = CURLE_OK;
struct Curl_easy *data = conn->data;
struct SingleRequest *k = &data->req;
struct pop3_conn *pop3c = &conn->proto.pop3c;
bool strip_dot = FALSE;
size_t last = 0;
size_t i;
for(i = 0; i < nread; i++) {
size_t prev = pop3c->eob;
switch(str[i]) {
case 0x0d:
if(pop3c->eob == 0) {
pop3c->eob++;
if(i) {
result = Curl_client_write(conn, CLIENTWRITE_BODY, &str[last],
i - last);
if(result)
return result;
last = i;
}
}
else if(pop3c->eob == 3)
pop3c->eob++;
else
pop3c->eob = 1;
break;
case 0x0a:
if(pop3c->eob == 1 || pop3c->eob == 4)
pop3c->eob++;
else
pop3c->eob = 0;
break;
case 0x2e:
if(pop3c->eob == 2)
pop3c->eob++;
else if(pop3c->eob == 3) {
strip_dot = TRUE;
pop3c->eob = 0;
}
else
pop3c->eob = 0;
break;
default:
pop3c->eob = 0;
break;
}
if(prev && prev >= pop3c->eob) {
while(prev && pop3c->strip) {
prev--;
pop3c->strip--;
}
if(prev) {
result = Curl_client_write(conn, CLIENTWRITE_BODY, (char *)POP3_EOB,
strip_dot ? prev - 1 : prev);
if(result)
return result;
last = i;
strip_dot = FALSE;
}
}
}
if(pop3c->eob == POP3_EOB_LEN) {
result = Curl_client_write(conn, CLIENTWRITE_BODY, (char *)POP3_EOB, 2);
k->keepon &= ~KEEP_RECV;
pop3c->eob = 0;
return result;
}
if(pop3c->eob)
return CURLE_OK;
if(nread - last) {
result = Curl_client_write(conn, CLIENTWRITE_BODY, &str[last],
nread - last);
}
return result;
}
#endif 
