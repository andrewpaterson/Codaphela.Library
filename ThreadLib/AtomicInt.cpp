#include "BaseLib/Timer.h"
#include "BaseLib/NativeThread.h"
#include "AtomicInt.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAtomicInt::WaitUntilAtLeast(int iValue, int iTimeoutMilliseconds)
{
	CTimer	cTimer;
	bool	bResult;

	if (Get() >= iValue)
	{
		return true;
	}

	cTimer.Init();
	bResult = true;
	while (Get() < iValue)
	{
		if (iTimeoutMilliseconds != ATOMIC_WAIT_FOREVER)
		{
			cTimer.Update();
			if (cTimer.GetTotalTimeInMillieconds() >= iTimeoutMilliseconds)
			{
				bResult = false;
				break;
			}
		}
		ThreadYield();
	}
	cTimer.Kill();
	return bResult;
}

