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
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareBaseObjectPtr(const CBaseObject* pvArg1, const CBaseObject* pvArg2)
{
	CBaseObject* pcObject1;
	CBaseObject* pcObject2;

	pcObject1 = (CBaseObject*)pvArg1;
	pcObject2 = (CBaseObject*)pvArg2;

	
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareBaseObjectPtrPtr(const void* ppvArg1, const void* ppvArg2)
{
	return CompareBaseObjectPtr((CBaseObject*)*((void**)ppvArg1), (CBaseObject*)*((void**)ppvArg2));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBlockObjectPtr* CDistCalculator::Calculate(CBaseObject* pcFromChanged, bool bHeapFromChanged)
{
	CArrayBlockObjectPtr*	papcObjects;

	if (bHeapFromChanged)
	{
		papcObjects = CalculateHeapFromChanged(pcFromChanged);
	}
	else
	{
		papcObjects = CalculateStackFromChanged(pcFromChanged);
	}

	papcObjects->Sort();

	return papcObjects;
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
	if (pcFromChanged->HasStackFroms())
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::Print(CChars* psz)
{
	mcParameters.Print(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::Dump(void)
{
	CChars	sz;

	sz.Init();
	mcParameters.Print(&sz, false);
	sz.DumpKill();
}


