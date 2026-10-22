#include "BaseLib/Logger.h"
#include "ExcellonInterpreter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonInterpreter::Init(CExcellonCommands* pcCommands, CExcellonInterpreterListener* pcListener)
{
	mpcCommands = pcCommands;
	mpcListener = pcListener;

	meUnit = EU_Unknown;
	mapcTools.Init();
	mpcCurrentTool = NULL;
	mbAbsolute = false;
	mbPoint = false;
	msPoint.cX.Zero();
	msPoint.cY.Zero();

	mbRouteMode = false;
	mbLinear = false;
	mbToolDown = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonInterpreter::Kill(void)
{
	mapcTools.Kill();
	mpcListener = NULL;
	mpcCommands = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::Interpret(void)
{
	size	i;
	size	uiNumCommands;

	uiNumCommands = mpcCommands->NumCommands();
	for (i = 0; i < uiNumCommands; i++)
	{
		if (!InterpretCommand(mpcCommands->GetCommand(i)))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EExcellonUnit CExcellonInterpreter::GetUnit(void)
{
	return meUnit;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CExcellonInterpreter::NumTools(void)
{
	return mapcTools.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CExcellonCommandToolDefine* CExcellonInterpreter::GetTool(uint32 uiTool)
{
	size							i;
	size							uiNumElements;
	CExcellonCommandToolDefine*		pcTool;

	uiNumElements = mapcTools.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcTool = *mapcTools.Get(i);
		if (pcTool->GetTool() == uiTool)
		{
			return pcTool;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::InterpretCommand(CExcellonCommand* pcCommand)
{
	switch (pcCommand->GetType())
	{
		case EC_Unit:
			if (meUnit != EU_Unknown)
			{
				return Error("Excellon unit (INCH or METRIC) can only be set once.");
			}
			meUnit = ((CExcellonCommandUnit*)pcCommand)->GetUnit();
			return true;

		case EC_ToolDefine:
			return InterpretToolDefine((CExcellonCommandToolDefine*)pcCommand);

		case EC_Absolute:
			mbAbsolute = true;
			return true;

		case EC_ToolSelect:
			return InterpretToolSelect((CExcellonCommandToolSelect*)pcCommand);

		case EC_Hit:
			return InterpretHit((CExcellonCommandCoordinate*)pcCommand);

		case EC_RouteMove:
			return InterpretRouteMove((CExcellonCommandCoordinate*)pcCommand);

		case EC_RouteLinear:
			return InterpretRouteLinear((CExcellonCommandCoordinate*)pcCommand);

		case EC_ToolDown:
			return InterpretToolDown();

		case EC_ToolUp:
			return InterpretToolUp();

		case EC_DrillMode:
			return InterpretDrillMode();

		case EC_Slot:
			return InterpretSlot((CExcellonCommandSlot*)pcCommand);

		case EC_Comment:
		case EC_FileAttribute:
		case EC_ApertureAttribute:
		case EC_HeaderStart:
		case EC_Format:
		case EC_DeleteAttribute:
		case EC_HeaderEnd:
			return true;

		case EC_EndOfProgram:
			if (mbToolDown)
			{
				return Error("Excellon end of program (M30) with the tool down.");
			}
			return true;

		default:
			return Error("Unknown Excellon command.");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::InterpretToolDefine(CExcellonCommandToolDefine* pcToolDefine)
{
	if (GetTool(pcToolDefine->GetTool()) != NULL)
	{
		return ToolError("Excellon tool T", pcToolDefine->GetTool(), " is already defined.");
	}
	mapcTools.Add(pcToolDefine);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::InterpretToolSelect(CExcellonCommandToolSelect* pcToolSelect)
{
	if (mbToolDown)
	{
		return Error("Excellon tool cannot change with the tool down.");
	}

	//Tool 0 unloads the tool.
	if (pcToolSelect->GetTool() == 0)
	{
		mpcCurrentTool = NULL;
		return true;
	}

	mpcCurrentTool = GetTool(pcToolSelect->GetTool());
	if (mpcCurrentTool == NULL)
	{
		return ToolError("Excellon tool T", pcToolSelect->GetTool(), " is not defined.");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::InterpretHit(CExcellonCommandCoordinate* pcHit)
{
	SGerberPoint	sPoint;

	if (!ValidateMachining())
	{
		return false;
	}
	if (!ResolvePoint(pcHit->GetX(), pcHit->HasX(), pcHit->GetY(), pcHit->HasY(), &sPoint))
	{
		return false;
	}

	if (mbRouteMode)
	{
		//In route mode coordinates continue the route; cut if the tool is down and the motion is linear.
		if (mbToolDown && mbLinear)
		{
			mpcListener->Slot(mpcCurrentTool, &msPoint, &sPoint);
		}
		else if (mbToolDown)
		{
			return Error("Excellon rapid move (G00) with the tool down.");
		}
	}
	else
	{
		mpcListener->Hole(mpcCurrentTool, &sPoint);
	}

	mbPoint = true;
	msPoint.cX.Init(&sPoint.cX);
	msPoint.cY.Init(&sPoint.cY);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::InterpretRouteMove(CExcellonCommandCoordinate* pcMove)
{
	//G00 selects route mode and moves with the tool up.
	if (mbToolDown)
	{
		return Error("Excellon rapid move (G00) with the tool down.");
	}
	mbRouteMode = true;
	mbLinear = false;
	return InterpretHit(pcMove);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::InterpretRouteLinear(CExcellonCommandCoordinate* pcLinear)
{
	if (!mbRouteMode)
	{
		return Error("Excellon linear route (G01) needs route mode (G00).");
	}
	mbLinear = true;
	return InterpretHit(pcLinear);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::InterpretToolDown(void)
{
	if (!mbRouteMode)
	{
		return Error("Excellon tool down (M15) needs route mode (G00).");
	}
	if (mpcCurrentTool == NULL)
	{
		return Error("Excellon tool down (M15) needs a tool.");
	}
	if (!mbPoint)
	{
		return Error("Excellon tool down (M15) needs a position.");
	}
	mbToolDown = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::InterpretToolUp(void)
{
	if (!mbRouteMode)
	{
		return Error("Excellon tool up (M16) needs route mode (G00).");
	}
	mbToolDown = false;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::InterpretDrillMode(void)
{
	if (mbToolDown)
	{
		return Error("Excellon drill mode (G05) with the tool down.");
	}
	mbRouteMode = false;
	mbLinear = false;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::InterpretSlot(CExcellonCommandSlot* pcSlot)
{
	SGerberPoint	sStart;
	SGerberPoint	sEnd;

	if (!ValidateMachining())
	{
		return false;
	}
	if (mbRouteMode)
	{
		return Error("Excellon canned slot (G85) needs drill mode.");
	}

	if (!ResolvePoint(pcSlot->GetStartX(), pcSlot->HasStartX(), pcSlot->GetStartY(), pcSlot->HasStartY(), &sStart))
	{
		return false;
	}

	//Omitted end coordinates are taken from the start.
	sEnd.cX.Init(pcSlot->HasEndX() ? pcSlot->GetEndX() : &sStart.cX);
	sEnd.cY.Init(pcSlot->HasEndY() ? pcSlot->GetEndY() : &sStart.cY);

	mpcListener->Slot(mpcCurrentTool, &sStart, &sEnd);

	mbPoint = true;
	msPoint.cX.Init(&sEnd.cX);
	msPoint.cY.Init(&sEnd.cY);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::ValidateMachining(void)
{
	if (meUnit == EU_Unknown)
	{
		return Error("Excellon unit (INCH or METRIC) must be set before the first hole.");
	}
	if (!mbAbsolute)
	{
		return Error("Excellon absolute mode (G90) must be set before the first hole.");
	}
	if (mpcCurrentTool == NULL)
	{
		return Error("Excellon hole needs a tool.");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::ResolvePoint(CNumber* pcX, bool bX, CNumber* pcY, bool bY, SGerberPoint* psPoint)
{
	//Omitted coordinates are taken from the previous position.
	if (bX)
	{
		psPoint->cX.Init(pcX);
	}
	else if (mbPoint)
	{
		psPoint->cX.Init(&msPoint.cX);
	}
	else
	{
		return Error("Excellon X coordinate omitted but there is no previous hole.");
	}

	if (bY)
	{
		psPoint->cY.Init(pcY);
	}
	else if (mbPoint)
	{
		psPoint->cY.Init(&msPoint.cY);
	}
	else
	{
		return Error("Excellon Y coordinate omitted but there is no previous hole.");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::Error(const char* szError)
{
	gcLogger.Error2(__METHOD__, " ", szError, NULL);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonInterpreter::ToolError(const char* szLeft, uint32 uiTool, const char* szRight)
{
	CChars	sz;

	sz.Init(szLeft);
	sz.Append(uiTool);
	sz.Append(szRight);
	Error(sz.Text());
	sz.Kill();
	return false;
}

