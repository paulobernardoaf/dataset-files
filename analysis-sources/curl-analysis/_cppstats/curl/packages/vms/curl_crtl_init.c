#include <stdio.h>
#include <string.h>
#include <descrip.h>
#include <lnmdef.h>
#include <stsdef.h>
#pragma member_alignment save
#pragma nomember_alignment longword
#pragma message save
#pragma message disable misalgndmem
struct itmlst_3 {
unsigned short int buflen;
unsigned short int itmcode;
void *bufadr;
unsigned short int *retlen;
};
#pragma message restore
#pragma member_alignment restore
#if defined(__VAX)
#define ENABLE "ENABLE"
#define DISABLE "DISABLE"
#else
#define ENABLE TRUE
#define DISABLE 0
int decc$feature_get_index (const char *name);
int decc$feature_set_value (int index, int mode, int value);
#endif
int SYS$TRNLNM(
const unsigned long * attr,
const struct dsc$descriptor_s * table_dsc,
struct dsc$descriptor_s * name_dsc,
const unsigned char * acmode,
const struct itmlst_3 * item_list);
int SYS$CRELNM(
const unsigned long * attr,
const struct dsc$descriptor_s * table_dsc,
const struct dsc$descriptor_s * name_dsc,
const unsigned char * acmode,
const struct itmlst_3 * item_list);
static int sys_trnlnm
(const char * logname,
char * value,
int value_len)
{
const $DESCRIPTOR(table_dsc, "LNM$FILE_DEV");
const unsigned long attr = LNM$M_CASE_BLIND;
struct dsc$descriptor_s name_dsc;
int status;
unsigned short result;
struct itmlst_3 itlst[2];
itlst[0].buflen = value_len;
itlst[0].itmcode = LNM$_STRING;
itlst[0].bufadr = value;
itlst[0].retlen = &result;
itlst[1].buflen = 0;
itlst[1].itmcode = 0;
name_dsc.dsc$w_length = strlen(logname);
name_dsc.dsc$a_pointer = (char *)logname;
name_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
name_dsc.dsc$b_class = DSC$K_CLASS_S;
status = SYS$TRNLNM(&attr, &table_dsc, &name_dsc, 0, itlst);
if ($VMS_STATUS_SUCCESS(status)) {
value[result] = '\0';
}
return status;
}
static int sys_crelnm
(const char * logname,
const char * value)
{
int ret_val;
const char * proc_table = "LNM$PROCESS_TABLE";
struct dsc$descriptor_s proc_table_dsc;
struct dsc$descriptor_s logname_dsc;
struct itmlst_3 item_list[2];
proc_table_dsc.dsc$a_pointer = (char *) proc_table;
proc_table_dsc.dsc$w_length = strlen(proc_table);
proc_table_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
proc_table_dsc.dsc$b_class = DSC$K_CLASS_S;
logname_dsc.dsc$a_pointer = (char *) logname;
logname_dsc.dsc$w_length = strlen(logname);
logname_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
logname_dsc.dsc$b_class = DSC$K_CLASS_S;
item_list[0].buflen = strlen(value);
item_list[0].itmcode = LNM$_STRING;
item_list[0].bufadr = (char *)value;
item_list[0].retlen = NULL;
item_list[1].buflen = 0;
item_list[1].itmcode = 0;
ret_val = SYS$CRELNM(NULL, &proc_table_dsc, &logname_dsc, NULL, item_list);
return ret_val;
}
#if defined(__VAX)
static void set_feature_default(const char *name, const char *value)
{
sys_crelnm(name, value);
}
#else
static void set_feature_default(const char *name, int value)
{
int index;
index = decc$feature_get_index(name);
if (index > 0)
decc$feature_set_value (index, 0, value);
}
#endif
static void set_features(void)
{
int status;
char unix_shell_name[255];
int use_unix_settings = 1;
status = sys_trnlnm("GNV$UNIX_SHELL",
unix_shell_name, sizeof unix_shell_name -1);
if (!$VMS_STATUS_SUCCESS(status)) {
use_unix_settings = 0;
}
set_feature_default("DECC$ACL_ACCESS_CHECK", ENABLE);
set_feature_default ("DECC$ARGV_PARSE_STYLE" , ENABLE);
set_feature_default("DECC$DISABLE_POSIX_ROOT", DISABLE);
set_feature_default ("DECC$EFS_CHARSET", ENABLE);
set_feature_default ("DECC$EFS_CASE_PRESERVE", ENABLE);
set_feature_default ("DECC$EFS_FILE_TIMESTAMPS", ENABLE);
set_feature_default ("DECC$ENABLE_GETENV_CACHE", ENABLE);
#if defined(__VAX)
set_feature_default ("DECC$EXEC_FILEATTR_INHERITANCE", "2");
#else
set_feature_default ("DECC$EXEC_FILEATTR_INHERITANCE", 2);
#endif
set_feature_default ("DECC$READDIR_DROPDOTNOTYPE", ENABLE);
set_feature_default ("DECC$STDIO_CTX_EOL", ENABLE);
set_feature_default ("DECC$RENAME_NO_INHERIT", ENABLE);
if (use_unix_settings) {
set_feature_default ("DECC$ALLOW_REMOVE_OPEN_FILES", ENABLE);
set_feature_default ("DECC$FILENAME_UNIX_ONLY", ENABLE);
set_feature_default ("DECC$FILE_PERMISSION_UNIX", ENABLE);
set_feature_default ("DECC$FILE_SHARING", ENABLE);
set_feature_default ("DECC$FILE_OWNER_UNIX", ENABLE);
set_feature_default ("DECC$POSIX_SEEK_STREAM_FILE", ENABLE);
} else {
set_feature_default("DECC$FILENAME_UNIX_REPORT", ENABLE);
}
set_feature_default ("DECC$GLOB_UNIX_STYLE", ENABLE);
set_feature_default("DECC$FILENAME_UNIX_NO_VERSION", ENABLE);
set_feature_default("DECC$UNIX_PATH_BEFORE_LOGNAME", ENABLE);
set_feature_default("DECC$STRTOL_ERANGE", ENABLE);
}
#pragma nostandard
#pragma extern_model save
#if defined(__VAX)
#pragma extern_model strict_refdef "LIB$INITIALIZE" nowrt, long, nopic
#else
#pragma extern_model strict_refdef "LIB$INITIALIZE" nowrt, long
#if __INITIAL_POINTER_SIZE
#pragma __pointer_size __save
#pragma __pointer_size 32
#else
#pragma __required_pointer_size __save
#pragma __required_pointer_size 32
#endif
#endif
void (* const iniarray[])(void) = {set_features, } ;
#if !defined(__VAX)
#if __INITIAL_POINTER_SIZE
#pragma __pointer_size __restore
#else
#pragma __required_pointer_size __restore
#endif
#endif
int LIB$INITIALIZE(void);
#if defined(__DECC)
#pragma extern_model strict_refdef
#endif
int lib_init_ref = (int) LIB$INITIALIZE;
#if defined(__DECC)
#pragma extern_model restore
#pragma standard
#endif
