#include <vector>
#include <string>
#include <map>
namespace adaptive
{
namespace xml
{
class Node
{
public:
Node ();
virtual ~Node ();
const std::vector<Node *>& getSubNodes () const;
void addSubNode (Node *node);
const std::string& getName () const;
void setName (const std::string& name);
bool hasAttribute (const std::string& name) const;
void addAttribute (const std::string& key, const std::string& value);
const std::string& getAttributeValue (const std::string& key) const;
std::vector<std::string> getAttributeKeys () const;
const std::string& getText () const;
void setText( const std::string &text );
const std::map<std::string, std::string>& getAttributes () const;
int getType() const;
void setType( int type );
std::vector<std::string> toString(int) const;
private:
static const std::string EmptyString;
std::vector<Node *> subNodes;
std::map<std::string, std::string> attributes;
std::string name;
std::string text;
int type;
};
}
}
