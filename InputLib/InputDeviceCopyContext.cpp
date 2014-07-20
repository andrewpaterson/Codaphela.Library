/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "InputDeviceCopyContext.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceCopyContext::Init(CInputDeviceDesc* pcSourceDeviceDesc, CInputDeviceDesc* pcDestDeviceDesc)
{
	mpcSourceDeviceDesc = pcSourceDeviceDesc;
	mpcDestDeviceDesc = pcDestDeviceDesc;

	mmppSources.Init(&gcSystemAllocator, 16);
	mmppChords.Init(&gcSystemAllocator, 16);
	mmppValues.Init(&gcSystemAllocator, 16);
	mmppVariables.Init(&gcSystemAllocator, 16);
	mmppEvaluators.Init(&gcSystemAllocator, 16);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceCopyContext::Kill(void)
{
	mmppEvaluators.Kill();
	mmppVariables.Kill();
	mmppValues.Kill();
	mmppChords.Kill();
	mmppSources.Kill();
}

