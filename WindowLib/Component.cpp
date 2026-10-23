#include "StandardLib/ClassDefines.h"
#include "Window.h"
#include "Component.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Init(Ptr<CWindow> pWindow)
{
	PreInit();

	maChildren.Init();
	mcActualBounds.Init();
	msDesiredSize.Init(-1, -1);
	meHorizontalAlignment = CSH_Center;
	meVerticalAlignment = CSV_Center;
	mbCanGetFocus = false;
	mpParent = NULL;
	mpWindow = pWindow;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Class(void)
{
		U_Data(CInt32Rect, mcActualBounds);
		U_2Int32(msDesiredSize);
		U_Enum(meHorizontalAlignment);
		U_Enum(meVerticalAlignment);
		U_Bool(mbCanGetFocus);
		M_Pointer(mpParent);
		M_Embedded(maChildren);
	Nes(M_Pointer(mpWindow));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::Draw(void)
{
	return DrawChildren();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::DrawChildren(void)
{
	size						i;
	Ptr<CComponent>	pComponent;
	bool					bResult;
	size					uiSize;

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		bResult = pComponent->Draw();
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::AddComponent(Ptr<CComponent> pComponent)
{
	maChildren.Add(pComponent);
	pComponent->mpParent = this;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::RemoveComponent(Ptr<CComponent> pComponent)
{
	bool	bResult;

	if (mpWindow)
	{
		mpWindow->GetFocus()->Forget(pComponent);
	}

	bResult = maChildren.Remove(pComponent);
	if (bResult)
	{
		pComponent->mpParent = NULL;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::RemoveAllComponents(void)
{
	size				i;
	Ptr<CComponent>		pComponent;
	size				uiSize;

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		if (mpWindow)
		{
			mpWindow->GetFocus()->Forget(pComponent);
		}
		pComponent->mpParent = NULL;
	}
	return maChildren.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CComponent::NumComponents(void)
{
	return maChildren.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::GetContainerBounds(CInt32Rect* pcDest)
{
	//The only way the function can be called is if the component is not a container.
	if (mpParent)
	{
		return mpParent->GetContainerBounds(pcDest);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the component is pAncestor or is somewhere inside it.
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::IsWithin(Ptr<CComponent> pAncestor)
{
	Ptr<CComponent>	pComponent;

	if (pAncestor.IsNull())
	{
		return false;
	}

	pComponent = this;
	while (pComponent.IsNotNull())
	{
		if (pComponent == pAncestor)
		{
			return true;
		}
		pComponent = pComponent->GetParent();
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A component that clips its children, like a scroll pane, only lets them
// draw inside its container bounds.  Most components draw their children
// wherever they are laid out.
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::ClipsChildren(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The area, in window client space, this component can be drawn in: the
// overlap of the container bounds of every ancestor that clips its
// children.  Returns false if no ancestor clips, so the component can be
// drawn anywhere.  The area can be empty.
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::GetClip(CInt32Rect* pcClip)
{
	Ptr<CComponent>	pAncestor;
	CInt32Rect		cBounds;
	bool			bClipped;

	bClipped = false;
	pAncestor = mpParent;
	while (pAncestor.IsNotNull())
	{
		if (pAncestor->ClipsChildren() && pAncestor->GetContainerBounds(&cBounds))
		{
			if (bClipped)
			{
				pcClip->ClipTo(&cBounds);
			}
			else
			{
				pcClip->Init(&cBounds);
				bClipped = true;
			}
		}
		pAncestor = pAncestor->GetParent();
	}
	return bClipped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::IsPointIn(int x, int y)
{
	//x and y are in window client space.  The right and bottom edges belong to the next component over.
	return (x >= mcActualBounds.msTopLeft.x) && (x < mcActualBounds.msBottomRight.x) &&
		   (y >= mcActualBounds.msTopLeft.y) && (y < mcActualBounds.msBottomRight.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::HasFocus(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CComponent::FindComponentAt(int x, int y)
{
	size					i;
	Ptr<CComponent>	pcComponent;
	size					uiSize;
	CInt32Rect				cBounds;

	//x and y are in window client space.
	if (!IsPointIn(x, y))
	{
		return NULL;
	}

	//Children are only found where they can be drawn.  The right and bottom edges belong to the next component over.
	if (ClipsChildren() && GetContainerBounds(&cBounds))
	{
		if ((x < cBounds.msTopLeft.x) || (x >= cBounds.msBottomRight.x) ||
			(y < cBounds.msTopLeft.y) || (y >= cBounds.msBottomRight.y))
		{
			return this;
		}
	}

	//Children drawn later are drawn over earlier ones so they are searched first.
	uiSize = maChildren.Size();
	for (i = uiSize; i > 0; i--)
	{
		pcComponent = maChildren.Get(i - 1);
		pcComponent = pcComponent->FindComponentAt(x, y);
		if (pcComponent)
		{
			return pcComponent;
		}
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::ToChildSpace(Ptr<CComponent> pcChildComponent, int x, int y, int* px, int* py)
{
	*px = x - pcChildComponent->mcActualBounds.msTopLeft.x;
	*py = y - pcChildComponent->mcActualBounds.msTopLeft.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::FromChildSpace(Ptr<CComponent> pcChildComponent, int x, int y, int* px, int* py)
{
	*px = x + pcChildComponent->mcActualBounds.msTopLeft.x;
	*py = y + pcChildComponent->mcActualBounds.msTopLeft.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{

	SetPosition(sPosition);
	SetActualSize(sAreaSize);

	LayoutChildren(sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::LayoutChildren(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	size					i;
	Ptr<CComponent>	pComponent;
	size					uiSize;

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		pComponent->Layout(sPosition, sAreaSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CComponent::GetDepth(void)
{
	size					iDepth;
	Ptr<CComponent>	pcComponent;

	iDepth = 0;
	pcComponent = mpParent;
	while (pcComponent != NULL)
	{
		iDepth++;
		pcComponent = pcComponent->mpParent;
	}
	return iDepth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponent::IsFocussed(void)
{
	if (mpWindow->GetFocus()->GetFocussedComponent() == this)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponent::SetActualSize(int fWidth, int fHeight) { mcActualBounds.SetSize(fWidth, fHeight); }
void CComponent::SetActualSize(SInt32Vec2 sSize) { mcActualBounds.SetSize(sSize.x, sSize.y); }
void CComponent::SetPosition(int x, int y) { mcActualBounds.SetPos(x, y); }
void CComponent::SetPosition(SInt32Vec2 sPosition) { mcActualBounds.SetPos(sPosition.x, sPosition.y); }
SInt32Vec2 CComponent::GetPosition(void) { return mcActualBounds.msTopLeft; }
bool CComponent::CanGetFocus(void) { return mbCanGetFocus; }
void CComponent::SetCanGetFocus(bool bCanGetFocus) { mbCanGetFocus = bCanGetFocus; }
bool CComponent::WantsEnter(void) { return false; }  //Enter goes to the window's default button unless the focussed component wants it.
void CComponent::PopupClosed(Ptr<CComponent> pContent) {}  //A popup this component opened, see CWindow::OpenPopup, has closed.
void CComponent::PopupItemClicked(Ptr<CComponent> pContent, Ptr<CComponent> pItem) {}  //An item in a popup this component opened, like a menu's, was clicked.  The popup has closed.
Ptr<CComponent> CComponent::GetParent(void) { return mpParent; }
void CComponent::SetDesiredSize(int fWidth, int fHeight) { msDesiredSize.Init(fWidth, fHeight); }
SInt32Vec2 CComponent::GetActualSize(void) { SInt32Vec2 sSize; sSize.Init(mcActualBounds.GetWidth(), mcActualBounds.GetHeight()); return sSize; }
SInt32Vec2 CComponent::GetDesiredSize(void) { return msDesiredSize; }
void CComponent::SetAlignment(EContainerStyleHorizontal eHorizontal, EContainerStyleVertical eVertical) { meHorizontalAlignment = eHorizontal; meVerticalAlignment = eVertical; }
EContainerStyleHorizontal CComponent::GetHorizontalAlignment(void) { return meHorizontalAlignment; }
EContainerStyleVertical CComponent::GetVerticalAlignment(void) { return meVerticalAlignment; }

