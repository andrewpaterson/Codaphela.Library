#ifndef __GERBER_GRAPHICS_STATE_H__
#define __GERBER_GRAPHICS_STATE_H__
#include "GerberNumbers.h"
#include "GerberAperture.h"
#include "GerberCommandMeasurementMode.h"
#include "GerberCommandPlotMode.h"
#include "GerberCommandQuadrantMode.h"
#include "GerberCommandLoadPolarity.h"
#include "GerberCommandLoadMirroring.h"


//The graphics state parameters.  2.7
class CGerberGraphicsState
{
public:
	bool					mbFormatSpecified;
	EGerberMeasurementMode	meUnit;

	bool					mbCurrentPoint;
	SGerberPoint			msCurrentPoint;
	CGerberAperture*		mpcCurrentAperture;
	EGerberPlotMode			mePlotMode;
	EGerberQuadrantMode		meQuadrantMode;

	EGerberPolarity			mePolarity;
	EGerberMirroring		meMirroring;
	CNumber					mcRotation;  //Degrees counterclockwise.
	CNumber					mcScaling;

	bool					mbInRegion;

public:
	void	Init(void);
};


#endif // __GERBER_GRAPHICS_STATE_H__

