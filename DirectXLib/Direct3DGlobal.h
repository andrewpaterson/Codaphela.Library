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
#ifndef __DIRECT_3D_DRAGON__
#define __DIRECT_3D_DRAGON__
#define INITGUID
#define COMPILE_MULTIMON_STUBS
#include <d3dx9.h>
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "BaseLib/GeometricTypes.h"
#include "Constants.h"
#include "TextureStates.h"


//THIS WHOLE FILE IS UP TO SHIT.

//Rewrite it.


#define MIN_HARDWARE_BONES	32


enum ED3DFlags
{
	Flags_Locked =  0x01,
	Flags_Discard = 0x02,
	Flags_Dynamic = 0x04,
};


enum ED3DTextureUsage
{
	TU_Plain,
	TU_AutoMipmap,
	TU_DepthStencil,
	TU_RenderTarget,
	TU_RenderTargetLockable,
	TU_RenderTargetStencil,
	TU_SystemMem,
	TU_Null,
};


struct D3DVectorRect
{
	D3DVECTOR	sTopLeft;
	D3DVECTOR	sBottomRight;
};


struct SIndexBuffer
{
	LPDIRECT3DINDEXBUFFER9	lpD3DIndexBuffer;
	int						iFlags;
	int						iIndexSize;
	void*					pvLockedBuffer;
};


struct SVertexBuffer
{
	LPDIRECT3DVERTEXBUFFER9	lpD3DVertexBuffer;
	int						iFlags;
	int						iVertexSize;
	int						iVertexFormat;  //FVF Only
	int						iNumVerticies;
	void*					pvLockedBuffer;
};


struct STexture
{
	int					iFlags;
	D3DFORMAT			mD3DFormat;
	ED3DTextureUsage	meUsage;
	int					miWidth;
	int					miHeight;
	IDirect3DTexture9*	mpD3DTexture;
	void	Zero(void);
};


struct SSurface
{
	LPDIRECT3DSURFACE9		mpD3DSurface;
	int						iFlags;
	D3DFORMAT				mD3DFormat;
	ED3DTextureUsage		meUsage;
	int						miWidth;
	int						miHeight;
	//int						miStride;

	void	Zero(void);
};


struct SMaterial
{
	D3DMATERIAL9		sD3DMaterial;

	void	Zero(void);
};


struct SMatrix
{
	SFloat4x4		sD3DMatrix;  //Must always be first.
	SFloat4x4		sD3DOldMatrix;  //Used for testing if sD3DMatrix has changed (for vertex caching etc...) 

	void			Zero(void);

	void			Set(SFloat4x4* psMatrix);
	void			Fix(void);
	bool			Update(void);

	SFloat3*		At(void);
	SFloat3*		Up(void);
	SFloat3*		Across(void);
	SFloat3*		Pos(void);

	void			SetPos(SFloat3* psVector);
};


struct SD3DVertexType
{
	int	iD3DVertexFormat;  //Register vertex types so as don't need to keep getting size etc...
	int iD3DVertexSize;
	int iNormalOffest;
	int iColourOffest;
	int iWeightOffest;
	int iUVOffest;
};


struct SUVCorners
{
	SFloat2		sUVTopLeft;
	SFloat2		sUVBottomRight;
};


struct SUVCornersFull
{
	SFloat2		asUV[4];  //0 -> top left ... clockwise.

	void Rotate(int iTimeClockwise);
	void Set(float u1, float v1, float u2, float v2);
};


struct SViewport
{
	D3DVIEWPORT9	sD3DViewport;

	void Zero(void);
};


typedef CArrayTemplate<SD3DVertexType>	CArrayD3DVertexType;


class CMonitor
{
public:
	int							iNumModes;
	D3DADAPTER_IDENTIFIER9 		cAdapterId;
	D3DDISPLAYMODE         		cDispayModes[MAX_DISPLAYMODES];
	int							aiStageStates[MAX_TEX][MAX_STAGE_STATES];
	int							aiSampleStates[MAX_SAMPLE][MAX_SAMPLE_STATES];
	HWND						hWnd;
	int							iSamplerStateID[MAX_SAMPLE];
	int							iStageStateID[MAX_TEX];
	int							aiRenderStates[MAX_RENDER_STATES];

	int							iViewportID;
	bool						bSoftwareMode;

	SIndexBuffer*				mpsCurrentIndexBuffer;
	SMaterial*					mpsCurrentMaterial;
	SVertexBuffer*				mpsCurrentVertexBuffer;
	STexture*					mpsCurrentTexture[MAX_TEX];
	SMatrix*					mpsCurrentMatrix[MAX_WORLD_MATRICES];
	SViewport*					mpsCurrentViewport;
	SSurface*					mpsCurrentSurface;

	D3DCAPS9					sD3DCaps;
	bool						bMixedVertexProcessing;
	bool						bAutoGenerateMipmaps;
	bool						bPureDevice;
	bool						bIndexBuffers;
	SSurface					sBackBuffer;
	D3DFORMAT					iFrameBufferFormat;
	D3DFORMAT					iDepthBufferFormat;
	D3DFORMAT					iTextureFormat;
	STexture					msNullTexture;
	int							iNumHardwareBlendMatricies;
};


class CD3D
{
protected:
	LPDIRECT3D9            	lpD3D;
	LPDIRECT3DDEVICE9      	lpD3DDevice;

public:
	D3DDEVTYPE				meDevType;
	int						iNumMonitors;
	CMonitor				acMonitor[MAX_MONITORS];
	int						iCurrMon;
	CArrayD3DVertexType		cD3DVertTypes;

	float					mfFogStart;
	float					mfFogEnd;
	float					mfFogDensity;
	float					mfPointSize;
	float					mfPointSizeMin;
	float					mfPointSizeMax;
	float					mfPointScaleA;
	float					mfPointScaleB;
	float					mfPointScaleC;
	float					mfTweenFactor;
	float					mfMinTessalation;
	float					mfMaxTessalation;

	bool					mbLogCalls;
	CChars					mszLog;
	size					miLogFlushSize;

	bool					mbAlwaysSetMaterial;
	bool					mbAlwaysSetTexture;
	bool					mbAlwaysSetVertexBuffer;
	bool					mbAlwaysSetIndexBuffer;
	bool					mbAlwaysSetViewport;

	//Initialising the main D3D object.
	void	Init(void);
	bool 	Init(D3DDEVTYPE d);
	void 	Kill(void);

	//Initialising each D3D display device.
	int		GetNVidiaPerfHUDAdapter(int iDefaultAd);
	bool 	InitDevice(int ad, HWND hWnd, unsigned int dx, unsigned int dy, D3DFORMAT fmt, bool bStencil);
	void 	InitGeneral(void);
	void	GetBackBuffer(void);
	void	SetupCache(int ad);
	void	PostInit(int ad, D3DFORMAT D3DFmt, D3DFORMAT D3DDepth);
	void	ResetDevice(int iAD);
	int		GetUsage(int iD3DVertexType);
	void	SetSoftwareVertexProcessing(bool bSoftware);

	//Texture sampler and state.
	void 	FillStageStates(int iAd);
	void 	SetStageState(int iStage, int iState, int iValue);
	void 	FillSamplerStates(int iAd);
	void 	SetSamplerState(int iSampler, int iState, int iValue);

	//Render state.
	void	FillRenderStates(int iAd);
	void	SetRenderState(D3DRENDERSTATETYPE iRenderState, int iValue);
	void	ForceRenderState(D3DRENDERSTATETYPE iRenderState, int iValue);
	int		GetRenderState(D3DRENDERSTATETYPE iRenderState);

	//Matrix.
	void	FillMatricies(int iAd);
	void	SetWorldMatrix(int iMatrixNum, SMatrix* psMatrix);
	void	SetWorldMatrix(int iMatrixNum, SFloat4x4* psMatrix);
	void	CreateMatrix(SMatrix* psMatrix);
	void	InvalidateWorldMatrix(int iMatrixNum);

	//Index buffer.
	void 	CreateIndexBuffer(int iD3DVertexType, int iNumIndices, D3DFORMAT eD3DFormat, SIndexBuffer* psIndexBuffer);
	void 	SetIndexBuffer(SIndexBuffer* psIndexBuffer);
	void 	ReleaseIndexBuffer(SIndexBuffer* psIndexBuffer);
	void 	InvalidateIndexBuffer(void);
	void*	LockIndexBuffer(SIndexBuffer* psIndexBuffer);
	void	UnlockIndexBuffer(SIndexBuffer* psIndexBuffer);
	D3DFORMAT GetIndexBufferFormat(SIndexBuffer* psIndexBuffer);

	//Vertex buffer.
	bool	CreateVertexBuffer(int iD3DVertexType, int iNumVerts, bool bDynamic, SVertexBuffer* psVertexBuffer);
	void	SetVertexBuffer(SVertexBuffer* psVertexBuffer);
	void	ReleaseVertexBuffer(SVertexBuffer* psVertexBuffer);
	void	InvalidateVertexBuffer(void);
	void*	LockVertexBuffer(SVertexBuffer* psVertexBuffer, bool bDiscard = false);
	void	UnlockVertexBuffer(SVertexBuffer* psVertexBuffer);
	D3DFORMAT GetVertexBufferFormat(SVertexBuffer* psVertexBuffer);

	//Texture buffer.
	void 	CreateTexture(int iWidth, int iHeight, D3DFORMAT D3DFormat, ED3DTextureUsage eUsage, bool bDynamic, STexture* psTexture);
	void 	ReleaseTexture(STexture* psTexture);
	void 	SetTexture(int iStage, STexture* psTexture);
	void 	InvalidateTexture(int iStage);
	STexture* GetNullTexture(void);

	//Material.
	void 	CreateMaterial(SMaterial* psMaterial);
	void 	SetMaterial(SMaterial* psMaterial);
	void 	ReleaseMaterial(SMaterial* psMaterial);
	void 	InvalidateMaterial(void);

	//Sample and Stage states.
	void	CreateSamplerState(SSamplerState* psSamplerState);
	void 	SetSamplerState(int iSampler, SSamplerState* psSamplerState);
	void 	ReleaseSamplerState(SSamplerState* psSamplerState);
	void 	InvalidateSamplerState(int iSampler);
	void	CreateStageState(SStageState* psStageState);
	void 	SetStageState(int iStage, SStageState* psStageState);
	void 	ReleaseStageState(SStageState* psStageState);
	void 	InvalidateStageState(int iStage);

	//Surfaces.
	void	CreateSurface(int iWidth, int iHeight, D3DFORMAT D3DFormat, ED3DTextureUsage eUsage, SSurface* psSurface);
	void	ReleaseSurface(SSurface* psSurface);
	void*	LockSurface(SSurface* psSurface);
	void	UnlockSurface(SSurface* psSurface);
	void	UpdateSurface(LPDIRECT3DSURFACE9 psSource, LPDIRECT3DSURFACE9 psDest);
	void	SetRenderTarget(SSurface* psSurface);
	HDC		CaptureSurfaceDC(SSurface* psSurface);
	void	FreeSurfaceDC(SSurface* psSurface, HDC hDC);
	void	CopySurfaceBits(SSurface* psSurfaceDest, RECT* psDestRect, SSurface* psSurfaceSource, RECT* psSourceRect, int iFilterType);
	void	CopySurfaceBits(SSurface* psSurfaceDest, POINT* psDestPoint, SSurface* psSurfaceSource, RECT* psSourceRect);
	void	GetSurface(STexture* psTexture, int iLevel, SSurface* psSurface);
	void	InvalidateSurface(void);

	//Vertex Format.
	void	AddVertexFormat(int iD3DVertexFormat);
	int		GetVertexFormatSize(int iD3DVertexFormat);
	void	RegisterAllFlexibleFormats(void);
	int		GetVertetFormatTextures(int iD3DVertexFormat);
	int		GetVertexFormatMatricies(int iD3DVertexFormat);
	int 	CalculatePositionSize(int iD3DVertexFormat);
 	int 	CalculateNormalSize(int iD3DVertexFormat);
	int 	CalculateColourSize(int iD3DVertexFormat);
	int 	CalculateWeightSize(int iD3DVertexFormat);
	int 	CalculateUVSize(int iD3DVertexFormat);
	int		CalculateVertexFormatSize(int iD3DVertexFormat);
	int		CalculateVertetFormatNormalOffest(int iD3DVertexFormat);
	int		CalculateVertetFormatColourOffest(int iD3DVertexFormat);
	int		CalculateVertetFormatWeightOffset(int iD3DVertexFormat);
	int		CalculateVertetFormatUVOffset(int iD3DVertexFormat);
	int		GetVertexFormatFor(bool bNormal, bool bColour, int iNumberOfTextures, int iNumberOfMatricies, bool bPretransformed);
	SD3DVertexType* GetVertexTypeFor(bool bNormal, bool bColour, int iNumberOfTextures, int iNumberOfMatricies, bool bPretransformed);
	SD3DVertexType* GetVertexType(int iD3DVertexFormat);
	int		GetVertexFormatPositionOffset(int iD3DVertexFormat);
	int		GetVertexFormatNormalOffset(int iD3DVertexFormat);
	int		GetVertetFormatColourOffset(int iD3DVertexFormat);
	int		GetVertetFormatWeightOffset(int iD3DVertexFormat, int iMatrixWeight);
	int		GetVertexFormatUVOffset(int iD3DVertexFormat, int iTextureLayer);

	//Viewports
	void	CreateViewport(SViewport* psViewport);
	void	SetViewport(SViewport* psViewport);
	void	ReleaseViewport(SViewport* psViewport);
	void	InvalidateViewport(void);

	//Device capabilities.
	int		GetDeviceBehaviour(int iAD);
	void	GetAdaptorCapabilities(int iAD);
	D3DFORMAT GetBestDepthBufferFormat(int iAD, D3DFORMAT AdapterFormat, bool bNeedsStencil);
	D3DFORMAT GetBestFrameBufferFormat(int iAD, D3DFORMAT AdapterFormat);
	D3DFORMAT GetBestTextureBufferFormat(int iAD, DWORD D3DUsage, bool bNeedsAlpha);
	D3DFORMAT GetBestIndexBufferFormat(int iAD, int iNumIndices);
	void	GetDisplayModes(int iAD, D3DFORMAT fmt);

	//Drawing
	void 	BeginScene(void);
	void 	EndScene(void);
	void 	ClearBuffer(int colour);
	void	ClearBuffer(int r, int g, int b);
	void 	Present(void);
	void 	Present(HWND hWnd);
	bool	DrawIndexedPrimitive(D3DPRIMITIVETYPE eType, int iStartVertex, int iNumVerticies, int iStartIndex, int iNumPrimitives);
	bool	DrawPrimitive(D3DPRIMITIVETYPE eType, int iStartVertex, int iNumPrimitives);
	bool	SetCamera(SMatrix* psProjection, SMatrix* psView);
	bool	LightEnable(int iLight, D3DLIGHT9* psLight);
	bool	LightDisable(int iLight);
	bool	CreateMesh(int iNumTriangles, int iNumIndices, int iMeshOptions, unsigned int iVertexFormat, ID3DXMesh** ppXMesh);

	//Logging
	void	Log(char* sz);
	void	LogSetRenderState(D3DRENDERSTATETYPE iRenderState, int iValue);
	void	LogSetStageState(int iStage, int iState, int iValue);
	void	LogSetSamplerState(int iSampler, int iState, int iValue);
	void	LogSetIndexBuffer(SIndexBuffer* psIndexBuffer);
	void	LogSetVertexBuffer(SVertexBuffer* psVertexBuffer);
	void	LogSetTexture(int iStage, STexture* psTexture);
	void	LogSetMaterial(SMaterial* psMaterial);
	void	LogSetWorldMatrix(int iMatrixNum, SFloat4x4* psMatrix);
	void	LogSetRenderTarget(SSurface* psSurface);
	void	LogSetViewport(SViewport* psViewport);
	void	LogD3DFILLMODE(CChars* psz, D3DFILLMODE eFillMode);
	void	LogD3DSHADEMODE(CChars* psz, D3DSHADEMODE iValue);
	void	LogD3DBLEND(CChars* psz, D3DBLEND iValue);
	void	LogD3DWRAP(CChars* psz, unsigned int iValue);
	void	LogD3DMATERIALCOLORSOURCE(CChars* psz, D3DMATERIALCOLORSOURCE iValue);
	void	LogD3DVERTEXBLENDFLAGS(CChars* psz, D3DVERTEXBLENDFLAGS iValue);
	void	LogD3DPATCHEDGESTYLE(CChars* psz, D3DPATCHEDGESTYLE iValue);
	void	LogD3DDEBUGMONITORTOKENS(CChars* psz, D3DDEBUGMONITORTOKENS iValue);
	void	LogD3DBLENDOP(CChars* psz, D3DBLENDOP iValue);
	void	LogD3DDEGREE(CChars* psz, D3DDEGREETYPE iValue);
	void	LogD3DRENDERSTATETYPE(CChars* psz, D3DRENDERSTATETYPE eRenderState);
	void	LogD3DRENDERSTATETYPEValue(CChars* psz, D3DRENDERSTATETYPE eRenderState, int iValue);
	void	LogD3DCOLORVALUE(CChars*psz, D3DCOLORVALUE* psColour);
	void	LogD3DUSAGE(CChars* psz, unsigned int uiUsage);
	void	LogD3DFORMAT(CChars* psz, D3DFORMAT eFormat);
	void	LogD3DPOOL(CChars* psz, D3DPOOL ePool);
	void	LogED3DTextureUsage(CChars* psz, ED3DTextureUsage eUsage);
	void	LogED3DFlags(CChars* psz, int eFlags);
	void	LogD3DTEXTURESTAGESTATETYPE(CChars* psz, D3DTEXTURESTAGESTATETYPE eStageStateType);
	void	LogD3DTEXTURESTAGESTATETYPEValue(CChars* psz, D3DTEXTURESTAGESTATETYPE eStageStateType, unsigned int uiValue);
	void	LogD3DTEXTURETRANSFORMFLAGS(CChars* psz, D3DTEXTURETRANSFORMFLAGS eFlags);
	void	LogD3DTEXTUREOP(CChars* psz, D3DTEXTUREOP eTextureOp);
	void	LogD3DTA(CChars* psz, unsigned int uiTa);
	void	LogD3DSAMPLERSTATETYPE(CChars* psz, D3DSAMPLERSTATETYPE eSamplerState);
	void	LogD3DSAMPLERSTATETYPEValue(CChars* psz, D3DSAMPLERSTATETYPE eSamplerState, unsigned int uiValue);
	void	LogD3DTEXTUREADDRESS(CChars* psz, D3DTEXTUREADDRESS eTextureAddress);
	void	LogD3DCOLOR(CChars* psz, D3DCOLOR uiColour);
	void	LogD3DTEXTUREFILTERTYPE(CChars* psz, D3DTEXTUREFILTERTYPE eTextureFilter);
	void 	LogD3DCULL(CChars* psz, D3DCULL eCull);
	void 	LogD3DCMPFUNC(CChars* psz, D3DCMPFUNC eCmpFunc);
	void 	LogD3DSTENCILOP(CChars* psz, D3DSTENCILOP eTextureAddress);
	void 	LogD3DFOGMODE(CChars* psz, D3DFOGMODE eTextureAddress);
	void	LogFVF(CChars* psz, unsigned int fvf);
	void	LogD3DXMATRIX(CChars* psz, SFloat4x4* psMatrix);
	void	LogD3DPRIMITIVETYPE(CChars* psz, D3DPRIMITIVETYPE eType);
};


extern CD3D	gcD3D;


//////////////////////////////////////////////////////////////////////////
//
//	General Functions.
//
//////////////////////////////////////////////////////////////////////////
bool D3DVectorRectContainPoint(D3DVectorRect* psRect, D3DVECTOR* psVec, float fEdgeThickness);


#endif //__DIRECT_3D_DRAGON__

