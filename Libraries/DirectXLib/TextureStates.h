/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela DirectXLib

Codaphela DirectXLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela DirectXLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela DirectXLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX and associated libraries are Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef TEXTURE_STATES_H
#define TEXTURE_STATES_H


struct SStageState
{
	int				iUniqueID;
	_D3DTEXTUREOP	iColourOp;
	int				iColourArg1;
	int				iColourArg2;
	_D3DTEXTUREOP	iAlphaOp;
	int				iAlphaArg1;
	int				iAlphaArg2;
	int				iTexCoord;
	int				iResultArg;
};


struct SSamplerState
{
	int						iUniqueID;
	_D3DTEXTUREADDRESS		iAddressU;
    _D3DTEXTUREADDRESS		iAddressV;
    _D3DTEXTUREFILTERTYPE	iMagFilter;
    _D3DTEXTUREFILTERTYPE	iMinFilter;
    _D3DTEXTUREFILTERTYPE	iMipFilter;
    float					fMipmapLodBias;
};


#endif // TEXTURE_STATES_H

