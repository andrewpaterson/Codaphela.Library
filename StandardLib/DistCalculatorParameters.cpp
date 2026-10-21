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
	size			i;
	SDistToRoot*	psDistToRoot;
	size			uiDists;

	uiDists = macExpectedDists.NumElements();
	for (i = 0; i < uiDists; i++)
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
	size			i;
	int				iMinDist;
	SDistToRoot*	pcMinDistToRoot;
	SDistToRoot*	psDistToRoot;
	size			uiDists;

	iMinDist = MAX_DIST_TO_ROOT;
	pcMinDistToRoot = NULL;

	uiDists = macExpectedDists.NumElements();
	for (i = 0; i < uiDists; i++)
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
size CDistCalculatorParameters::NumExpectedDists(void)
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
	size	iIndex;
	
	iIndex = macExpectedDists.GetIndex(psDistToRoot);
	macExpectedDists.RemoveAt(iIndex, true);  //Preserving order is (unfortunately) important.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddDetachedFromRoot(CBaseObject* pcObject)
{
	mapcDetachedFromRoot.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculatorParameters::NumDetachedFromRoot(void)
{
	return mapcDetachedFromRoot.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculatorParameters::GetDetachedFromRoot(size iIndex)
{
	return mapcDetachedFromRoot.GetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddCompletelyDetached(CBaseObject* pcObject)
{
	mapcCompletelyDetached.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculatorParameters::NumCompletelyDetached(void)
{
	return mapcCompletelyDetached.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculatorParameters::GetCompletelyDetached(size iIndex)
{
	return mapcCompletelyDetached.GetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::RemoveCompletelyDetached(size iIndex)
{
	mapcCompletelyDetached.RemoveAt(iIndex, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::CopyRootDetachedToCompletelyDetached(void)
{
	//Poor mans copy for now.  Should resize and memcpy rather.
	size			i;
	size			iNum;
	CBaseObject*	pcBaseObject;

	iNum = mapcDetachedFromRoot.NumElements();
	for (i = 0; i < iNum; i++)
	{
		pcBaseObject = *mapcDetachedFromRoot.Get(i);
		mapcCompletelyDetached.Add(pcBaseObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CDistCalculatorParameters::GetCompletelyDetachedArray(void)
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
		mapcTouched.Add(pcObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculatorParameters::NumTouched(void)
{
	return mapcTouched.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculatorParameters::GetTouched(size iIndex)
{
	return mapcTouched.GetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::PrintArray(CChars* psz, CArrayBaseObjectPtrE32* pcArray, bool bCommaSeparate)
{
	size			i;
	size			iNum;
	CBaseObject*	pcBaseObject;

	iNum = pcArray->NumElements();
	for (i = 0; i < iNum; i++)
	{
		if (!bCommaSeparate)
		{
			psz->Append("  ");
		}

		pcBaseObject = *pcArray->Get(i);
		pcBaseObject->PrintObject(psz, false);
		
		if (bCommaSeparate)
		{
			if (i != iNum - 1)
			{
				psz->Append(", ");
			}
		}
		else
		{
			psz->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::PrintArray(CChars* psz, CArrayBaseObjectPtr* pcArray, bool bCommaSeparate)
{
	size			i;
	size			iNum;
	CBaseObject*	pcBaseObject;

	iNum = pcArray->NumElements();
	for (i = 0; i < iNum; i++)
	{
		if (!bCommaSeparate)
		{
			psz->Append("  ");
		}

		pcBaseObject = *pcArray->Get(i);
		pcBaseObject->PrintObject(psz, false);

		if (bCommaSeparate)
		{
			if (i != iNum - 1)
			{
				psz->Append(", ");
			}
		}
		else
		{
			psz->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::PrintArray(CChars* psz, CArrayDistToRoot* pcArray, bool bCommaSeparate)
{
	size			i;
	size			iNum;
	SDistToRoot*	psDistToRoot;

	iNum = pcArray->NumElements();
	for (i = 0; i < iNum; i++)
	{
		if (!bCommaSeparate)
		{
			psz->Append("  ");
		}

		psDistToRoot = pcArray->Get(i);
		psDistToRoot->pcObject->PrintObject(psz, false);

		if (bCommaSeparate)
		{
			if (i != iNum - 1)
			{
				psz->Append(", ");
			}
		}
		else
		{
			psz->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::ClearTouchedFlags(void)
{
	size			i;
	size			iNumTouched;
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
void CDistCalculatorParameters::Print(CChars* psz, bool bCommaSeparate)
{
	psz->Append("--------- DistCalculatorParameters ---------");
	psz->AppendNewLine();

	if (bCommaSeparate)
	{
		psz->Append("      ");
	}
	psz->Append("Touched [");
	psz->Append(mapcTouched.NumElements());
	psz->Append("]:  ");
	if (!bCommaSeparate)
	{
		psz->AppendNewLine();
	}
	PrintArray(psz, &mapcTouched, bCommaSeparate);

	psz->AppendNewLine();

	psz->Append("ExpectedDists [");
	psz->Append(macExpectedDists.NumElements());
	psz->Append("]:  ");
	if (!bCommaSeparate)
	{
		psz->AppendNewLine();
	}
	PrintArray(psz, &macExpectedDists, bCommaSeparate);
	psz->AppendNewLine();

	if (bCommaSeparate)
	{
		psz->Append("     ");
	}
	psz->Append("Detached [");
	psz->Append(mapcDetachedFromRoot.NumElements());
	psz->Append("]:  ");
	if (!bCommaSeparate)
	{
		psz->AppendNewLine();
	}
	PrintArray(psz, &mapcDetachedFromRoot, bCommaSeparate);
	psz->AppendNewLine();

	if (bCommaSeparate)
	{
		psz->Append("  ");
	}
	psz->Append("C. Detached [");
	psz->Append(mapcCompletelyDetached.NumElements());
	psz->Append("]:  ");
	if (!bCommaSeparate)
	{
		psz->AppendNewLine();
	}
	PrintArray(psz, &mapcCompletelyDetached, bCommaSeparate);
	psz->AppendNewLine();

	psz->Append("--------------------------------------------");
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::Dump(void)
{
	CChars sz;

	sz.Init();
	Print(&sz, false);
	sz.DumpKill();
}
