#ifndef __LOCAL_MALLOCATOR_H__
#define __LOCAL_MALLOCATOR_H__
#include "Mallocator.h"


class CLocalMallocator : public CMallocator
{
public:
	virtual void		Kill(void) override =0;

			bool		IsLocal(void) const override;

	virtual bool		Read(CFileReader* pcFileReader) =0;
	virtual bool		Write(CFileWriter* pcFileWriter) =0;

			const char*	ShortName(void) override;
};


#endif // __LOCAL_MALLOCATOR_H__

