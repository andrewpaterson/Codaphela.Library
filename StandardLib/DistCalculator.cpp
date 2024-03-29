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
CArrayBlockObjectPtr* CDistCalculator::Calculate(CBaseObject* pcFromChanged, bool bHeapFromChanged)
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
CArrayBlockObjectPtr* CDistCalculator::CalculateHeapFromChanged(CBaseObject* pcFromChanged)
{
	mcDistToRootCalculator.Calculate(pcFromChanged, &mcParameters);

	mcDistToStackCalculator.CalculateFromTouched(&mcParameters);
	mcDistToStackCalculator.ResetObjectsToUnknownDistToStack(&mcParameters);
	mcParameters.ClearTouchedFlags();
	return mcParameters.GetCompletelyDetachedArray();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBlockObjectPtr* CDistCalculator::CalculateStackFromChanged(CBaseObject* pcFromChanged)
{
	if (pcFromChanged->HasStackPointers())
	{
		return mcParameters.GetCompletelyDetachedArray();  //Is empty.
	}
	else if (pcFromChanged->IsDistToRootValid())
	{
		return mcParameters.GetCompletelyDetachedArray();  //Is empty.
	}
	else
	{
		mcDistToStackCalculator.Calculate(pcFromChanged, &mcParameters);
		mcDistToStackCalculator.ResetObjectsToUnknownDistToStack(&mcParameters);
		mcParameters.ClearTouchedFlags();
		return mcParameters.GetCompletelyDetachedArray();
	}
}
