/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "KicadSchemNet.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNet::Init(void)
{
	mszName.Init();
	meNameSource = KSNNS_Pin;
	mapcSubnets.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNet::Kill(void)
{
	mapcSubnets.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNet::SetName(char* szName, EKicadSchemNetNameSource eSource)
{
	mszName.Set(szName);
	meNameSource = eSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemNet::AddSubnet(CKicadSchemSubnet* pcSubnet)
{
	mapcSubnets.Add(pcSubnet);
	pcSubnet->SetNet(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The pins in all of the net's subnets.
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemNet::NumPins(void)
{
	size	i;
	size	uiPins;

	uiPins = 0;
	for (i = 0; i < mapcSubnets.NumElements(); i++)
	{
		uiPins += mapcSubnets.GetPtr(i)->NumPins();
	}
	return uiPins;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CKicadSchemNet::GetName(void) { return mszName.Text(); }
EKicadSchemNetNameSource CKicadSchemNet::GetNameSource(void) { return meNameSource; }
size CKicadSchemNet::NumSubnets(void) { return mapcSubnets.NumElements(); }
CKicadSchemSubnet* CKicadSchemNet::GetSubnet(size uiIndex) { return mapcSubnets.SafeGetPtr(uiIndex); }

