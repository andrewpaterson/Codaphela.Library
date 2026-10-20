/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __IMAGE_COMBINER_H__
#define __IMAGE_COMBINER_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/MapStringInt.h"
#include "StandardLib/ArrayUnknown.h"
#include "Image.h"
#include "ImageCel.h"
#include "RectanglePacker.h"


enum EImageCombineLayout
{
	ICL_Best,
	ICL_LeftToRight,
	ICL_TopToBottom,
};


enum EImageCombineSize
{
	ICS_PowerOf2,
	ICS_Arbitrary,
	ICS_UserSpecified,
};

enum EImageCombineChannels
{
	ICC_RGB8bit,
	ICC_ARGB8bit,
	ICC_RGBA8bit,
	ICC_FromCels,
	ICC_UserSpecified,
};


class CImageCombiner : public CObject
{
CONSTRUCTABLE(CImageCombiner);
DESTRUCTABLE(CImageCombiner);
public:
	EImageCombineLayout		meLayout;
	EImageCombineSize		meSize;
	EImageCombineChannels	meChannels;
	SInt32Vec2				msSize;  //Only if ICS_UserSpecified.
	Ptr<CImage>				mpcDestImage;  //Assumed to be uninitialised.
	CArrayImageCel			mcSourceCels;
	CArrayChars				maszSourceFileNames;  //One for each source cel.  Empty if the cel has no file.
	CArrayImageCel			mcDestCels;  //Theres a one-to-one between dest and source cells.
	int						miOutsideEdgeWidth;  //Gap around the outside of the destination image and the rectangles.
	int						miInnerEdgeWidth;  //Gap between rectangles.
	CArrayChannel			masChannels;
	bool					mbNameCels;  //Name the dest cels from the source file names.

	void					Init(EImageCombineLayout eLayout, int iWidth, int iHeight, EImageCombineSize eSize, EImageCombineChannels eChannels, int iOutsideEdgeWidth, int iInnerEdgeWidth, bool bNameCels = false);
	void 					Init(EImageCombineLayout eLayout, EImageCombineSize eSize, EImageCombineChannels eChannels = ICC_FromCels, int iOutsideEdgeWidth = 0, int iInnerEdgeWidth = 0, bool bNameCels = false);  //Don't pass ICS_UserSpecified in here, use the one below.
	void 					Init(EImageCombineLayout eLayout, int iWidth, int iHeight, EImageCombineChannels eChannels = ICC_FromCels, int iOutsideEdgeWidth = 0, int iInnerEdgeWidth = 0, bool bNameCels = false);
	void					Class(void) override;
	void 					Free(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void 					AddCel(Ptr<CImageCel> pcCel, char* szFileName = NULL);
	void 					AddCels(Ptr<CArrayImageCel> pcCels, CArrayChars* paszFileNames = NULL);  //paszFileNames has one file name for each cel.
	void					AddChannel(EChannel eChannel, EPrimitiveType eType);
	Ptr<CImage>				Combine(void);
	Ptr<CArrayImageCel>		GetCels(void);  //If something else is taking ownership of the cels (the normal case) then bKillDestCels must be false.

private:
	CRectanglePacker*	GetPacker(void);
	Ptr<CImage>			InitiailiseDestImage(SInt32Vec2 sSize);
	void				CalculateChannels(void);
	void				CalculateChannelsFromCels(void);
	void				UpdateChannels(CArrayChannel* pasSource);
	SInt32Vec2			Pack(CArrayPackedRectangle* pacPackedRects);
	void				Draw(CArrayPackedRectangle* pacPackedRects);
	bool				CreateDestCels(CArrayPackedRectangle* pacPackedRects);
	void				CountFileNames(CMapStringInt* pmFileNameCounts);
	void				MakeCelName(CChars* szName, size iSourceIndex, CMapStringInt* pmFileNameCounts, CMapStringInt* pmFileNameIndices);
	CPackedRectangle*	GetPackedRectangle(CArrayPackedRectangle* pacPackedRects, Ptr<CImageCel> pcSourceCel);
};


#endif // __IMAGE_COMBINER_H__

