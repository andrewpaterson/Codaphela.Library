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

	mcReplacement.Init(-1, -1, gszEmptyString);
	maszArguments.Init();
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
	maszArguments.Kill();
	mcReplacement.Kill();
	miFlags = DEFINE_FLAGS_KILLED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::AddArgument(CExternalString* pcName)
{
	maszArguments.Add(pcName->msz, 0, pcName->miLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::SetVariadic(void)
{
	SetFlag(&miFlags, DEFINE_FLAGS_VARIADIC, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::AddReplacmentToken(CPPToken* pcToken)
{
	mcReplacement.GetTokenList()->Add(pcToken);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDefine::IsBacketed(void)
{
	return FixBool(miFlags & DEFINE_FLAGS_BRACKETED);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDefine::IsVariadic(void)
{
	return FixBool(miFlags & DEFINE_FLAGS_VARIADIC);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDefine::IsSpecial(void)
{
	return FixBool(miFlags & DEFINE_FLAGS_SPECIAL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDefine::IsInMap(void)
{
	return FixBool(miFlags & DEFINE_FLAGS_IN_MAP);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::SetSpecial(bool b)
{
	SetFlag(&miFlags, DEFINE_FLAGS_SPECIAL, b);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefine::SetBracketed(bool b)
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
		SetFlag(&miFlags, DEFINE_FLAGS_IN_MAP, true);
	}
	else
	{
		SetFlag(&miFlags, DEFINE_FLAGS_IN_MAP, false);
	}
	mpcDefineMap = pcDefineMap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDefine::Equals(CDefine* pcOther)
{
	if (pcOther)
	{
		if (IsBacketed() == pcOther->IsBacketed())
		{
			if (mcReplacement.Equals(&pcOther->mcReplacement))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDefine::CanProcessArguments(int iNumArguments)
{
	if (!IsVariadic())
	{
		return iNumArguments == maszArguments.NumElements();
	}
	else
	{
		return iNumArguments >= (maszArguments.NumElements() - 1);
	}
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
		for (i = 0; i < maszArguments.NumElements(); i++)
		{
			psz = maszArguments.Get(i);
			sz.Append(psz->Text());

			if (i != maszArguments.NumElements() - 1)
			{
				sz.Append(", ");
			}
		}

		if (IsVariadic())
		{
			sz.Append("...");
		}
		sz.Append(')');
	}

	if (mcReplacement.TokenLength() > 0)
	{
		sz.Append(' ');
		mcReplacement.Print(&sz);
	}
	else
	{
		sz.AppendNewLine();
	}

	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CDefine::GetArguments(void)
{
	return &maszArguments;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPLine* CDefine::GetReplacement(void)
{
	return &mcReplacement;
}

