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

	EExcellonUnit	GetUnit(void) const;
	bool			IsInches(void) const;
	bool			IsMillimeters(void) const;
};


#endif // __EXCELLON_COMMAND_UNIT_H__

