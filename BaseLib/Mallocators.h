#ifndef __MALLOCATORS_H__
#define __MALLOCATORS_H__
#include "MapStringTemplate.h"
#include "Mallocator.h"
#include "MapStringString.h"

class CMallocators
{
protected:
	CMapStringTemplate<CMallocator*>	mmszClasses;
	CMapStringString					mmShortNames;
	bool								mbInitialised;

public:
	void			Init(void);
	void			InitSystem(void);  //Only the null and system allocators: no global memory.
	void			Kill(void);

	bool			Add(CMallocator* pcMalloc);

	CMallocator*	Read(CFileReader* pcFileReader);
	bool			Write(CFileWriter* pcFileWriter, CMallocator* pcMalloc);

protected:
	bool			IsInitialised(void);
};


#endif // __MALLOCATORS_H__

