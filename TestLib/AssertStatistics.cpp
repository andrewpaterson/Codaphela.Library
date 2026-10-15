#include "BaseLib/Timer.h"
#include "BaseLib/Logger.h"
#include "BaseLib/NumberControl.h"
#include "AssertStatistics.h"


CAssertStatistics	gcAssertStatistics;


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAssertStatistics::Init(void)
{
	mcTotalTimer.Init();
	mcTestTimer.Init();

	gcLogger.Init();

	miTotalTestsRun = 0;
	miTotalTestsPassed = 0;
	miTotalTestsFailed = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAssertStatistics::Kill(void)
{
	mcTestTimer.Kill();
	mcTotalTimer.Kill();

	if (gbNumberControl)
	{
		NumberKill();
	}
	if (gbFastFunctions)
	{
		FastFunctionsKill();
	}

	gcLogger.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAssertStatistics::PrintTest(CChars* psz)
{
	float	fTotalTime;

	fTotalTime = mcTestTimer.GetTotalTimeInSeconds();

	if (miTestsRun > 0)
	{
		psz->Append("Tests Run: ");
		psz->Append(miTestsRun);
		psz->AppendNewLine();
		psz->Append("Passed: ");
		psz->Append(miTestsPassed);
		psz->AppendNewLine();
		psz->Append("Failed: ");
		psz->Append(miTestsFailed);
	}
	else
	{
		psz->Append("No Tests Run!");
	}
	if (gbFastFunctions && gbNumberControl)
	{
		psz->AppendNewLine();
		psz->Append("Time:   ");
		psz->Append(fTotalTime, 2);
		psz->Append("s");
	}
	psz->AppendNewLine();
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAssertStatistics::PrintTotal(CChars* psz)
{
	float	fTotalTime;

	mcTotalTimer.Update();
	fTotalTime = mcTotalTimer.GetTotalTimeInSeconds();

	psz->Append("------------------------------ Total Results ------------------------------\n");

	if (!gbFastFunctions)
	{
		FastFunctionsInit();
	}
	if (!gbNumberControl)
	{
		NumberInit();
	}

	if (miTotalTestsRun > 0)
	{
		psz->Append("Total Tests Run: ");
		psz->Append(miTotalTestsRun);
		psz->AppendNewLine();
		psz->Append("Total Passed: ");
		psz->Append(miTotalTestsPassed);
		psz->AppendNewLine();
		psz->Append("Total Failed: ");
		psz->Append(miTotalTestsFailed);
		psz->AppendNewLine();
		psz->AppendNewLine();
		psz->Append("Total Time:   ");
		psz->Append(fTotalTime, 2);
		psz->Append("s");
		psz->AppendNewLine();
		psz->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CAssertStatistics::GetTotalTestsFailed(void)
{
	return miTotalTestsFailed;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAssertStatistics::Failed(void)
{
	miTestsRun++;
	miTestsFailed++;

	miTotalTestsRun++;
	miTotalTestsFailed++;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAssertStatistics::Passed(void)
{
	miTestsRun++;
	miTestsPassed++;

	miTotalTestsRun++;
	miTotalTestsPassed++;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAssertStatistics::Begin(const char* szFile)
{
	CChars	sz;

	miTestsRun = 0;
	miTestsPassed = 0;
	miTestsFailed = 0;
	mcTestTimer.Reset();

	sz.Init();
	sz.Append("--------------- ");
	sz.Append(szFile);
	sz.Append(" ---------------\n");
	gcLogger.Add(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAssertStatistics::End(void)
{
	mcTestTimer.Update();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool Failed(void)
{
	gcAssertStatistics.Failed();

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool Pass(void)
{
	gcAssertStatistics.Passed();

	return true;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrivateBeginTests(const char* szFile)
{
	gcAssertStatistics.Begin(szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrivateTestStatistics(void)
{
	CChars		sz;

	gcAssertStatistics.End();
	sz.Init();
	gcAssertStatistics.PrintTest(&sz);
	gcLogger.Add(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InitTotalStatistics(void)
{
	gcAssertStatistics.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestTotalStatistics(void)
{
	CChars		sz;
	int			iTotalFailures;

	sz.Init();
	gcAssertStatistics.PrintTotal(&sz);
	gcLogger.Add(sz.Text());
	sz.Kill();

	iTotalFailures = gcAssertStatistics.GetTotalTestsFailed();
	gcAssertStatistics.Kill();
	return iTotalFailures;
}

