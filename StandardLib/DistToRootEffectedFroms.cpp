#include "EmbeddedObject.h"
#include "DistToRootEffectedFroms.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::Init(void)
{
	macExpectedDists.Init(1024);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::Kill(void)
{
	macExpectedDists.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::Add(CBaseObject* pcObject, int iExpectedDist)
{
	SDistToRoot*	pcDistToRoot;

	pcDistToRoot = Get(pcObject);

	if (!pcDistToRoot)
	{
		pcDistToRoot = macExpectedDists.Add();
		pcDistToRoot->iExpectedDist = iExpectedDist;
		pcDistToRoot->pcObject = pcObject;
	}
	else
	{
		if (pcDistToRoot->iExpectedDist > iExpectedDist)
		{
			pcDistToRoot->iExpectedDist = iExpectedDist;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDistToRoot* CDistToRootEffectedFroms::Get(CBaseObject* pcObject)
{
	int				i;
	SDistToRoot*	pcDistToRoot;

	for (i = 0; i < macExpectedDists.NumElements(); i++)
	{
		pcDistToRoot = macExpectedDists.Get(i);
		if (pcDistToRoot->pcObject == pcObject)
		{
			return pcDistToRoot;
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDistToRoot* CDistToRootEffectedFroms::GetLowest(void)
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDistToRoot* CDistToRootEffectedFroms::GetSecondLowest(SDistToRoot* pcLowest)
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
			if (pcDistToRoot->pcObject != pcLowest->pcObject)
			{
				iMinDist = pcDistToRoot->iExpectedDist;
				pcMinDistToRoot = pcDistToRoot;
			}
		}
	}

	return pcMinDistToRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDistToRootEffectedFroms::NumElements(void)
{
	return macExpectedDists.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDistToRoot* CDistToRootEffectedFroms::Get(int iIndex)
{
	return macExpectedDists.SafeGet(iIndex);
}

