#include "MapStringIntAndPointer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntAndPointer* CMapStringIntAndPointer::Get(char* szKey)
{
	SIntAndPointer* ps;

	ps = __CMapStringIntAndPointer::Get(szKey);
	return ps;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntAndPointer* CMapStringIntAndPointer::Get(const char* szKey)
{
	SIntAndPointer* ps;

	ps = __CMapStringIntAndPointer::Get(szKey);
	return ps;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapStringIntAndPointer::Get(char* szKey, uint* puiType)
{
	SIntAndPointer* ps;

	ps = __CMapStringIntAndPointer::Get(szKey);
	if (ps)
	{
		SafeAssign(puiType, ps->uiType);
		return ps->pvData;
	}
	else
	{
		SafeAssign(puiType, PT_Undefined);
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapStringIntAndPointer::Get(const char* szKey, uint* puiType)
{
	SIntAndPointer* ps;

	ps = __CMapStringIntAndPointer::Get(szKey);
	if (ps)
	{
		SafeAssign(puiType, ps->uiType);
		return ps->pvData;
	}
	else
	{
		SafeAssign(puiType, PT_Undefined);
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntAndPointer* CMapStringIntAndPointer::Put(char* szKey, uint uiType, void* pvData)
{
	SIntAndPointer*		ps;

	ps = __CMapStringIntAndPointer::Put(szKey);
	if (ps)
	{
		ps->pvData = pvData;
		ps->uiType = uiType;
		return ps;
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
SIntAndPointer* CMapStringIntAndPointer::Put(const char* szKey, uint uiType, void* pvData)
{
	SIntAndPointer* ps;

	ps = __CMapStringIntAndPointer::Put(szKey);
	if (ps)
	{
		ps->pvData = pvData;
		ps->uiType = uiType;
		return ps;
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
bool CMapStringIntAndPointer::StartIteration(SMapIterator* psIterator, void** ppvKey, size* piKeySize, void** ppvData, uint* puiType)
{
	SIntAndPointer*		ps;
	bool				bValid;

	bValid = __CMapStringIntAndPointer::StartIteration(psIterator, ppvKey, piKeySize, (void**)&ps, NULL);
	if (bValid)
	{
		SafeAssign(ppvData, ps->pvData);
		SafeAssign(puiType, ps->uiType);
	}
	else
	{
		SafeAssign(ppvData, NULL);
		SafeAssign(puiType, PT_Undefined);
	}
	return bValid;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapStringIntAndPointer::Iterate(SMapIterator* psIterator, void** ppvKey, size* piKeySize, void** ppvData, uint* puiType)
{
	SIntAndPointer*		ps;
	bool				bValid;

	bValid = __CMapStringIntAndPointer::Iterate(psIterator, ppvKey, piKeySize, (void**)&ps, NULL);
	if (bValid)
	{
		SafeAssign(ppvData, ps->pvData);
		SafeAssign(puiType, ps->uiType);
	}
	else
	{
		SafeAssign(ppvData, NULL);
		SafeAssign(puiType, PT_Undefined);
	}
	return bValid;
}

