#include "EmbeddedObject.h"
#include "DistToRootCalculator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::Init(void)
{
	macExpectedDists.Init(1024);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::Kill(void)
{
	macExpectedDists.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::Add(CBaseObject* pcObject, int iExpectedDist)
{
	SDistToRoot*	pcDistToRoot;

	pcDistToRoot = macExpectedDists.Add();
	pcDistToRoot->iExpectedDist = iExpectedDist;
	pcDistToRoot->pcObject = pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDistToRoot* CDistToRootCalculator::GetLowest(void)
{
	int				i;
	int				iMinDist;
	SDistToRoot*	pcMinDistToRoot;
	SDistToRoot*	pcDistToRoot;
	
	iMinDist = MAX_DIST_TO_ROOT;
	pcMinDistToRoot = NULL;

	for (i = 0; i < macExpectedDists.NumElements(); i++)
	{
		pcDistToRoot = macExpectedDists.Get(i);
		if (pcDistToRoot->iExpectedDist < iMinDist)
		{
			iMinDist = pcDistToRoot->iExpectedDist;
			pcMinDistToRoot = pcDistToRoot;
		}
	}

	return pcMinDistToRoot;
}

