#include <vector>
#include <string>
#include "CommonAttributesElements.h"
#include "SegmentInformation.hpp"
#include "Role.hpp"
#include "../StreamFormat.hpp"
namespace adaptive
{
class ID;
namespace playlist
{
class BaseRepresentation;
class BasePeriod;
class BaseAdaptationSet : public CommonAttributesElements,
public SegmentInformation
{
public:
BaseAdaptationSet(BasePeriod *);
virtual ~BaseAdaptationSet();
virtual StreamFormat getStreamFormat() const; 
std::vector<BaseRepresentation *>& getRepresentations ();
BaseRepresentation * getRepresentationByID(const ID &);
void setSegmentAligned(bool);
bool isSegmentAligned() const;
void setBitswitchAble(bool);
bool isBitSwitchable() const;
void setRole(const Role &);
const Role & getRole() const;
void addRepresentation( BaseRepresentation *rep );
const std::string& getLang() const;
void setLang( const std::string &lang );
void debug(vlc_object_t *,int = 0) const;
Property<std::string> description;
protected:
Role role;
std::vector<BaseRepresentation *> representations;
std::string lang;
Undef<bool> segmentAligned;
Undef<bool> bitswitchAble;
};
}
}
