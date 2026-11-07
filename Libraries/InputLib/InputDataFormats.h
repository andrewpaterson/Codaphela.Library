/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __INPUT_DATA_FORMATS_H__
#define __INPUT_DATA_FORMATS_H__
#include "StandardLib/Object.h"
#include "InputDataFormat.h"


//The data formats, in the order they were added.  The devices' descriptions and values point to them; this holds them.
class CInputDataFormats : public CObject
{
CONSTRUCTABLE(CInputDataFormats);
DESTRUCTABLE(CInputDataFormats);
protected:
	CArrayInputDataFormat	maFormats;

public:
	void				Init(void);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	CInputDataFormat*	Add(const char* szName);
	CInputDataFormat*	Add(void);
	CInputDataFormat*	Get(const char* szName);
};


#endif // __INPUT_DATA_FORMATS_H__

