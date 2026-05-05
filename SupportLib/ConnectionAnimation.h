/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __CONNECTION_ANIMATION_H__
#define __CONNECTION_ANIMATION_H__
#include "BaseLib/ChunkFile.h"
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/Object.h"


enum EKeyframeType
{
	KFT_Position = 0x1,
	KFT_Rotation = 0x2,
	KFT_Scale    = 0x4,
	KFT_Matrix	 = 0x8,
};


struct SAnimKeyFramePRS
{
	SFloat32Vec3			msPosition;
	SQuaternion		msRotation;
	//Scale
};

struct SAnimKeyFrameMatrix
{
	SFloat4x4		msMatrix;
};


struct SAnimKeyFrameData
{
	//union this please.
	float32 f[16];  //This is the sizeof SAnimKeyFramePRS or SAnimKeyFrameMatrix.  Whichever is greater.
};


struct CAnimKeyFrame
{
	int					miType;  //Which parts of the key frame are actually used...
	SAnimKeyFrameData	msf;
	float32				mfTime;  //Time until the next frame.
};


typedef CArrayTemplate<CAnimKeyFrame>	CArrayAnimKeyFrame;


class CConnectionAnimation : public CObject
{
CONSTRUCTABLE(CConnectionAnimation);
DESTRUCTABLE(CConnectionAnimation);
public:
	CArrayAnimKeyFrame	masKeyFrames;
	int					miConnectionIndex;  //The connection this key-frame animates

	void 			Init(void);
	void 			Free(void);
	void			Class(void);

	bool			Load(CObjectReader* pcFile);
	bool			Save(CObjectWriter* pcFile);
	void			Copy(CConnectionAnimation* pcConnection);

	void			Dump(void);
	CAnimKeyFrame*	Add(SFloat4x4* psMatrix, float32 fTime);
	CAnimKeyFrame*	Add(SFloat32Vec3* psPosition, SQuaternion* psRotation, float32 fTime);
	CAnimKeyFrame*	Add(SFloat32Vec3* psPosition, float32 fTime);
	CAnimKeyFrame*	Add(SQuaternion* psRotation, float32 fTime);
	void			SetConnectionID(int iConnectionID);
};


#endif // __CONNECTION_ANIMATION_H__

