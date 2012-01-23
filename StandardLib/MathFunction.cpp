/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "MathFunction.h"


void CMathFunction::Init(void)
{
	mathFunc = NULL;
	pvData = NULL;
}


void CMathFunction::Kill(void)
{
	if (pvData)
	{
		free(pvData);
		pvData = NULL;
	}
	mathFunc = NULL;
}


void* CMathFunction::AllocateData(int iType, int iSize)
{
	return UnknownTypeAllocate(iSize, iType);
}


void CMathFunction::SetFunction(MathFunc func)
{
	mathFunc = func;
}


void CMathFunction::SetData(void* pvData)
{
	this->pvData = pvData;
}


float SpringFunction(float fDistance, SSpringConstants* pvData)
{
	return fDistance * pvData->K;
}

