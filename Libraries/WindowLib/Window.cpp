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
#include "ToolTips.h"
#include "Window.h"


//What windows are made with, see CWindow::SetDefaultRedraw.
static bool	gbDefaultPartialRedraw = false;
static bool	gbDefaultVerifyRedraw = false;
static size	guiTotalRedrawMismatches = 0;


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
// sample rate, that mixes on its own thread while the window is shown.
// The canvas the window is drawn on is in pcFormat.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Init(const char* szTitle, CNativeWindowFactory* pcFactory, CNativeSoundFactory* pcSoundFactory, Ptr<CWindowTick> pTick, Ptr<CCanvasDraw> pDraw, CColourFormatHelper* pcFormat)
{
	PreInit();

	mszWindowTitle.Init(szTitle);
	mcDamage.Init();
	mcPaintedDamage.Init();
	mbPartialRedraw = gbDefaultPartialRedraw;
	mbVerifyRedraw = gbDefaultVerifyRedraw;
	muiRedrawMismatches = 0;

	mpcNativeWindow = pcFactory->CreateNativeWindow(this);
	CComponent::Init(this);

	mpWindowTick = pTick;
	mpCanvas = OMalloc<CCanvas>(this, pDraw, pcFormat);
	AddComponent(mpCanvas);
	mbTicking = false;
	mbModal = false;
	mpcComponentInput = NULL;
	mpCommands = NULL;
	mpToolTips = NULL;
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

	mcFocus.Init(this);
	mpClipboard = OMalloc<CClipboard>(this);
	mbContinuous = false;
	mbRedraw = true;
	mbLayoutDirty = true;
	mbCheckLayoutCache = WINDOW_CHECK_LAYOUT_CACHE;
	mpCaretBacking = NULL;
	msCloseApprover.Clear();
	mcCaret.Init();
	msCaretColour = 0;
	mbCaretPainted = false;
	mcActivated.Init();
	SetMaxFramesPerSecond(WINDOW_DEFAULT_FRAMES_PER_SECOND);
	miLastPaintTime = 0;
	miTickDelay = -1;
	mpOwner = NULL;
	maOwned.Init();
	masPosted.Init();
	mcClosed.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Free(void)
{
	mcClosed.Kill();
	GetFactory()->DestroyNativeWindow(mpcNativeWindow);
	mpcNativeWindow = NULL;

	mszWindowTitle.Kill();
	mcActivated.Kill();
	masPosted.Kill();

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
	M_Pointer(mpCaretBacking);
	M_Pointer(mpCommands);
	M_Pointer(mpToolTips);
	U_Unknown(SMemberCall, msCloseApprover);
	U_Data(CInt32Rect, mcCaret);
	U_UInt32(msCaretColour);
	U_Bool(mbCaretPainted);
	U_Bool(mbModal);
	U_Data(CDamageArea, mcDamage);
	U_Data(CDamageArea, mcPaintedDamage);
	U_Bool(mbPartialRedraw);
	U_Bool(mbVerifyRedraw);
	U_Size(muiRedrawMismatches);
	M_Pointer(mpWindowTick);
	U_Bool(mbTicking);
	M_Embedded(mcFocus);
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
	U_Unknown(CComponentEvent<CWindow>, mcActivated);
	U_UInt32(muiFrameMilliseconds);
	U_Int64(miLastPaintTime);
	U_Int64(miTickDelay);
	M_Pointer(mpOwner);
	M_Embedded(maOwned);
	U_Unknown(CArrayMemberCalls, masPosted);
	U_Unknown(CComponentEvent<CWindow>, mcClosed);
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
// back by the frame rate, a tick asked for with RequestTick or held chord
// inputs to let through.  The sound mixer, if there is one, mixes on its
// own thread from the first pass to the last, and the voices that have
// finished are taken out after each tick.  Input, the pointer, activation
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
	CreateOwned();
	mbRedraw = true;
	miTickDelay = -1;
	miLastPaintTime = GetDebugTickCount() - muiFrameMilliseconds;
	bFirst = true;
	if (mpSoundMixer.IsNotNull())
	{
		mpSoundMixer->Start();
	}
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
		PaintOwned(bContinuous);
		BlinkCaret(GetDebugTickCount());
		mcFocus.RepeatDrag(GetDebugTickCount());
		ShowToolTip(GetDebugTickCount());
		miTickDelay = -1;
		RunPosted();
		Tick(cTimer.GetUpdateTimeInMillieconds(), cTimer.GetTotalTimeInMillieconds());
	}
	if (mpSoundMixer.IsNotNull())
	{
		mpSoundMixer->Stop();
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
	int64			iWait;
	int64			iFrameWait;
	uint32			uiDelay;
	size			i;
	Ptr<CWindow>	pOwned;

	iWait = -1;
	if (mbRedraw)
	{
		iFrameWait = miLastPaintTime + muiFrameMilliseconds - GetDebugTickCount();
		iWait = (iFrameWait > 0) ? iFrameWait : 0;
	}
	iWait = ShorterWait(iWait, miTickDelay);
	iWait = ShorterWait(iWait, mcFocus.GetCaretBlinkWait(GetDebugTickCount()));
	iWait = ShorterWait(iWait, mcFocus.GetDragRepeatWait(GetDebugTickCount()));
	if (mpToolTips.IsNotNull())
	{
		iWait = ShorterWait(iWait, mpToolTips->GetWait(GetDebugTickCount()));
	}
	if (mpcComponentInput && mpcComponentInput->GetEvictionDelay(&uiDelay))
	{
		iWait = ShorterWait(iWait, uiDelay);
	}
	for (i = 0; i < maOwned.NumElements(); i++)
	{
		pOwned = maOwned.Get(i);
		if (pOwned->mbRedraw)
		{
			iFrameWait = pOwned->miLastPaintTime + pOwned->muiFrameMilliseconds - GetDebugTickCount();
			iWait = ShorterWait(iWait, (iFrameWait > 0) ? iFrameWait : 0);
		}
		iWait = ShorterWait(iWait, pOwned->mcFocus.GetCaretBlinkWait(GetDebugTickCount()));
		iWait = ShorterWait(iWait, pOwned->mcFocus.GetDragRepeatWait(GetDebugTickCount()));
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
// Whether this paint can draw only what was damaged: the window redraws
// partially, its canvas still fits it (it wasn't resized or shown for
// the first time) and its background is one colour that can be drawn in
// any part of it.  Without a background nothing would clear a damaged
// part before what is over it is drawn there again.
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::CanPaintPartially(void)
{
	ARGB32				sColour;
	Ptr<CCanvasDraw>	pDraw;

	if (!mbPartialRedraw || (mpCanvas->GetNativeCanvas() == NULL) || mpCanvas->HasNativeChanged())
	{
		return false;
	}
	pDraw = mpCanvas->GetCanvasDraw();
	return pDraw.IsNotNull() && pDraw->GetSolidColour(&sColour);
}


//////////////////////////////////////////////////////////////////////////
//
// Draws the damaged parts of the window, each in turn with nothing drawn
// outside it: its background and, in order, everything over it.
// Canvases that didn't change copy what they last drew.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Composite(void)
{
	size	i;

	if (mcDamage.IsAll())
	{
		mpCanvas->ClearPaintClip();
		Draw();
		return;
	}

	for (i = 0; i < mcDamage.NumRectangles(); i++)
	{
		mpCanvas->SetPaintClip(mcDamage.GetRectangle(i));
		Draw();
	}
	mpCanvas->ClearPaintClip();
}


//////////////////////////////////////////////////////////////////////////
//
// The canvases a partial paint left wrong under pcArea.
//
//////////////////////////////////////////////////////////////////////////
static void AppendDrawnAt(Ptr<CComponent> pComponent, CInt32Rect* pcArea, CChars* psz)
{
	CInt32Rect	cDrawn;
	size		i;

	pComponent->GetDrawnArea(&cDrawn);
	if (cDrawn.Intersect(pcArea))
	{
		psz->Append(" ");
		psz->Append(pComponent->ClassName());
	}
	for (i = 0; i < pComponent->NumComponents(); i++)
	{
		AppendDrawnAt(pComponent->GetComponent(i), pcArea, psz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Draws all of the window again over what the partial paint drew and
// logs where they differ: a canvas that changed without being
// invalidated, or something that moved without its area being damaged.
// The window is then all of it drawn, so it shows correctly, and all of
// it is shown if they differed.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::VerifyRedraw(void)
{
	Ptr<CImage>	pImage;
	size		uiBytes;
	size		uiPixelBytes;
	uint8*		puiPartial;
	uint8*		puiFull;
	int32		x;
	int32		y;
	int32		iWidth;
	int32		iHeight;
	CInt32Rect	cDiffers;
	size		uiPixels;
	CChars		sz;
	CDamageArea	cPartial;

	pImage = mpCanvas->GetImageOrNull();
	if (pImage.IsNull())
	{
		return;
	}
	iWidth = (int32)pImage->GetWidth();
	iHeight = (int32)pImage->GetHeight();
	uiBytes = pImage->GetByteSize();
	if ((iWidth <= 0) || (iHeight <= 0))
	{
		return;
	}
	uiPixelBytes = uiBytes / ((size)iWidth * (size)iHeight);
	puiPartial = (uint8*)malloc(uiBytes);
	memcpy(puiPartial, pImage->GetData(), uiBytes);

	cPartial = mcDamage;
	ContentChanged();
	mcDamage.AddAll();
	Composite();

	puiFull = (uint8*)pImage->GetData();
	uiPixels = 0;
	cDiffers.Init(iWidth, iHeight, 0, 0);
	for (y = 0; y < iHeight; y++)
	{
		if (memcmp(&puiPartial[(size)y * iWidth * uiPixelBytes], &puiFull[(size)y * iWidth * uiPixelBytes], iWidth * uiPixelBytes) == 0)
		{
			continue;
		}
		for (x = 0; x < iWidth; x++)
		{
			if (memcmp(&puiPartial[((size)y * iWidth + x) * uiPixelBytes], &puiFull[((size)y * iWidth + x) * uiPixelBytes], uiPixelBytes) != 0)
			{
				uiPixels++;
				cDiffers.GrowToContain(x, y);
			}
		}
	}
	SafeFree(puiPartial);

	if (uiPixels > 0)
	{
		muiRedrawMismatches++;
		guiTotalRedrawMismatches++;
		sz.Init(" A partial paint of [");
		sz.Append(mszWindowTitle.Text());
		sz.Append("] differs from drawing all of it in ");
		sz.Append((uint64)uiPixels);
		sz.Append(" pixels from [");
		sz.Append(cDiffers.GetLeft());
		sz.Append(", ");
		sz.Append(cDiffers.GetTop());
		sz.Append("] to [");
		sz.Append(cDiffers.GetRight());
		sz.Append(", ");
		sz.Append(cDiffers.GetBottom());
		sz.Append("], drawn by:");
		AppendDrawnAt(mpCanvas, &cDiffers, &sz);
		gcLogger.Error2(__METHOD__, sz.Text(), NULL);
		sz.Kill();
	}
	else
	{
		//They match, so only what the partial paint drew need be shown.
		mcDamage = cPartial;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Shows what this paint drew: all of the window, or its damaged parts and
// the caret drawn over them.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::PresentDamage(void)
{
	CDamageArea	cShown;

	cShown = mcDamage;
	if (mbCaretPainted)
	{
		cShown.Add(&mcCaret);
	}
	mpcNativeWindow->SignalPresentDamage(&cShown);
}


//////////////////////////////////////////////////////////////////////////
//
// Paints draw only what changed: components that were invalidated, moved,
// appeared or went, and what is over and under them.  Everything else
// keeps what it last drew.  Off, every paint draws all of the window.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetPartialRedraw(bool bPartialRedraw)
{
	mbPartialRedraw = bPartialRedraw;
	Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether windows made from now on redraw partially and verify their
// partial paints, as SetPartialRedraw and SetVerifyRedraw; so a test
// suite can run with every window redrawing partially.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetDefaultRedraw(bool bPartialRedraw, bool bVerifyRedraw)
{
	gbDefaultPartialRedraw = bPartialRedraw;
	gbDefaultVerifyRedraw = bVerifyRedraw;
}


//////////////////////////////////////////////////////////////////////////
//
// Partial paints of every window that differed from drawing all of it.
//
//////////////////////////////////////////////////////////////////////////
size CWindow::GetTotalRedrawMismatches(void)
{
	return guiTotalRedrawMismatches;
}


//////////////////////////////////////////////////////////////////////////
//
// Each partial paint also draws all of the window and logs where the two
// differ, finding what changed without being invalidated.  Slow; for
// tests and while making a window redraw partially.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetVerifyRedraw(bool bVerifyRedraw)
{
	mbVerifyRedraw = bVerifyRedraw;
}


//////////////////////////////////////////////////////////////////////////
//
// Redraws partially, and verifies, as pWindow does: a dialog or tool
// window as the window it belongs to.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetRedrawLike(Ptr<CWindow> pWindow)
{
	SetPartialRedraw(pWindow->mbPartialRedraw);
	SetVerifyRedraw(pWindow->mbVerifyRedraw);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::IsPartialRedraw(void) { return mbPartialRedraw; }
bool CWindow::IsVerifyRedraw(void) { return mbVerifyRedraw; }
size CWindow::GetRedrawMismatches(void) { return muiRedrawMismatches; }


//////////////////////////////////////////////////////////////////////////
//
// All of the window is drawn again, as soon as its frame rate allows.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Invalidate(void)
{
	ContentChanged();
	mcDamage.AddAll();
	mbRedraw = true;
}


//////////////////////////////////////////////////////////////////////////
//
// pcArea of the window, in client space, must be drawn again at the next
// paint.  An empty area adds nothing.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::AddDamage(CInt32Rect* pcArea)
{
	mcDamage.Add(pcArea);
}


//////////////////////////////////////////////////////////////////////////
//
// The window is painted soon, drawing what has been damaged.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::RequestPaint(void)
{
	mbRedraw = true;
}


//////////////////////////////////////////////////////////////////////////
//
// What had changed when the window was last painted: what components
// were invalidated, moved, appeared or went.  For tests.
//
//////////////////////////////////////////////////////////////////////////
CDamageArea* CWindow::GetPaintedDamage(void)
{
	return &mcPaintedDamage;
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
// The window will be laid out again before it is next painted.
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::IsLayoutDirty(void)
{
	return mbLayoutDirty;
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
// Shows the window as a dialog over pOwner and returns once it stops.  A
// floating tool window can be the owner: the window it floats over and
// every tool window over that are disabled too.
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
	Ptr<CWindow>		pRoot;

	//A dialog over a floating tool window disables the main window it floats over and all its tool windows.
	pRoot = pOwner;
	while (pRoot->mpOwner.IsNotNull())
	{
		pRoot = pRoot->mpOwner;
	}
	pOwner->CloseAllPopups();
	pOwner->Deactivate();
	pcOwnerNative = pOwner->GetNativeWindow();
	pRoot->GetNativeWindow()->SetEnabled(false);
	pRoot->SetOwnedEnabled(false);

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
	mbModal = true;
	bResult = Show();
	mbModal = false;
	mcFocus.Deactivate();

	//The owner is enabled before the dialog goes so that the owner is the window activated.
	pRoot->GetNativeWindow()->SetEnabled(true);
	pRoot->SetOwnedEnabled(true);
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
// A dialog with a minimum size can be resized, no smaller than it (-1, -1
// for one that can't be).  See CNativeWindow::SetMinimumSize.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetMinimumWindowSize(int32 iWidth, int32 iHeight)
{
	mpcNativeWindow->SetMinimumSize(iWidth, iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Paint(void)
{
	LayoutIfChanged();

	//Where each component draws this paint, and so what moved, appeared or went since the last one.
	PrepareDraw(true, false, NULL);
	mcPaintedDamage = mcDamage;

	//The caret was drawn into the window: what it covered is drawn again.
	if (mbCaretPainted)
	{
		AddDamage(&mcCaret);
	}

	if (!CanPaintPartially())
	{
		ContentChanged();
		mcDamage.AddAll();
	}
	if (!mcDamage.IsEmpty())
	{
		Composite();
		if (mbVerifyRedraw && !mcDamage.IsAll())
		{
			VerifyRedraw();
		}
	}

	PaintCaret();
	PresentDamage();
	mcDamage.Clear();

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
	mcFocus.Refresh();
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
//the window looks like, so it is laid out again too, but not for a key typing a character (that only changes text,
//and what shows it lays itself out again if its size changed, as a text area does), for input taken by actions that
//keep the layout (CInputAction::SetKeepsLayout, the text editing commands), or for a key's release nothing took.  The
//pointer moving, and the mouse's movement (CWindow::Input's WIET_Delta), only change what it is over; a component that
//changes as the mouse moves, or moves things as it is dragged, invalidates itself or its layout.
//A tool tip waits for the pointer to rest; input takes it away.
void CWindow::PointerMoved(int x, int y) { mcFocus.PointerMoved(x, y); if (mpToolTips.IsNotNull()) { mpToolTips->PointerMoved(mcFocus.GetMouseOverComponent(), GetDebugTickCount()); } RequestPaint(); }
void CWindow::PointerLeft(void) { mcFocus.PointerLeft(); if (mpToolTips.IsNotNull()) { mpToolTips->PointerLeft(); } RequestPaint(); }
void CWindow::InputCommand(EComponentInputCommand eCommand, bool bInsert) { DismissToolTip(); if (mcFocus.Command(eCommand, bInsert)) { RequestPaint(); } else { InvalidateLayout(); } }
bool CWindow::InputKeys(const char* szKeys) { bool bTaken; DismissToolTip(); bTaken = mcFocus.Keys(szKeys); InvalidateLayout(); return bTaken; }  //A command's keys, as a test gives them.
void CWindow::InputPrintable(char c, bool bInsert) { DismissToolTip(); mcFocus.Printable(c, bInsert); RequestPaint(); }


//////////////////////////////////////////////////////////////////////////
//
// The window became the active window (the platform's focus came to it):
// the activation event is called, as for an IDE to see whether its files
// were changed while it wasn't.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Activate(void)
{
	ClaimInput();
	mcFocus.Activate();
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
	bool	bLayoutKept;

	if (psWindowEvent->eType != WIET_Delta)
	{
		DismissToolTip();
	}
	bLayoutKept = mcFocus.Input(psWindowEvent, pcComponentInput);
	if ((psWindowEvent->eType == WIET_Delta) || bLayoutKept)
	{
		RequestPaint();
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
	DismissToolTip();
	mcFocus.Deactivate();
	//A tool window of the same owner that took the input, by being pressed, keeps what is held.
	if (mpcComponentInput && (mpcComponentInput->GetWindow() == this))
	{
		mpcComponentInput->ReleaseAll();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The window doesn't own the component input.  Kill it after the window
// stops showing.  The tool windows it owns that share its input get the
// new one too: one shown before the window had its input, as a layout is
// read before the application makes its input, takes it now.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetComponentInput(CComponentInput* pcComponentInput)
{
	CComponentInput*	pcOld;
	size				i;
	Ptr<CWindow>		pOwned;

	pcOld = mpcComponentInput;
	mpcComponentInput = pcComponentInput;
	for (i = 0; i < maOwned.NumElements(); i++)
	{
		pOwned = maOwned.Get(i);
		if (pOwned->mpcComponentInput == pcOld)
		{
			pOwned->SetComponentInput(pcComponentInput);
		}
	}
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
	while (maPopups.NumElements() > uiIndex)
	{
		pPopup = maPopups.Get(maPopups.NumElements() - 1);
		pClosedContent = pPopup->GetContent();
		pOwner = pPopup->GetOwner();
		mpCanvas->RemoveComponent(pPopup);
		pPopup->RemoveAllComponents();
		maPopups.RemoveAt(maPopups.NumElements() - 1);
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
	if (maPopups.NumElements() > 0)
	{
		ClosePopup(maPopups.Get(0)->GetContent());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Called when the pointer is pressed on pComponent, which can be NULL.
// Unless it is in a popup, or in a component that opened one that a press
// on its owner keeps open, every popup is closed.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::ClosePopupsOutside(Ptr<CComponent> pComponent)
{
	size			i;
	Ptr<CPopup>		pPopup;

	if (pComponent.IsNotNull())
	{
		for (i = 0; i < maPopups.NumElements(); i++)
		{
			pPopup = maPopups.Get(i);
			if (pComponent->IsWithin(pPopup) || (pPopup->IsOwnerPressKeepsOpen() && pComponent->IsWithin(pPopup->GetOwner())))
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

	for (i = 0; i < maPopups.NumElements(); i++)
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
size CWindow::NumPopups(void) { return maPopups.NumElements(); }


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
		mpSoundMixer->RemoveFinishedVoices();
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
// Whether the platform's close button may close the window: what the
// close approver answers, true without one.
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::ApproveClose(void)
{
	return !msCloseApprover.IsSet() || msCloseApprover.Ask();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::ClearCloseApprover(void)
{
	msCloseApprover.Clear();
}


size CWindow::NumPosted(void) { return masPosted.NumElements(); }  //The calls posted and not yet made.


//////////////////////////////////////////////////////////////////////////
//
// The calls posted (see Post), in the order they were.  One may post
// another: it is called at the tick after.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::RunPosted(void)
{
	CArrayMemberCalls	asCalls;
	size				i;

	if (masPosted.NumElements() == 0)
	{
		return;
	}
	asCalls.Init();
	asCalls.Copy(&masPosted);
	masPosted.RemoveRange(0, masPosted.NumElements());
	for (i = 0; i < asCalls.NumElements(); i++)
	{
		asCalls.Get(i)->Call(NULL);
	}
	asCalls.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The calls posted on pvObject aren't made: it is going.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::CancelPosted(void* pvObject)
{
	size	i;

	for (i = masPosted.NumElements(); i > 0; i--)
	{
		if (masPosted.Get(i - 1)->IsOn(pvObject))
		{
			masPosted.RemoveAt(i - 1);
		}
	}
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
Ptr<CFocus>				CWindow::GetFocus(void) { return &mcFocus; }
Ptr<CSoundMixer>		CWindow::GetSoundMixer(void) { return mpSoundMixer; }
Ptr<CClipboard>			CWindow::GetClipboard(void) { return mpClipboard; }
Ptr<CButton>			CWindow::GetDefaultButton(void) { return mpDefaultButton; }
Ptr<CButton>			CWindow::GetCancelButton(void) { return mpCancelButton; }
CComponentInput*		CWindow::GetComponentInput(void) { return mpcComponentInput; }
void					CWindow::SetCommands(Ptr<CCommandSet> pCommands) { mpCommands = pCommands; }  //Keys nothing focused takes run these; see CFocus::DispatchUnhandledCommand.
Ptr<CCommandSet>		CWindow::GetCommands(void) { return mpCommands; }
void					CWindow::SetToolTips(Ptr<CToolTips> pToolTips) { mpToolTips = pToolTips; }  //Made for this window; NULL shows none.
Ptr<CToolTips>			CWindow::GetToolTips(void) { return mpToolTips; }
Ptr<CContainer>			CWindow::GetContainer(void) { return mpCanvas->GetContainer(); }
CNativeWindowFactory*	CWindow::GetFactory(void) { return mpcNativeWindow->GetFactory(); }  //The native window was made by the factory.
EColourOrder			CWindow::GetColourOrder(void) { return mpCanvas->GetColourOrder(); }


//////////////////////////////////////////////////////////////////////////
//
// Shows the window as a tool window floating over pOwner, its client
// area at pcScreenRectangle, and returns straight away; if pOwner isn't
// shown yet, when it is.  It isn't modal:
// the owner's loop lays it out and paints it with its own, and the
// owner's component input comes to it while it is the active window.  It
// has no frame; its content draws its title and edges.  It stays until
// CloseOwned, or until the platform closes it, then the closed event is
// called.
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::ShowOwned(Ptr<CWindow> pOwner, CInt32Rect* pcScreenRectangle)
{
	if (pOwner.IsNull() || (pOwner == this))
	{
		return false;
	}
	mpcNativeWindow->SetOwner(pOwner->GetNativeWindow());
	mpcNativeWindow->SetTool(true);
	mpcNativeWindow->SetScreenRectangle(pcScreenRectangle);
	//An owner not on the screen yet, its layout read before it is shown say, makes it when it is.
	if (pOwner->mbTicking && !mpcNativeWindow->CreateNativeWindow())
	{
		mpcNativeWindow->SetOwner(NULL);
		return false;
	}
	mpOwner = pOwner;
	pOwner->maOwned.Add(this);
	SetComponentInput(pOwner->GetComponentInput());
	mbTicking = true;
	mbRedraw = true;
	mbLayoutDirty = true;
	miLastPaintTime = GetDebugTickCount() - muiFrameMilliseconds;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes a tool window off the screen.  Its owner gets the input back if
// it had it.  It can be shown again.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::CloseOwned(void)
{
	Ptr<CWindow>	pOwner;

	if (mpOwner.IsNull())
	{
		return;
	}
	pOwner = mpOwner;
	CloseAllPopups();
	mcFocus.Deactivate();
	if (mpcComponentInput && (mpcComponentInput->GetWindow() == this))
	{
		mpcComponentInput->SetWindow(&pOwner);
	}
	SetComponentInput(NULL);
	mpcNativeWindow->DestroyNativeWindow();
	mpcNativeWindow->SetOwner(NULL);
	mpcNativeWindow->SetTool(false);
	mbTicking = false;
	mpOwner = NULL;
	pOwner->maOwned.Remove(this);
	pOwner->Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
// The tool windows shown before this window was are put on the screen.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::CreateOwned(void)
{
	size			i;
	Ptr<CWindow>	pOwned;

	for (i = 0; i < maOwned.NumElements(); i++)
	{
		pOwned = maOwned.Get(i);
		if (!pOwned->mpcNativeWindow->IsCreated())
		{
			pOwned->mpcNativeWindow->SetOwner(mpcNativeWindow);
			pOwned->mpcNativeWindow->CreateNativeWindow();
			pOwned->Invalidate();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// After the window is drawn: what the focussed component's caret covers
// is kept, then the caret is drawn over it if it is shown.  It isn't
// drawn under an open popup, a menu over the text say.  A caret that has
// moved is shown again.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::PaintCaret(void)
{
	CInt32Rect			cCaret;
	ARGB32				sColour;
	CInt32Rect			cPopup;
	size				i;
	bool				bMoved;
	CColourFormatHelper	cFormat;

	mbCaretPainted = false;
	if (!mcFocus.GetCaret(&cCaret, &sColour) || (cCaret.GetWidth() <= 0) || (cCaret.GetHeight() <= 0))
	{
		return;
	}
	for (i = 0; i < maPopups.NumElements(); i++)
	{
		cPopup.Init(maPopups.Get(i)->GetPosition().x, maPopups.Get(i)->GetPosition().y, maPopups.Get(i)->GetPosition().x + maPopups.Get(i)->GetActualSize().x, maPopups.Get(i)->GetPosition().y + maPopups.Get(i)->GetActualSize().y);
		if (cPopup.Intersect(&cCaret))
		{
			return;
		}
	}

	bMoved = !cCaret.Equals(&mcCaret);
	mcCaret.Init(&cCaret);
	msCaretColour = sColour;
	if (mpCaretBacking.IsNull())
	{
		//Made the first time a caret is drawn, in the window's colour format.
		mpCanvas->GetColourFormat(&cFormat);
		mpCaretBacking = OMalloc<CCanvas>(this, Ptr<CCanvasDraw>(), &cFormat);
	}
	if (!mpCaretBacking->PrepareNativeCanvas(SInt32Vec2(cCaret.GetWidth(), cCaret.GetHeight())))
	{
		return;
	}
	mpCaretBacking->DrawCanvas(-cCaret.GetLeft(), -cCaret.GetTop(), mpCanvas);
	mbCaretPainted = true;
	if (bMoved)
	{
		mcFocus.ResetCaretBlink();
	}
	DrawCaret(mcFocus.IsCaretOn());
}


//////////////////////////////////////////////////////////////////////////
//
// The caret, or what it covers, onto the window's canvas.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::DrawCaret(bool bOn)
{
	CInt32Rect	cBox;

	if (bOn)
	{
		//A box includes its right and bottom edges.
		cBox.Init(mcCaret.GetLeft(), mcCaret.GetTop(), mcCaret.GetRight() - 1, mcCaret.GetBottom() - 1);
		mpCanvas->DrawBox(&cBox, true, msCaretColour);
	}
	else
	{
		mpCanvas->DrawCanvas(mcCaret.GetLeft(), mcCaret.GetTop(), mpCaretBacking);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the tool tip of the component the pointer has rested on, if it
// is due (see CToolTips).  True if it showed: its popup lays the window
// out again.
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::ShowToolTip(int64 iNow)
{
	return mpToolTips.IsNotNull() && mpToolTips->ShowIfDue(iNow);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::DismissToolTip(void)
{
	if (mpToolTips.IsNotNull())
	{
		mpToolTips->Dismiss();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Shows or hides the caret when its period is up (see CFocus::
// BlinkCaret) by changing only its pixels, without painting the window:
// a paint is ~20 ms of a big window, this is a few pixels.  A paint due
// draws it anyway.  True if it blinked.
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::BlinkCaret(int64 iNow)
{
	if (!mcFocus.BlinkCaret(iNow))
	{
		return false;
	}
	if (mbCaretPainted && !mbRedraw)
	{
		DrawCaret(mcFocus.IsCaretOn());
		mpcNativeWindow->SignalPresentArea(&mcCaret);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the last paint drew the caret, or would have while it is hidden.
// False if it didn't.
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::GetPaintedCaret(CInt32Rect* pcDest)
{
	if (!mbCaretPainted)
	{
		return false;
	}
	pcDest->Init(&mcCaret);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Lays out and paints the tool windows that have changed.  One the
// platform closed is taken off and its closed event called.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::PaintOwned(bool bContinuous)
{
	size			i;
	Ptr<CWindow>	pOwned;

	for (i = 0; i < maOwned.NumElements();)
	{
		pOwned = maOwned.Get(i);
		if (!pOwned->mpcNativeWindow->IsCreated())
		{
			pOwned->CloseOwned();
			pOwned->mcClosed.Call(&pOwned);
			continue;
		}
		if (bContinuous || (pOwned->mbRedraw && pOwned->IsFrameDue()))
		{
			pOwned->Paint();
		}
		pOwned->BlinkCaret(GetDebugTickCount());
		pOwned->mcFocus.RepeatDrag(GetDebugTickCount());
		i++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A modal dialog disables its owner's tool windows too.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetOwnedEnabled(bool bEnabled)
{
	size			i;
	Ptr<CWindow>	pOwned;

	for (i = 0; i < maOwned.NumElements(); i++)
	{
		pOwned = maOwned.Get(i);
		if (!bEnabled)
		{
			pOwned->CloseAllPopups();
			pOwned->Deactivate();
		}
		pOwned->mpcNativeWindow->SetEnabled(bEnabled);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The component input, shared by a window and its tool windows, comes to
// this window: it was activated or pressed.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::ClaimInput(void)
{
	if (mpcComponentInput && (mpcComponentInput->GetWindow() != this))
	{
		mpcComponentInput->SetWindow(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The client area's place on the screen.  A window the platform can't
// move, a reference window, is moved anyway as far as WindowLib knows.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetScreenRectangle(CInt32Rect* pcRectangle)
{
	mpcNativeWindow->SetScreenRectangle(pcRectangle);
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::GetScreenRectangle(CInt32Rect* pcDest)
{
	mpcNativeWindow->GetScreenRectangle(pcDest);
}


//////////////////////////////////////////////////////////////////////////
//
// Where the platform says the pointer is or, if it can't, where the
// window last saw it relative to its client area.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::GetPointerScreenPosition(SInt32Vec2* psDest)
{
	CInt32Rect	cScreen;
	SInt32Vec2	sPointer;

	if (mpcNativeWindow->GetPointerScreenPosition(psDest))
	{
		return;
	}
	mpcNativeWindow->GetScreenRectangle(&cScreen);
	sPointer = mcFocus.GetPointer();
	psDest->Init(cScreen.GetLeft() + sPointer.x, cScreen.GetTop() + sPointer.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindow::GetScreenArea(CInt32Rect* pcDest)
{
	return mpcNativeWindow->GetScreenArea(pcDest);
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer takes the shape of the component dragging, or else of the
// component under it or its nearest parent with a shape.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::UpdateCursor(void)
{
	Ptr<CComponent>	pComponent;
	ENativeCursor	eCursor;

	eCursor = NC_Arrow;
	pComponent = mcFocus.GetDragComponent();
	if (pComponent.IsNull() || (pComponent->GetCursor() == NC_Arrow))
	{
		pComponent = mcFocus.GetMouseOverComponent();
	}
	while (pComponent.IsNotNull())
	{
		eCursor = pComponent->GetCursor();
		if (eCursor != NC_Arrow)
		{
			break;
		}
		pComponent = pComponent->GetParent();
	}
	mpcNativeWindow->SetCursor(eCursor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindow::Raise(void)
{
	mpcNativeWindow->Raise();
}


bool CWindow::IsShown(void) { return mpcNativeWindow->IsCreated() && ((mpOwner.IsNotNull()) || mbTicking); }
bool CWindow::IsModal(void) { return mbModal; }
Ptr<CWindow> CWindow::GetOwner(void) { return mpOwner; }
size CWindow::NumOwned(void) { return maOwned.NumElements(); }
CComponentEvent<CWindow>* CWindow::GetClosedEvent(void) { return &mcClosed; }


//////////////////////////////////////////////////////////////////////////
//
// The platform shows the new title, if the window is on the screen.
//
//////////////////////////////////////////////////////////////////////////
void CWindow::SetWindowTitle(const char* szTitle)
{
	if (mszWindowTitle.Equals(szTitle))
	{
		return;
	}
	mszWindowTitle.Set(szTitle);
	mpcNativeWindow->TitleChanged();
}

