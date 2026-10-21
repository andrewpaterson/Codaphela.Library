#ifndef __EXCELLON_COMMAND_UNIT_H__
#define __EXCELLON_COMMAND_UNIT_H__
#include "ExcellonCommand.h"


class CExcellonCommandUnit : public CExcellonCommand
{
protected:
	EExcellonUnit	meUnit;

public:
	void			Init(EExcellonUnit eUnit);
	void			Kill(void);

	EExcellonUnit	GetUnit(void);
	bool			IsInches(void);
	bool			IsMillimeters(void);
};


#endif // __EXCELLON_COMMAND_UNIT_H__

