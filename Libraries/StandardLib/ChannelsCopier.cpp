/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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

	//The source's channels that the dest has too, read in the dest's types (the dest's other channels are left).
	cCreator.Init(pcSource);
	cCreator.AddAccess(pcDest);
	mpcSourceAccessor = cCreator.CreateAndKill();

	//The dest is written from what that reads: the same channels, in the same types.
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
void CChannelsCopier::Copy(lsize uiSourcePos, lsize uiDestPos, lsize uiLength)
{
	lsize		i;
	void*		pvData;
	lsize		uiSourceSize;
	lsize		uiDestSize;

	uiSourceSize = mpcSourceAccessor->GetChannels()->GetSize();
	uiDestSize = mpcDestAccessor->GetChannels()->GetSize();
	if ((uiSourcePos >= uiSourceSize) || (uiDestPos >= uiDestSize))
	{
		return;
	}

	if (uiSourcePos + uiLength > uiSourceSize)
	{
		uiLength = uiSourceSize - uiSourcePos;
	}

	if (uiDestPos + uiLength > uiDestSize)
	{
		uiLength = uiDestSize - uiDestPos;
	}

	for (i = 0; i < uiLength; i++)
	{
		pvData = mpcSourceAccessor->Get(uiSourcePos + i);
		mpcDestAccessor->Set(uiDestPos + i, pvData);
	}
}

