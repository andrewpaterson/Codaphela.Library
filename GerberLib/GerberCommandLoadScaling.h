#ifndef __GERBER_COMMAND_LOAD_SCALING_H__
#define __GERBER_COMMAND_LOAD_SCALING_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Number.h"
#include "GerberCommand.h"


class CGerberCommandLoadScaling : public CGerberCommand
{
protected:
	CNumber		mcScaling;

public:
	void		Init(CNumber* pcScaling);
	void		Kill(void);

	bool		IsLoadScaling(void);

	CNumber*	GetScaling(void);
};


#endif // __GERBER_COMMAND_LOAD_SCALING_H__

