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
#ifndef __OBJECT_SOURCE_CHUNKED_H__
#define __OBJECT_SOURCE_CHUNKED_H__
#include "BaseLib/ChunkFileFileSystem.h"
#include "ObjectSource.h"


class CExternalObjectReader;
class CChunkFileObjectReader;
class CObjectSourceChunked : public CObjectSource
{
CONSTRUCTABLE(CObjectSourceChunked)
protected:
	CArrayChars					mcNames;
	CChunkFile					mcChunkFile;
	CChunkFileFileSystem		mcChunkFileFileSystem;
	CChunkFileObjectReader*		mpcReader;

public:
	bool			Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, const char* szFilename);
	void			Kill(void) override;

	bool			ReadNames(void);
	bool			Contains(const char* szFullName) override;
	CBaseObject*	Convert(const char* szFullName) override;
	bool			IsNative(void) override;
	bool			IsMultiSource(void) override;

	char*			GetName(size iIndex);
	size			NumNames(void);

	CExternalObjectReader*	GetReader(void) override;
};


#endif // __OBJECT_SOURCE_CHUNKED_H__

