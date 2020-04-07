



















#if !defined(SD_DIRECTORY_H)
#define SD_DIRECTORY_H

#include <medialibrary/filesystem/IDirectory.h>
#include <medialibrary/filesystem/IFile.h>

#include "fs.h"

namespace vlc {
namespace medialibrary {

using namespace ::medialibrary::fs;

class SDDirectory : public IDirectory
{
public:
explicit SDDirectory(const std::string &mrl, SDFileSystemFactory &fs);
const std::string &mrl() const override;
const std::vector<std::shared_ptr<IFile>> &files() const override;
const std::vector<std::shared_ptr<IDirectory>> &dirs() const override;
std::shared_ptr<IDevice> device() const override;
std::shared_ptr<IFile> file( const std::string& mrl ) const override;

private:
void read() const;

std::string m_mrl;
SDFileSystemFactory &m_fs;

mutable bool m_read_done = false;
mutable std::vector<std::shared_ptr<IFile>> m_files;
mutable std::vector<std::shared_ptr<IDirectory>> m_dirs;
mutable std::shared_ptr<IDevice> m_device;
};

} 
} 

#endif
