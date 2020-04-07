





















#if !defined(VLC_MMS_BUFFER_H_)
#define VLC_MMS_BUFFER_H_

typedef struct
{
uint8_t *p_data; 
int i_data; 


int i_size; 
} var_buffer_t;




int var_buffer_initwrite( var_buffer_t *p_buf, int i_default_size );
int var_buffer_reinitwrite( var_buffer_t *p_buf, int i_default_size );
void var_buffer_add8 ( var_buffer_t *p_buf, uint8_t i_byte );
void var_buffer_add16( var_buffer_t *p_buf, uint16_t i_word );
void var_buffer_add32( var_buffer_t *p_buf, uint32_t i_word );
void var_buffer_add64( var_buffer_t *p_buf, uint64_t i_word );
void var_buffer_addmemory( var_buffer_t *p_buf, void *p_mem, int i_mem );
void var_buffer_addUTF16( stream_t *p_access, var_buffer_t *p_buf, const char *p_str );
void var_buffer_free( var_buffer_t *p_buf );


void var_buffer_initread( var_buffer_t *p_buf, void *p_data, int i_data );
uint8_t var_buffer_get8 ( var_buffer_t *p_buf );
uint16_t var_buffer_get16( var_buffer_t *p_buf );
uint32_t var_buffer_get32( var_buffer_t *p_buf );
uint64_t var_buffer_get64( var_buffer_t *p_buf );
int var_buffer_getmemory ( var_buffer_t *p_buf, void *p_mem, int64_t i_mem );
int var_buffer_readempty( var_buffer_t *p_buf );
void var_buffer_getguid( var_buffer_t *p_buf, vlc_guid_t *p_guid );

#endif
