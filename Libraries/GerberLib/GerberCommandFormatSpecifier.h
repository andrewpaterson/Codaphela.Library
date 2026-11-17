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
	void	Init(uint16 uiXWholes, uint16 uiXDecimals, uint16 uiYWholes, uint16 uiYDecimals);
	void	Kill(void);

	bool	IsFormatSpecifier(void) const;
	uint16	GetXWholeNumbers(void) const;
	uint16	GetXDecimals(void) const;
	uint16	GetYWholeNumbers(void) const;
	uint16	GetYDecimals(void) const;
};


#endif // __GERBER_COMMAND_FORMAT_SPECIFIER_H__


