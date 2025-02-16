#include "curlcheck.h"
#include "netrc.h"
#include "memdebug.h" 
static char *login;
static char *password;
static char filename[64];
static CURLcode unit_setup(void)
{
password = strdup("");
login = strdup("");
if(!password || !login) {
Curl_safefree(password);
Curl_safefree(login);
return CURLE_OUT_OF_MEMORY;
}
return CURLE_OK;
}
static void unit_stop(void)
{
Curl_safefree(password);
Curl_safefree(login);
}
UNITTEST_START
int result;
bool login_changed;
bool password_changed;
static const char * const filename1 = "log/netrc1304";
memcpy(filename, filename1, strlen(filename1));
result = Curl_parsenetrc("test.example.com", &login, &password,
&login_changed, &password_changed, filename);
fail_unless(result == 1, "Host not found should return 1");
abort_unless(password != NULL, "returned NULL!");
fail_unless(password[0] == 0, "password should not have been changed");
abort_unless(login != NULL, "returned NULL!");
fail_unless(login[0] == 0, "login should not have been changed");
free(login);
login = strdup("me");
abort_unless(login != NULL, "returned NULL!");
result = Curl_parsenetrc("example.com", &login, &password,
&login_changed, &password_changed, filename);
fail_unless(result == 0, "Host should have been found");
abort_unless(password != NULL, "returned NULL!");
fail_unless(password[0] == 0, "password should not have been changed");
fail_unless(!password_changed, "password should not have been changed");
abort_unless(login != NULL, "returned NULL!");
fail_unless(strncmp(login, "me", 2) == 0,
"login should not have been changed");
fail_unless(!login_changed, "login should not have been changed");
free(login);
login = strdup("me");
abort_unless(login != NULL, "returned NULL!");
result = Curl_parsenetrc("test.example.com", &login, &password,
&login_changed, &password_changed, filename);
fail_unless(result == 1, "Host not found should return 1");
abort_unless(password != NULL, "returned NULL!");
fail_unless(password[0] == 0, "password should not have been changed");
abort_unless(login != NULL, "returned NULL!");
fail_unless(strncmp(login, "me", 2) == 0,
"login should not have been changed");
free(login);
login = strdup("admi");
abort_unless(login != NULL, "returned NULL!");
result = Curl_parsenetrc("example.com", &login, &password,
&login_changed, &password_changed, filename);
fail_unless(result == 0, "Host should have been found");
abort_unless(password != NULL, "returned NULL!");
fail_unless(password[0] == 0, "password should not have been changed");
fail_unless(!password_changed, "password should not have been changed");
abort_unless(login != NULL, "returned NULL!");
fail_unless(strncmp(login, "admi", 4) == 0,
"login should not have been changed");
fail_unless(!login_changed, "login should not have been changed");
free(login);
login = strdup("adminn");
abort_unless(login != NULL, "returned NULL!");
result = Curl_parsenetrc("example.com", &login, &password,
&login_changed, &password_changed, filename);
fail_unless(result == 0, "Host should have been found");
abort_unless(password != NULL, "returned NULL!");
fail_unless(password[0] == 0, "password should not have been changed");
fail_unless(!password_changed, "password should not have been changed");
abort_unless(login != NULL, "returned NULL!");
fail_unless(strncmp(login, "adminn", 6) == 0,
"login should not have been changed");
fail_unless(!login_changed, "login should not have been changed");
free(login);
login = strdup("");
abort_unless(login != NULL, "returned NULL!");
result = Curl_parsenetrc("example.com", &login, &password,
&login_changed, &password_changed, filename);
fail_unless(result == 0, "Host should have been found");
abort_unless(password != NULL, "returned NULL!");
fail_unless(strncmp(password, "passwd", 6) == 0,
"password should be 'passwd'");
fail_unless(password_changed, "password should have been changed");
abort_unless(login != NULL, "returned NULL!");
fail_unless(strncmp(login, "admin", 5) == 0, "login should be 'admin'");
fail_unless(login_changed, "login should have been changed");
free(password);
password = strdup("");
abort_unless(password != NULL, "returned NULL!");
result = Curl_parsenetrc("example.com", &login, &password,
&login_changed, &password_changed, filename);
fail_unless(result == 0, "Host should have been found");
abort_unless(password != NULL, "returned NULL!");
fail_unless(strncmp(password, "passwd", 6) == 0,
"password should be 'passwd'");
fail_unless(password_changed, "password should have been changed");
abort_unless(login != NULL, "returned NULL!");
fail_unless(strncmp(login, "admin", 5) == 0, "login should be 'admin'");
fail_unless(!login_changed, "login should not have been changed");
free(password);
password = strdup("");
abort_unless(password != NULL, "returned NULL!");
free(login);
login = strdup("");
abort_unless(login != NULL, "returned NULL!");
result = Curl_parsenetrc("curl.example.com", &login, &password,
&login_changed, &password_changed, filename);
fail_unless(result == 0, "Host should have been found");
abort_unless(password != NULL, "returned NULL!");
fail_unless(strncmp(password, "none", 4) == 0,
"password should be 'none'");
fail_unless(password_changed, "password should have been changed");
abort_unless(login != NULL, "returned NULL!");
fail_unless(strncmp(login, "none", 4) == 0, "login should be 'none'");
fail_unless(login_changed, "login should have been changed");
free(password);
password = strdup("");
abort_unless(password != NULL, "returned NULL!");
result = Curl_parsenetrc("curl.example.com", &login, &password,
&login_changed, &password_changed, filename);
fail_unless(result == 0, "Host should have been found");
abort_unless(password != NULL, "returned NULL!");
fail_unless(strncmp(password, "none", 4) == 0,
"password should be 'none'");
fail_unless(password_changed, "password should have been changed");
abort_unless(login != NULL, "returned NULL!");
fail_unless(strncmp(login, "none", 4) == 0, "login should be 'none'");
fail_unless(!login_changed, "login should not have been changed");
UNITTEST_STOP
