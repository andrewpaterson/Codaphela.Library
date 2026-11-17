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
#ifndef __NAMED_OBJECT_FILE_H__
#define __NAMED_OBJECT_FILE_H__
#include "BaseObject.h"


//A named object and everything it points to, written to a chunk file of its own and read back into the objects in
//memory, as other objects (CExternalObjectSerialiser and CExternalObjectDeserialiser, on a CChunkFileObjectWriter and a
//CChunkFileSystemObjectReader).  szFilename is the file, with the object files' extension (OBJECT_FILE_EXTENSION)
//whether it's given or not.
//
//Read gives NULL if there's no such file, or nothing of that name in it.  The classes the file holds must have their
//constructors added first (CObjects::AddConstructor).  An object in memory of the same name is replaced.
bool			WriteNamedObjectFile(CBaseObject* pcObject, const char* szFilename);
CBaseObject*	ReadNamedObjectFile(const char* szFilename, const char* szObjectName);


#endif // __NAMED_OBJECT_FILE_H__

