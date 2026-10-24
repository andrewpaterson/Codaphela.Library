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
#ifndef __CIRCUIT_COLOURS_H__
#define __CIRCUIT_COLOURS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "SupportLib/ColourARGB32.h"
#include "MicroprocessorLib/Port.h"


#define NUM_VOLTAGE_COLOURS		71


//The colours a circuit is drawn in, as Logicim's Colours.  Voltages from 0 to 7 volts are coloured in tenths of a
//volt; anything outside that is an error.  InitLogicim matches Logicim's light editor, InitDark sits on the IntelliJ
//dark editor.
class CCircuitColours
{
public:
	ARGB32	msBackground;
	ARGB32	msSmallGridDot;
	ARGB32	msLargeGridDot;
	ARGB32	msTraceUndriven;
	ARGB32	msTraceError;
	ARGB32	masVoltage[NUM_VOLTAGE_COLOURS];
	ARGB32	masShort[NUM_VOLTAGE_COLOURS];
	ARGB32	msShapeBorder;
	ARGB32	msShapeFill;
	ARGB32	msDarkShapeFill;
	ARGB32	msPortHover;
	ARGB32	msViewHover;
	ARGB32	msSelected;
	ARGB32	msDisconnectedTrace;
	ARGB32	msDifferingBusTrace;
	ARGB32	msText;
	ARGB32	msCommentText;
	ARGB32	msHiddenText;

	void	InitLogicim(void);
	void	InitDark(void);
	void	Kill(void);

	ARGB32	GetTraceVoltage(float32 fVoltage);
	ARGB32	GetTraceShort(float32 fShortVoltage);
	ARGB32	GetColourForVoltage(float32 fVoltage);
	ARGB32	GetColourForTrace(CTrace* pcTrace, int64 iTime);
	ARGB32	GetColourForPorts(CArrayPortPtr* papcPorts, int64 iTime);

protected:
	void	GenerateGradient(ARGB32* asColours, ARGB32 sStart, int iStart, ARGB32 sEnd, int iEnd);
	ARGB32	AddShort(ARGB32 sVoltage, float32 fShortVoltage);
};


#endif // __CIRCUIT_COLOURS_H__
