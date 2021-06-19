#ifndef __LOCAL_MALLOCATOR_H__
#define __LOCAL_MALLOCATOR_H__
#include "Mallocator.h"


class CLocalMallocator : public CMallocator
{
public:
	virtual void	Kill(void) =0;

			BOOL	IsLocal(void);

	virtual BOOL	Read(CFileReader* pcFileReader) =0;
	virtual BOOL	Write(CFileWriter* pcFileWriter) =0;
};


#endif // !__LOCAL_MALLOCATOR_H__

