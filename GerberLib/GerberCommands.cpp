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
		else if (eMode == GPM_Linear)
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

