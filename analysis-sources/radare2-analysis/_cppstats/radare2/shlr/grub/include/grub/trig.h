#define GRUB_TRIG_ANGLE_MAX 256
#define GRUB_TRIG_ANGLE_MASK 255
#define GRUB_TRIG_FRACTION_SCALE 16384
extern short grub_trig_sintab[];
extern short grub_trig_costab[];
static __inline int
grub_sin (int x)
{
x &= GRUB_TRIG_ANGLE_MASK;
return grub_trig_sintab[x];
}
static __inline int
grub_cos (int x)
{
x &= GRUB_TRIG_ANGLE_MASK;
return grub_trig_costab[x];
}
