#ifndef __TILE_CEL_SOURCE_H__
#define __TILE_CEL_SOURCE_H__
#include "BaseLib/Killable.h"


class CTileCelSource : public CKillable
{
public:
	virtual bool	Matches(void* pvData) =0;
};



#endif // __TILE_CEL_SOURCE_H__

