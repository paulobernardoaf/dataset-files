struct background_worker_config {
vlc_tick_t default_timeout;
int max_threads;
void( *pf_release )( void* entity );
void( *pf_hold )( void* entity );
int( *pf_start )( void* owner, void* entity, void** out );
int( *pf_probe )( void* owner, void* handle );
void( *pf_stop )( void* owner, void* handle );
};
struct background_worker* background_worker_New( void* owner,
struct background_worker_config* config );
void background_worker_RequestProbe( struct background_worker* worker );
int background_worker_Push( struct background_worker* worker, void* entity,
void* id, int timeout );
void background_worker_Cancel( struct background_worker* worker, void* id );
void background_worker_Delete( struct background_worker* worker );
