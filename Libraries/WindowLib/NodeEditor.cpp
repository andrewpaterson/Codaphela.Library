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
#include <stdlib.h>
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SupportLib/VectorShapes.h"
#include "ComponentInput.h"
#include "Focus.h"
#include "Window.h"
#include "NodeEditor.h"


bool CNodeEditorListener::CanLink(CNodeBox* pcFrom, size uiOutput, CNodeBox* pcTo, size uiInput) { return true; }
void CNodeEditorListener::Linked(CNodeBox* pcFrom, size uiOutput, CNodeBox* pcTo, size uiInput) {}
void CNodeEditorListener::Unlinked(CNodeBox* pcFrom, size uiOutput, CNodeBox* pcTo, size uiInput) {}
void CNodeEditorListener::NodesMoved(void) {}
void CNodeEditorListener::DeleteSelected(void) {}
void CNodeEditorListener::DuplicateSelected(void) {}
void CNodeEditorListener::AddNode(int32 x, int32 y) {}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditorDraw::Init(CNodeEditor* pcEditor)
{
	PreInit();

	CCelCanvasDraw::Init();
	mpcEditor = pcEditor;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditorDraw::Class(void)
{
	CCelCanvasDraw::Class();

	U_Pointer(mpcEditor);
}


//////////////////////////////////////////////////////////////////////////
//
// The sockets are where the nodes were last laid out, in the window; the
// canvas is the editor's.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditorDraw::Paint(CImageDraw* pcDraw, SInt32Vec2 sSize)
{
	Ptr<CWindowTheme>	pTheme;
	SNodeLink*			psLink;
	SInt32Vec2			sEditor;
	SInt32Vec2			sFrom;
	SInt32Vec2			sTo;
	CInt32Rect			cBox;
	ARGB32				sColour;
	size				i;

	pTheme = mpcEditor->GetTheme();
	pcDraw->SetColour(pTheme->GetColour(TC_Editor));
	pcDraw->DrawRectangle(0, 0, sSize.x, sSize.y);

	sEditor = mpcEditor->GetPosition();
	for (i = 0; i < mpcEditor->NumLinks(); i++)
	{
		psLink = mpcEditor->GetLink(i);
		sFrom = psLink->pcFrom->GetSocketPosition(false, psLink->uiOutput);
		sTo = psLink->pcTo->GetSocketPosition(true, psLink->uiInput);
		PaintLink(pcDraw, SInt32Vec2(sFrom.x - sEditor.x, sFrom.y - sEditor.y), SInt32Vec2(sTo.x - sEditor.x, sTo.y - sEditor.y), psLink->pcFrom->GetSocket(false, psLink->uiOutput)->sColour);
	}

	if (mpcEditor->GetDraggedLink(&sFrom, &sTo, &sColour))
	{
		PaintLink(pcDraw, sFrom, sTo, sColour);
	}
	if (mpcEditor->GetDraggedBox(&cBox))
	{
		pcDraw->SetColour(pTheme->GetColour(TC_Accent));
		pcDraw->DrawBox(&cBox, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// From an output to an input, in the canvas: the curve leaves the output
// to the right and comes to the input from the left, its controls half
// the way across or NODE_EDITOR_LINK_REACH out, whichever is further.
// It's stroked as short straight pieces with round ends.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditorDraw::PaintLink(CImageDraw* pcDraw, SInt32Vec2 sFrom, SInt32Vec2 sTo, ARGB32 sColour)
{
	SFloat64Vec2	sStart;
	SFloat64Vec2	sEnd;
	SFloat64Vec2	sControl1;
	SFloat64Vec2	sControl2;
	SFloat64Vec2	sLast;
	SFloat64Vec2	sPoint;
	float64			fReach;
	int32			i;

	sStart.Init(sFrom.x, sFrom.y);
	sEnd.Init(sTo.x, sTo.y);
	fReach = abs(sTo.x - sFrom.x) * 0.5;
	if (fReach < NODE_EDITOR_LINK_REACH)
	{
		fReach = NODE_EDITOR_LINK_REACH;
	}
	VectorLevelCubic(&sStart, &sEnd, fReach, &sControl1, &sControl2);

	pcDraw->SetColour(sColour);
	sLast = sStart;
	for (i = 1; i <= NODE_EDITOR_LINK_STEPS; i++)
	{
		VectorCubicPoint(&sPoint, &sStart, &sControl1, &sControl2, &sEnd, (float64)i / NODE_EDITOR_LINK_STEPS);
		pcDraw->DrawStroke(sLast.x, sLast.y, sPoint.x, sPoint.y, NODE_EDITOR_LINK_WIDTH);
		sLast = sPoint;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme)
{
	PreInit();

	CContainer::Init(pWindow);
	mpTheme = pTheme;
	mapNodes.Init();
	masLinks.Init();
	mpcListener = NULL;
	msPan.Init(0, 0);
	meDrag = NED_None;
	msPressed.Init(0, 0);
	msPointer.Init(0, 0);
	mbDragged = false;
	mpcLinkNode = NULL;
	mbLinkInput = false;
	muiLinkSocket = 0;
	mbPanning = false;

	mpDraw = OMalloc<CNodeEditorDraw>(this);
	mpBack = OMalloc<CCanvas>(pWindow, mpDraw);
	mpBack->SetAlignment(CSH_Stretch, CSV_Stretch);
	CContainer::AddComponent(mpBack);

	SetAlignment(CSH_Stretch, CSV_Stretch);
	SetCanGetFocus(true);
	AddInputAction(CIC_ActivateComponent, this, &CNodeEditor::ActivateComponent);
	AddInputAction(CIC_ExecuteComponent, this, &CNodeEditor::ExecuteComponent);
	AddInputAction(CIC_ContDragSelect, this, &CNodeEditor::ContDragSelect);
	AddInputAction(CIC_StopDragSelect, this, &CNodeEditor::StopDragSelect);
	AddInputAction(IAF_Movement, NULL, this, &CNodeEditor::MovementAction);
	AddInputAction(IAF_KeyDown, "Button 3", this, &CNodeEditor::PanPressAction);
	AddInputAction(IAF_KeyUp, "Button 3", this, &CNodeEditor::PanReleaseAction);
	AddInputAction(IAF_KeyDown, "Home", this, &CNodeEditor::HomeAction);
	AddInputAction(IAF_KeyDown, "Delete", this, &CNodeEditor::DeleteAction);
	AddInputAction(IAF_KeyDown, "X", this, &CNodeEditor::DeleteAction);
	AddInputAction(IAF_KeyDown, "D", this, &CNodeEditor::DuplicateAction);
	AddInputAction(IAF_KeyDown, "A", this, &CNodeEditor::AddAction);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::Class(void)
{
	CContainer::Class();

	M_Pointer(mpTheme);
	M_Pointer(mpDraw);
	M_Pointer(mpBack);
	M_Embedded(mapNodes);
	U_Unknown(CArrayNodeLink, masLinks);
	U_Pointer(mpcListener);
	U_2Int32(msPan);
	U_Enum(meDrag);
	U_2Int32(msPressed);
	U_2Int32(msPointer);
	U_Bool(mbDragged);
	U_Pointer(mpcLinkNode);
	U_Bool(mbLinkInput);
	U_Size(muiLinkSocket);
	U_Bool(mbPanning);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::Free(void)
{
	masLinks.Kill();
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// The back fills the editor.  Each node is where it says it is from the
// editor's origin, which the pan moves, and the size it needs.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	Ptr<CNodeBox>	pNode;
	SInt32Vec2		sNode;
	size			i;

	SetContainerBounds(sPosition, sAreaSize);
	LayoutComponentInArea(mpBack, sPosition, sAreaSize);
	for (i = 0; i < mapNodes.NumElements(); i++)
	{
		pNode = mapNodes.Get(i);
		sNode = pNode->GetNodePosition();
		LayoutComponentInArea(pNode, SInt32Vec2(sPosition.x + msPan.x + sNode.x, sPosition.y + msPan.y + sNode.y), GetMeasuredSize(pNode));
	}
	SetActualBounds(sPosition, sAreaSize);

	//The links are drawn to where the sockets now are.
	mpDraw->Repaint();
	mpBack->Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
// It needs nothing: it takes the room it's given, and its nodes are seen
// through it wherever they are.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::SetRequiredSize(void)
{
	msRequiredSize.Init(0, 0);
}


void CNodeEditor::SetListener(CNodeEditorListener* pcListener) { mpcListener = pcListener; }
Ptr<CWindowTheme> CNodeEditor::GetTheme(void) { return mpTheme; }
size CNodeEditor::NumNodes(void) { return mapNodes.NumElements(); }
Ptr<CNodeBox> CNodeEditor::GetNode(size uiIndex) { return mapNodes.Get(uiIndex); }
size CNodeEditor::NumLinks(void) { return masLinks.NumElements(); }
SNodeLink* CNodeEditor::GetLink(size uiIndex) { return masLinks.SafeGet(uiIndex); }
SInt32Vec2 CNodeEditor::GetPan(void) { return msPan; }
ENodeEditorDrag CNodeEditor::GetDrag(void) { return meDrag; }


//////////////////////////////////////////////////////////////////////////
//
// The links are painted again and the nodes laid out again.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::Changed(void)
{
	mpDraw->Repaint();
	mpBack->Invalidate();
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// Nodes added later are drawn over those added earlier.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::AddNode(Ptr<CNodeBox> pNode)
{
	mapNodes.Add(pNode);
	CContainer::AddComponent(pNode);
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
// With its links.  False if it isn't one of the editor's.
//
//////////////////////////////////////////////////////////////////////////
bool CNodeEditor::RemoveNode(Ptr<CNodeBox> pNode)
{
	if (!mapNodes.Contains(pNode))
	{
		return false;
	}
	RemoveLinksOf(&pNode);
	if (mpcLinkNode == &pNode)
	{
		mpcLinkNode = NULL;
		meDrag = NED_None;
	}
	CContainer::RemoveComponent(pNode);
	mapNodes.Remove(pNode);
	Changed();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::RemoveAllNodes(void)
{
	while (mapNodes.NumElements() > 0)
	{
		RemoveNode(mapNodes.Get(mapNodes.NumElements() - 1));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The inputs the node fed have nothing linked to them any more.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::RemoveLinksOf(CNodeBox* pcNode)
{
	SNodeLink*	psLink;
	size		i;

	i = masLinks.NumElements();
	while (i > 0)
	{
		i--;
		psLink = masLinks.Get(i);
		if ((psLink->pcFrom == pcNode) || (psLink->pcTo == pcNode))
		{
			psLink->pcTo->SetInputLinked(psLink->uiInput, false);
			masLinks.RemoveAt(i);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The topmost node the window's point x, y is in: NULL for none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CNodeBox> CNodeEditor::FindNodeAt(int32 x, int32 y)
{
	Ptr<CNodeBox>	pNode;
	CInt32Rect		cBounds;
	size			i;

	i = mapNodes.NumElements();
	while (i > 0)
	{
		i--;
		pNode = mapNodes.Get(i);
		pNode->GetActualBounds(&cBounds);
		if (cBounds.ContainsPoint(x, y))
		{
			return pNode;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A link from the output to the input, in place of any the input had.
// False if either isn't there, or they're the same node's.  The listener
// isn't told.
//
//////////////////////////////////////////////////////////////////////////
bool CNodeEditor::Link(CNodeBox* pcFrom, size uiOutput, CNodeBox* pcTo, size uiInput)
{
	SNodeLink*	psLink;

	if ((pcFrom == pcTo) || (pcFrom->GetSocket(false, uiOutput) == NULL) || (pcTo->GetSocket(true, uiInput) == NULL))
	{
		return false;
	}
	Unlink(pcTo, uiInput);
	psLink = masLinks.Add();
	psLink->pcFrom = pcFrom;
	psLink->uiOutput = uiOutput;
	psLink->pcTo = pcTo;
	psLink->uiInput = uiInput;
	pcTo->SetInputLinked(uiInput, true);
	Changed();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the input's link.  False if it had none.
//
//////////////////////////////////////////////////////////////////////////
bool CNodeEditor::Unlink(CNodeBox* pcTo, size uiInput)
{
	SNodeLink*	psLink;

	psLink = FindLinkTo(pcTo, uiInput);
	if (psLink == NULL)
	{
		return false;
	}
	masLinks.RemoveAt(masLinks.GetIndex(psLink));
	pcTo->SetInputLinked(uiInput, false);
	Changed();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SNodeLink* CNodeEditor::FindLinkTo(CNodeBox* pcTo, size uiInput)
{
	SNodeLink*	psLink;
	size		i;

	for (i = 0; i < masLinks.NumElements(); i++)
	{
		psLink = masLinks.Get(i);
		if ((psLink->pcTo == pcTo) && (psLink->uiInput == uiInput))
		{
			return psLink;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// pNode is selected and, unless bAdd, nothing else is.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::Select(Ptr<CNodeBox> pNode, bool bAdd)
{
	if (!bAdd)
	{
		SelectNone();
	}
	pNode->SetSelected(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::SelectNone(void)
{
	size	i;

	for (i = 0; i < mapNodes.NumElements(); i++)
	{
		mapNodes.Get(i)->SetSelected(false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CNodeEditor::NumSelected(void)
{
	size	uiCount;
	size	i;

	uiCount = 0;
	for (i = 0; i < mapNodes.NumElements(); i++)
	{
		if (mapNodes.Get(i)->IsSelected())
		{
			uiCount++;
		}
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::SetPan(int32 x, int32 y)
{
	msPan.Init(x, y);
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
// Pans so the middle of all the nodes is the middle of the editor.  The
// nodes' sizes are what they need, so it's right before they've been
// laid out.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::FrameAll(void)
{
	Ptr<CNodeBox>	pNode;
	SInt32Vec2		sNode;
	SInt32Vec2		sSize;
	SInt32Vec2		sMinimum;
	SInt32Vec2		sMaximum;
	SInt32Vec2		sEditor;
	size			i;

	if (mapNodes.NumElements() == 0)
	{
		SetPan(0, 0);
		return;
	}
	for (i = 0; i < mapNodes.NumElements(); i++)
	{
		pNode = mapNodes.Get(i);
		sNode = pNode->GetNodePosition();
		sSize = GetMeasuredSize(pNode);
		if (i == 0)
		{
			sMinimum = sNode;
			sMaximum.Init(sNode.x + sSize.x, sNode.y + sSize.y);
		}
		sMinimum.Init((sNode.x < sMinimum.x) ? sNode.x : sMinimum.x, (sNode.y < sMinimum.y) ? sNode.y : sMinimum.y);
		sMaximum.Init((sNode.x + sSize.x > sMaximum.x) ? sNode.x + sSize.x : sMaximum.x, (sNode.y + sSize.y > sMaximum.y) ? sNode.y + sSize.y : sMaximum.y);
	}
	sEditor = GetActualSize();
	SetPan((sEditor.x - (sMinimum.x + sMaximum.x)) / 2, (sEditor.y - (sMinimum.y + sMaximum.y)) / 2);
}


//////////////////////////////////////////////////////////////////////////
//
// The link being dragged, in the editor: from an output to the pointer,
// or from the pointer to an input.  False if none is.
//
//////////////////////////////////////////////////////////////////////////
bool CNodeEditor::GetDraggedLink(SInt32Vec2* psFrom, SInt32Vec2* psTo, ARGB32* psColour)
{
	SInt32Vec2	sEditor;
	SInt32Vec2	sSocket;

	if ((meDrag != NED_Link) || (mpcLinkNode == NULL))
	{
		return false;
	}
	sEditor = GetPosition();
	sSocket = mpcLinkNode->GetSocketPosition(mbLinkInput, muiLinkSocket);
	sSocket.Init(sSocket.x - sEditor.x, sSocket.y - sEditor.y);
	*psFrom = mbLinkInput ? msPointer : sSocket;
	*psTo = mbLinkInput ? sSocket : msPointer;
	*psColour = mpcLinkNode->GetSocket(mbLinkInput, muiLinkSocket)->sColour;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The box being selected with, in the editor: from where the pointer was
// pressed to where it is.  False if none is.
//
//////////////////////////////////////////////////////////////////////////
bool CNodeEditor::GetDraggedBox(CInt32Rect* pcDest)
{
	if ((meDrag != NED_Box) || !mbDragged)
	{
		return false;
	}
	pcDest->Init((msPressed.x < msPointer.x) ? msPressed.x : msPointer.x, (msPressed.y < msPointer.y) ? msPressed.y : msPointer.y, (msPressed.x > msPointer.x) ? msPressed.x : msPointer.x, (msPressed.y > msPointer.y) ? msPressed.y : msPointer.y);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNodeEditor::IsShift(void)
{
	CComponentInput*	pcInput;

	pcInput = mpWindow->GetComponentInput();
	return (pcInput != NULL) && pcInput->IsShift();
}


//////////////////////////////////////////////////////////////////////////
//
// The left button's gone down at x, y in the editor: on a socket, a node
// or the background (see the class).  An input's link is taken hold of
// by removing it and dragging a new one from its output.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::ActivateComponent(int x, int y)
{
	Ptr<CNodeBox>	pNode;
	SNodeLink*		psLink;
	SNodeLink		sLink;
	SInt32Vec2		sEditor;
	bool			bInput;
	size			uiSocket;
	size			i;

	sEditor = GetPosition();
	msPressed.Init(x, y);
	msPointer = msPressed;
	mbDragged = false;
	meDrag = NED_Box;

	//A socket's dot hangs over its node's edge, so the sockets are looked for before the nodes.
	i = mapNodes.NumElements();
	while ((i > 0) && (meDrag == NED_Box))
	{
		i--;
		pNode = mapNodes.Get(i);
		if (pNode->FindSocketAt(sEditor.x + x, sEditor.y + y, &bInput, &uiSocket))
		{
			meDrag = NED_Link;
			mpcLinkNode = &pNode;
			mbLinkInput = bInput;
			muiLinkSocket = uiSocket;
			psLink = bInput ? FindLinkTo(&pNode, uiSocket) : NULL;
			if (psLink != NULL)
			{
				sLink = *psLink;
				Unlink(sLink.pcTo, sLink.uiInput);
				if (mpcListener != NULL)
				{
					mpcListener->Unlinked(sLink.pcFrom, sLink.uiOutput, sLink.pcTo, sLink.uiInput);
				}
				mpcLinkNode = sLink.pcFrom;
				mbLinkInput = false;
				muiLinkSocket = sLink.uiOutput;
			}
		}
	}

	if (meDrag == NED_Box)
	{
		pNode = FindNodeAt(sEditor.x + x, sEditor.y + y);
		if (pNode.IsNotNull())
		{
			meDrag = NED_Nodes;
			if (IsShift())
			{
				pNode->SetSelected(!pNode->IsSelected());
			}
			else if (!pNode->IsSelected())
			{
				Select(pNode, false);
			}
		}
		else if (!IsShift())
		{
			SelectNone();
		}
	}

	mpWindow->GetFocus()->StartDrag(FD_Select, x, y);
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
// The left button's come up over the editor.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::ExecuteComponent(int x, int y)
{
	if (meDrag != NED_None)
	{
		mpWindow->GetFocus()->StopDrag(FD_Select);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer's moved to x, y in the editor with the left button down:
// the selected nodes go with it, or the link's loose end or the box's
// corner does.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::ContDragSelect(int x, int y)
{
	Ptr<CNodeBox>	pNode;
	SInt32Vec2		sNode;
	size			i;

	if ((meDrag == NED_None) || ((x == msPointer.x) && (y == msPointer.y)))
	{
		return;
	}
	if (meDrag == NED_Nodes)
	{
		for (i = 0; i < mapNodes.NumElements(); i++)
		{
			pNode = mapNodes.Get(i);
			if (pNode->IsSelected())
			{
				sNode = pNode->GetNodePosition();
				pNode->SetNodePosition(sNode.x + x - msPointer.x, sNode.y + y - msPointer.y);
			}
		}
	}
	mbDragged = true;
	msPointer.Init(x, y);
	if (meDrag == NED_Box)
	{
		SelectBox();
	}
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
// The nodes the box touches are the selected ones.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::SelectBox(void)
{
	Ptr<CNodeBox>	pNode;
	CInt32Rect		cBox;
	CInt32Rect		cBounds;
	SInt32Vec2		sEditor;
	size			i;

	if (!GetDraggedBox(&cBox))
	{
		return;
	}
	sEditor = GetPosition();
	for (i = 0; i < mapNodes.NumElements(); i++)
	{
		pNode = mapNodes.Get(i);
		pNode->GetActualBounds(&cBounds);
		pNode->SetSelected((cBounds.GetLeft() - sEditor.x <= cBox.GetRight()) && (cBounds.GetRight() - sEditor.x >= cBox.GetLeft()) && (cBounds.GetTop() - sEditor.y <= cBox.GetBottom()) && (cBounds.GetBottom() - sEditor.y >= cBox.GetTop()));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The left button's let go at x, y in the editor.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::StopDragSelect(int x, int y)
{
	ENodeEditorDrag		eDrag;

	eDrag = meDrag;
	meDrag = NED_None;
	if (eDrag == NED_Link)
	{
		DropLink(x, y);
	}
	else if ((eDrag == NED_Nodes) && mbDragged && (mpcListener != NULL))
	{
		mpcListener->NodesMoved();
	}
	mpcLinkNode = NULL;
	mbDragged = false;
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
// The dragged link's loose end is let go at x, y in the editor: over a
// socket of the other kind on another node the link is made, if the
// listener allows it, in place of any the input had.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::DropLink(int x, int y)
{
	Ptr<CNodeBox>	pNode;
	CNodeBox*		pcFrom;
	CNodeBox*		pcTo;
	SNodeLink*		psOld;
	SNodeLink		sOld;
	SInt32Vec2		sEditor;
	bool			bInput;
	size			uiSocket;
	size			uiOutput;
	size			uiInput;
	size			i;

	if (mpcLinkNode == NULL)
	{
		return;
	}
	sEditor = GetPosition();
	i = mapNodes.NumElements();
	while (i > 0)
	{
		i--;
		pNode = mapNodes.Get(i);
		if ((&pNode != mpcLinkNode) && pNode->FindSocketAt(sEditor.x + x, sEditor.y + y, &bInput, &uiSocket) && (bInput != mbLinkInput))
		{
			pcFrom = mbLinkInput ? &pNode : mpcLinkNode;
			uiOutput = mbLinkInput ? uiSocket : muiLinkSocket;
			pcTo = mbLinkInput ? mpcLinkNode : &pNode;
			uiInput = mbLinkInput ? muiLinkSocket : uiSocket;
			if ((mpcListener == NULL) || mpcListener->CanLink(pcFrom, uiOutput, pcTo, uiInput))
			{
				psOld = FindLinkTo(pcTo, uiInput);
				if (psOld != NULL)
				{
					sOld = *psOld;
					Unlink(pcTo, uiInput);
					if (mpcListener != NULL)
					{
						mpcListener->Unlinked(sOld.pcFrom, sOld.uiOutput, sOld.pcTo, sOld.uiInput);
					}
				}
				Link(pcFrom, uiOutput, pcTo, uiInput);
				if (mpcListener != NULL)
				{
					mpcListener->Linked(pcFrom, uiOutput, pcTo, uiInput);
				}
			}
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The middle button pans while it's down.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::MovementAction(SComponentInputEvent* psEvent)
{
	if (mbPanning)
	{
		SetPan(msPan.x + psEvent->x - msPointer.x, msPan.y + psEvent->y - msPointer.y);
		msPointer.Init(psEvent->x, psEvent->y);
	}
	else
	{
		psEvent->bPass = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::PanPressAction(SComponentInputEvent* psEvent)
{
	mbPanning = true;
	msPointer.Init(psEvent->x, psEvent->y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::PanReleaseAction(SComponentInputEvent* psEvent)
{
	mbPanning = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::HomeAction(SComponentInputEvent* psEvent)
{
	FrameAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::DeleteAction(SComponentInputEvent* psEvent)
{
	if (mpcListener != NULL)
	{
		mpcListener->DeleteSelected();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Shift+D; D alone is someone else's.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::DuplicateAction(SComponentInputEvent* psEvent)
{
	if (psEvent->bShift && (mpcListener != NULL))
	{
		mpcListener->DuplicateSelected();
	}
	else
	{
		psEvent->bPass = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Shift+A; A alone is someone else's.
//
//////////////////////////////////////////////////////////////////////////
void CNodeEditor::AddAction(SComponentInputEvent* psEvent)
{
	if (psEvent->bShift && (mpcListener != NULL))
	{
		mpcListener->AddNode(psEvent->x - msPan.x, psEvent->y - msPan.y);
	}
	else
	{
		psEvent->bPass = true;
	}
}

