#include "AbstractAdaptationLogic.h"
namespace adaptive
{
namespace logic
{
class AlwaysBestAdaptationLogic : public AbstractAdaptationLogic
{
public:
AlwaysBestAdaptationLogic (vlc_object_t *);
virtual BaseRepresentation *getNextRepresentation(BaseAdaptationSet *, BaseRepresentation *);
};
}
}
