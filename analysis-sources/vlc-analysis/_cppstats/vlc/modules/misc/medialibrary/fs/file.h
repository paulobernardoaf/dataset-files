#include <medialibrary/filesystem/IFile.h>
namespace vlc {
namespace medialibrary {
using namespace ::medialibrary::fs;
class SDFile : public IFile
{
public:
explicit SDFile(const std::string &mrl);
virtual ~SDFile() = default;
const std::string& mrl() const override;
const std::string& name() const override;
const std::string& extension() const override;
unsigned int lastModificationDate() const override;
int64_t size() const override;
inline bool isNetwork() const override { return true; }
private:
std::string m_mrl;
std::string m_name;
std::string m_extension;
};
} 
} 
