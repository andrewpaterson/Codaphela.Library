#ifndef __MALLOCATORS_H__
#define __MALLOCATORS_H__
#include "MapStringTemplate.h"
#include "Mallocator.h"


class CMallocators
{
protected:
	CMapStringTemplate<CMallocator*>	mmszClasses;

public:
	void			Init(void);
	void			Kill(void);

	BOOL			Add(CMallocator* pcMalloc);

	CMallocator*	Read(CFileReader* pcFileReader);
	BOOL			Write(CFileWriter* pcFileWriter, CMallocator* pcMalloc);
};


#endif // __MALLOCATORS_H__

