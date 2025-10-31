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
#include "StandardLib/Unknowns.h"
#include "StandardLib/ChannelsAccessorContiguous.h"
#include "Image.h"
#include "ImageAccessorByte.h"
#include "ImageAccessorCreator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorCreator::Init(Ptr<CImage> pcImage)
{
	mcCreator.Init(&pcImage->mcChannels);
	mpcImage = pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorCreator::Kill(void)
{
	mpcImage = NULL;
	mcCreator.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageAccessor* CImageAccessorCreator::Create(void)
{
	CChannelsAccessor*				pcAccessor;
	CImageAccessor*					pcChannelBased;
	CImageAccessorByte*				pcByte;
	CChannelsAccessorContiguous*	pcContiguous;

	pcAccessor = mcCreator.Create();

	if (!pcAccessor)
	{
		return NULL;
	}

	if (pcAccessor->IsContiguous())
	{
		pcContiguous = (CChannelsAccessorContiguous*)pcAccessor;
		if (pcContiguous->GetByteSize() == 1)
		{
			pcByte = UMalloc(CImageAccessorByte);
			pcByte->Init(mpcImage, pcContiguous);
			return pcByte;
		}
	}

	pcChannelBased = UMalloc(CImageAccessor);
	pcChannelBased->Init(mpcImage, pcAccessor);
	return pcChannelBased;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageAccessor* CImageAccessorCreator::CreateAndKill(void)
{
	CImageAccessor*	pcAccessor;

	pcAccessor = Create();
	Kill();
	return pcAccessor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorCreator::AddAccess(int iChannel, EPrimitiveType eType)
{
	mcCreator.AddAccess(iChannel, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorCreator::AddAccess(int iChannel1, int iChannel2, EPrimitiveType eType)
{
	AddAccess(iChannel1, eType);
	AddAccess(iChannel2, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorCreator::AddAccess(int iChannel1, int iChannel2, int iChannel3, EPrimitiveType eType)
{
	AddAccess(iChannel1, eType);
	AddAccess(iChannel2, eType);
	AddAccess(iChannel3, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorCreator::AddAccess(int iChannel1, int iChannel2, int iChannel3, int iChannel4, EPrimitiveType eType)
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
void CImageAccessorCreator::AddAccess(CArrayInt* paiChannels, EPrimitiveType eType)
{
	mcCreator.AddAccess(paiChannels, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorCreator::AddAccess(Ptr<CImage> pcChannels)
{
	mcCreator.AddAccess(&pcChannels->mcChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorCreator::AddAccess(CImageAccessor* pcChannels)
{
	mcCreator.AddAccess(pcChannels->GetAccessor());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorCreator::AddAccess(CImageColour* pcColour)
{
	CArrayInt	aiChannels;

	aiChannels.Init();
	pcColour->GetChannels(&aiChannels);
	AddAccess(&aiChannels, PT_Undefined);
	aiChannels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageAccessorCreator::AddAccessFromIntersectionOf(CArrayInt* paiChannels1, CArrayInt* paiChannels2)  //Hmmm...
{
	CArrayInt	aiIntersection;

	aiIntersection.Init();
	aiIntersection.Intersect(paiChannels1, paiChannels2);
	AddAccess(&aiIntersection);
	aiIntersection.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageAccessor* CImageAccessorCreator::CreateEmpty(void)
{
	CImageAccessor*	pcAccessor;

	pcAccessor = UMalloc(CImageAccessor);
	pcAccessor->Init(NULL, NULL);
	return pcAccessor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageAccessor* CImageAccessorCreator::Create(Ptr<CImage> pcImage, int iFirst, ...)
{
	CImageAccessorCreator	cCreator;
	va_list					vaMarker;
	int						iValue;

	cCreator.Init(pcImage);

	va_start(vaMarker, iFirst);
	cCreator.AddAccess(iFirst);
	iValue = va_arg(vaMarker, int);
	while (iValue != CHANNEL_ZERO)
	{
		cCreator.AddAccess(iValue);
		iValue = va_arg(vaMarker, int);
	}
	va_end(vaMarker);

	return cCreator.CreateAndKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageAccessor* CImageAccessorCreator::Create(Ptr<CImage> pcImage)
{
	CImageAccessorCreator	cCreator;

	cCreator.Init(pcImage);
	cCreator.AddAccess(pcImage);
	return cCreator.CreateAndKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageAccessor* CImageAccessorCreator::Create(Ptr<CImage> pcImage, Ptr<CImage> pcChannels)
{
	CImageAccessorCreator	cCreator;

	cCreator.Init(pcImage);
	cCreator.AddAccess(pcChannels);
	return cCreator.CreateAndKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageAccessor* CImageAccessorCreator::Create(Ptr<CImage> pcImage, CArrayInt* paiChannels)
{
	CImageAccessorCreator	cCreator;

	cCreator.Init(pcImage);
	cCreator.AddAccess(paiChannels);
	return cCreator.CreateAndKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageAccessor* CImageAccessorCreator::Create(Ptr<CImage> pcImage, CImageColour* pcColour)
{
	CImageAccessorCreator	cCreator;

	cCreator.Init(pcImage);
	cCreator.AddAccess(pcColour);
	return cCreator.CreateAndKill();
}
