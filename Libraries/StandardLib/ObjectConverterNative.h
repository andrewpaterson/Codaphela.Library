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
#ifndef __OBJECT_CONVERTER_NATIVE_H__
#define __OBJECT_CONVERTER_NATIVE_H__
#include "ObjectConverter.h"
#include "ObjectSourceChunked.h"
#include "ObjectSourceSimple.h"
#include "ExternalObjectReader.h"
#include "IndexNewOld.h"


class CObjectSource;
class CObjectConverterNative : public CObjectConverter
{
CONSTRUCTABLE(CObjectConverterNative)
protected:
	CArrayIndexNewOld			mcIndexRemap;  //For testing.

public:
	void			Init(CObjects* pcObjects);
	void			Kill(void) override;

	const char*		GetFileExtension(void) override;
	bool			IsFor(CAbstractFile* pcFile) override;
	CObjectSource*	CreateSource(CAbstractFile* pcFile, const char* szFilename) override;
	CBaseObject*	Convert(CObjectSource* pcSource, const char* szObjectName) override;
	bool			IsNative(void);

	OIndex			TestGetNewIndexFromOld(OIndex oiOld);

protected:
	int32			ReadFileType(CFileBasic* pcFile);
};


#endif // __OBJECT_CONVERTER_NATIVE_H__

