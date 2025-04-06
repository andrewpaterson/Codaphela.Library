#include "GerberCommandModeSet.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommandMeasurementMode::Init(EGerberMeasurementMode eMode)
{
	CGerberCommand::Init(GC_MO);
	meMode = eMode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommandMeasurementMode::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberCommandMeasurementMode::IsMeasurementMode(void)
{
	return true;
}

