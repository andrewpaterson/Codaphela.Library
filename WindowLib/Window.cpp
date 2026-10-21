#include "BaseLib/Timer.h"
#include "BaseLib/DebugOutput.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "NativeWindowFactory.h"
#include "WindowTick.h"
#include "ComponentInput.h"
#include "Button.h"
#include "Window.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Init(const char* szTitle, CNativeWindowFactory* pcFactory, Ptr<CWindowTick> pTick, Ptr<CCanvasDraw> pDraw, EColourFormat eFormat, EColourOrder eOrder, ERGBColourBits eColourBits, ERGBAlphaBits eAlphaBits)
{
	PreInit();

	mszWindowTitle.Init(szTitle);

	mpcFactory = pcFactory;
	mpcNativeWindow = pcFactory->CreateNativeWindow(this);
	CComponent::Init(this);
	
	mpWindowTick = pTick;
	mpCanvas = OMalloc<CCanvas>(this, pDraw, eFormat, eOrder, eColourBits, eAlphaBits);
	AddComponent(mpCanvas);
	mbTicking = false;
	mpcComponentInput = NULL;
	mpDefaultButton = NULL;
	mbLayoutAgain = false;

	mFocus.Init(this);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Free(void)
{
	CNativeWindowFactory*	pcNativeFactory;

	pcNativeFactory = mpcNativeWindow->GetFactory();
	pcNativeFactory->DestroyNativeWindow(mpcNativeWindow);
	mpcNativeWindow = NULL;

	mszWindowTitle.Kill();

	CComponent::Free();

	mpcFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Class(void)
{
	CComponent::Class();

	U_String(mszWindowTitle);

	U_Pointer(mpcNativeWindow);
	M_Pointer(mpCanvas);
	M_Pointer(mpWindowTick);
	U_Bool(mbTicking);
	M_Embedded(mFocus);
	U_Pointer(mpcFactory);
	U_Pointer(mpcComponentInput);
	M_Pointer(mpDefaultButton);
	U_Bool(mbLayoutAgain);
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
		if (mpcComponentInput)
		{
			mpcComponentInput->GetInput()->Update();
		}
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
void CWindow::Paint(void)
{
	Layout();
	Draw();
	mpcNativeWindow->SignalPresent();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Layout(void)
{
	CRectangle	cRect;
	int			iPass;

	mpcNativeWindow->GetRectangle(&cRect);
	SetActualSize(cRect.GetWidth(), cRect.GetHeight());
	SetPosition(0, 0);  //The position of the Window within itself.

	//Some components only know their required size once they have been given a width, a word wrapped text area needs
	//more lines when it is narrower.  They ask for the components to be laid out again with their new size.  The
	//passes are limited in case components keep changing each other's sizes.
	for (iPass = 0; iPass < 3; iPass++)
	{
		mbLayoutAgain = false;
		CComponent::Layout(GetPosition(), GetActualSize());
		if (!mbLayoutAgain)
		{
			break;
		}
	}
	mbLayoutAgain = false;

	//Components may have moved under the pointer.
	mFocus.Refresh();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::LayoutAgain(void) { mbLayoutAgain = true; }
void CWindow::PointerMoved(int x, int y) { mFocus.PointerMoved(x, y); }
void CWindow::PointerLeft(void) { mFocus.PointerLeft(); }
void CWindow::InputCommand(EComponentInputCommand eCommand, bool bInsert) { mFocus.Command(eCommand, bInsert); }
void CWindow::InputPrintable(char c, bool bInsert) { mFocus.Printable(c, bInsert); }
void CWindow::Activate(void) { mFocus.Activate(); }


//////////////////////////////////////////////////////////////////////////
//
// Keys released while another window has focus are never seen.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Deactivate(void)
{
	mFocus.Deactivate();
	if (mpcComponentInput)
	{
		mpcComponentInput->ReleaseAll();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The window doesn't own the component input.  Kill it after the window
// stops showing.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetComponentInput(CComponentInput* pcComponentInput)
{
	mpcComponentInput = pcComponentInput;
}


//////////////////////////////////////////////////////////////////////////
//
// A window has at most one default button.  It takes on its default look
// and the one it replaces loses it.  NULL clears it.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetDefaultButton(Ptr<CButton> pButton)
{
	if (&mpDefaultButton == &pButton)
	{
		return;
	}

	if (mpDefaultButton)
	{
		mpDefaultButton->SetDefault(false);
	}
	mpDefaultButton = pButton;
	if (mpDefaultButton)
	{
		mpDefaultButton->SetDefault(true);
	}
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
bool CWindow::SetContainer(Ptr<CContainer> pContainer)
{
	return mpCanvas->SetContainer(pContainer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::ClearContainer(void)
{
	return mpCanvas->ClearContainer();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::GetColourFormat(CColourFormatHelper* pcHelper)
{
	pcHelper->Init(GetColourFormat(), GetColourOrder(), GetColourBits(), GetAlphaBits());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCanvas>			CWindow::GetCanvas(void) { return mpCanvas; }
Ptr<CFocus>				CWindow::GetFocus(void) { return &mFocus; }
Ptr<CButton>			CWindow::GetDefaultButton(void) { return mpDefaultButton; }
CComponentInput*		CWindow::GetComponentInput(void) { return mpcComponentInput; }
Ptr<CContainer>			CWindow::GetContainer(void) { return mpCanvas->GetContainer(); }
CNativeWindowFactory*	CWindow::GetFactory(void) { return mpcFactory; }
EColourFormat			CWindow::GetColourFormat(void) { return mpCanvas->GetColourFormat(); }
EColourOrder			CWindow::GetColourOrder(void) { return mpCanvas->GetColourOrder(); }
ERGBColourBits			CWindow::GetColourBits(void) { return mpCanvas->GetColourBits(); }
ERGBAlphaBits			CWindow::GetAlphaBits(void) { return mpCanvas->GetAlphaBits(); }

