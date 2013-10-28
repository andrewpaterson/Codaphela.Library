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
	mapcUnattched.Init(1024);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::Kill(void)
{
	mapcUnattched.Kill();
	mapcLowestFroms.Kill();
	macExpectedDists.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::AddExpectedDist(CBaseObject* pcObject, int iExpectedDist)
{
	SDistToRoot*	psDistToRoot;

	psDistToRoot = GetExpectedDist(pcObject);

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
void CDistToRootEffectedFroms::AddUnattached(CBaseObject* pcObject)
{
	mapcUnattched.Add(&pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDistToRoot* CDistToRootEffectedFroms::GetExpectedDist(CBaseObject* pcObject)
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
BOOL CDistToRootEffectedFroms::ContainsUnattached(CBaseObject* pcObject)
{
	int				i;
	CBaseObject*	pcCurrent;

	for (i = 0; i < mapcUnattched.NumElements(); i++)
	{
		pcCurrent = *mapcUnattched.Get(i);
		if (pcCurrent == pcObject)
		{
			return TRUE;
		}
	}

	return FALSE;
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
CBaseObject* CDistToRootEffectedFroms::GetUnattached(void)
{
	if (mapcUnattched.IsEmpty())
	{
		return NULL;
	}

	return *mapcUnattched.Tail();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDistToRootEffectedFroms::NumExpectedDists(void)
{
	return macExpectedDists.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDistToRoot* CDistToRootEffectedFroms::GetExpectedDist(int iIndex)
{
	return macExpectedDists.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::RemoveExpectedDist(int iIndex)
{
	macExpectedDists.RemoveAt(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::RemoveExpectedDist(SDistToRoot* psDistToRoot)
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
void CDistToRootEffectedFroms::RemoveUnattached(CBaseObject* pcBaseObject)
{
	int				i;
	CBaseObject*	pcCurrent;

	for (i = mapcUnattched.NumElements()-1; i >= 0; i--)
	{
		pcCurrent = *mapcUnattched.Get(i);
		if (pcCurrent == pcBaseObject)
		{
			mapcUnattched.RemoveAt(i, FALSE);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootEffectedFroms::MarkExpectedDistLowestFroms(void)
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
void CDistToRootEffectedFroms::MarkUnattachedLowestFroms(void)
{
	int				i;
	CBaseObject*	pcCurrent;

	for (i = 0; i < mapcUnattched.NumElements(); i++)
	{
		pcCurrent = *mapcUnattched.Get(i);
		mapcLowestFroms.Add(&pcCurrent);
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
	int				iNumElements;

	iNumElements = mapcLowestFroms.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcObject = *mapcLowestFroms.Get(i);
		if (pcObject == pcFromChanged)
		{
			return;
		}
	}

	mapcLowestFroms.Add(&pcFromChanged);
}

