#ifndef __TEMPORARY_MEMORY_H__
#define __TEMPORARY_MEMORY_H__
#include "PrimitiveTypes.h"


class CTemporaryMemory
{
protected:
	void*	mpvTemp;
	size	muiTempSize;

public:
	void	Init(void);
	void	Kill(void);

	void*	Size(size uiSize);
	void*	Get(void);
};


#endif // __TEMPORARY_MEMORY_H__

