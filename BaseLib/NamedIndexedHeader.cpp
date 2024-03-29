#include "FastMemcpy.h"
#include "StringHelper.h"
#include "PointerRemapper.h"
#include "NamedIndexedHeader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedHeader::Init(char* szName, int iNameLength, void* pvData, unsigned int uiDataSize)
{
	char* szDestName;
	void* szDestData;

	if (iNameLength > 0)
	{
		miNameLength = iNameLength;

		szDestName = GetName();
		memcpy(szDestName, szName, iNameLength);
		szDestName[iNameLength] = '\0';

		szDestData = GetData();
		memcpy(szDestData, pvData, uiDataSize);
	}
	else
	{
		Init(pvData, uiDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedHeader::Init(void* pvData, unsigned int uiDataSize)
{
	void* szDestData;

	miNameLength = 0;

	szDestData = GetData();
	memcpy(szDestData, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CNamedIndexedHeader::GetName(void)
{
	if (miNameLength > 0)
	{
		return (char*)RemapSinglePointer(this, sizeof(CNamedIndexedHeader));
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
int CNamedIndexedHeader::GetNameLength(void)
{
	return miNameLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CNamedIndexedHeader::GetData(void)
{
	return RemapSinglePointer(this, GetHeaderSize());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNamedIndexedHeader::GetHeaderSize(void)
{
	if (miNameLength == 0)
	{
		return sizeof(CNamedIndexedHeader);
	}
	else
	{
		return sizeof(CNamedIndexedHeader) + miNameLength + sizeof(char);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedHeader::HasName(void)
{
	return (miNameLength > 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t NamedIndexedHeaderSize(char* szName, unsigned int uiDataSize)
{
	return NamedIndexedHeaderSize(strlen(szName), uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t NamedIndexedHeaderSize(int iNameLength, unsigned int uiDataSize)
{
	if (iNameLength > 0)
	{
		return sizeof(CNamedIndexedHeader) + iNameLength + sizeof(char) + uiDataSize;
	}
	else
	{
		return NamedIndexedHeaderSize(uiDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t NamedIndexedHeaderSize(unsigned int uiDataSize)
{
	return sizeof(CNamedIndexedHeader) + uiDataSize;
}

