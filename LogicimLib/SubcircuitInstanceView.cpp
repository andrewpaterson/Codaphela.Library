/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "MicroprocessorLib/SubcircuitInstance.h"
#include "MicroprocessorLib/SubcircuitSimulation.h"
#include "CircuitView.h"
#include "CircuitViews.h"
#include "SubcircuitInstanceView.h"


//A side of an instance's pins grouped by anchour, as Logicim's PinViewLists: each group sorted by weight and then
//name (and then the order the pins are in their circuit), the centre group spread out from the middle and the
//positive group reversed.  The pin size is the pins and a space between groups; the extended size has a space at
//each end too.
struct SPinSide
{
	CArrayTemplatePtr<CPinView>		apcNegative;
	CArrayTemplatePtr<CPinView>		apcCentre;
	CArrayTemplatePtr<CPinView>		apcPositive;
	int								iPinSize;
	int								iExtendedSize;

	void	Init(CCircuitView* pcCircuit, EPinAlignment eAlignment);
	void	Kill(void);
};


//Where a pin is along its side, as Logicim's PinViewOffset.
struct SPinOffset
{
	CPinView*	pcPin;
	int			iOffset;
};


typedef CArrayTemplate<SPinOffset>	CArrayPinOffset;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int IntMin(int i1, int i2)
{
	return (i1 < i2) ? i1 : i2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int IntMax(int i1, int i2)
{
	return (i1 > i2) ? i1 : i2;
}


//////////////////////////////////////////////////////////////////////////
//
// Weight, then name.
//
//////////////////////////////////////////////////////////////////////////
static int ComparePinViews(CPinView* pcPin1, CPinView* pcPin2)
{
	if (pcPin1->GetWeight() != pcPin2->GetWeight())
	{
		return (pcPin1->GetWeight() < pcPin2->GetWeight()) ? -1 : 1;
	}
	return strcmp(pcPin1->GetName(), pcPin2->GetName());
}


//////////////////////////////////////////////////////////////////////////
//
// A stable insertion sort, so pins that compare equal stay in the order
// they're in their circuit (as Logicim's ids).
//
//////////////////////////////////////////////////////////////////////////
static void SortPinViews(CArrayTemplatePtr<CPinView>* papcPins)
{
	size		i;
	size		j;
	CPinView*	pcPin;

	for (i = 1; i < papcPins->NumElements(); i++)
	{
		pcPin = papcPins->GetPtr(i);
		j = i;
		while ((j > 0) && (ComparePinViews(papcPins->GetPtr(j - 1), pcPin) > 0))
		{
			papcPins->SetPtr(j, papcPins->GetPtr(j - 1));
			j--;
		}
		papcPins->SetPtr(j, pcPin);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's orderCenterPins: the first pin in the middle and the rest
// alternately before and after it.
//
//////////////////////////////////////////////////////////////////////////
static void OrderCentrePinViews(CArrayTemplatePtr<CPinView>* papcPins)
{
	CArrayTemplatePtr<CPinView>	apcOrdered;
	int							iEven;
	int							iOdd;
	bool						bEven;
	size						i;
	int							iIndex;

	if (papcPins->NumElements() == 0)
	{
		return;
	}

	apcOrdered.Init();
	for (i = 0; i < papcPins->NumElements(); i++)
	{
		apcOrdered.Add(NULL);
	}
	iEven = ((int)papcPins->NumElements() - 1) / 2;
	iOdd = iEven + 1;
	bEven = true;
	for (i = 0; i < papcPins->NumElements(); i++)
	{
		if (bEven)
		{
			iIndex = iEven;
			iEven--;
		}
		else
		{
			iIndex = iOdd;
			iOdd++;
		}
		apcOrdered.SetPtr(iIndex, papcPins->GetPtr(i));
		bEven = !bEven;
	}
	for (i = 0; i < papcPins->NumElements(); i++)
	{
		papcPins->SetPtr(i, apcOrdered.GetPtr(i));
	}
	apcOrdered.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void ReversePinViews(CArrayTemplatePtr<CPinView>* papcPins)
{
	size		i;
	size		uiNum;
	CPinView*	pcPin;

	uiNum = papcPins->NumElements();
	for (i = 0; i < uiNum / 2; i++)
	{
		pcPin = papcPins->GetPtr(i);
		papcPins->SetPtr(i, papcPins->GetPtr(uiNum - 1 - i));
		papcPins->SetPtr(uiNum - 1 - i, pcPin);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The circuit's pins on the side, as Logicim's QuadPinViewLists.
//
//////////////////////////////////////////////////////////////////////////
void SPinSide::Init(CCircuitView* pcCircuit, EPinAlignment eAlignment)
{
	CArrayTemplatePtr<CPinView>	apcPins;
	size						i;
	CPinView*					pcPin;
	int							iGroups;

	apcNegative.Init();
	apcCentre.Init();
	apcPositive.Init();

	apcPins.Init();
	GetCircuitPinViews(pcCircuit, &apcPins);
	for (i = 0; i < apcPins.NumElements(); i++)
	{
		pcPin = apcPins.GetPtr(i);
		if (pcPin->GetAlignment() == eAlignment)
		{
			if (pcPin->GetAnchour() == PAN_Negative)
			{
				apcNegative.Add(pcPin);
			}
			else if (pcPin->GetAnchour() == PAN_Centre)
			{
				apcCentre.Add(pcPin);
			}
			else if (pcPin->GetAnchour() == PAN_Positive)
			{
				apcPositive.Add(pcPin);
			}
		}
	}
	apcPins.Kill();

	SortPinViews(&apcNegative);
	SortPinViews(&apcCentre);
	SortPinViews(&apcPositive);
	OrderCentrePinViews(&apcCentre);
	ReversePinViews(&apcPositive);

	iGroups = 0;
	iGroups += (apcNegative.NumElements() > 0) ? 1 : 0;
	iGroups += (apcCentre.NumElements() > 0) ? 1 : 0;
	iGroups += (apcPositive.NumElements() > 0) ? 1 : 0;
	if (iGroups == 0)
	{
		iPinSize = 0;
		iExtendedSize = 0;
	}
	else
	{
		iPinSize = (iGroups - 1) + (int)(apcNegative.NumElements() + apcCentre.NumElements() + apcPositive.NumElements());
		iExtendedSize = iPinSize + 2;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SPinSide::Kill(void)
{
	apcPositive.Kill();
	apcCentre.Kill();
	apcNegative.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Consecutive offsets from iOffset, and a space after them; the offset
// after that space.
//
//////////////////////////////////////////////////////////////////////////
static int AddPinOffsets(CArrayPinOffset* pasOffsets, int iOffset, CArrayTemplatePtr<CPinView>* papcPins)
{
	size			i;
	SPinOffset*		psOffset;

	if (papcPins->NumElements() > 0)
	{
		for (i = 0; i < papcPins->NumElements(); i++)
		{
			psOffset = pasOffsets->Add();
			psOffset->pcPin = papcPins->GetPtr(i);
			psOffset->iOffset = iOffset;
			iOffset++;
		}
		iOffset++;
	}
	return iOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's calculatePinViewOffsets: the negative pins from just after
// iOffset, the positive pins up to iOpposite and the centre pins in the
// middle of the space left, into an initialised array.
//
//////////////////////////////////////////////////////////////////////////
static void CalculatePinOffsets(CArrayPinOffset* pasOffsets, SPinSide* psSide, int iOffset, int iOpposite)
{
	int		iMissing;
	int		iLastNegative;

	iMissing = (iOpposite - iOffset) - psSide->iPinSize;
	iLastNegative = AddPinOffsets(pasOffsets, iOffset + 1, &psSide->apcNegative);
	AddPinOffsets(pasOffsets, iOpposite - (int)psSide->apcPositive.NumElements(), &psSide->apcPositive);
	AddPinOffsets(pasOffsets, iLastNegative + iMissing / 2, &psSide->apcCentre);
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's PinViewOffsets.calculateMaxSize: the larger of two extended
// sizes, less one.
//
//////////////////////////////////////////////////////////////////////////
static int CalculateMaxGridSize(int iFirst, int iSecond)
{
	iFirst = (iFirst != 0) ? iFirst - 1 : 0;
	iSecond = (iSecond != 0) ? iSecond - 1 : 0;
	return IntMax(iFirst, iSecond);
}


//////////////////////////////////////////////////////////////////////////
//
// The widest of the side's pins' labels, measured as the pins' own bold
// labels (as Logicim does).
//
//////////////////////////////////////////////////////////////////////////
static float64 MaxPinLabelWidth(CCircuitFonts* pcFonts, SPinSide* psSide)
{
	CArrayTemplatePtr<CPinView>*	apapcGroups[3];
	size							i;
	size							j;
	float64							fWidth;
	float64							fMax;

	apapcGroups[0] = &psSide->apcNegative;
	apapcGroups[1] = &psSide->apcCentre;
	apapcGroups[2] = &psSide->apcPositive;
	fMax = 0;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < apapcGroups[i]->NumElements(); j++)
		{
			fWidth = pcFonts->Measure(CF_SansSerifBold, VALUE_FONT_SIZE, apapcGroups[i]->GetPtr(j)->GetName()).fWidth;
			if (fWidth > fMax)
			{
				fMax = fWidth;
			}
		}
	}
	return fMax;
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's TopBottomPinViewOffsets: the span of the left and right sides
// widened (evenly) to fit their pins.
//
//////////////////////////////////////////////////////////////////////////
static void FitLeftRightPins(SPinSide* psLeft, SPinSide* psRight, int* piTop, int* piBottom)
{
	int		iMaxGridHeight;
	int		iSize;
	int		iHeight;

	iMaxGridHeight = CalculateMaxGridSize(psLeft->iExtendedSize, psRight->iExtendedSize);
	iSize = *piBottom - *piTop;
	if (iMaxGridHeight >= iSize)
	{
		iHeight = iMaxGridHeight - iSize;
		*piTop -= iHeight / 2;
		*piBottom += iHeight - (iHeight / 2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's SubcircuitView.findAllPins: the circuit's pins in the order
// they're in it.
//
//////////////////////////////////////////////////////////////////////////
void GetCircuitPinViews(CCircuitView* pcCircuit, CArrayTemplatePtr<CPinView>* papcPins)
{
	size				i;
	CComponentView*		pcComponent;

	for (i = 0; i < pcCircuit->NumComponents(); i++)
	{
		pcComponent = pcCircuit->GetComponent(i);
		if (StringCompare(pcComponent->GetType(), "Pin") == 0)
		{
			papcPins->Add((CPinView*)pcComponent);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A new instance, laid out with the fonts.
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szComment, CCircuitView* pcInstanceCircuitView, CCircuitFonts* pcFonts)
{
	SSubcircuitInstanceLayout	sLayout;

	MeasureLayout(pcFonts, pcInstanceCircuitView, szName, szComment, &sLayout);
	Init(x, y, eRotation, szName, szComment, pcInstanceCircuitView->GetTypeName(), &sLayout);
	SetInstanceCircuitView(pcInstanceCircuitView);
}


//////////////////////////////////////////////////////////////////////////
//
// An instance of the subcircuit named szTypeName, which has no pins or
// shapes until the subcircuit is set.
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szComment, const char* szTypeName, SSubcircuitInstanceLayout* psLayout)
{
	CComponentView::Init(x, y, eRotation, szName);
	mszTypeName.Init(szTypeName);
	mszComment.Init(szComment);
	mpcInstanceCircuitView = NULL;
	msLayout = *psLayout;
	masPins.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceView::Kill(void)
{
	masPins.Kill();
	mpcInstanceCircuitView = NULL;
	mszComment.Kill();
	mszTypeName.Kill();
	CComponentView::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Sets the subcircuit, which must have the instance's type name, and lays
// out its pins.  It can only be set once.
//
//////////////////////////////////////////////////////////////////////////
bool CSubcircuitInstanceView::SetInstanceCircuitView(CCircuitView* pcInstanceCircuitView)
{
	if (mpcInstanceCircuitView != NULL)
	{
		return gcLogger.Error2(__METHOD__, " Instance [", mszName.Text(), "] already has its subcircuit.", NULL);
	}
	if (!mszTypeName.Equals(pcInstanceCircuitView->GetTypeName()))
	{
		return gcLogger.Error2(__METHOD__, " Instance [", mszName.Text(), "] is of [", mszTypeName.Text(), "], not [", pcInstanceCircuitView->GetTypeName(), "].", NULL);
	}
	mpcInstanceCircuitView = pcInstanceCircuitView;
	LayOut();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's createTypeGraphics and createPinsAndGraphics.  Logicim
// works in a frame turned against the instance's; it's kept here, as R,
// so the steps read as Logicim's, and turned into the instance's at the
// end (R's x is -x and R's y is y).  The type name, comment and name are
// stacked down the instance, each centred, and their box is padded; then
// each side is widened to fit its pins and their labels.
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceView::MeasureLayout(CCircuitFonts* pcFonts, CCircuitView* pcInstanceCircuitView, const char* szName, const char* szComment, SSubcircuitInstanceLayout* psLayout)
{
	SCircuitTextSize	asSizes[3];
	bool				abShown[3];
	float64*			apfX[3];
	float64				fHeight;
	float64				fMaxOffset;
	float64				fOffset;
	float64				fMinX;
	float64				fMaxX;
	float64				fMaxY;
	size				i;
	bool				bFirst;
	int					iLeftR;
	int					iTopR;
	int					iRightR;
	int					iBottomR;
	SPinSide			sTop;
	SPinSide			sBottom;
	SPinSide			sLeft;
	SPinSide			sRight;
	int					iMaxGridWidth;
	int					iTop;
	int					iBottom;

	asSizes[0] = pcFonts->Measure(CF_SansSerifBold, SUBCIRCUIT_TYPE_FONT_SIZE, pcInstanceCircuitView->GetTypeName());
	abShown[0] = true;
	asSizes[1] = pcFonts->Measure(CF_SansSerif, SUBCIRCUIT_TEXT_FONT_SIZE, szComment);
	abShown[1] = !StrEmpty(szComment);
	asSizes[2] = pcFonts->Measure(CF_SansSerif, SUBCIRCUIT_TEXT_FONT_SIZE, szName);
	abShown[2] = !StrEmpty(szName);
	apfX[0] = &psLayout->fTypeY;
	apfX[1] = &psLayout->fCommentY;
	apfX[2] = &psLayout->fNameY;

	//TextView.centerHorizontally.
	fHeight = 0;
	fMaxOffset = 0;
	for (i = 0; i < 3; i++)
	{
		*apfX[i] = 0;
		if (abShown[i])
		{
			fHeight += asSizes[i].fHeight;
			if (asSizes[i].fHeight / 2 > fMaxOffset)
			{
				fMaxOffset = asSizes[i].fHeight / 2;
			}
		}
	}
	fOffset = -fHeight / 2 + fMaxOffset;
	for (i = 0; i < 3; i++)
	{
		if (abShown[i])
		{
			*apfX[i] = fOffset;
			fOffset += asSizes[i].fHeight;
		}
	}

	//The texts' box (TextView.calculateTopLeft and calculateBottomRight).
	bFirst = true;
	fMinX = 0;
	fMaxX = 0;
	fMaxY = 0;
	for (i = 0; i < 3; i++)
	{
		if (abShown[i])
		{
			if (bFirst || (*apfX[i] - asSizes[i].fHeight / 2 < fMinX))
			{
				fMinX = *apfX[i] - asSizes[i].fHeight / 2;
			}
			if (bFirst || (*apfX[i] + asSizes[i].fHeight / 2 > fMaxX))
			{
				fMaxX = *apfX[i] + asSizes[i].fHeight / 2;
			}
			if (bFirst || (asSizes[i].fWidth / 2 + asSizes[i].fHeight / 4 > fMaxY))
			{
				fMaxY = asSizes[i].fWidth / 2 + asSizes[i].fHeight / 4;
			}
			bFirst = false;
		}
	}

	//Turned right into R.
	iLeftR = -(int)ceil(fMaxY + 0.4);
	iRightR = -(int)floor(-fMaxY - 0.4);
	iTopR = (int)floor(fMinX);
	iBottomR = (int)ceil(fMaxX);

	sTop.Init(pcInstanceCircuitView, PA_Top);
	sBottom.Init(pcInstanceCircuitView, PA_Bottom);
	sLeft.Init(pcInstanceCircuitView, PA_Left);
	sRight.Init(pcInstanceCircuitView, PA_Right);

	//LeftRightPinViewOffsets.
	iMaxGridWidth = CalculateMaxGridSize(sTop.iExtendedSize, sBottom.iExtendedSize);
	iTopR = IntMin(iTopR, (int)ceil(iTopR - MaxPinLabelWidth(pcFonts, &sTop)));
	iBottomR = IntMax(iBottomR, (int)floor(iBottomR + MaxPinLabelWidth(pcFonts, &sBottom)));
	iLeftR = IntMin(iLeftR, -(iMaxGridWidth / 2));
	iRightR = IntMax(iRightR, iMaxGridWidth - iMaxGridWidth / 2);

	//TopBottomPinViewOffsets.
	iTop = iTopR;
	iBottom = iBottomR;
	FitLeftRightPins(&sLeft, &sRight, &iTop, &iBottom);
	iTopR = IntMin(iTopR, iTop);
	iBottomR = IntMax(iBottomR, iBottom);

	//The left side's pins go on R's right and the right side's on R's left.
	iRightR = IntMax(iRightR, (int)ceil(iRightR + MaxPinLabelWidth(pcFonts, &sLeft)));
	iLeftR = IntMin(iLeftR, (int)floor(iLeftR - MaxPinLabelWidth(pcFonts, &sRight)));

	sRight.Kill();
	sLeft.Kill();
	sBottom.Kill();
	sTop.Kill();

	psLayout->sBoxMinimum.Init(-iRightR, iTopR);
	psLayout->sBoxMaximum.Init(-iLeftR, iBottomR);
}


//////////////////////////////////////////////////////////////////////////
//
// The shapes and port views from the kept layout and the subcircuit's
// pins.  The pins' offsets along their sides only depend on how many there
// are and the box, so they come out where Logicim put them when it was
// measured.
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceView::LayOut(void)
{
	SPinSide			sTop;
	SPinSide			sBottom;
	SPinSide			sLeft;
	SPinSide			sRight;
	int					iMaxGridWidth;
	int					iLeftR;
	int					iTopR;
	int					iRightR;
	int					iBottomR;
	int					iTop;
	int					iBottom;
	size				uiShape;

	iLeftR = -msLayout.sBoxMaximum.x;
	iRightR = -msLayout.sBoxMinimum.x;
	iTopR = msLayout.sBoxMinimum.y;
	iBottomR = msLayout.sBoxMaximum.y;

	mcShapes.AddRectangle(msLayout.sBoxMinimum.x, msLayout.sBoxMinimum.y, msLayout.sBoxMaximum.x, msLayout.sBoxMaximum.y, true, true);

	sTop.Init(mpcInstanceCircuitView, PA_Top);
	sBottom.Init(mpcInstanceCircuitView, PA_Bottom);
	sLeft.Init(mpcInstanceCircuitView, PA_Left);
	sRight.Init(mpcInstanceCircuitView, PA_Right);

	iMaxGridWidth = CalculateMaxGridSize(sTop.iExtendedSize, sBottom.iExtendedSize);
	AddSidePins(&sTop, -(iMaxGridWidth / 2), iMaxGridWidth - iMaxGridWidth / 2, iTopR, 0, -1, TA_Left, 1);
	AddSidePins(&sBottom, -(iMaxGridWidth / 2), iMaxGridWidth - iMaxGridWidth / 2, iBottomR, 0, 1, TA_Right, 1);

	iTop = iTopR;
	iBottom = iBottomR;
	FitLeftRightPins(&sLeft, &sRight, &iTop, &iBottom);
	AddSidePins(&sLeft, iTop, iBottom, -iRightR, -1, 0, TA_Left, 0);
	AddSidePins(&sRight, iTop, iBottom, -iLeftR, 1, 0, TA_Right, 0);

	sRight.Kill();
	sLeft.Kill();
	sBottom.Kill();
	sTop.Kill();

	mcShapes.AddText(0, msLayout.fTypeY, mszTypeName.Text(), CF_SansSerifBold, SUBCIRCUIT_TYPE_FONT_SIZE, TA_Centre);
	if (!mszComment.Empty())
	{
		uiShape = mcShapes.NumShapes();
		mcShapes.AddText(0, msLayout.fCommentY, mszComment.Text(), CF_SansSerif, SUBCIRCUIT_TEXT_FONT_SIZE, TA_Centre);
		mcShapes.SetBorder(uiShape, SC_CommentText);
	}
	if (!mszName.Empty())
	{
		mcShapes.AddText(0, msLayout.fNameY, mszName.Text(), CF_SansSerif, SUBCIRCUIT_TEXT_FONT_SIZE, TA_Centre);
	}
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// The side's pins between iOffset and iOpposite along it (Logicim's
// offsets), on the box's edge at iEdge across it, their ports facing out
// along (iOutX, iOutY).  Top and bottom pins run right to left along x
// and their edge is a y; left and right pins run down y and their edge is
// an x.
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceView::AddSidePins(SPinSide* psSide, int iOffset, int iOpposite, int iEdge, int iOutX, int iOutY, ETextAlignment eAlignment, int iRightRotations)
{
	CArrayPinOffset		asOffsets;
	size				i;
	SPinOffset*			psOffset;

	asOffsets.Init();
	CalculatePinOffsets(&asOffsets, psSide, iOffset, iOpposite);
	for (i = 0; i < asOffsets.NumElements(); i++)
	{
		psOffset = asOffsets.Get(i);
		if (iOutY != 0)
		{
			AddPin(psOffset->pcPin, -psOffset->iOffset, iEdge, iOutX, iOutY, eAlignment, iRightRotations);
		}
		else
		{
			AddPin(psOffset->pcPin, iEdge, psOffset->iOffset, iOutX, iOutY, eAlignment, iRightRotations);
		}
	}
	asOffsets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A pin at (x, y) on the box's edge, as Logicim's SubcircuitPinView: its
// port a grid unit out along (iOutX, iOutY), a line or an inverting
// bubble out to it, its name half a unit in (turned right
// iRightRotations times) and, if it has one, a clock notch on the edge
// pointing in.
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceView::AddPin(CPinView* pcPin, int32 x, int32 y, int iOutX, int iOutY, ETextAlignment eAlignment, int iRightRotations)
{
	SFloat64Vec2			asNotch[3];
	SFloat64Vec2			sAlong;
	int32					iPortX;
	int32					iPortY;
	float64					fTextX;
	float64					fTextY;
	size					i;
	SComponentShape*		psShape;
	CArrayChars				aszPortNames;
	CChars					szPortName;
	SSubcircuitPinView*		psPin;

	iPortX = x + iOutX;
	iPortY = y + iOutY;
	fTextX = x - 0.5 * iOutX;
	fTextY = y - 0.5 * iOutY;

	mcShapes.AddText(fTextX, fTextY, pcPin->GetName(), CF_SansSerif, SUBCIRCUIT_TEXT_FONT_SIZE, eAlignment, iRightRotations);
	if (pcPin->IsInverting())
	{
		mcShapes.AddCircle(x + 0.5 * iOutX, y + 0.5 * iOutY, 0.5, false, true);
	}
	else
	{
		mcShapes.AddLine(iPortX, iPortY, x, y);
	}
	if (pcPin->IsClockNotch())
	{
		if (iOutY != 0)
		{
			sAlong.Init(0.5, 0);
		}
		else
		{
			sAlong.Init(0, -0.5);
		}
		asNotch[0].Init(x + sAlong.x, y + sAlong.y);
		asNotch[1].Init(fTextX, fTextY);
		asNotch[2].Init(x - sAlong.x, y - sAlong.y);
		psShape = mcShapes.AddPolygon(asNotch, 3, false, true);
		psShape->fLineWidth = PORT_CLOCK_NOTCH_WIDTH;
	}

	aszPortNames.Init();
	for (i = 0; i < (size)pcPin->GetBitWidth(); i++)
	{
		szPortName.Init(pcPin->GetName());
		szPortName.Append(' ');
		szPortName.Append(i);
		aszPortNames.Add(szPortName.Text());
		szPortName.Kill();
	}
	psPin = masPins.Add();
	psPin->pcPin = pcPin;
	psPin->pcPortView = AddPortView(&aszPortNames, iPortX, iPortY);
	aszPortNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CSubcircuitInstanceView::GetType(void)
{
	return "Subcircuit";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CSubcircuitInstanceView::GetXMLName(void)
{
	return "Subcircuit";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CComponentView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("type", mszTypeName.Text());
	if (!mszComment.Empty())
	{
		pcWriter->Attribute("comment", mszComment.Text());
	}
	SaveXMLBox(pcWriter, msLayout.sBoxMinimum, msLayout.sBoxMaximum);
	pcWriter->Attribute("typeY", msLayout.fTypeY);
	if (!mszComment.Empty())
	{
		pcWriter->Attribute("commentY", msLayout.fCommentY);
	}
	if (!mszName.Empty())
	{
		pcWriter->Attribute("nameY", msLayout.fNameY);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The subcircuit is set afterwards, from the type.
//
//////////////////////////////////////////////////////////////////////////
bool CSubcircuitInstanceView::LoadXML(CMarkupTag* pcTag)
{
	int32						x;
	int32						y;
	ERotation					eRotation;
	char*						szName;
	char*						szTypeName;
	char*						szComment;
	SSubcircuitInstanceLayout	sLayout;

	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szName))
	{
		return false;
	}
	szTypeName = pcTag->GetStringAttribute("type");
	if (StrEmpty(szTypeName))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a type.", NULL);
	}
	szComment = pcTag->GetStringAttribute("comment");
	if (szComment == NULL)
	{
		szComment = (char*)"";
	}
	if (!LoadXMLBox(pcTag, &sLayout.sBoxMinimum, &sLayout.sBoxMaximum))
	{
		return false;
	}
	sLayout.fCommentY = 0;
	sLayout.fNameY = 0;
	if (!pcTag->GetFloat64Attribute("typeY", &sLayout.fTypeY) ||
		(!StrEmpty(szComment) && !pcTag->GetFloat64Attribute("commentY", &sLayout.fCommentY)) ||
		(!StrEmpty(szName) && !pcTag->GetFloat64Attribute("nameY", &sLayout.fNameY)))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs typeY, and commentY and nameY for its comment and name.", NULL);
	}

	Init(x, y, eRotation, szName, szComment, szTypeName, &sLayout);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's createComponent: a subcircuit instance in the path's
// subcircuit simulation with trace ports for every pin of the subcircuit
// (shown on the instance or not), and the subcircuit simulation its
// subcircuit is made in for the path through this instance.
//
//////////////////////////////////////////////////////////////////////////
CCircuitComponent* CSubcircuitInstanceView::CreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CFamilyVoltageConfigurationStore* pcFamilies)
{
	CSubcircuitSimulation*			pcSimulation;
	CViewPath*						pcChild;
	CSubcircuitInstance*			pcInstance;
	CSubcircuitInstanceSimulation*	pcInstanceSimulation;
	CArrayTemplatePtr<CPinView>		apcPins;
	size							i;
	CPinView*						pcPin;
	bool							bResult;

	pcSimulation = CanCreateComponent(pcViewPath, pcCircuitSimulation);
	if (pcSimulation == NULL)
	{
		return NULL;
	}
	if (mpcInstanceCircuitView == NULL)
	{
		gcLogger.Error2(__METHOD__, " Instance [", mszName.Text(), "] has no subcircuit [", mszTypeName.Text(), "].", NULL);
		return NULL;
	}
	pcChild = pcViewPath->GetChild(this);
	if (pcChild == NULL)
	{
		return NULL;
	}

	pcInstance = NewMalloc<CSubcircuitInstance>();
	pcInstance->Init(pcSimulation, mszName.Text());
	apcPins.Init();
	GetCircuitPinViews(mpcInstanceCircuitView, &apcPins);
	bResult = true;
	for (i = 0; i < apcPins.NumElements(); i++)
	{
		pcPin = apcPins.GetPtr(i);
		if (pcInstance->AddTracePorts(pcPin->GetName(), (size)pcPin->GetBitWidth()) == NULL)
		{
			bResult = false;
		}
	}
	apcPins.Kill();
	if (!bResult)
	{
		return NULL;
	}

	pcInstanceSimulation = pcCircuitSimulation->AddInstanceSimulation(pcInstance);
	pcInstance->SetSubcircuitInstanceSimulation(pcInstanceSimulation);
	if (!pcChild->AddSubcircuitSimulation(pcInstanceSimulation))
	{
		return NULL;
	}
	if (!PostCreateComponent(pcViewPath, pcCircuitSimulation, pcInstance))
	{
		return NULL;
	}
	return pcInstance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCircuitInstanceView* CSubcircuitInstanceView::GetCircuitInstanceView(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL until the subcircuit is set.
//
//////////////////////////////////////////////////////////////////////////
CCircuitView* CSubcircuitInstanceView::GetInstanceCircuitView(void)
{
	return mpcInstanceCircuitView;
}


//////////////////////////////////////////////////////////////////////////
//
// The type name and, if it has one, the name in brackets.
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceView::AppendDescription(CChars* psz)
{
	psz->Append(mszTypeName);
	if (!mszName.Empty())
	{
		psz->Append(" (");
		psz->Append(mszName);
		psz->Append(")");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each shown pin's port view with the bus port view of the pin inside.
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceView::GetPinPortJoins(CArrayPinPortJoin* pasJoins)
{
	size				i;
	SSubcircuitPinView*	psPin;
	SPinPortJoin*		psJoin;

	for (i = 0; i < masPins.NumElements(); i++)
	{
		psPin = masPins.Get(i);
		psJoin = pasJoins->Add();
		psJoin->pcOutside = psPin->pcPortView;
		psJoin->pcInside = psPin->pcPin->GetPortView((size)0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CSubcircuitInstanceView::GetTypeName(void) { return mszTypeName.Text(); }
char* CSubcircuitInstanceView::GetComment(void) { return mszComment.Text(); }
SSubcircuitInstanceLayout* CSubcircuitInstanceView::GetLayout(void) { return &msLayout; }
size CSubcircuitInstanceView::NumPins(void) { return masPins.NumElements(); }
SSubcircuitPinView* CSubcircuitInstanceView::GetPin(size uiIndex) { return masPins.SafeGet(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// NULL if the pin isn't shown on the instance.
//
//////////////////////////////////////////////////////////////////////////
SSubcircuitPinView* CSubcircuitInstanceView::GetPin(const char* szPinName)
{
	size				i;
	SSubcircuitPinView*	psPin;

	for (i = 0; i < masPins.NumElements(); i++)
	{
		psPin = masPins.Get(i);
		if (StringCompare(psPin->pcPin->GetName(), szPinName) == 0)
		{
			return psPin;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The subcircuit it's an instance of, its name and its comment, as
// Logicim's SubcircuitInstanceProperties.  Its size comes from its
// subcircuit's pins.
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceView::GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	pcProperties->AddText("subcircuitTypeName", mszTypeName.Text());
	CComponentView::GetProperties(pcProperties, psContext);
	pcProperties->AddText("comment", mszComment.Text());
}


//////////////////////////////////////////////////////////////////////////
//
// An instance of the subcircuit named, which must be one of the project's,
// measured again with the fonts.  A subcircuit that holds the circuit the
// instance is in mustn't be named (see CCircuitEditor::SetProperties).
//
//////////////////////////////////////////////////////////////////////////
bool CSubcircuitInstanceView::InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	CCircuitView*	pcSubcircuit;
	const char*		szTypeName;

	szTypeName = pcProperties->GetText("subcircuitTypeName", "");
	pcSubcircuit = psContext->pcCircuits->Get(szTypeName);
	if (pcSubcircuit == NULL)
	{
		return gcLogger.Error2(__METHOD__, " There is no subcircuit [", szTypeName, "].", NULL);
	}
	Init(x, y, eRotation, pcProperties->GetText("name", ""), pcProperties->GetText("comment", ""), pcSubcircuit, psContext->pcFonts);
	return true;
}
