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
#ifndef __EXTERNAL_OBJECT_WRITER_H__
#define __EXTERNAL_OBJECT_WRITER_H__
#include "CoreLib/IndexedGeneral.h"
#include "Unknown.h"


class CSerialisedObject;
class CExternalObjectWriter : public CUnknown
{
CONSTRUCTABLE(CExternalObjectWriter);
public:
			void Init(char* szBaseName);
			void Kill(void);

	virtual BOOL Begin(void);
	virtual BOOL Write(CSerialisedObject* pcSerialised) =0;
	virtual BOOL End(void);
};


#endif // !__EXTERNAL_OBJECT_WRITER_H__

