/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "ChannelsAccessorTypeConvert.h"
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
	mpcChannels = pcChannels;
	masAccesses.Init();
	macAccessors.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::Kill(void)
{
	mpcChannels = NULL;
	macAccessors.Kill();
	masAccesses.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannelsAccessor* CChannelsAccessorCreator::Create(void)
{
	bool				bContiguous;
	bool				bSourceTypesSame;
	bool				bChannelByteAligned;
	bool				bAccessByteAligned;

	size				iBitSize;
	size				iByteSize;
	size				iBufferSize;

	CChannelsAccessor*	pcAccessor;
	bool				bResult;

	//Must be called before calculations
	bResult = CreateAccessors();

	if (bResult)
	{
		bContiguous = CalculateContiguous();
		bSourceTypesSame = CalculateSourceTypesSame();
		bChannelByteAligned = CalculateChannelByteAligned();
		bAccessByteAligned = CalculateAccessByteAligned();
		iBitSize = CalclulateBitSize();

		if (iBitSize % 8 == 0)
		{
			iByteSize = iBitSize / 8;
			iBufferSize = iByteSize;
		}
		else
		{
			iByteSize = -1;
			iBufferSize = iBitSize / 8 + 1;
		}
		
		pcAccessor = NULL;

		if (bContiguous && bSourceTypesSame && bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorContiguous);
			((CChannelsAccessorContiguous*)pcAccessor)->Init(mpcChannels, &macAccessors, iByteSize, iBitSize, iBufferSize);
			return pcAccessor;
		}

		if (!bContiguous && bSourceTypesSame && bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorByteAligned);
			pcAccessor->Init(mpcChannels, &macAccessors, iByteSize, iBitSize, iBufferSize);
			return pcAccessor;
		}

		if (bContiguous && !bSourceTypesSame && bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorTypeConvert);
			pcAccessor->Init(mpcChannels, &macAccessors, iByteSize, iBitSize, iBufferSize);
			return pcAccessor;
		}

		if (!bContiguous && !bSourceTypesSame && bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorTypeConvert);
			pcAccessor->Init(mpcChannels, &macAccessors, iByteSize, iBitSize, iBufferSize);
			return pcAccessor;
		}

		if (bChannelByteAligned && !bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorAccessBitty);
			pcAccessor->Init(mpcChannels, &macAccessors, iByteSize, iBitSize, iBufferSize);
			return pcAccessor;
		}

		if (!bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorChannelBitty);
			pcAccessor->Init(mpcChannels, &macAccessors, iByteSize, iBitSize, iBufferSize);
			return pcAccessor;
		}

		if (!bChannelByteAligned && !bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorWorstCase);
			pcAccessor->Init(mpcChannels, &macAccessors, iByteSize, iBitSize, iBufferSize);
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
bool CChannelsAccessorCreator::CreateAccessors(void)
{
	size				i;
	SChannelAccess*		psAccess;
	CChannelAccessor*	pcAccessor;
	size				iByteSize;
	size				iBitSize;
	EPrimitiveType		eAccessType;
	CChannel*			pcChannel;
	size				iIndex;
	size				uiNumAccess;

	if (CalculateEmpty())
	{
		return false;
	}

	uiNumAccess = masAccesses.NumElements();
	for (i = 0; i < uiNumAccess; i++)
	{
		psAccess = masAccesses.Get(i);
		pcAccessor = macAccessors.Add();

		pcChannel = mpcChannels->GetChannel(psAccess->iChannel);
		iIndex = mpcChannels->GetIndexOfChannel(pcChannel);

		eAccessType = psAccess->eType;
		if (eAccessType == PT_Undefined)
		{
			eAccessType = pcChannel->eType;
		}

		iByteSize = gcTypeNames.GetByteSize(eAccessType);
		iBitSize = gcTypeNames.GetBitSize(eAccessType);
		if (iByteSize == 0)
		{
			iByteSize = -1;
		}

		pcAccessor->Init(iByteSize, iBitSize, eAccessType, pcChannel->miByteOffset, pcChannel->miByteSize, pcChannel->eType, pcChannel->bReverse, pcChannel->miBitSize, pcChannel->miBitOffset, pcChannel->iChannel);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessorCreator::CalculateEmpty(void)
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
bool CChannelsAccessorCreator::CalculateContiguous(void)
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
bool CChannelsAccessorCreator::CalculateSourceTypesSame(void)
{
	size				i;
	CChannelAccessor*	pcAccessor;;
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
bool CChannelsAccessorCreator::CalculateChannelByteAligned(void)
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
bool CChannelsAccessorCreator::CalculateAccessByteAligned(void)
{
	size				i;
	CChannelAccessor*	pcAccessor;
	size				uiNumAccessors;

	uiNumAccessors = macAccessors.NumElements();
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);
		if (pcAccessor->miAccessByteSize == -1)
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
size CChannelsAccessorCreator::CalclulateBitSize(void)
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
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(size iChannel, EPrimitiveType eType)
{
	SChannelAccess*	psAccess;

	//Can't create an accessor to a channel that does not exist in the source.
	if (mpcChannels->HasChannel(iChannel))
	{
		psAccess = masAccesses.Add();
		psAccess->eType = eType;
		psAccess->iChannel = iChannel;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(size iChannel1, size iChannel2, EPrimitiveType eType)
{
	AddAccess(iChannel1, eType);
	AddAccess(iChannel2, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(size iChannel1, size iChannel2, size iChannel3, EPrimitiveType eType)
{
	AddAccess(iChannel1, eType);
	AddAccess(iChannel2, eType);
	AddAccess(iChannel3, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(size iChannel1, size iChannel2, size iChannel3, size iChannel4, EPrimitiveType eType)
{
	AddAccess(iChannel1, eType);
	AddAccess(iChannel2, eType);
	AddAccess(iChannel3, eType);
	AddAccess(iChannel4, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(CArrayInt* paiChannels, EPrimitiveType eType)
{
	size	i;
	size	iChannel;
	size	uiNumChannels;

	uiNumChannels = paiChannels->NumElements();
	for (i = 0; i < uiNumChannels; i++)
	{
		iChannel = paiChannels->GetValue(i);
		AddAccess(iChannel, eType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(CChannel* pcChannel)
{
	AddAccess(pcChannel->iChannel, pcChannel->eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(CChannels* pcChannels)
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
void CChannelsAccessorCreator::AddAccess(CChannelAccessor* pcChannel)
{
	AddAccess(pcChannel->miChannel, pcChannel->meAccessType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(CChannelsAccessor* pcChannels)
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

