#include <string>
#include <list>
namespace adaptive
{
class Helper
{
public:
static std::string combinePaths (const std::string &path1, const std::string &path2);
static std::string getDirectoryPath (const std::string &path);
static std::string getFileExtension (const std::string &uri);
static bool icaseEquals (std::string str1, std::string str2);
static bool ifind (std::string haystack, std::string needle);
static std::list<std::string> tokenize(const std::string &, char);
};
}
