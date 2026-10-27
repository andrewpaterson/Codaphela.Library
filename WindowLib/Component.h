#ifndef __COMPONENT_H__
#define __COMPONENT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "SupportLib/ColourFormat.h"
#include "Focus.h"
#include "FocusListener.h"
#include "SupportLib/Int32Rect.h"
#include "ContainerStyle.h"
#include "InputAction.h"


class CWindow;
//Input reaches a component through the input actions added to it (see AddInputAction), given by the window's focus.
class CComponent : public CObject, public CFocusListener
{
CONSTRUCTABLE(CComponent);
DESTRUCTABLE(CComponent);
protected:
	CInt32Rect			mcActualBounds;
	SInt32Vec2			msDesiredSize;
	EContainerStyleHorizontal	meHorizontalAlignment;  //Where a fixed size component sits in spare space.  Stretch makes it fill the space.
	EContainerStyleVertical		meVerticalAlignment;
	bool				mbCanGetFocus;
	Ptr<CComponent>		mpParent;
	CArray<CComponent>	maChildren;
	Ptr<CWindow>		mpWindow;
	CArray<CInputAction>	maInputActions;

public:
			void				Init(Ptr<CWindow> pWindow);
			void				Class(void) override;
			void 				Free(void) override;

			bool				Save(CObjectWriter* pcFile) override;
			bool				Load(CObjectReader* pcFile) override;

	virtual	bool				Draw(void);
			bool				DrawChildren(void);
	virtual bool				GetContainerBounds(CInt32Rect* pcDest);
	virtual	bool				ClipsChildren(void);
			bool				GetClip(CInt32Rect* pcClip);

	virtual	bool				AddComponent(Ptr<CComponent> pComponent);
	virtual	bool				RemoveComponent(Ptr<CComponent> pComponent);
	virtual	bool				RemoveAllComponents(void);
			size				NumComponents(void);

			bool				IsPointIn(int x, int y);
			bool				HasFocus(void);
	virtual	Ptr<CComponent>		FindComponentAt(int x, int y);
			void				ToChildSpace(Ptr<CComponent> pcChildComponent, int x, int y, int* px, int* py);
			void				FromChildSpace(Ptr<CComponent> pcChildComponent, int x, int y, int* px, int* py);

	virtual	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize);
			void				LayoutChildren(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize);
			size				GetDepth(void);

			bool				IsFocussed(void);
			bool				CanGetFocus(void);
			void				SetCanGetFocus(bool bCanGetFocus);
	virtual	bool				WantsEnter(void);
	virtual	void				PopupClosed(Ptr<CComponent> pContent);
	virtual	void				PopupItemClicked(Ptr<CComponent> pContent, Ptr<CComponent> pItem);
			Ptr<CComponent>		GetParent(void);
			Ptr<CWindow>		GetWindow(void);
			bool				IsWithin(Ptr<CComponent> pAncestor);

			void				SetActualSize(int fWidth, int fHeight);
			void				SetActualSize(SInt32Vec2 sSize);
			void				SetPosition(int x, int y);
			void				SetPosition(SInt32Vec2 sPosition);
			SInt32Vec2			GetPosition(void);
	virtual	SInt32Vec2			GetDesiredSize(void);
	virtual	void				ScrollToShow(CInt32Rect* pcRect);
	virtual	bool				GetScrollView(CInt32Rect* pcDest);
			void				SetDesiredSize(int fWidth, int fHeight);
			SInt32Vec2			GetActualSize(void);

			void				SetAlignment(EContainerStyleHorizontal eHorizontal, EContainerStyleVertical eVertical);
			EContainerStyleHorizontal	GetHorizontalAlignment(void);
			EContainerStyleVertical		GetVerticalAlignment(void);

	template<class M>
			Ptr<CInputAction>	AddInputAction(EInputActionFilter eFilter, const char* szName, M* pcObject, void(M::*fFunction)(SComponentInputEvent*));
	template<class M>
			Ptr<CInputAction>	AddInputAction(EComponentInputCommand eCommand, M* pcObject, void(M::*fFunction)(SComponentInputEvent*));
	template<class M>
			void				AddInputActions(const EComponentInputCommand* aeCommands, size uiNumCommands, M* pcObject, void(M::*fFunction)(SComponentInputEvent*));
			bool				RemoveInputAction(Ptr<CInputAction> pAction);
			size				NumInputActions(void);
			bool				CallInputActions(SComponentInputEvent* psEvent);

protected:
			Ptr<CInputAction>	MallocInputAction(void);
			void				AddInputAction(Ptr<CInputAction> pAction);
};


//////////////////////////////////////////////////////////////////////////
//
// szName is the key or chord for IAF_KeyDown, IAF_KeyUp and IAF_Chord,
// otherwise NULL.  pcObject is usually this component.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<CInputAction> CComponent::AddInputAction(EInputActionFilter eFilter, const char* szName, M* pcObject, void(M::*fFunction)(SComponentInputEvent*))
{
	Ptr<CInputAction>	pAction;

	pAction = MallocInputAction();
	pAction->Init(eFilter, szName, pcObject, fFunction);
	AddInputAction(pAction);
	return pAction;
}


//////////////////////////////////////////////////////////////////////////
//
// An IAF_Command action.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<CInputAction> CComponent::AddInputAction(EComponentInputCommand eCommand, M* pcObject, void(M::*fFunction)(SComponentInputEvent*))
{
	Ptr<CInputAction>	pAction;

	pAction = MallocInputAction();
	pAction->Init(eCommand, pcObject, fFunction);
	AddInputAction(pAction);
	return pAction;
}


//////////////////////////////////////////////////////////////////////////
//
// An IAF_Command action for each command, all calling fFunction, which
// can switch on the event's command.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CComponent::AddInputActions(const EComponentInputCommand* aeCommands, size uiNumCommands, M* pcObject, void(M::*fFunction)(SComponentInputEvent*))
{
	size	i;

	for (i = 0; i < uiNumCommands; i++)
	{
		AddInputAction(aeCommands[i], pcObject, fFunction);
	}
}


#endif // __COMPONENT_H__

