

















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>

#if !defined(_DVBPSI_DVBPSI_H_)
#include <dvbpsi/dvbpsi.h>
#endif
#if !defined(_DVBPSI_DEMUX_H_)
#include <dvbpsi/demux.h>
#endif
#include <dvbpsi/psi.h>

#include "ts_decoders.h"

typedef struct
{
DVBPSI_DECODER_COMMON

ts_dvbpsi_rawsections_callback_t pf_callback;
void * p_cb_data;

} ts_dvbpsi_rawtable_decoder_t;

static void ts_dvbpsi_RawSubDecoderGatherSections( dvbpsi_t *p_dvbpsi,
dvbpsi_decoder_t *p_decoder,
dvbpsi_psi_section_t * p_section )
{
dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;
ts_dvbpsi_rawtable_decoder_t *p_tabledec = (ts_dvbpsi_rawtable_decoder_t*)p_decoder;
if ( !p_tabledec )
{
dvbpsi_DeletePSISections( p_section );
return;
}

if ( p_demux->b_discontinuity )
{
dvbpsi_decoder_reset( DVBPSI_DECODER(p_decoder), true );
p_tabledec->b_discontinuity = false;
p_demux->b_discontinuity = false;
}
else if( p_decoder->i_last_section_number != p_section->i_last_number )
{
dvbpsi_decoder_reset( DVBPSI_DECODER(p_decoder), true );
}


(void) dvbpsi_decoder_psi_section_add( DVBPSI_DECODER(p_decoder), p_section );
p_decoder->i_last_section_number = p_section->i_last_number;


if ( dvbpsi_decoder_psi_sections_completed( DVBPSI_DECODER(p_decoder) ) )
{
p_tabledec->b_current_valid = true;

p_tabledec->pf_callback( p_dvbpsi,
p_decoder->p_sections,
p_tabledec->p_cb_data );


dvbpsi_decoder_reset( DVBPSI_DECODER(p_decoder), false );
}
}

static void ts_dvbpsi_RawDecoderGatherSections( dvbpsi_t *p_dvbpsi,
dvbpsi_psi_section_t * p_section )
{
ts_dvbpsi_RawSubDecoderGatherSections( p_dvbpsi, p_dvbpsi->p_decoder, p_section );
}

void ts_dvbpsi_DetachRawSubDecoder( dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension )
{
dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;

dvbpsi_demux_subdec_t* p_subdec;
p_subdec = dvbpsi_demuxGetSubDec( p_demux, i_table_id, i_extension );
if ( p_subdec == NULL || !p_subdec->p_decoder )
return;

dvbpsi_DetachDemuxSubDecoder( p_demux, p_subdec );
dvbpsi_DeleteDemuxSubDecoder( p_subdec );
}

bool ts_dvbpsi_AttachRawSubDecoder( dvbpsi_t* p_dvbpsi,
uint8_t i_table_id, uint16_t i_extension,
ts_dvbpsi_rawsections_callback_t pf_callback,
void *p_cb_data )
{
dvbpsi_demux_t *p_demux = (dvbpsi_demux_t*)p_dvbpsi->p_decoder;
if ( dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension) )
return false;

ts_dvbpsi_rawtable_decoder_t *p_decoder;
p_decoder = (ts_dvbpsi_rawtable_decoder_t *) dvbpsi_decoder_new( NULL, 0, true, sizeof(*p_decoder) );
if ( p_decoder == NULL )
return false;


dvbpsi_demux_subdec_t* p_subdec;
p_subdec = dvbpsi_NewDemuxSubDecoder( i_table_id, i_extension,
ts_dvbpsi_DetachRawSubDecoder,
ts_dvbpsi_RawSubDecoderGatherSections,
DVBPSI_DECODER(p_decoder) );
if (p_subdec == NULL)
{
dvbpsi_decoder_delete( DVBPSI_DECODER(p_decoder) );
return false;
}


dvbpsi_AttachDemuxSubDecoder( p_demux, p_subdec );

p_decoder->pf_callback = pf_callback;
p_decoder->p_cb_data = p_cb_data;

return true;
}

bool ts_dvbpsi_AttachRawDecoder( dvbpsi_t* p_dvbpsi,
ts_dvbpsi_rawsections_callback_t pf_callback,
void *p_cb_data )
{
if ( p_dvbpsi->p_decoder )
return false;

ts_dvbpsi_rawtable_decoder_t *p_decoder = dvbpsi_decoder_new( NULL, 4096, true, sizeof(*p_decoder) );
if ( p_decoder == NULL )
return false;
p_dvbpsi->p_decoder = DVBPSI_DECODER(p_decoder);

p_decoder->pf_gather = ts_dvbpsi_RawDecoderGatherSections;
p_decoder->pf_callback = pf_callback;
p_decoder->p_cb_data = p_cb_data;

return true;
}

void ts_dvbpsi_DetachRawDecoder( dvbpsi_t *p_dvbpsi )
{
if( dvbpsi_decoder_present( p_dvbpsi ) )
dvbpsi_decoder_delete( p_dvbpsi->p_decoder );
p_dvbpsi->p_decoder = NULL;
}
