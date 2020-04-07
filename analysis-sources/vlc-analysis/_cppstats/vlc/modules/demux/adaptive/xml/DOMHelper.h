#include <vector>
#include <string>
#include "Node.h"
namespace adaptive
{
namespace xml
{
class DOMHelper
{
public:
static std::vector<Node *> getElementByTagName (Node *root, const std::string& name, bool selfContain);
static std::vector<Node *> getChildElementByTagName (Node *root, const std::string& name);
static Node* getFirstChildElementByName( Node *root, const std::string& name );
private:
static void getElementsByTagName(Node *root, const std::string& name, std::vector<Node *> *elements, bool selfContain);
};
}
}
