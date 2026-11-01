#include "BaseLib/GlobalMemory.h"
#include "BaseLib/LogString.h"
#include "BaseLib/LocalMallocatorConstructors.h"
#include "BaseLib/DataOrderers.h"
#include "BaseLib/IndexTemplateTemplate.h"
#include "TestLib/Assert.h"


struct SIndexTemplateKey
{
	double	f;
	char	c;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTemplateTemplateStuff(void)
{
	CIndexTemplateTemplate<SIndexTemplateKey, int64>	cIndex;

	cIndex.Init();
	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTemplateTemplate(void)
{
	BeginTests();
	MemoryInit();
	LocalMallocatorsInit(&gcConstructors, &gcMallocators);
	DataOrderersInit(&gcConstructors, &gcDataOrderers);

	TestIndexTemplateTemplateStuff();

	DataOrderersKill();
	MemoryKill();
	TestStatistics();
}

