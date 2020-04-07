




















static void* media_list_add_file_path(libvlc_instance_t *vlc, libvlc_media_list_t *ml, const char * file_path)
{
libvlc_media_t *md = libvlc_media_new_location (vlc, file_path);
libvlc_media_list_add_media (ml, md);
libvlc_media_release (md);
return md;
}

