#include "BaseLib/Logger.h"
#include "GerberMacroVariables.h"
#include "GerberInterpreter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberInterpreter::Init(CGerberCommands* pcCommands, CGerberInterpreterListener* pcListener)
{
	mpcCommands = pcCommands;
	mpcListener = pcListener;
	mcState.Init();
	mcApertures.Init();
	mapcMacros.Init();
	mbEndOfFile = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberInterpreter::Kill(void)
{
	mapcMacros.Kill();
	mcApertures.Kill();
	mpcListener = NULL;
	mpcCommands = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::Interpret(void)
{
	return InterpretCommands(mpcCommands);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberGraphicsState* CGerberInterpreter::GetGraphicsState(void)
{
	return &mcState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberApertures* CGerberInterpreter::GetApertures(void)
{
	return &mcApertures;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandApertureMacro* CGerberInterpreter::GetMacro(char* szName)
{
	size							i;
	size							uiNumElements;
	CGerberCommandApertureMacro*	pcMacro;

	uiNumElements = mapcMacros.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcMacro = *mapcMacros.Get(i);
		if (strcmp(pcMacro->NameText(), szName) == 0)
		{
			return pcMacro;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::InterpretCommands(CGerberCommands* pcCommands)
{
	size	i;
	size	uiNumCommands;

	uiNumCommands = pcCommands->NumCommands();
	for (i = 0; i < uiNumCommands; i++)
	{
		if (mbEndOfFile)
		{
			break;
		}
		if (!InterpretCommand(pcCommands->GetCommand(i)))
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
bool CGerberInterpreter::InterpretCommand(CGerberCommand* pcCommand)
{
	if (pcCommand->IsComment())
	{
		return true;
	}
	else if (pcCommand->IsMeasurementMode())
	{
		return InterpretMeasurementMode((CGerberCommandMeasurementMode*)pcCommand);
	}
	else if (pcCommand->IsFormatSpecifier())
	{
		return InterpretFormatSpecifier((CGerberCommandFormatSpecifier*)pcCommand);
	}
	else if (pcCommand->IsApertureMacro())
	{
		return InterpretApertureMacro((CGerberCommandApertureMacro*)pcCommand);
	}
	else if (pcCommand->IsApertureDefine())
	{
		return InterpretApertureDefine((CGerberCommandApertureDefine*)pcCommand);
	}
	else if (pcCommand->IsSetAperture())
	{
		return InterpretSetAperture((CGerberCommandSetAperture*)pcCommand);
	}
	else if (pcCommand->IsPlotMode())
	{
		return InterpretPlotMode((CGerberCommandPlotMode*)pcCommand);
	}
	else if (pcCommand->IsQuadrantMode())
	{
		mcState.meQuadrantMode = ((CGerberCommandQuadrantMode*)pcCommand)->GetMode();
		return true;
	}
	else if (pcCommand->IsOperation())
	{
		return InterpretOperation((CGerberCommandOperation*)pcCommand);
	}
	else if (pcCommand->IsLoadPolarity())
	{
		mcState.mePolarity = ((CGerberCommandLoadPolarity*)pcCommand)->IsClear() ? GP_Clear : GP_Dark;
		return true;
	}
	else if (pcCommand->IsLoadMirroring())
	{
		mcState.meMirroring = ((CGerberCommandLoadMirroring*)pcCommand)->GetMirroring();
		return true;
	}
	else if (pcCommand->IsLoadRotation())
	{
		mcState.mcRotation.Init(((CGerberCommandLoadRotation*)pcCommand)->GetRotation());
		return true;
	}
	else if (pcCommand->IsLoadScaling())
	{
		mcState.mcScaling.Init(((CGerberCommandLoadScaling*)pcCommand)->GetScaling());
		return true;
	}
	else if (pcCommand->IsRegion())
	{
		return InterpretRegion((CGerberCommandRegion*)pcCommand);
	}
	else if (pcCommand->IsApertureBlock())
	{
		return InterpretApertureBlock((CGerberCommandApertureBlock*)pcCommand);
	}
	else if (pcCommand->IsStepAndRepeat())
	{
		return InterpretStepAndRepeat((CGerberCommandStepAndRepeat*)pcCommand);
	}
	else if (pcCommand->IsFileAttribute() || pcCommand->IsApertureAttribute() || pcCommand->IsObjectAttribute() || pcCommand->IsDeleteAttribute())
	{
		//Attributes do not affect the image.
		return true;
	}
	else if (pcCommand->IsEndOfFile())
	{
		mbEndOfFile = true;
		return true;
	}
	return Error("Unknown command.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::InterpretMeasurementMode(CGerberCommandMeasurementMode* pcMeasurementMode)
{
	//MO must be used once and only once.  4.2.1
	if (mcState.meUnit != GMM_Unknown)
	{
		return Error("Unit (MO) can only be set once.");
	}

	if (pcMeasurementMode->IsMillimeters())
	{
		mcState.meUnit = GMM_Millimeters;
	}
	else if (pcMeasurementMode->IsInches())
	{
		mcState.meUnit = GMM_Inches;
	}
	else
	{
		return Error("Unit (MO) must be MM or IN.");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::InterpretFormatSpecifier(CGerberCommandFormatSpecifier* pcFormatSpecifier)
{
	//FS must be used once and only once.  The parser has already scaled the coordinates.  4.2.2
	if (mcState.mbFormatSpecified)
	{
		return Error("Format Specification (FS) can only be set once.");
	}
	mcState.mbFormatSpecified = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::InterpretApertureMacro(CGerberCommandApertureMacro* pcMacro)
{
	CChars	sz;

	if (GetMacro(pcMacro->NameText()) != NULL)
	{
		sz.Init("Aperture macro [");
		sz.Append(pcMacro->NameText());
		sz.Append("] is already defined.");
		Error(sz.Text());
		sz.Kill();
		return false;
	}

	mapcMacros.Add(&pcMacro);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::InterpretApertureDefine(CGerberCommandApertureDefine* pcDefine)
{
	char*							szTemplate;
	EGerberApertureType				eType;
	CGerberAperture*				pcAperture;
	CGerberCommandApertureMacro*	pcMacro;
	size							i;
	size							uiNumParameters;
	CChars							sz;

	szTemplate = pcDefine->TemplateText();
	pcMacro = NULL;
	if (strcmp(szTemplate, "C") == 0)
	{
		eType = GAT_Circle;
	}
	else if (strcmp(szTemplate, "R") == 0)
	{
		eType = GAT_Rectangle;
	}
	else if (strcmp(szTemplate, "O") == 0)
	{
		eType = GAT_Obround;
	}
	else if (strcmp(szTemplate, "P") == 0)
	{
		eType = GAT_Polygon;
	}
	else
	{
		eType = GAT_Macro;
		pcMacro = GetMacro(szTemplate);
		if (pcMacro == NULL)
		{
			sz.Init("Aperture macro [");
			sz.Append(szTemplate);
			sz.Append("] is not defined.");
			Error(sz.Text());
			sz.Kill();
			return false;
		}
	}

	pcAperture = mcApertures.Add(pcDefine->GetDCode(), eType);
	if (pcAperture == NULL)
	{
		return DCodeError("Aperture D", pcDefine->GetDCode(), " is already defined.");
	}

	uiNumParameters = pcDefine->NumParameters();
	for (i = 0; i < uiNumParameters; i++)
	{
		pcAperture->AddParameter(pcDefine->GetParameter(i));
	}

	if (eType == GAT_Macro)
	{
		pcAperture->SetMacro(pcMacro);
		return EvaluateMacro(pcAperture);
	}
	else
	{
		return ValidateStandardAperture(pcAperture);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::ValidateStandardAperture(CGerberAperture* pcAperture)
{
	size		uiMinimum;
	size		uiMaximum;
	size		uiNumParameters;
	size		i;
	CNumber*	pcVertices;
	CNumber		cLimit;

	switch (pcAperture->GetType())
	{
		case GAT_Circle:
			uiMinimum = 1;
			uiMaximum = 2;
			break;
		case GAT_Rectangle:
		case GAT_Obround:
			uiMinimum = 2;
			uiMaximum = 3;
			break;
		case GAT_Polygon:
			uiMinimum = 2;
			uiMaximum = 4;
			break;
		default:
			return Error("Not a standard aperture.");
	}

	uiNumParameters = pcAperture->NumParameters();
	if ((uiNumParameters < uiMinimum) || (uiNumParameters > uiMaximum))
	{
		return DCodeError("Aperture D", pcAperture->GetDCode(), " has the wrong number of parameters.");
	}

	for (i = 0; i < uiNumParameters; i++)
	{
		if ((pcAperture->GetType() == GAT_Polygon) && (i == 2))
		{
			//The polygon rotation can be negative.
			continue;
		}
		if (pcAperture->GetParameter(i)->IsNegative())
		{
			return DCodeError("Aperture D", pcAperture->GetDCode(), " has a negative size.");
		}
	}

	if (pcAperture->GetType() == GAT_Polygon)
	{
		//3 to 12 vertices.  4.4.5
		pcVertices = pcAperture->GetParameter(1);
		if (!pcVertices->IsInteger() || pcVertices->LessThan(cLimit.Init(3)) || pcVertices->GreaterThan(cLimit.Init(12)))
		{
			return DCodeError("Aperture D", pcAperture->GetDCode(), " polygon must have 3 to 12 vertices.");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::EvaluateMacro(CGerberAperture* pcAperture)
{
	CGerberMacroVariables			cVariables;
	CGerberCommandApertureMacro*	pcMacro;
	size							i;
	size							uiNum;
	bool							bResult;

	//The AD parameters are $1, $2 ...  4.5.4.1
	cVariables.Init();
	uiNum = pcAperture->NumParameters();
	for (i = 0; i < uiNum; i++)
	{
		cVariables.Set((uint32)(i + 1), pcAperture->GetParameter(i));
	}

	bResult = true;
	pcMacro = pcAperture->GetMacro();
	uiNum = pcMacro->NumPrimitives();
	for (i = 0; i < uiNum; i++)
	{
		bResult = pcMacro->GetPrimitive(i)->Evaluate(&cVariables, pcAperture->GetPrimitives());
		if (!bResult)
		{
			DCodeError("Could not evaluate aperture macro for aperture D", pcAperture->GetDCode(), ".");
			break;
		}
	}

	cVariables.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::InterpretSetAperture(CGerberCommandSetAperture* pcSetAperture)
{
	CGerberAperture*	pcAperture;

	pcAperture = mcApertures.Get(pcSetAperture->GetDCode());
	if (pcAperture == NULL)
	{
		return DCodeError("Aperture D", pcSetAperture->GetDCode(), " is not defined.");
	}
	mcState.mpcCurrentAperture = pcAperture;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::InterpretPlotMode(CGerberCommandPlotMode* pcPlotMode)
{
	if (pcPlotMode->IsLinear())
	{
		mcState.mePlotMode = GPM_Linear;
	}
	else if (pcPlotMode->IsClockwise())
	{
		mcState.mePlotMode = GPM_CircularClockwise;
	}
	else
	{
		mcState.mePlotMode = GPM_CircularAnticlockwise;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::ResolvePoint(CGerberCommandOperation* pcOperation, SGerberPoint* psPoint)
{
	//Omitted coordinates are taken from the current point (modal coordinates).  2.7
	if (pcOperation->HasX())
	{
		psPoint->cX.Init(pcOperation->GetX());
	}
	else if (mcState.mbCurrentPoint)
	{
		psPoint->cX.Init(&mcState.msCurrentPoint.cX);
	}
	else
	{
		return Error("X coordinate omitted but the current point is undefined.");
	}

	if (pcOperation->HasY())
	{
		psPoint->cY.Init(pcOperation->GetY());
	}
	else if (mcState.mbCurrentPoint)
	{
		psPoint->cY.Init(&mcState.msCurrentPoint.cY);
	}
	else
	{
		return Error("Y coordinate omitted but the current point is undefined.");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::InterpretOperation(CGerberCommandOperation* pcOperation)
{
	SGerberPoint	sPoint;

	//The unit and the coordinate format are needed by every operation.  2.7
	if (mcState.meUnit == GMM_Unknown)
	{
		return Error("Unit (MO) must be set before the first operation.");
	}
	if (!mcState.mbFormatSpecified)
	{
		return Error("Format Specification (FS) must be set before the first operation.");
	}

	if (!ResolvePoint(pcOperation, &sPoint))
	{
		return false;
	}

	if (pcOperation->IsMove())
	{
		mpcListener->Move(&mcState, &sPoint);
	}
	else if (pcOperation->IsPlot())
	{
		if (!InterpretPlot(pcOperation, &sPoint))
		{
			return false;
		}
	}
	else if (pcOperation->IsFlash())
	{
		if (mcState.mpcCurrentAperture == NULL)
		{
			return Error("Flash (D03) needs a current aperture.");
		}
		mpcListener->Flash(&mcState, &sPoint);
	}

	mcState.mbCurrentPoint = true;
	mcState.msCurrentPoint.cX.Init(&sPoint.cX);
	mcState.msCurrentPoint.cY.Init(&sPoint.cY);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::InterpretPlot(CGerberCommandOperation* pcOperation, SGerberPoint* psPoint)
{
	SGerberPoint	sCenter;

	if (!mcState.mbCurrentPoint)
	{
		return Error("Plot (D01) needs a current point to start from.");
	}

	if (!mcState.mbInRegion)
	{
		//Only the solid circle can stroke draws and arcs.  2.4
		if (mcState.mpcCurrentAperture == NULL)
		{
			return Error("Plot (D01) needs a current aperture.");
		}
		if (!mcState.mpcCurrentAperture->IsSolidCircle())
		{
			return DCodeError("Plot (D01) needs a solid circle aperture but D", mcState.mpcCurrentAperture->GetDCode(), " is not.");
		}
	}

	//The plot state is initially undefined (2.7) but Ucamco's own example files (4.6.4 Nested blocks, sample_macro)
	//plot without a G01, so an undefined plot state is linear.
	if ((mcState.mePlotMode == GPM_Linear) || (mcState.mePlotMode == GPM_Unknown))
	{
		mpcListener->Draw(&mcState, &mcState.msCurrentPoint, psPoint);
		return true;
	}
	else
	{
		if (mcState.meQuadrantMode == GQM_Unknown)
		{
			return Error("G75 must be issued before the first circular plot (D01).");
		}
		if (mcState.meQuadrantMode == GQM_Single)
		{
			return Error("Single quadrant (G74) arcs are not supported.");
		}
		if (!pcOperation->HasI() || !pcOperation->HasJ())
		{
			return Error("Circular plot (D01) needs both I and J.");
		}

		//The center is the start point plus the I and J offsets.  4.7.2
		sCenter.cX.Init(&mcState.msCurrentPoint.cX)->Add(pcOperation->GetI());
		sCenter.cY.Init(&mcState.msCurrentPoint.cY)->Add(pcOperation->GetJ());
		mpcListener->Arc(&mcState, &mcState.msCurrentPoint, psPoint, &sCenter, mcState.mePlotMode == GPM_CircularClockwise);
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::InterpretRegion(CGerberCommandRegion* pcRegion)
{
	bool	bResult;

	mcState.mbInRegion = true;
	mpcListener->BeginRegion(&mcState);

	bResult = InterpretCommands(pcRegion->GetCommands());

	mcState.mbInRegion = false;
	if (bResult)
	{
		mpcListener->EndRegion(&mcState);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::InterpretApertureBlock(CGerberCommandApertureBlock* pcBlock)
{
	CGerberAperture*	pcAperture;

	if (mcApertures.Get(pcBlock->GetDCode()) != NULL)
	{
		return DCodeError("Aperture D", pcBlock->GetDCode(), " is already defined.");
	}

	mpcListener->BeginBlock(&mcState, pcBlock);
	if (!InterpretCommands(pcBlock->GetCommands()))
	{
		return false;
	}

	//The block aperture is available after the AB statement, also outside an enclosing AB statement.  4.11
	pcAperture = mcApertures.Add(pcBlock->GetDCode(), GAT_Block);
	pcAperture->SetBlock(pcBlock);

	//The graphics state remains as at the end of the block, except for the current point which is undefined.  4.11
	mcState.mbCurrentPoint = false;
	mpcListener->EndBlock(&mcState, pcAperture);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::InterpretStepAndRepeat(CGerberCommandStepAndRepeat* pcStepAndRepeat)
{
	mpcListener->BeginStepAndRepeat(&mcState, pcStepAndRepeat);
	if (!InterpretCommands(pcStepAndRepeat->GetCommands()))
	{
		return false;
	}

	//The current point is undefined after an SR statement.  4.12
	mcState.mbCurrentPoint = false;
	mpcListener->EndStepAndRepeat(&mcState);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::Error(const char* szError)
{
	gcLogger.Error2(__METHOD__, " ", szError, NULL);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberInterpreter::DCodeError(const char* szLeft, uint32 uiDCode, const char* szRight)
{
	CChars	sz;

	sz.Init(szLeft);
	sz.Append(uiDCode);
	sz.Append(szRight);
	Error(sz.Text());
	sz.Kill();
	return false;
}

