#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_http.h>
#include <vlc_md5.h>
#include <vlc_rand.h>
#include <vlc_strings.h>
#include "libvlc.h"
static char *AuthGetParam( const char *psz_header, const char *psz_param )
{
char psz_what[strlen(psz_param)+3];
sprintf( psz_what, "%s=\"", psz_param );
psz_header = strstr( psz_header, psz_what );
if ( psz_header )
{
const char *psz_end;
psz_header += strlen( psz_what );
psz_end = strchr( psz_header, '"' );
if ( !psz_end ) 
return strdup( psz_header );
return strndup( psz_header, psz_end - psz_header );
}
else
{
return NULL;
}
}
static char *AuthGetParamNoQuotes( const char *psz_header, const char *psz_param )
{
char psz_what[strlen(psz_param)+2];
sprintf( psz_what, "%s=", psz_param );
psz_header = strstr( psz_header, psz_what );
if ( psz_header )
{
const char *psz_end;
psz_header += strlen( psz_what );
psz_end = strchr( psz_header, ',' );
if ( !psz_end ) 
return strdup( psz_header );
return strndup( psz_header, psz_end - psz_header );
}
else
{
return NULL;
}
}
static char *GenerateCnonce()
{
char ps_random[32];
struct md5_s md5;
vlc_rand_bytes( ps_random, sizeof( ps_random ) );
InitMD5( &md5 );
AddMD5( &md5, ps_random, sizeof( ps_random ) );
EndMD5( &md5 );
return psz_md5_hash( &md5 );
}
static char *AuthDigest( vlc_object_t *p_this, vlc_http_auth_t *p_auth,
const char *psz_method, const char *psz_path,
const char *psz_username, const char *psz_password )
{
char *psz_HA1 = NULL;
char *psz_HA2 = NULL;
char *psz_ent = NULL;
char *psz_result = NULL;
char psz_inonce[9];
struct md5_s md5;
struct md5_s ent;
if ( p_auth->psz_realm == NULL )
{
msg_Warn( p_this, "Digest Authentication: "
"Mandatory 'realm' value not available" );
goto error;
}
if ( p_auth->psz_HA1 )
{
psz_HA1 = strdup( p_auth->psz_HA1 );
if ( psz_HA1 == NULL )
goto error;
}
else
{
InitMD5( &md5 );
AddMD5( &md5, psz_username, strlen( psz_username ) );
AddMD5( &md5, ":", 1 );
AddMD5( &md5, p_auth->psz_realm, strlen( p_auth->psz_realm ) );
AddMD5( &md5, ":", 1 );
AddMD5( &md5, psz_password, strlen( psz_password ) );
EndMD5( &md5 );
psz_HA1 = psz_md5_hash( &md5 );
if ( psz_HA1 == NULL )
goto error;
if ( p_auth->psz_algorithm &&
strcmp( p_auth->psz_algorithm, "MD5-sess" ) == 0 )
{
InitMD5( &md5 );
AddMD5( &md5, psz_HA1, 32 );
AddMD5( &md5, ":", 1 );
AddMD5( &md5, p_auth->psz_nonce, strlen( p_auth->psz_nonce ) );
AddMD5( &md5, ":", 1 );
AddMD5( &md5, p_auth->psz_cnonce, strlen( p_auth->psz_cnonce ) );
EndMD5( &md5 );
free( psz_HA1 );
psz_HA1 = psz_md5_hash( &md5 );
if ( psz_HA1 == NULL )
goto error;
p_auth->psz_HA1 = strdup( psz_HA1 );
if ( p_auth->psz_HA1 == NULL )
goto error;
}
}
InitMD5( &md5 );
if ( *psz_method )
AddMD5( &md5, psz_method, strlen( psz_method ) );
AddMD5( &md5, ":", 1 );
if ( psz_path )
AddMD5( &md5, psz_path, strlen( psz_path ) );
else
AddMD5( &md5, "/", 1 );
if ( p_auth->psz_qop && strcmp( p_auth->psz_qop, "auth-int" ) == 0 )
{
InitMD5( &ent );
AddMD5( &ent, "", 0 );
EndMD5( &ent );
psz_ent = psz_md5_hash( &ent );
if ( psz_ent == NULL )
goto error;
AddMD5( &md5, ":", 1 );
AddMD5( &md5, psz_ent, 32 );
}
EndMD5( &md5 );
psz_HA2 = psz_md5_hash( &md5 );
if ( psz_HA2 == NULL )
goto error;
InitMD5( &md5 );
AddMD5( &md5, psz_HA1, 32 );
AddMD5( &md5, ":", 1 );
AddMD5( &md5, p_auth->psz_nonce, strlen( p_auth->psz_nonce ) );
AddMD5( &md5, ":", 1 );
if ( p_auth->psz_qop &&
( strcmp( p_auth->psz_qop, "auth" ) == 0 ||
strcmp( p_auth->psz_qop, "auth-int" ) == 0 ) )
{
snprintf( psz_inonce, sizeof( psz_inonce ), "%08x", p_auth->i_nonce );
AddMD5( &md5, psz_inonce, 8 );
AddMD5( &md5, ":", 1 );
AddMD5( &md5, p_auth->psz_cnonce, strlen( p_auth->psz_cnonce ) );
AddMD5( &md5, ":", 1 );
AddMD5( &md5, p_auth->psz_qop, strlen( p_auth->psz_qop ) );
AddMD5( &md5, ":", 1 );
}
AddMD5( &md5, psz_HA2, 32 );
EndMD5( &md5 );
psz_result = psz_md5_hash( &md5 );
error:
free( psz_HA1 );
free( psz_HA2 );
free( psz_ent );
return psz_result;
}
void vlc_http_auth_ParseWwwAuthenticateHeader(
vlc_object_t *p_this, vlc_http_auth_t *p_auth,
const char *psz_header )
{
static const char psz_basic_prefix[] = "Basic ";
static const char psz_digest_prefix[] = "Digest ";
if ( strncasecmp( psz_header, psz_basic_prefix,
sizeof( psz_basic_prefix ) - 1 ) == 0 )
{
msg_Dbg( p_this, "Using Basic Authentication" );
psz_header += sizeof( psz_basic_prefix ) - 1;
p_auth->psz_realm = AuthGetParam( psz_header, "realm" );
if ( p_auth->psz_realm == NULL )
msg_Warn( p_this, "Basic Authentication: "
"Mandatory 'realm' parameter is missing" );
}
else if ( strncasecmp( psz_header, psz_digest_prefix,
sizeof( psz_digest_prefix ) - 1 ) == 0 )
{
msg_Dbg( p_this, "Using Digest Access Authentication" );
if ( p_auth->psz_nonce )
return;
psz_header += sizeof( psz_digest_prefix ) - 1;
p_auth->psz_realm = AuthGetParam( psz_header, "realm" );
p_auth->psz_domain = AuthGetParam( psz_header, "domain" );
p_auth->psz_nonce = AuthGetParam( psz_header, "nonce" );
p_auth->psz_opaque = AuthGetParam( psz_header, "opaque" );
p_auth->psz_stale = AuthGetParamNoQuotes( psz_header, "stale" );
p_auth->psz_algorithm = AuthGetParamNoQuotes( psz_header, "algorithm" );
p_auth->psz_qop = AuthGetParam( psz_header, "qop" );
p_auth->i_nonce = 0;
if ( p_auth->psz_realm == NULL )
msg_Warn( p_this, "Digest Access Authentication: "
"Mandatory 'realm' parameter is missing" );
if ( p_auth->psz_nonce == NULL )
msg_Warn( p_this, "Digest Access Authentication: "
"Mandatory 'nonce' parameter is missing" );
if ( p_auth->psz_qop )
{
char *psz_tmp = strchr( p_auth->psz_qop, ',' );
if ( psz_tmp )
*psz_tmp = '\0';
}
}
else
{
const char *psz_end = strchr( psz_header, ' ' );
if ( psz_end )
msg_Warn( p_this, "Unknown authentication scheme: '%*s'",
(int)(psz_end - psz_header), psz_header );
else
msg_Warn( p_this, "Unknown authentication scheme: '%s'",
psz_header );
}
}
int vlc_http_auth_ParseAuthenticationInfoHeader(
vlc_object_t *p_this, vlc_http_auth_t *p_auth,
const char *psz_header, const char *psz_method, const char *psz_path,
const char *psz_username, const char *psz_password )
{
char *psz_nextnonce = AuthGetParam( psz_header, "nextnonce" );
char *psz_qop = AuthGetParamNoQuotes( psz_header, "qop" );
char *psz_rspauth = AuthGetParam( psz_header, "rspauth" );
char *psz_cnonce = AuthGetParam( psz_header, "cnonce" );
char *psz_nc = AuthGetParamNoQuotes( psz_header, "nc" );
char *psz_digest = NULL;
int i_err = VLC_SUCCESS;
int i_nonce;
if ( psz_cnonce )
{
if ( strcmp( psz_cnonce, p_auth->psz_cnonce ) != 0 )
{
msg_Err( p_this, "HTTP Digest Access Authentication: server "
"replied with a different client nonce value." );
i_err = VLC_EGENERIC;
goto error;
}
if ( psz_nc )
{
i_nonce = strtol( psz_nc, NULL, 16 );
if ( i_nonce != p_auth->i_nonce )
{
msg_Err( p_this, "HTTP Digest Access Authentication: server "
"replied with a different nonce count "
"value." );
i_err = VLC_EGENERIC;
goto error;
}
}
if ( psz_qop && p_auth->psz_qop &&
strcmp( psz_qop, p_auth->psz_qop ) != 0 )
msg_Warn( p_this, "HTTP Digest Access Authentication: server "
"replied using a different 'quality of "
"protection' option" );
psz_digest = AuthDigest( p_this, p_auth, psz_method, psz_path,
psz_username, psz_password );
if( psz_digest == NULL || strcmp( psz_digest, psz_rspauth ) != 0 )
{
msg_Err( p_this, "HTTP Digest Access Authentication: server "
"replied with an invalid response digest "
"(expected value: %s).", psz_digest );
i_err = VLC_EGENERIC;
goto error;
}
}
if ( psz_nextnonce )
{
free( p_auth->psz_nonce );
p_auth->psz_nonce = psz_nextnonce;
psz_nextnonce = NULL;
}
error:
free( psz_nextnonce );
free( psz_qop );
free( psz_rspauth );
free( psz_cnonce );
free( psz_nc );
free( psz_digest );
return i_err;
}
char *vlc_http_auth_FormatAuthorizationHeader(
vlc_object_t *p_this, vlc_http_auth_t *p_auth,
const char *psz_method, const char *psz_path,
const char *psz_username, const char *psz_password )
{
char *psz_result = NULL;
char *psz_buffer = NULL;
char *psz_base64 = NULL;
int i_rc;
if ( p_auth->psz_nonce )
{
if ( p_auth->psz_algorithm &&
strcmp( p_auth->psz_algorithm, "MD5" ) != 0 &&
strcmp( p_auth->psz_algorithm, "MD5-sess" ) != 0 )
{
msg_Err( p_this, "Digest Access Authentication: "
"Unknown algorithm '%s'", p_auth->psz_algorithm );
goto error;
}
if ( p_auth->psz_qop != NULL || p_auth->psz_cnonce == NULL )
{
free( p_auth->psz_cnonce );
p_auth->psz_cnonce = GenerateCnonce();
if ( p_auth->psz_cnonce == NULL )
goto error;
}
++p_auth->i_nonce;
psz_buffer = AuthDigest( p_this, p_auth, psz_method, psz_path,
psz_username, psz_password );
if ( psz_buffer == NULL )
goto error;
i_rc = asprintf( &psz_result,
"Digest "
"username=\"%s\", "
"realm=\"%s\", "
"nonce=\"%s\", "
"uri=\"%s\", "
"response=\"%s\", "
"%s%s%s" 
"%s%s%s" 
"%s%s%s" 
"%s%s%s" 
"%s=\"%08x\"", 
psz_username,
p_auth->psz_realm,
p_auth->psz_nonce,
psz_path ? psz_path : "/",
psz_buffer,
p_auth->psz_algorithm ? "algorithm=\"" : "",
p_auth->psz_algorithm ? p_auth->psz_algorithm : "",
p_auth->psz_algorithm ? "\", " : "",
p_auth->psz_cnonce ? "cnonce=\"" : "",
p_auth->psz_cnonce ? p_auth->psz_cnonce : "",
p_auth->psz_cnonce ? "\", " : "",
p_auth->psz_opaque ? "opaque=\"" : "",
p_auth->psz_opaque ? p_auth->psz_opaque : "",
p_auth->psz_opaque ? "\", " : "",
p_auth->psz_qop ? "qop=\"" : "",
p_auth->psz_qop ? p_auth->psz_qop : "",
p_auth->psz_qop ? "\", " : "",
p_auth->i_nonce ? "nc" : "uglyhack",
p_auth->i_nonce
);
if ( i_rc < 0 )
goto error;
}
else
{
i_rc = asprintf( &psz_buffer, "%s:%s", psz_username, psz_password );
if ( i_rc < 0 )
goto error;
psz_base64 = vlc_b64_encode( psz_buffer );
if ( psz_base64 == NULL )
goto error;
i_rc = asprintf( &psz_result, "Basic %s", psz_base64 );
if ( i_rc < 0 )
goto error;
}
error:
free( psz_buffer );
free( psz_base64 );
return psz_result;
}
void vlc_http_auth_Init( vlc_http_auth_t *p_auth )
{
memset( p_auth, 0, sizeof( *p_auth ) );
}
void vlc_http_auth_Deinit( vlc_http_auth_t *p_auth )
{
free( p_auth->psz_realm );
free( p_auth->psz_domain );
free( p_auth->psz_nonce );
free( p_auth->psz_opaque );
free( p_auth->psz_stale );
free( p_auth->psz_algorithm );
free( p_auth->psz_qop );
free( p_auth->psz_cnonce );
free( p_auth->psz_HA1 );
}
