

























struct vcddev_s
{
char *psz_dev; 


int i_vcdimage_handle; 
vcddev_toc_t toc; 



#if defined(_WIN32)
HANDLE h_device_handle; 
#elif defined( __OS2__ )
HFILE hcd; 
#else
int i_device_handle; 
#endif

};

#if !defined(O_BINARY)
#define O_BINARY 0
#endif

#define VCDDEV_T 1




#if defined( __APPLE__ )
#define darwin_freeTOC( p ) free( (void*)p )
#define CD_MIN_TRACK_NO 01
#define CD_MAX_TRACK_NO 99
#endif

#if defined( _WIN32 )


#if !defined(MAXIMUM_NUMBER_TRACKS)
#define MAXIMUM_NUMBER_TRACKS 100
#endif
typedef struct _TRACK_DATA {
UCHAR Reserved;
UCHAR Control : 4;
UCHAR Adr : 4;
UCHAR TrackNumber;
UCHAR Reserved1;
UCHAR Address[4];
} TRACK_DATA, *PTRACK_DATA;
typedef struct _CDROM_TOC {
UCHAR Length[2];
UCHAR FirstTrack;
UCHAR LastTrack;
TRACK_DATA TrackData[MAXIMUM_NUMBER_TRACKS];
} CDROM_TOC, *PCDROM_TOC;
typedef enum _TRACK_MODE_TYPE {
YellowMode2,
XAForm2,
CDDA
} TRACK_MODE_TYPE, *PTRACK_MODE_TYPE;
typedef struct __RAW_READ_INFO {
LARGE_INTEGER DiskOffset;
ULONG SectorCount;
TRACK_MODE_TYPE TrackMode;
} RAW_READ_INFO, *PRAW_READ_INFO;
typedef struct _CDROM_READ_TOC_EX {
UCHAR Format : 4;
UCHAR Reserved1 : 3;
UCHAR Msf : 1;
UCHAR SessionTrack;
UCHAR Reserved2;
UCHAR Reserved3;
} CDROM_READ_TOC_EX, *PCDROM_READ_TOC_EX;

#if !defined(IOCTL_CDROM_BASE)
#define IOCTL_CDROM_BASE FILE_DEVICE_CD_ROM
#endif
#if !defined(IOCTL_CDROM_READ_TOC)
#define IOCTL_CDROM_READ_TOC CTL_CODE(IOCTL_CDROM_BASE, 0x0000, METHOD_BUFFERED, FILE_READ_ACCESS)

#endif
#if !defined(IOCTL_CDROM_RAW_READ)
#define IOCTL_CDROM_RAW_READ CTL_CODE(IOCTL_CDROM_BASE, 0x000F, METHOD_OUT_DIRECT, FILE_READ_ACCESS)

#endif
#define IOCTL_CDROM_READ_TOC_EX CTL_CODE(IOCTL_CDROM_BASE, 0x0015, METHOD_BUFFERED, FILE_READ_ACCESS)



#define MINIMUM_CDROM_READ_TOC_EX_SIZE 2
#define CDROM_READ_TOC_EX_FORMAT_CDTEXT 0x05

#endif 

#if defined(__OS2__)
#pragma pack( push, 1 )
typedef struct os2_msf_s
{
unsigned char frame;
unsigned char second;
unsigned char minute;
unsigned char reserved;
} os2_msf_t;

typedef struct cdrom_get_tochdr_s
{
unsigned char sign[4];
} cdrom_get_tochdr_t;

typedef struct cdrom_tochdr_s
{
unsigned char first_track;
unsigned char last_track;
os2_msf_t lead_out;
} cdrom_tochdr_t;

typedef struct cdrom_get_track_s
{
unsigned char sign[4];
unsigned char track;
} cdrom_get_track_t;

typedef struct cdrom_track_s
{
os2_msf_t start;
unsigned char adr:4;
unsigned char control:4;
} cdrom_track_t;

typedef struct cdrom_readlong_s
{
unsigned char sign[4];
unsigned char addr_mode;
unsigned short sectors;
unsigned long start;
unsigned char reserved;
unsigned char interleaved_size;
} cdrom_readlong_t;

#pragma pack( pop )
#endif

#define SECTOR_TYPE_MODE2_FORM2 0x14
#define SECTOR_TYPE_CDDA 0x04
#define READ_CD_RAW_MODE2 0xF0
#define READ_CD_USERDATA 0x10




static int OpenVCDImage( vlc_object_t *, const char *, struct vcddev_s * );
static void CloseVCDImage( vlc_object_t *, struct vcddev_s * );

#if defined( __APPLE__ )
static CDTOC *darwin_getTOC( vlc_object_t *, const struct vcddev_s * );
static int darwin_getNumberOfTracks( CDTOC *, int, int *, int * );

#elif defined( _WIN32 )
static int win32_vcd_open( vlc_object_t *, const char *, struct vcddev_s *);

#elif defined( __OS2__ )
static int os2_vcd_open( vlc_object_t *, const char *, struct vcddev_s *);
#endif
