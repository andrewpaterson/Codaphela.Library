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
#include "InputVirtualDeviceDesc.h"
#include "InputSourceDesc.h"
#include "InputVirtualSourceDesc.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualSourceDesc::Init(CInputVirtualDeviceDesc* pcVirtualDeviceDesc, CInputSourceDesc* pcSourceDesc, int iDescriptionID)
{
	mpcVirtualDeviceDesc = pcVirtualDeviceDesc;
	mpcSourceDesc = pcSourceDesc;
	miDescriptionID = iDescriptionID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualSourceDesc::Kill(void)
{
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDeviceDesc* CInputVirtualSourceDesc::GetVirtualDeviceDesc(void)
{
	return mpcVirtualDeviceDesc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CInputVirtualSourceDesc::GetDescriptionID(void)
{
	return miDescriptionID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CInputVirtualSourceDesc::GetSourceDesc(void)
{
	return mpcSourceDesc;
}

