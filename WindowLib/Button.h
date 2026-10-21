/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __BUTTON_H__
#define __BUTTON_H__
#include "StandardLib/ListenerCall.h"
#include "SupportLib/Font.h"
#include "GridContainer.h"
#include "FillContainer.h"
#include "ImageCanvasDraw.h"
#include "ClearCanvasDraw.h"
#include "Label.h"
#include "ButtonImages.h"
#include "ButtonListener.h"


//A button is a grid of its parts: eight border pieces around a face, laid out by CBorderLayoutHelper.  The face is a
//canvas filled with the face colour under a centred caption.  The face needs the caption's size plus the padding and
//stretches with the button.
//
//The button is hit as a whole so it, not its parts, has the pointer over it and takes focus.  A click is a press and
//release with the pointer over the button both times, or space or enter while it has focus.
//
//A window's default button is also clicked by enter when the focussed component doesn't want it.  It is drawn with
//its default images, if it has them, while it is the default.
class CButton : public CGridContainer
{
CONSTRUCTABLE(CButton);
DESTRUCTABLE(CButton);
protected:
	Ptr<CButtonImages>			mpImages;
	Ptr<CButtonImages>			mpDefaultImages;  //Optional.
	Ptr<CClearCanvasDraw>		mpFaceDraw;
	CArray<CImageCanvasDraw>	maBorderDraws;  //One for each EButtonBorderPiece.
	Ptr<CFillContainer>			mpFace;
	Ptr<CLabel>					mpCaption;
	SInt32Vec2					msPadding;
	CListenerCall				mcListeners;
	EButtonState				meState;
	bool						mbMouseOver;
	bool						mbFocussed;
	bool						mbPressed;  //Pressed on and not yet released.
	bool						mbDefault;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CButtonImages> pImages, Ptr<CFont> pFont, const char* szCaption);
	void				Init(Ptr<CWindow> pWindow, Ptr<CButtonImages> pImages, Ptr<CButtonImages> pDefaultImages, Ptr<CFont> pFont, const char* szCaption);
	void				Class(void) override;
	void 				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				SetCaption(const char* szCaption);
	char*				GetCaption(void);
	void				SetPadding(int32 iHorizontal, int32 iVertical);
	SInt32Vec2			GetDesiredSize(void) override;

	bool				AddListener(CButtonListener* pcListener);
	bool				RemoveListener(CButtonListener* pcListener);
	void				Click(void);

	EButtonState		GetState(void);
	bool				IsPressed(void);
	void				SetDefault(bool bDefault);
	bool				IsDefault(void);
	Ptr<CButtonImages>	GetImages(void);

	Ptr<CComponent>		FindComponentAt(int x, int y) override;
	bool				WantsEnter(void) override;

	void				FocusGained(void) override;
	void				FocusLost(void) override;
	void				MovedInto(void) override;
	void				MovedOutof(void) override;
	void				ActivateComponent(int x, int y) override;
	void				ExecuteComponent(int x, int y) override;
	void				Space(bool bInsert) override;
	void				Enter(void) override;

protected:
	void				UpdateState(void);
	void				UpdateLook(void);
	void				UpdateFaceSize(void);
};


#endif // __BUTTON_H__

