



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vlc_common.h>
#include <vlc_strings.h>

const char vlc_module_name[] = "test_xmlent";

static void decode (const char *in, const char *out)
{
char buf[strlen (in) + 1];

printf ("\"%s\" -> \"%s\" ?\n", in, out);
strcpy (buf, in);
vlc_xml_decode (buf);

if (strcmp (buf, out))
{
printf (" ERROR: got \"%s\"\n", buf);
exit (2);
}
}

static void encode (const char *in, const char *out)
{
char *buf;

printf ("\"%s\" -> \"%s\" ?\n", in, out);
buf = vlc_xml_encode (in);

if (buf == NULL)
{
puts(" ERROR: got NULL");
exit(2);
}

if (strcmp (buf, out))
{
printf (" ERROR: got \"%s\"\n", buf);
exit (2);
}
free (buf);
}

int main (void)
{
(void)setvbuf (stdout, NULL, _IONBF, 0);
decode ("This should not be modified 1234",
"This should not be modified 1234");

decode ("R&eacute;mi&nbsp;Fran&ccedil;ois&nbsp;&amp;&nbsp;&Eacute;mile",
"Rémi François & Émile");
decode ("R&#233;mi&nbsp;Fran&#231;ois&nbsp;&amp;&nbsp;&#201;mile",
"Rémi François & Émile");
decode ("R&#xe9;mi&nbsp;Fran&#xe7;ois&nbsp;&amp;&nbsp;&#xc9;mile",
"Rémi François & Émile");
decode ("R&#xE9;mi&nbsp;Fran&#xE7;ois&nbsp;&amp;&nbsp;&#xC9;mile",
"Rémi François & Émile");

decode ("", "");


decode ("&<\"'", "&<\"'");
decode ("&oelig", "&oelig");

encode ("", "");
encode ("a'àc\"çe&én<ño>ö1:", "a&#39;àc&quot;çe&amp;én&lt;ño&gt;ö1:");
encode ("\xC2\x81\xC2\x85", "&#129;\xC2\x85");
encode (" \t\r\n", " \t\r\n");

return 0;
}
