#include <string>
#include <map>
namespace dash
{
namespace mpd
{
class ProgramInformation
{
public:
virtual ~ProgramInformation(){}
const std::string& getMoreInformationUrl() const;
void setMoreInformationUrl( const std::string &url );
const std::string& getTitle() const;
void setTitle( const std::string &title);
const std::string& getSource() const;
void setSource( const std::string &source);
const std::string& getCopyright() const;
void setCopyright( const std::string &copyright);
private:
std::string moreInformationUrl;
std::string title;
std::string source;
std::string copyright;
};
}
}
