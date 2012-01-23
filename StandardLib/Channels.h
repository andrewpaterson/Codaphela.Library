/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __CHANNELS__H__
#define __CHANNELS__H__
#include "Unknown.h"
#include "Channel.h"


class CChannels : public CUnknown
{
BASE_FUNCTIONS(CChannels);
protected:
	CArrayChannelOffset		masChannelOffsets;
	int						miSize;  //The number of 'pixels' in the channels (not the size in bytes).
	int						miByteStride;  //The number of bytes between 'pixels' zero if not a whole byte.
	int						miBitStride;  //The number of bits between pixels.
	BOOL					mbOnlyBasicTypes;  //Channels are only chars, shorts, ints, longs and floats.  Nothing bit'ty.

	CArrayChar				mabData;
	char*					mpvUserData;

	SChannelsChangingDesc*	mpsChangingDesc;
	char*					mpvDataCache;  //A pointer to either mabData.pvArray or mpvUserData

public:
	void 					Init(void);
	void 					Init(int iSize, EPrimitiveTypes eType, int iFirst, ...);
	void 					Init(int iSize, void* pvUserData, EPrimitiveTypes eType, int iFirst, ...);
	void					Init(CChannels* pcSource);
	void 					Kill(void);

	void 					BeginChange(void);
	void 					SetSize(int iSize);
	void 					PrivateAddChannel(int iChannel, EPrimitiveTypes eType, BOOL bReverse);
	void 					AddChannel(int iChannel, EPrimitiveTypes eType, BOOL bReverse = FALSE);
	void 					AddChannel(int iChannel1, int iChannel2, EPrimitiveTypes eType, BOOL bReverse = FALSE);
	void 					AddChannel(int iChannel1, int iChannel2, int iChannel3, EPrimitiveTypes eType, BOOL bReverse = FALSE);
	void 					AddChannel(int iChannel1, int iChannel2, int iChannel3, int iChannel4, EPrimitiveTypes eType, BOOL bReverse = FALSE);
	void 					AddChannels(CArrayChannel* pasChannels);
	void 					AddChannels(CChannels* pcSourceChannels);
	void					RemoveChannel(int iChannel);
	void					SetData(void* pvMem);
	void					ByteAlign(void);
	void					RenameChannel(int iOldName, int iNewName);
	BOOL 					EndChange(void);
	BOOL					IsChanging(void);
	BOOL					IsSameFormat(CChannels* psOther);
	BOOL					IsChannelRemoved(int iChannel);
	BOOL					IsChannelAdded(int iChannel);
	BOOL					IsIgnored(void);

	BOOL					Is8BitAligned(int iBitStart, int iBitLength);
	int						CalculateByteStride(int iBitStride);
	int						CalculateByteSize(int iBitStride, int iSize);
	void					AllocateData(void);
	void					FreeData(void);

	BOOL					Load(CFileReader* pcFile);
	BOOL					Save(CFileWriter* pcFile);
	void					Copy(CChannels* pcData);
	void					Clear(void);
	void					Dump(int iLineLength);

	char*					GetData(void);
	BOOL					IsUserAllocated(void);
	CChannel*				GetChannel(int iChannel);		
	int						GetNumChannels(void);
	CChannel*				GetChannelAtIndex(int iIndex);
	int						GetIndexOfChannel(CChannel* pcChannel);
	BOOL					HasChannel(int iChannel);
	BOOL					HasChannel(int iChannel1, int iChannel2);
	BOOL					HasChannel(int iChannel1, int iChannel2, int iChannel3);
	BOOL					HasChannels(int iFirst, ...);
	int						GetSize(void);
	int						GetByteSize(void);
	CArrayChannelOffset*	GetChannelOffsets(void);
	int						GetByteStride(void);
	int						GetBitStride(void);

	void					GetAllChannels(CArrayInt* paiChannels);
	void					GetAllChannels(CArrayChannel* pasChannels);
	EPrimitiveTypes			GetPrimitiveType(void);  //Returns PT_Undefined if more than one.
	void					GetAllPrimitiveTypes(CArrayInt* paiPrimitiveTypes);
	void					GetChannelsForType(EPrimitiveTypes eType, CArrayInt* paiChannels);
	BOOL					IsOnlyBasicTypes(void);

	void					SetByteStride(int iByteStride);
	void					SetBitStrideFromByteStride(void);

	BOOL					IsValid(int iPos);

	EPrimitiveTypes			GetLargestPrimitiveType(void);

protected:
	void					Recalculate(void);
};


#endif // __CHANNELS__H__

