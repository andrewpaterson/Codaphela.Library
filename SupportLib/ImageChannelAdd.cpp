/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "ImageChannelAdd.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageChannelAdd::Init(EChannel eChannel, EPrimitiveType eType, int iGroup)
{
	maiChannels.Init();
	meType = eType;
	miGroup = iGroup;
	AddChannel(eChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageChannelAdd::Kill(void)
{
	maiChannels.Kill();
	CImageModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageChannelAdd::AddChannel(EChannel eChannel)
{
	maiChannels.Add((int)eChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageChannelAdd::Modify(Ptr<CImage> pcImage)
{
	size		i;
	EChannel	eChannel;

	pcImage->BeginChange();

	for (i = 0; i < maiChannels.NumElements(); i++)
	{
		eChannel = (EChannel)maiChannels.GetValue(i);
		pcImage->AddChannel(eChannel, meType, miGroup);
	}
	
	pcImage->EndChange();

	return pcImage;
}

