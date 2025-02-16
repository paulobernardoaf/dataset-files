#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#include <allegro5/allegro.h>
#include <allegro5/internal/aintern_iphone.h>
#include <allegro5/internal/aintern_list.h>
@interface EAGLView : UIView {
@private
EAGLContext *context;
ALLEGRO_DISPLAY *allegro_display;
GLuint viewRenderbuffer, viewFramebuffer;
GLuint depthRenderbuffer;
_AL_LIST* touch_list;
NSMutableIndexSet* touch_id_set;
UITouch* primary_touch;
int next_free_touch_id;
float scale;
}
@property (nonatomic, retain) EAGLContext *context;
@property GLint backingWidth;
@property GLint backingHeight;
- (void)make_current;
- (void)flip;
- (void)reset_framebuffer;
- (void)set_allegro_display:(ALLEGRO_DISPLAY *)display;
- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;
- (BOOL)orientation_supported:(UIInterfaceOrientation)o;
@end
