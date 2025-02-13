#include <vlc_common.h>
#include <vlc_fs.h>
#include <vlc_charset.h>
#if defined( _WIN32 )
#include <mmsystem.h>
#elif defined(__linux__)
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <linux/cdrom.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>
#include <scsi/scsi_ioctl.h>
#elif defined (HAVE_DVD_H)
#include <unistd.h>
#include <fcntl.h>
#include <dvd.h>
#endif
#if defined(__linux__)
static int EjectSCSI( int i_fd )
{
struct sdata
{
int inlen;
int outlen;
char cmd[256];
} scsi_cmd;
scsi_cmd.inlen = 0;
scsi_cmd.outlen = 0;
scsi_cmd.cmd[0] = ALLOW_MEDIUM_REMOVAL;
scsi_cmd.cmd[1] = 0;
scsi_cmd.cmd[2] = 0;
scsi_cmd.cmd[3] = 0;
scsi_cmd.cmd[4] = 0;
scsi_cmd.cmd[5] = 0;
if( ioctl( i_fd, SCSI_IOCTL_SEND_COMMAND, (void *)&scsi_cmd ) < 0 )
return VLC_EGENERIC;
scsi_cmd.inlen = 0;
scsi_cmd.outlen = 0;
scsi_cmd.cmd[0] = START_STOP;
scsi_cmd.cmd[1] = 0;
scsi_cmd.cmd[2] = 0;
scsi_cmd.cmd[3] = 0;
scsi_cmd.cmd[4] = 1;
scsi_cmd.cmd[5] = 0;
if( ioctl( i_fd, SCSI_IOCTL_SEND_COMMAND, (void *)&scsi_cmd ) < 0 )
return VLC_EGENERIC;
scsi_cmd.inlen = 0;
scsi_cmd.outlen = 0;
scsi_cmd.cmd[0] = START_STOP;
scsi_cmd.cmd[1] = 0;
scsi_cmd.cmd[2] = 0;
scsi_cmd.cmd[3] = 0;
scsi_cmd.cmd[4] = 2;
scsi_cmd.cmd[5] = 0;
if( ioctl( i_fd, SCSI_IOCTL_SEND_COMMAND, (void *)&scsi_cmd ) < 0 )
return VLC_EGENERIC;
ioctl( i_fd, BLKRRPART );
return VLC_SUCCESS;
}
#endif
#undef intf_Eject
static int intf_Eject( vlc_object_t *p_this, const char *psz_device )
{
VLC_UNUSED(p_this);
#if defined(_WIN32)
MCI_OPEN_PARMS op;
DWORD i_flags;
WCHAR psz_drive[4];
memset( &op, 0, sizeof(MCI_OPEN_PARMS) );
op.lpstrDeviceType = (LPCTSTR)MCI_DEVTYPE_CD_AUDIO;
wcscpy( psz_drive, TEXT("X:") );
psz_drive[0] = psz_device[0];
op.lpstrElementName = psz_drive;
i_flags = MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID |
MCI_OPEN_ELEMENT | MCI_OPEN_SHAREABLE;
if( mciSendCommand( 0, MCI_OPEN, i_flags, (uintptr_t)&op ) )
return VLC_EGENERIC;
mciSendCommand( op.wDeviceID, MCI_SET, MCI_SET_DOOR_OPEN, 0 );
mciSendCommand( op.wDeviceID, MCI_CLOSE, MCI_WAIT, 0 );
return VLC_SUCCESS;
#elif defined (__linux__) || defined (HAVE_DVD_H)
int fd = vlc_open( psz_device, O_RDONLY | O_NONBLOCK );
if( fd == -1 )
{
msg_Err( p_this, "could not open device %s", psz_device );
return VLC_EGENERIC;
}
#if defined(__linux__)
if( ioctl( fd, CDROMEJECT, 0 ) < 0
&& EjectSCSI( fd ) )
#else
if( ioctl( fd, CDROMEJECT, 0 ) < 0 )
#endif
{
msg_Err( p_this, "could not eject %s", psz_device );
vlc_close( fd );
return VLC_EGENERIC;
}
vlc_close( fd );
return VLC_SUCCESS;
#else
VLC_UNUSED( psz_device );
msg_Warn( p_this, "CD-Rom ejection unsupported on this platform" );
return VLC_EGENERIC;
#endif
}
#define intf_Eject(o, p) intf_Eject(VLC_OBJECT(o), p)
