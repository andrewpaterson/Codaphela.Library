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
#ifndef __SINGLE_CHANNEL_ACCESSOR_H__
#define __SINGLE_CHANNEL_ACCESSOR_H__
#include "ChannelAccessor.h"
#include "Channels.h"


class CSingleChannelAccessor : protected CChannelAccessor
{
public:
	CChannels*	mpcChannels;

	void	Init(CChannels* pcChannels);

	float	GetConvertToFloat(int iChannel, int iPos = 0);
	float	GetCastToFloat(int iChannel, int iPos = 0);
	void	GetConvertTo(int iChannel, EPrimitiveType eType, void* pvDest, int iPos = 0);
	void	GetCastTo(int iChannel, EPrimitiveType eType, void* pvDest, int iPos = 0);
	void	GetNative(int iChannel, void* pvDest, int iPos = 0);
};


#endif // !__SINGLE_CHANNEL_ACCESSOR_H__

