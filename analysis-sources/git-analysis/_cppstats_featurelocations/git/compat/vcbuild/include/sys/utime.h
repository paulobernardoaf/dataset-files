#if !defined(_UTIME_H_)
#define _UTIME_H_
















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

#endif
