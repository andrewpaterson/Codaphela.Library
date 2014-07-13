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
#include "StandardLib/ObjectSerialiser.h"
#include "StandardLib/ObjectDeserialiser.h"
#include "Sequence.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSequence::Init(void)
{
	masKeyFrames.Init(128);
	miConnectionIndex = -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSequence::KillData(void)
{	
	masKeyFrames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAnimKeyFrame* CSequence::Add(SFloat3* psPosition, SQuaternion* psRotation, float fTime)
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
CAnimKeyFrame* CSequence::Add(SFloat4x4* psMatrix, float fTime)
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
CAnimKeyFrame* CSequence::Add(SFloat3* psPosition, float fTime)
{
	return Add(psPosition, NULL, fTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAnimKeyFrame* CSequence::Add(SQuaternion* psRotation, float fTime)
{
	return Add(NULL, psRotation, fTime);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CSequence::Load(CObjectDeserialiser* pcFile)
{
	ReturnOnFalse(pcFile->ReadInt(&miConnectionIndex));
	ReturnOnFalse(masKeyFrames.Read(pcFile));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CSequence::Save(CObjectSerialiser* pcFile)
{
	ReturnOnFalse(pcFile->WriteInt(miConnectionIndex));
	ReturnOnFalse(masKeyFrames.Write(pcFile));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSequence::Copy(CSequence* pCSequence)
{
	Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSequence::SetConnectionID(int iConnectionID)
{
	miConnectionIndex = iConnectionID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSequence::Dump(void)
{
}

