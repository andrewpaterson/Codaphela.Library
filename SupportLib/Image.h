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
#ifndef __IMAGE__H__
#define __IMAGE__H__
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/Channels.h"
#include "StandardLib/Array.h"
#include "ImageChannel.h"
#include "Rectangle.h"
#include "ImageColour.h"


#define IMAGE_DATA_FLAGS_DONT_FREE	1
#define IMAGE_DATA_FLAGS_A8R8G8B8	4  //Standard 32 bit colour image which Colour functions will work on.
#define IMAGE_DATA_CHANGING			8
#define USE_DEFAULT_SIZE			0x80000000


struct SImageChangingDesc
{
	int		iWidth;
	int		iHeight;
};


class CImage : public CObject
{
CONSTRUCTABLE(CImage);
DESTRUCTABLE(CImage);
public:
	CChannels				mcChannels;
	int						miWidth;
	int						miHeight;
	SImageChangingDesc*		mpsImageChangingDesc;
	
	Ptr<CImage>				Init(void);
	Ptr<CImage>				Init(int iWidth, int iHeight, EPrimitiveType eType, EChannel eFirst, ...);
	Ptr<CImage>				Init(int iWidth, int iHeight, void* pvUserData, EPrimitiveType eType, EChannel eFirst, ...);
	Ptr<CImage>				Init(int iWidth, int iHeight, CImageChannelsSource* pcSource);
	Ptr<CImage>				Init(int iWidth, int iHeight, void* pvUserData, CImageChannelsSource* pcSource);
	Ptr<CImage>				Init(int iWidth, int iHeight, Ptr<CImage> pcChannelsSource);
	Ptr<CImage>				Init(Ptr<CImage> pcChannelsSource);  //This only sets up channels and dimensions.  
	void					Class(void);
	void					Free(void);

	void					ReInit(void);
	void					ReInit(int iWidth, int iHeight, EPrimitiveType eType, EChannel eFirst, ...);

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void					BeginChange(void);
	void 					AddChannel(size iChannel, EPrimitiveType eType, bool bReverse = false);
	void 					AddChannel(size iChannel, EPrimitiveType eType, char* szShortName, char* szLongName = NULL, bool bReverse = false);
	bool					AddChannel(Ptr<CImage> pcSourceImage, size iChannel, EPrimitiveType eType);
	void 					AddChannel(size iChannel1, size iChannel2, EPrimitiveType eType, bool bReverse = false);
	void 					AddChannel(size iChannel1, size iChannel2, size iChannel3, EPrimitiveType eType, bool bReverse = false);
	void 					AddChannel(size iChannel1, size iChannel2, size iChannel3, size iChannel4, EPrimitiveType eType, bool bReverse = false);
	void					AddChannels(CImageChannelsSource* pcSource);
	void 					AddChannels(CArrayChannel* pasChannels);
	void 					AddChannels(Ptr<CImage> pcSourceChannels);
	bool					RemoveChannel(size iChannel);
	void					RemovePurpose(EImagePurpose ePurpose);
	void					SetSize(int iWidth, int iHeight);
	int						GetHeight(void);
	int						GetWidth(void);
	void 					ByteAlignChannels(void);
	bool					RenameChannel(size iOldName, size iNewName);
	bool 					EndChange(void);
	bool					IsChanging(void);

	void					Copy(Ptr<CImage> pcSource);
	void					Copy2(Ptr<CImage> pcSource);
	void					Clear(void);

	void					SetData(void* pvData);
	void*					GetData(void);
	size					GetByteSize(void);
	CChannels*				GetChannels(void);
	size					GetPixelSize(void);
	
	bool					IsSameFormat(Ptr<CImage> psOther);
	bool					IsValid(int x, int y);
	CChannel*				GetChannel(size iChannel);
	size					GetChannelsCount(void);
	void					GetAllChannels(CArrayInt* paiChannels);
	void					GetAllChannels(CArrayChannel* pasChannels);
	EPrimitiveType			GetPrimitiveType(void);  //Returns PT_Undefined if more than one.
	void					GetAllPrimitiveTypes(CArrayInt* paiPrimitiveTypes);
	void					GetChannelsForType(EPrimitiveType eType, CArrayInt* paiChannels);

	char*					GetChannelLongName(size iChannel);
	char*					GetChannelShortName(size iChannel);
	bool					HasChannel(size iChannel);
	bool					HasChannels(size iFirst, ...);

	void					SetChannelDebugNames(size iChannel);

	void					Print(CChars* psz);
	void					Dump(void);

protected:
	void					PrivateInit(void);
};


#endif // __IMAGE_H__

