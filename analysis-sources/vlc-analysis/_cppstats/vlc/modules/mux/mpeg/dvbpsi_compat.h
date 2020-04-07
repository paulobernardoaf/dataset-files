static inline void dvbpsi_messages(dvbpsi_t *p_dvbpsi, const dvbpsi_msg_level_t level, const char* msg)
{
vlc_object_t *obj = (vlc_object_t *)p_dvbpsi->p_sys;
switch(level)
{
case DVBPSI_MSG_ERROR:
{
#if DVBPSI_VERSION_INT <= ((1 << 16) + (2 << 8))
if( strncmp( msg, "libdvbpsi (PMT decoder): ", 25 ) ||
( strncmp( &msg[25], "invalid section", 15 ) &&
strncmp( &msg[25], "'program_number' don't match", 28 ) ) )
#endif
msg_Err( obj, "%s", msg ); break;
}
case DVBPSI_MSG_WARN: msg_Warn( obj, "%s", msg ); break;
case DVBPSI_MSG_NONE:
case DVBPSI_MSG_DEBUG:
#if defined(DVBPSI_DEBUG)
msg_Dbg( obj, "%s", msg );
#endif
break;
}
}
