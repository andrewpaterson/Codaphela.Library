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
#include "BaseLib/PointerFunctions.h"
#include "SupportLib/ColourARGB.h"
#include "SupportLib/ImageResampler.h"
#include "SupportLib/Image.h"
#include "SceneConverter.h"
#include "TextureConverter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextureConverter::Init(CSceneConverter* pcSceneConverter)
{
	mpcSceneConverter = pcSceneConverter;
	mpcWorld = mpcSceneConverter->GetWorld();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextureConverter::Init(CWorld* pcWorld)
{
	mpcSceneConverter = NULL;
	mpcWorld = pcWorld;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextureConverter::Kill(void)
{
	mpcSceneConverter = NULL;
	mpcWorld = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextureConverter::Convert(CGraphicsTexture** ppcGraphicsTexture, D3DFORMAT d3dFormat, Ptr<CImage> pImage, Ptr<CImage> pExport, BOOL bDiscard)
{
	CImageCopier		cCopier;
	BOOL				bResult;

	cCopier.Init(pImage, pExport);
	pExport->SetID(pImage->GetOI());
	bResult = Convert(ppcGraphicsTexture, d3dFormat, &cCopier, bDiscard);
	cCopier.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextureConverter::Convert(CGraphicsTexture** ppcGraphicsTexture, D3DFORMAT eFormat, CImageCopier* pcCopier, BOOL bDiscard)
{
	CGraphicsTexture*		pcGraphicsTexture;
	int*					paiPixels;
	CChars					sz;

	if (!pcCopier)
	{
		sz.Init("Can't convert using a NULL exporter.");
		gcUserError.Set(sz.Text());
		sz.Kill();
		return FALSE;
	}

	if ((mpcSceneConverter) && (mpcSceneConverter->GetMapper()->GetImage(pcCopier->mpcSource->GetOI(), eFormat, &pcGraphicsTexture)))
	{
		return TRUE;
	}

	pcGraphicsTexture = mpcWorld->CreateGraphicsTexture(pcCopier->mpcDest->GetWidth(), pcCopier->mpcDest->GetHeight(), eFormat, TU_AutoMipmap, FALSE);

	SSurface			sSource;
	SSurface			sDest;

	gcD3D.GetSurface(pcGraphicsTexture->GetTexture(), 0, &sDest);
	//pcGraphicsTexture->GetTexture()->mpD3DTexture->GetSurfaceLevel(0, &psSurface);

	gcD3D.CreateSurface(pcCopier->mpcDest->GetWidth(), pcCopier->mpcDest->GetHeight(), eFormat, TU_SystemMem, &sSource);
	paiPixels = (int*)gcD3D.LockSurface(&sSource);

	if (paiPixels)
	{
		pcCopier->mpcDest->SetData(paiPixels);
		pcCopier->Copy(0, 0);

		if ((mpcSceneConverter) && (pcCopier->mpcDest->GetOI() != -1))
		{
			mpcSceneConverter->GetMapper()->AddImage(pcCopier->mpcDest->GetOI(), eFormat, pcGraphicsTexture);
		}

		gcD3D.UnlockSurface(&sSource);
		gcD3D.UpdateSurface(sSource.mpD3DSurface, sDest.mpD3DSurface);
		gcD3D.ReleaseSurface(&sSource);
		SafeAssign(ppcGraphicsTexture, pcGraphicsTexture);
		return TRUE;
	}
	else
	{
		pcGraphicsTexture->Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextureConverter::Convert(CGraphicsTexture** ppcGraphicsTexture, Ptr<CImage> pImage, BOOL bDiscard)
{
	D3DFORMAT			d3dFormat;
	CImage				cExport;
	CImageCopier		cCopier;
	BOOL				bResult;

	if (!pImage)
	{
		gcUserError.Set("Can't convert NULL Image.");
		return NULL;
	}

	d3dFormat = gcD3D.acMonitor[gcD3D.iCurrMon].iTextureFormat;
	if ((mpcSceneConverter) && (mpcSceneConverter->GetMapper()->GetImage(pImage->GetOI(), d3dFormat, ppcGraphicsTexture)))
	{
		return TRUE;
	}

	cExport.Init();
	cExport.BeginChange();
	SetExternalChannel(&cExport, d3dFormat);
	cExport.SetSize(pImage->GetWidth(), pImage->GetHeight());
	cExport.SetData((void*)1);
	cExport.EndChange();

	cCopier.Init(pImage, &cExport);
	bResult = Convert(ppcGraphicsTexture, d3dFormat, &cCopier, bDiscard);
	cCopier.Kill();
	cExport.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextureConverter::SetExternalChannel(Ptr<CImage> pcExport, D3DFORMAT d3dFormat)
{
	if (d3dFormat == D3DFMT_A8R8G8B8)
	{
		pcExport->AddChannel(
			IMAGE_DIFFUSE_BLUE,
			IMAGE_DIFFUSE_GREEN,
			IMAGE_DIFFUSE_RED, 
			IMAGE_OPACITY,
			PT_uchar);
	}
	else if (d3dFormat == D3DFMT_X8R8G8B8)
	{
		pcExport->AddChannel(
			IMAGE_DIFFUSE_BLUE,
			IMAGE_DIFFUSE_GREEN,
			IMAGE_DIFFUSE_RED, 
			IMAGE_IGNORED,
			PT_uchar);
	}
	else if (d3dFormat == D3DFMT_A8B8G8R8)
	{
		pcExport->AddChannel(
			IMAGE_DIFFUSE_RED, 
			IMAGE_DIFFUSE_GREEN, 
			IMAGE_DIFFUSE_BLUE, 
			IMAGE_OPACITY, 
			PT_uchar);
	}
	else if (d3dFormat == D3DFMT_R8G8B8)
	{
		pcExport->AddChannel(
			IMAGE_DIFFUSE_RED, 
			IMAGE_DIFFUSE_GREEN, 
			IMAGE_DIFFUSE_BLUE, 
			PT_uchar);
	}

	else if (d3dFormat == D3DFMT_R5G6B5)
	{
		pcExport->AddChannel(IMAGE_DIFFUSE_BLUE, PT_nickle);
		pcExport->AddChannel(IMAGE_DIFFUSE_GREEN, PT_sixbits);
		pcExport->AddChannel(IMAGE_DIFFUSE_RED, PT_nickle);
	}
	else if (d3dFormat == D3DFMT_X1R5G5B5)
	{
		pcExport->AddChannel(IMAGE_DIFFUSE_BLUE, PT_nickle);
		pcExport->AddChannel(IMAGE_DIFFUSE_GREEN, PT_nickle);
		pcExport->AddChannel(IMAGE_DIFFUSE_RED, PT_nickle);
		pcExport->AddChannel(IMAGE_IGNORED, PT_bit);
	}
	else if (d3dFormat == D3DFMT_A1R5G5B5)
	{
		pcExport->AddChannel(IMAGE_DIFFUSE_BLUE, PT_nickle);
		pcExport->AddChannel(IMAGE_DIFFUSE_GREEN, PT_nickle);
		pcExport->AddChannel(IMAGE_DIFFUSE_RED, PT_nickle);
		pcExport->AddChannel(IMAGE_OPACITY, PT_bit);
	}
	else if (d3dFormat == D3DFMT_A4R4G4B4)
	{
		pcExport->AddChannel( 
			IMAGE_DIFFUSE_BLUE, 
			IMAGE_DIFFUSE_GREEN, 
			IMAGE_DIFFUSE_RED, 
			IMAGE_OPACITY, 
			PT_nybble);
	}
	else if (d3dFormat == D3DFMT_R3G3B2)
	{
	}
	else if (d3dFormat == D3DFMT_A8)
	{
		pcExport->AddChannel(IMAGE_OPACITY, PT_uchar);
	}
	else if (d3dFormat == D3DFMT_A8R3G3B2)
	{
	}
	else if (d3dFormat == D3DFMT_X4R4G4B4)
	{
		pcExport->AddChannel( 
			IMAGE_DIFFUSE_BLUE, 
			IMAGE_DIFFUSE_GREEN, 
			IMAGE_DIFFUSE_RED, 
			IMAGE_IGNORED,
			PT_nybble);
	}
	else if (d3dFormat == D3DFMT_A2B10G10R10)
	{
	}
	else if (d3dFormat == D3DFMT_X8B8G8R8)
	{
		pcExport->AddChannel(
			IMAGE_DIFFUSE_RED, 
			IMAGE_DIFFUSE_GREEN, 
			IMAGE_DIFFUSE_BLUE, 
			IMAGE_IGNORED,
			PT_uchar);
	}
	else if (d3dFormat == D3DFMT_G16R16)
	{
		pcExport->AddChannel( 
			IMAGE_DIFFUSE_RED, 
			IMAGE_DIFFUSE_GREEN, 
			PT_ushort);
	}
	else if (d3dFormat == D3DFMT_A2R10G10B10)
	{
	}
	else if (d3dFormat == D3DFMT_A16B16G16R16)
	{
		pcExport->AddChannel( 
			IMAGE_DIFFUSE_RED, 
			IMAGE_DIFFUSE_GREEN, 
			IMAGE_DIFFUSE_BLUE, 
			IMAGE_OPACITY,
			PT_ushort);
	}
	else if (d3dFormat == D3DFMT_A8P8)
	{
	}
	else if (d3dFormat == D3DFMT_P8)
	{
	}
}

