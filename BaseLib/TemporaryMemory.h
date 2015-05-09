#ifndef __TEMPORARY_MEMORY_H__
#define __TEMPORARY_MEMORY_H__


class CTemporaryMemory
{
protected:
	void*	mpvTemp;
	size_t	muiTempSize;

public:
	void	Init(void);
	void	Kill(void);

	void*	Size(size_t uiSize);
	void*	Get(void);
};


#endif // __TEMPORARY_MEMORY_H__

