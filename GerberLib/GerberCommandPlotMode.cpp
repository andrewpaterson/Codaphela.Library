#include "GerberCommandPlotMode.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommandPlotMode::Init(EGerberCommandEnum eType, EGerberPlotMode eMode)
{
	CGerberCommand::Init(eType);
	meMode = eMode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommandPlotMode::Kill(void)
{
}

