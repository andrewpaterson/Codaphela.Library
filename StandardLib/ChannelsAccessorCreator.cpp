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
#include "BaseLib/TypeNames.h"
#include "Unknowns.h"
#include "ChannelsAccessorContiguous.h"
#include "ChannelsAccessorByteAligned.h"
#include "ChannelsAccessorTypeConverter.h"
#include "ChannelsAccessorAccessBitty.h"
#include "ChannelsAccessorChannelBitty.h"
#include "ChannelsAccessorWorstCase.h"
#include "ChannelsAccessorCreator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::Init(CChannels* pcChannels)
{
	mcParams.Init(pcChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::Kill(void)
{
	mcParams.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannelsAccessor* CChannelsAccessorCreator::Create(void)
{
	bool					bContiguous;
	bool					bSourceTypesSame;
	bool					bChannelByteAligned;
	bool					bAccessByteAligned;
	size					iBitSize;
	size					iByteSize;
	CArrayChannelAccessor*  pacAccessors;
	CChannels*				pcChannels;

	CChannelsAccessor*	pcAccessor;
	bool				bResult;

	//Must be called before calculations
	bResult = mcParams.CreateAccessorParams();

	if (bResult)
	{
		bContiguous = mcParams.IsContiguous();
		bSourceTypesSame = mcParams.IsSourceTypesSame();
		bChannelByteAligned = mcParams.IsChannelByteAligned();
		bAccessByteAligned = mcParams.IsAccessByteAligned();
		iBitSize = mcParams.GetBitSize();
		iByteSize = mcParams.GetByteSize();
		pcChannels = mcParams.GetChannels();
		pacAccessors = mcParams.GetAccessors();
		pcAccessor = NULL;

		if (bContiguous && bSourceTypesSame && bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorContiguous);
			((CChannelsAccessorContiguous*)pcAccessor)->Init(pcChannels, pacAccessors, iByteSize, iBitSize);
			return pcAccessor;
		}

		if (!bContiguous && bSourceTypesSame && bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorByteAligned);
			pcAccessor->Init(pcChannels, pacAccessors, iByteSize, iBitSize);
			return pcAccessor;
		}

		if (bContiguous && !bSourceTypesSame && bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorTypeConverter);
			pcAccessor->Init(pcChannels, pacAccessors, iByteSize, iBitSize);
			return pcAccessor;
		}

		if (!bContiguous && !bSourceTypesSame && bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorTypeConverter);
			pcAccessor->Init(pcChannels, pacAccessors, iByteSize, iBitSize);
			return pcAccessor;
		}

		if (bChannelByteAligned && !bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorAccessBitty);
			pcAccessor->Init(pcChannels, pacAccessors, iByteSize, iBitSize);
			return pcAccessor;
		}

		if (!bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorChannelBitty);
			pcAccessor->Init(pcChannels, pacAccessors, iByteSize, iBitSize);
			return pcAccessor;
		}

		if (!bChannelByteAligned && !bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorWorstCase);
			pcAccessor->Init(pcChannels, pacAccessors, iByteSize, iBitSize);
			return pcAccessor;
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannelsAccessor* CChannelsAccessorCreator::CreateAndKill(void)
{
	CChannelsAccessor*	pcAccessor;

	pcAccessor = Create();
	Kill();
	return pcAccessor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannelsAccessor* CChannelsAccessorCreator::CreateSingleChannelAccessor(CChannels* pcChannels, size iChannel, EPrimitiveType eType)
{
	CChannelsAccessorCreator	cCreator;

	cCreator.Init(pcChannels);
	cCreator.AddAccess(iChannel, eType);
	return cCreator.CreateAndKill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(size iChannel, EPrimitiveType eType)
{
	mcParams.AddAccess(iChannel, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(size iChannel1, size iChannel2, EPrimitiveType eType)
{
	mcParams.AddAccess(iChannel1, iChannel2, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(size iChannel1, size iChannel2, size iChannel3, EPrimitiveType eType)
{
	mcParams.AddAccess(iChannel1, iChannel2, iChannel3, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(size iChannel1, size iChannel2, size iChannel3, size iChannel4, EPrimitiveType eType)
{
	mcParams.AddAccess(iChannel1, iChannel2, iChannel3, iChannel4, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(CArrayInt* paiChannels, EPrimitiveType eType)
{
	mcParams.AddAccess(paiChannels, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(CChannel* pcChannel)
{
	mcParams.AddAccess(pcChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(CChannels* pcChannels)
{
	mcParams.AddAccess(pcChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(CChannelAccessor* pcChannel)
{
	mcParams.AddAccess(pcChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(CChannelsAccessor* pcChannels)
{
	mcParams.AddAccess(pcChannels);
}

