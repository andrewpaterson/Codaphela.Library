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
#ifndef __CHUNK_FILE_OBJECT_READER_H__
#define __CHUNK_FILE_OBJECT_READER_H__
#include "BaseLib/ChunkFileFileSystem.h"
#include "ExternalObjectReader.h"


class CChunkFileObjectReader : public CExternalObjectReader
{
CONSTRUCTABLE(CChunkFileObjectReader);
protected:
	CChunkFileFileSystem*	mpcChunkFile;

public:
	void				Init(CChunkFileFileSystem* mpcChunkFile);
	void				Kill(void);

	CSerialisedObject*	Read(OIndex oi);
	CSerialisedObject*  Read(char* szObjectName);
};


#endif // !__CHUNK_FILE_OBJECT_READER_H__

