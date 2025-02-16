






















#if !defined(VLC_CDROM_H)
#define VLC_CDROM_H

enum {
CDDA_TYPE = 0,
VCD_TYPE = 1,
};


#define CD_RAW_SECTOR_SIZE 2352
#define CD_ROM_MODE1_DATA_SIZE 2048
#define CD_ROM_MODE2_DATA_SIZE 2336

#define CD_ROM_XA_MODE2_F1_DATA_SIZE 2048
#define CD_ROM_XA_MODE2_F2_DATA_SIZE 2324

#define CD_ROM_XA_FRAMES 75
#define CD_ROM_XA_INTERVAL ((60 + 90 + 2) * CD_ROM_XA_FRAMES)


#define CD_ROM_DATA_FLAG 0x04


#define CD_SECTOR_SIZE CD_ROM_MODE1_DATA_SIZE


#define VCD_DATA_START 24

#define VCD_DATA_SIZE CD_ROM_XA_MODE2_F2_DATA_SIZE

#define VCD_SECTOR_SIZE CD_RAW_SECTOR_SIZE

#define VCD_ENTRIES_SECTOR 151


#define CDDA_DATA_START 0

#define CDDA_DATA_SIZE CD_RAW_SECTOR_SIZE

#define CDDA_SECTOR_SIZE CD_RAW_SECTOR_SIZE




static inline int MSF_TO_LBA(uint8_t min, uint8_t sec, uint8_t frame)
{
return (int)(frame + 75 * (sec + 60 * min));
}
static inline int MSF_TO_LBA2(uint8_t min, uint8_t sec, uint8_t frame)
{
return (int)(frame + 75 * (sec -2 + 60 * min));
}


#define BCD_TO_BIN(i) (uint8_t)((uint8_t)(0xf & (uint8_t)i)+((uint8_t)10*((uint8_t)i >> 4)))


typedef struct vcddev_s vcddev_t;
typedef struct
{
int i_lba;
int i_control;
} vcddev_sector_t;

typedef struct
{
int i_tracks;
vcddev_sector_t *p_sectors;
int i_first_track;
int i_last_track;
} vcddev_toc_t;

static inline vcddev_toc_t * vcddev_toc_New( void )
{
return calloc(1, sizeof(vcddev_toc_t));
}

static inline void vcddev_toc_Reset( vcddev_toc_t *toc )
{
free(toc->p_sectors);
memset(toc, 0, sizeof(*toc));
}

static inline void vcddev_toc_Free( vcddev_toc_t *toc )
{
free(toc->p_sectors);
free(toc);
}




typedef struct msf_s
{
uint8_t minute;
uint8_t second;
uint8_t frame;
} msf_t;




typedef struct entries_sect_s
{
char psz_id[8]; 
uint8_t i_version; 

uint8_t i_sys_prof_tag; 

uint16_t i_entries_nb; 

struct
{
uint8_t i_track; 
msf_t msf; 

} entry[500];
uint8_t zeros[36]; 
} entries_sect_t;




vcddev_t *ioctl_Open ( vlc_object_t *, const char * );
void ioctl_Close ( vlc_object_t *, vcddev_t * );
vcddev_toc_t * ioctl_GetTOC ( vlc_object_t *, const vcddev_t *, bool );
int ioctl_ReadSectors ( vlc_object_t *, const vcddev_t *,
int, uint8_t *, int, int );



int ioctl_GetCdText( vlc_object_t *, const vcddev_t *,
vlc_meta_t ***ppp_tracks, int *pi_tracks );

#endif 
