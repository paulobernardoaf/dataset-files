

















#if !defined(AVFILTER_THREAD_H)
#define AVFILTER_THREAD_H

#include "avfilter.h"

int ff_graph_thread_init(AVFilterGraph *graph);

void ff_graph_thread_free(AVFilterGraph *graph);

#endif 
