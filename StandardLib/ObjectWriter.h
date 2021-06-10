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
#ifndef __OBJET_WRITER_H__
#define __OBJET_WRITER_H__
#include "BaseLib/FileWriter.h"
#include "BaseLib/MemoryFile.h"
#include "BaseObject.h"
#include "ObjectHeader.h"
#include "DependentWriteObjects.h"


class CObjectWriter : public CFileWriter
{
protected:
	CMemoryFile*				mpcMemory;
	CFileBasic					mcFile;
	CDependentWriteObjects*		mpcDependentObjects;

public:
	BOOL			Init(CDependentWriteObjects* pcDependentObjects);
	void			Kill(void);

	BOOL			Save(CBaseObject* pcThis);

	void*			GetData(void);
	int				GetLength(void);

	BOOL			WritePointer(CPointer& pObject);
	BOOL			WritePointer(CPointer* pObject);
	BOOL			WriteDependent(CEmbeddedObject* pcBaseObject);

protected:
	BOOL			WriteObjectHeader(CObjectHeader* psHeader);
	BOOL			WriteIdentifier(CObjectIdentifier* psIdentifier);

	void			InitObjectHeader(CObjectHeader* psHeader, CBaseObject* pcObject);
	void			InitIdentifier(CObjectIdentifier* psHeader, CBaseObject* pcObject);

	filePos			Write(const void* pvSource, filePos iSize, filePos iCount);
};


#endif // __OBJET_WRITER_H__

