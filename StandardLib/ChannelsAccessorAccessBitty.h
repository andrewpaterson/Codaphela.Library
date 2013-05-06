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
#ifndef __CHANNELS_ACCESSOR_ACCESSOR_BITTY_H__
#define __CHANNELS_ACCESSOR_ACCESSOR_BITTY_H__
#include "ChannelsAccessor.h"


class CChannelsAccessorAccessBitty : public CChannelsAccessor
{
public:
	BASE_FUNCTIONS(CChannelsAccessorAccessBitty);

	void*	Get(int iPos);
	void	Set(int iPos, void* pvData);
};


#endif // __CHANNELS_ACCESSOR_ACCESSOR_BITTY_H__

