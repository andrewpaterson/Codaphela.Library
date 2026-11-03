#ifndef __WAIT_TEST_REF_WINDOW_H__
#define __WAIT_TEST_REF_WINDOW_H__
#include "BaseLib/ArrayTemplate.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "WinRefLib/WinRefWindow.h"
#include "WindowLib/Component.h"
#include "WindowLib/WindowTick.h"


class CWindow;


//Called in place of sleeping each time the window waits for events, with how many waits there were before it and how
//long the window asked to wait (-1 for until there are events).  It does what would happen while the window slept,
//moving the pointer or stopping the window say, and returns true if it did anything.  If it didn't, the window
//sleeps for as long as it asked; asking to wait for ever with nothing to happen stops the window.
typedef bool (*WaitTestScript)(Ptr<CWindow> pWindow, size uiWait, int64 iMilliseconds, void* pvContext);


//A reference window that waits for events, as an on screen window does, so CWindow::Show sleeps between passes.
class CWaitTestRefWindow : public CWinRefWindow
{
CONSTRUCTABLE(CWaitTestRefWindow);
protected:
	CArrayTemplate<int64>	maiWaits;
	WaitTestScript			mfScript;
	void*					mpvContext;
	bool					mbStuck;  //Asked to wait for ever with nothing to happen.

public:
	void	Init(CWindow* pcWindow, CNativeWindowFactory* pcWindowFactory, CInt32Rect* pcBounds, char* szImagePath, WaitTestScript fScript, void* pvContext);
	void	Kill(void);

	bool	CanWaitForEvents(void) override;
	void	WaitForEvents(int64 iMilliseconds) override;

	size	NumWaits(void);
	int64	GetWait(size uiIndex);
	bool	IsStuck(void);
	void	SetSize(int32 iWidth, int32 iHeight);
};


class CWaitTestRefWindowFactory : public CWinRefWindowFactory
{
CONSTRUCTABLE(CWaitTestRefWindowFactory);
protected:
	WaitTestScript	mfScript;
	void*			mpvContext;

public:
	void			Init(CMallocator* pcMallocator, int32 iWidth, int32 iHeight, char* szImagePath, WaitTestScript fScript, void* pvContext);
	CNativeWindow*	CreateNativeWindow(CWindow* pcWindow) override;
};


//Counts how many times it is laid out and drawn.
class CLayoutCountComponent : public CComponent
{
CONSTRUCTABLE(CLayoutCountComponent);
DESTRUCTABLE(CLayoutCountComponent);
public:
	int32	miLayouts;
	int32	miDraws;

	void	Init(Ptr<CWindow> pWindow);
	void	Class(void) override;
	void	Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool	Draw(void) override;
};


//Counts its ticks.  The first miRequests of them each ask for another tick muiRequestMilliseconds later.
class CCountingTick : public CWindowTick
{
CONSTRUCTABLE(CCountingTick);
DESTRUCTABLE(CCountingTick);
public:
	int32	miTicks;
	int32	miRequests;
	uint32	muiRequestMilliseconds;
	int32	miStop;  //Stops the window after this many ticks, -1 never.

	void	Init(int32 iRequests, uint32 uiRequestMilliseconds, int32 iStop);
	void	Class(void) override;
	void	Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) override;
};


#endif // __WAIT_TEST_REF_WINDOW_H__
