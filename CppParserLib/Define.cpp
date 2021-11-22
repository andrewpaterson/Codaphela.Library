#include "BaseLib/IntegerHelper.h"
#include "DefineMap.h"
#include "Define.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::Init(CExternalString* pcName, int64 uiID, CDefineMap* pcDefineMap)
{
	CPostMalloc<CPPLine>	cPostMalloc;

	cPostMalloc.PostMalloc(&mcReplacement);

	mcReplacement.Init(-1, -1);
	mcArguments.Init();
	miFlags = ((pcDefineMap == NULL) ? 0 : DEFINE_FLAGS_IN_MAP);
	mlliID = uiID;
	mpcDefineMap = pcDefineMap;
	mszName.Init(pcName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::Kill(void)
{
	mszName.Kill();
	mcArguments.Kill();
	mcReplacement.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::AddArgument(CExternalString* pcName)
{
	mcArguments.Add(pcName->msz, 0, pcName->miLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::AddVariadic(void)
{
	SetFlag(&miFlags, DEFINE_FLAGS_VARIADIC, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::AddReplacmentToken(CPPToken* pcToken)
{
	mcReplacement.mcTokens.Add(&pcToken);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDefine::IsBacketed(void)
{
	return miFlags & DEFINE_FLAGS_BRACKETED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDefine::IsSpecial(void)
{
	return miFlags & DEFINE_FLAGS_SPECIAL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDefine::IsInMap(void)
{
	return miFlags & DEFINE_FLAGS_IN_MAP;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::SetSpecial(BOOL b)
{
	SetFlag(&miFlags, DEFINE_FLAGS_SPECIAL, b);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::SetBracketed(BOOL b)
{
	SetFlag(&miFlags, DEFINE_FLAGS_BRACKETED, b);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::SetDefineMap(CDefineMap* pcDefineMap)
{
	if (pcDefineMap)
	{
		SetFlag(&miFlags, DEFINE_FLAGS_IN_MAP, TRUE);
	}
	else
	{
		SetFlag(&miFlags, DEFINE_FLAGS_IN_MAP, FALSE);
	}
	mpcDefineMap = pcDefineMap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDefine::Equals(CDefine* pcOther)
{
	if (pcOther)
	{
		if (IsBacketed() == pcOther->IsBacketed())
		{
			if (mcReplacement.Equals(&pcOther->mcReplacement))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CDefine::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CDefine::GetID(void)
{
	return mlliID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::Dump(void)
{
	CChars	sz;
	int		i;
	CChars* psz;

	sz.Init();

	if (IsInMap())
	{
		sz.Append(GetName());
	}
	else
	{
		sz.Append("*Not In Map*");
	}

	if (IsBacketed())
	{
		sz.Append('(');
		for (i = 0; i < mcArguments.NumElements(); i++)
		{
			psz = mcArguments.Get(i);
			sz.Append(psz->Text());

			if (i != mcArguments.NumElements() - 1)
			{
				sz.Append(", ");
			}
		}
		sz.Append(')');
	}

	if (mcReplacement.TokenLength() > 0)
	{
		sz.Append(' ');
		mcReplacement.Append(&sz);
	}
	else
	{
		sz.AppendNewLine();
	}

	sz.Dump();
	sz.Kill();
}

