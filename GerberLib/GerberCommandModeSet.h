#ifndef __GERBER_COMMAND_MODE_SET_H__
#define __GERBER_COMMAND_MODE_SET_H__
#include "BaseLib/Chars.h"
#include "GerberCommand.h"


enum EGerberMode
{
	GM_Unknown,
	GM_Millimeters,
	GM_Inches,
};


class CGerberCommandModeSet : public CGerberCommand
{
protected:
	EGerberMode meMode;

public:
	void Init(EGerberMode eMode);
	void Kill(void);
};


#endif // __GERBER_COMMAND_MODE_SET_H__


