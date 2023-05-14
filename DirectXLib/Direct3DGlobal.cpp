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
#include <stdio.h> // For sprintf
#include "Direct3DGlobal.h"
#include "BaseLib/ErrorHandler.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/FloatHelper.h"
#include "BaseLib/Logger.h"
#include "BaseLib/Numbers.h"
#include "FlexibleVertexFormats.h"
#include "BaseLib/GeometricTypes.h"
#include "SupportLib/ColourARGB.h"


CD3D	gcD3D;
#include "BaseLib/Timer.h"


#define ReturnOnFailure(hResult, message) \
	if (FAILED(hResult))\
{\
	gcUserError.Set("Could not " message);\
	return;\
}


#define ReturnNULLOnFailure(hResult, message) \
	if (FAILED(hResult))\
{\
	gcUserError.Set("Could not " message);\
	return NULL;\
}


#define D3D_CASE(cas,strng)	case cas: strng->Append(#cas); break
#define D3D_OR(cas,var,strng)	if (var & cas) strng->Append(#cas##"|")


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* SMatrix::At(void)
{
	return sD3DMatrix.At();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* SMatrix::Up(void)
{
	return sD3DMatrix.Up();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* SMatrix::Across(void)
{
	return sD3DMatrix.Across();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* SMatrix::Pos(void)
{
	return sD3DMatrix.Pos();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SMaterial::Zero(void)
{
	memset(this, 0, sizeof(SMaterial));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSurface::Zero(void)
{
	memset(this, 0, sizeof(SSurface));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SMatrix::Zero(void)
{
	memset(this, 0, sizeof(SMatrix));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STexture::Zero(void)
{
	memset(this, 0, sizeof(STexture));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SViewport::Zero(void)
{
	memset(this, 0, sizeof(SViewport));
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::Init(void)
{
	lpD3D = NULL;
	lpD3DDevice = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CD3D::Init(D3DDEVTYPE d)
{
	char			szText[256];

	mbAlwaysSetMaterial = FALSE;
	mbAlwaysSetTexture = FALSE;
	mbAlwaysSetVertexBuffer = FALSE;
	mbAlwaysSetIndexBuffer = FALSE;
	mbAlwaysSetViewport = FALSE;

	mbLogCalls = FALSE;
	mszLog.Init();
	miLogFlushSize = 1;

	meDevType = d;
	//Zero the current device.
	lpD3DDevice = NULL;

	//Create the main Direct3D object. 
	lpD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (lpD3D == NULL)
	{
		gcUserError.Set("Cannot initialise DirectX9");
		return FALSE;
	}

	if (d == D3DDEVTYPE_SW)
	{
		//Find out how to register the old RAMP rasterizer...
		lpD3D->RegisterSoftwareDevice(NULL);
		return FALSE;
	}

	//Get the number of video cards in this system.
	iNumMonitors = lpD3D->GetAdapterCount();

	sprintf(szText, "Number of display adapter's [%i]\n", iNumMonitors);
	gcLogger.Add(szText);

	iCurrMon = -1;

	cD3DVertTypes.Init();
	RegisterAllFlexibleFormats();

	if (iNumMonitors == 0)
	{
		gcUserError.Set("No DirectX devices available!");
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::GetNVidiaPerfHUDAdapter(int iDefaultAd)
{
	D3DADAPTER_IDENTIFIER9  sIdentifier; 
	HRESULT					hRes; 

	for (unsigned int Adapter = 0; Adapter < lpD3D->GetAdapterCount(); Adapter++)  
	{ 
		hRes = lpD3D->GetAdapterIdentifier(Adapter, 0, &sIdentifier); 
		if (strstr(sIdentifier.Description,"NVPerfHUD") != 0) 
		{ 
			meDevType = D3DDEVTYPE_REF;
			return Adapter;
		} 
	} 
	return iDefaultAd;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CD3D::InitDevice(int ad, HWND hWnd, unsigned int dx, unsigned int dy, D3DFORMAT fmt, BOOL bStencil)
{
	D3DFORMAT				w;
	D3DPRESENT_PARAMETERS	dp;
	D3DDISPLAYMODE			dmc;
	int						iBehaviourFlags;
	char					s[512];
	int						nvAd;
	HRESULT					hResult;

	sprintf(s, "Initialising display adapter [%i]\n", ad);
	gcLogger.Add(s);

	acMonitor[ad].hWnd = hWnd;
	acMonitor[ad].mpsCurrentIndexBuffer = NULL;
	acMonitor[ad].mpsCurrentVertexBuffer = NULL;
	acMonitor[ad].mpsCurrentMaterial = NULL;

	SetupCache(ad);

	acMonitor[ad].iFrameBufferFormat = D3DFMT_UNKNOWN;
	acMonitor[ad].iDepthBufferFormat = D3DFMT_UNKNOWN;


	//Check if hardware or software vertex processing.
	iBehaviourFlags = GetDeviceBehaviour(ad);
	if (iBehaviourFlags == 0)
	{
		return FALSE;
	}

	nvAd = GetNVidiaPerfHUDAdapter(ad);

	if ((dx == 0) && (dy == 0) && (fmt == D3DFMT_UNKNOWN))
	{
		gcLogger.Add("Using windowed mode\n");

		if (lpD3D->GetAdapterDisplayMode(ad, &dmc) != D3D_OK)
		{
			gcUserError.Set("Cannot get adapter display mode.");
			return FALSE;
		}

		ZeroMemory(&dp, sizeof(D3DPRESENT_PARAMETERS));
		dp.Windowed = TRUE;
		dp.SwapEffect = D3DSWAPEFFECT_FLIP;
		dp.BackBufferFormat = dmc.Format;
		dp.EnableAutoDepthStencil = TRUE;
		dp.AutoDepthStencilFormat = GetBestDepthBufferFormat(ad, dmc.Format, bStencil);
		dp.hDeviceWindow = hWnd;
		dp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		dp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		hResult = lpD3D->CreateDevice(nvAd, meDevType, hWnd, iBehaviourFlags, &dp, &lpD3DDevice);
		if (FAILED(hResult))
		{
			gcUserError.Set("Cannot create Direct3DDevice.");
			return FALSE;
		}
		PostInit(ad, dmc.Format, dp.AutoDepthStencilFormat);
		return TRUE;
	}

	gcLogger.Add("Using full screen mode\n");
	if (fmt == D3DFMT_UNKNOWN)
	{
		w = GetBestFrameBufferFormat(ad, D3DFMT_UNKNOWN);
	}
	else
	{
		w = fmt;
	}

	GetDisplayModes(ad, w);
	if (acMonitor[ad].iNumModes == 0)
	{
		gcUserError.Set("Cannot find frame buffer format.");
		return FALSE;
	}

	acMonitor[ad].iFrameBufferFormat = w;

	ZeroMemory(&dp, sizeof(dp));
	dp.BackBufferWidth = dx;
	dp.BackBufferHeight = dy;
	dp.BackBufferFormat = w;
	dp.BackBufferCount = 1;
	dp.MultiSampleType = D3DMULTISAMPLE_NONE;
	dp.SwapEffect = D3DSWAPEFFECT_FLIP;
	dp.hDeviceWindow = hWnd;
	dp.Windowed = FALSE;
	dp.EnableAutoDepthStencil = TRUE;
	dp.AutoDepthStencilFormat = GetBestDepthBufferFormat(ad, w, bStencil);;
	dp.Flags = 0;
	dp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	dp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (lpD3D->CreateDevice(nvAd, meDevType, hWnd, iBehaviourFlags, &dp, &lpD3DDevice )!=D3D_OK)
	{
		gcUserError.Set("Cannot create Direct3DDevice.");
		return FALSE;
	}

	PostInit(ad, dp.BackBufferFormat, dp.AutoDepthStencilFormat);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::PostInit(int ad, D3DFORMAT D3DFmt, D3DFORMAT D3DDepth)
{
	acMonitor[ad].iFrameBufferFormat = D3DFmt;
	acMonitor[ad].iDepthBufferFormat = D3DDepth;
	GetAdaptorCapabilities(ad);
	acMonitor[ad].bSoftwareMode = FALSE;
	acMonitor[ad].iTextureFormat = GetBestTextureBufferFormat(ad, 0, TRUE);
	iCurrMon = ad;
	FillStageStates(ad);
	FillSamplerStates(ad);
	FillRenderStates(ad);
	FillMatricies(ad);
	InitGeneral();
	GetBackBuffer();
	CreateTexture(0, 0, D3DFMT_UNKNOWN, TU_Null, FALSE, &acMonitor[ad].msNullTexture);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::ResetDevice(int iAD)
{
	D3DPRESENT_PARAMETERS	dp;
	HRESULT					hResult;
	int						i;

	if (lpD3DDevice != NULL)
	{
		for (i = 0; i < 100; i--)
		{
			hResult = lpD3DDevice->TestCooperativeLevel();
			ReturnOnFailure(hResult, "TestCooperativeLevel");
			if (hResult == S_OK)
			{
				gcLogger.Add("Cooperative Level OK\n");
				break;
			}
			if (hResult == D3DERR_DEVICENOTRESET)
			{
				ZeroMemory(&dp, sizeof(D3DPRESENT_PARAMETERS));
				dp.Windowed = TRUE;
				dp.SwapEffect = D3DSWAPEFFECT_COPY;
				dp.BackBufferFormat = acMonitor[iAD].iFrameBufferFormat;
				dp.EnableAutoDepthStencil = TRUE;
				dp.AutoDepthStencilFormat = acMonitor[iAD].iDepthBufferFormat;
				dp.hDeviceWindow = acMonitor[iAD].hWnd;

				if (mbLogCalls)
				{
					Log("Reset();\n");
				}

				hResult = lpD3DDevice->Reset(&dp);
				ReturnOnFailure(hResult, "Reset");
				gcLogger.Add("Reset Device\n");
				break;
			}
			Sleep(16);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetupCache(int ad)
{
	int		i;

	for (i = 0; i < MAX_WORLD_MATRICES; i++)	
	{  
		acMonitor[ad].mpsCurrentMatrix[i] = NULL;
	}

	for (i = 0; i < MAX_TEX; i++)	
	{  
		acMonitor[ad].mpsCurrentTexture[i] = NULL;
	}
	acMonitor[ad].mpsCurrentSurface = NULL;
	acMonitor[ad].mpsCurrentViewport = NULL;

	for (i = 0; i < MAX_TEX; i++)	
	{  
		acMonitor[ad].iSamplerStateID[i] = -1;	
	}

	for (i = 0; i < MAX_SAMPLE; i++)	
	{  acMonitor[ad].iStageStateID[i] = -1;	
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::GetBackBuffer(void)
{
	D3DSURFACE_DESC		sDesc;
	HRESULT				hResult;

	hResult = lpD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &acMonitor[iCurrMon].sBackBuffer.mpD3DSurface);
	ReturnOnFailure(hResult, "GetBackBuffer");
	hResult = acMonitor[iCurrMon].sBackBuffer.mpD3DSurface->GetDesc(&sDesc);
	ReturnOnFailure(hResult, "GetDesc");
	acMonitor[iCurrMon].sBackBuffer.mD3DFormat = sDesc.Format;
	if (sDesc.Usage & D3DUSAGE_DEPTHSTENCIL)
	{
		acMonitor[iCurrMon].sBackBuffer.meUsage = TU_RenderTargetStencil;
	}
	else
	{
		acMonitor[iCurrMon].sBackBuffer.meUsage = TU_RenderTarget;
	}
	acMonitor[iCurrMon].sBackBuffer.miWidth = sDesc.Width;
	acMonitor[iCurrMon].sBackBuffer.miHeight = sDesc.Height;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::InitGeneral(void)
{
	SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	SetRenderState(D3DRS_LIGHTING, FALSE);
	SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::Kill(void)
{
	int		i;
	char	s[512];
	HRESULT	hResult;

	if (mbLogCalls)
	{
		Log("Kill();\n");
	}

	if (lpD3DDevice != NULL)
	{
		for (i = 0; i < iNumMonitors; i++)
		{
			ReleaseTexture(&acMonitor[i].msNullTexture);
			acMonitor[i].sBackBuffer.mpD3DSurface->Release();
			sprintf(s, "Released display adapter [%i]\n", i);
			gcLogger.Add(s);
		}

		hResult = lpD3DDevice->Release();
		ReturnOnFailure(hResult, "Release");
		lpD3DDevice = NULL;
	}

	if (lpD3D != NULL)
	{
		lpD3D->Release();
		lpD3D = NULL;
		gcLogger.Add("Released direct3D\n");
	}

	cD3DVertTypes.Kill();
	mszLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::BeginScene(void)
{
	HRESULT	hResult;

	if (mbLogCalls)
	{
		Log("BeginScene();\n");
	}

	hResult = lpD3DDevice->BeginScene();
	ReturnOnFailure(hResult, "BeginScene");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::EndScene(void)
{
	HRESULT	hResult;

	if (mbLogCalls)
	{
		Log("EndScene();\n");
	}

	hResult = lpD3DDevice->EndScene();
	ReturnOnFailure(hResult, "EndScene");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::Present(void)
{
	Present(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::Present(HWND hWnd)
{
	HRESULT	hResult;

	if (mbLogCalls)
	{
		CChars sz;

		sz.Init("Present(");
		sz.Append("hWnd:");
		sz.AppendPointer(hWnd);

		sz.Append(");\n");

		Log(sz.Text());
		sz.Kill();
	}

	hResult = lpD3DDevice->Present(NULL, NULL, hWnd, NULL);
	if (hResult == D3DERR_DEVICELOST)
	{
		ResetDevice(0);
	}
	else 
	{
		ReturnOnFailure(hResult, "EndScene");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	HRESULT	hResult;
	if (acMonitor[iCurrMon].bSoftwareMode != bSoftware)
	{
		if (mbLogCalls)
		{
			CChars	sz;

			sz.Init("SetSoftwareVertexProcessing(");
			sz.Append("bSoftware:");
			sz.AppendBool(bSoftware);
			sz.Append(");\n");

			Log(sz.Text());
			sz.Kill();
		}

		hResult = lpD3DDevice->SetSoftwareVertexProcessing(bSoftware);
		ReturnOnFailure(hResult, "SetSoftwareVertexProcessing");
		acMonitor[iCurrMon].bSoftwareMode = bSoftware;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::ClearBuffer(int colour)
{
	HRESULT	hResult;

	hResult = lpD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, colour, 1.0f, 0);
	ReturnOnFailure(hResult, "Clear");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::ClearBuffer(int r, int g, int b)
{
	ClearBuffer(Set32BitColour(r, g, b, 255));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::FillStageStates(int iAd)
{
	memset(&acMonitor[iAd].aiStageStates, -1, sizeof(int) * MAX_TEX * MAX_STAGE_STATES);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::FillSamplerStates(int iAd)
{
	memset(&acMonitor[iAd].aiSampleStates, -1, sizeof(int) * MAX_SAMPLE * MAX_SAMPLE_STATES);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::FillMatricies(int iAd)
{
	int			i;
	SFloat4x4	c;

	Float4x4Identity(&c);
	for (i = 0; i < 256; i++)
	{
		SetWorldMatrix(i, &c);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::FillRenderStates(int iAd)
{
	mfFogStart = 0.1f;
	mfFogEnd = 1.0f;
	mfFogDensity = 1.0f;
	mfPointSize = 1.0f;
	mfPointSizeMin = 1.0f;
	mfPointScaleA = 1.0f;
	mfPointScaleB = 0.0f;
	mfPointScaleC = 0.0f;
	mfPointSizeMax = 64.0f;
	mfTweenFactor = 0.0f;
	mfMinTessalation = 1.0f;
	mfMaxTessalation = 4.0f;

	memset(&acMonitor[iAd].aiRenderStates, -1, sizeof(int) * MAX_RENDER_STATES);

	ForceRenderState(D3DRS_ZENABLE, TRUE);
	ForceRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	ForceRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	ForceRenderState(D3DRS_ZWRITEENABLE, TRUE);
	ForceRenderState(D3DRS_LASTPIXEL, FALSE);
	ForceRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
	ForceRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	ForceRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	ForceRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	ForceRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	ForceRenderState(D3DRS_ALPHAREF, 255);
	ForceRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	ForceRenderState(D3DRS_DITHERENABLE, FALSE);
	ForceRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	ForceRenderState(D3DRS_FOGENABLE, FALSE);
	ForceRenderState(D3DRS_SPECULARENABLE, FALSE);
	ForceRenderState(D3DRS_FOGCOLOR, 0);
	ForceRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP);
	ForceRenderState(D3DRS_FOGSTART, *((DWORD*) (&mfFogStart)));
	ForceRenderState(D3DRS_FOGEND, *((DWORD*) (&mfFogEnd)));
	ForceRenderState(D3DRS_FOGDENSITY, *((DWORD*) (&mfFogDensity)));
	ForceRenderState(D3DRS_RANGEFOGENABLE, FALSE);
	ForceRenderState(D3DRS_STENCILENABLE, FALSE);
	ForceRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	ForceRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	ForceRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
	ForceRenderState(D3DRS_STENCILFUNC, D3DCMP_GREATER);
	ForceRenderState(D3DRS_STENCILREF, 0);
	ForceRenderState(D3DRS_STENCILMASK, 0);
	ForceRenderState(D3DRS_STENCILWRITEMASK, 0);
	ForceRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
	ForceRenderState(D3DRS_WRAP0, 0);
	ForceRenderState(D3DRS_WRAP1, 0);
	ForceRenderState(D3DRS_WRAP2, 0);
	ForceRenderState(D3DRS_WRAP3, 0);
	ForceRenderState(D3DRS_WRAP4, 0);
	ForceRenderState(D3DRS_WRAP5, 0);
	ForceRenderState(D3DRS_WRAP6, 0);
	ForceRenderState(D3DRS_WRAP7, 0);
	ForceRenderState(D3DRS_CLIPPING, TRUE);
	ForceRenderState(D3DRS_LIGHTING, FALSE);
	ForceRenderState(D3DRS_AMBIENT, 0x00000000);
	ForceRenderState(D3DRS_FOGVERTEXMODE, FALSE);
	ForceRenderState(D3DRS_COLORVERTEX, TRUE);
	ForceRenderState(D3DRS_LOCALVIEWER, TRUE);
	ForceRenderState(D3DRS_NORMALIZENORMALS, FALSE);
	ForceRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	ForceRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
	ForceRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	ForceRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
	ForceRenderState(D3DRS_VERTEXBLEND, FALSE);
	ForceRenderState(D3DRS_CLIPPLANEENABLE, 0);
	ForceRenderState(D3DRS_POINTSIZE, *((DWORD*) (&mfPointSize)));
	ForceRenderState(D3DRS_POINTSIZE_MIN, *((DWORD*) (&mfPointSizeMin)));
	ForceRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
	ForceRenderState(D3DRS_POINTSCALEENABLE, FALSE);
	ForceRenderState(D3DRS_POINTSCALE_A, *((DWORD*) (&mfPointScaleA)));
	ForceRenderState(D3DRS_POINTSCALE_B, *((DWORD*) (&mfPointScaleB)));
	ForceRenderState(D3DRS_POINTSCALE_C, *((DWORD*) (&mfPointScaleC)));
	ForceRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
	ForceRenderState(D3DRS_MULTISAMPLEMASK, 0xffffffff);
	ForceRenderState(D3DRS_PATCHEDGESTYLE, D3DPATCHEDGE_DISCRETE);
	ForceRenderState(D3DRS_DEBUGMONITORTOKEN, D3DDMT_ENABLE);
	ForceRenderState(D3DRS_POINTSIZE_MAX, *((DWORD*) (&mfPointSizeMax)));
	ForceRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
	ForceRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_RED);
	ForceRenderState(D3DRS_TWEENFACTOR, *((DWORD*) (&mfTweenFactor)));
	ForceRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	ForceRenderState(D3DRS_POSITIONDEGREE, D3DDEGREE_CUBIC);
	ForceRenderState(D3DRS_NORMALDEGREE, D3DDEGREE_LINEAR);
	ForceRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	ForceRenderState(D3DRS_SLOPESCALEDEPTHBIAS, 0);  //This is the coplanar z-fighting thing
	ForceRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);
	ForceRenderState(D3DRS_MINTESSELLATIONLEVEL, *((DWORD*) (&mfMinTessalation)));
	ForceRenderState(D3DRS_MAXTESSELLATIONLEVEL, *((DWORD*) (&mfMaxTessalation)));
	ForceRenderState(D3DRS_ADAPTIVETESS_X, 0);
	ForceRenderState(D3DRS_ADAPTIVETESS_Y, 0);
	ForceRenderState(D3DRS_ADAPTIVETESS_Z, 0);
	ForceRenderState(D3DRS_ADAPTIVETESS_W, 0);
	ForceRenderState(D3DRS_ENABLEADAPTIVETESSELLATION, FALSE);
	ForceRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE);
	ForceRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);
	ForceRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
	ForceRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP);
	ForceRenderState(D3DRS_CCW_STENCILFUNC, D3DSTENCILOP_KEEP);
	ForceRenderState(D3DRS_COLORWRITEENABLE1, 0xf);
	ForceRenderState(D3DRS_COLORWRITEENABLE2, 0xf);
	ForceRenderState(D3DRS_COLORWRITEENABLE3, 0xf);
	ForceRenderState(D3DRS_BLENDFACTOR, 0xffffffff);
	ForceRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
	ForceRenderState(D3DRS_DEPTHBIAS, 0);  //This is the coplanar z-fighting thing
	ForceRenderState(D3DRS_WRAP8, 0);
	ForceRenderState(D3DRS_WRAP9, 0);
	ForceRenderState(D3DRS_WRAP10, 0);
	ForceRenderState(D3DRS_WRAP11, 0);
	ForceRenderState(D3DRS_WRAP12, 0);
	ForceRenderState(D3DRS_WRAP13, 0);
	ForceRenderState(D3DRS_WRAP14, 0);
	ForceRenderState(D3DRS_WRAP15, 0);
	ForceRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	ForceRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	ForceRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
	ForceRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetStageState(int iStage, int iState, int iValue)
{
	HRESULT	hResult;

	if (iValue != -1)
	{
		if (mbLogCalls)
		{
			LogSetStageState(iStage, iState, iValue);
		}

		hResult = lpD3DDevice->SetTextureStageState(iStage, (D3DTEXTURESTAGESTATETYPE)iState, iValue);
		ReturnOnFailure(hResult, "SetTextureStageState");
		acMonitor[iCurrMon].aiStageStates[iStage][iState] = iValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetSamplerState(int iSampler, int iState, int iValue)
{
	HRESULT	hResult;

	if (mbLogCalls)
	{
		LogSetSamplerState(iSampler, iState, iValue);
	}

	hResult = lpD3DDevice->SetSamplerState(iSampler, (D3DSAMPLERSTATETYPE)iState, iValue);
	ReturnOnFailure(hResult, "SetSamplerState");
	acMonitor[iCurrMon].aiSampleStates[iSampler][iState] = iValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetRenderState(D3DRENDERSTATETYPE iRenderState, int iValue)
{
	if (acMonitor[iCurrMon].aiRenderStates[iRenderState] != iValue)
	{
		ForceRenderState(iRenderState, iValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::ForceRenderState(D3DRENDERSTATETYPE iRenderState, int iValue)
{
	HRESULT		hResult;

	if (mbLogCalls)
	{
		LogSetRenderState(iRenderState, iValue);
	}

	hResult = lpD3DDevice->SetRenderState(iRenderState, iValue);
	ReturnOnFailure(hResult, "SetRenderState");
	acMonitor[iCurrMon].aiRenderStates[iRenderState] = iValue;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::GetRenderState(D3DRENDERSTATETYPE iRenderState)
{
	return acMonitor[iCurrMon].aiRenderStates[iRenderState];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::GetUsage(int iD3DVertexType)
{
	//Check if hardware or software vertex processing is needed.
	if ((acMonitor[iCurrMon].bMixedVertexProcessing) && (iD3DVertexType & D3DFVF_LASTBETA_UBYTE4))
	{
		return D3DUSAGE_SOFTWAREPROCESSING;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::CreateIndexBuffer(int iD3DVertexType, int iNumIndices, D3DFORMAT eD3DFormat, SIndexBuffer* psIndexBuffer)
{
	HRESULT		hResult;
	int			iUsage;

	//Check if hardware or software vertex processing is needed.
	iUsage = GetUsage(iD3DVertexType);

	//Check wether using 16 or 32 bit indices and set the size accordingly.
	if (eD3DFormat == D3DFMT_INDEX16)
	{
		psIndexBuffer->iIndexSize = sizeof(short int);
	}
	else if (eD3DFormat == D3DFMT_INDEX32)
	{
		psIndexBuffer->iIndexSize = sizeof(int);
	}
	else
	{
		psIndexBuffer->iIndexSize = 0;
	}

	hResult = lpD3DDevice->CreateIndexBuffer(iNumIndices * psIndexBuffer->iIndexSize, iUsage, eD3DFormat, D3DPOOL_DEFAULT, &psIndexBuffer->lpD3DIndexBuffer, NULL);
	ReturnOnFailure(hResult, "CreateIndexBuffer");	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetIndexBuffer(SIndexBuffer* psIndexBuffer)
{
	HRESULT		hResult;

	if ((psIndexBuffer != acMonitor[iCurrMon].mpsCurrentIndexBuffer) || mbAlwaysSetIndexBuffer)
	{
		if (mbLogCalls)
		{
			LogSetIndexBuffer(psIndexBuffer);
		}
		hResult = lpD3DDevice->SetIndices(psIndexBuffer->lpD3DIndexBuffer);
		ReturnOnFailure(hResult, "SetIndices");
		acMonitor[iCurrMon].mpsCurrentIndexBuffer = psIndexBuffer;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::ReleaseIndexBuffer(SIndexBuffer* psIndexBuffer)
{
	if (acMonitor[iCurrMon].mpsCurrentIndexBuffer == psIndexBuffer)
	{
		InvalidateIndexBuffer();
	}

	if (psIndexBuffer->lpD3DIndexBuffer)
	{
		psIndexBuffer->lpD3DIndexBuffer->Release();
		psIndexBuffer->lpD3DIndexBuffer = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::InvalidateIndexBuffer(void)
{
	acMonitor[iCurrMon].mpsCurrentIndexBuffer = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CD3D::LockIndexBuffer(SIndexBuffer* psIndexBuffer)
{
	void*		pData;
	HRESULT		hResult;

	hResult = psIndexBuffer->lpD3DIndexBuffer->Lock(0, 0, (void**)&pData, 0);
	ReturnNULLOnFailure(hResult, "LockIndexBuffer");
	SetFlag(&psIndexBuffer->iFlags, Flags_Locked, (hResult == S_OK));
	psIndexBuffer->pvLockedBuffer = pData;
	return pData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::UnlockIndexBuffer(SIndexBuffer* psIndexBuffer)
{
	HRESULT		hResult;

	hResult = psIndexBuffer->lpD3DIndexBuffer->Unlock();
	ReturnOnFailure(hResult, "UnlockIndexBuffer");
	SetFlag(&psIndexBuffer->iFlags, Flags_Locked, FALSE);
	psIndexBuffer->pvLockedBuffer = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CD3D::CreateVertexBuffer(int iD3DVertexType, int iNumVerts, BOOL bDynamic, SVertexBuffer* psVertexBuffer)
{
	HRESULT		hResult;
	int			iUsage;
	D3DPOOL		D3DPool;
	int			iVertexSize;

	if (iNumVerts == 0)
	{
		gcUserError.Set("Could not create vertex buffer with no vertices.");
		return FALSE;
	}

	//Check if hardware or software vertex processing is needed.
	iUsage = GetUsage(iD3DVertexType);

	iVertexSize = GetVertexFormatSize(iD3DVertexType);
	if (iVertexSize == 0)
	{
		gcUserError.Set("Could not create vertex buffer with a zero vertex size.");
		return FALSE;
	}

	psVertexBuffer->iVertexFormat = iD3DVertexType;
	psVertexBuffer->iVertexSize = iVertexSize;
	psVertexBuffer->iNumVerticies = iNumVerts;
	psVertexBuffer->iFlags = 0;
	psVertexBuffer->pvLockedBuffer = NULL;
	D3DPool = D3DPOOL_DEFAULT;
	if (bDynamic)
	{
		psVertexBuffer->iFlags |= Flags_Dynamic;
		iUsage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
		D3DPool = D3DPOOL_DEFAULT;
	}

	hResult = lpD3DDevice->CreateVertexBuffer(iNumVerts * iVertexSize, iUsage, iD3DVertexType, D3DPool, &psVertexBuffer->lpD3DVertexBuffer, NULL);
	if (FAILED(hResult))
	{
		gcUserError.Set("Could not CreateVertexBuffer");
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetVertexBuffer(SVertexBuffer* psVertexBuffer)
{
	HRESULT		hResult;

	//No implementation for streams other than zero.
	if ((psVertexBuffer != acMonitor[iCurrMon].mpsCurrentVertexBuffer) || mbAlwaysSetVertexBuffer)
	{
		if (mbLogCalls)
		{
			LogSetVertexBuffer(psVertexBuffer);
		}

		hResult = lpD3DDevice->SetFVF(psVertexBuffer->iVertexFormat);
		ReturnOnFailure(hResult, "SetFVF");

		if (psVertexBuffer->lpD3DVertexBuffer == NULL)
		{
			gcUserError.Set("Can't set a NULL vertex buffer");
			return;
		}

		hResult = lpD3DDevice->SetStreamSource(0, psVertexBuffer->lpD3DVertexBuffer, 0, psVertexBuffer->iVertexSize);
		ReturnOnFailure(hResult, "SetStreamSource");	

		acMonitor[iCurrMon].mpsCurrentVertexBuffer = psVertexBuffer;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::ReleaseVertexBuffer(SVertexBuffer* psVertexBuffer)
{
	if (acMonitor[iCurrMon].mpsCurrentVertexBuffer == psVertexBuffer)
	{
		InvalidateVertexBuffer();
	}

	if (psVertexBuffer->lpD3DVertexBuffer)
	{
		psVertexBuffer->lpD3DVertexBuffer->Release();
		psVertexBuffer->lpD3DVertexBuffer = NULL;
		psVertexBuffer->iNumVerticies = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::InvalidateVertexBuffer(void)
{
	acMonitor[iCurrMon].mpsCurrentVertexBuffer = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CD3D::LockVertexBuffer(SVertexBuffer* psVertexBuffer, BOOL bDiscard)
{
	HRESULT		hResult;
	int			D3DLock;

	D3DLock = 0;
	if (bDiscard)
	{
		if (psVertexBuffer->iFlags & Flags_Dynamic)
		{
			D3DLock = D3DLOCK_DISCARD;
			SetFlag(&psVertexBuffer->iFlags, Flags_Discard, TRUE);
		}
	}

	if (psVertexBuffer->lpD3DVertexBuffer == NULL)
	{
		gcUserError.Set("Can't lock a NULL vertex buffer");
		return NULL;
	}
	hResult = psVertexBuffer->lpD3DVertexBuffer->Lock(0, 0, &psVertexBuffer->pvLockedBuffer, D3DLock);
	ReturnNULLOnFailure(hResult, "LockVertexBuffer");
	SetFlag(&psVertexBuffer->iFlags, Flags_Locked, (hResult == S_OK));
	return psVertexBuffer->pvLockedBuffer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::UnlockVertexBuffer(SVertexBuffer* psVertexBuffer)
{
	HRESULT		hResult;

	hResult = psVertexBuffer->lpD3DVertexBuffer->Unlock();
	ReturnOnFailure(hResult, "UnlockVertexBuffer");
	SetFlag(&psVertexBuffer->iFlags, Flags_Locked | Flags_Discard, FALSE);
	psVertexBuffer->pvLockedBuffer = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::CreateTexture(int iWidth, int iHeight, D3DFORMAT D3DFormat, ED3DTextureUsage eUsage, BOOL bDynamic, STexture* psTexture)
{
	HRESULT			hResult;
	int				iUsage;

	psTexture->mD3DFormat = D3DFormat;
	psTexture->iFlags = 0;

	//Check the usage.
	iUsage = 0;
	if (bDynamic)
	{
		iUsage = D3DUSAGE_DYNAMIC;
		psTexture->iFlags |= Flags_Dynamic;
	}

	//Create the texture.
	if (eUsage != TU_Null)
	{
		if (eUsage == TU_AutoMipmap)
		{
			iUsage |= D3DUSAGE_AUTOGENMIPMAP;
			hResult = lpD3DDevice->CreateTexture(iWidth, iHeight, 0, iUsage, D3DFormat, D3DPOOL_DEFAULT, (IDirect3DTexture9**)&psTexture->mpD3DTexture, NULL);
			ReturnOnFailure(hResult, "CreateTexture");	
			hResult = psTexture->mpD3DTexture->SetAutoGenFilterType(D3DTEXF_LINEAR);
			ReturnOnFailure(hResult, "SetAutoGenFilterType");

			psTexture->mpD3DTexture->GenerateMipSubLevels();
		}
		else if (eUsage == TU_SystemMem)
		{
			hResult = lpD3DDevice->CreateTexture(iWidth, iHeight, 1, iUsage, D3DFormat, D3DPOOL_SYSTEMMEM, (IDirect3DTexture9**)&psTexture->mpD3DTexture, NULL);
			ReturnOnFailure(hResult, "CreateTexture");	
		}
		else
		{
			if (eUsage == TU_RenderTarget)
			{
				iUsage |= D3DUSAGE_RENDERTARGET;
			}
			if (eUsage == TU_DepthStencil)
			{
				iUsage |= D3DUSAGE_DEPTHSTENCIL;
			}
			if (eUsage == TU_RenderTargetStencil)
			{
				iUsage |= (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL);
			}

			hResult = lpD3DDevice->CreateTexture(iWidth, iHeight, 1, iUsage, D3DFormat, D3DPOOL_DEFAULT, (IDirect3DTexture9**)&psTexture->mpD3DTexture, NULL);
			ReturnOnFailure(hResult, "CreateTexture");	
		}
	}
	else
	{
		psTexture->mpD3DTexture = NULL;
	}
	psTexture->miHeight = iHeight;
	psTexture->miWidth = iWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::ReleaseTexture(STexture* psTexture)
{
	int		i;

	for (i = 0; i < MAX_TEX; i++)
	{
		if (acMonitor[iCurrMon].mpsCurrentTexture[i] == psTexture)
		{
			InvalidateTexture(i);
		}
	}

	if (psTexture->mpD3DTexture)
	{
		psTexture->mpD3DTexture->Release();
		psTexture->mpD3DTexture = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetTexture(int iStage, STexture* psTexture)
{
	HRESULT	hResult;

	if (psTexture)
	{
		if ((psTexture != acMonitor[iCurrMon].mpsCurrentTexture[iStage]) || mbAlwaysSetTexture)
		{
			if (mbLogCalls)
			{
				LogSetTexture(iStage, psTexture);
			}
			hResult = lpD3DDevice->SetTexture(iStage, psTexture->mpD3DTexture);
			ReturnOnFailure(hResult, "SetTexture");
			acMonitor[iCurrMon].mpsCurrentTexture[iStage] = psTexture;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::InvalidateTexture(int iStage)
{
	acMonitor[iCurrMon].mpsCurrentTexture[iStage] = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STexture* CD3D::GetNullTexture(void)
{
	return &acMonitor[iCurrMon].msNullTexture;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::CreateMaterial(SMaterial* psMaterial)
{
	psMaterial->Zero();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetMaterial(SMaterial* psMaterial)
{
	HRESULT	hResult;

	if ((acMonitor[iCurrMon].mpsCurrentMaterial != psMaterial) || mbAlwaysSetMaterial)
	{
		if (mbLogCalls)
		{
			LogSetMaterial(psMaterial);
		}

		hResult = lpD3DDevice->SetMaterial(&psMaterial->sD3DMaterial);
		ReturnOnFailure(hResult, "SetMaterial");	
		acMonitor[iCurrMon].mpsCurrentMaterial = psMaterial;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::ReleaseMaterial(SMaterial* psMaterial)
{
	if (acMonitor[iCurrMon].mpsCurrentMaterial == psMaterial)
	{
		InvalidateMaterial();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::InvalidateMaterial(void)
{
	acMonitor[iCurrMon].mpsCurrentMaterial = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetWorldMatrix(int iMatrixNum, SMatrix* psMatrix)
{
	SetWorldMatrix(iMatrixNum, &psMatrix->sD3DMatrix);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetWorldMatrix(int iMatrixNum, SFloat4x4* psMatrix)
{
	HRESULT	hResult;

	if (mbLogCalls)
	{
		LogSetWorldMatrix(iMatrixNum, psMatrix);
	}

	hResult = lpD3DDevice->SetTransform(D3DTS_WORLDMATRIX(iMatrixNum), (D3DXMATRIX*)psMatrix);
	ReturnOnFailure(hResult, "SetTransform");	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::CreateMatrix(SMatrix* psMatrix)
{
	psMatrix->sD3DMatrix.Identity();
	psMatrix->sD3DOldMatrix.Zero();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::InvalidateWorldMatrix(int iMatrixNum)
{
	acMonitor[iCurrMon].mpsCurrentMatrix[iMatrixNum] = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::CreateSurface(int iWidth, int iHeight, D3DFORMAT D3DFormat, ED3DTextureUsage eUsage, SSurface* psSurface)
{
	BOOL			bLockable;
	BOOL			bRenderTarget;
	HRESULT			hResult;

	bRenderTarget = FALSE;
	bLockable = FALSE;
	if ((eUsage == TU_RenderTarget) || (eUsage == TU_RenderTargetLockable))
		bRenderTarget = TRUE;
	if (eUsage == TU_RenderTargetLockable)
		bLockable = TRUE;

	if (bRenderTarget)
	{
		hResult = lpD3DDevice->CreateRenderTarget(iWidth, iHeight, D3DFormat, D3DMULTISAMPLE_NONE, 0, bLockable, &psSurface->mpD3DSurface, NULL);
		ReturnOnFailure(hResult, "CreateRenderTarget");	
	}
	else
	{
		if (eUsage == TU_SystemMem)
		{
			hResult = lpD3DDevice->CreateOffscreenPlainSurface(iWidth, iHeight, D3DFormat, D3DPOOL_SYSTEMMEM, &psSurface->mpD3DSurface, NULL);
		}
		else
		{
			hResult = lpD3DDevice->CreateOffscreenPlainSurface(iWidth, iHeight, D3DFormat, D3DPOOL_DEFAULT, &psSurface->mpD3DSurface, NULL);
		}
		ReturnOnFailure(hResult, "CreateOffscreenPlainSurface");	
	}

	psSurface->mD3DFormat = D3DFormat;
	psSurface->meUsage = eUsage;
	psSurface->miHeight = iHeight;
	psSurface->miWidth = iWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::ReleaseSurface(SSurface* psSurface)
{
	if (acMonitor[iCurrMon].mpsCurrentSurface == psSurface)
	{
		InvalidateSurface();
	}

	if (psSurface)
	{
		psSurface->mpD3DSurface->Release();
		psSurface->mpD3DSurface = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::InvalidateSurface(void)
{
	acMonitor[iCurrMon].mpsCurrentSurface = NULL;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CD3D::LockSurface(SSurface* psSurface)
{
	HRESULT			hResult;
	D3DLOCKED_RECT	lRect;

	hResult = psSurface->mpD3DSurface->LockRect(&lRect, NULL, 0);
	ReturnNULLOnFailure(hResult, "LockRect - LockSurface");
	SetFlag(&psSurface->iFlags, Flags_Locked, (hResult == S_OK));
	return (void*)lRect.pBits;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::UnlockSurface(SSurface* psSurface)
{
	HRESULT			hResult;

	hResult = psSurface->mpD3DSurface->UnlockRect();
	ReturnOnFailure(hResult, "UnlockRect");
	SetFlag(&psSurface->iFlags, Flags_Locked, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::UpdateSurface(LPDIRECT3DSURFACE9 psSource, LPDIRECT3DSURFACE9 psDest)
{
	HRESULT			hResult;

	hResult = lpD3DDevice->UpdateSurface(psSource, NULL, psDest, NULL);
	ReturnOnFailure(hResult, "UpdateSurface");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetRenderTarget(SSurface* psSurface)
{
	HRESULT			hResult;

	if (psSurface != acMonitor[iCurrMon].mpsCurrentSurface)
	{
		if ((psSurface->meUsage == TU_RenderTarget) || (psSurface->meUsage == TU_RenderTargetLockable) || (psSurface->meUsage == TU_RenderTargetStencil))
		{
			if (mbLogCalls)
			{
				LogSetRenderTarget(psSurface);
			}

			hResult = lpD3DDevice->SetRenderTarget(0, psSurface->mpD3DSurface);
			ReturnOnFailure(hResult, "SetRenderTarget");
			acMonitor[iCurrMon].mpsCurrentSurface = psSurface;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
HDC CD3D::CaptureSurfaceDC(SSurface* psSurface)
{
	HDC			hDC;
	HRESULT		hResult;

	hResult = psSurface->mpD3DSurface->GetDC(&hDC);
	ReturnNULLOnFailure(hResult, "GetDC");
	return hDC;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::FreeSurfaceDC(SSurface* psSurface, HDC hDC)
{
	psSurface->mpD3DSurface->ReleaseDC(hDC);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::CopySurfaceBits(SSurface* psSurfaceDest, RECT* psDestRect, SSurface* psSurfaceSource, RECT* psSourceRect, int iFilterType)
{
	HRESULT		hResult;

	hResult = lpD3DDevice->StretchRect(psSurfaceSource->mpD3DSurface, psSourceRect, psSurfaceDest->mpD3DSurface, psDestRect, (D3DTEXTUREFILTERTYPE)iFilterType);
	ReturnOnFailure(hResult, "StretchRect");	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::CopySurfaceBits(SSurface* psSurfaceDest, POINT* psDestPoint, SSurface* psSurfaceSource, RECT* psSourceRect)
{
	HRESULT		hResult;

	hResult = lpD3DDevice->UpdateSurface(psSurfaceSource->mpD3DSurface, psSourceRect, psSurfaceDest->mpD3DSurface, psDestPoint);
	ReturnOnFailure(hResult, "UpdateSurface");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::GetSurface(STexture* psTexture, int iLevel, SSurface* psSurface)
{
	HRESULT		hResult;

	hResult = ((IDirect3DTexture9*)psTexture->mpD3DTexture)->GetSurfaceLevel(iLevel, &psSurface->mpD3DSurface);
	ReturnOnFailure(hResult, "GetSurfaceLevel");

	psSurface->mD3DFormat = psTexture->mD3DFormat;
	psSurface->meUsage = psTexture->meUsage;
	psSurface->miHeight = psTexture->miHeight;
	psSurface->miWidth = psTexture->miWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::CreateSamplerState(SSamplerState* psSamplerState)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetSamplerState(int iSampler, SSamplerState* psSamplerState)
{
	gcD3D.SetSamplerState(iSampler, D3DSAMP_ADDRESSU, psSamplerState->iAddressU);
	gcD3D.SetSamplerState(iSampler, D3DSAMP_ADDRESSV, psSamplerState->iAddressV);
	gcD3D.SetSamplerState(iSampler, D3DSAMP_MAGFILTER, psSamplerState->iMagFilter);
	gcD3D.SetSamplerState(iSampler, D3DSAMP_MINFILTER, psSamplerState->iMinFilter);
	gcD3D.SetSamplerState(iSampler, D3DSAMP_MIPFILTER, psSamplerState->iMipFilter);
	gcD3D.SetSamplerState(iSampler, D3DSAMP_MIPMAPLODBIAS, *((DWORD*)&psSamplerState->fMipmapLodBias));
	acMonitor[iCurrMon].iSamplerStateID[iSampler] = psSamplerState->iUniqueID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::ReleaseSamplerState(SSamplerState* psSamplerState)
{
	psSamplerState->iUniqueID = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::InvalidateSamplerState(int iSampler)
{
	acMonitor[iCurrMon].iSamplerStateID[iSampler] = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::CreateStageState(SStageState* psStageState)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetStageState(int iStage, SStageState* psStageState)
{
	gcD3D.SetStageState(iStage, D3DTSS_COLORARG1, psStageState->iColourArg1);
	gcD3D.SetStageState(iStage, D3DTSS_COLORARG2, psStageState->iColourArg2);
	gcD3D.SetStageState(iStage, D3DTSS_COLOROP, psStageState->iColourOp);
	gcD3D.SetStageState(iStage, D3DTSS_ALPHAARG1, psStageState->iAlphaArg1);
	gcD3D.SetStageState(iStage, D3DTSS_ALPHAARG2, psStageState->iAlphaArg2);
	gcD3D.SetStageState(iStage, D3DTSS_ALPHAOP, psStageState->iAlphaOp);
	gcD3D.SetStageState(iStage, D3DTSS_RESULTARG, psStageState->iResultArg);
	gcD3D.SetStageState(iStage,  D3DTSS_TEXCOORDINDEX, psStageState->iTexCoord);
	acMonitor[iCurrMon].iStageStateID[iStage] = psStageState->iUniqueID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::ReleaseStageState(SStageState* psStageState)
{
	psStageState->iUniqueID = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::InvalidateStageState(int iStage)
{
	acMonitor[iCurrMon].iStageStateID[iStage] = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::CreateViewport(SViewport* psViewport)
{
	HRESULT	hResult;

	//This works om pure devices also.
	hResult = lpD3DDevice->GetViewport(&psViewport->sD3DViewport);
	ReturnOnFailure(hResult, "GetViewport");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::SetViewport(SViewport* psViewport)
{
	HRESULT	hResult;

	if ((acMonitor[iCurrMon].mpsCurrentViewport != psViewport) || mbAlwaysSetViewport)
	{
		if (mbLogCalls)
		{
			LogSetViewport(psViewport);
		}

		hResult = lpD3DDevice->SetViewport(&psViewport->sD3DViewport);
		ReturnOnFailure(hResult, "SetViewport");	
		acMonitor[iCurrMon].mpsCurrentViewport = psViewport;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::ReleaseViewport(SViewport* psViewport)
{
	if (acMonitor[iCurrMon].mpsCurrentViewport == psViewport)
	{
		InvalidateViewport();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::InvalidateViewport(void)
{
	acMonitor[iCurrMon].mpsCurrentViewport = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::AddVertexFormat(int iD3DVertexFormat)
{
	SD3DVertexType	sType;

	if (iD3DVertexFormat)
	{
		sType.iD3DVertexSize = CalculateVertexFormatSize(iD3DVertexFormat);
		sType.iWeightOffest = CalculateVertetFormatWeightOffset(iD3DVertexFormat);
		sType.iNormalOffest = CalculateVertetFormatNormalOffest(iD3DVertexFormat);
		sType.iColourOffest = CalculateVertetFormatColourOffest(iD3DVertexFormat);
		sType.iUVOffest = CalculateVertetFormatUVOffset(iD3DVertexFormat);
		sType.iD3DVertexFormat = iD3DVertexFormat;
		cD3DVertTypes.AddIfUniqueKey(&sType, 0, sizeof(int));
	}
	else
	{
		CChars::Dump("Can't add vertex type without vertex format");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::CalculateVertetFormatNormalOffest(int iD3DVertexFormat)
{
	int iSize;

	if (CalculateNormalSize(iD3DVertexFormat))
	{
		iSize = CalculatePositionSize(iD3DVertexFormat);
		iSize += CalculateWeightSize(iD3DVertexFormat);
		return iSize;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int	CD3D::CalculateVertetFormatColourOffest(int iD3DVertexFormat)
{
	int iSize;

	if (CalculateColourSize(iD3DVertexFormat))
	{
		iSize = CalculatePositionSize(iD3DVertexFormat);
		iSize += CalculateWeightSize(iD3DVertexFormat);
		iSize += CalculateNormalSize(iD3DVertexFormat);
		return iSize;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int	CD3D::CalculateVertetFormatWeightOffset(int iD3DVertexFormat)
{
	int iSize;

	if (CalculateWeightSize(iD3DVertexFormat))
	{
		iSize = CalculatePositionSize(iD3DVertexFormat);
		return iSize;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int	CD3D::CalculateVertetFormatUVOffset(int iD3DVertexFormat)
{
	int iSize;

	if (CalculateUVSize(iD3DVertexFormat))
	{
		iSize = CalculatePositionSize(iD3DVertexFormat);
		iSize += CalculateWeightSize(iD3DVertexFormat);
		iSize += CalculateNormalSize(iD3DVertexFormat);
		iSize += CalculateColourSize(iD3DVertexFormat);
		return iSize;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::GetVertexFormatSize(int iD3DVertexFormat)
{
	int					iPos;
	SD3DVertexType*		psType;

	iPos = cD3DVertTypes.FindWithIntKey(iD3DVertexFormat, 0);
	if (iPos == -1)
	{
		return 0;
	}

	psType = cD3DVertTypes.Get(iPos);
	return psType->iD3DVertexSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::RegisterAllFlexibleFormats(void)
{
	AddVertexFormat(D3DFVF_DEBUGVERTEX);
	AddVertexFormat(D3DFVF_CVERTEX);
	AddVertexFormat(D3DFVF_HCVERTEX);
	AddVertexFormat(D3DFVF_NVERTEX);
	AddVertexFormat(D3DFVF_CNVERTEX);
	AddVertexFormat(D3DFVF_HCTVERTEX);
	AddVertexFormat(D3DFVF_CTVERTEX);
	AddVertexFormat(D3DFVF_CNT1VERTEX);
	AddVertexFormat(D3DFVF_CNT2VERTEX);
	AddVertexFormat(D3DFVF_CNT3VERTEX);
	AddVertexFormat(D3DFVF_CNT4VERTEX);
	AddVertexFormat(D3DFVF_NT1VERTEX);
	AddVertexFormat(D3DFVF_NT2VERTEX);
	AddVertexFormat(D3DFVF_NT3VERTEX);
	AddVertexFormat(D3DFVF_NT4VERTEX);
	AddVertexFormat(D3DFVF_HVERTEX);
	AddVertexFormat(D3DFVF_HT1VERTEX);
	AddVertexFormat(D3DFVF_HT2VERTEX);
	AddVertexFormat(D3DFVF_HT3VERTEX);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::GetVertetFormatTextures(int iD3DVertexFormat)
{
	int iPosition;

	iPosition =  (iD3DVertexFormat & D3DFVF_TEXCOUNT_MASK);
	iPosition >>= D3DFVF_TEXCOUNT_SHIFT;

	//Return the number of textures in this format.
	return iPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::GetVertexFormatMatricies(int iD3DVertexFormat)
{
	int		iPosition;
	int		iNumber;

	iPosition = (iD3DVertexFormat & D3DFVF_POSITION_MASK);
	switch(iPosition) 
	{
	case D3DFVF_XYZ:
		iNumber = 1;
		break;
	case D3DFVF_XYZB1:
		iNumber = 1;
		break;
	case D3DFVF_XYZB2:
		iNumber = 2;
		break;
	case D3DFVF_XYZB3:
		iNumber = 3;
		break;
	case D3DFVF_XYZB4:
		iNumber = 4;
		break;
	case D3DFVF_XYZB5:
		iNumber = 5;
		break;
	case D3DFVF_XYZRHW:
		iNumber = 1;
		break;
	}
	return iNumber;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::GetVertexFormatUVOffset(int iD3DVertexFormat, int iTextureLayer)
{
	int					iPos;
	SD3DVertexType*		psVertexType;

	iPos = cD3DVertTypes.FindWithIntKey(iD3DVertexFormat, 0);
	if (iPos == -1)
	{
		return 0;
	}
	psVertexType = cD3DVertTypes.Get(iPos);
	return psVertexType->iUVOffest + iTextureLayer * (sizeof(float) * 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::GetVertexFormatNormalOffset(int iD3DVertexFormat)
{
	int					iPos;
	SD3DVertexType*		psVertexType;

	iPos = cD3DVertTypes.FindWithIntKey(iD3DVertexFormat, 0);
	if (iPos == -1)
	{
		return 0;
	}
	psVertexType = cD3DVertTypes.Get(iPos);
	return psVertexType->iNormalOffest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::GetVertetFormatWeightOffset(int iD3DVertexFormat, int iMatrixWeight)
{
	int					iPos;
	SD3DVertexType*		psVertexType;

	iPos = cD3DVertTypes.FindWithIntKey(iD3DVertexFormat, 0);
	if (iPos == -1)
	{
		return 0;
	}
	psVertexType = cD3DVertTypes.Get(iPos);
	return psVertexType->iWeightOffest + sizeof(float) * iMatrixWeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::GetVertetFormatColourOffset(int iD3DVertexFormat)
{
	int					iPos;
	SD3DVertexType*		psVertexType;

	iPos = cD3DVertTypes.FindWithIntKey(iD3DVertexFormat, 0);
	if (iPos == -1)
	{
		return 0;
	}
	psVertexType = cD3DVertTypes.Get(iPos);
	return psVertexType->iColourOffest;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::GetVertexFormatPositionOffset(int iD3DVertexFormat)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::GetVertexFormatFor(BOOL bNormal, BOOL bColour, int iNumberOfTextures, int iNumberOfMatricies, BOOL bPretransformed)
{
	int iD3DVertexFormat;

	iD3DVertexFormat = 0;
	if (bNormal)
	{
		iD3DVertexFormat |= D3DFVF_NORMAL;
	}

	if (bColour)
	{
		iD3DVertexFormat |= D3DFVF_DIFFUSE;
	}

	iD3DVertexFormat |= iNumberOfTextures << D3DFVF_TEXCOUNT_SHIFT;

	switch (iNumberOfMatricies) 
	{
	case 0:
		if (!bPretransformed)
		{
			//Zero matricies implies that there are no PER VERTEX matricies.  The entire stream is still transformed by the world matrix.
			iD3DVertexFormat |= D3DFVF_XYZ;
		}
		else
		{
			iD3DVertexFormat |= D3DFVF_XYZRHW;
		}
		break;
	case 1:
		//Note this verext format does not world on NVidia cards.  Pad it to two instead.
		iD3DVertexFormat |= (D3DFVF_XYZB1 | D3DFVF_LASTBETA_UBYTE4);
		break;
	case 2:
		iD3DVertexFormat |= (D3DFVF_XYZB2 | D3DFVF_LASTBETA_UBYTE4);
		break;
	case 3:
		iD3DVertexFormat |= (D3DFVF_XYZB3 | D3DFVF_LASTBETA_UBYTE4);
		break;
	case 4:
		iD3DVertexFormat |= (D3DFVF_XYZB4 | D3DFVF_LASTBETA_UBYTE4);
		break;
	case 5:
		iD3DVertexFormat |= (D3DFVF_XYZB5 | D3DFVF_LASTBETA_UBYTE4);
		break;
	default:
		gcLogger.Error("Too many matricies");
		return 0;
	}

	return iD3DVertexFormat;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SD3DVertexType* CD3D::GetVertexTypeFor(BOOL bNormal, BOOL bColour, int iNumberOfTextures, int iNumberOfMatricies, BOOL bPretransformed)
{
	int		iD3DVertexFormat;

	iD3DVertexFormat = GetVertexFormatFor(bNormal, bColour, iNumberOfTextures, iNumberOfMatricies, bPretransformed);
	return GetVertexType(iD3DVertexFormat);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SD3DVertexType* CD3D::GetVertexType(int iD3DVertexFormat)
{
	int		iPos;

	iPos = cD3DVertTypes.FindWithIntKey(iD3DVertexFormat, 0);
	if (iPos == -1)
	{
		return NULL;
	}
	return cD3DVertTypes.Get(iPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::CalculateNormalSize(int iD3DVertexFormat)
{
	if (iD3DVertexFormat & D3DFVF_NORMAL)
	{
		return sizeof(float) * 3;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::CalculateColourSize(int iD3DVertexFormat)
{
	int iSize = 0;

	if (iD3DVertexFormat & D3DFVF_DIFFUSE)
	{
		iSize += sizeof(DWORD);
	}
	if (iD3DVertexFormat & D3DFVF_SPECULAR)
	{
		iSize += sizeof(DWORD);
	} 

	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::CalculatePositionSize(int iD3DVertexFormat)
{
	int iPosition = D3DFVF_POSITION_MASK & iD3DVertexFormat;

	if ((iPosition == D3DFVF_XYZ) || 
		(iPosition == D3DFVF_XYZB1) || 
		(iPosition == D3DFVF_XYZB2) || 
		(iPosition == D3DFVF_XYZB3) || 
		(iPosition == D3DFVF_XYZB4) || 
		(iPosition == D3DFVF_XYZB5))
	{
		return sizeof(float) * 3;
	}
	else if (iPosition == D3DFVF_XYZRHW)
	{
		return sizeof(float) * 4;
	}
	else if (iPosition = D3DFVF_XYZW)
	{
		return sizeof(float) * 4;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::CalculateWeightSize(int iD3DVertexFormat)
{
	int iPosition;
	int iSize;

	iPosition = D3DFVF_POSITION_MASK & iD3DVertexFormat;
	if ((iPosition >= D3DFVF_XYZB1) && (iPosition <= D3DFVF_XYZB5))
	{
		iSize = iPosition / 2;
		iSize -= 2;
		return iSize * sizeof(float);
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::CalculateUVSize(int iD3DVertexFormat)
{
	int iTextures = GetVertetFormatTextures(iD3DVertexFormat);
	return iTextures * sizeof(float) * 2;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::CalculateVertexFormatSize(int iD3DVertexFormat)
{
	int iSize;

	iSize = CalculatePositionSize(iD3DVertexFormat);
	iSize += CalculateWeightSize(iD3DVertexFormat);
	iSize += CalculateNormalSize(iD3DVertexFormat);
	iSize += CalculateColourSize(iD3DVertexFormat);
	iSize += CalculateUVSize(iD3DVertexFormat);
	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::GetAdaptorCapabilities(int iAD)
{
	HRESULT		hResult;
	char		s[256];

	//First get the devices true abilities.
	hResult = lpD3D->GetDeviceCaps(iAD, meDevType, &acMonitor[iAD].sD3DCaps);
	ReturnOnFailure(hResult, "GetDeviceCaps");
	acMonitor[iAD].iNumHardwareBlendMatricies = acMonitor[iAD].sD3DCaps.MaxVertexBlendMatrixIndex;
	acMonitor[iAD].bMixedVertexProcessing = (acMonitor[iAD].sD3DCaps.MaxVertexBlendMatrixIndex < MIN_HARDWARE_BONES);

	//Then get the abilities exposed through software also.
	hResult = lpD3DDevice->GetDeviceCaps(&acMonitor[iAD].sD3DCaps);
	ReturnOnFailure(hResult, "GetDeviceCaps");

	acMonitor[iAD].bAutoGenerateMipmaps = FixBool(acMonitor[iAD].sD3DCaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP);
	acMonitor[iAD].bPureDevice = (FixBool(acMonitor[iAD].sD3DCaps.DevCaps & D3DDEVCAPS_PUREDEVICE)) && (!acMonitor[iAD].bMixedVertexProcessing);
	acMonitor[iAD].bIndexBuffers = FixBool(acMonitor[iAD].sD3DCaps.MaxVertexIndex);
	if (acMonitor[iAD].bAutoGenerateMipmaps)
	{
		gcLogger.Add("Can auto generate mipmaps\n");
	}
	else
	{
		gcLogger.Add("Cannot auto generate mipmaps\n");
	}

	if (acMonitor[iAD].bPureDevice)
	{
		gcLogger.Add("Using pure hardware device\n");
	}
	else
	{
		gcLogger.Add("Device is not a pure hardware device\n");
	}

	sprintf(s, "Hardware index blend matricies [%i]\n", acMonitor[iAD].iNumHardwareBlendMatricies);
	gcLogger.Add(s);

	if (acMonitor[iAD].bMixedVertexProcessing)
	{
		gcLogger.Add("Using mixed vertex processing\n");
	}
	else
	{
		gcLogger.Add("Using hardware vertex processing\n");
	}

	if (acMonitor[iAD].bIndexBuffers)
	{
		sprintf(s, "Using index buffers of length [%i]\n", acMonitor[iAD].sD3DCaps.MaxVertexIndex);
		gcLogger.Add(s);
	}
	else
	{
		gcLogger.Add("Device does not support index buffers\n");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
D3DFORMAT CD3D::GetBestDepthBufferFormat(int iAD, D3DFORMAT AdapterFormat, BOOL bNeedsStencil)
{
	HRESULT	hResult;

	//D3DUSAGE_SOFTWAREPROCESSING
	//No Stencil.
	if (!bNeedsStencil)
	{
		hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, AdapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32);
		if (SUCCEEDED(hResult))
		{
			gcLogger.Add("Best Depth/Stencil Format D3DFMT_D32\n");
			return D3DFMT_D32;
		}
		hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, AdapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8);
		if (SUCCEEDED(hResult))
		{
			gcLogger.Add("Best Depth/Stencil Format D3DFMT_D24X8\n");
			return D3DFMT_D24X8;
		}
		hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, AdapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16);
		if (SUCCEEDED(hResult))
		{
			gcLogger.Add("Best Depth/Stencil Format D3DFMT_D16\n");
			return D3DFMT_D16;
		}
	}
	//With Stencil.
	else
	{
		hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, AdapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8);
		if (SUCCEEDED(hResult))
		{
			gcLogger.Add("Best Depth/Stencil Format D3DFMT_D24S8\n");
			return D3DFMT_D24S8;
		}
		hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, AdapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X4S4);
		if (SUCCEEDED(hResult))
		{
			gcLogger.Add("Best Depth/Stencil Format D3DFMT_D24X4S4\n");
			return D3DFMT_D24X4S4;
		}
		hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, AdapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D15S1);
		if (SUCCEEDED(hResult))
		{
			gcLogger.Add("Best Depth/Stencil Format D3DFMT_D15S1\n");
			return D3DFMT_D15S1;
		}
	}

	gcLogger.Add("Best Depth/Stencil Format D3DFMT_UNKNOWN\n");
	return D3DFMT_UNKNOWN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
D3DFORMAT CD3D::GetBestFrameBufferFormat(int iAD, D3DFORMAT AdapterFormat)
{
	HRESULT	hResult;

	hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, D3DFMT_A8R8G8B8, D3DUSAGE_RENDERTARGET,  D3DRTYPE_SURFACE, D3DFMT_A8R8G8B8);
	if (SUCCEEDED(hResult))
	{
		gcLogger.Add("Best Frame Format D3DFMT_A8R8G8B8\n");
		return D3DFMT_A8R8G8B8;
	}
	hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, D3DFMT_X8R8G8B8, D3DUSAGE_RENDERTARGET,  D3DRTYPE_SURFACE, D3DFMT_X8R8G8B8);
	if (SUCCEEDED(hResult))
	{
		gcLogger.Add("Best Frame Format D3DFMT_X8R8G8B8\n");
		return D3DFMT_X8R8G8B8;
	}
	hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, D3DFMT_R8G8B8, D3DUSAGE_RENDERTARGET,  D3DRTYPE_SURFACE, D3DFMT_R8G8B8);
	if (SUCCEEDED(hResult))
	{
		gcLogger.Add("Best Frame Format D3DFMT_R8G8B8\n");
		return D3DFMT_R8G8B8;
	}
	hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, D3DFMT_A1R5G5B5, D3DUSAGE_RENDERTARGET,  D3DRTYPE_SURFACE, D3DFMT_A1R5G5B5);
	if (SUCCEEDED(hResult))
	{
		gcLogger.Add("Best Frame Format D3DFMT_A1R5G5B5\n");
		return D3DFMT_A1R5G5B5;
	}
	hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, D3DFMT_R5G6B5, D3DUSAGE_RENDERTARGET,  D3DRTYPE_SURFACE, D3DFMT_R5G6B5);
	if (SUCCEEDED(hResult))
	{
		gcLogger.Add("Best Frame Format D3DFMT_R5G6B5\n");
		return D3DFMT_R5G6B5;
	}
	hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, D3DFMT_X1R5G5B5, D3DUSAGE_RENDERTARGET,  D3DRTYPE_SURFACE, D3DFMT_X1R5G5B5);
	if (SUCCEEDED(hResult))
	{
		gcLogger.Add("Best Frame Format D3DFMT_X1R5G5B5\n");
		return D3DFMT_X1R5G5B5;
	}
	gcLogger.Add("Best Frame Format D3DFMT_UNKNOWN\n");
	return D3DFMT_UNKNOWN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
D3DFORMAT CD3D::GetBestTextureBufferFormat(int iAD, DWORD D3DUsage, BOOL bNeedsAlpha)
{
	HRESULT	hResult;

	hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, acMonitor[iAD].iFrameBufferFormat, D3DUsage,  D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8);
	if (SUCCEEDED(hResult))
	{
		gcLogger.Add("Best Texture Format D3DFMT_A8R8G8B8\n");
		return D3DFMT_A8R8G8B8;
	}
	hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, acMonitor[iAD].iFrameBufferFormat, D3DUsage,  D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8);
	if (SUCCEEDED(hResult))
	{
		if (!bNeedsAlpha)
		{
			gcLogger.Add("Best Texture Format D3DFMT_X8R8G8B8\n");
			return D3DFMT_X8R8G8B8;
		}
	}
	hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, acMonitor[iAD].iFrameBufferFormat, D3DUsage,  D3DRTYPE_TEXTURE, D3DFMT_R8G8B8);
	if (SUCCEEDED(hResult))
	{
		if (!bNeedsAlpha)
		{
			gcLogger.Add("Best Texture Format D3DFMT_R8G8B8\n");
			return D3DFMT_R8G8B8;
		}
	}
	hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, acMonitor[iAD].iFrameBufferFormat, D3DUsage,  D3DRTYPE_TEXTURE, D3DFMT_A1R5G5B5);
	if (SUCCEEDED(hResult))
	{
		gcLogger.Add("Best Texture Format D3DFMT_A1R5G5B5\n");
		return D3DFMT_A1R5G5B5;
	}
	hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, acMonitor[iAD].iFrameBufferFormat, D3DUsage,  D3DRTYPE_TEXTURE, D3DFMT_R5G6B5);
	if (SUCCEEDED(hResult))
	{
		if (!bNeedsAlpha)
		{
			gcLogger.Add("Best Texture Format D3DFMT_R5G6B5\n");
			return D3DFMT_R5G6B5;
		}
	}
	hResult = lpD3D->CheckDeviceFormat(iAD, meDevType, acMonitor[iAD].iFrameBufferFormat, D3DUsage,  D3DRTYPE_TEXTURE, D3DFMT_X1R5G5B5);
	if (SUCCEEDED(hResult))
	{
		if (!bNeedsAlpha)
		{
			gcLogger.Add("Best Texture Format D3DFMT_X1R5G5B5\n");
			return D3DFMT_X1R5G5B5;
		}
	}
	if (bNeedsAlpha)
	{
		//Couldn't get it with alpha, try without.
		return GetBestTextureBufferFormat(iAD, D3DUsage, FALSE);
	}
	gcLogger.Add("Best Texture Format D3DFMT_UNKNOWN\n");
	return D3DFMT_UNKNOWN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
D3DFORMAT CD3D::GetBestIndexBufferFormat(int iAD, int iNumIndices)
{
	if (acMonitor[iAD].sD3DCaps.MaxVertexIndex < (unsigned int)iNumIndices)
	{
		//gcLogger.Add("Best Index Buffer Format D3DFMT_UNKNOWN\n");
		return D3DFMT_UNKNOWN;
	}

	if (iNumIndices > 65535)
	{
		//gcLogger.Add("Best Index Buffer Format D3DFMT_INDEX32\n");
		return D3DFMT_INDEX32;
	}
	else
	{
		//gcLogger.Add("Best Index Buffer Format D3DFMT_INDEX16\n");
		return D3DFMT_INDEX16;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
D3DFORMAT CD3D::GetIndexBufferFormat(SIndexBuffer* psIndexBuffer)
{
	D3DINDEXBUFFER_DESC sDesc;

	psIndexBuffer->lpD3DIndexBuffer->GetDesc(&sDesc);
	return sDesc.Format;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
D3DFORMAT CD3D::GetVertexBufferFormat(SVertexBuffer* psVertexBuffer)
{
	D3DVERTEXBUFFER_DESC sDesc;

	psVertexBuffer->lpD3DVertexBuffer->GetDesc(&sDesc);
	return sDesc.Format;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CD3D::GetDeviceBehaviour(int iAD)
{
	HRESULT		hResult;
	int			iBehaviour;

	iBehaviour = 0;
	hResult = lpD3D->GetDeviceCaps(iAD, meDevType, &acMonitor[iAD].sD3DCaps);
	if (FAILED(hResult))
	{
		gcUserError.Set("Could not get Direct3DDevice capabilities.");
		return 0;
	}

	//Arbitrarilly decided that 32 is the min number of bones. :(
	if (acMonitor[iAD].sD3DCaps.MaxVertexBlendMatrixIndex >= MIN_HARDWARE_BONES)
	{
		if (acMonitor[iAD].sD3DCaps.DevCaps & D3DDEVCAPS_PUREDEVICE)
		{
			iBehaviour = D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING; 
		}
		else
		{
			iBehaviour = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		}
	}
	else
	{
		iBehaviour = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	return D3DCREATE_SOFTWARE_VERTEXPROCESSING;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::GetDisplayModes(int iAD, D3DFORMAT fmt)
{
	int		i, t;
	char	s[256];

	t = iAD;
	//Check we're not dealing with more monitors than we can handle.
	if (t >= MAX_MONITORS)
	{
		return;
	}

	lpD3D->GetAdapterIdentifier(t, 0, &acMonitor[t].cAdapterId);
	acMonitor[t].iNumModes = lpD3D->GetAdapterModeCount(t, fmt);

	sprintf(s, "Number of adapter display modes [%i]\nDisplay Modes:\n", acMonitor[t].iNumModes);
	gcLogger.Add(s);

	for (i = 0; i < acMonitor[t].iNumModes; i++)
	{
		//Check we're not dealing with more display modes than we can handle.
		if (i == MAX_DISPLAYMODES)
		{
			acMonitor[t].iNumModes = MAX_DISPLAYMODES;
			break;
		}
		lpD3D->EnumAdapterModes(t, fmt, i, &acMonitor[t].cDispayModes[i]);

		if (mbLogCalls)
		{
			sprintf(s, "EnumAdapterModes([%i, %i] @ %iHz)", acMonitor[t].cDispayModes[i].Width, acMonitor[t].cDispayModes[i].Height, acMonitor[t].cDispayModes[i].RefreshRate);
			Log(s);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CD3D::DrawIndexedPrimitive(D3DPRIMITIVETYPE eType, int iStartVertex, int iNumVerticies, int iStartIndex, int iNumPrimitives)
{
	HRESULT		hResult;

	if (mbLogCalls)
	{
		CChars sz;

		sz.Init("DrawIndexedPrimitive(");
		sz.Append("eType:");
		LogD3DPRIMITIVETYPE(&sz, eType);
		sz.Append(", iStartVertex:");
		sz.Append(iStartVertex);
		sz.Append(", iNumVerticies:");
		sz.Append(iNumVerticies);
		sz.Append(", iStartIndex:");
		sz.Append(iStartIndex);
		sz.Append(", iNumPrimitives:");
		sz.Append(iNumPrimitives);

		sz.Append(");\n");

		Log(sz.Text());
		sz.Kill();
	}

	hResult = gcD3D.lpD3DDevice->DrawIndexedPrimitive(eType, iStartVertex, 0, iNumVerticies, iStartIndex, iNumPrimitives);
	if (FAILED(hResult))
	{
		gcUserError.Set("Could not DrawIndexedPrimitive");
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CD3D::DrawPrimitive(D3DPRIMITIVETYPE eType, int iStartVertex, int iNumPrimitives)
{
	HRESULT		hResult;

	if (mbLogCalls)
	{
		CChars sz;

		sz.Init("DrawPrimitive(");
		sz.Append("eType:");
		LogD3DPRIMITIVETYPE(&sz, eType);
		sz.Append(", iStartVertex:");
		sz.Append(iStartVertex);
		sz.Append(", iNumPrimitives:");
		sz.Append(iNumPrimitives);

		sz.Append(");\n");

		Log(sz.Text());
		sz.Kill();
	}

	hResult = gcD3D.lpD3DDevice->DrawPrimitive(eType, iStartVertex, iNumPrimitives);
	if (FAILED(hResult))
	{
		gcUserError.Set("Could not DrawPrimitive");
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CD3D::SetCamera(SMatrix* psProjection, SMatrix* psView)
{
	HRESULT hResult; 

	if (mbLogCalls)
	{
		CChars sz;

		sz.Init("SetCamera(");
		sz.Append("psProjection:");
		LogD3DXMATRIX(&sz, &psProjection->sD3DMatrix);
		sz.Append(", psView:");
		LogD3DXMATRIX(&sz, &psView->sD3DMatrix);

		sz.Append(");\n");

		Log(sz.Text());
		sz.Kill();
	}

	hResult = gcD3D.lpD3DDevice->SetTransform(D3DTS_PROJECTION, (D3DXMATRIX*)&psProjection->sD3DMatrix);
	if (FAILED(hResult))
	{
		gcUserError.Set("Could not SetTransform");
		return FALSE;
	}

	hResult = gcD3D.lpD3DDevice->SetTransform(D3DTS_VIEW, (D3DXMATRIX*)&psView->sD3DMatrix);
	if (FAILED(hResult))
	{
		gcUserError.Set("Could not SetTransform");
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CD3D::LightDisable(int iLight)
{
	HRESULT hResult;

	hResult = lpD3DDevice->LightEnable(iLight, FALSE);
	if (FAILED(hResult))
	{
		gcUserError.Set("Could not LightEnable");
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CD3D::LightEnable(int iLight, D3DLIGHT9* psLight)
{
	HRESULT hResult;

	hResult = lpD3DDevice->LightEnable(iLight, TRUE);
	if (FAILED(hResult))
	{
		gcUserError.Set("Could not LightEnable");
		return FALSE;
	}

	hResult = gcD3D.lpD3DDevice->SetLight(iLight, psLight);
	if (FAILED(hResult))
	{
		gcUserError.Set("Could not SetLight");
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CD3D::CreateMesh(int iNumTriangles, int iNumIndices, int iMeshOptions, unsigned int iVertexFormat, ID3DXMesh** ppXMesh)
{
	HRESULT hResult;

	hResult = D3DXCreateMeshFVF(iNumTriangles, iNumIndices, iMeshOptions, iVertexFormat, gcD3D.lpD3DDevice, ppXMesh);
	if (FAILED(hResult))
	{
		gcUserError.Set("Could not D3DXCreateMeshFVF");
		return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::Log(char* sz)
{
	mszLog.Append(sz);
	if (mszLog.Length() >= miLogFlushSize)
	{
		gcLogger.Add(mszLog.Text());
		mszLog.Clear();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DCOLORVALUE(CChars* psz, D3DCOLORVALUE* psColour)
{
	psz->Append("[");
	psz->Append(psColour->r, 2);
	psz->Append(", ");
	psz->Append(psColour->g, 2);
	psz->Append(", ");
	psz->Append(psColour->b, 2);
	psz->Append(", ");
	psz->Append(psColour->a, 2);
	psz->Append("]");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DFILLMODE(CChars* psz, D3DFILLMODE eFillMode)
{
	switch (eFillMode)
	{
	D3D_CASE(D3DFILL_POINT, psz);
	D3D_CASE(D3DFILL_WIREFRAME, psz);
	D3D_CASE(D3DFILL_SOLID, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DSHADEMODE(CChars* psz, D3DSHADEMODE iValue)
{
	switch (iValue)
	{
	D3D_CASE(D3DFILL_POINT, psz);
	D3D_CASE(D3DFILL_WIREFRAME, psz);
	D3D_CASE(D3DFILL_SOLID, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DBLEND(CChars* psz, D3DBLEND iValue)
{
	switch (iValue)
	{
	D3D_CASE(D3DBLEND_ZERO, psz);
	D3D_CASE(D3DBLEND_ONE, psz);
	D3D_CASE(D3DBLEND_SRCCOLOR, psz);
	D3D_CASE(D3DBLEND_INVSRCCOLOR, psz);
	D3D_CASE(D3DBLEND_SRCALPHA, psz);
	D3D_CASE(D3DBLEND_INVSRCALPHA, psz);
	D3D_CASE(D3DBLEND_DESTALPHA, psz);
	D3D_CASE(D3DBLEND_INVDESTALPHA, psz);
	D3D_CASE(D3DBLEND_DESTCOLOR, psz);
	D3D_CASE(D3DBLEND_INVDESTCOLOR, psz);
	D3D_CASE(D3DBLEND_SRCALPHASAT, psz);
	D3D_CASE(D3DBLEND_BOTHSRCALPHA, psz);
	D3D_CASE(D3DBLEND_BOTHINVSRCALPHA, psz);
	D3D_CASE(D3DBLEND_BLENDFACTOR, psz);
	D3D_CASE(D3DBLEND_INVBLENDFACTOR, psz);

	#if !defined(D3D_DISABLE_9EX)
	D3D_CASE(D3DBLEND_SRCCOLOR2, psz);
	D3D_CASE(D3DBLEND_INVSRCCOLOR2, psz);
	#endif // !D3D_DISABLE_9EX

	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DWRAP(CChars* psz, unsigned int iValue)
{
	psz->Append("[");
	D3D_OR(D3DWRAP_U, iValue, psz);
	D3D_OR(D3DWRAP_V, iValue, psz);
	D3D_OR(D3DWRAP_W, iValue, psz);
	D3D_OR(D3DWRAPCOORD_3, iValue, psz);

	if (psz->EndsWith("|"))
	{
		psz->RemoveFromEnd(1);
	}
	psz->Append("]");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DMATERIALCOLORSOURCE(CChars* psz, D3DMATERIALCOLORSOURCE iValue)
{
	switch (iValue)
	{
	D3D_CASE(D3DMCS_MATERIAL, psz);
	D3D_CASE(D3DMCS_COLOR1, psz);
	D3D_CASE(D3DMCS_COLOR2, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DVERTEXBLENDFLAGS(CChars* psz, D3DVERTEXBLENDFLAGS iValue)
{
	switch (iValue)
	{
	D3D_CASE(D3DVBF_DISABLE, psz);
	D3D_CASE(D3DVBF_1WEIGHTS, psz);
	D3D_CASE(D3DVBF_2WEIGHTS, psz);
	D3D_CASE(D3DVBF_3WEIGHTS, psz);
	D3D_CASE(D3DVBF_TWEENING, psz);
	D3D_CASE(D3DVBF_0WEIGHTS, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DPATCHEDGESTYLE(CChars* psz, D3DPATCHEDGESTYLE iValue)
{
	switch (iValue)
	{
	D3D_CASE(D3DPATCHEDGE_DISCRETE, psz);
	D3D_CASE(D3DPATCHEDGE_CONTINUOUS, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DDEBUGMONITORTOKENS(CChars* psz, D3DDEBUGMONITORTOKENS iValue)
{
	switch (iValue)
	{
	D3D_CASE(D3DDMT_ENABLE, psz);
	D3D_CASE(D3DDMT_DISABLE, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DBLENDOP(CChars* psz, D3DBLENDOP iValue)
{
	switch (iValue)
	{
	D3D_CASE(D3DBLENDOP_ADD, psz);
	D3D_CASE(D3DBLENDOP_SUBTRACT, psz);
	D3D_CASE(D3DBLENDOP_REVSUBTRACT, psz);
	D3D_CASE(D3DBLENDOP_MIN, psz);
	D3D_CASE(D3DBLENDOP_MAX, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DDEGREE(CChars* psz, D3DDEGREETYPE iValue)
{
	switch (iValue)
	{
	D3D_CASE(D3DDEGREE_LINEAR, psz);
	D3D_CASE(D3DDEGREE_QUADRATIC, psz);
	D3D_CASE(D3DDEGREE_CUBIC, psz);
	D3D_CASE(D3DDEGREE_QUINTIC, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DUSAGE(CChars* psz, unsigned int uiUsage)
{
	psz->Append("[");
	D3D_OR(D3DUSAGE_AUTOGENMIPMAP, uiUsage, psz);
	D3D_OR(D3DUSAGE_DEPTHSTENCIL, uiUsage, psz);
	D3D_OR(D3DUSAGE_DMAP, uiUsage, psz);
	D3D_OR(D3DUSAGE_DONOTCLIP, uiUsage, psz);
	D3D_OR(D3DUSAGE_DYNAMIC, uiUsage, psz);
	D3D_OR(D3DUSAGE_NONSECURE, uiUsage, psz);
	D3D_OR(D3DUSAGE_NPATCHES, uiUsage, psz);
	D3D_OR(D3DUSAGE_POINTS, uiUsage, psz);
	D3D_OR(D3DUSAGE_RENDERTARGET, uiUsage, psz);
	D3D_OR(D3DUSAGE_RTPATCHES, uiUsage, psz);
	D3D_OR(D3DUSAGE_SOFTWAREPROCESSING, uiUsage, psz);
	D3D_OR(D3DUSAGE_TEXTAPI, uiUsage, psz);
	D3D_OR(D3DUSAGE_WRITEONLY, uiUsage, psz);

	if (psz->EndsWith("|"))
	{
		psz->RemoveFromEnd(1);
	}
	psz->Append("]");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DFORMAT(CChars* psz, D3DFORMAT eFormat)
{
	switch (eFormat)
	{
		D3D_CASE(D3DFMT_UNKNOWN, psz);
		D3D_CASE(D3DFMT_R8G8B8, psz);
		D3D_CASE(D3DFMT_A8R8G8B8, psz);
		D3D_CASE(D3DFMT_X8R8G8B8, psz);
		D3D_CASE(D3DFMT_R5G6B5, psz);
		D3D_CASE(D3DFMT_X1R5G5B5, psz);
		D3D_CASE(D3DFMT_A1R5G5B5, psz);
		D3D_CASE(D3DFMT_A4R4G4B4, psz);
		D3D_CASE(D3DFMT_R3G3B2, psz);
		D3D_CASE(D3DFMT_A8, psz);
		D3D_CASE(D3DFMT_A8R3G3B2, psz);
		D3D_CASE(D3DFMT_X4R4G4B4, psz);
		D3D_CASE(D3DFMT_A2B10G10R10, psz);
		D3D_CASE(D3DFMT_A8B8G8R8, psz);
		D3D_CASE(D3DFMT_X8B8G8R8, psz);
		D3D_CASE(D3DFMT_G16R16, psz);
		D3D_CASE(D3DFMT_A2R10G10B10, psz);
		D3D_CASE(D3DFMT_A16B16G16R16, psz);
		D3D_CASE(D3DFMT_A8P8, psz);
		D3D_CASE(D3DFMT_P8, psz);
		D3D_CASE(D3DFMT_L8, psz);
		D3D_CASE(D3DFMT_A8L8, psz);
		D3D_CASE(D3DFMT_A4L4, psz);
		D3D_CASE(D3DFMT_V8U8, psz);
		D3D_CASE(D3DFMT_L6V5U5, psz);
		D3D_CASE(D3DFMT_X8L8V8U8, psz);
		D3D_CASE(D3DFMT_Q8W8V8U8, psz);
		D3D_CASE(D3DFMT_V16U16, psz);
		D3D_CASE(D3DFMT_A2W10V10U10, psz);
		D3D_CASE(D3DFMT_UYVY, psz);
		D3D_CASE(D3DFMT_R8G8_B8G8, psz);
		D3D_CASE(D3DFMT_YUY2, psz);
		D3D_CASE(D3DFMT_G8R8_G8B8, psz);
		D3D_CASE(D3DFMT_DXT1, psz);
		D3D_CASE(D3DFMT_DXT2, psz);
		D3D_CASE(D3DFMT_DXT3, psz);
		D3D_CASE(D3DFMT_DXT4, psz);
		D3D_CASE(D3DFMT_DXT5, psz);
		D3D_CASE(D3DFMT_D16_LOCKABLE, psz);
		D3D_CASE(D3DFMT_D32, psz);
		D3D_CASE(D3DFMT_D15S1, psz);
		D3D_CASE(D3DFMT_D24S8, psz);
		D3D_CASE(D3DFMT_D24X8, psz);
		D3D_CASE(D3DFMT_D24X4S4, psz);
		D3D_CASE(D3DFMT_D16, psz);
		D3D_CASE(D3DFMT_D32F_LOCKABLE, psz);
		D3D_CASE(D3DFMT_D24FS8, psz);
		D3D_CASE(D3DFMT_L16, psz);
		D3D_CASE(D3DFMT_VERTEXDATA, psz);
		D3D_CASE(D3DFMT_INDEX16, psz);
		D3D_CASE(D3DFMT_INDEX32, psz);
		D3D_CASE(D3DFMT_Q16W16V16U16, psz);
		D3D_CASE(D3DFMT_MULTI2_ARGB8, psz);
		D3D_CASE(D3DFMT_R16F, psz);
		D3D_CASE(D3DFMT_G16R16F, psz);
		D3D_CASE(D3DFMT_A16B16G16R16F, psz);
		D3D_CASE(D3DFMT_R32F, psz);
		D3D_CASE(D3DFMT_G32R32F, psz);
		D3D_CASE(D3DFMT_A32B32G32R32F, psz);
		D3D_CASE(D3DFMT_CxV8U8, psz);

#if !defined(D3D_DISABLE_9EX)
		D3D_CASE(D3DFMT_D32_LOCKABLE, psz);
		D3D_CASE(D3DFMT_S8_LOCKABLE, psz);
		D3D_CASE(D3DFMT_A1, psz);
		D3D_CASE(D3DFMT_BINARYBUFFER, psz);
#endif
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DPOOL(CChars* psz, D3DPOOL ePool)
{
	switch (ePool)
	{
		D3D_CASE(D3DPOOL_DEFAULT, psz);
		D3D_CASE(D3DPOOL_MANAGED, psz);
		D3D_CASE(D3DPOOL_SYSTEMMEM, psz);
		D3D_CASE(D3DPOOL_SCRATCH, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogED3DFlags(CChars* psz, int eFlags)
{
	psz->Append("[");
	D3D_OR(Flags_Locked, eFlags, psz);
	D3D_OR(Flags_Discard, eFlags, psz);
	D3D_OR(Flags_Dynamic, eFlags, psz);

	if (psz->EndsWith("|"))
	{
		psz->RemoveFromEnd(1);
	}
	psz->Append("]");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogED3DTextureUsage(CChars* psz, ED3DTextureUsage eUsage)
{
	switch (eUsage)
	{
		D3D_CASE(TU_Plain, psz);
		D3D_CASE(TU_AutoMipmap, psz);
		D3D_CASE(TU_DepthStencil, psz);
		D3D_CASE(TU_RenderTarget, psz);
		D3D_CASE(TU_RenderTargetLockable, psz);
		D3D_CASE(TU_RenderTargetStencil, psz);
		D3D_CASE(TU_Null, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DTEXTURESTAGESTATETYPE(CChars* psz, D3DTEXTURESTAGESTATETYPE eStageStateType)
{
	switch (eStageStateType)
	{
		D3D_CASE(D3DTSS_COLOROP, psz);
		D3D_CASE(D3DTSS_COLORARG1, psz);
		D3D_CASE(D3DTSS_COLORARG2, psz);
		D3D_CASE(D3DTSS_ALPHAOP, psz);
		D3D_CASE(D3DTSS_ALPHAARG1, psz);
		D3D_CASE(D3DTSS_ALPHAARG2, psz);
		D3D_CASE(D3DTSS_BUMPENVMAT00, psz);
		D3D_CASE(D3DTSS_BUMPENVMAT01, psz);
		D3D_CASE(D3DTSS_BUMPENVMAT10, psz);
		D3D_CASE(D3DTSS_BUMPENVMAT11, psz);
		D3D_CASE(D3DTSS_TEXCOORDINDEX, psz);
		D3D_CASE(D3DTSS_BUMPENVLSCALE, psz);
		D3D_CASE(D3DTSS_BUMPENVLOFFSET, psz);
		D3D_CASE(D3DTSS_TEXTURETRANSFORMFLAGS, psz);
		D3D_CASE(D3DTSS_COLORARG0, psz);
		D3D_CASE(D3DTSS_ALPHAARG0, psz);
		D3D_CASE(D3DTSS_RESULTARG, psz);
		D3D_CASE(D3DTSS_CONSTANT, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DTEXTURESTAGESTATETYPEValue(CChars* psz, D3DTEXTURESTAGESTATETYPE eStageStateType, unsigned int uiValue)
{
	switch (eStageStateType)
	{
		case D3DTSS_COLOROP:
		case D3DTSS_ALPHAOP:
			LogD3DTEXTUREOP(psz, (D3DTEXTUREOP)uiValue);
			break;
		case D3DTSS_COLORARG1:
		case D3DTSS_COLORARG2:
		case D3DTSS_ALPHAARG1:
		case D3DTSS_ALPHAARG2:
		case D3DTSS_COLORARG0:
		case D3DTSS_ALPHAARG0:
		case D3DTSS_RESULTARG:
		case D3DTSS_CONSTANT:
			LogD3DTA(psz, uiValue);
			break;
		case D3DTSS_BUMPENVMAT00:
		case D3DTSS_BUMPENVMAT01:
		case D3DTSS_BUMPENVMAT10:
		case D3DTSS_BUMPENVMAT11:
		case D3DTSS_BUMPENVLSCALE:
		case D3DTSS_BUMPENVLOFFSET:
			psz->Append(*((float*)((void*)&uiValue)), 2);
			break;
		case D3DTSS_TEXCOORDINDEX:
			psz->Append(uiValue);
			break;
		case D3DTSS_TEXTURETRANSFORMFLAGS:
			LogD3DTEXTURETRANSFORMFLAGS(psz, (D3DTEXTURETRANSFORMFLAGS)uiValue);
			break;
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DTEXTURETRANSFORMFLAGS(CChars* psz, D3DTEXTURETRANSFORMFLAGS eFlags)
{
	int iCount;
	int bProjected;

	iCount = eFlags & ~D3DTTFF_PROJECTED;
	bProjected = eFlags & D3DTTFF_PROJECTED;

	switch (iCount)
	{
		D3D_CASE(D3DTTFF_DISABLE, psz);
		D3D_CASE(D3DTTFF_COUNT1, psz);
		D3D_CASE(D3DTTFF_COUNT2, psz);
		D3D_CASE(D3DTTFF_COUNT3, psz);
		D3D_CASE(D3DTTFF_COUNT4, psz);
	default:
		psz->Append("Error!");
	}

	if (bProjected)
	{
		psz->Append("|D3DTTFF_PROJECTED");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DTEXTUREOP(CChars* psz, D3DTEXTUREOP eTextureOp)
{
	switch (eTextureOp)
	{
		D3D_CASE(D3DTOP_DISABLE, psz);
		D3D_CASE(D3DTOP_SELECTARG1, psz);
		D3D_CASE(D3DTOP_SELECTARG2, psz);
		D3D_CASE(D3DTOP_MODULATE, psz);
		D3D_CASE(D3DTOP_MODULATE2X, psz);
		D3D_CASE(D3DTOP_MODULATE4X, psz);
		D3D_CASE(D3DTOP_ADD, psz);
		D3D_CASE(D3DTOP_ADDSIGNED, psz);
		D3D_CASE(D3DTOP_ADDSIGNED2X, psz);
		D3D_CASE(D3DTOP_SUBTRACT, psz);
		D3D_CASE(D3DTOP_ADDSMOOTH, psz);
		D3D_CASE(D3DTOP_BLENDDIFFUSEALPHA, psz);
		D3D_CASE(D3DTOP_BLENDTEXTUREALPHA, psz);
		D3D_CASE(D3DTOP_BLENDFACTORALPHA, psz);
		D3D_CASE(D3DTOP_BLENDTEXTUREALPHAPM, psz);
		D3D_CASE(D3DTOP_BLENDCURRENTALPHA, psz);
		D3D_CASE(D3DTOP_PREMODULATE, psz);
		D3D_CASE(D3DTOP_MODULATEALPHA_ADDCOLOR, psz);
		D3D_CASE(D3DTOP_MODULATECOLOR_ADDALPHA, psz);
		D3D_CASE(D3DTOP_MODULATEINVALPHA_ADDCOLOR, psz);
		D3D_CASE(D3DTOP_MODULATEINVCOLOR_ADDALPHA, psz);
		D3D_CASE(D3DTOP_BUMPENVMAP, psz);
		D3D_CASE(D3DTOP_BUMPENVMAPLUMINANCE, psz);
		D3D_CASE(D3DTOP_DOTPRODUCT3, psz);
		D3D_CASE(D3DTOP_MULTIPLYADD, psz);
		D3D_CASE(D3DTOP_LERP, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DTA(CChars* psz, unsigned int uiTa)
{
	switch (uiTa)
	{
		D3D_CASE(D3DTA_SELECTMASK, psz);
		D3D_CASE(D3DTA_DIFFUSE, psz);
		D3D_CASE(D3DTA_CURRENT, psz);
		D3D_CASE(D3DTA_TEXTURE, psz);
		D3D_CASE(D3DTA_TFACTOR, psz);
		D3D_CASE(D3DTA_SPECULAR, psz);
		D3D_CASE(D3DTA_TEMP, psz);
		D3D_CASE(D3DTA_CONSTANT, psz);
		D3D_CASE(D3DTA_COMPLEMENT, psz);
		D3D_CASE(D3DTA_ALPHAREPLICATE, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DSAMPLERSTATETYPE(CChars* psz, D3DSAMPLERSTATETYPE eSamplerState)
{
	switch (eSamplerState)
	{
		D3D_CASE(D3DSAMP_ADDRESSU, psz);
		D3D_CASE(D3DSAMP_ADDRESSV, psz);
		D3D_CASE(D3DSAMP_ADDRESSW, psz);
		D3D_CASE(D3DSAMP_BORDERCOLOR, psz);
		D3D_CASE(D3DSAMP_MAGFILTER, psz);
		D3D_CASE(D3DSAMP_MINFILTER, psz);
		D3D_CASE(D3DSAMP_MIPFILTER, psz);
		D3D_CASE(D3DSAMP_MIPMAPLODBIAS, psz);
		D3D_CASE(D3DSAMP_MAXMIPLEVEL, psz);
		D3D_CASE(D3DSAMP_MAXANISOTROPY, psz);
		D3D_CASE(D3DSAMP_SRGBTEXTURE, psz);
		D3D_CASE(D3DSAMP_ELEMENTINDEX, psz);
		D3D_CASE(D3DSAMP_DMAPOFFSET, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DSAMPLERSTATETYPEValue(CChars* psz, D3DSAMPLERSTATETYPE eSamplerState, unsigned int uiValue)
{
	switch (eSamplerState)
	{
		case D3DSAMP_ADDRESSU:  //D3DTEXTUREADDRESS
		case D3DSAMP_ADDRESSV:
		case D3DSAMP_ADDRESSW:
			LogD3DTEXTUREADDRESS(psz, (D3DTEXTUREADDRESS)uiValue);
			break;
		case D3DSAMP_BORDERCOLOR:  //D3DCOLOR
			LogD3DCOLOR(psz, (D3DCOLOR)uiValue);
			break;
		case D3DSAMP_MAGFILTER:
		case D3DSAMP_MINFILTER:
		case D3DSAMP_MIPFILTER:
			LogD3DTEXTUREFILTERTYPE(psz, (D3DTEXTUREFILTERTYPE)uiValue);
			break;
		case D3DSAMP_MIPMAPLODBIAS:
			psz->Append(*((float*)((void*)&uiValue)),2);
			break;
		case D3DSAMP_MAXMIPLEVEL:
		case D3DSAMP_MAXANISOTROPY:
		case D3DSAMP_SRGBTEXTURE:
		case D3DSAMP_ELEMENTINDEX:
		case D3DSAMP_DMAPOFFSET:
			psz->Append(uiValue);
			break;
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DTEXTUREADDRESS(CChars* psz, D3DTEXTUREADDRESS eTextureAddress)
{
	switch (eTextureAddress)
	{
	D3D_CASE(D3DTADDRESS_WRAP, psz);
	D3D_CASE(D3DTADDRESS_MIRROR, psz);
	D3D_CASE(D3DTADDRESS_CLAMP, psz);
	D3D_CASE(D3DTADDRESS_BORDER, psz);
	D3D_CASE(D3DTADDRESS_MIRRORONCE, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogFVF(CChars* psz, unsigned int fvf)
{
	int		iValue;

	psz->Append("[");
	iValue = fvf & D3DFVF_POSITION_MASK;
	switch (iValue)
	{
	D3D_CASE(D3DFVF_XYZ, psz);
	D3D_CASE(D3DFVF_XYZW, psz);
	D3D_CASE(D3DFVF_XYZRHW, psz);
	D3D_CASE(D3DFVF_XYZB1, psz);
	D3D_CASE(D3DFVF_XYZB2, psz);
	D3D_CASE(D3DFVF_XYZB3, psz);
	D3D_CASE(D3DFVF_XYZB4, psz);
	D3D_CASE(D3DFVF_XYZB5, psz);
	default:
		psz->Append("Error!");
	}

	psz->Append("|");
	D3D_OR(D3DFVF_NORMAL, fvf, psz);
	D3D_OR(D3DFVF_PSIZE, fvf, psz);
	D3D_OR(D3DFVF_DIFFUSE, fvf, psz);
	D3D_OR(D3DFVF_SPECULAR, fvf, psz);

	iValue = fvf & D3DFVF_TEXCOUNT_MASK;
	switch (iValue)
	{
	D3D_CASE(D3DFVF_TEX0, psz);
	D3D_CASE(D3DFVF_TEX1, psz);
	D3D_CASE(D3DFVF_TEX2, psz);
	D3D_CASE(D3DFVF_TEX3, psz);
	D3D_CASE(D3DFVF_TEX4, psz);
	D3D_CASE(D3DFVF_TEX5, psz);
	D3D_CASE(D3DFVF_TEX6, psz);
	D3D_CASE(D3DFVF_TEX7, psz);
	D3D_CASE(D3DFVF_TEX8, psz);
	default:
		psz->Append("Error!");
	}

	psz->Append("|");
	D3D_OR(D3DFVF_LASTBETA_UBYTE4, fvf, psz);
	D3D_OR(D3DFVF_LASTBETA_D3DCOLOR, fvf, psz);

	if (psz->EndsWith("|"))
	{
		psz->RemoveFromEnd(1);
	}
	psz->Append("]");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DCOLOR(CChars* psz, D3DCOLOR uiColour)
{
	int	iValue;

	//D3DFMT_A8R8G8B8
	psz->Append("[");

	iValue = GetM8BitRedColour(uiColour);
	psz->AppendHexHiLo(&iValue, 1);
	psz->Append(", ");
	iValue = GetM8BitGreenColour(uiColour);
	psz->AppendHexHiLo(&iValue, 1);
	psz->Append(", ");
	iValue = GetM8BitBlueColour(uiColour);
	psz->AppendHexHiLo(&iValue, 1);
	psz->Append(", ");
	iValue = GetM8BitAlphaColour(uiColour);
	psz->AppendHexHiLo(&iValue, 1);
	psz->Append("]");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DTEXTUREFILTERTYPE(CChars* psz, D3DTEXTUREFILTERTYPE eTextureFilter)
{
	switch (eTextureFilter)
	{
	D3D_CASE(D3DTEXF_NONE, psz);
	D3D_CASE(D3DTEXF_POINT, psz);
	D3D_CASE(D3DTEXF_LINEAR, psz);
	D3D_CASE(D3DTEXF_ANISOTROPIC, psz);
	D3D_CASE(D3DTEXF_PYRAMIDALQUAD, psz);
	D3D_CASE(D3DTEXF_GAUSSIANQUAD, psz);
	#if !defined(D3D_DISABLE_9EX)
	D3D_CASE(D3DTEXF_CONVOLUTIONMONO, psz);
	#endif // !D3D_DISABLE_9EX
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DCULL(CChars* psz, D3DCULL eCull)
{
	switch (eCull)
	{
	D3D_CASE(D3DCULL_NONE, psz);
	D3D_CASE(D3DCULL_CW, psz);
	D3D_CASE(D3DCULL_CCW, psz);
	default:
		psz->Append("Error!");
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DCMPFUNC(CChars* psz, D3DCMPFUNC eCmpFunc)
{
	switch (eCmpFunc)
	{
	D3D_CASE(D3DCMP_NEVER, psz);
	D3D_CASE(D3DCMP_LESS, psz);
	D3D_CASE(D3DCMP_EQUAL, psz);
	D3D_CASE(D3DCMP_LESSEQUAL, psz);
	D3D_CASE(D3DCMP_GREATER, psz);
	D3D_CASE(D3DCMP_NOTEQUAL, psz);
	D3D_CASE(D3DCMP_GREATEREQUAL, psz);
	D3D_CASE(D3DCMP_ALWAYS, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DSTENCILOP(CChars* psz, D3DSTENCILOP eTextureAddress)
{
	switch (eTextureAddress)
	{
		D3D_CASE(D3DSTENCILOP_KEEP, psz);
		D3D_CASE(D3DSTENCILOP_ZERO, psz);
		D3D_CASE(D3DSTENCILOP_REPLACE, psz);
		D3D_CASE(D3DSTENCILOP_INCRSAT, psz);
		D3D_CASE(D3DSTENCILOP_DECRSAT, psz);
		D3D_CASE(D3DSTENCILOP_INVERT, psz);
		D3D_CASE(D3DSTENCILOP_INCR, psz);
		D3D_CASE(D3DSTENCILOP_DECR, psz);
	default:
		psz->Append("Error!");
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DFOGMODE(CChars* psz, D3DFOGMODE eTextureAddress)
{
	switch (eTextureAddress)
	{
		D3D_CASE(D3DFOG_NONE, psz);
		D3D_CASE(D3DFOG_EXP, psz);
		D3D_CASE(D3DFOG_EXP2, psz);
		D3D_CASE(D3DFOG_LINEAR, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DXMATRIX(CChars* psz, SFloat4x4* psMatrix)
{
	psMatrix->ToString(psz, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogSetRenderState(D3DRENDERSTATETYPE iRenderState, int iValue)
{
	CChars	sz;

	sz.Init("SetRenderState(");
	sz.Append("iRenderState:");
	LogD3DRENDERSTATETYPE(&sz, iRenderState);
	sz.Append(", iValue:");
	LogD3DRENDERSTATETYPEValue(&sz, iRenderState, iValue);
	sz.Append(");\n");
	Log(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DRENDERSTATETYPE(CChars* psz, D3DRENDERSTATETYPE eRenderState)
{
	switch (eRenderState)
	{
		D3D_CASE(D3DRS_ZENABLE, psz);
		D3D_CASE(D3DRS_FILLMODE, psz);
		D3D_CASE(D3DRS_SHADEMODE, psz);
		D3D_CASE(D3DRS_ZWRITEENABLE, psz);
		D3D_CASE(D3DRS_ALPHATESTENABLE, psz);
		D3D_CASE(D3DRS_LASTPIXEL, psz);
		D3D_CASE(D3DRS_SRCBLEND, psz);
		D3D_CASE(D3DRS_DESTBLEND, psz);
		D3D_CASE(D3DRS_CULLMODE, psz);
		D3D_CASE(D3DRS_ZFUNC, psz);
		D3D_CASE(D3DRS_ALPHAREF, psz);
		D3D_CASE(D3DRS_ALPHAFUNC, psz);
		D3D_CASE(D3DRS_DITHERENABLE, psz);
		D3D_CASE(D3DRS_ALPHABLENDENABLE, psz);
		D3D_CASE(D3DRS_FOGENABLE, psz);
		D3D_CASE(D3DRS_SPECULARENABLE, psz);
		D3D_CASE(D3DRS_FOGCOLOR, psz);
		D3D_CASE(D3DRS_FOGTABLEMODE, psz);
		D3D_CASE(D3DRS_FOGSTART, psz);
		D3D_CASE(D3DRS_FOGEND, psz);
		D3D_CASE(D3DRS_FOGDENSITY, psz);
		D3D_CASE(D3DRS_RANGEFOGENABLE, psz);
		D3D_CASE(D3DRS_STENCILENABLE, psz);
		D3D_CASE(D3DRS_STENCILFAIL, psz);
		D3D_CASE(D3DRS_STENCILZFAIL, psz);
		D3D_CASE(D3DRS_STENCILPASS, psz);
		D3D_CASE(D3DRS_STENCILFUNC, psz);
		D3D_CASE(D3DRS_STENCILREF, psz);
		D3D_CASE(D3DRS_STENCILMASK, psz);
		D3D_CASE(D3DRS_STENCILWRITEMASK, psz);
		D3D_CASE(D3DRS_TEXTUREFACTOR, psz);
		D3D_CASE(D3DRS_WRAP0, psz);
		D3D_CASE(D3DRS_WRAP1, psz);
		D3D_CASE(D3DRS_WRAP2, psz);
		D3D_CASE(D3DRS_WRAP3, psz);
		D3D_CASE(D3DRS_WRAP4, psz);
		D3D_CASE(D3DRS_WRAP5, psz);
		D3D_CASE(D3DRS_WRAP6, psz);
		D3D_CASE(D3DRS_WRAP7, psz);
		D3D_CASE(D3DRS_CLIPPING, psz);
		D3D_CASE(D3DRS_LIGHTING, psz);
		D3D_CASE(D3DRS_AMBIENT, psz);
		D3D_CASE(D3DRS_FOGVERTEXMODE, psz);
		D3D_CASE(D3DRS_COLORVERTEX, psz);
		D3D_CASE(D3DRS_LOCALVIEWER, psz);
		D3D_CASE(D3DRS_NORMALIZENORMALS, psz);
		D3D_CASE(D3DRS_DIFFUSEMATERIALSOURCE, psz);
		D3D_CASE(D3DRS_SPECULARMATERIALSOURCE, psz);
		D3D_CASE(D3DRS_AMBIENTMATERIALSOURCE, psz);
		D3D_CASE(D3DRS_EMISSIVEMATERIALSOURCE, psz);
		D3D_CASE(D3DRS_VERTEXBLEND, psz);
		D3D_CASE(D3DRS_CLIPPLANEENABLE, psz);
		D3D_CASE(D3DRS_POINTSIZE, psz);
		D3D_CASE(D3DRS_POINTSIZE_MIN, psz);
		D3D_CASE(D3DRS_POINTSPRITEENABLE, psz);
		D3D_CASE(D3DRS_POINTSCALEENABLE, psz);
		D3D_CASE(D3DRS_POINTSCALE_A, psz);
		D3D_CASE(D3DRS_POINTSCALE_B, psz);
		D3D_CASE(D3DRS_POINTSCALE_C, psz);
		D3D_CASE(D3DRS_MULTISAMPLEANTIALIAS, psz);
		D3D_CASE(D3DRS_MULTISAMPLEMASK, psz);
		D3D_CASE(D3DRS_PATCHEDGESTYLE, psz);
		D3D_CASE(D3DRS_DEBUGMONITORTOKEN, psz);
		D3D_CASE(D3DRS_POINTSIZE_MAX, psz);
		D3D_CASE(D3DRS_INDEXEDVERTEXBLENDENABLE, psz);
		D3D_CASE(D3DRS_COLORWRITEENABLE, psz);
		D3D_CASE(D3DRS_TWEENFACTOR, psz);
		D3D_CASE(D3DRS_BLENDOP, psz);
		D3D_CASE(D3DRS_POSITIONDEGREE, psz);
		D3D_CASE(D3DRS_NORMALDEGREE, psz);
		D3D_CASE(D3DRS_SCISSORTESTENABLE, psz);
		D3D_CASE(D3DRS_SLOPESCALEDEPTHBIAS, psz);
		D3D_CASE(D3DRS_ANTIALIASEDLINEENABLE, psz);
		D3D_CASE(D3DRS_MINTESSELLATIONLEVEL, psz);
		D3D_CASE(D3DRS_MAXTESSELLATIONLEVEL, psz);
		D3D_CASE(D3DRS_ADAPTIVETESS_X, psz);
		D3D_CASE(D3DRS_ADAPTIVETESS_Y, psz);
		D3D_CASE(D3DRS_ADAPTIVETESS_Z, psz);
		D3D_CASE(D3DRS_ADAPTIVETESS_W, psz);
		D3D_CASE(D3DRS_ENABLEADAPTIVETESSELLATION, psz);
		D3D_CASE(D3DRS_TWOSIDEDSTENCILMODE, psz);
		D3D_CASE(D3DRS_CCW_STENCILFAIL, psz);
		D3D_CASE(D3DRS_CCW_STENCILZFAIL, psz);
		D3D_CASE(D3DRS_CCW_STENCILPASS, psz);
		D3D_CASE(D3DRS_CCW_STENCILFUNC, psz);
		D3D_CASE(D3DRS_COLORWRITEENABLE1, psz);
		D3D_CASE(D3DRS_COLORWRITEENABLE2, psz);
		D3D_CASE(D3DRS_COLORWRITEENABLE3, psz);
		D3D_CASE(D3DRS_BLENDFACTOR, psz);
		D3D_CASE(D3DRS_SRGBWRITEENABLE, psz);
		D3D_CASE(D3DRS_DEPTHBIAS, psz);
		D3D_CASE(D3DRS_WRAP8, psz);
		D3D_CASE(D3DRS_WRAP9, psz);
		D3D_CASE(D3DRS_WRAP10, psz);
		D3D_CASE(D3DRS_WRAP11, psz);
		D3D_CASE(D3DRS_WRAP12, psz);
		D3D_CASE(D3DRS_WRAP13, psz);
		D3D_CASE(D3DRS_WRAP14, psz);
		D3D_CASE(D3DRS_WRAP15, psz);
		D3D_CASE(D3DRS_SEPARATEALPHABLENDENABLE, psz);
		D3D_CASE(D3DRS_SRCBLENDALPHA, psz);
		D3D_CASE(D3DRS_DESTBLENDALPHA, psz);
		D3D_CASE(D3DRS_BLENDOPALPHA, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DRENDERSTATETYPEValue(CChars* psz, D3DRENDERSTATETYPE eRenderState, int iValue)
{
	switch (eRenderState)
	{
	case D3DRS_FILLMODE:
		LogD3DFILLMODE(psz, (D3DFILLMODE)iValue);
		break;

	case D3DRS_SHADEMODE:
		LogD3DSHADEMODE(psz, (D3DSHADEMODE)iValue);
	break;

	case D3DRS_ZENABLE:
	case D3DRS_ZWRITEENABLE:
	case D3DRS_ALPHATESTENABLE:
	case D3DRS_LASTPIXEL:
	case D3DRS_DITHERENABLE:
	case D3DRS_ALPHABLENDENABLE:
	case D3DRS_FOGENABLE:
	case D3DRS_SPECULARENABLE:
	case D3DRS_RANGEFOGENABLE:
	case D3DRS_STENCILENABLE:
	case D3DRS_CLIPPING:
	case D3DRS_LIGHTING:
	case D3DRS_COLORVERTEX:
	case D3DRS_LOCALVIEWER:
	case D3DRS_NORMALIZENORMALS:
	case D3DRS_CLIPPLANEENABLE:
	case D3DRS_POINTSPRITEENABLE:
	case D3DRS_POINTSCALEENABLE:
	case D3DRS_MULTISAMPLEANTIALIAS:
	case D3DRS_INDEXEDVERTEXBLENDENABLE:
	case D3DRS_COLORWRITEENABLE:
	case D3DRS_SCISSORTESTENABLE:
	case D3DRS_ANTIALIASEDLINEENABLE:
	case D3DRS_ENABLEADAPTIVETESSELLATION:
	case D3DRS_TWOSIDEDSTENCILMODE:
	case D3DRS_SRGBWRITEENABLE:
	case D3DRS_SEPARATEALPHABLENDENABLE:
		psz->AppendBool(iValue);
		break;

	case D3DRS_SRCBLEND:
	case D3DRS_DESTBLEND:
	case D3DRS_SRCBLENDALPHA:
	case D3DRS_DESTBLENDALPHA:
		LogD3DBLEND(psz, (D3DBLEND)iValue);
		break;

	case D3DRS_CULLMODE:
		LogD3DCULL(psz, (D3DCULL)iValue);
		break;

	case D3DRS_ZFUNC:
	case D3DRS_ALPHAFUNC:
	case D3DRS_STENCILFUNC:
	case D3DRS_CCW_STENCILFUNC:
		LogD3DCMPFUNC(psz, (D3DCMPFUNC)iValue);
		break;

	case D3DRS_FOGCOLOR:
	case D3DRS_TEXTUREFACTOR:
	case D3DRS_AMBIENT:
	case D3DRS_BLENDFACTOR:
		LogD3DCOLOR(psz, (D3DCOLOR)iValue);
		break;

	case D3DRS_FOGTABLEMODE:
		LogD3DFOGMODE(psz, (D3DFOGMODE)iValue);
		break;

	case D3DRS_FOGSTART:
	case D3DRS_FOGEND:
	case D3DRS_FOGDENSITY:
	case D3DRS_POINTSIZE:
	case D3DRS_POINTSIZE_MIN:
	case D3DRS_POINTSCALE_A:
	case D3DRS_POINTSCALE_B:
	case D3DRS_POINTSCALE_C:
	case D3DRS_POINTSIZE_MAX:
	case D3DRS_TWEENFACTOR:
	case D3DRS_SLOPESCALEDEPTHBIAS:
	case D3DRS_MINTESSELLATIONLEVEL:
	case D3DRS_MAXTESSELLATIONLEVEL:
	case D3DRS_ADAPTIVETESS_X:
	case D3DRS_ADAPTIVETESS_Y:
	case D3DRS_ADAPTIVETESS_Z:
	case D3DRS_ADAPTIVETESS_W:
	case D3DRS_DEPTHBIAS:
		psz->Append(*((float*)((void*)&iValue)),2);
		break;

	case D3DRS_STENCILFAIL:
	case D3DRS_STENCILZFAIL:
	case D3DRS_STENCILPASS:
		LogD3DSTENCILOP(psz, (D3DSTENCILOP)iValue);
		break;

	case D3DRS_STENCILREF:
	case D3DRS_STENCILMASK:
	case D3DRS_STENCILWRITEMASK:
	case D3DRS_MULTISAMPLEMASK:
	case D3DRS_COLORWRITEENABLE1:
	case D3DRS_COLORWRITEENABLE2:
	case D3DRS_COLORWRITEENABLE3:
	case D3DRS_ALPHAREF:
		psz->Append(iValue);
		break;

	case D3DRS_WRAP0:
	case D3DRS_WRAP1:
	case D3DRS_WRAP2:
	case D3DRS_WRAP3:
	case D3DRS_WRAP4:
	case D3DRS_WRAP5:
	case D3DRS_WRAP6:
	case D3DRS_WRAP7:
	case D3DRS_WRAP8:
	case D3DRS_WRAP9:
	case D3DRS_WRAP10:
	case D3DRS_WRAP11:
	case D3DRS_WRAP12:
	case D3DRS_WRAP13:
	case D3DRS_WRAP14:
	case D3DRS_WRAP15:
		LogD3DWRAP(psz, iValue); //D3DWRAP_U
		break;

	case D3DRS_FOGVERTEXMODE:
		LogD3DFOGMODE(psz, (D3DFOGMODE)iValue);
		break;

	case D3DRS_DIFFUSEMATERIALSOURCE:
	case D3DRS_SPECULARMATERIALSOURCE:
	case D3DRS_AMBIENTMATERIALSOURCE:
	case D3DRS_EMISSIVEMATERIALSOURCE:
		LogD3DMATERIALCOLORSOURCE(psz, (D3DMATERIALCOLORSOURCE)iValue);
		break;

	case D3DRS_VERTEXBLEND:
		LogD3DVERTEXBLENDFLAGS(psz, (D3DVERTEXBLENDFLAGS)iValue);
		break;

	case D3DRS_PATCHEDGESTYLE:
		LogD3DPATCHEDGESTYLE(psz, (D3DPATCHEDGESTYLE)iValue);
		break;

	case D3DRS_DEBUGMONITORTOKEN:
		LogD3DDEBUGMONITORTOKENS(psz, (D3DDEBUGMONITORTOKENS)iValue);
		break;

	case D3DRS_BLENDOP:
	case D3DRS_BLENDOPALPHA:
		LogD3DBLENDOP(psz, (D3DBLENDOP)iValue);
		break;

	case D3DRS_POSITIONDEGREE:
	case D3DRS_NORMALDEGREE:
		LogD3DDEGREE(psz, (D3DDEGREETYPE)iValue);
		break;

	case D3DRS_CCW_STENCILFAIL:
	case D3DRS_CCW_STENCILZFAIL:
	case D3DRS_CCW_STENCILPASS:
		LogD3DSTENCILOP(psz, (D3DSTENCILOP)iValue);
		break;

	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogD3DPRIMITIVETYPE(CChars* psz, D3DPRIMITIVETYPE eType)
{
	switch(eType)
	{
	D3D_CASE(D3DPT_POINTLIST, psz);
	D3D_CASE(D3DPT_LINELIST, psz);
	D3D_CASE(D3DPT_LINESTRIP, psz);
	D3D_CASE(D3DPT_TRIANGLELIST, psz);
	D3D_CASE(D3DPT_TRIANGLESTRIP, psz);
	D3D_CASE(D3DPT_TRIANGLEFAN, psz);
	default:
		psz->Append("Error!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogSetStageState(int iStage, int iState, int iValue)
{
	CChars sz;

	sz.Init("SetStageState(");
	sz.Append("iStage:");
	sz.Append(iStage);
	sz.Append(", iState:");
	LogD3DTEXTURESTAGESTATETYPE(&sz, (D3DTEXTURESTAGESTATETYPE)iState);
	sz.Append(", iValue:");
	LogD3DTEXTURESTAGESTATETYPEValue(&sz, (D3DTEXTURESTAGESTATETYPE)iState, iValue);
	sz.Append(");\n");

	Log(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogSetSamplerState(int iSampler, int iState, int iValue)
{
	CChars sz;

	sz.Init("SetSamplerState(");
	sz.Append("iSampler:");
	sz.Append(iSampler);
	sz.Append(", iState:");
	LogD3DSAMPLERSTATETYPE(&sz, (D3DSAMPLERSTATETYPE)iState);
	sz.Append(", iValue:");
	LogD3DSAMPLERSTATETYPEValue(&sz, (D3DSAMPLERSTATETYPE)iState, iValue);
	sz.Append(");\n");
	Log(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogSetIndexBuffer(SIndexBuffer* psIndexBuffer)
{
	CChars	sz;

	sz.Init("SetIndexBuffer(:");
	sz.Append("iFlags:");
	LogED3DFlags(&sz, psIndexBuffer->iFlags);
	sz.Append(", iIndexSize:");
	sz.Append(psIndexBuffer->iIndexSize);
	sz.Append(", pvLockedBuffer:");
	sz.AppendPointer(psIndexBuffer->pvLockedBuffer);
	sz.Append(", lpD3DIndexBuffer:");
	sz.AppendPointer(psIndexBuffer->lpD3DIndexBuffer);
	if (psIndexBuffer->lpD3DIndexBuffer != NULL)
	{
	#ifdef D3D_DEBUG_INFO
		//sz.Append(", Length:");
		//sz.Append((unsigned int)psIndexBuffer->lpD3DIndexBuffer->Length);
		//sz.Append(", Usage:");
		//LogD3DUSAGE(&sz, (unsigned int)psIndexBuffer->lpD3DIndexBuffer->Usage);
		//sz.Append(", Format:");
		//LogD3DFORMAT(&sz, psIndexBuffer->lpD3DIndexBuffer->Format);
		//sz.Append(", Pool:");
		//LogD3DPOOL(&sz, psIndexBuffer->lpD3DIndexBuffer->Pool);
		//sz.Append(", LockCount:");
		//sz.Append((unsigned int)psIndexBuffer->lpD3DIndexBuffer->LockCount);
	#endif // D3D_DEBUG_INFO
	}
	sz.Append(");\n");

	Log(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogSetVertexBuffer(SVertexBuffer* psVertexBuffer)
{
	CChars	sz;

	sz.Init("SetVertexBuffer(");
	sz.Append("iFlags:");
	LogED3DFlags(&sz, psVertexBuffer->iFlags);
	sz.Append(", iVertexSize:");
	sz.Append(psVertexBuffer->iVertexSize);
	sz.Append(", iVertexFormat:");
	LogFVF(&sz, psVertexBuffer->iVertexFormat);
	sz.Append(", iNumVerticies:");
	sz.Append(psVertexBuffer->iNumVerticies);
	sz.Append(", pvLockedBuffer:");
	sz.AppendPointer(psVertexBuffer->pvLockedBuffer);
	sz.Append(", lpD3DVertexBuffer:");
	sz.AppendPointer(psVertexBuffer->lpD3DVertexBuffer);
	if (psVertexBuffer->lpD3DVertexBuffer != NULL)
	{
	#ifdef D3D_DEBUG_INFO
		//sz.Append(", Length:");
		//sz.Append((unsigned int)psVertexBuffer->lpD3DVertexBuffer->Length);
		//sz.Append(", Usage:");
		//LogD3DUSAGE(&sz, (unsigned int)psVertexBuffer->lpD3DVertexBuffer->Usage);
		//sz.Append(", FVF:");
		//sz.Append((unsigned int)psVertexBuffer->lpD3DVertexBuffer->FVF);
		//sz.Append(", Pool:");
		//LogD3DPOOL(&sz, psVertexBuffer->lpD3DVertexBuffer->Pool);
		//sz.Append(", LockCount:");
		//sz.Append((unsigned int)psVertexBuffer->lpD3DVertexBuffer->LockCount);
	#endif // D3D_DEBUG_INFO
	}

	sz.Append(");\n");

	Log(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogSetTexture(int iStage, STexture* psTexture)
{
	CChars	sz;

	sz.Init("SetTexture(");
	sz.Append("iFlags:");
	LogED3DFlags(&sz, psTexture->iFlags);
	sz.Append(", mD3DFormat:");
	LogD3DFORMAT(&sz, psTexture->mD3DFormat);
	sz.Append(", meUsage:");
	LogED3DTextureUsage(&sz, psTexture->meUsage);
	sz.Append(", miWidth:");
	sz.Append(psTexture->miWidth);
	sz.Append(", miHeight:");
	sz.Append(psTexture->miHeight);
	sz.Append(", mpD3DTexture:");
	sz.AppendPointer(psTexture->mpD3DTexture);

	if (psTexture->mpD3DTexture != NULL)
	{
	#ifdef D3D_DEBUG_INFO
		sz.Append(", Width:");
		sz.Append((unsigned int)(((IDirect3DTexture9*)(psTexture->mpD3DTexture))->Width));
		sz.Append(", Height:");
		sz.Append((unsigned int)(((IDirect3DTexture9*)(psTexture->mpD3DTexture))->Height));
		sz.Append(", Levels:");
		sz.Append((unsigned int)(((IDirect3DTexture9*)(psTexture->mpD3DTexture))->Levels));
		sz.Append(", Usage:");
		LogD3DUSAGE(&sz, (unsigned int)(((IDirect3DTexture9*)(psTexture->mpD3DTexture))->Usage));
		sz.Append(", Pool:");
		LogD3DPOOL(&sz, ((IDirect3DTexture9*)(psTexture->mpD3DTexture))->Pool);
		sz.Append(", LOD:");
		sz.Append((unsigned int)(((IDirect3DTexture9*)(psTexture->mpD3DTexture))->LOD));
		sz.Append(", FilterType:");
		LogD3DTEXTUREFILTERTYPE(&sz, ((IDirect3DTexture9*)(psTexture->mpD3DTexture))->FilterType);
		sz.Append(", LockCount:");
		sz.Append((unsigned int)(((IDirect3DTexture9*)(psTexture->mpD3DTexture))->LockCount));
	#endif // D3D_DEBUG_INFO
	}

	sz.Append(");\n");
	Log(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogSetMaterial(SMaterial* psMaterial)
{
	CChars sz;

	sz.Init("SetMaterial(");
	sz.Append("Diffuse:");
	LogD3DCOLORVALUE(&sz, &psMaterial->sD3DMaterial.Diffuse);
	sz.Append(", Ambient:");
	LogD3DCOLORVALUE(&sz, &psMaterial->sD3DMaterial.Ambient);
	sz.Append(", Specular:");
	LogD3DCOLORVALUE(&sz, &psMaterial->sD3DMaterial.Specular);
	sz.Append(", Emissive:");
	LogD3DCOLORVALUE(&sz, &psMaterial->sD3DMaterial.Emissive);
	sz.Append(", Power:");
	sz.Append(psMaterial->sD3DMaterial.Power, 2);
	sz.Append(");\n");

	Log(sz.Text());
	sz.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogSetWorldMatrix(int iMatrixNum, SFloat4x4* psMatrix)
{
	CChars	sz;

	sz.Init("SetWorldMatrix(");
	sz.Append("iMatrixNum:");
	sz.Append(iMatrixNum);
	sz.Append(", psMatrix:");
	LogD3DXMATRIX(&sz, psMatrix);

	sz.Append(");\n");
	Log(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogSetRenderTarget(SSurface* psSurface)
{
	CChars	sz;

	sz.Init("SetRenderTarget(");
	sz.Append("iFlags:");
	LogED3DFlags(&sz, psSurface->iFlags);
	sz.Append(", mD3DFormat:");
	LogD3DFORMAT(&sz, psSurface->mD3DFormat);
	sz.Append(", meUsage:");
	LogED3DTextureUsage(&sz, psSurface->meUsage);
	sz.Append(", miWidth:");
	sz.Append(psSurface->miWidth);
	sz.Append(", miHeight:");
	sz.Append(psSurface->miHeight);
	//sz.Append(", miStride:");
	//sz.Append(psSurface->miStride);

	#ifdef D3D_DEBUG_INFO
	sz.Append(", Width:");
	sz.Append(psSurface->mpD3DSurface->Width);
	sz.Append(", Height:");
	sz.Append(psSurface->mpD3DSurface->Height);
	sz.Append(", Usage:");
	LogD3DUSAGE(&sz, psSurface->mpD3DSurface->Usage);
	sz.Append(", Format:");
	LogD3DFORMAT(&sz, psSurface->mpD3DSurface->Format);
	sz.Append(", Pool:");
	LogD3DPOOL(&sz, psSurface->mpD3DSurface->Pool);
	sz.Append(", MultiSampleType:");
	sz.Append((int)psSurface->mpD3DSurface->MultiSampleType);
	sz.Append(", MultiSampleQuality:");
	sz.Append((unsigned int)psSurface->mpD3DSurface->MultiSampleQuality);
	sz.Append(", LockCount:");
	sz.Append((unsigned int)psSurface->mpD3DSurface->LockCount);
	sz.Append(", DCCount:");
	sz.Append((unsigned int)psSurface->mpD3DSurface->DCCount);
	#endif // D3D_DEBUG_INFO
	sz.Append(");\n");

	Log(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CD3D::LogSetViewport(SViewport* psViewport)
{
	CChars	sz;

	sz.Init("SetViewport(");
	sz.Append("X:");
	sz.Append((unsigned int)psViewport->sD3DViewport.X);
	sz.Append(", Y:");
	sz.Append((unsigned int)psViewport->sD3DViewport.Y);
	sz.Append(", Width:");
	sz.Append((unsigned int)psViewport->sD3DViewport.Width);
	sz.Append(", Height:");
	sz.Append((unsigned int)psViewport->sD3DViewport.Height);
	sz.Append(", MinZ:");
	sz.Append(psViewport->sD3DViewport.MinZ, 2);
	sz.Append(", MaxZ:");
	sz.Append(psViewport->sD3DViewport.MaxZ, 2);

	sz.Append(");\n");
	Log(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SMatrix::Set(SFloat4x4* psMatrix)
{
	sD3DMatrix = *psMatrix;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SMatrix::SetPos(SFloat3* psVector)
{
	sD3DMatrix.pos.x = psVector->x;
	sD3DMatrix.pos.y = psVector->y;
	sD3DMatrix.pos.z = psVector->z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SMatrix::Fix(void)
{
	sD3DMatrix.x.x = RoundFloat(sD3DMatrix.x.x, BINARY_PRECISION);
	sD3DMatrix.x.y = RoundFloat(sD3DMatrix.x.y, BINARY_PRECISION);
	sD3DMatrix.x.z = RoundFloat(sD3DMatrix.x.z, BINARY_PRECISION);
	sD3DMatrix.x.w = RoundFloat(sD3DMatrix.x.w, BINARY_PRECISION);
	sD3DMatrix.y.x = RoundFloat(sD3DMatrix.y.x, BINARY_PRECISION);
	sD3DMatrix.y.y = RoundFloat(sD3DMatrix.y.y, BINARY_PRECISION);
	sD3DMatrix.y.z = RoundFloat(sD3DMatrix.y.z, BINARY_PRECISION);
	sD3DMatrix.y.w = RoundFloat(sD3DMatrix.y.w, BINARY_PRECISION);
	sD3DMatrix.z.x = RoundFloat(sD3DMatrix.z.x, BINARY_PRECISION);
	sD3DMatrix.z.y = RoundFloat(sD3DMatrix.z.y, BINARY_PRECISION);
	sD3DMatrix.z.z = RoundFloat(sD3DMatrix.z.z, BINARY_PRECISION);
	sD3DMatrix.z.w = RoundFloat(sD3DMatrix.z.w, BINARY_PRECISION);
	sD3DMatrix.pos.x = RoundFloat(sD3DMatrix.pos.x, BINARY_PRECISION);
	sD3DMatrix.pos.y = RoundFloat(sD3DMatrix.pos.y, BINARY_PRECISION);
	sD3DMatrix.pos.z = RoundFloat(sD3DMatrix.pos.z, BINARY_PRECISION);
	sD3DMatrix.pos.w = RoundFloat(sD3DMatrix.pos.w, BINARY_PRECISION);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SMatrix::Update(void)
{
	if (Float4x4Equals(&sD3DMatrix, &sD3DOldMatrix))
	{
		return TRUE;
	}

	sD3DOldMatrix = sD3DMatrix;
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//	General Functions.
//////////////////////////////////////////////////////////////////////////
BOOL D3DVectorRectContainPoint(D3DVectorRect* psRect, D3DVECTOR* psVec, float fEdgeThickness)
{
	if (psVec->x > psRect->sBottomRight.x + fEdgeThickness)
		return FALSE;
	if (psVec->x < psRect->sTopLeft.x - fEdgeThickness)
		return FALSE;
	if (psVec->y > psRect->sBottomRight.y + fEdgeThickness)
		return FALSE;
	if (psVec->y < psRect->sTopLeft.y - fEdgeThickness)
		return FALSE;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SUVCornersFull::Rotate(int iTimeClockwise)
{
	SUVCornersFull	sTemp;
	int				i;
	int				j;

	for (i = 0; i < 4; i++)
	{
		j = i + iTimeClockwise;
		if (j >= 4)
		{
			j = j % 4;
		}
		sTemp.asUV[j] = asUV[i];
	}
	memcpy(this, &sTemp, sizeof(SUVCornersFull));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SUVCornersFull::Set(float u1, float v1, float u2, float v2)
{
	asUV[0].x = u1;
	asUV[0].y = v1;
	asUV[1].x = u2;
	asUV[1].y = v1;
	asUV[2].x = u2;
	asUV[2].y = v2;
	asUV[3].x = u1;
	asUV[3].y = v2;
}
