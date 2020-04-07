




















#include "libavutil/avutil.h"
#include "libavutil/avstring.h"
#include "libavutil/opt.h"

#include "tee_common.h"

static const char *const slave_opt_open = "[";
static const char *const slave_opt_close = "]";
static const char *const slave_opt_delim = ":]"; 

int ff_tee_parse_slave_options(void *log, char *slave,
AVDictionary **options, char **filename)
{
const char *p;
char *key, *val;
int ret;

if (!strspn(slave, slave_opt_open)) {
*filename = slave;
return 0;
}
p = slave + 1;
if (strspn(p, slave_opt_close)) {
*filename = (char *)p + 1;
return 0;
}
while (1) {
ret = av_opt_get_key_value(&p, "=", slave_opt_delim, 0, &key, &val);
if (ret < 0) {
av_log(log, AV_LOG_ERROR, "No option found near \"%s\"\n", p);
goto fail;
}
ret = av_dict_set(options, key, val,
AV_DICT_DONT_STRDUP_KEY | AV_DICT_DONT_STRDUP_VAL);
if (ret < 0)
goto fail;
if (strspn(p, slave_opt_close))
break;
p++;
}
*filename = (char *)p + 1;
return 0;

fail:
av_dict_free(options);
return ret;
}
