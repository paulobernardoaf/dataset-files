#define ACOUSTID_ANON_SERVER "fingerprint.videolan.org"
#define ACOUSTID_ANON_SERVER_PATH "/acoustid.php"
#define MB_ID_SIZE 36
struct acoustid_mb_result_t
{
char *psz_artist;
char *psz_title;
char s_musicbrainz_id[MB_ID_SIZE];
};
typedef struct acoustid_mb_result_t acoustid_mb_result_t;
struct acoustid_result_t
{
double d_score;
char *psz_id;
struct
{
unsigned int count;
acoustid_mb_result_t *p_recordings;
} recordings;
};
typedef struct acoustid_result_t acoustid_result_t;
struct acoustid_results_t
{
acoustid_result_t * p_results;
unsigned int count;
};
typedef struct acoustid_results_t acoustid_results_t;
struct acoustid_fingerprint_t
{
char *psz_fingerprint;
unsigned int i_duration;
acoustid_results_t results;
};
typedef struct acoustid_fingerprint_t acoustid_fingerprint_t;
typedef struct
{
vlc_object_t *p_obj;
char *psz_server;
char *psz_apikey;
} acoustid_config_t;
int acoustid_lookup_fingerprint( const acoustid_config_t *, acoustid_fingerprint_t * );
void acoustid_result_release( acoustid_result_t * );
