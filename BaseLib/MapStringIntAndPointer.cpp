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
	ps->pvData = pvData;
	ps->uiType = uiType;
	return ps;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntAndPointer* CMapStringIntAndPointer::Put(const char* szKey, uint uiType, void* pvData)
{
	SIntAndPointer* ps;

	ps = __CMapStringIntAndPointer::Put(szKey);
	ps->pvData = pvData;
	ps->uiType = uiType;
	return ps;
}

