#include "BaseObject.h"
#include "DistCalculatorParameters.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::Init(void)
{
	macExpectedDists.Init();
	mapcDetachedFromRoot.Init();
	mapcCompletelyDetached.Init();
	mapcTouched.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::Kill(void)
{
	mapcTouched.Kill();
	mapcCompletelyDetached.Kill();
	mapcDetachedFromRoot.Kill();
	macExpectedDists.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddExpectedDist(CBaseObject* pcObject, int iExpectedDist)
{
	//This method is never called with an iExpectedDist less than ROOT_DIST_TO_ROOT

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
SDistToRoot* CDistCalculatorParameters::GetExpectedDist(CBaseObject* pcObject)
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
SDistToRoot* CDistCalculatorParameters::GetLowestExpectedDist(void)
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
int CDistCalculatorParameters::NumExpectedDists(void)
{
	return macExpectedDists.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDistToRoot* CDistCalculatorParameters::GetExpectedDist(int iIndex)
{
	return macExpectedDists.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::RemoveExpectedDist(int iIndex)
{
	macExpectedDists.RemoveAt(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::RemoveExpectedDist(SDistToRoot* psDistToRoot)
{
	int	iIndex;
	
	iIndex = macExpectedDists.GetIndex(psDistToRoot);
	macExpectedDists.RemoveAt(iIndex, TRUE);  //Preserving order is (unfortunately) important.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddDetachedFromRoot(CBaseObject* pcObject)
{
	mapcDetachedFromRoot.Add(&pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDistCalculatorParameters::NumDetachedFromRoot(void)
{
	return mapcDetachedFromRoot.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculatorParameters::GetDetachedFromRoot(int iIndex)
{
	return *mapcDetachedFromRoot.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject** CDistCalculatorParameters::GetNextDetachedFromRoot(void)
{
	return mapcDetachedFromRoot.SafeGet(0);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::RemoveDetachedFromRoot(CBaseObject** ppcObject)
{
	int	iIndex;

	iIndex = mapcDetachedFromRoot.GetIndex(ppcObject);
	mapcDetachedFromRoot.RemoveAt(iIndex, TRUE);  //Preserving order is (unfortunately) important.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddCompletelyDetached(CBaseObject* pcObject)
{
	mapcCompletelyDetached.Add(&pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDistCalculatorParameters::NumCompletelyDetached(void)
{
	return mapcCompletelyDetached.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculatorParameters::GetCompletelyDetached(int iIndex)
{
	return *mapcCompletelyDetached.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::RemoveCompletelyDetached(int iIndex)
{
	mapcCompletelyDetached.RemoveAt(iIndex, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::CopyRootDetachedToCompletelyDetached(void)
{
	//Poor mans copy for now.  Should resize and memcpy rather.
	int				i;
	int				iNum;
	CBaseObject*	pcBaseObject;

	iNum = mapcDetachedFromRoot.NumElements();
	for (i = 0; i < iNum; i++)
	{
		pcBaseObject = *mapcDetachedFromRoot.Get(i);
		mapcCompletelyDetached.Add(&pcBaseObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBlockObjectPtr* CDistCalculatorParameters::GetCompletelyDetachedArray(void)
{
	return &mapcCompletelyDetached;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddTouched(CBaseObject* pcObject)
{
	if (!pcObject->HasDistTouchedFlag())
	{
		mapcTouched.Add(&pcObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDistCalculatorParameters::NumTouched(void)
{
	return mapcTouched.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculatorParameters::GetTouched(int iIndex)
{
	return *mapcTouched.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::PrintArray(CChars* psz, CArrayTemplateEmbeddedBaseObjectPtr* pcArray)
{
	int				i;
	int				iNum;
	CBaseObject*	pcBaseObject;

	iNum = pcArray->NumElements();
	for (i = 0; i < iNum; i++)
	{
		pcBaseObject = *pcArray->Get(i);
		pcBaseObject->PrintObject(psz, FALSE);
		
		if (i != iNum -1)
		{
			psz->Append(", ");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::PrintArray(CChars* psz, CArrayBlockObjectPtr* pcArray)
{
	int				i;
	int				iNum;
	CBaseObject*	pcBaseObject;

	iNum = pcArray->NumElements();
	for (i = 0; i < iNum; i++)
	{
		pcBaseObject = *pcArray->Get(i);
		pcBaseObject->PrintObject(psz, FALSE);

		if (i != iNum -1)
		{
			psz->Append(", ");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::PrintArray(CChars* psz, CArrayDistToRoot* pcArray)
{
	int				i;
	int				iNum;
	SDistToRoot*	psDistToRoot;

	iNum = pcArray->NumElements();
	for (i = 0; i < iNum; i++)
	{
		psDistToRoot = pcArray->Get(i);
		psDistToRoot->pcObject->PrintObject(psz, FALSE);

		if (i != iNum -1)
		{
			psz->Append(", ");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::ClearTouchedFlags(void)
{
	int				i;
	int				iNumTouched;
	CBaseObject*	pcBaseObject;

	iNumTouched = NumTouched();

	for (i = 0; i < iNumTouched; i++)
	{
		pcBaseObject = GetTouched(i);
		pcBaseObject->ClearDistTouchedFlags();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::Dump(void)
{
	CChars sz;

	sz.Init("--------- DistCalculatorParameters ---------");
	sz.AppendNewLine();

	sz.Append("      Touched [");
	sz.Append(mapcTouched.NumElements());
	sz.Append("]:  ");
	PrintArray(&sz, &mapcTouched);

	sz.AppendNewLine();

	sz.Append("ExpectedDists [");
	sz.Append(macExpectedDists.NumElements());
	sz.Append("]:  ");
	PrintArray(&sz, &macExpectedDists);
	sz.AppendNewLine();

	sz.Append("     Detached [");
	sz.Append(mapcDetachedFromRoot.NumElements());
	sz.Append("]:  ");
	PrintArray(&sz, &mapcDetachedFromRoot);
	sz.AppendNewLine();
	sz.AppendNewLine();

	sz.Append("  C. Detached [");
	sz.Append(mapcCompletelyDetached.NumElements());
	sz.Append("]:  ");
	PrintArray(&sz, &mapcCompletelyDetached);
	sz.AppendNewLine();

	sz.Append("--------------------------------------------");
	sz.AppendNewLine();

	sz.Dump();
	sz.Kill();
}

