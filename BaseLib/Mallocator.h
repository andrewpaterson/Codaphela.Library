#ifndef __MALLOCATOR_H__
#define __MALLOCATOR_H__


class CFileReader;
class CFileWriter;
class CMallocator
{
public:
	virtual void*	Malloc(size_t tSize) =0;
	virtual void*	Realloc(void* pv, size_t tSize) =0;
	virtual void	Free(void* pv) =0;

	virtual char*	GetName(void) =0;

	//virtual BOOL	Read(CFileReader* pcFileReader) =0;
	//virtual BOOL	Write(CFileWriter* pcFileReader) =0;
};


#endif // __MALLOCATOR_H__

