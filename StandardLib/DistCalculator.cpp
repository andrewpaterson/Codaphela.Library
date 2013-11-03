#include "BaseObject.h"
#include "DistCalculator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::Init(void)
{
	mcParameters.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::Kill(void)
{
	mcParameters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CDistCalculator::Calculate(CBaseObject* pcFromChanged, BOOL bHeapFromChanged)
{
	if (bHeapFromChanged)
	{
		return CalculateHeapFromChanged(pcFromChanged);
	}
	else
	{
		return CalculateStackFromChanged(pcFromChanged);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CDistCalculator::CalculateHeapFromChanged(CBaseObject* pcFromChanged)
{
	mcDistToRootCalculator.Calculate(pcFromChanged, &mcParameters);

	mcDistToStackCalculator.CalculateFromTouched(&mcParameters);

	return mcParameters.GetCompletelyDetachedArray();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CDistCalculator::CalculateStackFromChanged(CBaseObject* pcFromChanged)
{
	if (pcFromChanged->HasStackPointers())
	{
		return mcParameters.GetCompletelyDetachedArray();
	}
	else if (pcFromChanged->IsDistToRootValid())
	{
		return mcParameters.GetCompletelyDetachedArray();
	}
	else
	{
		pcFromChanged->CollectAndClearInvalidDistToRootObjects(&mcParameters);
		ChangeClearedDistToUnattachedDist(&mcParameters);
		mcDistToStackCalculator.CalculateFromTouched(&mcParameters);
		ClearTouchedFlags(&mcParameters);
		return mcParameters.GetCompletelyDetachedArray();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::ClearTouchedFlags(CDistCalculatorParameters* pcParameters)
{
	int				i;
	int				iNumTouched;
	CBaseObject*	pcBaseObject;

	iNumTouched = pcParameters->NumTouched();

	for (i = 0; i < iNumTouched; i++)
	{
		pcBaseObject = pcParameters->GetTouched(i);
		pcBaseObject->ClearDistTouchedFlags();
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::ChangeClearedDistToUnattachedDist(CDistCalculatorParameters* pcParameters)
{
	int				i;
	int				iNumTouched;
	CBaseObject*	pcBaseObject;

	iNumTouched = pcParameters->NumTouched();

	for (i = 0; i < iNumTouched; i++)
	{
		pcBaseObject = pcParameters->GetTouched(i);
		pcBaseObject->SetDistToRoot(UNATTACHED_DIST_TO_ROOT);
	}
}
