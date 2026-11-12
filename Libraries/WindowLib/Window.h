/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __WINDOW_H__
#define __WINDOW_H__
#include "Component.h"
#include "Canvas.h"
#include "NativeWindow.h"
#include "CanvasDraw.h"
#include "ComponentInputCommand.h"
#include "Popup.h"
#include "DamageArea.h"
#include "SoundMixer.h"
#include "Clipboard.h"
#include "CommandSet.h"
#include "ComponentEvent.h"
#include "StandardLib/MemberCall.h"


#define WINDOW_DEFAULT_FRAMES_PER_SECOND	60  //At most one paint every 17 ms; at 30 a key typed soon after a paint waited up to 34 ms for the next.


class CWindowTick;
class CNativeSoundFactory;
class CComponentInput;
class CButton;
class CToolTips;


typedef CArrayTemplate<SMemberCall>	CArrayMemberCalls;


class CWindow : public CComponent
{
CONSTRUCTABLE(CWindow);
DESTRUCTABLE(CWindow);
protected:
	CChars						mszWindowTitle;
	CNativeWindow*				mpcNativeWindow;
	Ptr<CCanvas>				mpCanvas;
	Ptr<CWindowTick>			mpWindowTick;
	bool						mbTicking;
	CFocus						mcFocus;
	CComponentInput*			mpcComponentInput;  //Optional.  Updated every tick when set.
	Ptr<CCommandSet>			mpCommands;  //The application's commands, NULL for none.  Dialogs and tool windows use their owner's.
	Ptr<CToolTips>				mpToolTips;  //Shows the components' tool tips, NULL for none (see SetToolTips).
	Ptr<CButton>				mpDefaultButton;  //Clicked by enter unless the focussed component wants it.
	Ptr<CButton>				mpCancelButton;  //Clicked by escape when no popup is open.
	bool						mbLayoutAgain;  //A component's required size changed while it was being laid out.
	bool						mbLayingOut;  //Layout is laying the components out: LayoutAgain asks for another pass of it.
	CArray<CPopup>				maPopups;  //Open popups, the last opened last.  Each opened by one before it is closed with it.
	Ptr<CSoundMixer>			mpSoundMixer;  //Only if the window was given a sound factory.  Mixes on its own thread while the window is shown.
	Ptr<CClipboard>				mpClipboard;  //The system's clipboard.
	bool						mbContinuous;  //Ticked and painted every pass, as a game is, rather than waiting for events.
	bool						mbRedraw;  //Something has changed since the window was last painted.
	bool						mbLayoutDirty;  //Something the layout depends on has changed since the window was last laid out.
	bool						mbCheckLayoutCache;  //Lay out every paint anyway and log components that moved, see CheckLayoutCache.
	CComponentEvent<CWindow>	mcActivated;  //Called when the window becomes the active window again.
	uint32						muiFrameMilliseconds;  //The least time between paints while waiting for events, 0 for none.
	int64						miLastPaintTime;  //GetDebugTickCount when last painted.
	int64						miTickDelay;  //Milliseconds after this tick that another was asked for, -1 if none was.
	Ptr<CWindow>				mpOwner;  //The window a tool window floats over, see ShowOwned.  NULL for the others.
	bool						mbModal;  //It's being shown by ShowModal, a dialog.
	bool						mbKeepsActive;  //It stays active when the platform's focus leaves it (SetKeepsActive).
	CArray<CWindow>				maOwned;  //Tool windows shown over this one, run by its loop.
	CComponentEvent<CWindow>	mcClosed;  //A tool window was closed by the platform, Alt+F4 say.
	SMemberCall					msCloseApprover;  //Asked whether the close button may close the window, not set if it always may.
	CArrayMemberCalls			masPosted;  //Called at the next tick, outside the input that posted them (see Post).
	Ptr<CCanvas>				mpCaretBacking;  //What the caret covers, kept so hiding it needn't paint the window.
	CInt32Rect					mcCaret;  //Where the focussed component's caret was drawn, window space, right and bottom exclusive.
	ARGB32						msCaretColour;
	bool						mbCaretPainted;  //The caret's place was kept by the last paint: blinking only changes it.
	CDamageArea					mcDamage;  //What must be drawn again at the next paint, see AddDamage.
	CDamageArea					mcPaintedDamage;  //What changed for the last paint, whether or not all of the window was drawn.
	bool						mbPartialRedraw;  //Paints draw only what was damaged, see SetPartialRedraw.
	bool						mbVerifyRedraw;  //Each partial paint is checked against drawing all of the window, see SetVerifyRedraw.
	size						muiRedrawMismatches;  //Partial paints that differed from drawing all of the window.

public:
	void					Init(const char* szTitle, CNativeWindowFactory* pcFactory, Ptr<CWindowTick>	pTick, Ptr<CCanvasDraw> pDraw, EColourFormat eFormat, EColourOrder eOrder, ERGBColourBits eColourBits, ERGBAlphaBits eAlphaBits);
	void					Init(const char* szTitle, CNativeWindowFactory* pcFactory, CNativeSoundFactory* pcSoundFactory, Ptr<CWindowTick>	pTick, Ptr<CCanvasDraw> pDraw, EColourFormat eFormat, EColourOrder eOrder, ERGBColourBits eColourBits, ERGBAlphaBits eAlphaBits);
	void					Init(const char* szTitle, CNativeWindowFactory* pcFactory, Ptr<CWindowTick>	pTick, Ptr<CCanvasDraw> pDraw, CColourFormatHelper* pcFormat);
	void					Init(const char* szTitle, CNativeWindowFactory* pcFactory, CNativeSoundFactory* pcSoundFactory, Ptr<CWindowTick>	pTick, Ptr<CCanvasDraw> pDraw, CColourFormatHelper* pcFormat);
	void					Class(void) override;
	void					Free(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	bool						Show(void);
	bool						ShowModal(Ptr<CWindow> pOwner);
	bool						ShowOwned(Ptr<CWindow> pOwner, CInt32Rect* pcScreenRectangle);
	void						CloseOwned(void);
	bool						IsShown(void);
	bool						IsModal(void);
	Ptr<CWindow>				GetOwner(void);
	size						NumOwned(void);
	CComponentEvent<CWindow>*	GetClosedEvent(void);
	void						ClaimInput(void);
	void						SetScreenRectangle(CInt32Rect* pcRectangle);
	void						GetScreenRectangle(CInt32Rect* pcDest);
	void						GetPointerScreenPosition(SInt32Vec2* psDest);
	bool						GetScreenArea(CInt32Rect* pcDest);
	void						UpdateCursor(void);
	void						Raise(void);
	void						SetWindowSize(int32 iWidth, int32 iHeight);
	void						SetMinimumWindowSize(int32 iWidth, int32 iHeight);
	CNativeWindow*				GetNativeWindow(void);
	void						Paint(void);
	bool						BlinkCaret(int64 iNow);
	bool						GetPaintedCaret(CInt32Rect* pcDest);
	void						Invalidate(void) override;
	void						InvalidateLayout(void) override;
	bool						IsLayoutDirty(void);
	void						AddDamage(CInt32Rect* pcArea);
	void						RequestPaint(void);
	CDamageArea*				GetPaintedDamage(void);
	static void					SetDefaultRedraw(bool bPartialRedraw, bool bVerifyRedraw);
	static size					GetTotalRedrawMismatches(void);
	void						SetPartialRedraw(bool bPartialRedraw);
	bool						IsPartialRedraw(void);
	void						SetVerifyRedraw(bool bVerifyRedraw);
	bool						IsVerifyRedraw(void);
	void						SetRedrawLike(Ptr<CWindow> pWindow);
	size						GetRedrawMismatches(void);
	void						RequestTick(uint32 uiMilliseconds);
	void						SetContinuous(bool bContinuous);
	bool						IsContinuous(void);
	void						SetMaxFramesPerSecond(uint32 uiFramesPerSecond);
	void						SetCheckLayoutCache(bool bCheckLayoutCache);
	void						Tick(int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds);
	void						Stop(void);
	template<class M, class O>
	void					SetCloseApprover(O* pcObject, bool(M::*fApprove)(void));
	template<class M, class O>
	void					Post(O* pcObject, void(M::*fFunction)(void));
	void					CancelPosted(void* pvObject);
	void					RunPosted(void);
	size					NumPosted(void);
	void					ClearCloseApprover(void);
	bool					ApproveClose(void);
	void					Layout(void);
	void					LayoutAgain(void);

	//Input in window client space.  Native windows call these.
	void						PointerMoved(int x, int y);
	void						PointerLeft(void);
	void						Input(SWindowInputEvent* psWindowEvent, CComponentInput* pcComponentInput);
	void						InputCommand(EComponentInputCommand eCommand, bool bInsert);
	bool						InputKeys(const char* szKeys);
	void						InputPrintable(char c, bool bInsert);
	void						Activate(void);
	CComponentEvent<CWindow>*	GetActivationEvent(void);
	void						Deactivate(void);
	void						PlatformDeactivated(void);
	void						SetKeepsActive(bool bKeepsActive);
	bool						KeepsActive(void);

	void					SetComponentInput(CComponentInput* pcComponentInput);
	CComponentInput*		GetComponentInput(void);
	void					SetCommands(Ptr<CCommandSet> pCommands);
	Ptr<CCommandSet>		GetCommands(void);
	void					SetToolTips(Ptr<CToolTips> pToolTips);
	Ptr<CToolTips>			GetToolTips(void);
	bool					ShowToolTip(int64 iNow);

	const char*				GetWindowTitle(void);
	void					SetWindowTitle(const char* szTitle);
	Ptr<CCanvas>			GetCanvas(void);
	Ptr<CFocus>				GetFocus(void);
	Ptr<CSoundMixer>		GetSoundMixer(void);
	Ptr<CClipboard>			GetClipboard(void);

	void					SetDefaultButton(Ptr<CButton> pButton);
	Ptr<CButton>			GetDefaultButton(void);
	void					SetCancelButton(Ptr<CButton> pButton);
	Ptr<CButton>			GetCancelButton(void);

	Ptr<CPopup>				OpenPopup(Ptr<CComponent> pContent, SInt32Vec2 sPosition, Ptr<CComponent> pOwner);
	bool					ClosePopup(Ptr<CComponent> pContent);
	void					CloseAllPopups(void);
	void					ClosePopupsOutside(Ptr<CComponent> pComponent);
	Ptr<CPopup>				GetPopup(Ptr<CComponent> pContent);
	bool					IsPopupOpen(Ptr<CComponent> pContent);
	size					NumPopups(void);

	bool					SetContainer(Ptr<CContainer> pContainer);
	Ptr<CContainer>			GetContainer(void);
	bool					ClearContainer(void);

	CNativeWindowFactory*	GetFactory(void);

	EColourOrder			GetColourOrder(void);
	void					GetColourFormat(CColourFormatHelper* pcDest);

protected:
	bool					FindPopup(Ptr<CComponent> pContent, size* puiIndex);
	void					LayoutIfChanged(void);
	void					CheckLayoutCache(void);
	void					DismissToolTip(void);
	int64					GetWaitMilliseconds(void);
	bool					IsFrameDue(void);
	void					PaintOwned(bool bContinuous);
	void					PaintCaret(void);
	bool					CanPaintPartially(void);
	void					Composite(void);
	void					VerifyRedraw(void);
	void					PresentDamage(void);
	void					DrawCaret(bool bOn);
	void					CreateOwned(void);
	void					SetOwnedEnabled(bool bEnabled);
};




//////////////////////////////////////////////////////////////////////////
//
// The platform's close button (or Alt+F4) only closes the window if
// fApprove answers true, so an application can ask to save its changes
// first.  The object isn't held; it calls ClearCloseApprover before it
// goes.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CWindow::SetCloseApprover(O* pcObject, bool(M::*fApprove)(void))
{
	msCloseApprover.Init(pcObject, fApprove);
}


//////////////////////////////////////////////////////////////////////////
//
// fFunction is called at the next tick, from the window's loop rather
// than the input (a key, a menu click) that posts it: what can't be done
// from an input action, like showing a dialog (see ShowModal), can be
// done from it.  A call posted again before it's made is made once.  The
// object isn't held; it calls CancelPosted before it goes.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CWindow::Post(O* pcObject, void(M::*fFunction)(void))
{
	SMemberCall	sCall;
	size		i;

	sCall.Init(pcObject, fFunction);
	for (i = 0; i < masPosted.NumElements(); i++)
	{
		if (masPosted.Get(i)->Equals(&sCall))
		{
			return;
		}
	}
	memcpy(masPosted.Add(), &sCall, sizeof(SMemberCall));
	RequestTick(0);
}


#endif // __WINDOW_H__

