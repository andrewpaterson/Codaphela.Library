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
#ifndef __GRAPHICS_STATE_H__
#define __GRAPHICS_STATE_H__
#include <d3dx9.h>
#include "DirectXLib/TextureStates.h"
#include "DirectXLib/Constants.h"
#include "StandardLib/StandardHeader.h"


class CGraphicsState : public CUnknown
{
BASE_FUNCTIONS(CGraphicsState);
protected:
	int							miNumTextures;
	int							miNumSamplers;
	SStageState					mcStageState[MAX_TEX];  
	SSamplerState				mcSamplerState[MAX_SAMPLE];

public:
	void 			Init(void);
	void 			Kill(void);
	void 			Draw(void);
	void 			SetNumTextures(int iNum);
	void 			SetNumSamplers(int iNum);

	void 			SetZeroTextureDefault(void);
	void 			SetZeroTextureDefaultPartial(void);
	void 			SetDefaults(int iNumTextureLayers, D3DTEXTUREFILTERTYPE iFilter = D3DTEXF_LINEAR);
	void 			SetDefaultTextureArgs(void);
	void 			SetDefaultSamplerStates(void);
	void			SetDefaultSamplerStates(D3DTEXTUREFILTERTYPE iFilter);

	SSamplerState*	GetSamplerState(int iSamplerState);
	SStageState*	GetStageState(int iStageState);
};


typedef CArrayTemplate<CGraphicsState*>		CGraphicsStatePtrArray;


#endif //__GRAPHICS_STATE_H__

