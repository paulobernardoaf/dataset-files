#include <string>
namespace adaptive
{
namespace playlist
{
class CommonAttributesElements
{
public:
CommonAttributesElements();
virtual ~CommonAttributesElements();
virtual const std::string& getMimeType() const;
void setMimeType( const std::string &mimeType );
int getWidth() const;
void setWidth( int width );
int getHeight() const;
void setHeight( int height );
protected:
std::string mimeType;
int width;
int height;
};
}
}
