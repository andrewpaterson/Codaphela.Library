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
	masAccess.Init();
	macAccessor.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::Kill(void)
{
	mpcChannels = NULL;
	macAccessor.Kill();
	masAccess.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannelsAccessor* CChannelsAccessorCreator::Create(void)
{
	BOOL				bContiguous;
	BOOL				bSourceTypesSame;
	BOOL				bChannelByteAligned;
	BOOL				bAccessByteAligned;

	int					iBitSize;
	int					iByteSize;
	int					iBufferSize;

	CChannelsAccessor*	pcAccessor;
	BOOL				bResult;

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
			((CChannelsAccessorContiguous*)pcAccessor)->Init(mpcChannels, &macAccessor, iByteSize, iBitSize, iBufferSize);
			return pcAccessor;
		}

		if (!bContiguous && bSourceTypesSame && bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorByteAligned);
			pcAccessor->Init(mpcChannels, &macAccessor, iByteSize, iBitSize, iBufferSize);
			return pcAccessor;
		}

		if (bContiguous && !bSourceTypesSame && bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorTypeConvert);
			pcAccessor->Init(mpcChannels, &macAccessor, iByteSize, iBitSize, iBufferSize);
			return pcAccessor;
		}

		if (!bContiguous && !bSourceTypesSame && bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorTypeConvert);
			pcAccessor->Init(mpcChannels, &macAccessor, iByteSize, iBitSize, iBufferSize);
			return pcAccessor;
		}

		if (bChannelByteAligned && !bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorAccessBitty);
			pcAccessor->Init(mpcChannels, &macAccessor, iByteSize, iBitSize, iBufferSize);
			return pcAccessor;
		}

		if (!bChannelByteAligned && bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorChannelBitty);
			pcAccessor->Init(mpcChannels, &macAccessor, iByteSize, iBitSize, iBufferSize);
			return pcAccessor;
		}

		if (!bChannelByteAligned && !bAccessByteAligned)
		{
			pcAccessor = UMalloc(CChannelsAccessorWorstCase);
			pcAccessor->Init(mpcChannels, &macAccessor, iByteSize, iBitSize, iBufferSize);
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
CChannelsAccessor* CChannelsAccessorCreator::CreateSingleChannelAccessor(CChannels* pcChannels, int iChannel, EPrimitiveTypes eType)
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
BOOL CChannelsAccessorCreator::CreateAccessors(void)
{
	int					i;
	SChannelAccess*		psAccess;
	CChannelAccessor*	pcAccessor;
	int					iByteSize;
	int					iBitSize;
	EPrimitiveTypes		eAccessType;
	CChannel*			pcChannel;
	int					iIndex;

	if (CalculateEmpty())
	{
		return FALSE;
	}

	macAccessor.Init();  //Safe to call init again if no elements have been added.

	for (i = 0; i < masAccess.NumElements(); i++)
	{
		psAccess = masAccess.Get(i);
		pcAccessor = macAccessor.Add();

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
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannelsAccessorCreator::CalculateEmpty(void)
{
	if (masAccess.NumElements() == 0)
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannelsAccessorCreator::CalculateContiguous(void)
{
	int					i;
	CChannel*			psChannel;
	SChannelAccess*		psAccess;
	int					iLastIndex;
	int					iIndex;

	psAccess = masAccess.Get(0);
	psChannel = mpcChannels->GetChannel(psAccess->iChannel);
	iLastIndex = mpcChannels->GetIndexOfChannel(psChannel);

	for (i = 1; i < masAccess.NumElements(); i++)
	{
		psAccess = masAccess.Get(i);
		psChannel = mpcChannels->GetChannel(psAccess->iChannel);
		iIndex = mpcChannels->GetIndexOfChannel(psChannel);

		if (iIndex != iLastIndex + 1)
		{
			return FALSE;
		}

		if (psChannel->bReverse)
		{
			return FALSE;
		}
		iLastIndex = iIndex;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannelsAccessorCreator::CalculateSourceTypesSame(void)
{
	int					i;
	CChannelAccessor*	pcAccessor;;

	for (i = 0; i < macAccessor.NumElements(); i++)
	{
		pcAccessor = macAccessor.Get(i);

		if (pcAccessor->meAccessType != pcAccessor->meChannelType)
		{
			return FALSE;
		}
		if (pcAccessor->mbChannelReverse)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannelsAccessorCreator::CalculateChannelByteAligned(void)
{
	int					i;
	CChannel*			pcChannel;
	SChannelAccess*		psAccess;

	if (mpcChannels->GetByteStride() == -1)
	{
		return FALSE;
	}

	for (i = 0; i < masAccess.NumElements(); i++)
	{
		psAccess = masAccess.Get(i);
		pcChannel = mpcChannels->GetChannel(psAccess->iChannel);

		if (pcChannel->miByteSize == -1)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannelsAccessorCreator::CalculateAccessByteAligned(void)
{
	int					i;
	CChannelAccessor*	pcAccessor;

	for (i = 0; i < macAccessor.NumElements(); i++)
	{
		pcAccessor = macAccessor.Get(i);
		if (pcAccessor->miAccessByteSize == -1)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChannelsAccessorCreator::CalclulateBitSize(void)
{
	int					i;
	CChannelAccessor*	pcAccessor;
	int					iBitSize;

	iBitSize = 0;
	for (i = 0; i < macAccessor.NumElements(); i++)
	{
		pcAccessor = macAccessor.Get(i);
		iBitSize += pcAccessor->miAccessBitSize;
	}
	return iBitSize;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(int iChannel, EPrimitiveTypes eType)
{
	SChannelAccess*	psAccess;

	//Can't create an accessor to a channel that does not exist in the source.
	if (mpcChannels->HasChannel(iChannel))
	{
		psAccess = masAccess.Add();
		psAccess->eType = eType;
		psAccess->iChannel = iChannel;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(int iChannel1, int iChannel2, EPrimitiveTypes eType)
{
	AddAccess(iChannel1, eType);
	AddAccess(iChannel2, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(int iChannel1, int iChannel2, int iChannel3, EPrimitiveTypes eType)
{
	AddAccess(iChannel1, eType);
	AddAccess(iChannel2, eType);
	AddAccess(iChannel3, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorCreator::AddAccess(int iChannel1, int iChannel2, int iChannel3, int iChannel4, EPrimitiveTypes eType)
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
void CChannelsAccessorCreator::AddAccess(CArrayInt* paiChannels, EPrimitiveTypes eType)
{
	int		i;
	int		iChannel;

	for (i = 0; i < paiChannels->NumElements(); i++)
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
	int			i;
	CChannel*	pcChannel;

	for (i = 0; i < pcChannels->GetChannelOffsets()->NumElements(); i++)
	{
		pcChannel = pcChannels->GetChannelOffsets()->Get(i);
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
	int						i;
	CChannelAccessor*		pcChannel;
	CArrayChannelAccessor*	pacAccessors;

	pacAccessors = pcChannels->GetAccessors();
	for (i = 0; i < pacAccessors->NumElements(); i++)
	{
		pcChannel = pacAccessors->Get(i);
		AddAccess(pcChannel);
	}
}
