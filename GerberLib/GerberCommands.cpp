#include "BaseLib/ConstructorCall.h"
#include "GerberCommands.h"


#define ADD_COMMAND(type) 	type*	pcCommand;\
pcCommand = (type*)macCommands.Add(sizeof(type)); \
New<type>(pcCommand);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommands::Init(void)
{
	macCommands.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommands::Kill(void)
{
	size				i;
	size				uiNumElements;
	CGerberCommand*		pcCommand;


	uiNumElements = macCommands.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcCommand = (CGerberCommand*)macCommands.Get(i);
		pcCommand->Kill();
	}
	macCommands.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandComment* CGerberCommands::AddComment(size uiCommentLength)
{
	ADD_COMMAND(CGerberCommandComment);
	pcCommand->Init(uiCommentLength);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandMeasurementMode* CGerberCommands::AddModeSet(EGerberMeasurementMode eMode)
{
	ADD_COMMAND(CGerberCommandMeasurementMode);
	pcCommand->Init(eMode);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandFileAttribute* CGerberCommands::AddFileAttribute(size iNameLength)
{
	ADD_COMMAND(CGerberCommandFileAttribute);
	pcCommand->Init(iNameLength);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandApertureAttribute* CGerberCommands::AddApertureAttribute(size iNameLength)
{
	ADD_COMMAND(CGerberCommandApertureAttribute);
	pcCommand->Init(iNameLength);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandObjectAttribute* CGerberCommands::AddObjectAttribute(size iNameLength)
{
	ADD_COMMAND(CGerberCommandObjectAttribute);
	pcCommand->Init(iNameLength);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandDeleteAttribute* CGerberCommands::AddDeleteAttribute(size iNameLength)
{
	ADD_COMMAND(CGerberCommandDeleteAttribute);
	pcCommand->Init(iNameLength);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandApertureDefine* CGerberCommands::AddApertureDefine(uint32 uiDCode, size iTemplateLength)
{
	ADD_COMMAND(CGerberCommandApertureDefine);
	pcCommand->Init(uiDCode, iTemplateLength);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandSetAperture* CGerberCommands::AddSetAperture(uint32 uiDCode)
{
	ADD_COMMAND(CGerberCommandSetAperture);
	pcCommand->Init(uiDCode);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandOperation* CGerberCommands::AddOperation(EGerberCommandEnum eType)
{
	if ((eType == GC_D01) || (eType == GC_D02) || (eType == GC_D03))
	{
		ADD_COMMAND(CGerberCommandOperation);
		pcCommand->Init(eType);
		return pcCommand;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandFormatSpecifier* CGerberCommands::AddFormatSpecifier(uint16 uiXWholes, uint16 uiXDecimals, uint16 uiYWholes, uint16 uiYDecimals)
{
	ADD_COMMAND(CGerberCommandFormatSpecifier);
	pcCommand->Init(uiXWholes, uiXDecimals, uiYWholes, uiYDecimals);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandLoadPolarity* CGerberCommands::AddLoadPolarity(EGerberPolarity ePolarity)
{
	ADD_COMMAND(CGerberCommandLoadPolarity);
	pcCommand->Init(ePolarity);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandLoadMirroring* CGerberCommands::AddLoadMirroring(EGerberMirroring eMirroring)
{
	ADD_COMMAND(CGerberCommandLoadMirroring);
	pcCommand->Init(eMirroring);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandLoadRotation* CGerberCommands::AddLoadRotation(CNumber* pcDegrees)
{
	ADD_COMMAND(CGerberCommandLoadRotation);
	pcCommand->Init(pcDegrees);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandLoadScaling* CGerberCommands::AddLoadScaling(CNumber* pcScaling)
{
	ADD_COMMAND(CGerberCommandLoadScaling);
	pcCommand->Init(pcScaling);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandPlotMode* CGerberCommands::AddPlotMode(EGerberPlotMode eMode)
{
	if ((eMode == GPM_Linear) || (eMode == GPM_CircularClockwise) || (eMode == GPM_CircularAnticlockwise))
	{
		ADD_COMMAND(CGerberCommandPlotMode);
		if (eMode == GPM_Linear)
		{
			pcCommand->Init(GC_G01, GPM_Linear);
		}
		else if (eMode == GPM_CircularClockwise)
		{
			pcCommand->Init(GC_G02, GPM_CircularClockwise);
		}
		else if (eMode == GPM_CircularAnticlockwise)
		{
			pcCommand->Init(GC_G03, GPM_CircularAnticlockwise);
		}
		return pcCommand;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandQuadrantMode* CGerberCommands::AddQuadrantMode(EGerberQuadrantMode eMode)
{
	if ((eMode == GQM_Single) || (eMode == GQM_Multi))
	{
		ADD_COMMAND(CGerberCommandQuadrantMode);
		if (eMode == GQM_Single)
		{
			pcCommand->Init(GC_G74, GQM_Single);
		}
		else
		{
			pcCommand->Init(GC_G75, GQM_Multi);
		}
		return pcCommand;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandRegion* CGerberCommands::AddRegion(void)
{
	ADD_COMMAND(CGerberCommandRegion);
	pcCommand->Init();
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandApertureBlock* CGerberCommands::AddApertureBlock(uint32 uiDCode)
{
	ADD_COMMAND(CGerberCommandApertureBlock);
	pcCommand->Init(uiDCode);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandStepAndRepeat* CGerberCommands::AddStepAndRepeat(uint32 uiXRepeats, uint32 uiYRepeats, CNumber* pcXStep, CNumber* pcYStep)
{
	ADD_COMMAND(CGerberCommandStepAndRepeat);
	pcCommand->Init(uiXRepeats, uiYRepeats, pcXStep, pcYStep);
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandApertureMacro* CGerberCommands::AddApertureMacro(size iNameLength)
{
	ADD_COMMAND(CGerberCommandApertureMacro);
	pcCommand->Init(iNameLength);
	return pcCommand;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandEndOfFile* CGerberCommands::AddEndOfFile(void)
{
	ADD_COMMAND(CGerberCommandEndOfFile);
	pcCommand->Init();
	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGerberCommands::NumCommands(void)
{
	return macCommands.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommand* CGerberCommands::GetCommand(size uiIndex)
{
	return (CGerberCommand*)macCommands.Get(uiIndex);
}

