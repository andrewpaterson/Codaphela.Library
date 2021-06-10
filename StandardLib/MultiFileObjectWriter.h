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
#ifndef __MULTI_FILE_OBJECT_WRITER_SIMPLE_H__
#define __MULTI_FILE_OBJECT_WRITER_SIMPLE_H__
#include "BaseLib/Chars.h"
#include "ObjectIO.h"
#include "ExternalObjectWriter.h"


class CMultiFileObjectWriter : public CExternalObjectWriter, public CObjectIO
{
CONSTRUCTABLE(CMultiFileObjectWriter);
protected:
	CChars	mszObjectBaseName;
	CChars	mszDirectory;

public:
	void	Init(char* szDirectory, char* szBaseName);
	void	Kill(void);

	BOOL	Begin(void);
	BOOL	Write(CSerialisedObject* pcSerialised);
	BOOL	End(void);

protected:
	BOOL	ObjectStartsWithBaseName(char* szObjectName);
	void	RemainingName(CChars* pszRemainingName, char* szObjectName);
};


#endif // __MULTI_FILE_OBJECT_WRITER_SIMPLE_H__

