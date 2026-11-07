/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __OBJECT_CONVERTER_IMAGE_H__
#define __OBJECT_CONVERTER_IMAGE_H__
#include "StandardLib/ObjectConverter.h"


//Converts image files (every type ReadImage reads) into CImages.  The object name is the file's path (for example
//"Input/Resources/GrassTexture.png") and the image is named it, so it goes into the Codabase like any named object.
class CObjectConverterImage : public CObjectConverter
{
CONSTRUCTABLE(CObjectConverterImage)
public:
	void			Init(CObjects* pcObjects);
	void			Kill(void);

	const char*		GetFileExtension(void);
	bool			IsFor(CAbstractFile* pcFile);
	CObjectSource*	CreateSource(CAbstractFile* pcFile, const char* szFilename);
	CBaseObject*	Convert(CObjectSource* pcSource, const char* szObjectName);
};


#endif // __OBJECT_CONVERTER_IMAGE_H__

