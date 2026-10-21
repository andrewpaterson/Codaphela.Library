#ifndef __GERBER_COMMAND_SET_APERTURE_H__
#define __GERBER_COMMAND_SET_APERTURE_H__
#include "GerberCommand.h"


class CGerberCommandSetAperture : public CGerberCommand
{
protected:
	uint32	muiDCode;

public:
	void	Init(uint32 uiDCode);
	void	Kill(void);

	bool	IsSetAperture(void);

	uint32	GetDCode(void);
};


#endif // __GERBER_COMMAND_SET_APERTURE_H__

