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
CGerberCommandModeSet* CGerberCommands::AddModeSet(EGerberMode eMode)
{
	ADD_COMMAND(CGerberCommandModeSet);
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


