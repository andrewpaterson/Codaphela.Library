#include "ValueSequenceConfig.h"
#include "SequenceFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSequence* CSequenceFactory::Create(const char* szDirectory)
{
	CLifeInit<CSequenceConfig>	cConfig;
	CSequence*					pcSequence;

	cConfig = CValueSequenceConfig::Create(szDirectory);
	pcSequence = NewMalloc<CSequence>();
	pcSequence->Init(cConfig);

	return pcSequence;
}

