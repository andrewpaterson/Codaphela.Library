#ifndef __GERBER_COMMAND_ATTRIBUTE_H__
#define __GERBER_COMMAND_ATTRIBUTE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "GerberCommand.h"


class CGerberCommandAttribute : public CGerberCommand
{
protected:
	CChars			mszName;
	CArrayChars		mszValues;

public:
	void	Init(EGerberCommandEnum eType, size iNameLength);
	void	Kill(void);

	char*	AddValue(size iValueLength);

	char*	NameText(void);

	size	NumValues(void);
	char*	ValueText(size uiIndex);
};


#endif // __GERBER_COMMAND_ATTRIBUTE_H__

