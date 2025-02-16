#if defined(__cplusplus)
extern "C" {
#endif
typedef struct libvlc_picture_t libvlc_picture_t;
typedef enum libvlc_picture_type_t
{
libvlc_picture_Argb,
libvlc_picture_Png,
libvlc_picture_Jpg,
} libvlc_picture_type_t;
LIBVLC_API void
libvlc_picture_retain( libvlc_picture_t* pic );
LIBVLC_API void
libvlc_picture_release( libvlc_picture_t* pic );
LIBVLC_API int
libvlc_picture_save( const libvlc_picture_t* pic, const char* path );
LIBVLC_API const unsigned char*
libvlc_picture_get_buffer( const libvlc_picture_t* pic, size_t *size );
LIBVLC_API libvlc_picture_type_t
libvlc_picture_type( const libvlc_picture_t* pic );
LIBVLC_API unsigned int
libvlc_picture_get_stride( const libvlc_picture_t* pic );
LIBVLC_API unsigned int
libvlc_picture_get_width( const libvlc_picture_t* pic );
LIBVLC_API unsigned int
libvlc_picture_get_height( const libvlc_picture_t* pic );
LIBVLC_API libvlc_time_t
libvlc_picture_get_time( const libvlc_picture_t* pic );
#if defined(__cplusplus)
}
#endif
