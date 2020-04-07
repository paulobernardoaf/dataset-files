



















#if !defined(SD_DEVICE_H)
#define SD_DEVICE_H

#include <medialibrary/filesystem/IDevice.h>
#include <vector>

namespace vlc {
namespace medialibrary {

using namespace ::medialibrary::fs;

class SDDevice : public IDevice
{
public:
SDDevice( const std::string& uuid, std::string mrl );

const std::string &uuid() const override;
bool isRemovable() const override;
bool isPresent() const override;
const std::string &mountpoint() const override;
void addMountpoint( std::string mrl ) override;
void removeMountpoint( const std::string& mrl ) override;
std::tuple<bool, std::string> matchesMountpoint( const std::string& mrl ) const override;
std::string relativeMrl( const std::string& absoluteMrl ) const override;
std::string absoluteMrl( const std::string& relativeMrl ) const override;


private:
std::string m_uuid;
std::vector<std::string> m_mountpoints;
};

} 
} 

#endif
