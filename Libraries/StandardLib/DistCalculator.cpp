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
CArrayBaseObjectPtr* CDistCalculator::Calculate(CBaseObject* pcFromChanged, bool bHeapFromChanged)
{
	CArrayBaseObjectPtr*	papcObjects;

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
CArrayBaseObjectPtr* CDistCalculator::CalculateHeapFromChanged(CBaseObject* pcFromChanged)
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
CArrayBaseObjectPtr* CDistCalculator::CalculateStackFromChanged(CBaseObject* pcFromChanged)
{
	if (pcFromChanged->HasStackFroms())
	{
		return mcParameters.GetCompletelyDetachedArray();  //Is empty.
	}
	else if (pcFromChanged->IsHeldFromStackOrRoot())
	{
		return mcParameters.GetCompletelyDetachedArray();  //Is empty.
	}
	else
	{
		//Calculate clears the touched flags.
		mcDistToStackCalculator.Calculate(pcFromChanged, &mcParameters);
		mcDistToStackCalculator.ResetObjectsToUnknownDistToStack(&mcParameters);
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


