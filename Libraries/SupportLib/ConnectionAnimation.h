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
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Float4x4.h"
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "Connection.h"
#include "Skeleton.h"


//Where a connection is at fTime, seconds from the start of the animation: its local matrix, relative to its parent's, or to
//the world when it has no parent.
struct SConnectionKey
{
	float32		fTime;
	SFloat4x4	sMatrix;
};


typedef CArrayTemplate<SConnectionKey>	CArrayConnectionKey;


//A track: the keys that move one connection, in time order.  GetMatrix is where they put it at a time.  Between two keys each
//is split into a scale, a rotation and a position (Float4x4Decompose), and they're blended, the rotation by slerp, so that a
//bone stays rigid as it turns.  What the split loses, a shear say, is blended straight, so it's each key itself at its time.
//Before the first key it's at the first, and after the last at the last.  Apply sets the connection's local matrix; its world
//matrix is made by CScene::UpdateConnections.
//
//It moves either mpConnection, in the scene's own animation, or mpBone, in a skeleton's, whose connection is a different one in
//each instance playing it: ApplyTo sets that.
class CConnectionAnimation : public CObject
{
CONSTRUCTABLE(CConnectionAnimation);
DESTRUCTABLE(CConnectionAnimation);
public:
	CArrayConnectionKey	masKeys;
	Ptr<CConnection>	mpConnection;  //What these keys move in the scene's own animation.
	Ptr<CBone>			mpBone;  //What they move in a skeleton's.

	void 			Init(void);
	void 			Free(void) override;
	void			Class(void) override;

	SConnectionKey*	AddKey(SFloat4x4* psMatrix, float32 fTime);
	size			NumKeys(void);
	SConnectionKey*	GetKey(size uiIndex);
	float32			GetDuration(void);

	bool			GetMatrix(SFloat4x4* psDest, float32 fTime);
	void			Apply(float32 fTime);
	void			ApplyTo(Ptr<CConnection> pConnection, float32 fTime);
};


//Tracks that play together.  It lasts as long as its longest track.  It's one of two kinds:
//
//	A skeleton's, with mpSkeleton, each track moving one of its bones.  A walk is one of these.  It moves nothing itself: a
//	CAnimationPlayer bound to an instance with that skeleton plays it on that instance's connections, so many instances share it.
//	The scene's own, with no skeleton, each track moving its connection.  Apply puts them where their tracks have them at fTime.
//
//It never mixes them, as a connection in a shared animation would move whatever instance played it.
class CSceneAnimation : public CObject
{
CONSTRUCTABLE(CSceneAnimation);
DESTRUCTABLE(CSceneAnimation);
public:
	CChars							mszName;
	Ptr<CSkeleton>					mpSkeleton;
	CArray<CConnectionAnimation>	mapTracks;

	void 	Init(void);
	void 	Free(void) override;
	void	Class(void) override;

	float32	GetDuration(void);
	bool	IsSkeletal(void);
	void	Apply(float32 fTime);
};


#endif // __CONNECTION_ANIMATION_H__
