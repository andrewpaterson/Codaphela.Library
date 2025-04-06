#ifndef __GERBER_COMMAND_FILE_ATTRIBUTE_H__
#define __GERBER_COMMAND_FILE_ATTRIBUTE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "GerberCommand.h"


class CGerberCommandFileAttribute : public CGerberCommand
{
protected:
	CChars			szName;
	CArrayChars		szValues;

public:
	void	Init(size iNameLength);
	void	Kill(void);

	bool	IsFileAttribute(void);

	char*	NameText(void);

	char*	AddValue(size iValueLength);
};


#endif // __GERBER_COMMAND_FILE_ATTRIBUTE_H__

