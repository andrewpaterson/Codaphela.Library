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
#include "MicroprocessorLib/Trace.h"
#include "CircuitColours.h"


//////////////////////////////////////////////////////////////////////////
//
// Logicim's own colours, on a white editor.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitColours::InitLogicim(void)
{
	msBackground = Set32BitColour((uint8)255, (uint8)255, (uint8)255);
	msSmallGridDot = Set32BitColour((uint8)0xCB, (uint8)0xCB, (uint8)0xCB);
	msLargeGridDot = Set32BitColour((uint8)0xAB, (uint8)0xAB, (uint8)0xAB);
	msTraceUndriven = Set32BitColour((uint8)110, (uint8)90, (uint8)90);
	msTraceError = Set32BitColour((uint8)255, (uint8)0, (uint8)100);

	GenerateGradient(masVoltage, Set32BitColour((uint8)0, (uint8)50, (uint8)0), 0, Set32BitColour((uint8)0, (uint8)180, (uint8)0), 33);
	GenerateGradient(masVoltage, Set32BitColour((uint8)0, (uint8)180, (uint8)0), 33, Set32BitColour((uint8)100, (uint8)255, (uint8)100), 50);
	GenerateGradient(masVoltage, Set32BitColour((uint8)100, (uint8)255, (uint8)100), 50, Set32BitColour((uint8)255, (uint8)253, (uint8)75), 70);
	GenerateGradient(masShort, Set32BitColour((uint8)0, (uint8)0, (uint8)0), 0, Set32BitColour((uint8)200, (uint8)0, (uint8)0), 33);
	GenerateGradient(masShort, Set32BitColour((uint8)200, (uint8)0, (uint8)0), 33, Set32BitColour((uint8)255, (uint8)255, (uint8)100), 50);
	GenerateGradient(masShort, Set32BitColour((uint8)255, (uint8)255, (uint8)100), 50, Set32BitColour((uint8)255, (uint8)255, (uint8)200), 70);

	msShapeBorder = Set32BitColour((uint8)0, (uint8)0, (uint8)0);
	msShapeFill = Set32BitColour((uint8)232, (uint8)232, (uint8)232);
	msDarkShapeFill = Set32BitColour((uint8)170, (uint8)170, (uint8)170);
	msPortHover = Set32BitColour((uint8)0, (uint8)255, (uint8)0);
	msViewHover = Set32BitColour((uint8)0, (uint8)255, (uint8)0);
	msSelected = Set32BitColour((uint8)255, (uint8)255, (uint8)255);
	msDisconnectedTrace = Set32BitColour((uint8)0, (uint8)170, (uint8)250);
	msDifferingBusTrace = Set32BitColour((uint8)20, (uint8)230, (uint8)180);
	msText = Set32BitColour((uint8)0, (uint8)0, (uint8)0);
}


//////////////////////////////////////////////////////////////////////////
//
// On the IntelliJ dark editor (JEWEL_EDITOR).  Shapes are light outlines
// on a fill a little lighter than the editor.  Low voltages start brighter
// than Logicim's so they show on the dark background.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitColours::InitDark(void)
{
	msBackground = Set32BitColour((uint8)0x1E, (uint8)0x1F, (uint8)0x22);
	msSmallGridDot = Set32BitColour((uint8)0x43, (uint8)0x45, (uint8)0x4A);
	msLargeGridDot = Set32BitColour((uint8)0x6F, (uint8)0x73, (uint8)0x7A);
	msTraceUndriven = Set32BitColour((uint8)150, (uint8)125, (uint8)125);
	msTraceError = Set32BitColour((uint8)255, (uint8)40, (uint8)120);

	GenerateGradient(masVoltage, Set32BitColour((uint8)30, (uint8)95, (uint8)30), 0, Set32BitColour((uint8)0, (uint8)180, (uint8)0), 33);
	GenerateGradient(masVoltage, Set32BitColour((uint8)0, (uint8)180, (uint8)0), 33, Set32BitColour((uint8)100, (uint8)255, (uint8)100), 50);
	GenerateGradient(masVoltage, Set32BitColour((uint8)100, (uint8)255, (uint8)100), 50, Set32BitColour((uint8)255, (uint8)253, (uint8)75), 70);
	GenerateGradient(masShort, Set32BitColour((uint8)0, (uint8)0, (uint8)0), 0, Set32BitColour((uint8)200, (uint8)0, (uint8)0), 33);
	GenerateGradient(masShort, Set32BitColour((uint8)200, (uint8)0, (uint8)0), 33, Set32BitColour((uint8)255, (uint8)255, (uint8)100), 50);
	GenerateGradient(masShort, Set32BitColour((uint8)255, (uint8)255, (uint8)100), 50, Set32BitColour((uint8)255, (uint8)255, (uint8)200), 70);

	msShapeBorder = Set32BitColour((uint8)0xCE, (uint8)0xD0, (uint8)0xD6);
	msShapeFill = Set32BitColour((uint8)0x39, (uint8)0x3B, (uint8)0x40);
	msDarkShapeFill = Set32BitColour((uint8)0x5A, (uint8)0x5D, (uint8)0x63);
	msPortHover = Set32BitColour((uint8)0x46, (uint8)0x82, (uint8)0xFA);
	msViewHover = Set32BitColour((uint8)0x46, (uint8)0x82, (uint8)0xFA);
	msSelected = Set32BitColour((uint8)0x35, (uint8)0x74, (uint8)0xF0);
	msDisconnectedTrace = Set32BitColour((uint8)0, (uint8)170, (uint8)250);
	msDifferingBusTrace = Set32BitColour((uint8)20, (uint8)230, (uint8)180);
	msText = Set32BitColour((uint8)0xDF, (uint8)0xE1, (uint8)0xE5);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitColours::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's ColourGradient; channels are truncated towards the start.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitColours::GenerateGradient(ARGB32* asColours, ARGB32 sStart, int iStart, ARGB32 sEnd, int iEnd)
{
	int			i;
	float32		f;
	uint32		auiStart[4];
	uint32		auiEnd[4];
	uint32		auiColour[4];
	int			j;

	Get32BitColour(&auiStart[0], &auiStart[1], &auiStart[2], &auiStart[3], sStart);
	Get32BitColour(&auiEnd[0], &auiEnd[1], &auiEnd[2], &auiEnd[3], sEnd);
	for (i = iStart; i <= iEnd; i++)
	{
		f = (float32)(i - iStart) / (float32)(iEnd - iStart);
		for (j = 0; j < 4; j++)
		{
			auiColour[j] = (uint32)((int)auiStart[j] + (int)((float32)((int)auiEnd[j] - (int)auiStart[j]) * f));
		}
		asColours[i] = Set32BitColour(auiColour[0], auiColour[1], auiColour[2], auiColour[3]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CCircuitColours::GetTraceVoltage(float32 fVoltage)
{
	int		iIndex;

	iIndex = (int)(fVoltage * 10);
	if (iIndex >= NUM_VOLTAGE_COLOURS)
	{
		iIndex = NUM_VOLTAGE_COLOURS - 1;
	}
	if (iIndex < 0)
	{
		iIndex = 0;
	}
	return masVoltage[iIndex];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CCircuitColours::GetTraceShort(float32 fShortVoltage)
{
	int		iIndex;

	if ((fShortVoltage > 7.0f) || (fShortVoltage <= 0.0f))
	{
		return msTraceError;
	}
	iIndex = (int)(fShortVoltage * 10);
	if (iIndex >= NUM_VOLTAGE_COLOURS)
	{
		iIndex = NUM_VOLTAGE_COLOURS - 1;
	}
	return masShort[iIndex];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CCircuitColours::GetColourForVoltage(float32 fVoltage)
{
	if ((fVoltage < 0.0f) || (fVoltage > 7.0f))
	{
		return msTraceError;
	}
	return GetTraceVoltage(fVoltage);
}


//////////////////////////////////////////////////////////////////////////
//
// A shorted trace adds the short's colour to the voltage's, clamped.
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CCircuitColours::AddShort(ARGB32 sVoltage, float32 fShortVoltage)
{
	uint32	auiVoltage[4];
	uint32	auiShort[4];
	int		j;

	Get32BitColour(&auiVoltage[0], &auiVoltage[1], &auiVoltage[2], &auiVoltage[3], sVoltage);
	Get32BitColour(&auiShort[0], &auiShort[1], &auiShort[2], &auiShort[3], GetTraceShort(fShortVoltage));
	for (j = 0; j < 3; j++)
	{
		auiVoltage[j] += auiShort[j];
		if (auiVoltage[j] > 255)
		{
			auiVoltage[j] = 255;
		}
	}
	return Set32BitColour(auiVoltage[0], auiVoltage[1], auiVoltage[2], (uint32)255);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CCircuitColours::GetColourForTrace(CTrace* pcTrace, int64 iTime)
{
	float32		fVoltage;
	float32		fShortVoltage;

	if ((pcTrace == NULL) || (iTime == -1))
	{
		return msDisconnectedTrace;
	}

	fVoltage = pcTrace->GetVoltage(iTime);
	if (isnan(fVoltage))
	{
		return msTraceUndriven;
	}

	fShortVoltage = pcTrace->GetShortVoltage(iTime);
	if (fShortVoltage == 0)
	{
		return GetColourForVoltage(fVoltage);
	}
	return AddShort(GetColourForVoltage(fVoltage), fShortVoltage);
}


//////////////////////////////////////////////////////////////////////////
//
// The average voltage the ports see; a port not on a trace counts what it
// drives itself.
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CCircuitColours::GetColourForPorts(CArrayPortPtr* papcPorts, int64 iTime)
{
	size		i;
	CPort*		pcPort;
	CTrace*		pcTrace;
	float32		fVoltage;
	float32		fAverage;
	size		uiDriven;

	if ((papcPorts == NULL) || (papcPorts->NumElements() == 0) || (iTime == -1))
	{
		return msDisconnectedTrace;
	}

	fAverage = 0;
	uiDriven = 0;
	for (i = 0; i < papcPorts->NumElements(); i++)
	{
		pcPort = papcPorts->GetPtr(i);
		pcTrace = pcPort->GetTrace();
		if (pcTrace != NULL)
		{
			fVoltage = pcTrace->GetVoltage(iTime);
		}
		else
		{
			fVoltage = pcPort->GetVoltageOut(iTime);
		}

		if (!isnan(fVoltage))
		{
			fAverage += fVoltage;
			uiDriven++;
		}
	}

	if (uiDriven == 0)
	{
		return msTraceUndriven;
	}
	return GetColourForVoltage(fAverage / (float32)uiDriven);
}

