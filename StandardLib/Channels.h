/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __CHANNELS__H__
#define __CHANNELS__H__
#include "Unknown.h"
#include "Object.h"
#include "ObjectReader.h"
#include "ObjectWriter.h"
#include "ChannelDebugDescriptor.h"
#include "Channel.h"


class CChannels : public CObject
{
CONSTRUCTABLE(CChannels);
DESTRUCTABLE(CChannels);
protected:
	CArrayChannelOffset			masChannelOffsets;
	size						miSize;				//The number of 'pixels' in the channels (not the size in bytes).
	size						miByteStride;		//The number of bytes between 'pixels' zero if not a whole byte.
	size						miBitStride;		//The number of bits between pixels.
	bool						mbOnlyBasicTypes;	//Channels are only chars, shorts, ints, longs and floats.  Nothing bit'ty.

	CArrayChar					mabData;
	char*						mpvUserData;

	SChannelsChangingDesc*		mpsChangingDesc;
	char*						mpvDataCache;		//A pointer to either mabData.pvArray or mpvUserData

	CMapIntChannelDescriptor*	mpmicChannelDebugs;

public:
	void 					Init(void);
	void 					Init(size iSize, EPrimitiveType eType, size iFirst, ...);
	void 					Init(size iSize, void* pvUserData, EPrimitiveType eType, size iFirst, ...);
	void					Init(CChannels* pcSource);
	void					Class(void) override;
	void 					Free(void) override;

	void					ReInit(void);

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void 					BeginChange(void);
	void 					SetSize(size iSize);
	void 					PrivateAddChannel(size iChannel, EPrimitiveType eType, bool bReverse);
	void 					AddChannel(size iChannel, EPrimitiveType eType, bool bReverse = false);
	void 					AddChannel(size iChannel, EPrimitiveType eType, char* szShortName, char* szLongName = NULL, bool bReverse = false);
	void 					AddChannel(size iChannel1, size iChannel2, EPrimitiveType eType, bool bReverse = false);
	void 					AddChannel(size iChannel1, size iChannel2, size iChannel3, EPrimitiveType eType, bool bReverse = false);
	void 					AddChannel(size iChannel1, size iChannel2, size iChannel3, size iChannel4, EPrimitiveType eType, bool bReverse = false);
	void 					AddChannels(CArrayChannel* pasChannels);
	void 					AddChannels(CChannels* pcSourceChannels);
	bool					RemoveChannel(size iChannel);
	void					SetData(void* pvMem);
	void					ByteAlign(void);
	bool					RenameChannel(size iOldName, size iNewName);
	bool 					EndChange(void);
	bool					IsChanging(void);
	bool					IsSameFormat(CChannels* psOther);
	bool					IsChannelRemoved(size iChannel);
	bool					IsChannelAdded(size iChannel);
	bool					IsIgnored(void);

	bool					Is8BitAligned(size iBitStart, size iBitLength);
	size					CalculateByteStride(size iBitStride);
	size					CalculateByteSize(size iBitStride, size iSize);
	void					AllocateData(void);
	void					FreeData(void);

	void					Copy(CChannels* pcData);
	void					CopyIntoInitialised(CChannels* pcSource);
	void					Clear(void);
	void					Dump(size iLineLength);

	char*					GetData(void);
	bool					IsUserAllocated(void);
	CChannel*				GetChannel(size iChannel);		
	size					GetNumChannels(void);
	CChannel*				GetChannelAtIndex(size iIndex);
	size					GetIndexOfChannel(CChannel* pcChannel);
	bool					HasChannel(size iChannel);
	bool					HasChannel(size iChannel1, size iChannel2);
	bool					HasChannel(size iChannel1, size iChannel2, size iChannel3);
	bool					HasChannels(size iFirst, ...);
	size					GetSize(void);
	size					GetByteSize(void);
	CArrayChannelOffset*	GetChannelOffsets(void);
	size					GetByteStride(void);
	size					GetBitStride(void);
	char*					GetChannelLongName(size iChannel);
	char*					GetChannelShortName(size iChannel);

	void					GetAllChannels(CArrayInt* paiChannels);
	void					GetAllChannels(CArrayChannel* pasChannels);
	EPrimitiveType			GetPrimitiveType(void);  //Returns PT_Undefined if more than one.
	void					GetAllPrimitiveTypes(CArrayInt* paiPrimitiveTypes);
	void					GetChannelsForType(EPrimitiveType eType, CArrayInt* paiChannels);
	bool					IsOnlyBasicTypes(void);

	void					SetByteStride(size iByteStride);
	void					SetBitStrideFromByteStride(void);

	bool					IsValid(size iPos);
	void					SetChannelDebugNames(size iChannel, char* szShortName, char* szLongName);

	EPrimitiveType			GetLargestPrimitiveType(void);

protected:
	void					PrivateInit(void);
	void					Recalculate(void);

	void					CopyChannelDebugs(CMapIntChannelDescriptor* mpmicSourceChannelDebugs);
};


#endif // __CHANNELS__H__

