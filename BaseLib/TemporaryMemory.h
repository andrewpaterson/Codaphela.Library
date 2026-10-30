#ifndef __TEMPORARY_MEMORY_H__
#define __TEMPORARY_MEMORY_H__
#include "PrimitiveTypes.h"


class CTemporaryMemory
{
protected:
	void*	mpvTemp;
	lsize	muiTempSize;

public:
	void	Init(void);
	void	Kill(void);

	void*	Size(lsize uiSize);
	void*	Get(void);
};


#endif // __TEMPORARY_MEMORY_H__

