/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "SupportLib/ColourARGB.h"
#include "DirectXLib/FlexibleVertexFormats.h"
#include "World.h"
#include "ParticleSystem.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CParticleSystem::Init(void)
{
	memset(&msParticleType, 0, sizeof(SParticleType));
	mpcGraphicsObject = NULL;
	miNumParticles = 0;
	miVertexType = 0;
	miVerticiesPerParticle = 0;
	miTrianglesPerParticle = 0;
	miNumUVLayers = 0;
	mpcGraphicsState = NULL;
	mpcGraphicsMaterial = NULL;
	memset(&msDefaultUVs, 0, sizeof(SUVCorners));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CParticleSystem::Init(int iFlags, int iNumUVLayers, int iUserDataSize, CGraphicsMaterial* pcMaterial, CGraphicsState* pcState, CGraphicsViewport* pcViewport, CWorld* pcWorld)
{
	int		iOffset;

	InitVars();
	msParticleType.iFlags = iFlags = iFlags | PSF_Position | PSF_Size;
	msParticleType.iUserDataSize = iUserDataSize;
	miNumUVLayers = iNumUVLayers;
	Float2Assign(&msDefaultUVs.sUVTopLeft, 0.0f, 0.0f);
	Float2Assign(&msDefaultUVs.sUVBottomRight, 1.0f, 1.0f);

	mpcWorld = pcWorld;
	mpcGraphicsState = pcState;
	mpcGraphicsMaterial = pcMaterial;
	mpcViewport = pcViewport;

	mpcGraphicsObject = mpcWorld->CreateGraphicsObject(true);
	mpcGraphicsObject->AddMatrix(mpcWorld->GetIdentityMatrix());							 

	//Calculate how many verts are used to drawn one particle.
	miVerticiesPerParticle = 6;
	miTrianglesPerParticle = 2;
	if (iFlags & PSF_SingleTriangle)
	{
		miTrianglesPerParticle = 1;
		miVerticiesPerParticle = 3;
	}

	//Particle position is always first (0) so skip it.
	iOffset = sizeof(SFloat3);

	//Size is always present after position.
	msParticleType.iSizeOffset = iOffset;
	iOffset += sizeof(SFloat2);

	//Make sure we need memory for at least one UV coord.
	if (iFlags & (PSF_UVCoords1 | PSF_UVCoords2 | PSF_UVCoords3))
	{
		msParticleType.iUVOffset = iOffset;
	}

	//Find out how much memory we needed for the UV coords.
	if (iFlags & PSF_UVCoords1)
		iOffset += sizeof(SUVCorners);
	if (iFlags & PSF_UVCoords2)
		iOffset += sizeof(SUVCorners);
	if (iFlags & PSF_UVCoords3)
		iOffset += sizeof(SUVCorners);

	//Across axis pointer.
	if (iFlags & PSF_AcrossAxis)
	{
		msParticleType.iAcrossAxisOffset = iOffset;
		iOffset += sizeof(SFloat3*);
	}

	//Up axis pointer.
	if (iFlags & PSF_UpAxis)
	{
		msParticleType.iUpAxisOffset = iOffset;
		iOffset += sizeof(SFloat3*);
	}
	
	//Forward axis pointer.
	if (iFlags & PSF_ForwardAxis)
	{
		msParticleType.iUpAxisOffset = iOffset;
		iOffset += sizeof(SFloat3*);
	}

	//Across rotation.
	if (iFlags & PSF_AcrossRotation)
	{
		msParticleType.iAcrossRotation = iOffset;
		iOffset += sizeof(float);
	}

	//Up rotation.
	if (iFlags & PSF_UpRotation)
	{
		msParticleType.iUpRotation = iOffset;
		iOffset += sizeof(float);
	}

	//Forward rotation.
	if (iFlags & PSF_ForwardRotation)
	{
		msParticleType.iForwardRotation = iOffset;
		iOffset += sizeof(float);
	}

	//Forward cutoff.
	if (iFlags & PSF_FowardCutoff)
	{
		msParticleType.iForwardCutoff = iOffset;
		iOffset += sizeof(float);
	}

	//Transformation matrix pointer.  Who knows what for.
	if (iFlags & PSF_TransformMatrix)
	{
		msParticleType.iMatrixOffset = iOffset;
		iOffset += sizeof(SFloat4x4*);
	}

	//Particle colour
	if (iFlags & PSF_Colour)
	{
		msParticleType.iColourOffset = iOffset;
		iOffset += sizeof(int);
	}

	//Sort index stuff.
	if (iFlags & (PSF_SortBackToFront | PSF_SortFrontToBack))
	{
		msParticleType.iSortIndexOffset = iOffset;
		iOffset += sizeof(int);
	}

	//User data and final structure size.
	msParticleType.iUserDataOffset = iOffset;
	iOffset += iUserDataSize;
	msParticleType.iSize = iOffset;

	//Setup the free list.  I should probably alow the chunk size to be passed in.
	mcParticles.Init(256, msParticleType.iSize);

	miVertexType = gcD3D.GetVertexFormatFor(false, FixBool(iFlags & (PSF_Colour | PSF_Fade)), iNumUVLayers, 0, false);
	gcD3D.AddVertexFormat(miVertexType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CParticleSystem::InitVars(void)
{
	memset(&msParticleType, -1, sizeof(SParticleType));
	mfMaxParticleRadius = 0.0f;
	mcParticles.Init(32, 0);  //Temporarily make this zero so it is safe to kill.
	mcSphere.Init();
	Float3Zero(&mcSphere.msPosition);
	mcSphere.mfRadius = 0.0f;
	miNumParticles = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CParticleSystem::Kill(void)
{
	mpcWorld->ReleaseGraphicsObject(mpcGraphicsObject);
	mcParticles.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CParticleSystem::GetParticleObjectSize(void)
{
	return msParticleType.iSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CParticleSystem::CalcBounding(void)
{
	SFreeListIterator	sIterator;
	void*				pvParticle;
	SFloat3*			psPostition;
	SFloat2*			psSize;
	float				fMaxSize;
	float				fSize;
	SFloat3				cPosition;

	fMaxSize = 0.0f;
	msCenter.Zero();
	miNumParticles = 0;

	//First calculate the size of the biggest particle.
	//Secondly calculate the center of the particle system.
	pvParticle = mcParticles.StartIteration(&sIterator);
	while (pvParticle)
	{
		miNumParticles++;

		psPostition = GetPosition(pvParticle);
		psSize = GetSize(pvParticle);
		fSize = Float2LengthSq(psSize);
		if (fSize > fMaxSize)
		{
			fMaxSize = fSize;
		}

		msCenter += *psPostition;
		pvParticle = mcParticles.Iterate(&sIterator);
	}
	mfMaxParticleRadius = sqrtf(fMaxSize) / 2.0f;
	msCenter /= (float)miNumParticles;

	//Thirdly calculate the radius of the particle system.
	mfParicleSystemRadius = 0.0f;
	pvParticle = mcParticles.StartIteration(&sIterator);
	while (pvParticle)
	{
		psPostition = GetPosition(pvParticle);

		cPosition = msCenter - *psPostition;
		fSize = Float3LengthSq(&cPosition);
		if (fSize > mfParicleSystemRadius)
		{
			mfParicleSystemRadius = fSize;
		}
		pvParticle = mcParticles.Iterate(&sIterator);
	}
	mfParicleSystemRadius = sqrtf(mfParicleSystemRadius);

	//Update the bounding information.
	mfParicleSystemRadius += mfMaxParticleRadius;
	mcSphere.mfRadius = mfParicleSystemRadius;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CParticleSystem::DrawParticles(CCameraInstance* pcCameraInstance)
{
	SFreeListIterator		sIterator;
	void*					pvParticle;
	void*					pvVertex;
	SD3DVertexType*			psVertexType;
	int						iUVOffset;
	int						iColourOffset;
	int						iParticleStyle;
	int						iParticlesNotDrawn;
	CGraphicsPrimitive*		pcGraphicsPrimitive;
	SFloat4x4				sMatrix;

	mpcGraphicsObject->BeginDynamic();

	pcGraphicsPrimitive = mpcGraphicsObject->StartPrimitive(D3DPT_TRIANGLELIST, miVertexType, mpcGraphicsMaterial, mpcGraphicsState, mpcViewport);
	if (!pcGraphicsPrimitive)
	{
		//Couldn't add the graphics primitive for some reason.
		return false;
	}
	pcGraphicsPrimitive->miFlags = GRAPH_PRIM_FLAGS_DRAW;

	miDrawnParticles = 0;
	if (!((msParticleType.iFlags & PSF_Position) && (msParticleType.iFlags & PSF_Size)))
	{
		//Can't handle particles without size and position
		return false;
	}

	iParticleStyle = msParticleType.iFlags & PSF_ParticleTypeMask;
	if (iParticleStyle == PSF_ALIGNMENT_UNKNOWN)
	{
		//Can't draw particles we don't know how to align.
		return false;
	}

	pvVertex = mpcGraphicsObject->AddDynamicVerticies(pcGraphicsPrimitive, miVertexType, miNumParticles * miTrianglesPerParticle, miNumParticles * miVerticiesPerParticle);
	if (!pvVertex)
	{
		//Couldn't add the required number of particles.  bail.
		return false;
	}

	iUVOffset = gcD3D.GetVertexFormatUVOffset(miVertexType, 0);
	iColourOffset = gcD3D.GetVertetFormatColourOffset(miVertexType);

	pcCameraInstance->Adjust(&sMatrix);
	
	pvParticle = mcParticles.StartIteration(&sIterator);
	while (pvParticle)
	{
		psVertexType = gcD3D.GetVertexType(miVertexType);
		pvVertex = DrawUnalignedUnboundParticle(&sMatrix, pvParticle, pvVertex, psVertexType, iUVOffset, iColourOffset);

		//Move on one vertex and get the next particle.
		pvParticle = mcParticles.Iterate(&sIterator);
	}

	//If we allocated to much space because some particles could not be drawn then remove them.
	//This is probably a bad idea.  I must make them degenerate instead.
	iParticlesNotDrawn = miNumParticles - miDrawnParticles;
	if (iParticlesNotDrawn > 0)
	{
		mpcGraphicsObject->RemoveDynamicVerticies(pcGraphicsPrimitive, miVertexType, iParticlesNotDrawn, iParticlesNotDrawn * miVerticiesPerParticle);
	}

	mpcGraphicsObject->EndDynamic();
	return mpcGraphicsObject->Draw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CParticleSystem::Draw(CCameraInstance* pcCameraInstance)
{
	CalcBounding();
	return DrawParticles(pcCameraInstance);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CParticleSystem::DrawUnalignedUnboundParticle(SFloat4x4* psCameraMatrix, void* pvParticle, void* pvVertex, SD3DVertexType* psVertexType, int iVertexUVOffset, int iVertexColourOffset)
{
	SFloat3		sHalfWidth;
	SFloat3		sHalfHeight;

	SFloat3*	psPosition;
	SFloat2*	psSize;
	SFloat3*	psForwardAxis;
	float			fForwardRotation;

	SFloat3*	psVertPosition;
	float			fForwardScale;
	float			fForwardDot;
	int				iColour;
	int				iVertNum;
	
	psPosition = GetPosition(pvParticle);
	psSize = GetSize(pvParticle);

	fForwardDot = ScaledForwardDotProduct(psCameraMatrix->At(), pvParticle);
	if (fForwardDot == 0.0f)
	{
		//Don't add any vertices for this particle.  (Because its evil!)
		return pvVertex;
	}

	fForwardScale = 0.5f;

	if (msParticleType.iFlags & PSF_Shrink)
	{
		fForwardScale *= fForwardDot;
	}
	Float3Scale(&sHalfWidth, psCameraMatrix->Across(), psSize->x * fForwardScale);
	Float3Scale(&sHalfHeight, psCameraMatrix->Up(), -psSize->y * fForwardScale);

	iColour = AdjustedColour(pvParticle, fForwardDot);
	
	CornersForUnalignedUnbound(pvVertex, psVertexType->iD3DVertexSize, &sHalfWidth, &sHalfHeight);
	UVs(pvVertex, psVertexType->iD3DVertexSize, iVertexUVOffset, pvParticle);
	Colours(pvVertex, psVertexType->iD3DVertexSize, iVertexColourOffset, pvParticle, iColour);

	psVertPosition = (SFloat3*)pvVertex;
	if (msParticleType.iFlags & PSF_ForwardRotation)
	{
		if (msParticleType.iFlags  & PSF_ForwardAxis)
		{
			psForwardAxis = GetForwardAxis(pvParticle);
		}
		else
		{
			psForwardAxis = psCameraMatrix->At();
		}
		fForwardRotation = GetForwardRotation(pvParticle);

		for (iVertNum = 0; iVertNum < miVerticiesPerParticle; iVertNum++)
		{
			Float3RotateAboutNormalisedAxis(psVertPosition, *psVertPosition, fForwardRotation, *psForwardAxis);
			psVertPosition = (SFloat3*)RemapSinglePointer(psVertPosition, psVertexType->iD3DVertexSize);
		}
	}

	D3DTVERTEX* p = (D3DTVERTEX*)pvVertex;
	psVertPosition = (SFloat3*)pvVertex;
	for (iVertNum = 0; iVertNum < miVerticiesPerParticle; iVertNum++)
	{
		Float3Add(psVertPosition, psVertPosition, psPosition);
		psVertPosition = (SFloat3*)RemapSinglePointer(psVertPosition, psVertexType->iD3DVertexSize);
	}
	miDrawnParticles++;
	return RemapSinglePointer(pvVertex, psVertexType->iD3DVertexSize * miVerticiesPerParticle);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CParticleSystem::ScaledForwardDotProduct(SFloat3* psCameraForward, void* pvParticle)
{
	SFloat3*	psForwardAxis;
	float			fDot;
	float			fCutoff;
	float			fRange;

	if (!(msParticleType.iFlags & PSF_FowardCutoff))
	{
		return 1.0f;
	}
	psForwardAxis = GetForwardAxis(pvParticle);
	fDot = -Float3Dot(psCameraForward, psForwardAxis);
	fCutoff = GetForwardCutoff(pvParticle);
	fRange = 1.0f - fCutoff;
	fDot = (fDot - fCutoff) / fRange;
	if (fDot > 0.0f)
	{
		return fDot;
	}
	return 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CParticleSystem::AdjustedColour(void* pvParticle, float fForwardDot)
{
	int		iColour;

	if (msParticleType.iFlags & PSF_Colour)
	{
		iColour = GetColour(pvParticle);
	}
	else
	{
		iColour = 0xffffffff;
	}

	if (msParticleType.iFlags & PSF_Fade)
	{
		iColour |= Set8BitAlphaColour(ConvertFloatTo8BitColour(fForwardDot));
	}
	return iColour;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CParticleSystem::CornersForUnalignedUnbound(void* pvVertex, int iD3DVertexSize, SFloat3* psHalfWidth, SFloat3* psHalfHeight)
{
	SFloat3*	psVertPosition;

	psVertPosition = (SFloat3*)pvVertex;
	if (miVerticiesPerParticle == 6)
	{
		//0
		Float3Zero(psVertPosition);
		Float3Subtract(psVertPosition, psVertPosition, psHalfWidth);
		Float3Subtract(psVertPosition, psVertPosition, psHalfHeight);

		//2
		psVertPosition = (SFloat3*)RemapSinglePointer(psVertPosition, iD3DVertexSize);
		Float3Assign(psVertPosition, psHalfHeight);
		Float3Subtract(psVertPosition, psVertPosition, psHalfWidth);

		//1
		psVertPosition = (SFloat3*)RemapSinglePointer(psVertPosition, iD3DVertexSize);
		Float3Assign(psVertPosition, psHalfWidth);
		Float3Subtract(psVertPosition, psVertPosition, psHalfHeight);

		//3
		psVertPosition = (SFloat3*)RemapSinglePointer(psVertPosition, iD3DVertexSize);
		Float3Assign(psVertPosition, psHalfWidth);
		Float3Subtract(psVertPosition, psVertPosition, psHalfHeight);

		//5
		psVertPosition = (SFloat3*)RemapSinglePointer(psVertPosition, iD3DVertexSize);
		Float3Assign(psVertPosition, psHalfHeight);
		Float3Subtract(psVertPosition, psVertPosition, psHalfWidth);

		//4
		psVertPosition = (SFloat3*)RemapSinglePointer(psVertPosition, iD3DVertexSize);
		Float3Assign(psVertPosition, psHalfWidth);
		Float3Add(psVertPosition, psVertPosition, psHalfHeight);
	}
	else if (miVerticiesPerParticle == 3)
	{
		//0
		Float3Zero(psVertPosition);
		Float3Subtract(psVertPosition, psVertPosition, psHalfWidth);
		Float3Subtract(psVertPosition, psVertPosition, psHalfHeight);

		//2
		psVertPosition = (SFloat3*)RemapSinglePointer(psVertPosition, iD3DVertexSize);
		Float3Assign(psVertPosition, psHalfHeight);

		//1
		psVertPosition = (SFloat3*)RemapSinglePointer(psVertPosition, iD3DVertexSize);
		Float3Assign(psVertPosition, psHalfWidth);
		Float3Subtract(psVertPosition, psVertPosition, psHalfHeight);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CParticleSystem::UVs(void* pvVertex, int iD3DVertexSize, int iVertexUVOffset, void * pvParticle)
{
	SFloat2*	psVertUVs;
	SUVCorners*		psUVs;
	int				iLayerNum;
	int				iAssignedNum;
	SUVCorners*		psCurrent;
	int				iUVCoordsFlag;

	if (msParticleType.iUVOffset != -1)
	{
		psUVs = GetUVBase(pvParticle);
	}
	else
	{
		psUVs = NULL;
	}

	iAssignedNum = 0;
	iUVCoordsFlag = PSF_UVCoords1;
	for (iLayerNum = 0; iLayerNum < miNumUVLayers; iLayerNum++)
	{
		//Find out wether we are using the default UV's for this layer or not.
		if (msParticleType.iFlags & iUVCoordsFlag)
		{
			psCurrent = &psUVs[iAssignedNum];
			iAssignedNum++;
		}
		else
		{
			psCurrent = &msDefaultUVs;
		}

		if (miVerticiesPerParticle == 6)
		{
			//0
			psVertUVs = (SFloat2*)RemapSinglePointer(pvVertex, iVertexUVOffset + iLayerNum * sizeof(SFloat2));
			psVertUVs->x = psCurrent->sUVTopLeft.x;
			psVertUVs->y = psCurrent->sUVTopLeft.y;

			//2
			psVertUVs = (SFloat2*)RemapSinglePointer(psVertUVs, iD3DVertexSize);
			psVertUVs->x = psCurrent->sUVTopLeft.x;
			psVertUVs->y = psCurrent->sUVBottomRight.y;

			//1
			psVertUVs = (SFloat2*)RemapSinglePointer(psVertUVs, iD3DVertexSize);
			psVertUVs->x = psCurrent->sUVBottomRight.x;
			psVertUVs->y = psCurrent->sUVTopLeft.y;

			//3
			psVertUVs = (SFloat2*)RemapSinglePointer(psVertUVs, iD3DVertexSize);
			psVertUVs->x = psCurrent->sUVBottomRight.x;
			psVertUVs->y = psCurrent->sUVTopLeft.y;

			//5
			psVertUVs = (SFloat2*)RemapSinglePointer(psVertUVs, iD3DVertexSize);
			psVertUVs->x = psCurrent->sUVTopLeft.x;
			psVertUVs->y = psCurrent->sUVBottomRight.y;

			//4
			psVertUVs = (SFloat2*)RemapSinglePointer(psVertUVs, iD3DVertexSize);
			psVertUVs->x = psCurrent->sUVBottomRight.x;
			psVertUVs->y = psCurrent->sUVBottomRight.y;
		}
		else if (miVerticiesPerParticle == 3)
		{
			//0
			psVertUVs = (SFloat2*)RemapSinglePointer(pvVertex, iVertexUVOffset + iLayerNum * sizeof(SFloat2));
			psVertUVs->x = psCurrent->sUVTopLeft.x;
			psVertUVs->y = psCurrent->sUVTopLeft.y;

			//2
			psVertUVs = (SFloat2*)RemapSinglePointer(psVertUVs, iD3DVertexSize);
			psVertUVs->x = (psCurrent->sUVTopLeft.x + psCurrent->sUVBottomRight.x) * 0.5f;
			psVertUVs->y = psCurrent->sUVBottomRight.y;

			//1
			psVertUVs = (SFloat2*)RemapSinglePointer(psVertUVs, iD3DVertexSize);
			psVertUVs->x = psCurrent->sUVBottomRight.x;
			psVertUVs->y = psCurrent->sUVTopLeft.y;
		}

		iUVCoordsFlag <<= 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CParticleSystem::Colours(void* pvVertex, int iD3DVertexSize, int iVertexColourOffset, void* pvParticle, int iColour)
{
	int		iVertexNum;
	int*	piColour;

	if (msParticleType.iFlags & (PSF_Colour | PSF_Fade))
	{
		piColour = (int*)RemapSinglePointer(pvVertex, iVertexColourOffset);
		for (iVertexNum = 0; iVertexNum < miVerticiesPerParticle; iVertexNum++)
		{
			*piColour = iColour;
			piColour = (int*)RemapSinglePointer(pvVertex, iD3DVertexSize);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CParticleSystem::AddParticle(SFloat3* psPosition, SFloat2* psSize)
{
	void*	pvParticle;

	pvParticle = mcParticles.Add();
	Float3Assign(GetPosition(pvParticle), psPosition);
	Float2Assign(GetSize(pvParticle), psSize);
	return pvParticle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CParticleSystem::AddParticle(SFloat3* psPosition, SFloat2* psSize, SFloat2* psUVTopLeft, SFloat2* psUVBottomRight)
{
	void*	pvParticle;

	pvParticle = AddParticle(psPosition, psSize);
	Float2Assign(&GetUVBase(pvParticle)->sUVTopLeft, psUVTopLeft);
	Float2Assign(&GetUVBase(pvParticle)->sUVBottomRight, psUVBottomRight);
	return pvParticle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CParticleSystem::AddParticle(SFloat3* psPosition, SFloat2* psSize, SFloat2* psUVTopLeft, SFloat2* psUVBottomRight, float fForwardRotation)
{
	void*	pvParticle;

	pvParticle = AddParticle(psPosition, psSize, psUVTopLeft, psUVBottomRight);
	SetForwardRotation(pvParticle, fForwardRotation);
	return pvParticle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* CParticleSystem::GetPosition(void* pvParticle)
{
	return (SFloat3*)pvParticle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat2* CParticleSystem::GetSize(void* pvParticle)
{
	return (SFloat2*)RemapSinglePointer(pvParticle, msParticleType.iSizeOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SUVCorners* CParticleSystem::GetUVBase(void* pvParticle)
{
	return (SUVCorners*)RemapSinglePointer(pvParticle, msParticleType.iUVOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* CParticleSystem::GetForwardAxis(void* pvParticle)
{
	return *((SFloat3**)RemapSinglePointer(pvParticle, msParticleType.iForwardAxisOffset));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CParticleSystem::GetForwardCutoff(void* pvParticle)
{
	return *((float*)RemapSinglePointer(pvParticle, msParticleType.iForwardCutoff));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CParticleSystem::GetColour(void* pvParticle)
{
	return *((int*)RemapSinglePointer(pvParticle, msParticleType.iForwardCutoff));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CParticleSystem::GetForwardRotation(void* pvParticle)
{
	return *((float*)RemapSinglePointer(pvParticle, msParticleType.iForwardRotation));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CParticleSystem::SetForwardRotation(void* pvParticle, float fForwardRotation)
{
	*((float*)RemapSinglePointer(pvParticle, msParticleType.iForwardRotation)) = fForwardRotation;
}

