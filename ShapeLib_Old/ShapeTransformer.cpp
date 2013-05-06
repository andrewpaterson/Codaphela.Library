#include "ShapeTransformer.h"
#include "CoreLib\FastFunctions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphereTrans::Transform(CSphereImm* pcSphere)
{
	pcSphere->mfRadius = mfRadius;
	memcpy_fast_12bytes(&pcSphere->msPosition, mpsPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConeTrans::Transform(CConeImm* pcCone)
{
	//This function is incorrect.
	//It does not re-normalise direction.
//	pcCone->mfLength = mfLength;  -- Recalculate
	pcCone->mfRadius = mfRadius;
	memcpy_fast_12bytes(&pcCone->msPosition, mpsPosition);
//	memcpy_fast_12bytes(&pcCone->msDirection, mpsDirection);  -- Recalculate
	//pcCone->Set();  -- Rrite
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTriangleTrans::Transform(CTriangleImm* pcTriangle)
{
	memcpy_fast_12bytes(&pcTriangle->asPoints[0], mapsPoints[0]);
	memcpy_fast_12bytes(&pcTriangle->asPoints[1], mapsPoints[1]);
	memcpy_fast_12bytes(&pcTriangle->asPoints[2], mapsPoints[2]);
	pcTriangle->Set();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineSegmentTrans::Transform(CLineSegmentImm* pcLineSegment)
{
	memcpy_fast_12bytes(&pcLineSegment->msStart, mpsStart);
	memcpy_fast_12bytes(&pcLineSegment->msEnd, mpsEnd);
	pcLineSegment->Set();
}

