#include <vlc_picture.h>
libvlc_picture_t* libvlc_picture_new( vlc_object_t* p_obj, picture_t* p_pic,
libvlc_picture_type_t i_format,
unsigned int i_width, unsigned int i_height,
bool b_crop );
