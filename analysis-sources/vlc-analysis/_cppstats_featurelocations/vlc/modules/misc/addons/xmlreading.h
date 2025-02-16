



















#define BINDNODE(name,target,type)if ( ! strcmp(p_node, name) ){if ( type == TYPE_STRING ) data_pointer.u_data.ppsz = (char**)&target;else if ( type == TYPE_LONG ) data_pointer.u_data.pl = (long*)&target;else data_pointer.u_data.pi = (int*)&target;data_pointer.e_type = type;} else











struct
{
union
{
char ** ppsz;
int * pi;
long * pl;
} u_data;
enum
{
TYPE_NONE, TYPE_STRING, TYPE_INTEGER, TYPE_LONG
} e_type;
} data_pointer = { {NULL}, TYPE_NONE };


static inline int ReadType( const char *value )
{
if ( !strcmp( value, "playlist" ) )
return ADDON_PLAYLIST_PARSER;
else if ( !strcmp( value, "skin" ) )
return ADDON_SKIN2;
else if ( !strcmp( value, "discovery" ) )
return ADDON_SERVICE_DISCOVERY;
else if ( !strcmp( value, "extension" ) )
return ADDON_EXTENSION;
else if ( !strcmp( value, "interface" ) )
return ADDON_INTERFACE;
else if ( !strcmp( value, "meta" ) )
return ADDON_META;
else
return ADDON_UNKNOWN;
}

static inline const char * getTypePsz( int i_type )
{
switch( i_type )
{
case ADDON_PLAYLIST_PARSER:
return "playlist";
case ADDON_SKIN2:
return "skin";
case ADDON_SERVICE_DISCOVERY:
return "discovery";
case ADDON_EXTENSION:
return "extension";
case ADDON_INTERFACE:
return "interface";
case ADDON_META:
return "meta";
case ADDON_UNKNOWN:
default:
return "unknown";
}
}
