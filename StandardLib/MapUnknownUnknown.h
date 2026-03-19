#ifndef __MAP_UNKNOWN_UNKNOWN_H__
#define __MAP_UNKNOWN_UNKNOWN_H__
#include "BaseLib/MapPtrPtr.h"
#include "MapCommonUnknown.h"


class CMapUnknownUnknown : public CMapCommonUnknown
{
friend class CMapUnknownUnknownDataFree;
CONSTRUCTABLE(CMapUnknownUnknown);
private:
	CMapPtrPtr	mcMap;

public:	
	void		Init(bool bKillElements = true, bool bOverwriteExisting = true);
	void		Kill(void);
	bool		Save(CFileWriter* pcFile);
	bool		Load(CFileReader* pcFile);

	template<class M, class N>
	N*			Put(M* pcKey);  //Note this allocates N and returns a pointer to it.  Not a pointer to the pointer in the map.
	bool		Put(CUnknown* pcKey, CUnknown* pcValue);

	template<class M, class N>
	N*			Get(M* pcKey);
	CUnknown*	Get(CUnknown* pcKey);
	size		NumElements(void);
	size		GetSortedSize(void);
	size		GetHoldingSize(void);

	bool		StartIteration(SMapIterator* psIterator, CUnknown** ppvKey, CUnknown** ppvData);
	bool		Iterate(SMapIterator* psIterator, CUnknown** ppvKey, CUnknown** ppvData);

	void		Pack(void);
	void		Print(CChars* psz);
	void		Dump(void);
	void		Sort(void);

protected:
	void		FreeData(void* pvData) override;
	bool		WriteData(CFileWriter* pcFile, void* pvData) override;
	bool		ReadData(CFileReader* pcFile, void* pvData) override;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
N* CMapUnknownUnknown::Put(M* pcKey)
{
	N*		pv;
	bool	bResult;

	if (pcKey)
	{
		pv = gcUnknowns.Add<N>();
		bResult = Put(pcKey, pv);
		if (bResult)
		{
			return pv;
		}
		else
		{
			//Nasty, but if we allocated (and didn't initialise) the object then it must be freed but not killed.
			gcUnknowns.RemoveInKill(pv);
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
N* CMapUnknownUnknown::Get(M* pcKey)
{
	return (N*)Get((CUnknown*)pcKey);
}


#endif // __MAP_UNKNOWN_UNKNOWN_H__

