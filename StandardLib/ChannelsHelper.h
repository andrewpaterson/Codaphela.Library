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
#ifndef __CHANNELS_HELPER_H__
#define __CHANNELS_HELPER_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/DataTypes.h"


struct SChannel
{
	size			iChannel;
	EPrimitiveType	eType;
	bool			bReverse;

	void Init(size iChannel, EPrimitiveType eType, bool bReverse);
};


typedef CArrayTemplate<SChannel>	CArrayChannel;


struct SChannelsChangingDesc
{
	CArrayChannel	asAddedChannels;
	CArrayChannel	asRemovedChannels;
	size			iSize;
	void*			pvUserData;

	void Init();
	void Kill();
};


#endif // __CHANNELS_HELPER_H__

