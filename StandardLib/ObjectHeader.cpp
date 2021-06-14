#include "ObjectFileGeneral.h"
#include "ObjectHeader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectIdentifier::Init(void)
{
	mcType = 0;
	mszObjectName._Init();
	moi = INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectIdentifier::Init(OIndex oi)
{
	mcType = OBJECT_POINTER_ID;
	moi = oi;
	mszObjectName._Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectIdentifier::Init(char* szName)
{
	mcType = OBJECT_POINTER_NAMED;
	moi = INVALID_O_INDEX;
	mszObjectName.Init(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectIdentifier::Kill(void)
{
	mszObjectName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectIdentifier::IsNamed(void)
{
	if (mcType != OBJECT_POINTER_NAMED)
	{
		return FALSE;
	}
	
	if (mszObjectName.Empty())
	{
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectIdentifier::IsIndexed(void)
{
	if (mcType == OBJECT_POINTER_ID)
	{
		return TRUE;
	}

	if (mcType == OBJECT_POINTER_NAMED && mszObjectName.Empty())
	{
		return TRUE;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectIdentifier::GetName(void)
{
	if (mcType == OBJECT_POINTER_NAMED)
	{
		if (!mszObjectName.Empty())
		{
			return mszObjectName.Text();
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjectIdentifier::GetIndex()
{
	if (mcType == OBJECT_POINTER_ID || mcType == OBJECT_POINTER_NAMED)
	{
		return moi;
	}

	return INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectIdentifier::GetType(void)
{
	if (mcType == OBJECT_POINTER_ID)
	{
		return "IDX";
	}
	else if (mcType == OBJECT_POINTER_NAMED)
	{
		return "NAM";
	}
	else if (mcType == OBJECT_POINTER_NULL)
	{
		return "NUL";
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
void CObjectHeader::Init(void)
{
	mszClassName._Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectHeader::Kill(void)
{
	mszClassName.Kill();
	CObjectIdentifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectHeader::ClassName(void)
{
	return mszClassName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectHeader::GetIdentifier(CChars* psz)
{
	char* szName;

	psz->Append(ClassName());
	psz->Append(":");
	psz->Append(GetIndex());
	szName = GetName();
	if (!StrEmpty(szName))
	{
		psz->Append(":\"");
		psz->Append(szName);
		psz->Append("\"");
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerHeader::Init(void)
{
	//This should not call CObjectIdentifier::Init.
	miEmbeddedIndex = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerHeader::Kill(void)
{
	CObjectIdentifier::Kill();
}

