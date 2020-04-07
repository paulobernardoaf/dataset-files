





























#if !defined(__MFX_CRITICAL_SECTION_H)
#define __MFX_CRITICAL_SECTION_H

#include <mfxdefs.h>

namespace MFX
{


typedef volatile mfxL32 mfxCriticalSection;


void mfxEnterCriticalSection(mfxCriticalSection *pCSection);


void mfxLeaveCriticalSection(mfxCriticalSection *pCSection);

class MFXAutomaticCriticalSection
{
public:

explicit MFXAutomaticCriticalSection(mfxCriticalSection *pCSection)
{
m_pCSection = pCSection;
mfxEnterCriticalSection(m_pCSection);
}


~MFXAutomaticCriticalSection()
{
mfxLeaveCriticalSection(m_pCSection);
}

protected:

mfxCriticalSection *m_pCSection;

private:

MFXAutomaticCriticalSection(const MFXAutomaticCriticalSection &);
void operator=(const MFXAutomaticCriticalSection &);
};

} 

#endif 
