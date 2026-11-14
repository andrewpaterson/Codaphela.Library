/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "BaseLib/Float32Vec3.h"
#include "BaseLib/Quaternion.h"
#include "StandardLib/ClassDefines.h"
#include "ConnectionAnimation.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnectionAnimation::Init(void)
{
	PreInit();

	masKeys.Init();
	mpConnection = NULL;
	mpBone = NULL;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnectionAnimation::Free(void)
{
	masKeys.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnectionAnimation::Class(void)
{
	U_Unknown(CArrayConnectionKey, masKeys);
	M_Pointer(mpConnection);
	M_Pointer(mpBone);
}


//////////////////////////////////////////////////////////////////////////
//
// The key goes after those at or before fTime, so that keys stay in time
// order however they're added.
//
//////////////////////////////////////////////////////////////////////////
SConnectionKey* CConnectionAnimation::AddKey(SFloat4x4* psMatrix, float32 fTime)
{
	SConnectionKey*	psKey;
	size			uiIndex;

	uiIndex = masKeys.NumElements();
	while ((uiIndex > 0) && (masKeys.Get(uiIndex - 1)->fTime > fTime))
	{
		uiIndex--;
	}
	psKey = masKeys.InsertAt(uiIndex);
	psKey->fTime = fTime;
	psKey->sMatrix = *psMatrix;
	return psKey;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CConnectionAnimation::NumKeys(void)
{
	return masKeys.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SConnectionKey* CConnectionAnimation::GetKey(size uiIndex)
{
	return masKeys.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The last key's time, 0 with none.
//
//////////////////////////////////////////////////////////////////////////
float32 CConnectionAnimation::GetDuration(void)
{
	if (masKeys.NumElements() == 0)
	{
		return 0.0f;
	}
	return masKeys.Tail()->fTime;
}


//////////////////////////////////////////////////////////////////////////
//
// False, and psDest left as it was, with no keys.
//
//////////////////////////////////////////////////////////////////////////
bool CConnectionAnimation::GetMatrix(SFloat4x4* psDest, float32 fTime)
{
	SConnectionKey*	psBefore;
	SConnectionKey*	psAfter;
	SFloat32Vec3	sBeforeScale;
	SFloat32Vec3	sAfterScale;
	SFloat32Vec3	sBeforePosition;
	SFloat32Vec3	sAfterPosition;
	SQuaternion		sBeforeRotation;
	SQuaternion		sAfterRotation;
	SFloat32Vec3	sScale;
	SFloat32Vec3	sPosition;
	SQuaternion		sRotation;
	SFloat4x4		sBeforeBack;
	SFloat4x4		sAfterBack;
	float32*		pfDest;
	float32*		pfBefore;
	float32*		pfAfter;
	float32*		pfBeforeBack;
	float32*		pfAfterBack;
	size			uiNumKeys;
	size			uiAfter;
	size			i;
	float32			fAlong;

	uiNumKeys = masKeys.NumElements();
	if (uiNumKeys == 0)
	{
		return false;
	}

	uiAfter = 0;
	while ((uiAfter < uiNumKeys) && (masKeys.Get(uiAfter)->fTime <= fTime))
	{
		uiAfter++;
	}
	if (uiAfter == 0)
	{
		*psDest = masKeys.Get(0)->sMatrix;
		return true;
	}
	if (uiAfter == uiNumKeys)
	{
		*psDest = masKeys.Tail()->sMatrix;
		return true;
	}

	//At a key it's the key itself, as splitting it and putting it back loses a little.
	psBefore = masKeys.Get(uiAfter - 1);
	if (psBefore->fTime == fTime)
	{
		*psDest = psBefore->sMatrix;
		return true;
	}
	psAfter = masKeys.Get(uiAfter);
	fAlong = (fTime - psBefore->fTime) / (psAfter->fTime - psBefore->fTime);
	Float4x4Decompose(&sBeforeScale, &sBeforeRotation, &sBeforePosition, &psBefore->sMatrix);
	Float4x4Decompose(&sAfterScale, &sAfterRotation, &sAfterPosition, &psAfter->sMatrix);
	sScale.Init(sBeforeScale.x + (sAfterScale.x - sBeforeScale.x) * fAlong, sBeforeScale.y + (sAfterScale.y - sBeforeScale.y) * fAlong, sBeforeScale.z + (sAfterScale.z - sBeforeScale.z) * fAlong);
	sPosition.Init(sBeforePosition.x + (sAfterPosition.x - sBeforePosition.x) * fAlong, sBeforePosition.y + (sAfterPosition.y - sBeforePosition.y) * fAlong, sBeforePosition.z + (sAfterPosition.z - sBeforePosition.z) * fAlong);
	QuaternionSlerp(&sRotation, &sBeforeRotation, &sAfterRotation, fAlong);
	Float4x4Compose(psDest, &sScale, &sRotation, &sPosition);

	//What splitting each key and putting it back loses, a shear say, is blended straight, so that it's the keys themselves at
	//their times and nothing jumps as a key goes by.
	Float4x4Compose(&sBeforeBack, &sBeforeScale, &sBeforeRotation, &sBeforePosition);
	Float4x4Compose(&sAfterBack, &sAfterScale, &sAfterRotation, &sAfterPosition);
	pfDest = (float32*)psDest;
	pfBefore = (float32*)&psBefore->sMatrix;
	pfAfter = (float32*)&psAfter->sMatrix;
	pfBeforeBack = (float32*)&sBeforeBack;
	pfAfterBack = (float32*)&sAfterBack;
	for (i = 0; i < 16; i++)
	{
		pfDest[i] += (pfBefore[i] - pfBeforeBack[i]) * (1.0f - fAlong) + (pfAfter[i] - pfAfterBack[i]) * fAlong;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing changes with no keys or no connection.
//
//////////////////////////////////////////////////////////////////////////
void CConnectionAnimation::Apply(float32 fTime)
{
	ApplyTo(mpConnection, fTime);
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing moves for a NULL connection, as for a bone an instance hasn't
// got.
//
//////////////////////////////////////////////////////////////////////////
void CConnectionAnimation::ApplyTo(Ptr<CConnection> pConnection, float32 fTime)
{
	if (pConnection.IsNotNull())
	{
		GetMatrix(&pConnection->msLocalMatrix, fTime);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneAnimation::Init(void)
{
	PreInit();

	mszName.Init();
	mpSkeleton = NULL;
	mapTracks.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneAnimation::Free(void)
{
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneAnimation::Class(void)
{
	U_String(mszName);
	M_Pointer(mpSkeleton);
	M_Embedded(mapTracks);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneAnimation::IsSkeletal(void)
{
	return mpSkeleton.IsNotNull();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CSceneAnimation::GetDuration(void)
{
	float32	fDuration;
	float32	fTrack;
	size	i;

	fDuration = 0.0f;
	for (i = 0; i < mapTracks.NumElements(); i++)
	{
		fTrack = mapTracks.Get(i)->GetDuration();
		fDuration = (fTrack > fDuration) ? fTrack : fDuration;
	}
	return fDuration;
}


//////////////////////////////////////////////////////////////////////////
//
// A skeleton's animation moves nothing here: it's played on an instance by
// a CAnimationPlayer bound to it.
//
//////////////////////////////////////////////////////////////////////////
void CSceneAnimation::Apply(float32 fTime)
{
	size	i;

	if (IsSkeletal())
	{
		return;
	}

	for (i = 0; i < mapTracks.NumElements(); i++)
	{
		mapTracks.Get(i)->Apply(fTime);
	}
}
