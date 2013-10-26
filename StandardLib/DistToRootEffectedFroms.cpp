#include "EmbeddedObject.h"
#include "DistToRootEffectedFroms.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::Init(void)
{
	macExpectedDists.Init(1024);
	mapcLowestFroms.Init(1024);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::Kill(void)
{
	mapcLowestFroms.Kill();
	macExpectedDists.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::Add(CBaseObject* pcObject, int iExpectedDist)
{
	SDistToRoot*	psDistToRoot;

	psDistToRoot = Get(pcObject);

	if (!psDistToRoot)
	{
		psDistToRoot = macExpectedDists.Add();
		psDistToRoot->iExpectedDist = iExpectedDist;
		psDistToRoot->pcObject = pcObject;
	}
	else
	{
		if (psDistToRoot->iExpectedDist > iExpectedDist)
		{
			psDistToRoot->iExpectedDist = iExpectedDist;
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
	SDistToRoot*	psDistToRoot;

	for (i = 0; i < macExpectedDists.NumElements(); i++)
	{
		psDistToRoot = macExpectedDists.Get(i);
		if (psDistToRoot->pcObject == pcObject)
		{
			return psDistToRoot;
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
	SDistToRoot*	psDistToRoot;

	iMinDist = MAX_DIST_TO_ROOT;
	pcMinDistToRoot = NULL;

	for (i = 0; i < macExpectedDists.NumElements(); i++)
	{
		psDistToRoot = macExpectedDists.Get(i);
		if (psDistToRoot->iExpectedDist < iMinDist)
		{
			iMinDist = psDistToRoot->iExpectedDist;
			pcMinDistToRoot = psDistToRoot;
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::Remove(int iIndex)
{
	macExpectedDists.RemoveAt(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::Remove(SDistToRoot* psDistToRoot)
{
	int				i;
	SDistToRoot*	psCurrent;

	for (i = 0; i < macExpectedDists.NumElements(); i++)
	{
		psCurrent = macExpectedDists.Get(i);
		if (psCurrent == psDistToRoot)
		{
			macExpectedDists.RemoveAt(i, FALSE);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::MarkLowestFroms(void)
{
	int				i;
	SDistToRoot*	psCurrent;

	for (i = 0; i < macExpectedDists.NumElements(); i++)
	{
		psCurrent = macExpectedDists.Get(i);
		mapcLowestFroms.Add(&psCurrent->pcObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CDistToRootEffectedFroms::GetLowestFroms(void)
{
	return &mapcLowestFroms;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::AddChangedFromAsLowest(CBaseObject* pcFromChanged)
{
	int				i;
	CBaseObject*	pcObject;

	for (i = 0; i < mapcLowestFroms.NumElements(); i++)
	{
		pcObject = *mapcLowestFroms.Get(i);
		if (pcObject == pcFromChanged)
		{
			return;
		}
	}

	mapcLowestFroms.Add(&pcFromChanged);
}

