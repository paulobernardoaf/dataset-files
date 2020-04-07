static NSMenu *o_keys_menu = nil;
@interface VLCConfigControl : NSView
@property (readonly) module_config_t *p_item;
@property (readwrite) NSTextField *label;
@property (readonly) NSString *name;
@property (readonly) int type;
@property (readwrite) int viewType;
@property (readonly) int intValue;
@property (readonly) float floatValue;
@property (readonly) char *stringValue;
@property (readonly) int labelSize;
+ (VLCConfigControl *)newControl:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
+ (int)calcVerticalMargin:(int)i_curItem lastItem:(int)i_lastItem;
- (id)initWithFrame:(NSRect)frame
item:(module_config_t *)p_item;
- (void)setYPos:(int)i_yPos;
- (void)alignWithXPosition:(int)i_xPos;
- (void)applyChanges;
- (void)resetValues;
@end
@interface StringConfigControl : VLCConfigControl
- (id)initWithItem:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
@end
@interface StringListConfigControl : VLCConfigControl
- (id)initWithItem:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
@end
@interface FileConfigControl : VLCConfigControl
- (id)initWithItem:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
- (IBAction)openFileDialog:(id)sender;
@end
@interface ModuleConfigControl : VLCConfigControl
- (id)initWithItem:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
@end
@interface IntegerConfigControl : VLCConfigControl
- (id)initWithItem:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
- (IBAction)stepperChanged:(id)sender;
@end
@interface IntegerListConfigControl : VLCConfigControl
- (id)initWithItem:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
@end
@interface RangedIntegerConfigControl : VLCConfigControl
- (id)initWithItem:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
- (IBAction)sliderChanged:(id)sender;
@end
@interface BoolConfigControl : VLCConfigControl
- (id)initWithItem:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
@end
@interface FloatConfigControl : VLCConfigControl
- (id)initWithItem:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
- (IBAction)stepperChanged:(id)sender;
@end
@interface RangedFloatConfigControl : VLCConfigControl
- (id)initWithItem:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
- (IBAction)sliderChanged:(id)sender;
@end
@interface KeyConfigControl : VLCConfigControl
- (id)initWithItem:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
@end
@interface ModuleListConfigControl : VLCConfigControl
- (id)initWithItem:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
@end
@interface SectionControl : VLCConfigControl
- (id)initWithItem:(module_config_t *)_p_item
withView:(NSView *)o_parent_view;
@end
