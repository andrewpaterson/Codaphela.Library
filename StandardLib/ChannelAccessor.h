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
#ifndef __CHANNEL_ACCESSOR_H__
#define __CHANNEL_ACCESSOR_H__
#include "BaseLib/DataTypes.h"
#include "BaseLib/ArrayTemplate.h"


class CChannelAccessor
{
public:
	int					miAccessByteSize;  
	int					miAccessBitSize;
	EPrimitiveType		meAccessType;

	int					miChannelByteOffset;
	int					miChannelByteSize;
	EPrimitiveType		meChannelType;
	BOOL				mbChannelReverse;
	int					miChannelBitSize;
	int					miChannelBitOffset;

	int					miChannel;

	void	Init(int iAccessByteSize, int iAccessBitSize, EPrimitiveType eAccessType, int iChannelByteOffset, int iChannelByteSize, EPrimitiveType eChannelType, BOOL bChannelReverse, int iChannelBitSize, int iChannelBitOffset, int iChannel);
	
	void	GetNative(void* pvBase, void* pvDest); //Not valid if miByteSize or miByteOffset -1
	void	GetAs(EPrimitiveType eType, void* pvBase, void* pvDest); //Not valid if miByteSize or miByteOffset -1
	void	GetAsSub(EPrimitiveType eType, void* pvBase, int iBitOffset, void* pvDest);	

	void	SetNative(void* pvBase, void* pvSource); //Not valid if miByteSize or miByteOffset -1
	void	SetFrom(EPrimitiveType eType, void* pvBase, void* pvSource);	//Not valid if miByteSize or miByteOffset -1
	void	SetFromSub(EPrimitiveType eType, void* pvBase, int iBitOffset, void* pvSource);	

	void	GetBits(void* pvBase, int iBitOffset, int iNumBits, void* pvDest);
	void	SetBits(void* pvBase, int iBitOffset, int iNumBits, void* pvSource);

	void	GetCast(EPrimitiveType eType, void* pvBase, void* pvDest);
	void	GetCastSub(EPrimitiveType eType, void* pvBase, int iBitOffset, void* pvDest);
};


typedef CArrayTemplate<CChannelAccessor>	CArrayChannelAccessor;


#endif // __CHANNEL_ACCESSOR_H__ 

