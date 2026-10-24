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
#include "MicroprocessorLib/Pins.h"
#include "ComponentShapes.h"
#include "LogicGateView.h"


//////////////////////////////////////////////////////////////////////////
//
// A negated gate has a bubble on its output.  Subclasses add their shapes,
// then the port views, then CalculateBounds.
//
//////////////////////////////////////////////////////////////////////////
void CLogicGateView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iInputCount, int iInputWidth, bool bNegated)
{
	CIntegratedCircuitView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts);
	miInputCount = (iInputCount < 1) ? 1 : iInputCount;
	miInputWidth = (iInputWidth < 1) ? 1 : iInputWidth;
	mbNegated = bNegated;
}


//////////////////////////////////////////////////////////////////////////
//
// -n to n for 2n + 1 inputs and -n to n without 0 for 2n.
//
//////////////////////////////////////////////////////////////////////////
void CLogicGateView::CalculatePortOffsets(CArrayInt* paiOffsets, int iInputCount)
{
	int		iEnd;
	int		i;
	bool	bSkipZero;

	iEnd = iInputCount / 2;
	bSkipZero = (iInputCount % 2) == 0;
	for (i = -iEnd; i <= iEnd; i++)
	{
		if (!((i == 0) && bSkipZero))
		{
			paiOffsets->Add(i);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Half the gate's width.
//
//////////////////////////////////////////////////////////////////////////
float64 CLogicGateView::CalculateWidth(int iInputCount)
{
	if (iInputCount <= 3)
	{
		return 1.5;
	}
	return 1.5 + ((iInputCount / 2) - 1);
}


//////////////////////////////////////////////////////////////////////////
//
// The names of the ports of input (or output) iPortNumber.  A gate with a
// single port calls it szName alone, as the pins do.
//
//////////////////////////////////////////////////////////////////////////
void CLogicGateView::GetPortNames(CArrayChars* paszNames, const char* szName, int iPortNumber, int iInputWidth, int iInputCount)
{
	int		i;
	CChars	szPortName;

	for (i = iPortNumber * iInputWidth; i < (iPortNumber + 1) * iInputWidth; i++)
	{
		szPortName.Init();
		CPins::CalculatePortName(&szPortName, szName, i, iInputWidth * iInputCount);
		paszNames->Add(&szPortName);
		szPortName.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogicGateView::CreateGatePortViews(int iInputOffset)
{
	CArrayInt		aiOffsets;
	CArrayChars		aszNames;
	size			i;
	CPortView*		pcOutput;

	CreatePowerPortViews();

	aiOffsets.Init();
	CalculatePortOffsets(&aiOffsets, miInputCount);
	for (i = 0; i < aiOffsets.NumElements(); i++)
	{
		aszNames.Init();
		GetPortNames(&aszNames, "Input", (int)i, miInputWidth, miInputCount);
		AddPortView(&aszNames, aiOffsets.GetValue(i), 1 + iInputOffset);
		aszNames.Kill();
	}
	aiOffsets.Kill();

	pcOutput = AddPortView("Output", 0, -2);
	if (mbNegated)
	{
		pcOutput->SetInverting(ROT_North);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A gate is three grid units wide at least, whatever its shape.
//
//////////////////////////////////////////////////////////////////////////
bool CLogicGateView::IncludeExtraBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool bHasBounds)
{
	if (!bHasBounds)
	{
		psMinimum->Init(-1.5, 0);
		psMaximum->Init(1.5, 0);
	}
	IncludeBoundsPoint(psMinimum, psMaximum, -1.5, 0);
	IncludeBoundsPoint(psMinimum, psMaximum, 1.5, 0);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CLogicGateView::GetInputCount(void) { return miInputCount; }
int CLogicGateView::GetInputWidth(void) { return miInputWidth; }
bool CLogicGateView::IsNegated(void) { return mbNegated; }

