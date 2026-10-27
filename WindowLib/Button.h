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
#include "ImageBorderContainer.h"
#include "WindowTheme.h"
#include "Label.h"
#include "IconComponent.h"
#include "ButtonImages.h"
#include "ButtonListener.h"


//A button is a face inside a border of image pieces, see CImageBorderContainer.  The face is filled with the face
//colour under a centred caption.  The face needs the caption's size plus the padding and
//stretches with the button.
//
//The button is hit as a whole so it, not its parts, has the pointer over it and takes focus.  A click is a press and
//release with the pointer over the button both times, or space or enter while it has focus.
//
//A window's default button is also clicked by enter when the focussed component doesn't want it.  It is drawn with
//its default images, if it has them, while it is the default.
class CButton : public CImageBorderContainer
{
CONSTRUCTABLE(CButton);
DESTRUCTABLE(CButton);
protected:
	Ptr<CButtonImages>			mpNormalImages;
	Ptr<CButtonImages>			mpDefaultImages;  //Optional.
	Ptr<CFillContainer>			mpPadded;  //The face less its padding.
	Ptr<CGridContainer>			mpContent;  //The icon and the caption, side by side, on the face.
	Ptr<CIconComponent>			mpIcon;  //Shows nothing without a cel.
	Ptr<CLabel>					mpCaption;
	SInt32Vec2					msPadding;
	int32						miIconGap;  //Between the icon and the caption.
	CListenerCall				mcListeners;
	EButtonState				meState;
	bool						mbMouseOver;
	bool						mbFocussed;
	bool						mbPressed;  //Pressed on and not yet released.
	bool						mbDefault;
	bool						mbShowsPushed;  //False keeps the mouse over look while pressed.
	bool						mbHighlighted;  //Has the mouse over look wherever the pointer is.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szCaption);
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CImageCel> pIcon, const char* szCaption);
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, EButtonLook eLook, Ptr<CImageCel> pIcon, const char* szCaption);
	void				Class(void) override;
	void 				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				SetCaption(const char* szCaption);
	char*				GetCaption(void);
	void				SetIcon(Ptr<CImageCel> pIcon);
	Ptr<CImageCel>		GetIcon(void);
	void				SetIconGap(int32 iIconGap);
	Ptr<CIconComponent>	GetIconComponent(void);
	Ptr<CLabel>			GetCaptionLabel(void);
	void				SetPadding(int32 iHorizontal, int32 iVertical);

	bool				AddListener(CButtonListener* pcListener);
	bool				RemoveListener(CButtonListener* pcListener);
	void				Click(void);

	EButtonState		GetState(void);
	bool				IsPressed(void);
	void				SetDefault(bool bDefault);
	bool				IsDefault(void);
	void				SetShowsPushed(bool bShowsPushed);
	bool				IsShowsPushed(void);
	void				SetHighlighted(bool bHighlighted);
	bool				IsHighlighted(void);

	bool				WantsEnter(void) override;

	void				FocusGained(void) override;
	void				FocusLost(void) override;
	void				MovedInto(void) override;
	void				MovedOutof(void) override;
	void				CommandAction(SComponentInputEvent* psEvent);
	virtual void		ActivateComponent(int x, int y);
	virtual void		ExecuteComponent(int x, int y);
	virtual void		Space(bool bInsert);
	virtual void		Enter(void);

protected:
	void				Init(Ptr<CWindow> pWindow, Ptr<CButtonImages> pImages, Ptr<CButtonImages> pDefaultImages, Ptr<CImageCel> pIcon, Ptr<CFont> pFont, const char* szCaption);
	void				UpdateState(void);
	void				UpdateLook(void);
	void				UpdateFaceSize(void);
	void				UpdateContent(void);
	bool				HasCaption(void);
};


#endif // __BUTTON_H__

