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
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ClassDefines.h"
#include "ConnectionAnimation.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConnectionAnimation::Init(void)
{
	PreInit();

	masKeyFrames.Init();
	miConnectionIndex = -1;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConnectionAnimation::Free(void)
{	
	masKeyFrames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnectionAnimation::Class(void)
{
	U_Unknown(CArrayAnimKeyFrame, masKeyFrames);
	U_SInt(miConnectionIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAnimKeyFrame* CConnectionAnimation::Add(SFloat3* psPosition, SQuaternion* psRotation, float fTime)
{
	CAnimKeyFrame*		psKeyFrame;
	int					iType;
	SAnimKeyFramePRS*	psPRS;

	psKeyFrame = masKeyFrames.Add();
	memset(psKeyFrame, 0, sizeof(CAnimKeyFrame));
	iType = 0;
	psPRS = (SAnimKeyFramePRS*)&psKeyFrame->msf;
	if (psRotation)
	{
		psPRS->msRotation = *psRotation;
		iType |= KFT_Rotation;
	}
	if (psPosition)
	{
		psPRS->msPosition = *psPosition;
		iType |= KFT_Position;
	}
	psKeyFrame->mfTime = fTime;
	psKeyFrame->miType = iType;
	return psKeyFrame;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAnimKeyFrame* CConnectionAnimation::Add(SFloat4x4* psMatrix, float fTime)
{
	CAnimKeyFrame*			psKeyFrame;
	int						iType;
	SAnimKeyFrameMatrix*	psM;

	psKeyFrame = masKeyFrames.Add();
	memset(psKeyFrame, 0, sizeof(CAnimKeyFrame));
	iType = KFT_Matrix;
	psM = (SAnimKeyFrameMatrix*)&psKeyFrame->msf;
	psM->msMatrix = *psMatrix;
	psKeyFrame->mfTime = fTime;
	psKeyFrame->miType = iType;
	return psKeyFrame;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAnimKeyFrame* CConnectionAnimation::Add(SFloat3* psPosition, float fTime)
{
	return Add(psPosition, NULL, fTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAnimKeyFrame* CConnectionAnimation::Add(SQuaternion* psRotation, float fTime)
{
	return Add(NULL, psRotation, fTime);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CConnectionAnimation::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadSInt(&miConnectionIndex));
	ReturnOnFalse(masKeyFrames.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CConnectionAnimation::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteSInt(miConnectionIndex));
	ReturnOnFalse(masKeyFrames.Write(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnectionAnimation::Copy(CConnectionAnimation* pCSequence)
{
	Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnectionAnimation::SetConnectionID(int iConnectionID)
{
	miConnectionIndex = iConnectionID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnectionAnimation::Dump(void)
{
}

