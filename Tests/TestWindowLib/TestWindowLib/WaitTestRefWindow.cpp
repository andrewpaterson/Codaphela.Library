#include "BaseLib/NativeThread.h"
#include "StandardLib/ClassDefines.h"
#include "WindowLib/Window.h"
#include "WaitTestRefWindow.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWaitTestRefWindow::Init(CWindow* pcWindow, CNativeWindowFactory* pcWindowFactory, CInt32Rect* pcBounds, char* szImagePath, WaitTestScript fScript, void* pvContext)
{
	CWinRefWindow::Init(pcWindow, pcWindowFactory, pcBounds, szImagePath);
	maiWaits.Init();
	mfScript = fScript;
	mpvContext = pvContext;
	mbStuck = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWaitTestRefWindow::Kill(void)
{
	maiWaits.Kill();
	CWinRefWindow::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWaitTestRefWindow::CanWaitForEvents(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The script stands in for the events that would wake the window.
// Without any the window sleeps as asked, as a real one would.
//
//////////////////////////////////////////////////////////////////////////
void CWaitTestRefWindow::WaitForEvents(int64 iMilliseconds)
{
	size	uiWait;
	bool	bHappened;

	uiWait = maiWaits.NumElements();
	maiWaits.Add(&iMilliseconds);

	bHappened = false;
	if (mfScript)
	{
		bHappened = mfScript(mpcWindow, uiWait, iMilliseconds, mpvContext);
	}
	if (bHappened)
	{
		return;
	}

	if (iMilliseconds < 0)
	{
		mbStuck = true;
		mpcWindow->Stop();
		return;
	}

	ThreadSleep((int)iMilliseconds);
}


//////////////////////////////////////////////////////////////////////////
//
// As an on screen window's size changes when its user drags its edge.
// Its native window would tell the window to paint again.
//
//////////////////////////////////////////////////////////////////////////
void CWaitTestRefWindow::SetSize(int32 iWidth, int32 iHeight)
{
	mcWindowBounds.Init(0, 0, iWidth, iHeight);
	mpcWindow->Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CWaitTestRefWindow::NumWaits(void) { return maiWaits.NumElements(); }
int64 CWaitTestRefWindow::GetWait(size uiIndex) { return *maiWaits.Get(uiIndex); }
bool CWaitTestRefWindow::IsStuck(void) { return mbStuck; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWaitTestRefWindowFactory::Init(CMallocator* pcMallocator, int32 iWidth, int32 iHeight, char* szImagePath, WaitTestScript fScript, void* pvContext)
{
	CWinRefWindowFactory::Init(pcMallocator, iWidth, iHeight, szImagePath);
	mfScript = fScript;
	mpvContext = pvContext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNativeWindow* CWaitTestRefWindowFactory::CreateNativeWindow(CWindow* pcWindow)
{
	CWaitTestRefWindow*	pcNativeWindow;

	pcNativeWindow = NativeMalloc<CWaitTestRefWindow>();
	pcNativeWindow->Init(pcWindow, this, &mcWindowBounds, mszImagePath.Text(), mfScript, mpvContext);
	return pcNativeWindow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLayoutCountComponent::Init(Ptr<CWindow> pWindow)
{
	PreInit();
	CComponent::Init(pWindow);
	miLayouts = 0;
	miDraws = 0;
	SetCanGetFocus(true);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLayoutCountComponent::Class(void)
{
	CComponent::Class();
	U_Int32(miLayouts);
	U_Int32(miDraws);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLayoutCountComponent::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	miLayouts++;
	CComponent::Layout(sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLayoutCountComponent::Draw(void)
{
	miDraws++;
	return CComponent::Draw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCountingTick::Init(int32 iRequests, uint32 uiRequestMilliseconds, int32 iStop)
{
	PreInit();
	CWindowTick::Init();
	miTicks = 0;
	miRequests = iRequests;
	muiRequestMilliseconds = uiRequestMilliseconds;
	miStop = iStop;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCountingTick::Class(void)
{
	CWindowTick::Class();
	U_Int32(miTicks);
	U_Int32(miRequests);
	U_UInt32(muiRequestMilliseconds);
	U_Int32(miStop);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCountingTick::Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds)
{
	miTicks++;
	if (miRequests > 0)
	{
		miRequests--;
		pWindow->RequestTick(muiRequestMilliseconds);
	}
	if ((miStop >= 0) && (miTicks >= miStop))
	{
		pWindow->Stop();
	}
}
