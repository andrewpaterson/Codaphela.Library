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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __IMAGE_CELS_SOURCE_H__
#define __IMAGE_CELS_SOURCE_H__
#include "StandardLib/Unknown.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Array.h"
#include "Image.h"
#include "ImageCelGroup.h"
#include "ImageSource.h"
#include "ImageModifierStack.h"
#include "ImageCelSource.h"


class CImageSourceWithCelSources : public CObject
{
CONSTRUCTABLE(CImageSourceWithCelSources);
DESTRUCTABLE(CImageSourceWithCelSources);
protected:
	Ptr<CImageSource>	mpImageSource;
	CImageCelSource*	mpcCelsSource;  //Not owned, whoever added the source keeps it.

public:
	void				Init(Ptr<CImageSource> pImageSource, CImageCelSource* pcCelsSource);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	Ptr<CImageSource>	GetImageSource(void);
	CImageCelSource*	GetCelsSource(void);
};


typedef CArray<CImageSourceWithCelSources> CArrayImageSourceWithCelSources;


//This class *does not* add the Cels to a Group because they probably need to be hit with the Image Combiner first.
class CImageCelsSource : public CObject
{
CONSTRUCTABLE(CImageCelsSource);
DESTRUCTABLE(CImageCelsSource);
protected:
	bool								mbPackOnLoad;	//This will crop transparent edges as the image is loaded to save memory.
														//Also cropped images don't need the mask image.
	//Input
	CArrayImageSourceWithCelSources		maImageSources;
	CImageModifierStack					mcModifiers;
	CArrayUnknown						macCelSources;

	//Output
	CArrayImageCel						maImageCels;
	CArrayImage							maFillMasks;
	CArrayImage							maImages;

public:
	void 					Init(bool bPackOnLoad = false);
	void					Class(void) override;
	void 					Free(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void					AddSource(Ptr<CImageSource> pImageSource, CImageCelSource* pcCelSource);
	void					AddDiskFileSources(char* szPathName, char* szFileNameContains, char* szImageName, CImageCelSource* pcCelSource);
	void					AddDiskFileSource(char* szFilename, char* szImageName, CImageCelSource* pcCelSource);
	void					AddMemorySource(Ptr<CImage> pcImage, CImageCelSource* pcCelSource);

	void					AddModifier(Ptr<CImageModifier> pModifier);

	bool					Load(void);

	Ptr<CArrayImage>		GetImages(void);
	Ptr<CArrayImageCel>		GetImageCels(void);

private:
	Ptr<CImage>				Combine(size iFirstCelIndex);
	void					PopulateImageArray(void);
};


#endif // __IMAGE_CELS_SOURCE_H__

