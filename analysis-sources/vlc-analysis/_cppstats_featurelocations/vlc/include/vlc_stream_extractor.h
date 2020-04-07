



















#if !defined(VLC_STREAM_EXTRACTOR_H)
#define VLC_STREAM_EXTRACTOR_H

#if defined(__cplusplus)
extern "C" {
#endif























typedef struct stream_extractor_t {
struct vlc_object_t obj;









ssize_t (*pf_read)(struct stream_extractor_t*, void* buf, size_t len);
block_t* (*pf_block)(struct stream_extractor_t*, bool* eof);
int (*pf_seek)(struct stream_extractor_t*, uint64_t);
int (*pf_control)(struct stream_extractor_t*, int request, va_list args);


char const* identifier; 
stream_t* source; 
void* p_sys; 

} stream_extractor_t;

typedef struct stream_directory_t {
struct vlc_object_t obj;









int (*pf_readdir)(struct stream_directory_t*, input_item_node_t* );


stream_t* source; 
void* p_sys; 

} stream_directory_t;



















VLC_API stream_t * vlc_stream_NewMRL(vlc_object_t *obj, const char *mrl)
VLC_USED;
#define vlc_stream_NewMRL(a, b) vlc_stream_NewMRL(VLC_OBJECT(a), b)
















VLC_API char* vlc_stream_extractor_CreateMRL( stream_directory_t*,
char const* subentry );






















VLC_API int vlc_stream_extractor_Attach( stream_t** source,
char const* identifier,
char const* module_name );

VLC_API int vlc_stream_directory_Attach( stream_t** source,
char const* module_name );








#if defined(__cplusplus)
} 
#endif
#endif 
