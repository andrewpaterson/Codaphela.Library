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
#include "SoundMixer.h"
#include "Clipboard.h"


#define WINDOW_DEFAULT_FRAMES_PER_SECOND	30


class CWindowTick;
class CNativeSoundFactory;
class CComponentInput;
class CButton;
class CWindow : public CComponent
{
CONSTRUCTABLE(CWindow); 
DESTRUCTABLE(CWindow);
protected:
	CChars					mszWindowTitle;

	CNativeWindow*			mpcNativeWindow;
	Ptr<CCanvas>			mpCanvas;
	Ptr<CWindowTick>		mpWindowTick;
	bool					mbTicking;
	CFocus					mFocus;
	CComponentInput*		mpcComponentInput;  //Optional.  Updated every tick when set.
	Ptr<CButton>			mpDefaultButton;  //Clicked by enter unless the focussed component wants it.
	Ptr<CButton>			mpCancelButton;  //Clicked by escape when no popup is open.
	bool					mbLayoutAgain;  //A component's required size changed while it was being laid out.
	CArray<CPopup>			maPopups;  //Open popups, the last opened last.  Each opened by one before it is closed with it.
	Ptr<CSoundMixer>		mpSoundMixer;  //Only if the window was given a sound factory.  Mixed every tick.
	Ptr<CClipboard>			mpClipboard;  //The system's clipboard.
	bool					mbContinuous;  //Ticked and painted every pass, as a game is, rather than waiting for events.
	bool					mbRedraw;  //Something has changed since the window was last painted.
	bool					mbLayoutDirty;  //Something the layout depends on has changed since the window was last laid out.
	bool					mbCheckLayoutCache;  //Lay out every paint anyway and log components that moved, see CheckLayoutCache.
	uint32					muiFrameMilliseconds;  //The least time between paints while waiting for events, 0 for none.
	int64					miLastPaintTime;  //GetDebugTickCount when last painted.
	int64					miTickDelay;  //Milliseconds after this tick that another was asked for, -1 if none was.

public:
	void					Init(const char* szTitle, CNativeWindowFactory* pcFactory, Ptr<CWindowTick>	pTick, Ptr<CCanvasDraw> pDraw, EColourFormat eFormat, EColourOrder eOrder, ERGBColourBits eColourBits, ERGBAlphaBits eAlphaBits);
	void					Init(const char* szTitle, CNativeWindowFactory* pcFactory, CNativeSoundFactory* pcSoundFactory, Ptr<CWindowTick>	pTick, Ptr<CCanvasDraw> pDraw, EColourFormat eFormat, EColourOrder eOrder, ERGBColourBits eColourBits, ERGBAlphaBits eAlphaBits);
	void					Init(const char* szTitle, CNativeWindowFactory* pcFactory, Ptr<CWindowTick>	pTick, Ptr<CCanvasDraw> pDraw, CColourFormatHelper* pcFormat);
	void					Init(const char* szTitle, CNativeWindowFactory* pcFactory, CNativeSoundFactory* pcSoundFactory, Ptr<CWindowTick>	pTick, Ptr<CCanvasDraw> pDraw, CColourFormatHelper* pcFormat);
	void					Class(void) override;
	void 					Free(void) override;
	
	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;
	
	bool					Show(void);
	bool					ShowModal(Ptr<CWindow> pOwner);
	void					SetWindowSize(int32 iWidth, int32 iHeight);
	CNativeWindow*			GetNativeWindow(void);
	void					Paint(void);
	void					Invalidate(void) override;
	void					InvalidateLayout(void) override;
	void					RequestTick(uint32 uiMilliseconds);
	void					SetContinuous(bool bContinuous);
	bool					IsContinuous(void);
	void					SetMaxFramesPerSecond(uint32 uiFramesPerSecond);
	void					SetCheckLayoutCache(bool bCheckLayoutCache);
	void					Tick(int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds);
	void					Stop(void);
	void					Layout(void);
	void					LayoutAgain(void);

	//Input in window client space.  Native windows, and later CComponentInput, call these.
	void					PointerMoved(int x, int y);
	void					PointerLeft(void);
	void					Input(SWindowInputEvent* psWindowEvent, CComponentInput* pcComponentInput);
	void					InputCommand(EComponentInputCommand eCommand, bool bInsert);
	void					InputPrintable(char c, bool bInsert);
	void					Activate(void);
	void					Deactivate(void);

	void					SetComponentInput(CComponentInput* pcComponentInput);
	CComponentInput*		GetComponentInput(void);

	const char*				GetWindowTitle(void);
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
	int64					GetWaitMilliseconds(void);
	bool					IsFrameDue(void);
};


#endif // __WINDOW_H__

