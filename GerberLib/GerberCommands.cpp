#include "BaseLib/ConstructorCall.h"
#include "GerberCommands.h"


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
	CGerberCommandComment*	pcCommand;

	pcCommand = (CGerberCommandComment*)macCommands.Add(sizeof(CGerberCommandComment));
	New<CGerberCommandComment>(pcCommand);
	pcCommand->Init(uiCommentLength);

	return pcCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberCommandModeSet* CGerberCommands::AddModeSet(EGerberMode eMode)
{
	CGerberCommandModeSet* pcCommand;

	pcCommand = (CGerberCommandModeSet*)macCommands.Add(sizeof(CGerberCommandModeSet));
	New<CGerberCommandModeSet>(pcCommand);
	pcCommand->Init(eMode);

	return pcCommand;
}

