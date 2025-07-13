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
#ifndef __GRAPHICS_VERTEX_BUFFER_H__
#define __GRAPHICS_VERTEX_BUFFER_H__
#include "DirectXLib/Direct3DGlobal.h"
#include "BaseLib/ArrayTemplateMinimal.h"


//It is NEVER a good idea to change the size of the vertex buffer.  It's size should
//always be set large enough to contain the maximum expected number of vertices.
class CVertexBufferExtended : public SVertexBuffer
{
protected:
	bool PrivateChangeBufferSize(int iNewSize);
	bool PrivateReleaseAndCopy(void* pvDest);

public:
	int miUsedVerticies;
	int miChunkSize;

	void Init(void);
	void Init(int iD3DVertexType, int iNumVerts, bool bDynamic);
	void Kill(void);
	bool Finalise(void);	//Use this when the vertex buffer is unlikely to be changed again.
	bool AppendVerticies(int iNumVerts, void* psVerts);  //Assumes vertices exist and must be copied.
	bool AppendVerticies(int iNumVerts, void** pvFirstNewVert, int* iSizeInBytes = NULL);  //Only allocates memory (buffer).
	bool InsertVerticies(int iNumVerts, int iPos, void* psVerts);	//Vertices are inserted at pos, not after.
	bool RemoveVerticies(int iNumVerts);
};


class CGraphicsVertexBufferArray : public CArrayTemplateMinimal<CVertexBufferExtended>
{
public:
	void Init(void);
	void Kill(void);

	CVertexBufferExtended*	AddVertexBuffer(int iD3DVertexType, int iNumVerts, bool bUseExisting);
	CVertexBufferExtended*	GetVertexBuffer(int iD3DVertexType);
	void					RemoveVertexBuffer(int iD3DVertexType);
	void					SetNumVertexBuffers(size iNumBuffers);
	int						GetVertexBufferIndex(int iD3DVertexType);
};


#endif //__GRAPHICS_VERTEX_BUFFER_H__

