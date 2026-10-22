#include "GerberGraphicsState.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberGraphicsState::Init(void)
{
	mbFormatSpecified = false;
	meUnit = GMM_Unknown;

	mbCurrentPoint = false;
	msCurrentPoint.cX.Zero();
	msCurrentPoint.cY.Zero();
	mpcCurrentAperture = NULL;
	mePlotMode = GPM_Unknown;
	meQuadrantMode = GQM_Unknown;

	mePolarity = GP_Dark;
	meMirroring = GLM_None;
	mcRotation.Zero();
	mcScaling.Init(1);

	mbInRegion = false;
}

