struct _utimbuf
{
time_t actime; 
time_t modtime; 
};
#if !defined(_NO_OLDNAMES)
struct utimbuf
{
time_t actime;
time_t modtime;
};
#endif 
