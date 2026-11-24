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
#include "Window.h"
#include "TreeRow.h"


//////////////////////////////////////////////////////////////////////////
//
// The chevron and the icon are each as wide as the theme says and as tall
// as the row, their images in the middle (the icon's at the left).  The
// row shows no node until it is given one.
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::Init(Ptr<CWindow> pWindow, int32 iChevronWidth, int32 iIconWidth, int32 iRowHeight)
{
	PreInit();

	CContainer::Init(pWindow);
	maLabels.Init();
	muiNumLabels = 0;
	maCells.Init();
	maiCellLefts.Init();
	maiCellWidths.Init();
	miNameWidth = 0;
	miNode = TREE_NO_NODE;
	miIndent = 0;
	miChevronWidth = iChevronWidth;
	miIconWidth = iIconWidth;
	miRowHeight = iRowHeight;
	mbChevron = false;
	mbIcon = false;

	mpChevron = OMalloc<CIconComponent>(pWindow, Ptr<CImageCel>(), iChevronWidth, iRowHeight);
	mpChevron->SetCelAlignment(CSH_Center, CSV_Center);
	CContainer::AddComponent(mpChevron);
	mpIcon = OMalloc<CIconComponent>(pWindow, Ptr<CImageCel>(), iIconWidth, iRowHeight);
	mpIcon->SetCelAlignment(CSH_Left, CSV_Center);
	CContainer::AddComponent(mpIcon);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::Class(void)
{
	CContainer::Class();
	M_Pointer(mpChevron);
	M_Pointer(mpIcon);
	M_Embedded(maLabels);
	U_Size(muiNumLabels);
	M_Embedded(maCells);
	U_Unknown(CArrayInt, maiCellLefts);
	U_Unknown(CArrayInt, maiCellWidths);
	U_Int32(miNameWidth);
	U_Int32(miNode);
	U_Int32(miIndent);
	U_Int32(miChevronWidth);
	U_Int32(miIconWidth);
	U_Int32(miRowHeight);
	U_Bool(mbChevron);
	U_Bool(mbIcon);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::Free(void)
{
	maiCellWidths.Kill();
	maiCellLefts.Kill();
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTreeRow::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTreeRow::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The node the row shows, TREE_NO_NODE for none, and how far in it is.
// What of the node is shown is set after: its chevron, icon and labels.
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::SetNode(int32 iNode, int32 iIndent)
{
	miNode = iNode;
	miIndent = iIndent;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL for a node without children: the chevron's width is left empty.
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::SetChevron(Ptr<CImageCel> pCel)
{
	mbChevron = pCel.IsNotNull();
	if (mbChevron && (mpChevron->GetCel() != pCel))
	{
		mpChevron->SetCel(pCel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// NULL for a node without an icon: its name starts where the icon would.
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::SetIcon(Ptr<CImageCel> pCel)
{
	mbIcon = pCel.IsNotNull();
	if (mbIcon && (mpIcon->GetCel() != pCel))
	{
		mpIcon->SetCel(pCel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The uiIndex'th part of the node's name, a label the row has already if
// a node it showed had as many parts.  The parts are set in order, then
// how many there are (SetNumLabels).
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::SetLabel(size uiIndex, Ptr<CFont> pFont, const char* szText)
{
	SetLabelIn(&maLabels, uiIndex, pFont, szText);
}


//////////////////////////////////////////////////////////////////////////
//
// The node's uiIndex'th cell, after its name: in its column, a label made
// the first time it is set.
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::SetCell(size uiIndex, Ptr<CFont> pFont, const char* szText)
{
	while (maCells.NumElements() < uiIndex)
	{
		SetLabelIn(&maCells, maCells.NumElements(), pFont, "");
	}
	SetLabelIn(&maCells, uiIndex, pFont, szText);
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the row has no such cell.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CLabel> CTreeRow::GetCell(size uiIndex)
{
	return (uiIndex < maCells.NumElements()) ? maCells.Get(uiIndex) : Ptr<CLabel>();
}


//////////////////////////////////////////////////////////////////////////
//
// Where the columns are, from the row's left: the name is cut off at
// iNameWidth (0: it isn't), and each cell is at its left, as wide as its
// width.
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::SetColumns(int32 iNameWidth, CArrayInt* paiLefts, CArrayInt* paiWidths)
{
	miNameWidth = iNameWidth;
	maiCellLefts.ReInit();
	maiCellLefts.Copy(paiLefts);
	maiCellWidths.ReInit();
	maiCellWidths.Copy(paiWidths);
}


//////////////////////////////////////////////////////////////////////////
//
// The uiIndex'th of paLabels shows szText in pFont: the label the row has
// already, changed only where it differs, or one added.
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::SetLabelIn(CArray<CLabel>* paLabels, size uiIndex, Ptr<CFont> pFont, const char* szText)
{
	Ptr<CLabel>		pLabel;

	if (uiIndex < paLabels->NumElements())
	{
		pLabel = paLabels->Get(uiIndex);
		if (pLabel->GetFont() != pFont)
		{
			pLabel->SetFontAndText(pFont, szText);
		}
		else if (strcmp(pLabel->GetText(), szText) != 0)
		{
			pLabel->SetText(szText);
		}
	}
	else if (uiIndex == paLabels->NumElements())
	{
		pLabel = OMalloc<CLabel>(mpWindow, pFont, szText);
		pLabel->SetAlignment(CSH_Left, CSV_Center);
		paLabels->Add(pLabel);
		CContainer::AddComponent(pLabel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the row's labels are the node's; the rest aren't drawn.
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::SetNumLabels(size uiNumLabels)
{
	muiNumLabels = (uiNumLabels < maLabels.NumElements()) ? uiNumLabels : maLabels.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the node shown hasn't as many parts.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CLabel> CTreeRow::GetLabel(size uiIndex)
{
	if (uiIndex >= muiNumLabels)
	{
		return NULL;
	}
	return maLabels.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// What the node shown needs: its indent, the chevron's width, the icon's
// if it has one, and its labels' widths, by the row's height.
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::SetRequiredSize(void)
{
	size	i;

	msRequiredSize.Init(miIndent + miChevronWidth + (mbIcon ? miIconWidth : 0), miRowHeight);
	for (i = 0; i < muiNumLabels; i++)
	{
		msRequiredSize.x += maLabels.Get(i)->GetDesiredSize().x;
	}
	if (maiCellLefts.NumElements() > 0)
	{
		msRequiredSize.x = maiCellLefts.GetValue(maiCellLefts.NumElements() - 1) + maiCellWidths.GetValue(maiCellWidths.NumElements() - 1);
	}
	AddInsets(&msRequiredSize);
}


//////////////////////////////////////////////////////////////////////////
//
// From the left: the indent, the chevron, the icon, then each label as
// wide as its text.  What doesn't fit across the row is cut off at its
// right.
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	SInt32Vec2		sOrigin;
	SInt32Vec2		sArea;
	SInt32Vec2		sCellPosition;
	SInt32Vec2		sCellSize;
	Ptr<CLabel>		pLabel;
	size			i;

	sOrigin = sPosition;
	sArea = sAreaSize;
	InsetArea(&sOrigin, &sArea);
	SetContainerBounds(sOrigin, sArea);

	sCellPosition.Init(sOrigin.x + miIndent, sOrigin.y);
	sCellSize.Init(miChevronWidth, sArea.y);
	LayoutComponentInArea(mpChevron, sCellPosition, sCellSize);
	sCellPosition.x += miChevronWidth;

	sCellSize.x = miIconWidth;
	LayoutComponentInArea(mpIcon, sCellPosition, sCellSize);
	if (mbIcon)
	{
		sCellPosition.x += miIconWidth;
	}

	for (i = 0; i < muiNumLabels; i++)
	{
		pLabel = maLabels.Get(i);
		sCellSize.x = pLabel->GetDesiredSize().x;
		LayoutLabel(pLabel, sCellPosition, sArea.y, (miNameWidth > 0) ? sOrigin.x + miNameWidth - sCellPosition.x : sCellSize.x);
		sCellPosition.x += sCellSize.x;
	}

	for (i = 0; (i < maCells.NumElements()) && (i < maiCellLefts.NumElements()); i++)
	{
		LayoutLabel(maCells.Get(i), SInt32Vec2(sOrigin.x + maiCellLefts.GetValue(i), sOrigin.y), sArea.y, maiCellWidths.GetValue(i));
	}

	SetActualBounds(sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
// A label at sPosition across the row's middle, as wide as its text but
// cut off at iMostWidth.
//
//////////////////////////////////////////////////////////////////////////
void CTreeRow::LayoutLabel(Ptr<CLabel> pLabel, SInt32Vec2 sPosition, int32 iHeight, int32 iMostWidth)
{
	SInt32Vec2	sSize;

	sSize = pLabel->GetDesiredSize();
	sSize.x = (sSize.x < iMostWidth) ? sSize.x : ((iMostWidth > 0) ? iMostWidth : 0);
	sPosition.y += (iHeight - sSize.y) / 2;
	pLabel->Layout(sPosition, sSize);
}


//////////////////////////////////////////////////////////////////////////
//
// A row showing no node isn't drawn.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeRow::IsDrawn(void) const
{
	return miNode != TREE_NO_NODE;
}


//////////////////////////////////////////////////////////////////////////
//
// The chevron and the icon only if the node shown has them, and only the
// labels that are its.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeRow::IsChildDrawn(Ptr<CComponent> pChild)
{
	size	i;

	if (pChild == mpChevron)
	{
		return mbChevron;
	}
	if (pChild == mpIcon)
	{
		return mbIcon;
	}
	for (i = 0; i < muiNumLabels; i++)
	{
		if (pChild == maLabels.Get(i))
		{
			return true;
		}
	}
	for (i = 0; (i < maCells.NumElements()) && (i < maiCellLefts.NumElements()); i++)
	{
		if (pChild == maCells.Get(i))
		{
			return true;
		}
	}
	return false;
}


int32 CTreeRow::GetNode(void) const { return miNode; }
size CTreeRow::NumLabels(void) const { return muiNumLabels; }
bool CTreeRow::HasChevron(void) const { return mbChevron; }
bool CTreeRow::HasIcon(void) const { return mbIcon; }

