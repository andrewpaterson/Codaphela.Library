/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "MaterialProperties.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMaterialProperties::Init(void)
{
	sDiffuseColour.Zero();
	sAmbientColour.Zero();
	sEmissiveColour.Zero();
	sSpecularColour.Zero();
	fShininessPower = 0.0f;
	fShininessBase = 0.0f;
	bLit = false;
	meOpacity = MO_Opaque;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMaterialProperties::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMaterialProperties::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(sDiffuseColour.Save(pcFile));
	ReturnOnFalse(sAmbientColour.Save(pcFile));
	ReturnOnFalse(sEmissiveColour.Save(pcFile));
	ReturnOnFalse(sSpecularColour.Save(pcFile));
	ReturnOnFalse(pcFile->WriteFloat(fShininessPower));
	ReturnOnFalse(pcFile->WriteFloat(fShininessBase));
	ReturnOnFalse(pcFile->WriteBool(bLit));
	ReturnOnFalse(pcFile->WriteInt(meOpacity));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMaterialProperties::Load(CFileReader* pcFile)
{
	ReturnOnFalse(sDiffuseColour.Load(pcFile));
	ReturnOnFalse(sAmbientColour.Load(pcFile));
	ReturnOnFalse(sEmissiveColour.Load(pcFile));
	ReturnOnFalse(sSpecularColour.Load(pcFile));
	ReturnOnFalse(pcFile->ReadFloat(&fShininessPower));
	ReturnOnFalse(pcFile->ReadFloat(&fShininessBase));
	ReturnOnFalse(pcFile->ReadBool(&bLit));
	ReturnOnFalse(pcFile->ReadInt((int*)&meOpacity));
	return true;
}
