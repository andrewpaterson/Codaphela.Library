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
#ifndef __OBJECT_WRITER_CHUNKED_H__
#define __OBJECT_WRITER_CHUNKED_H__
#include "ObjectIO.h"
#include "BaseLib/ChunkFileFileSystem.h"
#include "ExternalObjectWriter.h"


class CChunkFileObjectWriter : public CExternalObjectWriter, public CObjectIO
{
CONSTRUCTABLE(CChunkFileObjectWriter);
DESTRUCTABLE(CChunkFileObjectWriter);
protected:
	CChunkFile				mcChunkFile;
	CChunkFileFileSystem	mcChunkFileFileSystem;
	CChars					mszFilename;
	CChars					mszDirectory;
	CChars					mszObjectBaseName;

public:
	void	Init(char* szDirectory, char* szBaseName, char* szChunkFilename);
	void	Kill(void);

	bool	Begin(void);
	bool	Write(CSerialisedObject* pcSerialised);
	bool	End(void);

protected:
	bool	ObjectStartsWithBaseName(char* szObjectName);
	void	RemainingName(CChars* pszRemainingName, char* szObjectName);
};


#endif // __OBJECT_WRITER_CHUNKED_H__

