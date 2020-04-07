#if !defined(TR2_CFG_H)
#define TR2_CFG_H





void tr2_cfg_list_config_fl(const char *file, int line);





void tr2_cfg_set_fl(const char *file, int line, const char *key,
const char *value);

void tr2_cfg_free_patterns(void);

#endif 
