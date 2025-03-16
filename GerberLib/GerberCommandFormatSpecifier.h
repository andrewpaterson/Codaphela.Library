#ifndef __GERBER_COMMAND_FORMAT_SPECIFIER_H__
#define __GERBER_COMMAND_FORMAT_SPECIFIER_H__
#include "BaseLib/Chars.h"
#include "GerberCommand.h"


class CGerberCommandFormatSpecifier : public CGerberCommand
{
protected:
	uint16	muiXWholes;
	uint16	muiXDecimals;
	uint16	muiYWholes;
	uint16	muiYDecimals;

public:
	void Init(uint16 uiXWholes, uint16 uiXDecimals, uint16 uiYWholes, uint16 uiYDecimals);
	void Kill(void);
};


#endif // __GERBER_COMMAND_FORMAT_SPECIFIER_H__


