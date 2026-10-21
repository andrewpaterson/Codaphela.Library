#ifndef __GERBER_COMMAND_REGION_H__
#define __GERBER_COMMAND_REGION_H__
#include "GerberCommandCompound.h"


//G36 to G37.  Contains the contours; D01, D02, G01, G02, G03 and G04.
class CGerberCommandRegion : public CGerberCommandCompound
{
public:
	void	Init(void);

	bool	IsRegion(void);
};


#endif // __GERBER_COMMAND_REGION_H__

