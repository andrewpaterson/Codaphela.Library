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
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "NodeBox.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeBoxDraw::Init(CNodeBox* pcBox)
{
	PreInit();

	CCelCanvasDraw::Init();
	mpcBox = pcBox;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeBoxDraw::Class(void)
{
	CCelCanvasDraw::Class();

	U_Pointer(mpcBox);
}


//////////////////////////////////////////////////////////////////////////
//
// The body is the canvas less a socket's radius each side, so the
// sockets' dots, centred on its edges, are within the canvas.  The
// border is the body drawn a pixel larger behind it; the header is the
// top of the body with the same corners, square below.
//
//////////////////////////////////////////////////////////////////////////
void CNodeBoxDraw::Paint(CImageDraw* pcDraw, SInt32Vec2 sSize)
{
	Ptr<CWindowTheme>	pTheme;
	SNodeBoxSocket*		psSocket;
	SInt32Vec2			sBox;
	SInt32Vec2			sSocket;
	float64				fLeft;
	float64				fRight;
	size				uiInput;
	size				i;

	pTheme = mpcBox->GetTheme();
	fLeft = NODE_BOX_SOCKET_RADIUS;
	fRight = sSize.x - NODE_BOX_SOCKET_RADIUS;

	pcDraw->SetColour(pTheme->GetColour(mpcBox->IsSelected() ? TC_Accent : TC_Border));
	pcDraw->DrawRoundedRectangle(fLeft, 0, fRight, sSize.y, NODE_BOX_CORNER_RADIUS);
	pcDraw->SetColour(pTheme->GetColour(TC_Panel));
	pcDraw->DrawRoundedRectangle(fLeft + 1, 1, fRight - 1, sSize.y - 1, NODE_BOX_CORNER_RADIUS - 1);
	pcDraw->SetColour(mpcBox->GetHeaderColour());
	pcDraw->DrawRoundedRectangle(fLeft + 1, 1, fRight - 1, mpcBox->GetHeaderHeight(), NODE_BOX_CORNER_RADIUS - 1, 0);

	sBox = mpcBox->GetPosition();
	for (uiInput = 0; uiInput < 2; uiInput++)
	{
		for (i = 0; i < mpcBox->NumSockets(uiInput == 1); i++)
		{
			psSocket = mpcBox->GetSocket(uiInput == 1, i);
			sSocket = mpcBox->GetSocketPosition(uiInput == 1, i);
			pcDraw->SetColour(pTheme->GetColour(TC_Border));
			pcDraw->DrawCircle(sSocket.x - sBox.x, sSocket.y - sBox.y, NODE_BOX_SOCKET_RADIUS);
			pcDraw->SetColour(psSocket->sColour);
			pcDraw->DrawCircle(sSocket.x - sBox.x, sSocket.y - sBox.y, NODE_BOX_SOCKET_RADIUS - 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// sHeader is the colour behind the title, which says what kind of node
// it is.
//
//////////////////////////////////////////////////////////////////////////
void CNodeBox::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szTitle, ARGB32 sHeader)
{
	PreInit();

	CFillContainer::Init(pWindow);
	mpTheme = pTheme;
	msHeader = sHeader;
	miNumRows = 0;
	msNodePosition.Init(0, 0);
	mbSelected = false;
	miData = 0;
	mapShelved.Init();
	masSockets.Init();

	mpDraw = OMalloc<CNodeBoxDraw>(this);
	mpBack = OMalloc<CCanvas>(pWindow, mpDraw);
	mpBack->SetTransparent(true);
	mpBack->SetAlignment(CSH_Stretch, CSV_Stretch);

	//The rows are inside the body, clear of the sockets' dots.
	mpRows = OMalloc<CGridContainer>(pWindow);
	mpRows->SetAlignment(CSH_Stretch, CSV_Top);
	mpRows->SetInsets(NODE_BOX_PADDING, 2 * NODE_BOX_SOCKET_RADIUS + NODE_BOX_PADDING, NODE_BOX_PADDING, 2 * NODE_BOX_SOCKET_RADIUS + NODE_BOX_PADDING);

	mpTitle = OMalloc<CLabel>(pWindow, pTheme->GetFont(), szTitle);
	mpTitle->SetAlignment(CSH_Left, CSV_Center);
	AddRow(mpTitle);

	SetDesiredSize(NODE_BOX_WIDTH, -1);
	SetAlignment(CSH_Left, CSV_Top);
	CFillContainer::AddComponent(mpBack);
	CFillContainer::AddComponent(mpRows);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeBox::Class(void)
{
	CFillContainer::Class();

	M_Pointer(mpTheme);
	M_Pointer(mpDraw);
	M_Pointer(mpBack);
	M_Pointer(mpRows);
	M_Pointer(mpTitle);
	M_Embedded(mapShelved);
	U_Unknown(CArrayNodeBoxSocket, masSockets);
	U_UInt32(msHeader);
	U_Int32(miNumRows);
	U_2Int32(msNodePosition);
	U_Bool(mbSelected);
	U_Int64(miData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeBox::Free(void)
{
	masSockets.Kill();
	CFillContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// A row under the last, as wide as the box's inside, with a little room
// under it.
//
//////////////////////////////////////////////////////////////////////////
void CNodeBox::AddRow(Ptr<CComponent> pComponent)
{
	mpRows->AddComponent(pComponent, 0, miNumRows, 1, 1, 1, 0);
	miNumRows++;
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
// Returns the output's index, which counts outputs only.
//
//////////////////////////////////////////////////////////////////////////
size CNodeBox::AddOutput(const char* szName, ARGB32 sColour)
{
	return AddSocket(false, szName, sColour, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns the input's index, which counts inputs only.  pValue, if it's
// given, is shown instead of the name until something is linked to the
// input.
//
//////////////////////////////////////////////////////////////////////////
size CNodeBox::AddInput(const char* szName, ARGB32 sColour, Ptr<CComponent> pValue)
{
	return AddSocket(true, szName, sColour, pValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CNodeBox::AddSocket(bool bInput, const char* szName, ARGB32 sColour, Ptr<CComponent> pValue)
{
	Ptr<CLabel>			pName;
	SNodeBoxSocket*		psSocket;

	pName = OMalloc<CLabel>(mpWindow, mpTheme->GetFont(), szName);
	pName->SetAlignment(bInput ? CSH_Left : CSH_Right, CSV_Center);

	psSocket = masSockets.Add();
	psSocket->bInput = bInput;
	psSocket->sColour = sColour;
	psSocket->iRow = miNumRows;
	psSocket->pcName = &pName;
	psSocket->pcValue = &pValue;
	psSocket->bLinked = false;
	miNumRows++;

	//Whichever of the name and the value isn't showing is kept here.
	mapShelved.Add(pName);
	if (pValue.IsNotNull())
	{
		pValue->SetAlignment(CSH_Stretch, CSV_Center);
		mapShelved.Add(pValue);
	}
	ShowInput(psSocket);
	return NumSockets(bInput) - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The socket's row holds its value if it has one and nothing's linked to
// it, its name otherwise.
//
//////////////////////////////////////////////////////////////////////////
void CNodeBox::ShowInput(SNodeBoxSocket* psSocket)
{
	CComponent*		pcShow;
	CComponent*		pcHide;

	pcShow = ((psSocket->pcValue != NULL) && !psSocket->bLinked) ? psSocket->pcValue : psSocket->pcName;
	pcHide = (pcShow == psSocket->pcName) ? psSocket->pcValue : psSocket->pcName;
	if (pcHide != NULL)
	{
		mpRows->RemoveComponent(pcHide);
	}
	mpRows->RemoveComponent(pcShow);
	mpRows->AddComponent(pcShow, 0, psSocket->iRow, 1, 1, 1, 0);
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CNodeBox::NumSockets(bool bInput)
{
	size	uiCount;
	size	i;

	uiCount = 0;
	for (i = 0; i < masSockets.NumElements(); i++)
	{
		if (masSockets.Get(i)->bInput == bInput)
		{
			uiCount++;
		}
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// The uiIndex'th input, or output: NULL if there are fewer.
//
//////////////////////////////////////////////////////////////////////////
SNodeBoxSocket* CNodeBox::GetSocket(bool bInput, size uiIndex)
{
	SNodeBoxSocket*		psSocket;
	size				i;

	for (i = 0; i < masSockets.NumElements(); i++)
	{
		psSocket = masSockets.Get(i);
		if (psSocket->bInput == bInput)
		{
			if (uiIndex == 0)
			{
				return psSocket;
			}
			uiIndex--;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The middle of the socket's dot in the window, once the box is laid
// out: on the body's left edge for an input and its right for an output,
// level with the middle of its row.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CNodeBox::GetSocketPosition(bool bInput, size uiIndex)
{
	SNodeBoxSocket*		psSocket;
	CComponent*			pcRow;
	SInt32Vec2			sPosition;

	sPosition = GetPosition();
	psSocket = GetSocket(bInput, uiIndex);
	if (psSocket == NULL)
	{
		return sPosition;
	}
	pcRow = ((psSocket->pcValue != NULL) && !psSocket->bLinked) ? psSocket->pcValue : psSocket->pcName;
	sPosition.x += bInput ? NODE_BOX_SOCKET_RADIUS : GetActualSize().x - NODE_BOX_SOCKET_RADIUS;
	sPosition.y = pcRow->GetPosition().y + pcRow->GetActualSize().y / 2;
	return sPosition;
}


//////////////////////////////////////////////////////////////////////////
//
// The socket whose dot the window's point x, y is on or just by.
//
//////////////////////////////////////////////////////////////////////////
bool CNodeBox::FindSocketAt(int32 x, int32 y, bool* pbInput, size* puiIndex)
{
	SInt32Vec2	sSocket;
	int32		iReach;
	size		uiInput;
	size		i;

	iReach = NODE_BOX_SOCKET_RADIUS + 2;
	for (uiInput = 0; uiInput < 2; uiInput++)
	{
		for (i = 0; i < NumSockets(uiInput == 1); i++)
		{
			sSocket = GetSocketPosition(uiInput == 1, i);
			if ((x >= sSocket.x - iReach) && (x <= sSocket.x + iReach) && (y >= sSocket.y - iReach) && (y <= sSocket.y + iReach))
			{
				*pbInput = (uiInput == 1);
				*puiIndex = i;
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Something is, or isn't any more, linked to the input: its value gives
// way to its name, or comes back.
//
//////////////////////////////////////////////////////////////////////////
void CNodeBox::SetInputLinked(size uiIndex, bool bLinked)
{
	SNodeBoxSocket*		psSocket;

	psSocket = GetSocket(true, uiIndex);
	if ((psSocket != NULL) && (psSocket->bLinked != bLinked))
	{
		psSocket->bLinked = bLinked;
		ShowInput(psSocket);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNodeBox::IsInputLinked(size uiIndex)
{
	SNodeBoxSocket*		psSocket;

	psSocket = GetSocket(true, uiIndex);
	return (psSocket != NULL) && psSocket->bLinked;
}


//////////////////////////////////////////////////////////////////////////
//
// The header goes from the box's top to under the title's row.
//
//////////////////////////////////////////////////////////////////////////
int32 CNodeBox::GetHeaderHeight(void)
{
	return (mpTitle->GetPosition().y - GetPosition().y) + mpTitle->GetActualSize().y + NODE_BOX_PADDING / 2;
}


//////////////////////////////////////////////////////////////////////////
//
// What it looks like has changed: its back is painted again, and it's
// laid out again.
//
//////////////////////////////////////////////////////////////////////////
void CNodeBox::Changed(void)
{
	if (mpDraw.IsNotNull())
	{
		mpDraw->Repaint();
		mpBack->Invalidate();
	}
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNodeBox::SetSelected(bool bSelected)
{
	if (mbSelected != bSelected)
	{
		mbSelected = bSelected;
		mpDraw->Repaint();
		mpBack->Invalidate();
	}
}


const char* CNodeBox::GetTitle(void) { return mpTitle->GetText(); }
ARGB32 CNodeBox::GetHeaderColour(void) const { return msHeader; }
Ptr<CWindowTheme> CNodeBox::GetTheme(void) const { return mpTheme; }
void CNodeBox::SetNodePosition(int32 x, int32 y) { msNodePosition.Init(x, y); InvalidateLayout(); }
SInt32Vec2 CNodeBox::GetNodePosition(void) const { return msNodePosition; }
bool CNodeBox::IsSelected(void) const { return mbSelected; }
void CNodeBox::SetData(int64 iData) { miData = iData; }
int64 CNodeBox::GetData(void) const { return miData; }

