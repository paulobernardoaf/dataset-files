typedef struct VCONTROLLER VCONTROLLER;
struct VCONTROLLER {
int button[8];
void *private_data;
void (*poll) (VCONTROLLER *);
void (*read_config) (VCONTROLLER *, const char *);
void (*write_config) (VCONTROLLER *, const char *);
int (*calibrate_button) (VCONTROLLER *, int);
const char *(*get_button_description) (VCONTROLLER *, int);
};
void destroy_vcontroller(VCONTROLLER * controller,
const char *config_path);
