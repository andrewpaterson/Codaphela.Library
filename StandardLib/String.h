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

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			D(void);

	Ptr<CString>	AddOperator(Ptr<CString> pString);
	Ptr<CString>	AddOperator(char* sz);

	size			Length(void) { return msz.Length(); }
	Ptr<CString>	Set(const char* sz) { msz.Set(sz); D(); return this; }
	Ptr<CString>	Set(Ptr<CString> pString);
	Ptr<CString>	Set(CChars sz) { msz.Set(sz); D(); return this; }
	Ptr<CString>	Set(CChars* psz) { msz.Set(psz); D(); return this; }
	Ptr<CString>	Append(const char* sz) { msz.Append(sz); D(); return this; }
	Ptr<CString>	Append(const char* sz, size iStrlen) { msz.Append(sz, iStrlen); D(); return this; }
	Ptr<CString>	Append(Ptr<CString> pString);
	Ptr<CString>	Append(Ptr<CString> pString, size iStrlen);
	Ptr<CString>	Append(CString* pcString);
	Ptr<CString>	Append(CChars sz) { msz.Append(sz); D(); return this; }
	Ptr<CString>	Append(CChars* psz) { msz.Append(psz); D(); return this; }
	Ptr<CString>	Append(char c) { msz.Append(c); D(); return this; }
	Ptr<CString>	Append(char c, size iNumber) { msz.Append(c, iNumber); D(); return this; }
	Ptr<CString>	Append(int32 i) { msz.Append(i); D(); return this; }
	Ptr<CString>	Append(int32 i, uint16 iBase) { msz.Append(i, iBase); D(); return this; }
	Ptr<CString>	Append(uint32 ui) { msz.Append(ui); D(); return this; }
	Ptr<CString>	Append(float f) { msz.Append(f); D(); return this; }
	Ptr<CString>	Append(float f, int iNumDecimals) { msz.Append(f, iNumDecimals); D(); return this; }
	Ptr<CString>	Append(double d) { msz.Append(d); D(); return this; }
	Ptr<CString>	Append(double d, int iNumDecimals) { msz.Append(d, iNumDecimals); D(); return this; }
	Ptr<CString>	Append(int64 lli) { msz.Append(lli); D(); return this; }
	Ptr<CString>	Append(uint64 ulli) { msz.Append(ulli); D(); return this; }
	Ptr<CString>	AppendQuoted(char c) { msz.AppendQuoted(c); D(); return this; }
	Ptr<CString>	AppendQuoted(const char* sz) { msz.AppendQuoted(sz); D(); return this; }
	Ptr<CString>	AppendQuoted(Ptr<CString> pString);
	Ptr<CString>	AppendNewLine(void) { msz.AppendNewLine(); D(); return this; }
	Ptr<CString>	AppendNewLine(CChars sz) { msz.AppendNewLine(sz); D(); return this; }
	Ptr<CString>	AppendHexHiLo(void* pv, size iNumBytes) { msz.AppendHexHiLo(pv, iNumBytes); D(); return this; }
	Ptr<CString>	AppendHexLoHi(void* pv, size iNumBytes) { msz.AppendHexLoHi(pv, iNumBytes); D(); return this; }
	Ptr<CString>	AppendSubString(const char* sz, size iLength) { msz.AppendSubString(sz, iLength); D(); return this; }
	Ptr<CString>	AppendSubString(const char* sz, size iStartInclusive, size iEndExclusive) { msz.AppendSubString(sz, iStartInclusive, iEndExclusive); D(); return this; }
	Ptr<CString>	AppendSubString(const char* szStartInclusive, const char* szEndExclusive) { msz.AppendSubString(szStartInclusive, szEndExclusive); D(); return this; }
	Ptr<CString>	AppendSubString(Ptr<CString> pString, size iLength);
	Ptr<CString>	AppendSubString(Ptr<CString> pString, size iStartInclusive, size iEndExclusive);
	Ptr<CString>	AppendSubString(CChars sz, size iStartInclusive, size iEndExclusive) { msz.AppendSubString(sz, iStartInclusive, iEndExclusive); D(); return this; }
	Ptr<CString>	AppendBool(bool bValue) { msz.AppendBool(bValue); D(); return this; }
	Ptr<CString>	AppendBool(bool bValue, const char* szTrue, const char* szFalse) { msz.AppendBool(bValue, szTrue, szFalse); D(); return this; }
	Ptr<CString>	AppendBool(bool bValue, Ptr<CString> pTrue, Ptr<CString> pFalse);
	Ptr<CString>	AppendData(const char* szData, size iMaxLength) { msz.AppendData(szData, iMaxLength); D(); return this; }
	Ptr<CString>	AppendData(const char* szData, size iDataLength, size iMaxLength) { msz.AppendData(szData, iDataLength, iMaxLength); D(); return this; }
	Ptr<CString>	AppendData2(const char* szData, size iDataLength) { msz.AppendData2(szData, iDataLength); D(); return this; }
	Ptr<CString>	AppendPointer(void* pv) { msz.AppendPointer(pv); D(); return this; }
	bool			AppendFlag(uint32 msFlags, uint32 uiFlag, const char* szFlagName, bool bAppendComma = false) { D(); return msz.AppendFlag(msFlags, uiFlag, szFlagName, bAppendComma); }
	bool			AppendFlag(uint32 msFlags, uint32 uiFlag, Ptr<CString> pFlagName, bool bAppendComma = false);
	Ptr<CString>	LeftAlign(CChars sz, char cPadCharacter, size iWidth) { msz.LeftAlign(sz, cPadCharacter, iWidth); D(); return this; }
	Ptr<CString>	LeftAlign(const char* sz, char cPadCharacter, size iWidth) { msz.LeftAlign(sz, cPadCharacter, iWidth); D(); return this; }
	Ptr<CString>	LeftAlign(Ptr<CString> pString, char cPadCharacter, size iWidth);
	Ptr<CString>	RightAlign(CChars sz, char cPadCharacter, size iWidth) { msz.RightAlign(sz, cPadCharacter, iWidth); D(); return this; }
	Ptr<CString>	RightAlign(const char* sz, char cPadCharacter, size iWidth) { msz.RightAlign(sz, cPadCharacter, iWidth); D(); return this; }
	Ptr<CString>	RightAlign(Ptr<CString> pString, char cPadCharacter, size iWidth);
	Ptr<CString>	RightAlign(char cPadCharacter, size iWidth) { msz.RightAlign(cPadCharacter, iWidth); D(); return this; }
	Ptr<CString>	StripWhitespace(bool bIncludeNewLines = false) { msz.StripWhitespace(bIncludeNewLines); D(); return this; }
	Ptr<CString>	Reverse(void) { msz.Reverse(); D(); return this; }
	Ptr<CString>	Clear(void) { msz.Clear(); D(); return this; }
	Ptr<CString>	LowerCase(void) { msz.LowerCase(); D(); return this; }
	Ptr<CString>	UpperCase(void) { msz.UpperCase(); D(); return this; }
	void			Insert(size iPos, char c) { msz.Insert(iPos, c); D(); }
	void			Insert(size iPos, const char* szString) { msz.Insert(iPos, szString); D(); }
	void			Insert(size iPos, Ptr<CString> pString);
	void			Insert(size iPos, CChars* pszString) { msz.Insert(iPos, pszString); D(); }
	void			Minimize(void) { msz.Minimize(); }
	char*			Text(void) { return msz.Text(); }
	char*			Text(size iIndex) { return msz.Text(iIndex); }
	CChars*			GetChars(void);
	bool			Empty(void) { return msz.Empty(); }
	void			RemoveLastCharacter(void) { msz.RemoveLastCharacter(); D(); }
	void			RemoveFromStart(size iNumChars) { msz.RemoveFromStart(iNumChars); D(); }
	void			RemoveFromEnd(size iNumChars) { msz.RemoveFromEnd(iNumChars); D(); }
	void			Remove(size iStart, size iEnd) { msz.Remove(iStart, iEnd); D(); }
	void			RemoveEnd(size iIndex) { msz.RemoveEnd(iIndex); D(); }
	void			RemoveCharacter(size iPos) { msz.RemoveCharacter(iPos); D(); }
	bool			Equals(const char* szString) { return msz.Equals(szString); }
	bool			Equals(Ptr<CString> pString);
	bool			Equals(CChars szString) { return msz.Equals(szString); }
	bool			Equals(const char* szString, size iLen) { return msz.Equals(szString, iLen); }
	bool			Equals(Ptr<CString> pString, size iLen);
	bool			Equals(CChars* pszString) { return msz.Equals(pszString); }
	bool			EqualsIgnoreCase(const char* szString) { return msz.EqualsIgnoreCase(szString); }
	bool			EqualsIgnoreCase(Ptr<CString> pString);
	bool			EqualsIgnoreCase(CChars szString) { return msz.EqualsIgnoreCase(szString); }
	bool			Contains(const char* szString) { return msz.Contains(szString); }
	bool			Contains(Ptr<CString> pString);
	bool			ContainsIgnoreCase(const char* szString) { return msz.ContainsIgnoreCase(szString); }
	bool			ContainsIgnoreCase(Ptr<CString> pString);
	bool			EndsWith(const char* szString) { return msz.EndsWith(szString); }
	bool			EndsWith(Ptr<CString> pString);
	bool			EndsWithIgnoreCase(const char* szString) { return msz.EndsWithIgnoreCase(szString); }
	bool			EndsWithIgnoreCase(Ptr<CString> pString);
	bool			StartsWith(const char* szString) { return msz.StartsWith(szString); }
	bool			StartsWith(Ptr<CString> pString);
	bool			StartsWithIgnoreCase(const char* szString) { return msz.StartsWithIgnoreCase(szString); }
	bool			StartsWithIgnoreCase(Ptr<CString> pString);
	size			Occurrences(const char* szString) { return msz.Occurrences(szString); }
	size			Occurrences(Ptr<CString> pString);
	bool			SubStringEquals(size iStart, const char* szString) { return msz.SubStringEquals(iStart, szString); }
	bool			SubStringEquals(size iStart, Ptr<CString> pString);
	bool			SubStringEqualsIgnoreCase(size iStart, const char* szString) { return msz.SubStringEqualsIgnoreCase(iStart, szString); }
	bool			SubStringEqualsIgnoreCase(size iStart, Ptr<CString> pString);
	size			FindFromEnd(const char* szString) { return msz.FindFromEnd(szString); }
	size			FindFromEnd(Ptr<CString> pString);
	size			FindFromEnd(size iPos, const char* szString) { return msz.FindFromEnd(iPos, szString); }
	size			FindFromEnd(size iPos, Ptr<CString> pString);
	size			FindFromEnd(char c) { return msz.FindFromEnd(c); }
	size			FindFromEnd(size iIndex, char c) { return msz.FindFromEnd(iIndex, c); }
	size			Find(Ptr<CString> pString);
	size			Find(const char* szString) { return msz.Find(szString); }
	size			Find(size iPos, const char* szString) { return msz.Find(iPos, szString); }
	size			Find(size iPos, Ptr<CString> pString);
	size			Find(size iPos, char c) { return msz.Find(iPos, c); }
	size			Find(char c) { return msz.Find(c); }
	size			FindDigit(size iStartIndex) { return msz.FindDigit(iStartIndex); }
	char			GetChar(size iIndex) { return msz.GetChar(iIndex); }
	void			SetChar(size iPos, char c) { msz.SetChar(iPos, c); D(); }
	void			Difference(CArrayInt* paiNewToOldIndices, CArrayInt* paiOldToNewIndices, CChars szOldString) { msz.Difference(paiNewToOldIndices, paiOldToNewIndices, szOldString); }
	bool			IsWhitespace(size iPos, bool bIncludeNewLines) { return msz.IsWhitespace(iPos, bIncludeNewLines); }
	bool			IsWhitespace(void) { return msz.IsWhitespace(); }
	void			SetLength(size iLength) { msz.SetLength(iLength); D(); }
	int				Compare(CChars* szOther) { return msz.Compare(szOther); }
	int				Compare(const char* szOther) { return msz.Compare(szOther); }
	int				Compare(Ptr<CString> pOther) { return Compare(pOther->Text()); }
	int				CompareIgnoreCase(CChars* szOther) { return msz.CompareIgnoreCase(szOther); }
	int				CompareIgnoreCase(const char* szOther) { return msz.CompareIgnoreCase(szOther); }
	int				CompareIgnoreCase(Ptr<CString> pOther) { return CompareIgnoreCase(pOther->Text()); }
	size			Replace(char cFind, char cReplace) { D();  return msz.Replace(cFind, cReplace); }
	size			Replace(const char* szFind, const char* szReplace) { D();  return msz.Replace(szFind, szReplace); }
	size			Replace(Ptr<CString> pFind, Ptr<CString> pReplace) { return Replace(pFind->Text(), pReplace->Text()); }
	void			Overwrite(size iPos, const char* szReplace) { msz.Overwrite(iPos, szReplace); D(); }
	void			Overwrite(size iPos, Ptr<CString> pReplace) { return Overwrite(iPos, pReplace->Text()); }
	size			CountNewLines(void) { return msz.CountNewLines(); }
	size			Count(char c) { return msz.Count(c); }

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


#endif // __STRING_H__

