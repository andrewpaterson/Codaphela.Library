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
#ifndef __MOVEMENT_H__
#define __MOVEMENT_H__
#include "Position.h"
#include "Rotation.h"
#include "Animation.h"
#include "TransformNode.h"
#include "DirectXLib/Direct3DGlobal.h"


enum EMovementType
{
	MT_Unknown,
	MT_Matrix,
	MT_PRS
};


class CMovement : public CUnknown
{
BASE_FUNCTIONS(CMovement);
protected:
	int				miType;
	CAnimation*		mpcRotationAnimation;
	CAnimation*		mpcPositionAnimation;
	CAnimation*		mpcMatrixAnimation;
	SMatrix*		mpsOutput;
	SMatrix*		mpsOffset;

public:
	void Init(void);
	void Kill(void);
	void Set(CAnimation* pcRotationAnimation, CAnimation* pcPositionAnimation, SMatrix* psOutput, SMatrix* psOffset);
	void Set(CAnimation* pcMatrixAnimation, SMatrix* psOutput, SMatrix* psOffset);
	void Update(void);
	void Restart(void);

	void SetType(int iType);
	void SetMatrixAnimation(CAnimation* pcAnimation);
	void SetPositionAnimation(CAnimation* pcAnimation);
	void SetRotationAnimation(CAnimation* pcAnimation);
	void SetOffset(SMatrix* psMatrix);
	void SetOutput(SMatrix* psMatrix);
};


#endif //__MOVEMENT_H__

