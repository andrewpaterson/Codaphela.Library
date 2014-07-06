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
#include "GraphicsVertexBuffer.h"
#include "BaseLib/FastFunctions.h"
#include "BaseLib/ErrorHandler.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/PointerFunctions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVertexBufferExtended::Init(void)
{
	iNumVerticies = 0;
	iVertexFormat = 0;
	iVertexSize = 0;
	lpD3DVertexBuffer = NULL;
	pvLockedBuffer = NULL;
	miUsedVerticies = 0;
	miChunkSize = 64;
	iFlags = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVertexBufferExtended::Init(int iD3DVertexType, int iNumVerts, BOOL bDynamic)
{
	gcD3D.CreateVertexBuffer(iD3DVertexType, iNumVerts, bDynamic, this);
	miUsedVerticies = 0;
	miChunkSize = 64;
	AppendVerticies(iNumVerts, NULL, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVertexBufferExtended::Kill(void)
{
	gcD3D.ReleaseVertexBuffer(this);
	miUsedVerticies = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CVertexBufferExtended::Finalise(void)
{
	return PrivateChangeBufferSize(miUsedVerticies);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CVertexBufferExtended::AppendVerticies(int iNumVerts, void* psVerts)
{
	void*	psPos;
	int		iByteSize;

	AppendVerticies(iNumVerts, &psPos, &iByteSize);

	//There are no vertex formats which are not 4 byte aligned.
	memcpy_fast(psPos, psVerts, iByteSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CVertexBufferExtended::InsertVerticies(int iNumVerts, int iPos, void* psVerts)
{
	void*	psStartPos;
	int		iMovedVerts;
	void*	psTrailingPos;
	void*	psIgnored;
	int		iByteSize;

	iMovedVerts = miUsedVerticies - iPos;	
	
	AppendVerticies(iNumVerts, &psIgnored, &iByteSize);

	//First move the origional verts.  This will invalidate any index buffers.
	psStartPos = RemapSinglePointer(pvLockedBuffer, iPos * iVertexSize);
	psTrailingPos = RemapSinglePointer(pvLockedBuffer, (iPos + iNumVerts) * iVertexSize);

	//Move the existing trailing verticies to the new end.
	memcpy_fast_backwards(psTrailingPos, psStartPos, iMovedVerts * iVertexSize);

	//Now copy the new verts into the empty spaces.
	memcpy_fast(psStartPos, psVerts, iByteSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CVertexBufferExtended::AppendVerticies(int iNumVerts, void** pvFirstNewVert, int* iSizeInBytes)
{
	int		iNeededVerts;
	void*	psBuffer;
	int		iOldUsedVerts;
	int		iNumAllocations;
	int 	iNumLeftOvers;

	//Change the size of the buffer if needed.
	iNeededVerts = miUsedVerticies + iNumVerts;
	iOldUsedVerts = miUsedVerticies;
	if (iNeededVerts > iNumVerticies)
	{
		iNumAllocations = iNeededVerts / miChunkSize;
		iNumLeftOvers = iNeededVerts % miChunkSize;

		if (iNumLeftOvers > 0)
		{
			iNumAllocations++;
		}

		if (!PrivateChangeBufferSize(iNumAllocations * miChunkSize))
		{
			return FALSE;
		}
	}
	psBuffer = gcD3D.LockVertexBuffer(this);
	if (!psBuffer)
	{
		gcUserError.Set("Could not lock vertex buffer.");
		return FALSE;
	}

	miUsedVerticies += iNumVerts;
	SafeAssign(iSizeInBytes, iNumVerts * iVertexSize);
	if (pvFirstNewVert)
	{
		*pvFirstNewVert = RemapSinglePointer(psBuffer, iOldUsedVerts * iVertexSize);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CVertexBufferExtended::RemoveVerticies(int iNumVerts)
{
	miUsedVerticies -= iNumVerts;
	if (miUsedVerticies < 0)
	{
		miUsedVerticies = 0;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CVertexBufferExtended::PrivateChangeBufferSize(int iNewSize)
{
	SVertexBuffer	sNewVertBuffer;
	void*			psNewVerts;

	//Don't change the buffer if it is not needed.
	if (iNewSize == iNumVerticies)
	{
		return TRUE;
	}

	if (!gcD3D.CreateVertexBuffer(iVertexFormat, iNewSize, (iFlags & Flags_Dynamic), &sNewVertBuffer))
	{
		gcUserError.Set("Could not change vertex buffer size.");
		return FALSE;
	}

	//Get where the verts are in memory.
	psNewVerts = gcD3D.LockVertexBuffer(&sNewVertBuffer);
	if (!psNewVerts)
	{
		gcUserError.Set("Could not change vertex buffer size.");
		return FALSE;
	}
	
	//Find how many need copying.
	if (miUsedVerticies > iNewSize)
	{
		miUsedVerticies = iNewSize;
	}

	//Release the old vertex buffer and copy it into the new one.
	PrivateReleaseAndCopy(psNewVerts);

	//Use the new vertex buffer.
	memcpy(this, &sNewVertBuffer, sizeof(SVertexBuffer));
	gcD3D.UnlockVertexBuffer(this);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CVertexBufferExtended::PrivateReleaseAndCopy(void* pvDest)
{
	void*	psOldVerts;

	if (iNumVerticies == 0)
	{
		//A vertex buffer cannot be created with 0 verts so there is nothing to release.
		return TRUE;
	}

	psOldVerts = gcD3D.LockVertexBuffer(this);
	if (!psOldVerts)
	{
		return FALSE;
	}

	//Copy them.
	memcpy_fast(pvDest, psOldVerts,	miUsedVerticies * iVertexSize);

	//Destroy the old vertex buffer.
	gcD3D.UnlockVertexBuffer(this);  //Not sure if this is necessary or if it will cause slowdown.
	gcD3D.ReleaseVertexBuffer(this);
	iNumVerticies = 0;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsVertexBufferArray::Init(void)
{
	CArrayTemplateMinimal<CVertexBufferExtended>::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsVertexBufferArray::Kill(void)
{
	int						i;
	CVertexBufferExtended*	psVertexBuffer;

	for (i = 0; i < miUsedElements; i++)
	{
		psVertexBuffer = Get(i);
		psVertexBuffer->Kill();
	}
	CArrayTemplateMinimal<CVertexBufferExtended>::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////s////////////////////////////////////////////////
CVertexBufferExtended* CGraphicsVertexBufferArray::AddVertexBuffer(int iD3DVertexType, int iNumVerts, BOOL bUseExisting)
{
	CVertexBufferExtended*	psVertexBuffer;
	void*					pvFirstNewVert;
	int						iSizeInBytes;
	BOOL					bResult;

	if (!iNumVerts)
	{
		return NULL;
	}

	psVertexBuffer = GetVertexBuffer(iD3DVertexType);
	if ((psVertexBuffer) && bUseExisting)
	{
		bResult = psVertexBuffer->AppendVerticies(iNumVerts, &pvFirstNewVert, &iSizeInBytes);
		if (bResult)
		{
			return psVertexBuffer;
		}
		else
		{
			gcUserError.Set("Could not allocate new vertex buffer.");
			return NULL;
		}
	}
	else
	{
		psVertexBuffer = Add();
		if (psVertexBuffer)
		{
			psVertexBuffer->Init(iD3DVertexType, iNumVerts, FALSE);
			return psVertexBuffer;
		}
		else
		{
			gcUserError.Set("Could not allocate new vertex buffer.");
			return NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CVertexBufferExtended* CGraphicsVertexBufferArray::GetVertexBuffer(int iD3DVertexType)
{
	int						i;
	CVertexBufferExtended*	psVertexBuffer;

	for (i = 0; i < miUsedElements; i++)
	{
		psVertexBuffer = Get(i);
		if (psVertexBuffer->iVertexFormat == iD3DVertexType)
		{
			return psVertexBuffer;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsVertexBufferArray::RemoveVertexBuffer(int iD3DVertexType)
{
	CVertexBufferExtended*	psVertexBuffer;
	int						iNum;

	psVertexBuffer = GetVertexBuffer(iD3DVertexType);
	if (psVertexBuffer)
	{
		psVertexBuffer->Kill();
		
		iNum = ((int)((size_t)psVertexBuffer) - (int)((size_t)mpvArray)) / sizeof(CVertexBufferExtended);
		RemoveAt(iNum);  //The order is not preserved which means that anything indexing
						 //miUsedElements must now index iNum instead.
		
		//Destroy the removed vertex buffer.
		gcD3D.ReleaseVertexBuffer(psVertexBuffer);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsVertexBufferArray::SetNumVertexBuffers(int iNumBuffers)
{
	int						iOldUsed;
	int						i;
	CVertexBufferExtended*	psBuffer;

	if (iNumBuffers != miUsedElements)
	{
		iOldUsed = miUsedElements;
		if (iNumBuffers > miUsedElements)
		{
			SetArraySize(iNumBuffers);

			//Initialise all the new vertex buffers to dud values.
			for (i = iOldUsed; i < miUsedElements; i++)		
			{
				psBuffer = Get(i);
				psBuffer->Init();
			}
		}
		else
		{
			//Free all the vertex buffers which will no longer exist.
			for (i = iNumBuffers; i < iOldUsed; i++)
			{
				psBuffer = Get(i);
				psBuffer->Kill();
			}
			SetArraySize(iNumBuffers);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsVertexBufferArray::GetVertexBufferIndex(int iD3DVertexType)
{
	int						i;
	CVertexBufferExtended*	psVertexBuffer;

	for (i = 0; i < miUsedElements; i++)
	{
		psVertexBuffer = Get(i);
		if (psVertexBuffer->iVertexFormat == iD3DVertexType)
		{
			return i;
		}
	}
	return -1;
}

