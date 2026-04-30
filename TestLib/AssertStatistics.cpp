#include <Time.h>
#include "BaseLib/Chars.h"
#include "BaseLib/Logger.h"
#include "BaseLib/NumberControl.h"
#include "AssertStatistics.h"


int	giTestsRun;
int	giTestsPassed;
int	giTestsFailed;

int	giTotalTestsRun;
int	giTotalTestsPassed;
int	giTotalTestsFailed;

clock_t gClock;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool Failed(void)
{
	giTestsRun++;
	giTestsFailed++;

	giTotalTestsRun++;
	giTotalTestsFailed++;

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool Pass(void)
{
	giTestsRun++;
	giTestsPassed++;

	giTotalTestsRun++;
	giTotalTestsPassed++;

	return true;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrivateBeginTests(char* szFile)
{
	CChars	sz;

	giTestsRun = 0;
	giTestsPassed = 0;
	giTestsFailed = 0;

	sz.Init();
	sz.Append("--------------- ");
	sz.Append(szFile);
	sz.Append(" ---------------\n");
	gcLogger.Add(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrivateTestStatistics(void)
{
	CChars	sz;

	sz.Init();

	if (giTestsRun > 0)
	{
		sz.Append("Tests Run: ");
		sz.Append(giTestsRun);
		sz.AppendNewLine();
		sz.Append("Passed: ");
		sz.Append(giTestsPassed);
		sz.AppendNewLine();
		sz.Append("Failed: ");
		sz.Append(giTestsFailed);
	}
	else
	{
		sz.Append("No Tests Run!");
	}

	sz.AppendNewLine();
	sz.AppendNewLine();

	gcLogger.Add(sz.Text());

	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InitTotalStatistics(void)
{
	gClock = clock();

	gcLogger.Init();

	giTotalTestsRun = 0;
	giTotalTestsPassed = 0;
	giTotalTestsFailed = 0;

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestTotalStatistics(void)
{
	CChars		sz;
	clock_t		time;

	time = clock() - gClock;

	sz.Init();
	sz.Append("------------------------------ Total Results ------------------------------\n");

	if (!gbFastFunctions)
	{
		FastFunctionsInit();
	}
	if (!gbNumberControl)
	{
		NumberInit();
	}

	if (giTotalTestsRun > 0)
	{
		sz.Append("Total Tests Run: ");
		sz.Append(giTotalTestsRun);
		sz.AppendNewLine();
		sz.Append("Total Passed: ");
		sz.Append(giTotalTestsPassed);
		sz.AppendNewLine();
		sz.Append("Total Failed: ");
		sz.Append(giTotalTestsFailed);
		sz.AppendNewLine();
		sz.AppendNewLine();
		sz.Append("Time Taken: ");
		sz.Append((float)time / ((float)CLOCKS_PER_SEC), 2);
		sz.Append("s");
		sz.AppendNewLine();
		sz.AppendNewLine();

		gcLogger.Add(sz.Text());

	}
	sz.Kill();

	if (gbNumberControl)
	{
		NumberKill();
	}
	if (gbFastFunctions)
	{
		FastFunctionsKill();
	}

	gcLogger.Kill();
	return giTotalTestsFailed;
}

