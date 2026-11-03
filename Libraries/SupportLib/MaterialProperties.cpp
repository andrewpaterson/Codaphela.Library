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
	mcDiffuseColour.Zero();
	mcAmbientColour.Zero();
	mcEmissiveColour.Zero();
	mcSpecularColour.Zero();
	mfShininessPower = 0.0f;
	mfShininessBase = 0.0f;
	mbLit = false;
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
	ReturnOnFalse(mcDiffuseColour.Save(pcFile));
	ReturnOnFalse(mcAmbientColour.Save(pcFile));
	ReturnOnFalse(mcEmissiveColour.Save(pcFile));
	ReturnOnFalse(mcSpecularColour.Save(pcFile));
	ReturnOnFalse(pcFile->WriteFloat(mfShininessPower));
	ReturnOnFalse(pcFile->WriteFloat(mfShininessBase));
	ReturnOnFalse(pcFile->WriteBool(mbLit));
	ReturnOnFalse(pcFile->WriteSInt(meOpacity));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMaterialProperties::Load(CFileReader* pcFile)
{
	ReturnOnFalse(mcDiffuseColour.Load(pcFile));
	ReturnOnFalse(mcAmbientColour.Load(pcFile));
	ReturnOnFalse(mcEmissiveColour.Load(pcFile));
	ReturnOnFalse(mcSpecularColour.Load(pcFile));
	ReturnOnFalse(pcFile->ReadFloat(&mfShininessPower));
	ReturnOnFalse(pcFile->ReadFloat(&mfShininessBase));
	ReturnOnFalse(pcFile->ReadBool(&mbLit));
	ReturnOnFalse(pcFile->ReadSInt((int*)&meOpacity));
	return true;
}
