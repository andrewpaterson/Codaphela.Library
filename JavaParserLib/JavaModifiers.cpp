#include "BaseLib/IntegerHelper.h"
#include "JavaModifiers.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaModifiers::Init(void)
{
	muiModifiers = JM_none;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaModifiers::Kill(void)
{
	muiModifiers = JM_none;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaModifiers::ReInit(void)
{
	muiModifiers = JM_none;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaModifiers::Set(EJavaModifier eModifier)
{
	muiModifiers |= eModifier;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaModifiers::IsPackageModifier(void)
{
	return !(FixBool((JM_public | JM_private | JM_protected) & muiModifiers));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaModifiers::IsValid(void)
{
	int iCount;

	iCount = 0;
	if (IsPublic())
	{
		iCount++;
	}
	if (IsProtected())
	{
		iCount++;
	}
	if (IsPrivate())
	{
		iCount++;
	}
	if (iCount > 1)
	{
		return false;
	}

	if (IsAbstract())
	{
		if (IsStatic())
		{
			return false;
		}
		if (IsFinal())
		{
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaModifiers::Print(CChars* pszDest)
{
	if (IsPublic())
	{
		pszDest->Append("public ");
	}
	if (IsPrivate())
	{
		pszDest->Append("private ");
	}
	if (IsProtected())
	{
		pszDest->Append("protected ");
	}
	if (IsStatic())
	{
		pszDest->Append("static ");
	}
	if (IsFinal())
	{
		pszDest->Append("final ");
	}
	if (IsAbstract())
	{
		pszDest->Append("abstract ");
	}
	if (IsStrictfp())
	{
		pszDest->Append("strictfp ");
	}

	if (!IsNone())
	{
		pszDest->RemoveFromEnd(1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaModifiers::IsNone(void) { return muiModifiers == JM_none; }
bool CJavaModifiers::IsPublic(void) { return FixBool(muiModifiers & JM_public); }
bool CJavaModifiers::IsProtected(void) { return FixBool(muiModifiers & JM_protected); }
bool CJavaModifiers::IsPrivate(void) { return FixBool(muiModifiers & JM_private); }
bool CJavaModifiers::IsStatic(void) { return FixBool(muiModifiers & JM_static); }
bool CJavaModifiers::IsAbstract(void) { return FixBool(muiModifiers & JM_abstract); }
bool CJavaModifiers::IsFinal(void) { return FixBool(muiModifiers & JM_final); }
bool CJavaModifiers::IsStrictfp(void) { return FixBool(muiModifiers & JM_strictfp); }

