#ifndef __GERBER_COMMAND_COMPOUND_H__
#define __GERBER_COMMAND_COMPOUND_H__
#include "GerberCommand.h"


class CGerberCommands;


//A statement that contains other commands; region (G36/G37), block aperture (AB) and step and repeat (SR).
class CGerberCommandCompound : public CGerberCommand
{
protected:
	CGerberCommands*	mpcCommands;

public:
	void				Init(EGerberCommandEnum eType);
	void				Kill(void);

	bool				IsCompound(void);

	CGerberCommands*	GetCommands(void);
};


#endif // __GERBER_COMMAND_COMPOUND_H__

