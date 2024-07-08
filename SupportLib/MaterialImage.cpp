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
#include "MaterialImage.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMaterialImage::Init(void)
{
	memset(this, 0, sizeof(CMaterialImage));
	iImageID = -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMaterialImage::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMaterialImage::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteSInt(iImageID));
	ReturnOnFalse(pcFile->WriteSInt(eAddressU));
	ReturnOnFalse(pcFile->WriteSInt(eAddressV));
	ReturnOnFalse(pcFile->WriteSInt(eMagFilter));
	ReturnOnFalse(pcFile->WriteSInt(eMinFilter));
	ReturnOnFalse(pcFile->WriteSInt(eMipFilter));
	ReturnOnFalse(pcFile->WriteSInt(eUVSource));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMaterialImage::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadSInt(&iImageID));
	ReturnOnFalse(pcFile->ReadSInt((int*)&eAddressU));
	ReturnOnFalse(pcFile->ReadSInt((int*)&eAddressV));
	ReturnOnFalse(pcFile->ReadSInt((int*)&eMagFilter));
	ReturnOnFalse(pcFile->ReadSInt((int*)&eMinFilter));
	ReturnOnFalse(pcFile->ReadSInt((int*)&eMipFilter));
	ReturnOnFalse(pcFile->ReadSInt((int*)&eUVSource));
	return true;
}

