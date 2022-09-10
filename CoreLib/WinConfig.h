/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __WIN_CONFIG_H__
#define __WIN_CONFIG_H__
#include "BaseLib/Define.h"


enum EWinMode
{
	WM_DX_HAL,
	WM_DX_REF,
	WM_DX_SW,
};


class CWinConfig
{
public:
	int			miWidth;
	int			miHeight;
	bool		mbWindowed;
	float		mfAspect;
	EWinMode	meMode;

	void Init(void);
	bool Read(void);
	void Write(void);
	void Default(void);
};


#endif // !__WIN_CONFIG_H__

