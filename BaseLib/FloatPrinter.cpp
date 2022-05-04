#include "StringHelper.h"
#include "IntegerHelper.h"
#include "Number.h"
#include "NumberControl.h"
#include "FloatPrinter.h"


int8	giPow2DigitsToPow10Digits[64] = { 0,1,1,1,2,2,2,3,3,3,4,4,4,4,5,5,5,6,6,6,7,7,7,7,8,8,8,9,9,9,10,10,10,10,11,11,11,12,12,12,13,13,13,13,14,14,14,15,15,15,16,16,16,16,17,17,17,18,18,18,19,19,19,19 };
char	szCommonLogarithmOfTwo[] = "0.3010299956639811952137388947244930267653"; 		//X.Init(2, 2, 40) X.CommonLogarithm();


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int GetPow2DigitsToPow10Digits(int iPow2)
{
	if (iPow2 < 0)
	{
		return -iPow2;  //The number of fractional digits in base 2 is the SAME as the number of fractional digits in base 10.
	}
	else if (iPow2 < 64)
	{
		return giPow2DigitsToPow10Digits[iPow2];
	}
	else
	{
		UNumber(5, 40)	cPow2;
		UNumber(5, 40)	cLog2;
		CNumber			cNum;

		cLog2.c.Init(szCommonLogarithmOfTwo, 5, 40);
		cPow2.c.Init(&cLog2.c);
		cNum.Init(iPow2);
		cPow2.c.Multiply(&cNum);
		cPow2.c.Ceiling();
		return cPow2.c.IntValue();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class FLOAT, class INTEGER>
char* NumberToString(char* szDest, int iDestLength, FLOAT f, int iMaxDecimals, BOOL bAppendType, char* szType, INTEGER iNegativeBit, INTEGER iMantissaMask, INTEGER iFirstMantisaBit, INTEGER iExponentMask, int iExponentShift, int iReservedExponent, int iExponentBias, int iMaxSignificantDigits)
{	
	unsigned char*	pui;
	INTEGER			iMantissa;
	BOOL			bNegative;
	int				iExponent;
	INTEGER			iValue;
	CNumber*		pcResult;
	CNumber*		pcTwoPower;
	int				iWholeDigits;
	int				iFractionalDigits;
	CNumber			cExponent;
	CNumber			cTwo;
	int				iLeftMost;
	CChars			sz;
	int				iFractionalPart;
	BOOL			bNumeric;
	int				iSignificantDigits;
	int				iIndex;
	int				iLength;

	pui = (unsigned char*)&f;
	iExponent = (int)((*((INTEGER*)pui) & iExponentMask) >> iExponentShift);
	iMantissa = *((INTEGER*)pui) & iMantissaMask;
	bNegative = FixBool((*((INTEGER*)pui) & iNegativeBit) == iNegativeBit);

	if (iExponent == 0)
	{
		return NULL;
	}
	else if (iExponent == iReservedExponent)
	{
		CChars	sz;

		sz.Init();
		if (iMantissa == 0)
		{
			if (bNegative)
			{
				sz.Append("-");
			}
			sz.Append(NUMBER_INFINITE_STRING);
			sz.CopyIntoBuffer(szDest, iDestLength);
		}
		else
		{
			sz.Append(NUMBER_NOT_A_NUMBER);
		}
		sz.Kill();

		return szDest;
	}
	else
	{
		iExponent = iExponent - iExponentBias;  //Convert by exponent bias.
		iValue = (iMantissa | iFirstMantisaBit);  //Add implied [1.fraction].

		cExponent.Init(iExponent);
		cTwo.Init(2, 1, 0);

		if (iExponent >= 0)
		{
			iWholeDigits = GetPow2DigitsToPow10Digits(iExponent);
		}
		else
		{
			iWholeDigits = GetPow2DigitsToPow10Digits(-iExponent);
		}
		iWholeDigits += 2;
		iFractionalPart = iExponent - 24;
		if (iFractionalPart >= 0)
		{
			iFractionalDigits = 0;
		}
		else
		{
			iFractionalDigits = GetPow2DigitsToPow10Digits(iFractionalPart);
			iFractionalDigits++;
		}

		pcResult = gcNumberControl.Add(iWholeDigits, iFractionalDigits);
		pcTwoPower = gcNumberControl.Add(iWholeDigits, iFractionalDigits);

		pcTwoPower->Init(2, iWholeDigits, iFractionalDigits);
		pcTwoPower->Power(&cExponent);

		pcResult->Zero(iWholeDigits, iFractionalDigits);

		INTEGER iDigit = iFirstMantisaBit;
		for (int i = 0; i <= iExponentShift; i++)
		{
			if (iValue & iDigit)
			{
				pcResult->Add(pcTwoPower);
			}
			pcTwoPower->Divide(&cTwo);
			iDigit >>= 1;
		}

		iLeftMost = pcResult->GetFirstNonZeroDigit();
		iSignificantDigits = iMaxSignificantDigits;
		if (iMaxDecimals > iMaxSignificantDigits)
		{
			iMaxDecimals = iMaxSignificantDigits;
		}

		if (iMaxDecimals >= 0)
		{
			if (iLeftMost < iMaxSignificantDigits)
			{
				if (iLeftMost + iMaxDecimals <= iSignificantDigits)
				{
					iSignificantDigits = iLeftMost + iMaxDecimals;
				}

				if ((iLeftMost < 0) && (iSignificantDigits < iMaxSignificantDigits))
				{
					iSignificantDigits++;
				}
			}
		}

		if (iSignificantDigits > 0)
		{
			pcResult->RoundSignificant(iSignificantDigits);
		}
		else
		{
			pcResult->Zero(iWholeDigits, iFractionalDigits);
		}

		if ((!pcResult->IsError() || pcResult->IsOverflow()) && !pcResult->IsZero())
		{
			if (bNegative)
			{
				pcResult->SetSign(-1);
			}
		}

		sz.Init();
		bNumeric = pcResult->PrintFloating(&sz);
		if (bNumeric && bAppendType)
		{
			sz.Append('f');
		}
		else
		{
			if (!sz.Contains("e"))
			{
				if (iMaxDecimals > 0)
				{
					iIndex = sz.Find('.');
					if (iIndex >= 0)
					{
						iLength = sz.Length();
						sz.Append('0', (iMaxDecimals - (iLength - iIndex)) + 1);
					}
				}
				else
				{
					if (sz.EndsWith('.'))
					{
						sz.RemoveFromEnd(1);
					}
				}
			}
		}
		sz.CopyIntoBuffer(szDest, iDestLength);
		sz.Kill();

		gcNumberControl.Remove(2);
		return szDest;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FloatToString(char* szDest, int iDestLength, float f, int iMaxDecimals, BOOL bAppendType)
{
	return NumberToString<float, int>(szDest, iDestLength, f, iMaxDecimals, bAppendType, "f", 0x80000000, 0x7fffff, 0x800000, 0x7f800000, 23, 255, 127, 9);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* DoubleToString(char* szDest, int iDestLength, double f, int iMaxDecimals, BOOL bAppendType)
{
	return NumberToString<double, long long int>(szDest, iDestLength, f, iMaxDecimals, bAppendType, "", 0x8000000000000000, 0x7ffffffffffff, 0x8000000000000, 0x3FF8000000000000, 51, 0x7FF, 1023, 16);
}

