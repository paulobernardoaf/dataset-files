






























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_filter.h> 

#include <vlc_xml.h>
#include <vlc_stream.h>

#include "../platform_fonts.h"

#define ANDROID_SYSTEM_FONTS_NOUGAT "file:///system/etc/fonts.xml"
#define ANDROID_SYSTEM_FONTS_LEGACY "file:///system/etc/system_fonts.xml"
#define ANDROID_FALLBACK_FONTS "file:///system/etc/fallback_fonts.xml"
#define ANDROID_VENDOR_FONTS "file:///vendor/etc/fallback_fonts.xml"
#define ANDROID_FONT_PATH "/system/fonts"

static int Android_ParseFont( filter_t *p_filter, xml_reader_t *p_xml,
vlc_family_t *p_family )
{
bool b_bold = false;
bool b_italic = false;
const char *psz_val = NULL;
const char *psz_attr = NULL;
int i_type = 0;
int i_weight = 0;

while( ( psz_attr = xml_ReaderNextAttr( p_xml, &psz_val ) ) )
{
if( !strcasecmp( "weight", psz_attr ) && psz_val && *psz_val )
i_weight = atoi( psz_val );
else if( !strcasecmp( "style", psz_attr ) && psz_val && *psz_val )
if( !strcasecmp( "italic", psz_val ) )
b_italic = true;
}

if( i_weight == 700 )
b_bold = true;

i_type = xml_ReaderNextNode( p_xml, &psz_val );

if( i_type != XML_READER_TEXT || !psz_val || !*psz_val )
{
msg_Warn( p_filter, "Android_ParseFont: no file name" );
return VLC_EGENERIC;
}

char *psz_fontfile;




if( i_weight == 400 || i_weight == 700 )
if( asprintf( &psz_fontfile, "%s/%s", ANDROID_FONT_PATH, psz_val ) < 0
|| !NewFont( psz_fontfile, 0, b_bold, b_italic, p_family ) )
return VLC_ENOMEM;

return VLC_SUCCESS;
}

static int Android_Nougat_ParseFamily( filter_t *p_filter, xml_reader_t *p_xml )
{
filter_sys_t *p_sys = p_filter->p_sys;
vlc_dictionary_t *p_dict = &p_sys->family_map;
vlc_family_t *p_family = NULL;
const char *psz_val = NULL;
const char *psz_attr = NULL;
const char *psz_name = NULL;
int i_type = 0;

while( ( psz_attr = xml_ReaderNextAttr( p_xml, &psz_val ) ) )
{
if( !strcasecmp( "name", psz_attr ) && psz_val && *psz_val )
{
psz_name = psz_val;
break;
}
}

if( psz_name && *psz_name )
{




char *psz_lc = ToLower( psz_name );
if( unlikely( !psz_lc ) )
return VLC_ENOMEM;

p_family = vlc_dictionary_value_for_key( p_dict, psz_lc );

free( psz_lc );
}

if( p_family == NULL )
{







p_family = NewFamily( p_filter, psz_name, &p_sys->p_families,
&p_sys->family_map, NULL );

if( unlikely( !p_family ) )
return VLC_ENOMEM;
}

while( ( i_type = xml_ReaderNextNode( p_xml, &psz_val ) ) > 0 )
{
switch( i_type )
{
case XML_READER_STARTELEM:
if( !strcasecmp( "font", psz_val ) )
if( Android_ParseFont( p_filter, p_xml, p_family ) == VLC_ENOMEM )
return VLC_ENOMEM;
break;

case XML_READER_ENDELEM:
if( !strcasecmp( "family", psz_val ) )
{
if( strcasestr( p_family->psz_name, FB_NAME ) )
{




vlc_family_t *p_fallback =
NewFamily( p_filter, p_family->psz_name,
NULL, &p_sys->fallback_map, FB_LIST_DEFAULT );

if( unlikely( !p_fallback ) )
return VLC_ENOMEM;

p_fallback->p_fonts = p_family->p_fonts;
}

return VLC_SUCCESS;
}
break;
}
}

msg_Warn( p_filter, "Android_ParseFamily: Corrupt font configuration file" );
return VLC_EGENERIC;
}

static int Android_ParseAlias( filter_t *p_filter, xml_reader_t *p_xml )
{
filter_sys_t *p_sys = p_filter->p_sys;
vlc_dictionary_t *p_dict = &p_sys->family_map;
vlc_family_t *p_dest = NULL;
char *psz_name = NULL;
char *psz_dest = NULL;
const char *psz_val = NULL;
const char *psz_attr = NULL;
int i_weight = 0;
int i_ret = VLC_SUCCESS;

while( ( psz_attr = xml_ReaderNextAttr( p_xml, &psz_val ) ) )
{
if( !strcasecmp( "weight", psz_attr ) && psz_val && *psz_val )
i_weight = atoi( psz_val );
else if( !strcasecmp( "to", psz_attr ) && psz_val && *psz_val )
psz_dest = ToLower( psz_val );
else if( !strcasecmp( "name", psz_attr ) && psz_val && *psz_val )
psz_name = ToLower( psz_val );
}

if( !psz_dest || !psz_name )
{
i_ret = VLC_EGENERIC;
goto done;
}

p_dest = vlc_dictionary_value_for_key( p_dict, psz_dest );

if( p_dest && i_weight == 0 )
if( vlc_dictionary_value_for_key( p_dict, psz_name ) == NULL )
vlc_dictionary_insert( p_dict, psz_name, p_dest );

done:
free( psz_dest );
free( psz_name );
return i_ret;
}

static int Android_Legacy_ParseFamily( filter_t *p_filter, xml_reader_t *p_xml )
{
filter_sys_t *p_sys = p_filter->p_sys;
vlc_dictionary_t *p_dict = &p_sys->family_map;
vlc_family_t *p_family = NULL;
char *psz_lc = NULL;
int i_counter = 0;
bool b_bold = false;
bool b_italic = false;
const char *p_node = NULL;
int i_type = 0;

while( ( i_type = xml_ReaderNextNode( p_xml, &p_node ) ) > 0 )
{
switch( i_type )
{
case XML_READER_STARTELEM:









if( !strcasecmp( "name", p_node ) )
{
i_type = xml_ReaderNextNode( p_xml, &p_node );

if( i_type != XML_READER_TEXT || !p_node || !*p_node )
{
msg_Warn( p_filter, "Android_ParseFamily: empty name" );
continue;
}

psz_lc = ToLower( p_node );
if( unlikely( !psz_lc ) )
return VLC_ENOMEM;

if( !p_family )
{
p_family = vlc_dictionary_value_for_key( p_dict, psz_lc );
if( p_family == kVLCDictionaryNotFound )
{
p_family =
NewFamily( p_filter, psz_lc, &p_sys->p_families, NULL, NULL );

if( unlikely( !p_family ) )
{
free( psz_lc );
return VLC_ENOMEM;
}

}
}

if( vlc_dictionary_value_for_key( p_dict, psz_lc ) == kVLCDictionaryNotFound )
vlc_dictionary_insert( p_dict, psz_lc, p_family );
free( psz_lc );
}






else if( !strcasecmp( "file", p_node ) )
{
i_type = xml_ReaderNextNode( p_xml, &p_node );

if( i_type != XML_READER_TEXT || !p_node || !*p_node )
{
++i_counter;
continue;
}

if( !p_family )
p_family = NewFamily( p_filter, NULL, &p_sys->p_families,
&p_sys->family_map, NULL );

if( unlikely( !p_family ) )
return VLC_ENOMEM;

switch( i_counter )
{
case 0:
b_bold = false;
b_italic = false;
break;
case 1:
b_bold = true;
b_italic = false;
break;
case 2:
b_bold = false;
b_italic = true;
break;
case 3:
b_bold = true;
b_italic = true;
break;
default:
msg_Warn( p_filter, "Android_ParseFamily: too many files" );
return VLC_EGENERIC;
}

char *psz_fontfile = NULL;
if( asprintf( &psz_fontfile, "%s/%s", ANDROID_FONT_PATH, p_node ) < 0
|| !NewFont( psz_fontfile, 0, b_bold, b_italic, p_family ) )
return VLC_ENOMEM;

++i_counter;
}
break;

case XML_READER_ENDELEM:
if( !strcasecmp( "family", p_node ) )
{
if( !p_family )
{
msg_Warn( p_filter, "Android_ParseFamily: empty family" );
return VLC_EGENERIC;
}





if( strcasestr( p_family->psz_name, FB_NAME ) )
{
vlc_family_t *p_fallback =
NewFamily( p_filter, p_family->psz_name,
NULL, &p_sys->fallback_map, FB_LIST_DEFAULT );

if( unlikely( !p_fallback ) )
return VLC_ENOMEM;

p_fallback->p_fonts = p_family->p_fonts;
}

return VLC_SUCCESS;
}
break;
}
}

msg_Warn( p_filter, "Android_ParseOldFamily: Corrupt font configuration file" );
return VLC_EGENERIC;
}

static int Android_ParseSystemFonts( filter_t *p_filter, const char *psz_path,
bool b_new_format )
{
int i_ret = VLC_SUCCESS;
stream_t *p_stream = vlc_stream_NewURL( p_filter, psz_path );

if( !p_stream )
return VLC_EGENERIC;

xml_reader_t *p_xml = xml_ReaderCreate( p_filter, p_stream );

if( !p_xml )
{
vlc_stream_Delete( p_stream );
return VLC_EGENERIC;
}

const char *p_node;
int i_type;
while( ( i_type = xml_ReaderNextNode( p_xml, &p_node ) ) > 0 )
{
if( i_type == XML_READER_STARTELEM && !strcasecmp( "family", p_node ) && b_new_format )
{
if( ( i_ret = Android_Nougat_ParseFamily( p_filter, p_xml ) ) )
break;
}
else if( i_type == XML_READER_STARTELEM && !strcasecmp( "family", p_node ) && !b_new_format )
{
if( ( i_ret = Android_Legacy_ParseFamily( p_filter, p_xml ) ) )
break;
}
else if( i_type == XML_READER_STARTELEM && !strcasecmp( "alias", p_node ) && b_new_format )
{
if( ( i_ret = Android_ParseAlias( p_filter, p_xml ) ) )
break;
}
}

xml_ReaderDelete( p_xml );
vlc_stream_Delete( p_stream );
return i_ret;
}

int Android_Prepare( filter_t *p_filter )
{
if( Android_ParseSystemFonts( p_filter, ANDROID_SYSTEM_FONTS_NOUGAT, true ) != VLC_SUCCESS )
{
if( Android_ParseSystemFonts( p_filter, ANDROID_SYSTEM_FONTS_LEGACY, false ) == VLC_ENOMEM )
return VLC_ENOMEM;
if( Android_ParseSystemFonts( p_filter, ANDROID_FALLBACK_FONTS, false ) == VLC_ENOMEM )
return VLC_ENOMEM;
if( Android_ParseSystemFonts( p_filter, ANDROID_VENDOR_FONTS, false ) == VLC_ENOMEM )
return VLC_ENOMEM;
}

return VLC_SUCCESS;
}

const vlc_family_t *Android_GetFamily( filter_t *p_filter, const char *psz_family )
{
filter_sys_t *p_sys = p_filter->p_sys;
char *psz_lc = ToLower( psz_family );
if( unlikely( !psz_lc ) )
return NULL;

vlc_family_t *p_family =
vlc_dictionary_value_for_key( &p_sys->family_map, psz_lc );

free( psz_lc );

if( p_family == kVLCDictionaryNotFound )
return NULL;

return p_family;
}

vlc_family_t *Android_GetFallbacks( filter_t *p_filter, const char *psz_family,
uni_char_t codepoint )
{
VLC_UNUSED( codepoint );

vlc_family_t *p_fallbacks = NULL;
filter_sys_t *p_sys = p_filter->p_sys;
char *psz_lc = ToLower( psz_family );
if( unlikely( !psz_lc ) )
return NULL;

p_fallbacks = vlc_dictionary_value_for_key( &p_sys->fallback_map, psz_lc );

free( psz_lc );

if( p_fallbacks == kVLCDictionaryNotFound )
return NULL;

return p_fallbacks;
}
