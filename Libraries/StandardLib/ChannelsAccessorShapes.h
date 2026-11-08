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
#ifndef __CHANNELS_ACCESSOR_SHAPES_H__
#define __CHANNELS_ACCESSOR_SHAPES_H__
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/PointerFunctions.h"
#include "ChannelsAccessor.h"


//The accessors other than the contiguous one come in two shapes: the values gotten are either
//packed in whole bytes (each in its access type's bytes) or packed in bits.  What varies is how
//each channel of the element is reached, which is one of these.

//The channels are byte aligned and of the types accessed.
struct SChannelsNative
{
	void*	pvElement;

	void	Start(CChannels* pcChannels, lsize iPos)			{ pvElement = RemapSinglePointer(pcChannels->GetData(), iPos * pcChannels->GetByteStride()); }
	void	Get(CChannelAccessor* pcAccessor, void* pvDest)		{ pcAccessor->GetNative(pvElement, pvDest); }
	void	Set(CChannelAccessor* pcAccessor, void* pvSource)	{ pcAccessor->SetNative(pvElement, pvSource); }
};


//The channels are byte aligned and converted to and from the types accessed.
struct SChannelsConverted
{
	void*	pvElement;

	void	Start(CChannels* pcChannels, lsize iPos)			{ pvElement = RemapSinglePointer(pcChannels->GetData(), iPos * pcChannels->GetByteStride()); }
	void	Get(CChannelAccessor* pcAccessor, void* pvDest)		{ pcAccessor->GetAs(pcAccessor->meAccessType, pvElement, pvDest); }
	void	Set(CChannelAccessor* pcAccessor, void* pvSource)	{ pcAccessor->SetFrom(pcAccessor->meAccessType, pvElement, pvSource); }
};


//The channels are not byte aligned: reached by the element's bit position, and converted.
struct SChannelsBitty
{
	void*	pvData;
	lsize	iElementBitPos;

	void	Start(CChannels* pcChannels, lsize iPos)			{ pvData = pcChannels->GetData(); iElementBitPos = pcChannels->GetBitStride() * iPos; }
	void	Get(CChannelAccessor* pcAccessor, void* pvDest)		{ pcAccessor->GetAsSub(pcAccessor->meAccessType, pvData, iElementBitPos, pvDest); }
	void	Set(CChannelAccessor* pcAccessor, void* pvSource)	{ pcAccessor->SetFromSub(pcAccessor->meAccessType, pvData, iElementBitPos, pvSource); }
};


//The values gotten and set are packed in whole bytes.
template<class TChannels>
class CChannelsAccessorByteAccess : public CChannelsAccessor
{
public:
	void*	Get(lsize iPos) override;
	void	Set(lsize iPos, void* pvSource) override;
};


//The values gotten and set are packed in bits.
template<class TChannels>
class CChannelsAccessorBitAccess : public CChannelsAccessor
{
public:
	void*	Get(lsize iPos) override;
	void	Set(lsize iPos, void* pvSource) override;
};


class CChannelsAccessorByteAligned : public CChannelsAccessorByteAccess<SChannelsNative>
{
CONSTRUCTABLE(CChannelsAccessorByteAligned)
};


class CChannelsAccessorTypeConverter : public CChannelsAccessorByteAccess<SChannelsConverted>
{
CONSTRUCTABLE(CChannelsAccessorTypeConverter)
};


class CChannelsAccessorChannelBitty : public CChannelsAccessorByteAccess<SChannelsBitty>
{
CONSTRUCTABLE(CChannelsAccessorChannelBitty)
};


class CChannelsAccessorAccessBitty : public CChannelsAccessorBitAccess<SChannelsConverted>
{
CONSTRUCTABLE(CChannelsAccessorAccessBitty)
};


class CChannelsAccessorWorstCase : public CChannelsAccessorBitAccess<SChannelsBitty>
{
CONSTRUCTABLE(CChannelsAccessorWorstCase)
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class TChannels>
void* CChannelsAccessorByteAccess<TChannels>::Get(lsize iPos)
{
	size				i;
	CChannelAccessor*	pcAccessor;
	void*				pvDest;
	size				uiNumAccessors;
	TChannels			sChannels;

	uiNumAccessors = macAccessors.NumElements();
	sChannels.Start(mpcChannels, iPos);
	pvDest = msBuffer.GetStackData();
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);
		sChannels.Get(pcAccessor, pvDest);
		pvDest = RemapSinglePointer(pvDest, pcAccessor->miAccessByteSize);
	}
	return msBuffer.GetStackData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class TChannels>
void CChannelsAccessorByteAccess<TChannels>::Set(lsize iPos, void* pvSource)
{
	size				i;
	CChannelAccessor*	pcAccessor;
	size				uiNumAccessors;
	TChannels			sChannels;

	uiNumAccessors = macAccessors.NumElements();
	sChannels.Start(mpcChannels, iPos);
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);
		sChannels.Set(pcAccessor, pvSource);
		pvSource = RemapSinglePointer(pvSource, pcAccessor->miAccessByteSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class TChannels>
void* CChannelsAccessorBitAccess<TChannels>::Get(lsize iPos)
{
	size				i;
	CChannelAccessor*	pcAccessor;
	lsize				iDestBitPos;
	char				c[CHANNEL_SCRATCH_BYTES + 1];
	size				uiNumAccessors;
	uint8*				pvBuffer;
	TChannels			sChannels;

	uiNumAccessors = macAccessors.NumElements();
	sChannels.Start(mpcChannels, iPos);
	iDestBitPos = 0;
	pvBuffer = (uint8*)msBuffer.GetStackData();
	pvBuffer[GetBufferSize() - 1] = 0;

	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);
		sChannels.Get(pcAccessor, c);
		CopyBits(pvBuffer, iDestBitPos, c, 0, pcAccessor->miAccessBitSize, false);
		iDestBitPos += pcAccessor->miAccessBitSize;
	}
	return pvBuffer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class TChannels>
void CChannelsAccessorBitAccess<TChannels>::Set(lsize iPos, void* pvSource)
{
	size				i;
	CChannelAccessor*	pcAccessor;
	lsize				iSourceBitPos;
	char				c[CHANNEL_SCRATCH_BYTES + 1];
	size				uiNumAccessors;
	TChannels			sChannels;

	uiNumAccessors = macAccessors.NumElements();
	sChannels.Start(mpcChannels, iPos);
	iSourceBitPos = 0;
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);
		c[pcAccessor->miAccessBitSize / 8] = 0;
		CopyBits(c, 0, pvSource, iSourceBitPos, pcAccessor->miAccessBitSize, false);
		sChannels.Set(pcAccessor, c);
		iSourceBitPos += pcAccessor->miAccessBitSize;
	}
}


#endif // __CHANNELS_ACCESSOR_SHAPES_H__

