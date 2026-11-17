#ifndef __GERBER_COMMAND_APERTURE_BLOCK_H__
#define __GERBER_COMMAND_APERTURE_BLOCK_H__
#include "GerberCommandCompound.h"


//%ABDnn*% to %AB*%.  Defines block aperture Dnn from the contained commands.
class CGerberCommandApertureBlock : public CGerberCommandCompound
{
protected:
	uint32	muiDCode;

public:
	void	Init(uint32 uiDCode);

	bool	IsApertureBlock(void) const;

	uint32	GetDCode(void) const;
};


#endif // __GERBER_COMMAND_APERTURE_BLOCK_H__

