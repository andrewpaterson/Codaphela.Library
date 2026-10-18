#ifndef __CODABASE_VERSIONS_H__
#define __CODABASE_VERSIONS_H__
#include "MapLongTemplate.h"
#include "MapStringTemplate.h"
#include "ArrayInt64.h"
#include "ArrayChars.h"
#include "NamedIndexedData.h"


//An object as it was before the commit iValidUntil changed it.  The name (with its terminating zero) and then the
//data follow it.
class CVersion
{
public:
	int64		iValidUntil;
	CVersion*	pcNext;
	bool		bExists;
	int32		iNameLength;
	uint32		uiDataSize;

	char*	GetName(void);
	void*	GetData(void);
};


//The index a name had before the commit iValidUntil changed it.  INVALID_O_INDEX means the name did not exist.
struct SNameVersion
{
	int64			iValidUntil;
	SNameVersion*	psNext;
	OIndex			oi;
};


//Old versions of objects and names, kept only while a read transaction may still need them.  Each chain is ordered
//from oldest to newest.  A reader with snapshot S uses the first version whose iValidUntil is greater than S, or the
//store when there is none.  The versions are not durable: no read transaction survives a restart.
class CCodabaseVersions
{
protected:
	CMapLongTemplate<CVersion*>			mcRecords;
	CMapStringTemplate<SNameVersion*>	mcNames;

public:
	void			Init(void);
	void			Kill(void);

	bool			SaveRecord(int64 iValidUntil, OIndex oi, CNamedIndexedData* pcStore);
	bool			SaveName(int64 iValidUntil, const char* szName, CNamedIndexedData* pcStore);

	CVersion*		FindRecord(OIndex oi, int64 iSnapshot);
	bool			FindName(const char* szName, int64 iSnapshot, OIndex* poi);

	void			RemoveUpTo(int64 iOldestSnapshot);

	void			GetIndices(CArrayOIndex* paoi);
	void			GetNames(CArrayChars* paszNames);
	size			NumVersions(void);
};


#endif // __CODABASE_VERSIONS_H__

