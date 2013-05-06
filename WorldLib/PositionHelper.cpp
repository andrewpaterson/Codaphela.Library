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
#include "PositionHelper.h"


void PositionHelperMakeJump(CPosition* pcPosition, SFloat3* psStart, float fHeight)
{
	CDirectedInterpolator*	pcInterpolator;
	SFloat3				cDir;

	pcPosition->Kill();
	pcPosition->Init();
	pcInterpolator = (CDirectedInterpolator*)pcPosition->Set(PT_DirectedPosition);
	pcInterpolator->SetEndpoints(psStart, psStart);
	Float3Assign(&cDir, 0, -fHeight*2.0f, 0);
	pcInterpolator->SetDirections(&cDir, &cDir);
}


void PositionHelperMakeStationary(CPosition* pcPosition, SFloat3* psStart)
{
	CLinearPositionInterpolator*	pcInterpolator;

	pcPosition->Kill();
	pcPosition->Init();
	pcInterpolator = (CLinearPositionInterpolator*)pcPosition->Set(PT_LinearPosition);
	pcInterpolator->SetEndpoints(psStart, psStart);
}


void PositionHelperMakeQuarterCircle(CPosition* pcPosition, SFloat3* psCenter, float fRadius, int iQuadrant)
{
	CDirectedInterpolator*	pcInterpolator;
	SFloat3				cDir1, cDir2;
	SFloat3				cPos1, cPos2;

	pcPosition->Kill();
	pcPosition->Init();
	pcInterpolator = (CDirectedInterpolator*)pcPosition->Set(PT_DirectedPosition);

	cPos1 = *psCenter;
	cPos2 = *psCenter;
	cDir1 = SFloat3(0, 0, 0);
	cDir2 = cDir1;
	if (iQuadrant == 0)
	{
		cPos1.x -= fRadius;
		cPos2.y -= fRadius;
		cDir1.y -= fRadius;
		cDir2.x -= fRadius;
	}
	else if (iQuadrant == 1)
	{
		cPos1.y -= fRadius;
		cPos2.x += fRadius;
		cDir1.x += fRadius;
		cDir2.y -= fRadius;
	}
	else if (iQuadrant == 2)
	{
		cPos1.x += fRadius;
		cPos2.y += fRadius;
		cDir1.y += fRadius;
		cDir2.x += fRadius;
	}
	else if (iQuadrant == 3)
	{
		cPos1.y += fRadius;
		cPos2.x -= fRadius;
		cDir1.x -= fRadius;
		cDir2.y += fRadius;
	}
	pcInterpolator->SetEndpoints(&cPos1, &cPos2);
	pcInterpolator->SetDirections(&cDir1, &cDir2);
}


void PositionHelperMakeStraightLine(CPosition* pcPosition, SFloat3* psStart, SFloat3* psEnd)
{
	CLinearPositionInterpolator*	pcInterpolator;

	pcPosition->Kill();
	pcPosition->Init();
	pcInterpolator = (CLinearPositionInterpolator*)pcPosition->Set(PT_LinearPosition);
	pcInterpolator->SetEndpoints(psStart, psEnd);
}


void PositionHelperMakeBezierSpline(CPosition* pcPosition, SFloat3* psStartPosition, SFloat3* psStartControl, SFloat3* psEndPosition, SFloat3* psEndControl)
{
	SFloat3				cDir1, cDir2;
	CDirectedInterpolator*	pcInterpolator;

	pcPosition->Kill();
	pcPosition->Init();
	pcInterpolator = (CDirectedInterpolator*)pcPosition->Set(PT_DirectedPosition);

	cDir1 = *psStartControl - *psStartPosition;
	cDir2 = *psEndControl - *psEndPosition;

	pcInterpolator->SetDirections(&cDir1, &cDir2);
	pcInterpolator->SetEndpoints(psStartPosition, psEndPosition);
}


void PositionHelperMakeHalfCos(CPosition* pcPosition, SFloat3* psStart, SFloat3* psEnd, float fAmplitude)
{
	CDirectedInterpolator*	pcInterpolator;
	SFloat3					cDir1;
	SFloat3					cDir3;
	SFloat3					cPerp;
	SFloat3					cNormal;

	pcPosition->Kill();
	pcPosition->Init();
	pcInterpolator = (CDirectedInterpolator*)pcPosition->Set(PT_DirectedPosition);
	
	cDir3 = *psEnd - *psStart;
	Float3Normalize(&cDir3, &cDir3);
	cPerp = SFloat3(0, 0, 1);
	Float3Cross(&cNormal, &cDir3, &cPerp);
	cDir1 = cNormal * (fAmplitude * 2.0f);

	pcInterpolator->SetEndpoints(psStart, psEnd);
	pcInterpolator->SetDirections(&cDir1, &cDir1);
}

