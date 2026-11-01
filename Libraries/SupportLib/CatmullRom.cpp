/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "CatmullRom.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void GetCatmullRomWeights(float64 fFraction, float64* pafWeights)
{
	float64	t;

	t = fFraction;
	pafWeights[0] = ((-t + 2.0) * t - 1.0) * t * 0.5;
	pafWeights[1] = ((3.0 * t - 5.0) * t * t + 2.0) * 0.5;
	pafWeights[2] = ((-3.0 * t + 4.0) * t + 1.0) * t * 0.5;
	pafWeights[3] = (t - 1.0) * t * t * 0.5;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CatmullRom(float64 fBefore, float64 fAt, float64 fAfter, float64 fTwoAfter, float64 fFraction)
{
	float64	afWeights[4];

	GetCatmullRomWeights(fFraction, afWeights);
	return fBefore * afWeights[0] + fAt * afWeights[1] + fAfter * afWeights[2] + fTwoAfter * afWeights[3];
}

