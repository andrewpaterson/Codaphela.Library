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
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/FastFunctions.h"
#include "BaseLib/IntegerHelper.h"
#include "GraphicsDynamicVertexBuffer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsDynamicVertexBuffer::Init(void)
{
	CVertexBufferExtended::Init();
	mcVerts.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsDynamicVertexBuffer::Init(int iD3DVertexType)
{
	CVertexBufferExtended::Init();
	iVertexFormat = iD3DVertexType;
	iVertexSize = gcD3D.GetVertexFormatSize(iD3DVertexType);
	mcVerts.Init(iVertexSize);
	
	//Remember this must still be locked before use.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsDynamicVertexBuffer::Kill(void)
{
	CVertexBufferExtended::Kill();
	mcVerts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CGraphicsDynamicVertexBuffer::AddVertex(void)
{
	return AddVertices(1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CGraphicsDynamicVertexBuffer::AddVertices(int iMoreVerts)
{
	int		iOldLength;

	iOldLength = mcVerts.AddNum(iMoreVerts);
	return mcVerts.Get(iOldLength);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGraphicsDynamicVertexBuffer::Clear(void)
{
	if (mcVerts.NumElements() > 0)
	{
		mcVerts.ReInit();
	}
	if (iNumVerticies > 0)
	{
		gcD3D.ReleaseVertexBuffer(this);
	}
	miUsedVerticies = 0;
	iNumVerticies = 0;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGraphicsDynamicVertexBuffer::Lock(void)
{
	if (!(iFlags & Flags_Locked))
	{
		//This is a dynamic vertex buffer which implies lock and discard (Flags_Discard)
		gcD3D.ReleaseVertexBuffer(this);
		miUsedVerticies = 0;
		iNumVerticies = 0;
		mcVerts.ReInit();
		SetFlag(&iFlags, Flags_Locked, TRUE);
		return TRUE;
	}
	else
	{
		gcUserError.Set("Cannot lock an already locked vertex buffer.");
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGraphicsDynamicVertexBuffer::Unlock(void)
{
	if (iFlags & Flags_Locked)
	{
		if (mcVerts.NumElements() == 0)
		{
			SetFlag(&iFlags, Flags_Locked, FALSE);
			return TRUE;
		}

		if (!gcD3D.CreateVertexBuffer(iVertexFormat, mcVerts.NumElements(), TRUE, this))
		{
			miUsedVerticies = 0;
			gcUserError.Set("Could not change vertex buffer size.");
			return FALSE;
		}

		miUsedVerticies = mcVerts.NumElements();
		gcD3D.LockVertexBuffer(this, TRUE);
		if (pvLockedBuffer == NULL)
		{
			return FALSE;
		}

		memcpy_fast(pvLockedBuffer, mcVerts.GetData(), miUsedVerticies * iVertexSize);

		gcD3D.UnlockVertexBuffer(this);
		return TRUE;
	}
	gcUserError.Set("Cannot unlock an already unlocked vertex buffer.");
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsDynamicVertexBufferArray::Init(void)
{
	CArrayTemplateMinimal<CGraphicsDynamicVertexBuffer>::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsDynamicVertexBufferArray::Kill(void)
{
	int								i;
	CGraphicsDynamicVertexBuffer*	psVertexBuffer;

	for (i = 0; i < miUsedElements; i++)
	{
		psVertexBuffer = Get(i);
		psVertexBuffer->Kill();
	}
	CArrayTemplateMinimal<CGraphicsDynamicVertexBuffer>::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsDynamicVertexBuffer* CGraphicsDynamicVertexBufferArray::AddVertexBuffer(int iD3DVertexType, BOOL bUseExisting)
{
	CGraphicsDynamicVertexBuffer*	psVertexBuffer;

	psVertexBuffer = GetVertexBuffer(iD3DVertexType);
	if ((psVertexBuffer) && bUseExisting)
	{
		//A vertex buffer of this type already exists.  Just return it.
		return psVertexBuffer;
	}
	psVertexBuffer = Add();
	if (psVertexBuffer)
	{
		psVertexBuffer->Init(iD3DVertexType);
		return psVertexBuffer;
	}
	gcUserError.Set("Could not allocate new vertex buffer.");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsDynamicVertexBuffer* CGraphicsDynamicVertexBufferArray::GetVertexBuffer(int iD3DVertexType)
{
	int						i;
	CGraphicsDynamicVertexBuffer*	psVertexBuffer;

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
void CGraphicsDynamicVertexBufferArray::RemoveVertexBuffer(int iD3DVertexType)
{
	CGraphicsDynamicVertexBuffer*	psVertexBuffer;
	int						iNum;

	psVertexBuffer = GetVertexBuffer(iD3DVertexType);
	if (psVertexBuffer)
	{
		psVertexBuffer->Kill();
		
		iNum = ((int)((size_t)psVertexBuffer) - (int)((size_t)mpvArray)) / sizeof(CGraphicsDynamicVertexBuffer);
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
void CGraphicsDynamicVertexBufferArray::SetNumVertexBuffers(int iNumBuffers)
{
	int								iOldUsed;
	int								i;
	CGraphicsDynamicVertexBuffer*	psBuffer;

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
int CGraphicsDynamicVertexBufferArray::GetVertexBufferIndex(int iD3DVertexType)
{
	int								i;
	CGraphicsDynamicVertexBuffer*	psVertexBuffer;

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

