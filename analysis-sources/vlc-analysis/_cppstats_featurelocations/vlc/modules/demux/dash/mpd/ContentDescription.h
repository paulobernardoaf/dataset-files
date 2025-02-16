























#if !defined(CONTENTDESCRIPTION_H_)
#define CONTENTDESCRIPTION_H_

#include <string>

namespace dash
{
namespace mpd
{
class ContentDescription
{
public:
const std::string& getSchemeIdUri() const;
void setSchemeIdUri( const std::string &uri );
const std::string& getSchemeInformation() const;
void setSchemeInformation( const std::string &schemeInfo );

private:
std::string schemeIdUri;
std::string schemeInformation;
};
}
}

#endif 
