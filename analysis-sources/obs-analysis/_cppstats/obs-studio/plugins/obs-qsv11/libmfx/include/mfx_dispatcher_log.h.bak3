





























#if !defined(__MFX_DISPATCHER_LOG_H)
#define __MFX_DISPATCHER_LOG_H



#define DL_INFO 1
#define DL_WRN 2
#define DL_ERROR 4
#define DL_LOADED_LIBRARY 8


enum
{
DL_EVENT_START = 1,
DL_EVENT_STOP,
DL_EVENT_MSG
};

#define DL_SINK_NULL 0
#define DL_SINK_PRINTF 1
#define DL_SINK_IMsgHandler 2

#define MFXFOURCCTYPE() "%c%c%c%c"
#define ZERO_OR_SPACE(value) ((0==(value)) ? '0' : (value))
#define MFXU32TOFOURCC(mfxu32)ZERO_OR_SPACE((char)(mfxu32 & 0xFF)), ZERO_OR_SPACE((char)((mfxu32 >> 8) & 0xFF)),ZERO_OR_SPACE((char)((mfxu32 >> 16) & 0xFF)),ZERO_OR_SPACE((char)((mfxu32 >> 24) & 0xFF))





#define MFXGUIDTYPE() "%X-%X-%X-%X-%X-%X-%X-%X-%X-%X-%X-%X-%X-%X-%X-%X"

#define MFXGUIDTOHEX(guid)(guid)->Data[0],(guid)->Data[1],(guid)->Data[2],(guid)->Data[3],(guid)->Data[4],(guid)->Data[5],(guid)->Data[6],(guid)->Data[7],(guid)->Data[8],(guid)->Data[9],(guid)->Data[10],(guid)->Data[11],(guid)->Data[12],(guid)->Data[13],(guid)->Data[14],(guid)->Data[15]

















#if defined(MFX_DISPATCHER_LOG)





#define DISPATCHER_LOG_USE_FORMATING 1




#define DISPATCHER_LOG_HEAP_SINGLETONES

#if defined(_WIN32) || defined(_WIN64)

#define DISPATCHER_LOG_EVENT_GUID L"{EB0538CC-4FEE-484d-ACEE-1182E9F37A57}"






#define DISPACTHER_LOG_FW_PATH "c:\\dispatcher.log"

#endif 

#include <stdio.h>
#include <stdarg.h>


class IMsgHandler
{
public:
virtual ~IMsgHandler(){}
virtual void Write(int level, int opcode, const char * msg, va_list argptr) = 0;
};

#if defined(_WIN32) || defined(_WIN64)
#if DISPATCHER_LOG_USE_FORMATING

#define DISPATCHER_LOG(lvl, opcode, str){DispatcherLogBracketsHelper wrt(lvl,opcode);wrt.Write str;}




#else
#define DISPATCHER_LOG_VA_ARGS(...) wrt.Write(__VA_ARGS__, NULL)


#define DISPATCHER_LOG(lvl, opcode, str) {DispatcherLogBracketsHelper wrt(lvl, opcode);DISPATCHER_LOG_VA_ARGS str;}




#endif

#define DISPATCHER_LOG_OPERATION(operation) operation
#else
#define DISPATCHER_LOG(lvl, opcode, str)
#define DISPATCHER_LOG_OPERATION(operation)
#endif

#define __name_from_line( name, line ) name ##line
#define _name_from_line( name , line) __name_from_line( name, line ) 
#define name_from_line( name ) _name_from_line( name, __LINE__) 


#define DISPATCHER_LOG_AUTO(lvl, msg)DispatchLogBlockHelper name_from_line(__auto_log_)(lvl); name_from_line(__auto_log_).Write msg;


#include <memory>
#include <map>
#include <list>
#include <string>

template <class T> 
class DSSingleTone
{
public:
template <class TParam1>
inline static T & get(TParam1 par1)
{
T * pstored;
if (NULL == (pstored = store_or_load()))
{
return *store_or_load(new T(par1));
}
return *pstored; 
}

inline static T & get()
{
T * pstored;
if (NULL == (pstored = store_or_load()))
{
return *store_or_load(new T());
}
return *pstored;
}
private:


inline static T * store_or_load(T * obj = NULL)
{
static std::auto_ptr<T> instance;
if (NULL != obj)
{
instance.reset(obj);
}
return instance.get();
}
};

class DispatchLog 
: public DSSingleTone<DispatchLog>
{
friend class DSSingleTone<DispatchLog>;
std::list<IMsgHandler*>m_Recepients;
int m_DispatcherLogSink;

public:

void SetSink(int nsink, IMsgHandler *pHandler);
void AttachSink(int nsink, IMsgHandler *pHandler);
void DetachSink(int nsink, IMsgHandler *pHandler);
void ExchangeSink(int nsink, IMsgHandler *pOld, IMsgHandler *pNew);
void DetachAllSinks();
void Write(int level, int opcode, const char * msg, va_list argptr);

protected:
DispatchLog();
};


struct DispatcherLogBracketsHelper
{
int m_level;
int m_opcode;
DispatcherLogBracketsHelper(int level, int opcode)
:m_level(level)
,m_opcode(opcode)
{
}
void Write(const char * str, ...);
} ;


struct DispatchLogBlockHelper
{
int m_level;
void Write(const char * str, ...);
DispatchLogBlockHelper (int level)
: m_level(level)
{
}
~DispatchLogBlockHelper();
};


#if defined(_WIN32) || defined(_WIN64)
#if defined(DISPATCHER_LOG_REGISTER_EVENT_PROVIDER)
class ETWHandlerFactory
: public DSSingleTone<ETWHandlerFactory>
{
friend class DSSingleTone<ETWHandlerFactory>;
typedef std::map<std::wstring, IMsgHandler*> _storage_type;
_storage_type m_storage;

public:
~ETWHandlerFactory();
IMsgHandler *GetSink(const wchar_t* sguid = DISPATCHER_LOG_EVENT_GUID);

protected:
ETWHandlerFactory(){}
};
#endif
#endif 

#if defined(DISPATCHER_LOG_REGISTER_FILE_WRITER)
class FileSink 
: public DSSingleTone<FileSink>
, public IMsgHandler
{
friend class DSSingleTone<FileSink>;
public:
virtual void Write(int level, int opcode, const char * msg, va_list argptr);
~FileSink()
{
if (NULL != m_hdl)
fclose(m_hdl);
}
private:
FILE * m_hdl;
FileSink(const std::string & log_file)
{
#if defined(_WIN32) || defined(_WIN64)
fopen_s(&m_hdl, log_file.c_str(), "a");
#else
m_hdl = fopen(log_file.c_str(), "a");
#endif
}

};
#endif



std::string DispatcherLog_GetMFXImplString(int impl);
const char *DispatcherLog_GetMFXStatusString(int sts);

#else 

#define DISPATCHER_LOG(level, opcode, message)
#define DISPATCHER_LOG_AUTO(level, message)
#define DISPATCHER_LOG_OPERATION(operation) 

#endif


#define DISPATCHER_LOG_INFO(msg) DISPATCHER_LOG(DL_INFO, DL_EVENT_MSG, msg)
#define DISPATCHER_LOG_WRN(msg) DISPATCHER_LOG(DL_WRN, DL_EVENT_MSG, msg)
#define DISPATCHER_LOG_ERROR(msg) DISPATCHER_LOG(DL_ERROR, DL_EVENT_MSG, msg)
#define DISPATCHER_LOG_LIBRARY(msg) DISPATCHER_LOG(DL_LOADED_LIBRARY, DL_EVENT_MSG, msg)
#define DISPATCHER_LOG_BLOCK(msg) DISPATCHER_LOG_AUTO(DL_INFO, msg)

#endif 
