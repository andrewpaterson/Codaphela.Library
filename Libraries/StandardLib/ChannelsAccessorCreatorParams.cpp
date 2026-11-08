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
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "Unknowns.h"
#include "ChannelsAccessorCreatorParams.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreatorParams::Init(CChannels* pcChannels)
{
	mpcChannels = pcChannels;
	masAccesses.Init();

	macAccessors.Init();
	mbContiguous = false;
	mbSourceTypesSame = false;
	mbChannelByteAligned = false;
	mbAccessByteAligned = false;
	muiBitSize = MAX_SIZE;
	muiByteSize = CHANNEL_NON_ALIGNED_BYTES;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreatorParams::Kill(void)
{
	mpcChannels = NULL;
	macAccessors.Kill();
	masAccesses.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreatorParams::CreateAccessorParams(void)
{
	size				i;
	SChannelAccess*		psAccess;
	CChannelAccessor*	pcAccessor;
	size				iByteSize;
	size				iBitSize;
	EPrimitiveType		eAccessType;
	CChannel*			pcChannel;
	size				uiNumAccess;

	if (CalculateEmpty())
	{
		return false;
	}

	uiNumAccess = masAccesses.NumElements();
	for (i = 0; i < uiNumAccess; i++)
	{
		psAccess = masAccesses.Get(i);

		pcChannel = mpcChannels->GetChannel(psAccess->iChannel);
		if (pcChannel)
		{
			pcAccessor = macAccessors.Add();

			eAccessType = psAccess->eType;
			if (eAccessType == PT_Undefined)
			{
				eAccessType = pcChannel->eType;
			}

			iByteSize = gcTypeNames.GetByteSize(eAccessType);
			iBitSize = gcTypeNames.GetBitSize(eAccessType);
			if ((iBitSize > CHANNEL_SCRATCH_BYTES * 8) || (pcChannel->miBitSize > CHANNEL_SCRATCH_BYTES * 8))
			{
				return gcLogger.Error2(__METHOD__, " Channel [", SizeToString(pcChannel->iChannel), "] or its access is wider than [", SizeToString(CHANNEL_SCRATCH_BYTES * 8), "] bits.", NULL);
			}
			if (iByteSize == 0)
			{
				iByteSize = CHANNEL_NON_ALIGNED_BYTES;
			}

			pcAccessor->Init(iByteSize, iBitSize, eAccessType, pcChannel->miByteOffset, pcChannel->miByteSize, pcChannel->eType, pcChannel->bReverse, pcChannel->miBitSize, pcChannel->miBitOffset, pcChannel->iChannel);
		}
	}

	mbContiguous = CalculateContiguous();
	mbSourceTypesSame = CalculateSourceTypesSame();
	mbChannelByteAligned = CalculateChannelByteAligned();
	mbAccessByteAligned = CalculateAccessByteAligned();
	muiBitSize = CalculateBitSize();

	if (muiBitSize % 8 == 0)
	{
		muiByteSize = muiBitSize / 8;
	}
	else
	{
		muiByteSize = CHANNEL_NON_ALIGNED_BYTES;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreatorParams::CalculateEmpty(void)
{
	if (masAccesses.NumElements() == 0)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreatorParams::CalculateContiguous(void)
{
	size				i;
	CChannel*			psChannel;
	SChannelAccess*		psAccess;
	size				iLastIndex;
	size				iIndex;
	size				uiNumAccess;

	psAccess = masAccesses.Get(0);
	psChannel = mpcChannels->GetChannel(psAccess->iChannel);
	iLastIndex = mpcChannels->GetIndexOfChannel(psChannel);

	uiNumAccess = masAccesses.NumElements();
	for (i = 1; i < uiNumAccess; i++)
	{
		psAccess = masAccesses.Get(i);
		psChannel = mpcChannels->GetChannel(psAccess->iChannel);
		iIndex = mpcChannels->GetIndexOfChannel(psChannel);

		if (iIndex != iLastIndex + 1)
		{
			return false;
		}

		if (psChannel->bReverse)
		{
			return false;
		}
		iLastIndex = iIndex;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreatorParams::CalculateSourceTypesSame(void)
{
	size				i;
	CChannelAccessor*	pcAccessor;
	size				uiNumAccessors;

	uiNumAccessors = macAccessors.NumElements();
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);

		if (pcAccessor->meAccessType != pcAccessor->meChannelType)
		{
			return false;
		}
		if (pcAccessor->mbChannelReverse)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreatorParams::CalculateChannelByteAligned(void)
{
	size				i;
	CChannel*			pcChannel;
	SChannelAccess*		psAccess;
	size				uiNumAccess;

	if (mpcChannels->GetByteStride() == CHANNEL_NON_ALIGNED_BYTES)
	{
		return false;
	}

	uiNumAccess = masAccesses.NumElements();
	for (i = 0; i < uiNumAccess; i++)
	{
		psAccess = masAccesses.Get(i);
		pcChannel = mpcChannels->GetChannel(psAccess->iChannel);
		
		if (pcChannel->miByteSize == CHANNEL_NON_ALIGNED_BYTES)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreatorParams::CalculateAccessByteAligned(void)
{
	size				i;
	CChannelAccessor*	pcAccessor;
	size				uiNumAccessors;

	uiNumAccessors = macAccessors.NumElements();
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);
		if (pcAccessor->miAccessByteSize == CHANNEL_NON_ALIGNED_BYTES)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChannelsAccessorCreatorParams::CalculateBitSize(void)
{
	size				i;
	CChannelAccessor*	pcAccessor;
	size				iBitSize;
	size				uiNumAccessors;

	iBitSize = 0;
	uiNumAccessors = macAccessors.NumElements();
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);
		iBitSize += pcAccessor->miAccessBitSize;
	}
	return iBitSize;
}



//////////////////////////////////////////////////////////////////////////
//
// An access to iChannel, if the channels have it: whether they did.  A
// missing channel is skipped, not an error: an accessor reads the
// channels it can (an image's opacity, say, when it has one), and
// ChannelsCopier keeps only the channels both sides have this way.
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreatorParams::AddAccess(size iChannel, EPrimitiveType eType)
{
	SChannelAccess*	psAccess;

	if (!mpcChannels->HasChannel(iChannel))
	{
		return false;
	}

	psAccess = masAccesses.Add();
	psAccess->eType = eType;
	psAccess->iChannel = iChannel;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Each channel the channels have: whether they all were.
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreatorParams::AddAccess(size iChannel1, size iChannel2, EPrimitiveType eType)
{
	bool	bAll;

	bAll = AddAccess(iChannel1, eType);
	bAll &= AddAccess(iChannel2, eType);
	return bAll;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreatorParams::AddAccess(size iChannel1, size iChannel2, size iChannel3, EPrimitiveType eType)
{
	bool	bAll;

	bAll = AddAccess(iChannel1, eType);
	bAll &= AddAccess(iChannel2, eType);
	bAll &= AddAccess(iChannel3, eType);
	return bAll;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreatorParams::AddAccess(size iChannel1, size iChannel2, size iChannel3, size iChannel4, EPrimitiveType eType)
{
	bool	bAll;

	bAll = AddAccess(iChannel1, eType);
	bAll &= AddAccess(iChannel2, eType);
	bAll &= AddAccess(iChannel3, eType);
	bAll &= AddAccess(iChannel4, eType);
	return bAll;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreatorParams::AddAccess(CArraySize* paiChannels, EPrimitiveType eType)
{
	size	i;
	size	uiNumChannels;
	bool	bAll;

	bAll = true;
	uiNumChannels = paiChannels->NumElements();
	for (i = 0; i < uiNumChannels; i++)
	{
		bAll &= AddAccess(paiChannels->GetValue(i), eType);
	}
	return bAll;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreatorParams::AddAccess(CChannel* pcChannel)
{
	AddAccess(pcChannel->iChannel, pcChannel->eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreatorParams::AddAccess(CChannels* pcChannels)
{
	size					i;
	CChannel*				pcChannel;
	size					uiChannelOffsets;
	CArrayChannelOffset*	pcChannelOffsets;

	pcChannelOffsets = pcChannels->GetChannelOffsets();
	uiChannelOffsets = pcChannelOffsets->NumElements();
	for (i = 0; i < uiChannelOffsets; i++)
	{
		pcChannel = pcChannelOffsets->Get(i);
		AddAccess(pcChannel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreatorParams::AddAccess(CChannelAccessor* pcChannel)
{
	AddAccess(pcChannel->miChannel, pcChannel->meAccessType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreatorParams::AddAccess(CChannelsAccessor* pcChannels)
{
	size					i;
	CChannelAccessor*		pcChannel;
	CArrayChannelAccessor*	pacAccessors;
	size					uiNumAccessors;

	pacAccessors = pcChannels->GetAccessors();
	uiNumAccessors = pacAccessors->NumElements();
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcChannel = pacAccessors->Get(i);
		AddAccess(pcChannel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayChannelAccessor* CChannelsAccessorCreatorParams::GetAccessors(void) { return &macAccessors; }
CArrayChannelAccess* CChannelsAccessorCreatorParams::GetAccesses(void) { return &masAccesses; }
bool CChannelsAccessorCreatorParams::IsContiguous(void) { return mbContiguous; }
bool CChannelsAccessorCreatorParams::IsSourceTypesSame(void) { return mbSourceTypesSame; }
bool CChannelsAccessorCreatorParams::IsChannelByteAligned(void) { return mbChannelByteAligned; }
bool CChannelsAccessorCreatorParams::IsAccessByteAligned(void) { return mbAccessByteAligned; }
size CChannelsAccessorCreatorParams::GetBitSize(void) { return muiBitSize; }
size CChannelsAccessorCreatorParams::GetByteSize(void) { return muiByteSize; }
CChannels* CChannelsAccessorCreatorParams::GetChannels(void) { return mpcChannels; }

