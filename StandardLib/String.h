#ifndef __STRING_H__
#define __STRING_H__
#include "BaseLib/Chars.h"
#include "Object.h"


class CObjectSerialiser;
class CObjectDeserialiser;
class CString : public CObject
{
CONSTRUCTABLE(CString);
protected:
	CChars	msz;

public:
	void			Init(void);
	void			Init(char* sz);
	void			Init(const char* sz);
	void			Class(void);
	void			Free(void);

	BOOL			Save(CObjectSerialiser* pcFile);
	BOOL			Load(CObjectDeserialiser* pcFile);

	void			D(void);

	int				Length(void) { return msz.Length(); }
	void			Set(const char* sz) { msz.Set(sz); D(); }
	void			Set(CChars sz) { msz.Set(sz); }
	void			Set(CChars* psz) { msz.Set(psz); D(); }
	Ptr<CString>	Append(const char* sz) { msz.Append(sz); D(); return this; }
	Ptr<CString>	Append(const char* sz, int iStrlen) { msz.Append(sz, iStrlen); D(); return this; }
	void			Append(CChars sz) { msz.Append(sz); D(); }
	void			Append(CChars* psz) { msz.Append(psz); D(); }
	void			Append(char c) { msz.Append(c); D(); }
	void			Append(char c, int iNumber) { msz.Append(c, iNumber); D(); }
	Ptr<CString>	Append(int i) { msz.Append(i); D(); return this; }
	Ptr<CString>	Append(int i, int iBase) { msz.Append(i, iBase); D(); return this; }
	void			Append(unsigned int ui) { msz.Append(ui); D(); }
	void			Append(float f) { msz.Append(f); D(); }
	void			Append(float f, int iNumDecimals) { msz.Append(f, iNumDecimals); D(); }
	void			Append(double d) { msz.Append(d); D(); }
	void			Append(double d, int iNumDecimals) { msz.Append(d, iNumDecimals); D(); }
	void			Append(long long int lli) { msz.Append(lli); D(); }
	void			Append(unsigned long long int ulli) { msz.Append(ulli); D(); }
	void			AppendQuoted(char c) { msz.AppendQuoted(c); D(); }
	void			AppendQuoted(const char* sz) { msz.AppendQuoted(sz); D(); }
	Ptr<CString>	AppendNewLine(void) { msz.AppendNewLine(); D(); return this; }
	Ptr<CString>	AppendNewLine(CChars sz) { msz.AppendNewLine(sz); D(); return this; }
	void			AppendHexHiLo(void* pv, int iNumBytes) { msz.AppendHexHiLo(pv, iNumBytes); D(); }
	void			AppendHexLoHi(void* pv, int iNumBytes) { msz.AppendHexLoHi(pv, iNumBytes); D(); }
	void			AppendSubString(const char* sz, int iLength) { msz.AppendSubString(sz, iLength); D(); }
	void			AppendSubString(CChars sz, int iStartInclusive, int iEndExclusive) { msz.AppendSubString(sz, iStartInclusive, iEndExclusive); D(); }
	void			AppendSubString(const char* sz, int iStartInclusive, int iEndExclusive) { msz.AppendSubString(sz, iStartInclusive, iEndExclusive); D(); }
	void			AppendSubString(const char* szStartInclusive, const char* szEndExclusive) { msz.AppendSubString(szStartInclusive, szEndExclusive); D(); }
	void			AppendBool(BOOL bValue) { msz.AppendBool(bValue); D(); }
	void			AppendBool(BOOL bValue, const char* szTrue, const char* szFalse) { msz.AppendBool(bValue, szTrue, szFalse); D(); }
	Ptr<CString>	AppendData(const char* szData, size_t iMaxLength) { msz.AppendData(szData, iMaxLength); D(); return this; }
	Ptr<CString>	AppendData(const char* szData, size_t iDataLength, size_t iMaxLength) { msz.AppendData(szData, iDataLength, iMaxLength); D(); return this; }
	BOOL			AppendData2(const char* szData, size_t iDataLength) { msz.AppendData2(szData, iDataLength); D(); }
	void			AppendPointer(void* pv) { msz.AppendPointer(pv); D(); }
	BOOL			AppendFlag(unsigned int msFlags, unsigned int uiFlag, char* szFlagName, BOOL bAppendComma = FALSE) { msz.AppendFlag(msFlags, uiFlag, szFlagName, bAppendComma); D(); }
	void			Insert(int iPos, char c) { msz.Insert(iPos, c); D(); }
	void			Insert(int iPos, const char* szString) { msz.Insert(iPos, szString); D(); }
	void			Insert(int iPos, CChars* pszString) { msz.Insert(iPos, pszString); D(); }
	void			Minimize(void) { msz.Minimize(); }
	char*			Text(void) { return msz.Text(); }
	char*			Text(int iIndex) { return msz.Text(iIndex); }
	BOOL			Empty(void) { return msz.Empty(); }
	void			LeftAlign(CChars sz, char cPadCharacter, int iWidth) { msz.LeftAlign(sz, cPadCharacter, iWidth); D(); }
	void			LeftAlign(const char* sz, char cPadCharacter, int iWidth) { msz.LeftAlign(sz, cPadCharacter, iWidth); D(); }
	void			RightAlign(CChars sz, char cPadCharacter, int iWidth) { msz.RightAlign(sz, cPadCharacter, iWidth); D(); }
	void			RightAlign(const char* sz, char cPadCharacter, int iWidth) { msz.RightAlign(sz, cPadCharacter, iWidth); D(); }
	void			RightAlign(char cPadCharacter, int iWidth) { msz.RightAlign(cPadCharacter, iWidth); D(); }
	void			RemoveLastCharacter(void) { msz.RemoveLastCharacter(); D();}
	void			RemoveFromStart(int iNumChars) { msz.RemoveFromStart(iNumChars); D(); }
	void			RemoveFromEnd(int iNumChars) { msz.RemoveFromEnd(iNumChars); D(); }
	void			Remove(int iStart, int iEnd) { msz.Remove(iStart, iEnd); D(); }
	void			RemoveEnd(int iIndex) { msz.RemoveEnd(iIndex); D(); }
	void			RemoveCharacter(int iPos) { msz.RemoveCharacter(iPos); D(); }
	BOOL			Equals(const char* szString) { return msz.Equals(szString); }
	BOOL			Equals(CChars szString) { return msz.Equals(szString); }
	BOOL			Equals(const char* szString, int iLen) { return msz.Equals(szString, iLen); }
	BOOL			Equals(CChars* pszString) { return msz.Equals(pszString); }
	BOOL			EqualsIgnoreCase(const char* szString) { return msz.EqualsIgnoreCase(szString); }
	BOOL			EqualsIgnoreCase(CChars szString) { return msz.EqualsIgnoreCase(szString); }
	BOOL			Contains(const char* szString) { return msz.Contains(szString); }
	BOOL			ContainsIgnoreCase(const char* szString) { return msz.CompareIgnoreCase(szString); }
	BOOL			EndsWith(const char* szString) { return msz.EndsWith(szString); }
	BOOL			EndsWithIgnoreCase(const char* szString) { return msz.EndsWithIgnoreCase(szString); }
	BOOL			StartsWith(const char* szString) { return msz.StartsWith(szString); }
	BOOL			StartsWithIgnoreCase(const char* szString) { return msz.StartsWithIgnoreCase(szString); }
	int				Occurrences(const char* szString) { return msz.Occurrences(szString); }
	BOOL			SubStringEquals(int iStart, const char* szString) { return msz.SubStringEquals(iStart, szString); }
	BOOL			SubStringEqualsIgnoreCase(int iStart, const char* szString) { return msz.SubStringEqualsIgnoreCase(iStart, szString); }
	int				FindFromEnd(const char* szString) { return msz.FindFromEnd(szString); }
	int				FindFromEnd(int iPos, const char* szString) { return msz.FindFromEnd(iPos, szString); }
	int				FindFromEnd(char c) { return msz.FindFromEnd(c); }
	int				FindFromEnd(int iIndex, char c) { return msz.FindFromEnd(iIndex, c); }
	int				Find(int iPos, const char* szString) { return msz.Find(iPos, szString); }
	int				Find(int iPos, char c) { return msz.Find(iPos, c); }
	int				Find(char c) { return msz.Find(c); }
	int				FindDigit(int iStartIndex) { return msz.FindDigit(iStartIndex); }
	char			GetChar(int iIndex) { return msz.GetChar(iIndex); }
	void			SetChar(int iPos, char c) { msz.SetChar(iPos, c); D(); }
	void			Difference(CArrayInt* paiNewToOldIndices, CArrayInt* paiOldToNewIndices, CChars szOldString) { msz.Difference(paiNewToOldIndices, paiOldToNewIndices, szOldString); }
	BOOL			IsWhiteSpace(int iPos, BOOL bIncludeNewLines) { return msz.IsWhiteSpace(iPos, bIncludeNewLines); }
	BOOL			IsWhiteSpace(void) { return msz.IsWhiteSpace(); }
	void			SetLength(int iLength) { msz.SetLength(iLength); D(); }
	int				StripWhiteSpace(BOOL bIncludeNewLines = FALSE) { msz.StripWhiteSpace(bIncludeNewLines); D(); }
	int				Compare(CChars* szOther) { return msz.Compare(szOther); }
	int				Compare(const char* szOther) { return msz.Compare(szOther); }
	int				CompareIgnoreCase(CChars* szOther) { return msz.CompareIgnoreCase(szOther); }
	int				CompareIgnoreCase(const char* szOther) { return msz.CompareIgnoreCase(szOther); }
	int				Replace(char cFind, char cReplace) { return msz.Replace(cFind, cReplace); D(); }
	int				Replace(const char* szFind, const char* szReplace) { return msz.Replace(szFind, szReplace); D(); }
	void			Overwrite(int iPos, const char* szReplace) { msz.Overwrite(iPos, szReplace); D(); }
	void			Reverse(void) { msz.Reverse(); D(); }
	void			Clear(void) { msz.Clear(); D(); }
	int				CountNewLines(void) { return msz.CountNewLines(); }
	int				Count(char c) { return msz.Count(c); }
	void			LowerCase(void) { msz.LowerCase(); D(); }
	void			UpperCase(void) { msz.UpperCase(); D(); }

	void			Split(CArrayChars* aszDest, char cSplitter) { msz.Split(aszDest, cSplitter); }
	void			SplitLines(CArrayChars* aszDest) { msz.SplitLines(aszDest); }
};


#endif // __STRING_H__

