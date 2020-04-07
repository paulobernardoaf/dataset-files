



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <string.h>
#include <windows.h>

#include <vlc_common.h>
#include <vlc_network.h>
#include <vlc_url.h>

char *vlc_getProxyUrl(const char *psz_url)
{
VLC_UNUSED(psz_url);

char *proxy = config_GetPsz( "http-proxy" );
if (proxy == NULL)
return NULL;

char *proxy_pwd = config_GetPsz( "http-proxy-pwd" );
if (proxy_pwd == NULL)
return proxy;

vlc_url_t url;
if (vlc_UrlParse(&url, proxy) < 0) {
vlc_UrlClean(&url);
free (proxy);
free (proxy_pwd);
return NULL;
}

if (url.psz_password == NULL )
url.psz_password = proxy_pwd;

char *proxy_url = vlc_uri_compose (&url);
vlc_UrlClean (&url);

free (proxy_pwd);
free (proxy);

#if 0

HKEY h_key;


if( RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft"
"\\Windows\\CurrentVersion\\Internet Settings",
0, KEY_READ, &h_key ) == ERROR_SUCCESS )
return NULL;

DWORD len = sizeof( DWORD );
BYTE proxyEnable;


if( RegQueryValueEx( h_key, "ProxyEnable", NULL, NULL,
&proxyEnable, &len ) != ERROR_SUCCESS
|| !proxyEnable )
goto out;






unsigned char key[256];

len = sizeof( key );
if( RegQueryValueEx( h_key, "ProxyServer", NULL, NULL,
key, &len ) == ERROR_SUCCESS )
{

#warning FIXME.
char *psz_proxy = strstr( (char *)key, "http=" );
if( psz_proxy != NULL )
{
psz_proxy += 5;
char *end = strchr( psz_proxy, ';' );
if( end != NULL )
*end = '\0';
}
else
psz_proxy = (char *)key;
proxy_url = strdup( psz_proxy );
}

out:
RegCloseKey( h_key );
#endif
return proxy_url;
}
