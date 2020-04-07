






















#if !defined(_INPUT_ART_H)
#define _INPUT_ART_H 1

int input_FindArtInCache( input_item_t * );
int input_FindArtInCacheUsingItemUID( input_item_t * );

int input_SaveArt( vlc_object_t *, input_item_t *,
const void *, size_t, const char *psz_type );

#endif

