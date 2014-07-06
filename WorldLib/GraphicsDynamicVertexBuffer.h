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
#ifndef __GRAPHICS_DYNAMIC_VERTEX_BUFFER_H__
#define __GRAPHICS_DYNAMIC_VERTEX_BUFFER_H__
#include "GraphicsVertexBuffer.h"
#include "BaseLib/ArrayBlock.h"
#include "BaseLib/ArrayTemplateMinimal.h"

#define DYNAMIC_VERTEX_BUFFER_CHUNK_SIZE	1024


class CGraphicsDynamicVertexBuffer : public CVertexBufferExtended
{
public:
	CArrayBlock	mcVerts;

	void 	Init(void);
	void 	Init(int iD3DVertexType);
	void 	Kill(void);
	void*	AddVertex(void);
	void*	AddVertices(int iNumVerts);
	BOOL	Lock(void);
	BOOL	Unlock(void);
	BOOL	Clear(void);
};


class CGraphicsDynamicVertexBufferArray : public CArrayTemplateMinimal<CGraphicsDynamicVertexBuffer>
{
public:
	void Init(void);
	void Kill(void);

	CGraphicsDynamicVertexBuffer*	AddVertexBuffer(int iD3DVertexType, BOOL bUseExisting);
	CGraphicsDynamicVertexBuffer*	GetVertexBuffer(int iD3DVertexType);
	int								GetVertexBufferIndex(int iD3DVertexType);
	void							RemoveVertexBuffer(int iD3DVertexType);
	void							SetNumVertexBuffers(int iNumBuffers);
};


#endif //__GRAPHICS_DYNAMIC_VERTEX_BUFFER_H__

