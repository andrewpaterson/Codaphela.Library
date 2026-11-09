/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela OldDragonLib

Codaphela OldDragonLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela OldDragonLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela OldDragonLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __OLD_DRAGON_SCENE_H__
#define __OLD_DRAGON_SCENE_H__
#include "BaseLib/ArrayTemplate.h"
#include "OldDragonFile.h"


#define OLD_DRAGON_NAME_SIZE		256
#define OLD_DRAGON_MAX_TEXTURES		16
#define OLD_DRAGON_NO_INDEX			-1


struct SOldDragonVec2
{
	float32	x;
	float32	y;
};


struct SOldDragonVec3
{
	float32	x;
	float32	y;
	float32	z;
};


struct SOldDragonMatrix
{
	float32	af[16];
};


//An index is OLD_DRAGON_NO_INDEX where the version doesn't have that information.
struct SOldDragonFace
{
	int32	aiPositions[3];
	int32	aiNormals[3];
	int32	aiUVs[3];
	int32	iMaterial;
	uint32	uiSmoothing;
};


struct SOldDragonKeyFrame
{
	SOldDragonMatrix	sMatrix;
	float32				fTime;
};


class COldDragonMesh
{
public:
	char								mszName[OLD_DRAGON_NAME_SIZE];
	int32								miID;
	CArrayTemplate<SOldDragonVec3>		macPositions;
	CArrayTemplate<SOldDragonFace>		macFaces;
	CArrayTemplate<SOldDragonVec3>		macNormals;
	CArrayTemplate<SOldDragonVec2>		macUVs;

	void	Init(void);
	void	Kill(void);
};


//mpuiPixels is in the order the file has them and is NULL if they couldn't be found.
class COldDragonImage
{
public:
	char	mszName[OLD_DRAGON_NAME_SIZE];
	int32	miID;
	int32	miWidth;
	int32	miHeight;
	int32	miChannels;
	uint8*	mpuiPixels;

	void	Init(void);
	void	Kill(void);
	bool	SetPixels(const uint8* puiPixels, size uiSize);
};


//The colours are only in Into Oblivion's files.  The other versions' colours aren't decoded, as nothing says what they mean.
struct SOldDragonMaterial
{
	char	szName[OLD_DRAGON_NAME_SIZE];
	int32	iID;
	uint32	uiDiffuseColour;
	uint32	uiSpecularColour;
	float32	fSpecularStrength;
	int32	iNumTextures;
	int32	aiTextures[OLD_DRAGON_MAX_TEXTURES];
};


struct SOldDragonLight
{
	char	szName[OLD_DRAGON_NAME_SIZE];
	int32	iID;
	int32	iType;
	uint32	uiColour;
	float32	fDecayStart;
	float32	fRange;
	float32	fIntensity;
	bool	bShadows;
};


struct SOldDragonCamera
{
	char	szName[OLD_DRAGON_NAME_SIZE];
	int32	iID;
	int32	iType;
	float32	fFOV;
	float32	fNear;
	float32	fFar;
};


struct SOldDragonConnection
{
	char				szName[OLD_DRAGON_NAME_SIZE];
	int32				iID;
	SOldDragonMatrix	sWorld;
	SOldDragonMatrix	sLocal;
	bool				bSelected;
	bool				bTopLevel;
	bool				bParity;
};


class COldDragonSequence
{
public:
	char								mszName[OLD_DRAGON_NAME_SIZE];
	int32								miID;
	int32								miConnection;
	CArrayTemplate<SOldDragonKeyFrame>	macKeyFrames;

	void	Init(void);
	void	Kill(void);
};


//Ties an object (a mesh, light or camera) to the connections that place it.  Into Oblivion's links become instances with
//no object, and their nodes become connections.
class COldDragonInstance
{
public:
	char					mszName[OLD_DRAGON_NAME_SIZE];
	int32					miID;
	int32					miType;
	int32					miObject;
	CArrayTemplate<int32>	maiConnections;

	void	Init(void);
	void	Kill(void);
};


class COldDragonScene
{
public:
	EOldDragonVersion						meVersion;
	bool									mbDamaged;
	size									muiSkippedChunks;	//Chunks of a kind the reader doesn't decode.
	CArrayTemplate<COldDragonMesh>			macMeshes;
	CArrayTemplate<COldDragonImage>			macImages;
	CArrayTemplate<SOldDragonMaterial>		macMaterials;
	CArrayTemplate<SOldDragonLight>			macLights;
	CArrayTemplate<SOldDragonCamera>		macCameras;
	CArrayTemplate<SOldDragonConnection>	macConnections;
	CArrayTemplate<COldDragonSequence>		macSequences;
	CArrayTemplate<COldDragonInstance>		macInstances;

	void	Init(void);
	void	Kill(void);

	COldDragonMesh*			AddMesh(void);
	COldDragonImage*		AddImage(void);
	SOldDragonMaterial*		AddMaterial(void);
	SOldDragonLight*		AddLight(void);
	SOldDragonCamera*		AddCamera(void);
	SOldDragonConnection*	AddConnection(void);
	COldDragonSequence*		AddSequence(void);
	COldDragonInstance*		AddInstance(void);
};


//Every face's positions, normals and UVs have to be in range.  This is most of what shows a reader understood a mesh.
bool	ValidateOldDragonMesh(COldDragonMesh* pcMesh);


#endif // __OLD_DRAGON_SCENE_H__

