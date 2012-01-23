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
#ifndef __SEQUENCE_H__
#define __SEQUENCE_H__
#include "BaseLib/ChunkFile.h"
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/StandardHeader.h"
#include "StandardLib/TrackerTemplate.h"


enum EKeyframeType
{
	KFT_Position = 0x1,
	KFT_Rotation = 0x2,
	KFT_Scale    = 0x4,
	KFT_Matrix	 = 0x8,
};


struct SAnimKeyFramePRS
{
	SFloat3			msPosition;
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
	float f[16];  //This is the sizeof SAnimKeyFramePRS or SAnimKeyFrameMatrix.  Whichever is greater.
};


struct CAnimKeyFrame
{
	int					miType;  //Which parts of the key frame are actually used...
	SAnimKeyFrameData	msf;
	float				mfTime;  //Time untill the next frame.
};


typedef CArrayTemplate<CAnimKeyFrame>	CArrayAnimKeyFrame;


class CSequence : public CStandardTrackerObject
{
BASE_FUNCTIONS(CSequence);
public:
	CArrayAnimKeyFrame	masKeyFrames;
	int					miConnectionIndex;  //The connection this keyframe animates

	void 			Init(void);
	void 			Kill(void);
	BOOL			LoadSpecific(CFileReader* pcFile, int iChunkNum);;
	BOOL			Save(CFileWriter* pcFile);
	void			Copy(CSequence* pcConnection);
	void			Dump(void);
	CAnimKeyFrame*	Add(SFloat4x4* psMatrix, float fTime);
	CAnimKeyFrame*	Add(SFloat3* psPosition, SQuaternion* psRotation, float fTime);
	CAnimKeyFrame*	Add(SFloat3* psPosition, float fTime);
	CAnimKeyFrame*	Add(SQuaternion* psRotation, float fTime);
	void			SetConnectionID(int iConnectionID);
};


typedef CTrackerTemplate<CSequence> CSequenceTracker;


#endif // __SEQUENCE_H__

