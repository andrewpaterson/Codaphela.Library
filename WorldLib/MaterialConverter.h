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
#ifndef __MATERIAL_CONVERTER_H__
#define __MATERIAL_CONVERTER_H__
#include "BaseLib/ArrayTemplate.h"
#include "SupportLib/Material.h"
#include "WorldLib/GraphicsMaterial.h"
#include "WorldLib/GraphicsState.h"


enum EImageOpacity
{
	IO_Unknown,
	IO_Opaque,
	IO_Transparent,
	IO_Translucent,
};


class CImage;
class CSceneConverter;
class CMaterialConverter
{
protected:
	CSceneConverter*	mpcSceneConverter;
	CMaterial*			mpcMaterial;

public:
	void 					Init(CSceneConverter* pcSceneConverter);
	void 					Kill(void);
	bool 					Convert(CGraphicsMaterial** ppcGraphicsMaterial, CGraphicsState** ppcGraphicsState, CMaterial* pcMaterial);

	bool 					CompleteConversion(CGraphicsMaterial** ppcGraphicsMaterial, CGraphicsState** ppcGraphicsState);
	void 					ConvertSamplerState(SSamplerState* psSamplerState, CMaterialImage* psMaterialImage);
	_D3DTEXTUREADDRESS		ConvertTextureAddress(EUVAddress eUVAddress);
	_D3DTEXTUREFILTERTYPE	ConvertTextureFilterType(EUVFilter eUVFilter);
	void					ConvertMaterialProperties(CGraphicsMaterial* pcGraphicsMaterial, CMaterialProperties* psMaterialPropteries, EImageOpacity eOpacity);
	EImageOpacity			GetOpacity(CImage* pcImage);
	EMaterialOpacity		GetOpacity(int iValue);
	EImageOpacity			UpdateOpacity(EImageOpacity eOldOpacity, CImage* pcImage);
};


#endif //__MATERIAL_CONVERTER_H__

