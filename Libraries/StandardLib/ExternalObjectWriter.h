/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "BaseLib/IndexedGeneral.h"
#include "BaseLib/Chars.h"
#include "Unknown.h"


class CSerialisedObject;

//Writes serialised objects outside a Codabase, into mszDirectory.  Named objects are written only if their names start
//with mszObjectBaseName, by the rest of their names.
class CExternalObjectWriter : public CUnknown
{
CONSTRUCTABLE(CExternalObjectWriter)
protected:
	CChars	mszDirectory;
	CChars	mszObjectBaseName;	//'/' separated, without a trailing '/'.

public:
			void Init(const char* szDirectory, const char* szBaseName);
			void Kill(void) override;

	virtual bool Begin(void);
	virtual bool Write(CSerialisedObject* pcSerialised) =0;
	virtual bool End(void);

protected:
			bool ObjectStartsWithBaseName(const char* szObjectName);
			void RemainingName(CChars* pszRemainingName, const char* szObjectName);
};


#endif // __EXTERNAL_OBJECT_WRITER_H__

