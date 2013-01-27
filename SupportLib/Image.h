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
#include "StandardLib/NamedObject.h"
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


//Image shouldn't be aware of CImageAccessor.
class CImageAccessor;


class CImage : public CNamedObject
{
BASE_FUNCTIONS(CImage);
public:
	CChannels				mcChannels;
	int						miWidth;
	int						miHeight;
	SImageChangingDesc*		mpsImageChangingDesc;
	
	void					Init(void);
	void					Init(int iWidth, int iHeight, EPrimitiveTypes eType, EChannel eFirst, ...);
	void					Init(int iWidth, int iHeight, void* pvUserData, EPrimitiveTypes eType, EChannel eFirst, ...);
	void					Init(int iWidth, int iHeight, CImageChannelsSource* pcSource);
	void					Init(int iWidth, int iHeight, void* pvUserData, CImageChannelsSource* pcSource);
	void					Init(int iWidth, int iHeight, CImage* pcChannelsSource);
	void					Init(CImage* pcChannelsSource);  //This only sets up channels and dimensions.  
	void					KillData(void);

	void					BeginChange(void);
	void 					AddChannel(int iChannel, EPrimitiveTypes eType, BOOL bReverse = FALSE);
	void 					AddChannel(int iChannel1, int iChannel2, EPrimitiveTypes eType, BOOL bReverse = FALSE);
	void 					AddChannel(int iChannel1, int iChannel2, int iChannel3, EPrimitiveTypes eType, BOOL bReverse = FALSE);
	void 					AddChannel(int iChannel1, int iChannel2, int iChannel3, int iChannel4, EPrimitiveTypes eType, BOOL bReverse = FALSE);
	void					AddChannels(CImageChannelsSource* pcSource);
	void 					AddChannels(CArrayChannel* pasChannels);
	void 					AddChannels(CImage* pcSourceChannels);
	void					RemoveChannel(int iChannel);
	void					RemovePurpose(EImagePurpose ePurpose);
	void					SetSize(int iWidth, int iHeight);
	int						GetHeight(void);
	int						GetWidth(void);
	void 					ByteAlignChannels(void);
	void					RenameChannel(int iOldName, int iNewName);
	BOOL 					EndChange(void);
	BOOL					IsChanging(void);

	BOOL					Load(CObjectDeserialiser* pcFile);
	BOOL					Save(CObjectSerialiser* pcFile);
	void					Copy(CImage* pcSource);
	void					Clear(void);

	void					SetData(void* pvData);
	void*					GetData(void);
	int						GetByteSize(void);
	CChannels*				GetChannels(void);
	
	BOOL					IsSameFormat(CImage* psOther);
	BOOL					IsValid(int x, int y);
	CChannel*				GetChannel(int iChannel);
	int						GetChannelsCount(void);
	void					GetAllChannels(CArrayInt* paiChannels);
	void					GetAllChannels(CArrayChannel* pasChannels);
	EPrimitiveTypes			GetPrimitiveType(void);  //Returns PT_Undefined if more than one.
	void					GetAllPrimitiveTypes(CArrayInt* paiPrimitiveTypes);
	void					GetChannelsForType(EPrimitiveTypes eType, CArrayInt* paiChannels);

	BOOL					HasChannel(int iChannel);
	BOOL					HasChannels(int iFirst, ...);
};


#endif // __IMAGE_H__

