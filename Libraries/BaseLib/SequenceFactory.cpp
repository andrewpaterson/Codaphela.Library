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


//////////////////////////////////////////////////////////////////////////
//
// A sequence that hands out indices only from oiFirst up to (not
// including) oiEnd: a range another sequence reserved with ReserveRange.
//
//////////////////////////////////////////////////////////////////////////
CSequence* CSequenceFactory::CreateRange(const char* szDirectory, EDurableSync eSync, OIndex oiFirst, OIndex oiEnd, size uiBlockSize)
{
	CLifeInit<CSequenceConfig>	cConfig;
	CSequence*					pcSequence;

	cConfig = CValueSequenceConfig::CreateRange(szDirectory, eSync, oiFirst, oiEnd, uiBlockSize);
	pcSequence = NewMalloc<CSequence>();
	if (!pcSequence->Init(cConfig))
	{
		SafeKill(pcSequence);
		return NULL;
	}

	return pcSequence;
}

