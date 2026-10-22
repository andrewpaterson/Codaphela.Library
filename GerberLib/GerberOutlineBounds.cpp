#include "GerberGraphicsState.h"
#include "GerberOutlineBounds.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberOutlineBounds::Init(void)
{
	mcPath.Init();
	mcPath.BeginPart(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberOutlineBounds::Kill(void)
{
	mcPath.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberOutlineBounds::Draw(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd)
{
	SFloat64Vec2	sStart;
	SFloat64Vec2	sEnd;

	ToMillimetres(pcState, psStart, &sStart);
	ToMillimetres(pcState, psEnd, &sEnd);
	mcPath.MoveTo(&sStart);
	mcPath.LineTo(&sEnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberOutlineBounds::Arc(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd, SGerberPoint* psCenter, bool bClockwise)
{
	SFloat64Vec2	sStart;
	SFloat64Vec2	sEnd;
	SFloat64Vec2	sCenter;

	//The path includes the axis extremes the arc passes through, not just its ends.
	ToMillimetres(pcState, psStart, &sStart);
	ToMillimetres(pcState, psEnd, &sEnd);
	ToMillimetres(pcState, psCenter, &sCenter);
	mcPath.MoveTo(&sStart);
	mcPath.ArcTo(&sEnd, &sCenter, bClockwise);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberOutlineBounds::GetBounds(CFloat64Rect* pcBounds)
{
	mcPath.CalculateBounds();
	if (!mcPath.HasBounds())
	{
		pcBounds->Init();
		return false;
	}

	pcBounds->Init(*mcPath.GetMinimum(), *mcPath.GetMaximum());
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberOutlineBounds::ToMillimetres(CGerberGraphicsState* pcState, SGerberPoint* psPoint, SFloat64Vec2* psMillimetres)
{
	float64	fUnit;

	fUnit = (pcState->meUnit == GMM_Inches) ? 25.4 : 1.0;
	psMillimetres->Init(psPoint->cX.DoubleValue() * fUnit, psPoint->cY.DoubleValue() * fUnit);
}

