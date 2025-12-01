#include "BaseLib/Timer.h"
#include "BaseLib/DebugOutput.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "NativeWindowFactory.h"
#include "WindowTick.h"
#include "Window.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Init(const char* szTitle, CNativeWindowFactory* pcFactory, Ptr<CWindowTick> pTick, Ptr<CCanvasDraw> pDraw)
{
	PreInit();

	mszWindowTitle.Init(szTitle);

	mpcNativeWindow = pcFactory->CreateNativeWindow(this);
	CComplexComponent::Init(this, mpcNativeWindow);

	mpWindowTick = pTick;
	mpCanvas = OMalloc<CCanvas>(this, pcFactory, pDraw);
	AddComponent(mpCanvas);
	mbTicking = false;

	mFocus.Init(this);
	mcClientRect.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Free(void)
{
	mpcNativeWindow->GetFactory()->DestroyNativeWindow(mpcNativeWindow);
	mpcNativeWindow = NULL;

	mszWindowTitle.Kill();

	CComplexComponent::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Class(void)
{
	CComplexComponent::Class();

	U_String(mszWindowTitle);
	U_Pointer(mpcNativeWindow);
	M_Pointer(mpCanvas);
	M_Pointer(mpWindowTick);
	U_Bool(mbTicking);
	M_Embedded(mFocus);
	U_Data(CRectangle, mcClientRect);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::Show(void)
{
	bool	bCreated;
	CTimer	cTimer;
	bool	bResult;

	cTimer.Init();
	bCreated = mpcNativeWindow->CreateNativeWindow();
	if (!bCreated)
	{
		return false;
	}

	bResult = true;
	mbTicking = true;
	while (mbTicking)
	{
		cTimer.Update();
		Tick(cTimer.GetUpdateTimeInMillieconds(), cTimer.GetTotalTimeInMillieconds());
		bResult = mpcNativeWindow->ExecuteNativeWindow();
		mbTicking &= bResult;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::CreateCanvas(EColourFormat eFormat, int32 iWidth, int32 iHeight)
{
	CNativeWindowFactory*	pcFactory;
	Ptr<CCanvas>			pNewCanvas;
	Ptr<CCanvasDraw>		pDraw;

	pcFactory = mpcNativeWindow->GetFactory();
	pDraw = mpCanvas->GetCanvasDraw();

	mcClientRect.SetSize(iWidth, iHeight);
	pNewCanvas = OMalloc<CCanvas>(this, eFormat, iWidth, iHeight, pDraw, pcFactory);

	pNewCanvas->CopyCanvas(mpCanvas);

	//This should be optimised into ReplaceComponent.
	RemoveComponent(mpCanvas);
	AddComponent(pNewCanvas);

	mpCanvas = pNewCanvas;
	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::DestroyCanvas(void)
{
	mcClientRect.Kill();
	mcClientRect.Init();
	mpCanvas = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Paint(void)
{
	Draw();
	mpcNativeWindow->SignalPresent();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CWindow::GetWindowTitle(void)
{
	return mszWindowTitle.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Tick(int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds)
{
	mpWindowTick->Tick(this, iUpdateTimeInMillieconds, iTotalTimeInMillieconds);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Stop(void)
{
	mbTicking = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCanvas>	CWindow::GetCanvas(void) { return mpCanvas; }
Ptr<CFocus>		CWindow::GetFocus(void) { return &mFocus; }
CRectangle*		CWindow::GetClientRect(void) { return &mcClientRect; }

