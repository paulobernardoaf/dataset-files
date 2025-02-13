#include "libavutil/common.h"
#include "libavutil/frame.h"
#define REFERENCE_WHITE 100.0f
struct LumaCoefficients {
double cr, cg, cb;
};
struct PrimaryCoefficients {
double xr, yr, xg, yg, xb, yb;
};
struct WhitepointCoefficients {
double xw, yw;
};
void ff_matrix_invert_3x3(const double in[3][3], double out[3][3]);
void ff_matrix_mul_3x3(double dst[3][3],
const double src1[3][3], const double src2[3][3]);
void ff_fill_rgb2xyz_table(const struct PrimaryCoefficients *coeffs,
const struct WhitepointCoefficients *wp,
double rgb2xyz[3][3]);
const struct LumaCoefficients *ff_get_luma_coefficients(enum AVColorSpace csp);
void ff_fill_rgb2yuv_table(const struct LumaCoefficients *coeffs,
double rgb2yuv[3][3]);
double ff_determine_signal_peak(AVFrame *in);
void ff_update_hdr_metadata(AVFrame *in, double peak);
