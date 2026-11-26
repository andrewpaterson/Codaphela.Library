/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SCHMITT_THRESHOLDS_H__
#define __SCHMITT_THRESHOLDS_H__
#include "BaseLib/PrimitiveTypes.h"


//The input thresholds of the Schmitt trigger parts a logic family has: rising, an input switches at VT+, falling at
//VT-, from each part's datasheet (HCP65/Datasheets): typical figures where it gives them, else the middle of what its
//minimums, maximums and hysteresis allow.  A part is a 74 series part type in its family as a part number has it
//("14", "1G17").  Between the datasheet's VCCs the thresholds are interpolated, beyond them scaled with
//VCC.  An input's unsettled band, between VIL and VIH, already ignores a slow input until it passes the threshold on
//its way, so a Schmitt part is its family with VIL VT- and VIH VT+ (CFamilyVoltageConfigurationStore::GetPart), taken
//at each VCC the family has figures for; between those the family interpolates, as it does its other figures.
struct SSchmittThreshold
{
	const char*		szFamily;
	const char*		szPart;
	float32			fVcc;
	float32			fPositive;  //VT+.
	float32			fNegative;  //VT-.
};


bool	IsSchmittPart(const char* szFamily, const char* szPart);
bool	GetSchmittThresholds(const char* szFamily, const char* szPart, float32 fVcc, float32* pfPositive, float32* pfNegative);


#endif // __SCHMITT_THRESHOLDS_H__

