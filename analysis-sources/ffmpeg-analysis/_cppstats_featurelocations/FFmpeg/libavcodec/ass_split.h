




















#if !defined(AVCODEC_ASS_SPLIT_H)
#define AVCODEC_ASS_SPLIT_H




typedef struct {
char *script_type; 
char *collisions; 
int play_res_x; 
int play_res_y; 
float timer; 
} ASSScriptInfo;




typedef struct {
char *name; 
char *font_name; 
int font_size; 
int primary_color; 
int secondary_color;
int outline_color; 
int back_color; 
int bold; 
int italic; 
int underline; 
int strikeout;
float scalex;
float scaley;
float spacing;
float angle;
int border_style;
float outline;
float shadow;
int alignment; 


int margin_l;
int margin_r;
int margin_v;
int alpha_level;
int encoding;
} ASSStyle;




typedef struct {
int readorder;
int layer; 
int start; 
int end; 
char *style; 
char *name;
int margin_l;
int margin_r;
int margin_v;
char *effect;
char *text; 


} ASSDialog;




typedef struct {
ASSScriptInfo script_info; 
ASSStyle *styles; 
int styles_count; 
ASSDialog *dialogs; 
int dialogs_count; 
} ASS;




typedef struct ASSSplitContext ASSSplitContext;








ASSSplitContext *ff_ass_split(const char *buf);














ASSDialog *ff_ass_split_dialog(ASSSplitContext *ctx, const char *buf,
int cache, int *number);




void ff_ass_free_dialog(ASSDialog **dialogp);








ASSDialog *ff_ass_split_dialog2(ASSSplitContext *ctx, const char *buf);






void ff_ass_split_free(ASSSplitContext *ctx);






typedef struct {




void (*text)(void *priv, const char *text, int len);
void (*new_line)(void *priv, int forced);
void (*style)(void *priv, char style, int close);
void (*color)(void *priv, unsigned int , unsigned int color_id);
void (*alpha)(void *priv, int alpha, int alpha_id);
void (*font_name)(void *priv, const char *name);
void (*font_size)(void *priv, int size);
void (*alignment)(void *priv, int alignment);
void (*cancel_overrides)(void *priv, const char *style);






void (*move)(void *priv, int x1, int y1, int x2, int y2, int t1, int t2);
void (*origin)(void *priv, int x, int y);






void (*end)(void *priv);

} ASSCodesCallbacks;










int ff_ass_split_override_codes(const ASSCodesCallbacks *callbacks, void *priv,
const char *buf);








ASSStyle *ff_ass_style_get(ASSSplitContext *ctx, const char *style);

#endif 
