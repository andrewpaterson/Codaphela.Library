#include "ValueSequenceConfig.h"
#include "SequenceFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSequence* CSequenceFactory::Create(const char* szDirectory, EDurableSync eSync)
{
	CLifeInit<CSequenceConfig>	cConfig;
	CSequence*					pcSequence;

	cConfig = CValueSequenceConfig::Create(szDirectory, eSync);
	pcSequence = NewMalloc<CSequence>();
	if (!pcSequence->Init(cConfig))
	{
		SafeKill(pcSequence);
		return NULL;
	}

	return pcSequence;
}

