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
#ifndef __CHANNELS_ACCESSOR_CREATOR_H__
#define __CHANNELS_ACCESSOR_CREATOR_H__
#include "BaseLib/DataTypes.h"
#include "Unknown.h"
#include "ChannelsAccessor.h"
#include "Channels.h"


struct SChannelAccess 
{
	int					iChannel;
	EPrimitiveTypes		eType;
};


typedef CArrayTemplate<SChannelAccess>	CArrayChannelAccess;


class CChannelsAccessorCreator
{
public:
	CChannels*				mpcChannels;
	CArrayChannelAccess		masAccess;

	CArrayChannelAccessor	macAccessor;

	void				Init(CChannels* pcChannels);
	void				Kill(void);
	CChannelsAccessor*	Create(void);
	CChannelsAccessor*	CreateAndKill(void);

	static CChannelsAccessor*	CreateSingleChannelAccessor(CChannels* pcChannels, int iChannel, EPrimitiveTypes eType = PT_Undefined);

	void 	AddAccess(int iChannel, EPrimitiveTypes eType = PT_Undefined);
	void 	AddAccess(int iChannel1, int iChannel2, EPrimitiveTypes eType = PT_Undefined);
	void	AddAccess(int iChannel1, int iChannel2, int iChannel3, EPrimitiveTypes eType = PT_Undefined);
	void	AddAccess(int iChannel1, int iChannel2, int iChannel3, int iChannel4, EPrimitiveTypes eType = PT_Undefined);
	void	AddAccess(CArrayInt* paiChannels, EPrimitiveTypes eType = PT_Undefined);
	void	AddAccess(CChannel* pcChannel);
	void	AddAccess(CChannels* pcChannels);
	void	AddAccess(CChannelAccessor* pcChannel);
	void	AddAccess(CChannelsAccessor* pcChannels);

private:
	BOOL	CalculateEmpty(void);
	BOOL	CalculateContiguous(void);
	BOOL	CalculateSourceTypesSame(void);
	BOOL	CalculateChannelByteAligned(void);
	BOOL	CalculateAccessByteAligned(void);
	int		CalclulateBitSize(void);

	BOOL	CreateAccessors(void);
};


#endif // __CHANNELS_ACCESSOR_CREATOR_H__

