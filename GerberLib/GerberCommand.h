#ifndef __GERBER_COMMAND_H__
#define __GERBER_COMMAND_H__
#include "BaseLib/Chars.h"


class CGerberCommand
{
protected:
public:
	virtual void Kill(void) =0;
};


#endif // __GERBER_COMMAND_H__

