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
#include "ChannelsAccessorShapes.h"
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
	CChannelsAccessor*	pcAccessor;

	//Must be called before calculations
	if (!mcParams.CreateAccessorParams())
	{
		return NULL;
	}

	pcAccessor = Allocate(mcParams.IsContiguous(), mcParams.IsSourceTypesSame(), mcParams.IsChannelByteAligned(), mcParams.IsAccessByteAligned());
	pcAccessor->Init(mcParams.GetChannels(), mcParams.GetAccessors(), mcParams.GetByteSize(), mcParams.GetBitSize());
	return pcAccessor;
}


//////////////////////////////////////////////////////////////////////////
//
// The fastest accessor for the channels and the accesses: bitty
// channels or accesses need bit copies, different types conversions.
//
//////////////////////////////////////////////////////////////////////////
CChannelsAccessor* CChannelsAccessorCreator::Allocate(bool bContiguous, bool bSourceTypesSame, bool bChannelByteAligned, bool bAccessByteAligned)
{
	if (bChannelByteAligned && bAccessByteAligned)
	{
		if (!bSourceTypesSame)
		{
			return UMalloc(CChannelsAccessorTypeConverter);
		}
		else if (bContiguous)
		{
			return UMalloc(CChannelsAccessorContiguous);
		}
		else
		{
			return UMalloc(CChannelsAccessorByteAligned);
		}
	}
	else if (bChannelByteAligned)
	{
		return UMalloc(CChannelsAccessorAccessBitty);
	}
	else if (bAccessByteAligned)
	{
		return UMalloc(CChannelsAccessorChannelBitty);
	}
	else
	{
		return UMalloc(CChannelsAccessorWorstCase);
	}
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
bool CChannelsAccessorCreator::AddAccess(size iChannel, EPrimitiveType eType)
{
	return mcParams.AddAccess(iChannel, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreator::AddAccess(size iChannel1, size iChannel2, EPrimitiveType eType)
{
	return mcParams.AddAccess(iChannel1, iChannel2, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreator::AddAccess(size iChannel1, size iChannel2, size iChannel3, EPrimitiveType eType)
{
	return mcParams.AddAccess(iChannel1, iChannel2, iChannel3, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreator::AddAccess(size iChannel1, size iChannel2, size iChannel3, size iChannel4, EPrimitiveType eType)
{
	return mcParams.AddAccess(iChannel1, iChannel2, iChannel3, iChannel4, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreator::AddAccess(CArraySize* paiChannels, EPrimitiveType eType)
{
	return mcParams.AddAccess(paiChannels, eType);
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

