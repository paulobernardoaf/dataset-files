















#include <obs-module.h>
#include <graphics/matrix4.h>
#include <graphics/quat.h>



#define SETTING_GAMMA "gamma"
#define SETTING_CONTRAST "contrast"
#define SETTING_BRIGHTNESS "brightness"
#define SETTING_SATURATION "saturation"
#define SETTING_HUESHIFT "hue_shift"
#define SETTING_OPACITY "opacity"
#define SETTING_COLOR "color"

#define TEXT_GAMMA obs_module_text("Gamma")
#define TEXT_CONTRAST obs_module_text("Contrast")
#define TEXT_BRIGHTNESS obs_module_text("Brightness")
#define TEXT_SATURATION obs_module_text("Saturation")
#define TEXT_HUESHIFT obs_module_text("HueShift")
#define TEXT_OPACITY obs_module_text("Opacity")
#define TEXT_COLOR obs_module_text("Color")



struct color_correction_filter_data {
obs_source_t *context;

gs_effect_t *effect;

gs_eparam_t *gamma_param;
gs_eparam_t *final_matrix_param;

struct vec3 gamma;
float contrast;
float brightness;
float saturation;
float hue_shift;
float opacity;
struct vec4 color;


struct matrix4 con_matrix;
struct matrix4 bright_matrix;
struct matrix4 sat_matrix;
struct matrix4 hue_op_matrix;
struct matrix4 color_matrix;
struct matrix4 final_matrix;

struct vec3 rot_quaternion;
float rot_quaternion_w;
struct vec3 cross;
struct vec3 square;
struct vec3 wimag;
struct vec3 diag;
struct vec3 a_line;
struct vec3 b_line;
struct vec3 half_unit;
};

static const float root3 = 0.57735f;
static const float red_weight = 0.299f;
static const float green_weight = 0.587f;
static const float blue_weight = 0.114f;





static const char *color_correction_filter_name(void *unused)
{
UNUSED_PARAMETER(unused);
return obs_module_text("ColorFilter");
}







static void color_correction_filter_update(void *data, obs_data_t *settings)
{
struct color_correction_filter_data *filter = data;


double gamma = obs_data_get_double(settings, SETTING_GAMMA);
gamma = (gamma < 0.0) ? (-gamma + 1.0) : (1.0 / (gamma + 1.0));
vec3_set(&filter->gamma, (float)gamma, (float)gamma, (float)gamma);


filter->contrast =
(float)obs_data_get_double(settings, SETTING_CONTRAST) + 1.0f;
float one_minus_con = (1.0f - filter->contrast) / 2.0f;


filter->con_matrix = (struct matrix4){filter->contrast,
0.0f,
0.0f,
0.0f,
0.0f,
filter->contrast,
0.0f,
0.0f,
0.0f,
0.0f,
filter->contrast,
0.0f,
one_minus_con,
one_minus_con,
one_minus_con,
1.0f};


filter->brightness =
(float)obs_data_get_double(settings, SETTING_BRIGHTNESS);







filter->bright_matrix.t.x = filter->brightness;
filter->bright_matrix.t.y = filter->brightness;
filter->bright_matrix.t.z = filter->brightness;


filter->saturation =
(float)obs_data_get_double(settings, SETTING_SATURATION) + 1.0f;


float one_minus_sat_red = (1.0f - filter->saturation) * red_weight;
float one_minus_sat_green = (1.0f - filter->saturation) * green_weight;
float one_minus_sat_blue = (1.0f - filter->saturation) * blue_weight;
float sat_val_red = one_minus_sat_red + filter->saturation;
float sat_val_green = one_minus_sat_green + filter->saturation;
float sat_val_blue = one_minus_sat_blue + filter->saturation;


filter->sat_matrix = (struct matrix4){sat_val_red,
one_minus_sat_red,
one_minus_sat_red,
0.0f,
one_minus_sat_green,
sat_val_green,
one_minus_sat_green,
0.0f,
one_minus_sat_blue,
one_minus_sat_blue,
sat_val_blue,
0.0f,
0.0f,
0.0f,
0.0f,
1.0f};


filter->hue_shift =
(float)obs_data_get_double(settings, SETTING_HUESHIFT);


filter->opacity =
(float)obs_data_get_int(settings, SETTING_OPACITY) * 0.01f;


float half_angle = 0.5f * (float)(filter->hue_shift / (180.0f / M_PI));


float rot_quad1 = root3 * (float)sin(half_angle);
vec3_set(&filter->rot_quaternion, rot_quad1, rot_quad1, rot_quad1);
filter->rot_quaternion_w = (float)cos(half_angle);

vec3_mul(&filter->cross, &filter->rot_quaternion,
&filter->rot_quaternion);
vec3_mul(&filter->square, &filter->rot_quaternion,
&filter->rot_quaternion);
vec3_mulf(&filter->wimag, &filter->rot_quaternion,
filter->rot_quaternion_w);

vec3_mulf(&filter->square, &filter->square, 2.0f);
vec3_sub(&filter->diag, &filter->half_unit, &filter->square);
vec3_add(&filter->a_line, &filter->cross, &filter->wimag);
vec3_sub(&filter->b_line, &filter->cross, &filter->wimag);


filter->hue_op_matrix = (struct matrix4){filter->diag.x * 2.0f,
filter->b_line.z * 2.0f,
filter->a_line.y * 2.0f,
0.0f,

filter->a_line.z * 2.0f,
filter->diag.y * 2.0f,
filter->b_line.x * 2.0f,
0.0f,

filter->b_line.y * 2.0f,
filter->a_line.x * 2.0f,
filter->diag.z * 2.0f,
0.0f,

0.0f,
0.0f,
0.0f,
filter->opacity};


uint32_t color = (uint32_t)obs_data_get_int(settings, SETTING_COLOR);
vec4_from_rgba(&filter->color, color);







filter->color_matrix.x.x = filter->color.x;
filter->color_matrix.y.y = filter->color.y;
filter->color_matrix.z.z = filter->color.z;

filter->color_matrix.t.x = filter->color.w * filter->color.x;
filter->color_matrix.t.y = filter->color.w * filter->color.y;
filter->color_matrix.t.z = filter->color.w * filter->color.z;


matrix4_mul(&filter->final_matrix, &filter->bright_matrix,
&filter->con_matrix);

matrix4_mul(&filter->final_matrix, &filter->final_matrix,
&filter->sat_matrix);

matrix4_mul(&filter->final_matrix, &filter->final_matrix,
&filter->hue_op_matrix);

matrix4_mul(&filter->final_matrix, &filter->final_matrix,
&filter->color_matrix);
}






static void color_correction_filter_destroy(void *data)
{
struct color_correction_filter_data *filter = data;

if (filter->effect) {
obs_enter_graphics();
gs_effect_destroy(filter->effect);
obs_leave_graphics();
}

bfree(data);
}







static void *color_correction_filter_create(obs_data_t *settings,
obs_source_t *context)
{






struct color_correction_filter_data *filter =
bzalloc(sizeof(struct color_correction_filter_data));





char *effect_path = obs_module_file("color_correction_filter.effect");

filter->context = context;


vec3_set(&filter->half_unit, 0.5f, 0.5f, 0.5f);
matrix4_identity(&filter->bright_matrix);
matrix4_identity(&filter->color_matrix);


obs_enter_graphics();


filter->effect = gs_effect_create_from_file(effect_path, NULL);


if (filter->effect) {
filter->gamma_param = gs_effect_get_param_by_name(
filter->effect, SETTING_GAMMA);
filter->final_matrix_param = gs_effect_get_param_by_name(
filter->effect, "color_matrix");
}

obs_leave_graphics();

bfree(effect_path);






if (!filter->effect) {
color_correction_filter_destroy(filter);
return NULL;
}






color_correction_filter_update(filter, settings);
return filter;
}


static void color_correction_filter_render(void *data, gs_effect_t *effect)
{
struct color_correction_filter_data *filter = data;

if (!obs_source_process_filter_begin(filter->context, GS_RGBA,
OBS_ALLOW_DIRECT_RENDERING))
return;


gs_effect_set_vec3(filter->gamma_param, &filter->gamma);
gs_effect_set_matrix4(filter->final_matrix_param,
&filter->final_matrix);

obs_source_process_filter_end(filter->context, filter->effect, 0, 0);

UNUSED_PARAMETER(effect);
}







static obs_properties_t *color_correction_filter_properties(void *data)
{
obs_properties_t *props = obs_properties_create();

obs_properties_add_float_slider(props, SETTING_GAMMA, TEXT_GAMMA, -3.0,
3.0, 0.01);

obs_properties_add_float_slider(props, SETTING_CONTRAST, TEXT_CONTRAST,
-2.0, 2.0, 0.01);
obs_properties_add_float_slider(props, SETTING_BRIGHTNESS,
TEXT_BRIGHTNESS, -1.0, 1.0, 0.01);
obs_properties_add_float_slider(props, SETTING_SATURATION,
TEXT_SATURATION, -1.0, 5.0, 0.01);
obs_properties_add_float_slider(props, SETTING_HUESHIFT, TEXT_HUESHIFT,
-180.0, 180.0, 0.01);
obs_properties_add_int_slider(props, SETTING_OPACITY, TEXT_OPACITY, 0,
100, 1);

obs_properties_add_color(props, SETTING_COLOR, TEXT_COLOR);

UNUSED_PARAMETER(data);
return props;
}








static void color_correction_filter_defaults(obs_data_t *settings)
{
obs_data_set_default_double(settings, SETTING_GAMMA, 0.0);
obs_data_set_default_double(settings, SETTING_CONTRAST, 0.0);
obs_data_set_default_double(settings, SETTING_BRIGHTNESS, 0.0);
obs_data_set_default_double(settings, SETTING_SATURATION, 0.0);
obs_data_set_default_double(settings, SETTING_HUESHIFT, 0.0);
obs_data_set_default_double(settings, SETTING_OPACITY, 100.0);
obs_data_set_default_int(settings, SETTING_COLOR, 0xFFFFFF);
}











struct obs_source_info color_filter = {
.id = "color_filter",
.type = OBS_SOURCE_TYPE_FILTER,
.output_flags = OBS_SOURCE_VIDEO,
.get_name = color_correction_filter_name,
.create = color_correction_filter_create,
.destroy = color_correction_filter_destroy,
.video_render = color_correction_filter_render,
.update = color_correction_filter_update,
.get_properties = color_correction_filter_properties,
.get_defaults = color_correction_filter_defaults,
};
