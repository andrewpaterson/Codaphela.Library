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
#ifndef __GRAPHICS_MATERIAL_H__
#define __GRAPHICS_MATERIAL_H__
#include "BaseLib/ArraySimple.h"
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/StandardHeader.h"
#include "SupportLib/ImageColour.h"
#include "DirectXLib/Direct3DGlobal.h"


struct SRenderState
{
	D3DRENDERSTATETYPE	iD3DRenderState;
	DWORD				dwValue;
	DWORD				dwPrior;
};


typedef CArraySimple<SRenderState>	CRenderStateArray;


class CGraphicsTexture;
class CGraphicsMaterial : public CUnknown
{
BASE_FUNCTIONS(CGraphicsMaterial);
protected:
	SMaterial			msMaterial;
	CGraphicsTexture*	mapsTexture[MAX_TEX];
	int					miNumTextures;
	CRenderStateArray	masRenderStates;

public:
	void 				Init(void);
	void 				Kill(void);
	void 				Draw(void);
	void 				SetRenderState(D3DRENDERSTATETYPE iD3DRenderState, DWORD dwValue);
	void 				SetRenderStates(void);
	void 				UnsetRenderStates(void);
	void 				SetTexture(int iTexureNum, CGraphicsTexture* psTexture);

	void 				SetGreyDefaults(int iNumTextures);
	void 				SetWhiteDefaults(int iNumTextures);

	CGraphicsTexture*	GetTexture(int iTexureNum);
	int					GetNumTextures(void);
	void				SetNumTextures(int iNumTextures);
	void				SetDiffuse(CImageColourARGB* psfColour);
	void 				SetAmbient(CImageColourARGB* psfColour);
	void 				SetSpecular(CImageColourARGB* psfColour);
	void 				SetEmissive(CImageColourARGB* psfColour);
	void				SetPower(float fPower);
	void				ModulateSpecular(float fValue);
};


typedef CArrayTemplate<CGraphicsMaterial*>	CGraphicsMaterialPtrArray;


#endif // __GRAPHICS_MATERIAL_H__

