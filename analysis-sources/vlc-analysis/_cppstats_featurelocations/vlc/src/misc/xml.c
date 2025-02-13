





















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_xml.h>
#include <vlc_modules.h>
#include "../libvlc.h"

#undef xml_Create






xml_t *xml_Create( vlc_object_t *p_this )
{
xml_t *p_xml;

p_xml = vlc_custom_create( p_this, sizeof( *p_xml ), "xml" );

p_xml->p_module = module_need( p_xml, "xml", NULL, false );
if( !p_xml->p_module )
{
vlc_object_delete(p_xml);
msg_Err( p_this, "XML provider not found" );
return NULL;
}

return p_xml;
}




void xml_Delete( xml_t *p_xml )
{
module_unneed( p_xml, p_xml->p_module );
vlc_object_delete(p_xml);
}


#undef xml_ReaderCreate






xml_reader_t *xml_ReaderCreate(vlc_object_t *obj, stream_t *stream)
{
xml_reader_t *reader;

reader = vlc_custom_create(obj, sizeof(*reader), "xml reader");

reader->p_stream = stream;
reader->p_module = module_need(reader, "xml reader", NULL, false);
if (unlikely(reader->p_module == NULL))
{
msg_Err(reader, "XML reader not found");
vlc_object_delete(reader);
return NULL;
}
return reader;
}






void xml_ReaderDelete(xml_reader_t *reader)
{
module_unneed(reader, reader->p_module);
vlc_object_delete(reader);
}
