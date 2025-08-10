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
#ifndef __PARTICLE_SYSTEM_H__
#define __PARTICLE_SYSTEM_H__
#include "DirectXLib/Direct3DGlobal.h"
#include "SupportLib/Sphere.h"
#include "BaseLib/FreeList.h"
#include "GraphicsObject.h"
#include "CameraInstance.h"
#include "BaseLib/PointerRemapper.h"
#include "StandardLib/StandardHeader.h"
#include "SupportLib/ShapeHelper.h"


enum EParticleSystemFlags
{
	PSF_Position =			0x0000001,
	PSF_Size =				0x0000002,
	PSF_Ignored =			0x0000004,  //Just because I can.
	PSF_UpAxis =			0x0000008,
	PSF_ForwardAxis =		0x0000010,
	PSF_Shrink =			0x0000020,  //Depends on PSF_FowardCutoff
	PSF_Fade = 				0x0000040,	//Depends on PSF_FowardCutoff
	PSF_SortBackToFront = 	0x0000080,	//Used by translucent particles.
	PSF_ForwardRotation =	0x0000100,
	PSF_FowardCutoff =		0x0000200,  //Dot products below this are no longer 'forward'.
	PSF_UVCoords1 =			0x0000400,
	PSF_UVCoords2 =			0x0000800,
	PSF_UVCoords3 =			0x0001000,
	PSF_UpRotation =		0x0002000,
	PSF_AcrossRotation =	0x0004000,
	PSF_TransformMatrix =	0x0008000,  //A transform matrix.
	PSF_Colour =			0x0010000,
	PSF_AcrossAxis =		0x0020000,  //If ommited but required will be calculated from up and forward.
	PSF_SortFrontToBack = 	0x0040000,	//Only useful if particles write to the z buffer.
	PSF_SingleTriangle =	0x0080000,  //Use one triangle per particle instead of two.
	PSF_ParticleTypeMask =	0x1800000,
};

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//If the particle systems alignment is not known it will not be drawn.
#define PSF_ALIGNMENT_UNKNOWN					0

//The particle is only aligned to the camera.  
//Up is vertical in screen space.  This is smoke.
#define PSF_UNALIGNED_UNBOUND_PARTICLE			0x0800000

//The particle is aligned to its up axis but does not shrink to fit it.
//When the up axis points to the camera the particle is not drawn.
//This is a leaf.
#define PSF_UP_AXIS_ALIGNED_UNBOUND_PARTICLE	0x1000000

//The particle is aligned to its up axis and shrunk relative to the inverse 
//dot of the up axis and the camera forward.  This a billboard.
#define PSF_UP_AXIS_ALIGNED_BOUND_PARTICLE		0x1800000


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//The normal way to draw particles is using two triangles shown below (with UVs)
//
//	(0,0)  0-2  5   (1,0)
//         |/  /|
//  (0,1)  1  3-4   (1,1)
//
//If the PSF_SingleTriangle flag is set then only one triangle is used
//
//  (0,0)  0---2  (1,0)
//          \ /
//           1  (0.5,1)
//
//Tri-strips are not used because only one particle per render call can be drawn.
//Index buffers are not used because Dynamic Objects do not support them.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//Shown clockwise UV coordinates default to (0,0), (1,0), (1,1) and (0,1)
//or (0,0) (1,0) and (0.5,1) for the single triangle only if the appropriate
//UVCoords flag is ommited.  No memory is assigned for these UVs.
//
//For instance if miNumUVLayers is 3 but only PSF_UVCoords3 is specified then
//only memory for one SUVCorners will be allocated.  The first and second 
//layers will use the defaults.  The third will use whatever is specified in
//the particle.


//Test particle with everything
struct SParticle
{
	SFloat3		sPosition;
	SFloat2		sSize;
	SUVCorners		sUVs1;
	SUVCorners		sUVs2;
	SUVCorners		sUVs3;
	SUVCorners		sUVs4;
	SFloat3*	sAcrossAxis;
	SFloat3*	sUpAxis;
	SFloat3*	sForwardAxis;
	float			fAcrossRotation;
	float			fUpRotation;
	float			fForwardRotation;
	float			fForwardCutoff;
	SFloat4x4*		psMatrix;
	int				dwColour;
	int				iSortIndex;
};


struct SParticleType
{
	int		iFlags;
	int		iUserDataSize;

	int		iSize;	//Size of a particle structure.

	int		iSizeOffset;  //Width and height of particle.
	int		iUVOffset;
	int		iAcrossAxisOffset;
	int		iUpAxisOffset;
	int		iForwardAxisOffset;
	int		iAcrossRotation;
	int		iUpRotation;
	int		iForwardRotation;
	int		iForwardCutoff;
	int		iMatrixOffset;
	int		iColourOffset;
	int		iUserDataOffset;
	int		iSortIndexOffset;
};


class CParticleSystem : public CUnknown
{
CONSTRUCTABLE(CParticleSystem);
protected:
	SParticleType		msParticleType;
	CFreeList			mcParticles;
	CGraphicsObject*	mpcGraphicsObject;
	int					miVertexType;
	int					miVerticiesPerParticle;
	int					miTrianglesPerParticle;
	int					miNumUVLayers;
	SUVCorners			msDefaultUVs;
	CGraphicsState*		mpcGraphicsState;
	CGraphicsMaterial*	mpcGraphicsMaterial;
	CGraphicsViewport*	mpcViewport;
	CSphereHelper		mcSphere;
	CWorld*				mpcWorld;

	//Not valid until Set() it called.
	int					miNumParticles;
	SFloat3				msCenter;
	float				mfMaxParticleRadius;
	float				mfParicleSystemRadius;
	//mcShapeTrans is also not valid until Set() is called.

	//Not valid until Draw() is called.
	int					miDrawnParticles;

public:
	void			Init(void);
	void			Init(int iFlags, int iNumUVLayers, int iUserDataSize, CGraphicsMaterial* pcMaterial, CGraphicsState* pcState, CGraphicsViewport* pcViewport, CWorld* pcWorld);
	void			InitVars(void);
	void			Kill(void);

	int				GetParticleObjectSize(void);
	int				GetUserDataOffset(void);

	bool			Draw(CCameraInstance* pcCameraInstance);
	void			CalcBounding(void);
	bool			DrawParticles(CCameraInstance* pcCameraInstance);
	void*			DrawUnalignedUnboundParticle(SFloat4x4* psCameraMatrix, void* pvParticle, void* pvVertex, SD3DVertexType* psVertexType, int iVertexUVOffset, int iVertexColourOffset);
	float			ScaledForwardDotProduct(SFloat3* pvCameraForward, void* pvParticle);
	int				AdjustedColour(void* pvParticle, float fForwardDot);
	void			CornersForUnalignedUnbound(void* pvVertex, int iD3DVertexSize, SFloat3* psHalfWidth, SFloat3* psHalfHeight);
	void			UVs(void* pvVertex, int iD3DVertexSize, int iVertexUVOffset, void* pvParticle);
	void			Colours(void* pvVertex, int iD3DVertexSize, int iVertexColourOffset, void* pvParticle, int iColour);

	void*			AddParticle(SFloat3* psPosition, SFloat2* psSize);
	void*			AddParticle(SFloat3* psPosition, SFloat2* psSize, SFloat2* psUVTopLeft, SFloat2* psUVBottomRight);
	void*			AddParticle(SFloat3* psPosition, SFloat2* psSize, SFloat2* psUVTopLeft, SFloat2* psUVBottomRight, float fForwardRotation);

	SFloat3*		GetPosition(void* pvParticle);
	SFloat2*		GetSize(void* pvParticle);
	SUVCorners*		GetUVBase(void* pvParticle);
	SFloat3*		GetForwardAxis(void* pvParticle);
	float			GetForwardCutoff(void* pvParticle);
	int				GetColour(void* pvParticle);
	float			GetForwardRotation(void* pvParticle);
	void			SetForwardRotation(void* pvParticle, float fForwardRotation);
};


#endif //__PARTICLE_SYSTEM_H__

