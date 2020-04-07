#include <vlc_sout.h>
#include <vlc_network.h>
struct sout_packetizer_input_t
{
sout_instance_t *p_sout;
void *id;
bool b_flushed;
};
sout_instance_t *sout_NewInstance( vlc_object_t *, const char * );
#define sout_NewInstance(a,b) sout_NewInstance(VLC_OBJECT(a),b)
void sout_DeleteInstance( sout_instance_t * );
bool sout_instance_ControlsPace( sout_instance_t *sout );
sout_packetizer_input_t *sout_InputNew( sout_instance_t *, const es_format_t * );
int sout_InputDelete( sout_packetizer_input_t * );
int sout_InputSendBuffer( sout_packetizer_input_t *, block_t* );
bool sout_InputIsEmpty(sout_packetizer_input_t *);
enum sout_input_query_e
{
SOUT_INPUT_SET_SPU_HIGHLIGHT, 
};
int sout_InputControl( sout_packetizer_input_t *, int i_query, ... );
void sout_InputFlush( sout_packetizer_input_t * );
