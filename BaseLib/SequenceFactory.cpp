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
	pcSequence->Init(cConfig);

	return pcSequence;
}

