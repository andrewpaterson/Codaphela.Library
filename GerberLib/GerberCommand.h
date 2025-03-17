#ifndef __GERBER_COMMAND_H__
#define __GERBER_COMMAND_H__
#include "BaseLib/Chars.h"
#include "GerberCommandEnum.h"

class CGerberCommand
{
protected:
	EGerberCommandEnum	meType;

public:
			void Init(EGerberCommandEnum eType);
	virtual void Kill(void) =0;
};


#endif // __GERBER_COMMAND_H__

