#include "BaseLib/Timer.h"
#include "BaseLib/DebugOutput.h"
#include "NativeWindowFactory.h"
#include "Window.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Init(CNativeWindow* pcNativeWindow)
{
	mpcNativeWindow = pcNativeWindow;
	miTime = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Kill(void)
{
	mpcNativeWindow->GetFactory()->DestroyNativeWindow(mpcNativeWindow);
	mpcNativeWindow = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::Show(void)
{
	bool	bCreated;
	bool	bRunning;
	CTimer	cTimer;

	cTimer.Init();
	bCreated = mpcNativeWindow->CreateNativeWindow();
	if (!bCreated)
	{
		return false;
	}

	for (;;)
	{
		cTimer.Update();
		Tick(cTimer.GetUpdateTimeInMillieconds(), cTimer.GetTotalTimeInMillieconds());
		bRunning = mpcNativeWindow->ExecuteNativeWindow();
		if (!bRunning)
		{
			return true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Tick(int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds)
{
	if (iTotalTimeInMillieconds > miTime + 1000)
	{
		EngineOutput("Tick... ");
		miTime = iTotalTimeInMillieconds;
	}
}

