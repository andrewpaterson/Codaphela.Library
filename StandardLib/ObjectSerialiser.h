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
#ifndef __OBJET_SERIALISER_H__
#define __OBJET_SERIALISER_H__
#include "BaseLib/FileWriter.h"
#include "BaseLib/MemoryFile.h"
#include "BaseObject.h"
#include "ObjectSingleSerialiser.h"
#include "ObjectHeader.h"


class CObjectSerialiser : public CFileWriter
{
protected:
	CBaseObject*				mpcThis;
	CMemoryFile*				mpcMemory;
	CFileBasic					mcFile;
	CObjectSingleSerialiser*	mpcSerialiser;

public:
	BOOL			Init(CObjectSingleSerialiser* pcSerialiser, CBaseObject* pcObject);
	void			Kill(void);
	BOOL			Save(void);

	BOOL			WriteObjectHeader(CObjectHeader* psHeader);
	BOOL			WritePointer(CPointer& pObject);
	BOOL			WritePointer(CPointer* pObject);
	BOOL			WriteIdentifier(CObjectIdentifier* psIdentifier);
	BOOL			WriteDependent(CEmbeddedObject* pcBaseObject);

	void*			GetData(void);
	int				GetLength(void);

	void			InitObjectHeader(CObjectHeader* psHeader, CBaseObject* pcObject);
	void			InitIdentifier(CObjectIdentifier* psHeader, CBaseObject* pcObject);

protected:
	filePos			Write(const void* pvSource, filePos iSize, filePos iCount);
};


#endif // __OBJET_SERIALISER_H__

