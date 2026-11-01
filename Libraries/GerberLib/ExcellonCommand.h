#ifndef __EXCELLON_COMMAND_H__
#define __EXCELLON_COMMAND_H__
#include "BaseLib/PrimitiveTypes.h"
#include "ExcellonCommandEnum.h"


class CExcellonCommand
{
protected:
	EExcellonCommandEnum	meType;

public:
			void					Init(EExcellonCommandEnum eType);
	virtual void					Kill(void) = 0;

			bool					IsType(EExcellonCommandEnum eType);
			EExcellonCommandEnum	GetType(void);
};


#endif // __EXCELLON_COMMAND_H__

