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
void CJavaModifiers::Set(EJavaModifier eModifier)
{
	muiModifiers |= eModifier;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaModifiers::IsPackageModifier(void)
{
	return !(FixBool(JM_public | JM_private | JM_protected) & muiModifiers);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaModifiers::IsValid(void)
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
		return FALSE;
	}

	if (IsAbstract())
	{
		if (IsStatic())
		{
			return FALSE;
		}
		if (IsFinal())
		{
			return FALSE;
		}
	}

	return TRUE;
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
BOOL CJavaModifiers::IsNone(void) { return muiModifiers == JM_none; }
BOOL CJavaModifiers::IsPublic(void) { return FixBool(muiModifiers & JM_public); }
BOOL CJavaModifiers::IsProtected(void) { return FixBool(muiModifiers & JM_protected); }
BOOL CJavaModifiers::IsPrivate(void) { return FixBool(muiModifiers & JM_private); }
BOOL CJavaModifiers::IsStatic(void) { return FixBool(muiModifiers & JM_static); }
BOOL CJavaModifiers::IsAbstract(void) { return FixBool(muiModifiers & JM_abstract); }
BOOL CJavaModifiers::IsFinal(void) { return FixBool(muiModifiers & JM_final); }
BOOL CJavaModifiers::IsStrictfp(void) { return FixBool(muiModifiers & JM_strictfp); }

