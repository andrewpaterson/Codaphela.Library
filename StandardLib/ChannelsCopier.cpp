/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "Unknowns.h"
#include "ChannelsCopier.h"
#include "ChannelsAccessorCreator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsCopier::Init(CChannels* pcSource, CChannels* pcDest)
{
	CChannelsAccessorCreator	cCreator;

	//Now we have an accessor that takes what's in the source and converts it to the destinations format.
	//Although it may have gaps if the destination has channels the source does not.
	cCreator.Init(pcSource);
	cCreator.AddAccess(pcDest);
	mpcSourceAccessor = cCreator.CreateAndKill();

	//This looks a bit retarded but it creates an accessor capable of reading the source accesses output.
	cCreator.Init(pcDest);
	cCreator.AddAccess(mpcSourceAccessor);
	mpcDestAccessor = cCreator.CreateAndKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsCopier::Kill(void)
{
	mpcDestAccessor->Kill();
	mpcSourceAccessor->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsCopier::Copy(int iSourcePos, int iDestPos, int iLength)
{
	int		i;
	void*	pvData;
	int		iSourceSize;
	int		iDestSize;

	iSourceSize = mpcSourceAccessor->GetChannels()->GetSize();
	if (iSourcePos + iLength > iSourceSize)
	{
		iLength = iSourceSize - iSourcePos;
	}

	iDestSize = mpcDestAccessor->GetChannels()->GetSize();
	if (iDestPos + iLength > iDestSize)
	{
		iLength = iDestSize - iDestPos;
	}

	for (i = 0; i < iLength; i++)
	{
		pvData = mpcSourceAccessor->Get(iSourcePos + i);
		mpcDestAccessor->Set(iDestPos + i, pvData);
	}
}

