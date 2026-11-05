/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Timer.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "NativeWindowFactory.h"
#include "WindowTick.h"
#include "ComponentInput.h"
#include "Button.h"
#include "NativeSoundFactory.h"
#include "Window.h"


//Debug builds lay out every paint and log components that moved without their layout being invalidated, unless a
//window is told not to (see SetCheckLayoutCache).
#ifdef _DEBUG
#define WINDOW_CHECK_LAYOUT_CACHE	true
#else
#define WINDOW_CHECK_LAYOUT_CACHE	false
#endif // _DEBUG


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Init(const char* szTitle, CNativeWindowFactory* pcFactory, Ptr<CWindowTick> pTick, Ptr<CCanvasDraw> pDraw, EColourFormat eFormat, EColourOrder eOrder, ERGBColourBits eColourBits, ERGBAlphaBits eAlphaBits)
{
	Init(szTitle, pcFactory, NULL, pTick, pDraw, eFormat, eOrder, eColourBits, eAlphaBits);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Init(const char* szTitle, CNativeWindowFactory* pcFactory, CNativeSoundFactory* pcSoundFactory, Ptr<CWindowTick> pTick, Ptr<CCanvasDraw> pDraw, EColourFormat eFormat, EColourOrder eOrder, ERGBColourBits eColourBits, ERGBAlphaBits eAlphaBits)
{
	CColourFormatHelper	cFormat;

	cFormat.Init(eFormat, eOrder, eColourBits, eAlphaBits);
	Init(szTitle, pcFactory, pcSoundFactory, pTick, pDraw, &cFormat);
}


//////////////////////////////////////////////////////////////////////////
//
// Like another window's, a dialog's say: see GetColourFormat.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Init(const char* szTitle, CNativeWindowFactory* pcFactory, Ptr<CWindowTick> pTick, Ptr<CCanvasDraw> pDraw, CColourFormatHelper* pcFormat)
{
	Init(szTitle, pcFactory, NULL, pTick, pDraw, pcFormat);
}


//////////////////////////////////////////////////////////////////////////
//
// A window given a sound factory has a sound mixer, at the default
// sample rate, that is mixed after each tick.  The canvas the window is
// drawn on is in pcFormat.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Init(const char* szTitle, CNativeWindowFactory* pcFactory, CNativeSoundFactory* pcSoundFactory, Ptr<CWindowTick> pTick, Ptr<CCanvasDraw> pDraw, CColourFormatHelper* pcFormat)
{
	PreInit();

	mszWindowTitle.Init(szTitle);

	mpcNativeWindow = pcFactory->CreateNativeWindow(this);
	CComponent::Init(this);
	
	mpWindowTick = pTick;
	mpCanvas = OMalloc<CCanvas>(this, pDraw, pcFormat);
	AddComponent(mpCanvas);
	mbTicking = false;
	mpcComponentInput = NULL;
	mpDefaultButton = NULL;
	mpCancelButton = NULL;
	mbLayoutAgain = false;
	maPopups.Init();
	if (pcSoundFactory)
	{
		mpSoundMixer = OMalloc<CSoundMixer>(pcSoundFactory, (uint32)SOUND_MIXER_DEFAULT_SAMPLE_RATE);
	}
	else
	{
		mpSoundMixer = NULL;
	}

	mFocus.Init(this);
	mpClipboard = OMalloc<CClipboard>(this);
	mbContinuous = false;
	mbRedraw = true;
	mbLayoutDirty = true;
	mbCheckLayoutCache = WINDOW_CHECK_LAYOUT_CACHE;
	mcActivated.Init();
	SetMaxFramesPerSecond(WINDOW_DEFAULT_FRAMES_PER_SECOND);
	miLastPaintTime = 0;
	miTickDelay = -1;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Free(void)
{
	GetFactory()->DestroyNativeWindow(mpcNativeWindow);
	mpcNativeWindow = NULL;

	mszWindowTitle.Kill();
	mcActivated.Kill();

	CComponent::Free();
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
	U_Pointer(mpcComponentInput);
	M_Pointer(mpDefaultButton);
	M_Pointer(mpCancelButton);
	U_Bool(mbLayoutAgain);
	M_Embedded(maPopups);
	M_Pointer(mpSoundMixer);
	M_Pointer(mpClipboard);
	U_Bool(mbContinuous);
	U_Bool(mbRedraw);
	U_Bool(mbLayoutDirty);
	U_Bool(mbCheckLayoutCache);
	U_Data(CComponentEvent<CWindow>, mcActivated);
	U_UInt32(muiFrameMilliseconds);
	U_Int64(miLastPaintTime);
	U_Int64(miTickDelay);
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
// Each pass executes the native window's events, updates the input, paints
// the window if it has changed and then ticks it.  Between passes the
// window sleeps until it has events or something is due: a paint held
// back by the frame rate, a tick asked for with RequestTick, held chord
// inputs to let through or sound to mix.  Input, the pointer, activation
// and resizing mark the window changed; anything else that changes what
// it shows calls Invalidate.  A continuous window, or one whose native
// window can't wait, never sleeps and paints every pass.
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::Show(void)
{
	bool	bCreated;
	CTimer	cTimer;
	bool	bResult;
	bool	bContinuous;
	bool	bFirst;

	cTimer.Init();
	bCreated = mpcNativeWindow->CreateNativeWindow();
	if (!bCreated)
	{
		return false;
	}

	bContinuous = mbContinuous || !mpcNativeWindow->CanWaitForEvents();
	bResult = true;
	mbTicking = true;
	mbRedraw = true;
	miTickDelay = -1;
	miLastPaintTime = GetDebugTickCount() - muiFrameMilliseconds;
	bFirst = true;
	while (mbTicking)
	{
		//The first pass paints the window straight away.
		if (!bContinuous && !bFirst)
		{
			mpcNativeWindow->WaitForEvents(GetWaitMilliseconds());
		}
		bFirst = false;
		bResult = mpcNativeWindow->ExecuteNativeWindow();
		mbTicking &= bResult;
		if (!mbTicking)
		{
			break;
		}

		cTimer.Update();
		if (mpcComponentInput)
		{
			mpcComponentInput->Update();
		}
		if (bContinuous || (mbRedraw && IsFrameDue()))
		{
			Paint();
		}
		miTickDelay = -1;
		Tick(cTimer.GetUpdateTimeInMillieconds(), cTimer.GetTotalTimeInMillieconds());
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The shorter of two waits, either of which can be -1 for forever.
//
//////////////////////////////////////////////////////////////////////////
static int64 ShorterWait(int64 iWait, int64 iOther)
{
	if (iOther < 0)
	{
		return iWait;
	}
	if ((iWait < 0) || (iOther < iWait))
	{
		return iOther;
	}
	return iWait;
}


//////////////////////////////////////////////////////////////////////////
//
// How long the window can sleep before something is due, -1 if nothing
// is.
//
//////////////////////////////////////////////////////////////////////////
int64 CWindow::GetWaitMilliseconds(void)
{
	int64	iWait;
	int64	iFrameWait;
	uint32	uiDelay;

	iWait = -1;
	if (mbRedraw)
	{
		iFrameWait = miLastPaintTime + muiFrameMilliseconds - GetDebugTickCount();
		iWait = (iFrameWait > 0) ? iFrameWait : 0;
	}
	iWait = ShorterWait(iWait, miTickDelay);
	if (mpcComponentInput && mpcComponentInput->GetEvictionDelay(&uiDelay))
	{
		iWait = ShorterWait(iWait, uiDelay);
	}
	if (mpSoundMixer.IsNotNull() && mpSoundMixer->GetMixDelay(&uiDelay))
	{
		iWait = ShorterWait(iWait, uiDelay);
	}
	return iWait;
}


//////////////////////////////////////////////////////////////////////////
//
// A changed window is painted no more often than its frame rate.
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::IsFrameDue(void)
{
	return (GetDebugTickCount() - miLastPaintTime) >= muiFrameMilliseconds;
}


//////////////////////////////////////////////////////////////////////////
//
// The window will be painted again, as soon as its frame rate allows.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Invalidate(void)
{
	mbRedraw = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The window will be laid out and painted again.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::InvalidateLayout(void)
{
	mbLayoutDirty = true;
	mbRedraw = true;
}


//////////////////////////////////////////////////////////////////////////
//
// Ticks the window again within uiMilliseconds even if nothing else
// happens; a running simulation asks every tick.  Only the next tick is
// promised, so ask again each tick to keep ticking.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::RequestTick(uint32 uiMilliseconds)
{
	miTickDelay = ShorterWait(miTickDelay, uiMilliseconds);
}


//////////////////////////////////////////////////////////////////////////
//
// A continuous window doesn't wait for events: it is ticked and painted
// as fast as it can be, as a game or an emulator is.  Windows aren't
// continuous unless set.  Takes effect when the window is next shown.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetContinuous(bool bContinuous)
{
	mbContinuous = bContinuous;
}


//////////////////////////////////////////////////////////////////////////
//
// 0 doesn't hold paints back at all.  Continuous windows paint every
// pass whatever this is.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetMaxFramesPerSecond(uint32 uiFramesPerSecond)
{
	muiFrameMilliseconds = (uiFramesPerSecond == 0) ? 0 : (1000 + uiFramesPerSecond - 1) / uiFramesPerSecond;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::IsContinuous(void) { return mbContinuous; }


//////////////////////////////////////////////////////////////////////////
//
// On in Debug builds.  A test counting layouts turns it off: the check
// lays the window out on every paint.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetCheckLayoutCache(bool bCheckLayoutCache)
{
	mbCheckLayoutCache = bCheckLayoutCache;
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the window as a dialog over pOwner and returns once it stops.
// While it is shown the owner can't be used: it gets no keys or pointer
// and isn't ticked, though it is still drawn.  The owner's component
// input, if it has one, comes to the dialog until it closes.  The dialog
// comes off the screen when it stops but it can be shown again.
//
// Call it from the owner's tick, not from an input listener; reading the
// input while it is being read isn't supported.
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::ShowModal(Ptr<CWindow> pOwner)
{
	CComponentInput*	pcComponentInput;
	CNativeWindow*		pcOwnerNative;
	bool				bResult;

	pOwner->CloseAllPopups();
	pOwner->Deactivate();
	pcOwnerNative = pOwner->GetNativeWindow();
	pcOwnerNative->SetEnabled(false);

	pcComponentInput = pOwner->GetComponentInput();
	if (pcComponentInput)
	{
		pcComponentInput->ReleaseAll();
		pcComponentInput->SetWindow(this);
		SetComponentInput(pcComponentInput);
	}

	//The dialog is the active window, the native window may say so again.
	mpcNativeWindow->SetOwner(pcOwnerNative);
	Activate();
	bResult = Show();
	mFocus.Deactivate();

	//The owner is enabled before the dialog goes so that the owner is the window activated.
	pcOwnerNative->SetEnabled(true);
	mpcNativeWindow->DestroyNativeWindow();
	mpcNativeWindow->SetOwner(NULL);

	if (pcComponentInput)
	{
		pcComponentInput->ReleaseAll();
		pcComponentInput->SetWindow(&pOwner);
		SetComponentInput(NULL);
	}

	pOwner->Activate();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The size of the window's client area when it is shown.  Without one
// the native window chooses.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetWindowSize(int32 iWidth, int32 iHeight)
{
	mpcNativeWindow->SetRequestedSize(iWidth, iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Paint(void)
{
	LayoutIfChanged();
	Draw();
	mpcNativeWindow->SignalPresent();

	//Anything changed while painting was painted.
	mbRedraw = false;
	miLastPaintTime = GetDebugTickCount();
}


//////////////////////////////////////////////////////////////////////////
//
// Lays the window out in its client area whether it has changed or not.
// Paint only lays it out when its size has changed or its layout was
// invalidated.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Layout(void)
{
	CInt32Rect	cRect;
	uint		iPass;

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

	//Components invalidating their layout while they were laid out were laid out.
	mbLayoutDirty = false;

	//Components may have moved under the pointer.
	mFocus.Refresh();
}


//////////////////////////////////////////////////////////////////////////
//
// The layout is kept from the last paint unless the client area has
// changed size or the layout was invalidated.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::LayoutIfChanged(void)
{
	CInt32Rect	cRect;
	SInt32Vec2	sSize;

	mpcNativeWindow->GetRectangle(&cRect);
	sSize = GetActualSize();
	if (mbLayoutDirty || (cRect.GetWidth() != sSize.x) || (cRect.GetHeight() != sSize.y))
	{
		Layout();
		return;
	}

	if (mbCheckLayoutCache)
	{
		CheckLayoutCache();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Lays the window out anyway and logs the components that moved, which
// changed without invalidating the layout.  The kept layout would have
// been wrong.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::CheckLayoutCache(void)
{
	CArrayTemplate<SInt32Vec2>	asBounds;
	size						uiNumLogged;

	asBounds.Init();
	SaveLayout(&asBounds);
	Layout();
	uiNumLogged = 0;
	CheckLayout(&asBounds, 0, &uiNumLogged);
	asBounds.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::LayoutAgain(void) { mbLayoutAgain = true; }
//Input can change anything, so the window is painted again after it.  Keys, buttons and the wheel can change what
//the window looks like, so it is laid out again too.  The pointer moving only changes what it is over; a component
//that moves things as it is dragged invalidates the layout itself.
void CWindow::PointerMoved(int x, int y) { mFocus.PointerMoved(x, y); Invalidate(); }
void CWindow::PointerLeft(void) { mFocus.PointerLeft(); Invalidate(); }
void CWindow::InputCommand(EComponentInputCommand eCommand, bool bInsert) { mFocus.Command(eCommand, bInsert); InvalidateLayout(); }
void CWindow::InputPrintable(char c, bool bInsert) { mFocus.Printable(c, bInsert); InvalidateLayout(); }


//////////////////////////////////////////////////////////////////////////
//
// The window became the active window (the platform's focus came to it):
// the activation event is called, as for an IDE to see whether its files
// were changed while it wasn't.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Activate(void)
{
	mFocus.Activate();
	InvalidateLayout();
	mcActivated.Call(this);
}


CComponentEvent<CWindow>* CWindow::GetActivationEvent(void) { return &mcActivated; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Input(SWindowInputEvent* psWindowEvent, CComponentInput* pcComponentInput)
{
	mFocus.Input(psWindowEvent, pcComponentInput);
	if (psWindowEvent->eType == WIET_Delta)
	{
		Invalidate();
	}
	else
	{
		InvalidateLayout();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Keys released while another window has focus are never seen.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Deactivate(void)
{
	InvalidateLayout();
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
// A window has at most one cancel button, clicked by escape when no popup
// is open.  It looks like any other button.  NULL clears it.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetCancelButton(Ptr<CButton> pButton)
{
	mpCancelButton = pButton;
}


//////////////////////////////////////////////////////////////////////////
//
// Opens pContent over everything else in the window with its top left at
// sPosition, in window client space.  pOwner is the component opening it,
// it is told when the popup closes.  pContent must not be in another
// container.  The popup is laid out straight away so it can be hit before
// the window is next painted.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CPopup> CWindow::OpenPopup(Ptr<CComponent> pContent, SInt32Vec2 sPosition, Ptr<CComponent> pOwner)
{
	Ptr<CPopup>	pPopup;

	InvalidateLayout();
	pPopup = OMalloc<CPopup>(this, pContent, sPosition, pOwner);
	maPopups.Add(pPopup);
	mpCanvas->AddComponent(pPopup);
	pPopup->Layout(GetPosition(), GetActualSize());
	return pPopup;
}


//////////////////////////////////////////////////////////////////////////
//
// Closes the popup holding pContent and every popup opened after it, the
// last opened first.  Each popup's owner is told once its popup is gone.
// Returns false if pContent isn't in an open popup.
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::ClosePopup(Ptr<CComponent> pContent)
{
	size			uiIndex;
	Ptr<CPopup>		pPopup;
	Ptr<CComponent>	pClosedContent;
	Ptr<CComponent>	pOwner;

	if (!FindPopup(pContent, &uiIndex))
	{
		return false;
	}

	InvalidateLayout();

	//An owner told its popup has closed may close others, so the popups are counted again each time.
	while (maPopups.Size() > uiIndex)
	{
		pPopup = maPopups.Get(maPopups.Size() - 1);
		pClosedContent = pPopup->GetContent();
		pOwner = pPopup->GetOwner();
		mpCanvas->RemoveComponent(pPopup);
		pPopup->RemoveAllComponents();
		maPopups.RemoveAt(maPopups.Size() - 1);
		pPopup = NULL;
		if (pOwner.IsNotNull())
		{
			pOwner->PopupClosed(pClosedContent);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::CloseAllPopups(void)
{
	if (maPopups.Size() > 0)
	{
		ClosePopup(maPopups.Get(0)->GetContent());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Called when the pointer is pressed on pComponent, which can be NULL.
// Unless it is in a popup, or in a component that opened one, every popup
// is closed.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::ClosePopupsOutside(Ptr<CComponent> pComponent)
{
	size			i;
	Ptr<CPopup>		pPopup;

	if (pComponent.IsNotNull())
	{
		for (i = 0; i < maPopups.Size(); i++)
		{
			pPopup = maPopups.Get(i);
			if (pComponent->IsWithin(pPopup) || pComponent->IsWithin(pPopup->GetOwner()))
			{
				return;
			}
		}
	}
	CloseAllPopups();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CPopup> CWindow::GetPopup(Ptr<CComponent> pContent)
{
	size	uiIndex;

	if (FindPopup(pContent, &uiIndex))
	{
		return maPopups.Get(uiIndex);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Where in maPopups the popup holding pContent is.  False if pContent
// isn't in an open popup.
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::FindPopup(Ptr<CComponent> pContent, size* puiIndex)
{
	size	i;

	for (i = 0; i < maPopups.Size(); i++)
	{
		if (maPopups.Get(i)->GetContent() == pContent)
		{
			*puiIndex = i;
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::IsPopupOpen(Ptr<CComponent> pContent) { return GetPopup(pContent).IsNotNull(); }
size CWindow::NumPopups(void) { return maPopups.Size(); }


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
	if (mpSoundMixer.IsNotNull())
	{
		mpSoundMixer->Mix();
	}
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
	InvalidateLayout();
	return mpCanvas->SetContainer(pContainer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::ClearContainer(void)
{
	InvalidateLayout();
	return mpCanvas->ClearContainer();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::GetColourFormat(CColourFormatHelper* pcDest)
{
	mpCanvas->GetColourFormat(pcDest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCanvas>			CWindow::GetCanvas(void) { return mpCanvas; }
CNativeWindow*			CWindow::GetNativeWindow(void) { return mpcNativeWindow; }
Ptr<CFocus>				CWindow::GetFocus(void) { return &mFocus; }
Ptr<CSoundMixer>		CWindow::GetSoundMixer(void) { return mpSoundMixer; }
Ptr<CClipboard>			CWindow::GetClipboard(void) { return mpClipboard; }
Ptr<CButton>			CWindow::GetDefaultButton(void) { return mpDefaultButton; }
Ptr<CButton>			CWindow::GetCancelButton(void) { return mpCancelButton; }
CComponentInput*		CWindow::GetComponentInput(void) { return mpcComponentInput; }
Ptr<CContainer>			CWindow::GetContainer(void) { return mpCanvas->GetContainer(); }
CNativeWindowFactory*	CWindow::GetFactory(void) { return mpcNativeWindow->GetFactory(); }  //The native window was made by the factory.
EColourOrder			CWindow::GetColourOrder(void) { return mpCanvas->GetColourOrder(); }

