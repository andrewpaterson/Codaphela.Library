#ifndef __BASE_MAP_H__
#define __BASE_MAP_H__
#include "Malloc.h"


//	Map
//	 Key - Pointer to variable sized data
//	  - MapVariable
//	 Key - Pointer to fixed sized data
//	  - MapBlock
//	 Key - Inline variable sized data
//	  - MapVariableInline
//	 Key - Inline fixed sized data
//	  - MapBlockInline


class CBaseMap : public CMalloc
{

};


#endif // __BASE_MAP_H__

