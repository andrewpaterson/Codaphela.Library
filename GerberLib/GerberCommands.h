#ifndef __GERBER_COMMANDS_H__
#define __GERBER_COMMANDS_H__
#include "BaseLib/ListVariable.h"
#include "GerberCommandComment.h"
#include "GerberCommandModeSet.h"
#include "GerberCommandFileAttribute.h"
#include "GerberCommandFormatSpecifier.h"
#include "GerberCommandLoadPolarity.h"


class CGerberCommands
{
protected:
	CListVariable	macCommands;

public:
	void							Init(void);
	void							Kill(void);

	CGerberCommandComment*			AddComment(size uiCommentLength);
	CGerberCommandModeSet*			AddModeSet(EGerberMode eMode);
	CGerberCommandFileAttribute*	AddFileAttribute(size iNameLength);
	CGerberCommandFormatSpecifier*  AddFormatSpecifier(uint16 uiXWholes, uint16 uiXDecimals, uint16 uiYWholes, uint16 uiYDecimals);
	CGerberCommandLoadPolarity*		AddLoadPolarity(EGerberPolarity ePolarity);
};


#endif // __GERBER_COMMANDS_H__

