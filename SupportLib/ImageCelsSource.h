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
#ifndef __IMAGE_CELS_SOURCE_H__
#define __IMAGE_CELS_SOURCE_H__
#include "StandardLib/Unknown.h"
#include "Image.h"
#include "ImageCelGroup.h"
#include "ImageSource.h"
#include "ImageModifierStack.h"
#include "ImageCelSource.h"


class CImageSourceWithCelSources : public CUnknown
{
BASE_FUNCTIONS(CImageSourceWithCelSources);
protected:
	CImageSource*		mpcImageSource;
	CImageCelSource*	mpcCelsSource;

public:
	void				Init(CImageSource* pcImageSource, CImageCelSource* pcCelsSource);
	void				Kill(void);
	CImageSource*		GetImageSource(void);
	CImageCelSource*	GetCelsSource(void);
};


typedef CArrayType<CImageSourceWithCelSources> CArrayImageSourceWithCelSources;


//This class *does not* add the Cels to a Group because they probably need to be hit with the Image Combiner first.
class CImageCelsSource : public CUnknown
{
BASE_FUNCTIONS(CImageCelsSource);
protected:
	BOOL								mbPackOnLoad;	//This will crop transparent edges as the image is loaded to save memory.
														//Also cropped images don't need the mask image.
	//Input
	CArrayImageSourceWithCelSources		macImageSources;
	CImageModifierStack					mcModifiers;
	CArrayUnknown						macCelSources;

	//Output
	CArrayUnknown						macImageCels;
	CArray								macFillMasks;
	CArray								macImages;

public:
	void 			Init(BOOL bPackOnLoad = FALSE);
	void 			Kill(void);

	void			AddSource(CImageSource* pcImageSource, CImageCelSource* pcCelSource);
	void			AddDiskFileSources(char* szPathName, char* szFileNameContains, char* szImageName, CImageCelSource* pcCelSource);
	void			AddDiskFileSource(char* szFileName, char* szImageName, CImageCelSource* pcCelSource);
	void			AddMemorySource(CImage* pcImage, CImageCelSource* pcCelSource);

	void			AddModifier(CImageModifier* pcModifier);

	BOOL			Load(void);

	CArrayUnknown*	GetCels(void);
	CArrayUnknown*	TakeControlOfCels(void);
	CArray*			TakeControlOfImages(void);
	CArrayUnknown*	GetImageCels(void);

private:
	CImage*			Combine(int iFirstCelIndex);
	void			PopulateImageArray(void);
};


#endif // __IMAGE_CELS_SOURCE_H__

