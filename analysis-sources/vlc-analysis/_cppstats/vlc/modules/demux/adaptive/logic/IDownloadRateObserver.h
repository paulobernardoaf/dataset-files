#include <vlc_common.h>
namespace adaptive
{
class ID;
class IDownloadRateObserver
{
public:
virtual void updateDownloadRate(const ID &, size_t, vlc_tick_t) = 0;
virtual ~IDownloadRateObserver(){}
};
}
