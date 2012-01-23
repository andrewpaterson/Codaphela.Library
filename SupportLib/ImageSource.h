/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __IMAGE_SOURCE_H__
#define __IMAGE_SOURCE_H__
#include "StandardLib/Unknown.h"
#include "Image.h"


//CImageSource destroys the image when it's killed...
//This assumes that the image will be fed to the CImageCombiner which creates a new image.
//If ever you just want to load an image this will be a problem.
//Or you can call TakeControl() which will stop the image from being killed.
class CImageSource : public CUnknown
{
BASE_FUNCTIONS(CImageSource);
protected:
	CChars			mszImageName;
	CImage*			mpcImage;

public:
	void			Init(char* szName = NULL);
	void			Kill(void);

	CImage*			Allocate(void);
	virtual BOOL	Load(void) =0;
	CImage*			TakeControl(void);
	CImage*			GetImage(void);
	void			SetImage(CImage* pcImage);
};


#endif //__IMAGE_SOURCE_H__

