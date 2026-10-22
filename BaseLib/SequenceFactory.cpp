#include "ValueSequenceConfig.h"
#include "SequenceFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSequence* CSequenceFactory::Create(const char* szDirectory, EDurableSync eSync, size uiBlockSize)
{
	CLifeInit<CSequenceConfig>	cConfig;
	CSequence*					pcSequence;

	cConfig = CValueSequenceConfig::Create(szDirectory, eSync, uiBlockSize);
	pcSequence = NewMalloc<CSequence>();
	if (!pcSequence->Init(cConfig))
	{
		SafeKill(pcSequence);
		return NULL;
	}

	return pcSequence;
}

