#ifndef __STRING_H__
#define __STRING_H__
#include "BaseLib/Chars.h"
#include "Object.h"
#include "Array.h"


template <class SpecificClass, typename ... Args>
Ptr<SpecificClass> OString(Args ... args);

class CObjectWriter;
class CObjectReader;
class CString : public CObject
{
CONSTRUCTABLE(CString);
DESTRUCTABLE(CString);
protected:
	CChars	msz;

public:
	Ptr<CString>	Init(void);
	Ptr<CString>	Init(char* sz);
	Ptr<CString>	Init(const char* sz);
	Ptr<CString>	Init(CChars* psz);
	Ptr<CString>	Init(CChars sz);
	Ptr<CString>	Init(Ptr<CString> pString);
	void			Class(void);
	void			Free(void);

	BOOL			Save(CObjectWriter* pcFile) override;
	BOOL			Load(CObjectReader* pcFile) override;

	void			D(void);

	Ptr<CString>	AddOperator(Ptr<CString> pString);
	Ptr<CString>	AddOperator(char* sz);

	int				Length(void) { return msz.Length(); }
	Ptr<CString>	Set(const char* sz) { msz.Set(sz); D(); return this; }
	Ptr<CString>	Set(Ptr<CString> pString);
	Ptr<CString>	Set(CChars sz) { msz.Set(sz); D(); return this; }
	Ptr<CString>	Set(CChars* psz) { msz.Set(psz); D(); return this; }
	Ptr<CString>	Append(const char* sz) { msz.Append(sz); D(); return this; }
	Ptr<CString>	Append(const char* sz, int iStrlen) { msz.Append(sz, iStrlen); D(); return this; }
	Ptr<CString>	Append(Ptr<CString> pString);
	Ptr<CString>	Append(Ptr<CString> pString, int iStrlen);
	Ptr<CString>	Append(CString* pcString);
	Ptr<CString>	Append(CChars sz) { msz.Append(sz); D(); return this; }
	Ptr<CString>	Append(CChars* psz) { msz.Append(psz); D(); return this; }
	Ptr<CString>	Append(char c) { msz.Append(c); D(); return this; }
	Ptr<CString>	Append(char c, int iNumber) { msz.Append(c, iNumber); D(); return this; }
	Ptr<CString>	Append(int i) { msz.Append(i); D(); return this; }
	Ptr<CString>	Append(int i, int iBase) { msz.Append(i, iBase); D(); return this; }
	Ptr<CString>	Append(unsigned int ui) { msz.Append(ui); D(); return this; }
	Ptr<CString>	Append(float f) { msz.Append(f); D(); return this; }
	Ptr<CString>	Append(float f, int iNumDecimals) { msz.Append(f, iNumDecimals); D(); return this; }
	Ptr<CString>	Append(double d) { msz.Append(d); D(); return this; }
	Ptr<CString>	Append(double d, int iNumDecimals) { msz.Append(d, iNumDecimals); D(); return this; }
	Ptr<CString>	Append(long long int lli) { msz.Append(lli); D(); return this; }
	Ptr<CString>	Append(unsigned long long int ulli) { msz.Append(ulli); D(); return this; }
	Ptr<CString>	AppendQuoted(char c) { msz.AppendQuoted(c); D(); return this; }
	Ptr<CString>	AppendQuoted(const char* sz) { msz.AppendQuoted(sz); D(); return this; }
	Ptr<CString>	AppendQuoted(Ptr<CString> pString);
	Ptr<CString>	AppendNewLine(void) { msz.AppendNewLine(); D(); return this; }
	Ptr<CString>	AppendNewLine(CChars sz) { msz.AppendNewLine(sz); D(); return this; }
	Ptr<CString>	AppendHexHiLo(void* pv, int iNumBytes) { msz.AppendHexHiLo(pv, iNumBytes); D(); return this; }
	Ptr<CString>	AppendHexLoHi(void* pv, int iNumBytes) { msz.AppendHexLoHi(pv, iNumBytes); D(); return this; }
	Ptr<CString>	AppendSubString(const char* sz, int iLength) { msz.AppendSubString(sz, iLength); D(); return this; }
	Ptr<CString>	AppendSubString(const char* sz, int iStartInclusive, int iEndExclusive) { msz.AppendSubString(sz, iStartInclusive, iEndExclusive); D(); return this; }
	Ptr<CString>	AppendSubString(const char* szStartInclusive, const char* szEndExclusive) { msz.AppendSubString(szStartInclusive, szEndExclusive); D(); return this; }
	Ptr<CString>	AppendSubString(Ptr<CString> pString, int iLength);
	Ptr<CString>	AppendSubString(Ptr<CString> pString, int iStartInclusive, int iEndExclusive);
	Ptr<CString>	AppendSubString(CChars sz, int iStartInclusive, int iEndExclusive) { msz.AppendSubString(sz, iStartInclusive, iEndExclusive); D(); return this; }
	Ptr<CString>	AppendBool(BOOL bValue) { msz.AppendBool(bValue); D(); return this; }
	Ptr<CString>	AppendBool(BOOL bValue, const char* szTrue, const char* szFalse) { msz.AppendBool(bValue, szTrue, szFalse); D(); return this; }
	Ptr<CString>	AppendBool(BOOL bValue, Ptr<CString> pTrue, Ptr<CString> pFalse);
	Ptr<CString>	AppendData(const char* szData, size_t iMaxLength) { msz.AppendData(szData, iMaxLength); D(); return this; }
	Ptr<CString>	AppendData(const char* szData, size_t iDataLength, size_t iMaxLength) { msz.AppendData(szData, iDataLength, iMaxLength); D(); return this; }
	Ptr<CString>	AppendData2(const char* szData, size_t iDataLength) { msz.AppendData2(szData, iDataLength); D(); return this; }
	Ptr<CString>	AppendPointer(void* pv) { msz.AppendPointer(pv); D(); return this; }
	BOOL			AppendFlag(unsigned int msFlags, unsigned int uiFlag, const char* szFlagName, BOOL bAppendComma = FALSE) { D(); return msz.AppendFlag(msFlags, uiFlag, szFlagName, bAppendComma); }
	BOOL			AppendFlag(unsigned int msFlags, unsigned int uiFlag, Ptr<CString> pFlagName, BOOL bAppendComma = FALSE);
	Ptr<CString>	LeftAlign(CChars sz, char cPadCharacter, int iWidth) { msz.LeftAlign(sz, cPadCharacter, iWidth); D(); return this; }
	Ptr<CString>	LeftAlign(const char* sz, char cPadCharacter, int iWidth) { msz.LeftAlign(sz, cPadCharacter, iWidth); D(); return this; }
	Ptr<CString>	LeftAlign(Ptr<CString> pString, char cPadCharacter, int iWidth);
	Ptr<CString>	RightAlign(CChars sz, char cPadCharacter, int iWidth) { msz.RightAlign(sz, cPadCharacter, iWidth); D(); return this; }
	Ptr<CString>	RightAlign(const char* sz, char cPadCharacter, int iWidth) { msz.RightAlign(sz, cPadCharacter, iWidth); D(); return this; }
	Ptr<CString>	RightAlign(Ptr<CString> pString, char cPadCharacter, int iWidth);
	Ptr<CString>	RightAlign(char cPadCharacter, int iWidth) { msz.RightAlign(cPadCharacter, iWidth); D(); return this; }
	Ptr<CString>	StripWhiteSpace(BOOL bIncludeNewLines = FALSE) { msz.StripWhiteSpace(bIncludeNewLines); D(); return this; }
	Ptr<CString>	Reverse(void) { msz.Reverse(); D(); return this; }
	Ptr<CString>	Clear(void) { msz.Clear(); D(); return this; }
	Ptr<CString>	LowerCase(void) { msz.LowerCase(); D(); return this; }
	Ptr<CString>	UpperCase(void) { msz.UpperCase(); D(); return this; }
	void			Insert(int iPos, char c) { msz.Insert(iPos, c); D(); }
	void			Insert(int iPos, const char* szString) { msz.Insert(iPos, szString); D(); }
	void			Insert(int iPos, Ptr<CString> pString);
	void			Insert(int iPos, CChars* pszString) { msz.Insert(iPos, pszString); D(); }
	void			Minimize(void) { msz.Minimize(); }
	char*			Text(void) { return msz.Text(); }
	char*			Text(int iIndex) { return msz.Text(iIndex); }
	CChars*			GetChars(void);
	BOOL			Empty(void) { return msz.Empty(); }
	void			RemoveLastCharacter(void) { msz.RemoveLastCharacter(); D(); }
	void			RemoveFromStart(int iNumChars) { msz.RemoveFromStart(iNumChars); D(); }
	void			RemoveFromEnd(int iNumChars) { msz.RemoveFromEnd(iNumChars); D(); }
	void			Remove(int iStart, int iEnd) { msz.Remove(iStart, iEnd); D(); }
	void			RemoveEnd(int iIndex) { msz.RemoveEnd(iIndex); D(); }
	void			RemoveCharacter(int iPos) { msz.RemoveCharacter(iPos); D(); }
	BOOL			Equals(const char* szString) { return msz.Equals(szString); }
	BOOL			Equals(Ptr<CString> pString);
	BOOL			Equals(CChars szString) { return msz.Equals(szString); }
	BOOL			Equals(const char* szString, int iLen) { return msz.Equals(szString, iLen); }
	BOOL			Equals(Ptr<CString> pString, int iLen);
	BOOL			Equals(CChars* pszString) { return msz.Equals(pszString); }
	BOOL			EqualsIgnoreCase(const char* szString) { return msz.EqualsIgnoreCase(szString); }
	BOOL			EqualsIgnoreCase(Ptr<CString> pString);
	BOOL			EqualsIgnoreCase(CChars szString) { return msz.EqualsIgnoreCase(szString); }
	BOOL			Contains(const char* szString) { return msz.Contains(szString); }
	BOOL			Contains(Ptr<CString> pString);
	BOOL			ContainsIgnoreCase(const char* szString) { return msz.ContainsIgnoreCase(szString); }
	BOOL			ContainsIgnoreCase(Ptr<CString> pString);
	BOOL			EndsWith(const char* szString) { return msz.EndsWith(szString); }
	BOOL			EndsWith(Ptr<CString> pString);
	BOOL			EndsWithIgnoreCase(const char* szString) { return msz.EndsWithIgnoreCase(szString); }
	BOOL			EndsWithIgnoreCase(Ptr<CString> pString);
	BOOL			StartsWith(const char* szString) { return msz.StartsWith(szString); }
	BOOL			StartsWith(Ptr<CString> pString);
	BOOL			StartsWithIgnoreCase(const char* szString) { return msz.StartsWithIgnoreCase(szString); }
	BOOL			StartsWithIgnoreCase(Ptr<CString> pString);
	int				Occurrences(const char* szString) { return msz.Occurrences(szString); }
	int				Occurrences(Ptr<CString> pString);
	BOOL			SubStringEquals(int iStart, const char* szString) { return msz.SubStringEquals(iStart, szString); }
	BOOL			SubStringEquals(int iStart, Ptr<CString> pString);
	BOOL			SubStringEqualsIgnoreCase(int iStart, const char* szString) { return msz.SubStringEqualsIgnoreCase(iStart, szString); }
	BOOL			SubStringEqualsIgnoreCase(int iStart, Ptr<CString> pString);
	int				FindFromEnd(const char* szString) { return msz.FindFromEnd(szString); }
	int				FindFromEnd(Ptr<CString> pString);
	int				FindFromEnd(int iPos, const char* szString) { return msz.FindFromEnd(iPos, szString); }
	int				FindFromEnd(int iPos, Ptr<CString> pString);
	int				FindFromEnd(char c) { return msz.FindFromEnd(c); }
	int				FindFromEnd(int iIndex, char c) { return msz.FindFromEnd(iIndex, c); }
	int				Find(Ptr<CString> pString);
	int				Find(const char* szString) { return msz.Find(szString); }
	int				Find(int iPos, const char* szString) { return msz.Find(iPos, szString); }
	int				Find(int iPos, Ptr<CString> pString);
	int				Find(int iPos, char c) { return msz.Find(iPos, c); }
	int				Find(char c) { return msz.Find(c); }
	int				FindDigit(int iStartIndex) { return msz.FindDigit(iStartIndex); }
	char			GetChar(int iIndex) { return msz.GetChar(iIndex); }
	void			SetChar(int iPos, char c) { msz.SetChar(iPos, c); D(); }
	void			Difference(CArrayInt* paiNewToOldIndices, CArrayInt* paiOldToNewIndices, CChars szOldString) { msz.Difference(paiNewToOldIndices, paiOldToNewIndices, szOldString); }
	BOOL			IsWhiteSpace(int iPos, BOOL bIncludeNewLines) { return msz.IsWhiteSpace(iPos, bIncludeNewLines); }
	BOOL			IsWhiteSpace(void) { return msz.IsWhiteSpace(); }
	void			SetLength(int iLength) { msz.SetLength(iLength); D(); }
	int				Compare(CChars* szOther) { return msz.Compare(szOther); }
	int				Compare(const char* szOther) { return msz.Compare(szOther); }
	int				Compare(Ptr<CString> pOther);
	int				CompareIgnoreCase(CChars* szOther) { return msz.CompareIgnoreCase(szOther); }
	int				CompareIgnoreCase(const char* szOther) { return msz.CompareIgnoreCase(szOther); }
	int				CompareIgnoreCase(Ptr<CString> pOther);
	int				Replace(char cFind, char cReplace) { D();  return msz.Replace(cFind, cReplace); }
	int				Replace(const char* szFind, const char* szReplace) { D();  return msz.Replace(szFind, szReplace); }
	int				Replace(Ptr<CString> pFind, Ptr<CString> pReplace);
	void			Overwrite(int iPos, const char* szReplace) { msz.Overwrite(iPos, szReplace); D(); }
	void			Overwrite(int iPos, Ptr<CString> pReplace);
	int				CountNewLines(void) { return msz.CountNewLines(); }
	int				Count(char c) { return msz.Count(c); }

	void					Split(CArrayChars* aszDest, char cSplitter) { msz.Split(aszDest, cSplitter); }
	Ptr<CArray<CString>>	Split(char cSplitter);
	void					SplitLines(CArrayChars* aszDest) { msz.SplitLines(aszDest); }
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <typename ... Args>
Ptr<CString> OString(Args ... args)
{
	Ptr<CString> pObject = gcObjects.Malloc<CString>();
	if (pObject.IsNotNull())
	{
		pObject->Init(args...);
	}
	return pObject;
}


#endif // !__STRING_H__

