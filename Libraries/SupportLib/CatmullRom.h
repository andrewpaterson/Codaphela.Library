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
#ifndef __CATMULL_ROM_H__
#define __CATMULL_ROM_H__
#include "BaseLib/PrimitiveTypes.h"


//The Catmull-Rom cubic (Keys, a = -0.5) used by the sound and image resamplers.  The weights are for the samples one before,
//at, one after and two after the position; fFraction (0 to 1) is how far the position is past the sample at it.
void	GetCatmullRomWeights(float64 fFraction, float64* pafWeights);
float64	CatmullRom(float64 fBefore, float64 fAt, float64 fAfter, float64 fTwoAfter, float64 fFraction);


#endif // __CATMULL_ROM_H__

