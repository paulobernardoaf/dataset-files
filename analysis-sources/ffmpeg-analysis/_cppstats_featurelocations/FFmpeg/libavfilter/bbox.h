



















#if !defined(AVFILTER_BBOX_H)
#define AVFILTER_BBOX_H

#include <stdint.h>

typedef struct FFBoundingBox {
int x1, x2, y1, y2;
} FFBoundingBox;











int ff_calculate_bounding_box(FFBoundingBox *bbox,
const uint8_t *data, int linesize,
int w, int h, int min_val);

#endif 
