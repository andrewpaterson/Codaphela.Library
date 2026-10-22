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
#include "BaseLib/Logger.h"
#include "BaseLib/ArrayBit.h"
#include "StandardLib/ClassDefines.h"
#include "ContainerStyle.h"
#include "GridContainer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int32 SumTracks(CArrayInt* paiValues, size uiStart, size uiCount)
{
	size	i;
	int32	iSum;

	iSum = 0;
	for (i = uiStart; i < uiStart + uiCount; i++)
	{
		iSum += paiValues->GetValue(i);
	}
	return iSum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int32 GiveSlivers(CArrayInt* paiSizes, CArrayInt* paiWeights, size uiStart, size uiCount, int32 iPixels, int32 iChange, bool bWeightedOnly)
{
	CArrayBit	abHadOne;
	size		i;
	size		uiLargest;
	size		uiNumHadOne;
	int32		iDone;
	int32		iSize;

	//Pixels left over from sharing in proportion are handed out (iChange is 1) or taken (iChange is -1) one at a time,
	//each from the largest track that has not had one yet, ties going to the first.  Once every track has had one they
	//can all have another.  Only weighted tracks take part if bWeightedOnly, and a track can't be taken below zero.
	abHadOne.Init();
	for (i = 0; i < uiCount; i++)
	{
		abHadOne.Add(false);
	}

	iDone = 0;
	uiNumHadOne = 0;
	while (iDone < iPixels)
	{
		uiLargest = ARRAY_ELEMENT_NOT_FOUND;
		for (i = 0; i < uiCount; i++)
		{
			iSize = paiSizes->GetValue(uiStart + i);
			if (!abHadOne.Get(i) && (!bWeightedOnly || (paiWeights->GetValue(uiStart + i) > 0)) && ((iChange > 0) || (iSize > 0)))
			{
				if ((uiLargest == ARRAY_ELEMENT_NOT_FOUND) || (iSize > paiSizes->GetValue(uiStart + uiLargest)))
				{
					uiLargest = i;
				}
			}
		}

		if (uiLargest == ARRAY_ELEMENT_NOT_FOUND)
		{
			if (uiNumHadOne == 0)
			{
				break;
			}
			abHadOne.Zero();
			uiNumHadOne = 0;
		}
		else
		{
			paiSizes->SetValue(uiStart + uiLargest, paiSizes->GetValue(uiStart + uiLargest) + iChange);
			abHadOne.Set(uiLargest, true);
			uiNumHadOne++;
			iDone++;
		}
	}

	abHadOne.Kill();
	return iDone;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void ShareOut(CArrayInt* paiValues, size uiStart, size uiCount, int32 iAmount, CArrayInt* paiWeights)
{
	size	i;
	size	uiLast;
	int32	iTotalWeight;
	int32	iWeight;
	int32	iGiven;
	int32	iShare;

	//Adds iAmount to the tracks from uiStart in proportion to their weights.  With no weights it all goes to the last track.
	iTotalWeight = SumTracks(paiWeights, uiStart, uiCount);
	uiLast = uiStart + uiCount - 1;
	if (iTotalWeight == 0)
	{
		paiValues->SetValue(uiLast, paiValues->GetValue(uiLast) + iAmount);
		return;
	}

	iGiven = 0;
	for (i = uiStart; i < uiStart + uiCount; i++)
	{
		iWeight = paiWeights->GetValue(i);
		if (iWeight > 0)
		{
			iShare = (iAmount * iWeight) / iTotalWeight;
			paiValues->SetValue(i, paiValues->GetValue(i) + iShare);
			iGiven += iShare;
		}
	}
	GiveSlivers(paiValues, paiWeights, uiStart, uiCount, iAmount - iGiven, 1, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void GrowTracks(CArrayInt* paiSizes, CArrayInt* paiWeights, int32 iExtra)
{
	size	i;
	size	uiNumTracks;
	int32	iTotalWeight;
	int32	iGiven;
	int32	iShare;

	//The weighted tracks share the extra space in proportion to their weights.
	uiNumTracks = paiSizes->NumElements();
	iTotalWeight = SumTracks(paiWeights, 0, uiNumTracks);
	if (iTotalWeight == 0)
	{
		return;
	}

	iGiven = 0;
	for (i = 0; i < uiNumTracks; i++)
	{
		iShare = (iExtra * paiWeights->GetValue(i)) / iTotalWeight;
		paiSizes->SetValue(i, paiSizes->GetValue(i) + iShare);
		iGiven += iShare;
	}
	GiveSlivers(paiSizes, paiWeights, 0, uiNumTracks, iExtra - iGiven, 1, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void ShrinkTracks(CArrayInt* paiSizes, CArrayInt* paiWeights, int32 iDeficit)
{
	size	i;
	size	uiNumTracks;
	int32	iTotalWeight;
	int32	iTotalSize;
	int32	iSize;
	int32	iTake;
	int32	iTaken;
	int32	iRound;
	bool	bEmptied;

	uiNumTracks = paiSizes->NumElements();

	//First the weighted tracks give up space in proportion to their weights, none going below zero.  If a track runs out
	//what it couldn't give is shared again between the others, otherwise only rounding is left and is taken as slivers.
	while (iDeficit > 0)
	{
		iTotalWeight = 0;
		for (i = 0; i < uiNumTracks; i++)
		{
			if (paiSizes->GetValue(i) > 0)
			{
				iTotalWeight += paiWeights->GetValue(i);
			}
		}
		if (iTotalWeight == 0)
		{
			break;
		}

		iRound = iDeficit;
		iTaken = 0;
		bEmptied = false;
		for (i = 0; i < uiNumTracks; i++)
		{
			iSize = paiSizes->GetValue(i);
			if ((iSize > 0) && (paiWeights->GetValue(i) > 0))
			{
				iTake = (iRound * paiWeights->GetValue(i)) / iTotalWeight;
				if (iTake >= iSize)
				{
					iTake = iSize;
					bEmptied = true;
				}
				paiSizes->SetValue(i, iSize - iTake);
				iTaken += iTake;
			}
		}
		if (!bEmptied)
		{
			iTaken += GiveSlivers(paiSizes, paiWeights, 0, uiNumTracks, iRound - iTaken, -1, true);
		}
		iDeficit -= iTaken;
	}

	//Then every track shrinks in proportion to its size, with the rounding taken as slivers.
	if (iDeficit > 0)
	{
		iTotalSize = SumTracks(paiSizes, 0, uiNumTracks);
		if (iTotalSize <= iDeficit)
		{
			for (i = 0; i < uiNumTracks; i++)
			{
				paiSizes->SetValue(i, 0);
			}
			return;
		}

		iRound = iDeficit;
		for (i = 0; i < uiNumTracks; i++)
		{
			iSize = paiSizes->GetValue(i);
			iTake = (iRound * iSize) / iTotalSize;
			paiSizes->SetValue(i, iSize - iTake);
			iDeficit -= iTake;
		}
		GiveSlivers(paiSizes, paiWeights, 0, uiNumTracks, iDeficit, -1, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridContainer::Init(Ptr<CWindow> pWindow)
{
	CContainer::Init(pWindow);
	macCells.Init();
	maiColumnStarts.Init();
	maiColumnSizes.Init();
	maiRowStarts.Init();
	maiRowSizes.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridContainer::Free(void)
{
	maiRowSizes.Kill();
	maiRowStarts.Kill();
	maiColumnSizes.Kill();
	maiColumnStarts.Kill();
	macCells.Kill();
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridContainer::Class(void)
{
	CContainer::Class();
	U_Data(CArrayGridCells, macCells);
	U_Data(CArrayInt, maiColumnStarts);
	U_Data(CArrayInt, maiColumnSizes);
	U_Data(CArrayInt, maiRowStarts);
	U_Data(CArrayInt, maiRowSizes);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGridContainer::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGridContainer::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGridContainer::AddComponent(Ptr<CComponent> pComponent, int iColumn, int iRow, int iColumnSpan, int iRowSpan, int iWidthWeight, int iHeightWeight)
{
	CGridCell*	pcCell;

	CComponent::AddComponent(pComponent);

	pcCell = macCells.Add();
	pcCell->Init(iColumn < 0 ? 0 : iColumn, iRow < 0 ? 0 : iRow, iColumnSpan < 1 ? 1 : iColumnSpan, iRowSpan < 1 ? 1 : iRowSpan, iWidthWeight < 0 ? 0 : iWidthWeight, iHeightWeight < 0 ? 0 : iHeightWeight);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGridContainer::AddComponent(Ptr<CComponent> pComponent)
{
	return gcLogger.Error2(__METHOD__, " A component can only be added to a grid with a cell.", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGridContainer::RemoveComponent(Ptr<CComponent> pComponent)
{
	size	i;

	//The cells are kept at the same indices as the components so the component's cell goes with it.
	for (i = 0; i < maChildren.Size(); i++)
	{
		if (maChildren.Get(i) == pComponent)
		{
			macCells.RemoveAt(i);
			return CComponent::RemoveComponent(pComponent);
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGridContainer::RemoveAllComponents(void)
{
	macCells.Kill();
	macCells.Init();
	return CComponent::RemoveAllComponents();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGridCell* CGridContainer::GetCell(size uiIndex)
{
	return macCells.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGridContainer::NumTracks(bool bColumns)
{
	size		i;
	size		uiNumTracks;
	size		uiEnd;
	CGridCell*	pcCell;

	uiNumTracks = 0;
	for (i = 0; i < maChildren.Size(); i++)
	{
		pcCell = GetCell(i);
		uiEnd = (size)(ContainerStyleMainAxis(&pcCell->msPosition, bColumns) + ContainerStyleMainAxis(&pcCell->msSpan, bColumns));
		if (uiEnd > uiNumTracks)
		{
			uiNumTracks = uiEnd;
		}
	}
	return uiNumTracks;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridContainer::CalculateTracks(bool bColumns, CArrayInt* paiSizes, CArrayInt* paiWeights)
{
	size			i;
	size			uiNumTracks;
	size			uiStart;
	size			uiSpan;
	size			uiMaxSpan;
	CGridCell*		pcCell;
	Ptr<CComponent>	pComponent;
	int32			iRequired;
	int32			iWeight;
	int32			iHave;
	SInt32Vec2		sDesired;
	size			uiMainAxisSpan;

	// Works out every column's (or row's) required size and weight.
	uiNumTracks = NumTracks(bColumns);
	for (i = 0; i < uiNumTracks; i++)
	{
		paiSizes->Add(0);
		paiWeights->Add(0);
	}

	// Components sitting in a single track first.
	uiMaxSpan = 1;
	for (i = 0; i < maChildren.Size(); i++)
	{
		pcCell = GetCell(i);
		uiSpan = (size)ContainerStyleMainAxis(&pcCell->msSpan, bColumns);
		if (uiSpan > uiMaxSpan)
		{
			uiMaxSpan = uiSpan;
		}
		if (uiSpan == 1)
		{
			pComponent = maChildren.Get(i);
			uiStart = (size)ContainerStyleMainAxis(&pcCell->msPosition, bColumns);
			sDesired = pComponent->GetDesiredSize();
			iRequired = ContainerStyleMainAxis(&sDesired, bColumns);
			iWeight = ContainerStyleMainAxis(&pcCell->msWeight, bColumns);
			if (iRequired > paiSizes->GetValue(uiStart))
			{
				paiSizes->SetValue(uiStart, iRequired);
			}
			if (iWeight > paiWeights->GetValue(uiStart))
			{
				paiWeights->SetValue(uiStart, iWeight);
			}
		}
	}

	// Then components spanning tracks, narrowest first, add any weight and size their tracks still lack.
	for (uiSpan = 2; uiSpan <= uiMaxSpan; uiSpan++)
	{
		for (i = 0; i < maChildren.Size(); i++)
		{
			pcCell = GetCell(i);
			uiMainAxisSpan = (size)ContainerStyleMainAxis(&pcCell->msSpan, bColumns);
			if (uiMainAxisSpan == uiSpan)
			{
				pComponent = maChildren.Get(i);
				uiStart = (size)ContainerStyleMainAxis(&pcCell->msPosition, bColumns);
				iWeight = ContainerStyleMainAxis(&pcCell->msWeight, bColumns);
				iHave = SumTracks(paiWeights, uiStart, uiSpan);
				if (iWeight > iHave)
				{
					ShareOut(paiWeights, uiStart, uiSpan, iWeight - iHave, paiWeights);
				}
				sDesired = pComponent->GetDesiredSize();
				iRequired = ContainerStyleMainAxis(&sDesired, bColumns);
				iHave = SumTracks(paiSizes, uiStart, uiSpan);
				if (iRequired > iHave)
				{
					ShareOut(paiSizes, uiStart, uiSpan, iRequired - iHave, paiWeights);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridContainer::LayoutTracks(bool bColumns, int iStart, int iAvailable, CArrayInt* paiStarts, CArrayInt* paiSizes)
{
	CArrayInt	aiWeights;
	size		i;
	int32		iTotal;
	int32		iPos;

	aiWeights.Init();
	CalculateTracks(bColumns, paiSizes, &aiWeights);

	iTotal = SumTracks(paiSizes, 0, paiSizes->NumElements());
	if (iAvailable > iTotal)
	{
		GrowTracks(paiSizes, &aiWeights, iAvailable - iTotal);
	}
	else if (iAvailable < iTotal)
	{
		ShrinkTracks(paiSizes, &aiWeights, iTotal - iAvailable);
	}
	aiWeights.Kill();

	//Without weighted tracks to take the spare space the grid is centred in it.
	iTotal = SumTracks(paiSizes, 0, paiSizes->NumElements());
	iPos = iStart;
	if (iTotal < iAvailable)
	{
		iPos += (iAvailable - iTotal) / 2;
	}
	for (i = 0; i < paiSizes->NumElements(); i++)
	{
		paiStarts->Add(iPos);
		iPos += paiSizes->GetValue(i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridContainer::SetRequiredSize(void)
{
	CArrayInt	aiSizes;
	CArrayInt	aiWeights;

	//The required size is every column's and row's required size added up.
	aiSizes.Init();
	aiWeights.Init();
	CalculateTracks(true, &aiSizes, &aiWeights);
	msRequiredSize.x = SumTracks(&aiSizes, 0, aiSizes.NumElements());
	aiSizes.Kill();
	aiWeights.Kill();

	aiSizes.Init();
	aiWeights.Init();
	CalculateTracks(false, &aiSizes, &aiWeights);
	msRequiredSize.y = SumTracks(&aiSizes, 0, aiSizes.NumElements());
	aiSizes.Kill();
	aiWeights.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridContainer::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	size			i;
	CGridCell*		pcCell;
	Ptr<CComponent>	pComponent;
	SInt32Vec2		sCellPosition;
	SInt32Vec2		sCellSize;

	SetContainerBounds(sPosition, sAreaSize);

	//The tracks are kept so that what is in a column or row can be found later.
	maiColumnStarts.ReInit();
	maiColumnSizes.ReInit();
	maiRowStarts.ReInit();
	maiRowSizes.ReInit();
	LayoutTracks(true, sPosition.x, sAreaSize.x, &maiColumnStarts, &maiColumnSizes);
	LayoutTracks(false, sPosition.y, sAreaSize.y, &maiRowStarts, &maiRowSizes);

	for (i = 0; i < maChildren.Size(); i++)
	{
		pComponent = maChildren.Get(i);
		pcCell = GetCell(i);

		sCellPosition.Init(maiColumnStarts.GetValue(pcCell->msPosition.x), maiRowStarts.GetValue(pcCell->msPosition.y));
		sCellSize.Init(SumTracks(&maiColumnSizes, pcCell->msPosition.x, pcCell->msSpan.x), SumTracks(&maiRowSizes, pcCell->msPosition.y, pcCell->msSpan.y));
		LayoutComponentInArea(pComponent, sCellPosition, sCellSize);
	}

	SetActualBounds(sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
// Where a column (or row) started and how wide (or tall) it was the last
// time the grid was laid out, in window client space.  Returns false if
// the grid had no such column or row then.
//
//////////////////////////////////////////////////////////////////////////
bool CGridContainer::GetTrackBounds(bool bColumns, size uiTrack, int32* piStart, int32* piSize)
{
	CArrayInt*	paiStarts;
	CArrayInt*	paiSizes;

	paiStarts = bColumns ? &maiColumnStarts : &maiRowStarts;
	paiSizes = bColumns ? &maiColumnSizes : &maiRowSizes;
	if (uiTrack >= paiStarts->NumElements())
	{
		return false;
	}
	*piStart = paiStarts->GetValue(uiTrack);
	*piSize = paiSizes->GetValue(uiTrack);
	return true;
}
