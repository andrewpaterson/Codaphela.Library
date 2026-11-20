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
#include "BaseLib/StringHelper.h"
#include "StandardLib/Objects.h"
#include "GridContainer.h"
#include "NameRow.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGoToItems::Init(void)
{
	masItems.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGoToItems::Kill(void)
{
	Clear();
	masItems.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGoToItems::Clear(void)
{
	size		i;
	SGoToItem*	psItem;

	for (i = 0; i < masItems.NumElements(); i++)
	{
		psItem = masItems.Get(i);
		psItem->szName.Kill();
		psItem->szLocation.Kill();
	}
	masItems.RemoveRange(0, masItems.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
// psMatch is the matcher's match of the name; NULL for none.
//
//////////////////////////////////////////////////////////////////////////
SGoToItem* CGoToItems::Add(const char* szName, const char* szLocation, int32 iIcon, size uiData, SNameMatch* psMatch)
{
	SGoToItem*	psItem;

	psItem = masItems.Add();
	psItem->szName.Init(szName);
	psItem->szLocation.Init(szLocation == NULL ? "" : szLocation);
	psItem->iIcon = iIcon;
	psItem->uiData = uiData;
	if (psMatch)
	{
		memcpy(&psItem->sMatch, psMatch, sizeof(SNameMatch));
	}
	else
	{
		memset(&psItem->sMatch, 0, sizeof(SNameMatch));
	}
	return psItem;
}


//////////////////////////////////////////////////////////////////////////
//
// The best score first, then by name, then by where it is.
//
//////////////////////////////////////////////////////////////////////////
static int __cdecl CompareGoToItems(const void* pv1, const void* pv2)
{
	SGoToItem*	ps1;
	SGoToItem*	ps2;
	int			iResult;

	ps1 = (SGoToItem*)pv1;
	ps2 = (SGoToItem*)pv2;
	if (ps1->sMatch.iScore != ps2->sMatch.iScore)
	{
		return (ps1->sMatch.iScore > ps2->sMatch.iScore) ? -1 : 1;
	}
	iResult = StringInsensitiveCompare(ps1->szName.Text(), ps2->szName.Text());
	if (iResult != 0)
	{
		return iResult;
	}
	return StringCompare(ps1->szLocation.Text(), ps2->szLocation.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGoToItems::Sort(void)
{
	if (masItems.NumElements() > 1)
	{
		qsort(masItems.GetData(), masItems.NumElements(), sizeof(SGoToItem), CompareGoToItems);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGoToItems::NumItems(void) const { return masItems.NumElements(); }
SGoToItem* CGoToItems::Get(size uiIndex) { return masItems.Get(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// The icon, the name and where it is.  A row that isn't enabled is a
// heading: its name is in the dim font.
//
//////////////////////////////////////////////////////////////////////////
int32 AddNameRow(Ptr<CListContainer> pList, Ptr<CWindow> pWindow, SDialogLook* psLook, Ptr<CFont> pMatchFont, Ptr<CImageCel> pIcon, SGoToItem* psItem, bool bEnabled)
{
	Ptr<CIconComponent>		pIconComponent;
	Ptr<CLabel>				pLocation;
	Ptr<CFont>				pFont;
	int32					iRow;

	iRow = pList->AddRow(bEnabled);
	pIconComponent = OMalloc<CIconComponent>(pWindow, pIcon, psLook->pcTheme->GetMetric(TM_GoToIconWidth), psLook->pcTheme->GetMetric(TM_GoToRowHeight));
	pIconComponent->SetCelAlignment(CSH_Center, CSV_Center);
	pList->AddToRow(iRow, pIconComponent, 0);
	if (bEnabled)
	{
		pFont = psLook->pcTheme->GetFont();
	}
	else
	{
		pFont = psLook->pcDimFont;
	}
	pList->AddToRow(iRow, CreateNameRowName(pWindow, psLook, pFont, pMatchFont, psItem->szName.Text(), &psItem->sMatch), 1);

	pLocation = OMalloc<CLabel>(pWindow, psLook->pcDimFont, psItem->szLocation.Text());
	pLocation->SetAlignment(CSH_Left, CSV_Center);
	pList->AddToRow(iRow, pLocation, 2, 1, 1);
	return iRow;
}


//////////////////////////////////////////////////////////////////////////
//
// The name as a label for each run of it the pattern matched, in the
// match font, and for each run between, in pFont.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CreateNameRowName(Ptr<CWindow> pWindow, SDialogLook* psLook, Ptr<CFont> pFont, Ptr<CFont> pMatchFont, const char* szName, SNameMatch* psMatch)
{
	Ptr<CGridContainer>		pName;
	Ptr<CLabel>				pLabel;
	CChars					szPiece;
	size					uiPosition;
	size					uiEnd;
	size					uiRange;
	size					uiLength;
	bool					bMatched;
	int						iColumn;
	int32					iWidth;
	int32					iHeight;

	pName = OMalloc<CGridContainer>(pWindow);
	pName->SetAlignment(CSH_Left, CSV_Center);

	uiLength = strlen(szName);
	uiPosition = 0;
	uiRange = 0;
	iColumn = 0;
	iWidth = 0;
	iHeight = 0;
	while (uiPosition < uiLength)
	{
		if ((uiRange < psMatch->uiNumRanges) && (psMatch->asRanges[uiRange].uiStart == uiPosition))
		{
			uiEnd = uiPosition + psMatch->asRanges[uiRange].uiLength;
			uiRange++;
			bMatched = true;
		}
		else
		{
			uiEnd = (uiRange < psMatch->uiNumRanges) ? psMatch->asRanges[uiRange].uiStart : uiLength;
			bMatched = false;
		}
		if (uiEnd > uiLength)
		{
			uiEnd = uiLength;
		}
		szPiece.Init();
		szPiece.AppendSubString(szName, uiPosition, uiEnd);
		pLabel = OMalloc<CLabel>(pWindow, (bMatched && pMatchFont.IsNotNull()) ? pMatchFont : pFont, szPiece.Text());
		szPiece.Kill();
		pLabel->SetAlignment(CSH_Left, CSV_Center);
		pName->AddComponent(pLabel, iColumn, 0);
		iWidth += pLabel->GetDesiredSize().x;
		if (pLabel->GetDesiredSize().y > iHeight)
		{
			iHeight = pLabel->GetDesiredSize().y;
		}
		iColumn++;
		uiPosition = uiEnd;
	}

	//A grid in a grid is the size it's given, not the size its parts need.  A gap keeps where it is apart.
	pName->SetDesiredSize(iWidth + psLook->pcTheme->GetMetric(TM_GoToGap), iHeight);
	return pName;
}

