/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "BaseLib/TypeNames.h"
#include "ChannelPadder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelPadder::Init(CChannels* pcExternal)
{
	mpcExternal = pcExternal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelPadder::OptimalPadding(void)
{
	EPrimitiveType			eType;
	size					iMaxSize;
	CChannel*				pcChannel;
	size					i;
	int						iPos;
	int						iAvail;
	int						iClassSize;
	int						iPad;
	CArrayChannelOffset*	pacChannelOffsets;
	size					uiChannelOffsets;

	if (!mpcExternal->IsOnlyBasicTypes())
	{
		return false;
	}

	eType = mpcExternal->GetLargestPrimitiveType();
	iMaxSize = gcTypeNames.GetByteSize(eType);
	iPos = 0;
	iAvail = iMaxSize;
	pacChannelOffsets = mpcExternal->GetChannelOffsets();
	uiChannelOffsets = pacChannelOffsets->NumElements();
	for (i = 0; i < uiChannelOffsets; i++)
	{
		pcChannel = pacChannelOffsets->Get(i);
		iClassSize = (int)pcChannel->miByteSize;
		if (iAvail != iMaxSize)
		{
			if (iAvail >= iClassSize)
			{
				iPad = iAvail % iClassSize;
				iPos += iPad;
				iAvail -= iPad;
			}
			else
			{
				iPos += iAvail;
				iAvail = iMaxSize;
			}
		}

		pcChannel->miByteOffset = (size)iPos;
		pcChannel->miBitOffset = (size)(iPos * 8);

		iPos += iClassSize;
		iAvail -= iClassSize;
		if (iAvail == 0)
		{
			iAvail = iMaxSize;
		}
	}

	if (iAvail != iMaxSize)
	{
		mpcExternal->SetByteStride(iPos + iAvail);
	}
	else
	{
		mpcExternal->SetByteStride(iPos);
	}
	mpcExternal->SetBitStrideFromByteStride();
	return true;
}

