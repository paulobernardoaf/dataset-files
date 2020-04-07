#include "libavutil/pixfmt.h"
double avpriv_get_gamma_from_trc(enum AVColorTransferCharacteristic trc);
typedef double (*avpriv_trc_function)(double);
avpriv_trc_function avpriv_get_trc_function_from_trc(enum AVColorTransferCharacteristic trc);
