#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#define FROM "<sender@example.org>"
#define TO "<addressee@example.net>"
#define CC "<info@example.org>"
static const char *headers_text[] = {
"Date: Tue, 22 Aug 2017 14:08:43 +0100",
"To: " TO,
"From: " FROM " (Example User)",
"Cc: " CC " (Another example User)",
"Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@"
"rfcpedant.example.org>",
"Subject: example sending a MIME-formatted message",
NULL
};
static const char inline_text[] =
"This is the inline text message of the e-mail.\r\n"
"\r\n"
" It could be a lot of lines that would be displayed in an e-mail\r\n"
"viewer that is not able to handle HTML.\r\n";
static const char inline_html[] =
"<html><body>\r\n"
"<p>This is the inline <b>HTML</b> message of the e-mail.</p>"
"<br />\r\n"
"<p>It could be a lot of HTML data that would be displayed by "
"e-mail viewers able to handle HTML.</p>"
"</body></html>\r\n";
int main(void)
{
CURL *curl;
CURLcode res = CURLE_OK;
curl = curl_easy_init();
if(curl) {
struct curl_slist *headers = NULL;
struct curl_slist *recipients = NULL;
struct curl_slist *slist = NULL;
curl_mime *mime;
curl_mime *alt;
curl_mimepart *part;
const char **cpp;
curl_easy_setopt(curl, CURLOPT_URL, "smtp://mail.example.com");
curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);
recipients = curl_slist_append(recipients, TO);
recipients = curl_slist_append(recipients, CC);
curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
for(cpp = headers_text; *cpp; cpp++)
headers = curl_slist_append(headers, *cpp);
curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
mime = curl_mime_init(curl);
alt = curl_mime_init(curl);
part = curl_mime_addpart(alt);
curl_mime_data(part, inline_html, CURL_ZERO_TERMINATED);
curl_mime_type(part, "text/html");
part = curl_mime_addpart(alt);
curl_mime_data(part, inline_text, CURL_ZERO_TERMINATED);
part = curl_mime_addpart(mime);
curl_mime_subparts(part, alt);
curl_mime_type(part, "multipart/alternative");
slist = curl_slist_append(NULL, "Content-Disposition: inline");
curl_mime_headers(part, slist, 1);
part = curl_mime_addpart(mime);
curl_mime_filedata(part, "smtp-mime.c");
curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
res = curl_easy_perform(curl);
if(res != CURLE_OK)
fprintf(stderr, "curl_easy_perform() failed: %s\n",
curl_easy_strerror(res));
curl_slist_free_all(recipients);
curl_slist_free_all(headers);
curl_easy_cleanup(curl);
curl_mime_free(mime);
}
return (int)res;
}
