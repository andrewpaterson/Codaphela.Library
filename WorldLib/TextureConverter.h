/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __TEXTURE_CONVERTER_H__
#define __TEXTURE_CONVERTER_H__
#include "BaseLib/ArrayTemplate.h"
#include "SupportLib/ImageCopier.h"
#include "SupportLib/Scene.h"
#include "WorldLib/GraphicsTexture.h"


class CWorld;
class CImage;
class CSceneConverter;
class CTextureConverter
{
protected:
	CSceneConverter*	mpcSceneConverter;
	CWorld*				mpcWorld;

public:
	void 				Init(CSceneConverter* pcSceneConverter);
	void 				Init(CWorld* pcWorld);
	void 				Kill(void);
	BOOL				Convert(CGraphicsTexture** ppcGraphicsTexture, D3DFORMAT d3dFormat, CImageCopier* pcExport, BOOL bDiscard);
	BOOL				Convert(CGraphicsTexture** ppcGraphicsTexture, D3DFORMAT d3dFormat, CImage* pcImage, CImage* pcExport, BOOL bDiscard);
	BOOL				Convert(CGraphicsTexture** ppcGraphicsTexture, CImage* pcImage, BOOL bDiscard);

private:
	void				SetExternalChannel(CImage* pcExport, D3DFORMAT d3dFormat);
};


#endif // __TEXTURE_CONVERTER_H__

