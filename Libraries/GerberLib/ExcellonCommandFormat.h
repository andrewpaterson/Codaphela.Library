#ifndef __EXCELLON_COMMAND_FORMAT_H__
#define __EXCELLON_COMMAND_FORMAT_H__
#include "ExcellonCommand.h"


//FMAT,1 or FMAT,2.  The version of the Excellon command set.
class CExcellonCommandFormat : public CExcellonCommand
{
protected:
	uint16	muiFormat;

public:
	void	Init(uint16 uiFormat);
	void	Kill(void);

	uint16	GetFormat(void);
};


#endif // __EXCELLON_COMMAND_FORMAT_H__

