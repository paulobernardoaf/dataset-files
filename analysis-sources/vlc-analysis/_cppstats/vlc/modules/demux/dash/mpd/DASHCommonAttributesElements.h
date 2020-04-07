#include <list>
#include <string>
namespace dash
{
namespace mpd
{
class ContentDescription;
class DASHCommonAttributesElements
{
public:
DASHCommonAttributesElements();
virtual ~DASHCommonAttributesElements();
int getParX() const;
void setParX( int parX );
int getParY() const;
void setParY( int parY );
int getFrameRate() const;
void setFrameRate( int frameRate );
const std::list<std::string>& getNumberOfChannels() const;
void addChannel( const std::string &channel );
const std::list<int>& getSamplingRates() const;
void addSampleRate( int sampleRate );
const std::list<ContentDescription*>& getContentProtections() const;
void addContentProtection( ContentDescription *desc );
const std::list<ContentDescription*>& getAccessibilities() const;
void addAccessibility( ContentDescription *desc );
const std::list<ContentDescription*>& getRatings() const;
void addRating( ContentDescription* desc );
const std::list<ContentDescription*>& getViewpoints() const;
void addViewpoint( ContentDescription *desc );
protected:
int parX;
int parY;
int frameRate;
std::list<std::string> channels;
std::list<int> sampleRates;
std::list<ContentDescription*> contentProtections;
std::list<ContentDescription*> accessibilities;
std::list<ContentDescription*> ratings;
std::list<ContentDescription*> viewpoints;
};
}
}
