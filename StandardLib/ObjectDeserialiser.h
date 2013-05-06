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
#ifndef __OBJECT_DESERIALISER_H__
#define __OBJECT_DESERIALISER_H__
#include "BaseLib/FileReader.h"
#include "BaseLib/MemoryFile.h"
#include "ObjectAllocator.h"
#include "IndexNewOld.h"
#include "SerialisedObject.h"
#include "Pointer.h"


class CObjectHeader;
class CPointerHeader;
class CObjectDeserialiser : public CFileReader
{
protected:
	CMemoryFile*		mpcMemory;
	CFileBasic			mcFile;
	CObjectAllocator*	mpcAllocator;

public:
			BOOL			Init(CSerialisedObject* pcSerialised, CObjectAllocator* pcAllocator);
			void			Kill(void);
			CPointerObject	Load(OIndex oiNew);

			BOOL			ReadPointer(CPointerObject* pObject);
			BOOL			ReadPointerHeader(CPointerHeader* pcPointerHeader);
			BOOL			ReadObjectHeader(CObjectHeader* pcObjectHeader);
			BOOL			ReadDependent(CBaseObject** ppcObjectPtr, CBaseObject* pcContaining);

protected:
			filePos			Read(void* pvDest, filePos iSize, filePos iCount);
			void			ClearPointer(CPointerObject* pObject);
	virtual BOOL			AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining) =0;
	virtual void			AddIndexRemap(OIndex oiNew, OIndex oiOld);
};


#endif // __OBJECT_DESERIALISER_H__

