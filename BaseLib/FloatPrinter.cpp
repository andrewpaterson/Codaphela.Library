#include "StringHelper.h"
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
		CNumber		cPow2;
		CNumber		cLog2;
		CNumber		cNum;

		cLog2.Init(szCommonLogarithmOfTwo, 2, 40);
		cPow2.Init(&cLog2);
		cNum.Init(iPow2);
		cPow2.Multiply(&cNum);
		cPow2.Ceiling();
		return cPow2.IntValue();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FloatToString(char* szDest, int iDestLength, float f, int iMaxDecimals, BOOL bAppendF)
{
	unsigned char*	pui;
	int				iMantissa;
	int				iSign;
	int				iExponent;
	int				iValue;

	pui = (unsigned char*)&f;
	iExponent = (*((int*)pui) & 0x7f800000) >> 23;
	iMantissa = *((int*)pui) & 0x7fffff;
	iSign = (*((int*)pui) & 0x80000000) == 0x80000000;

	if (iExponent == 0)
	{
		return NULL;
	}
	else if (iExponent == 255)
	{
		return NULL;
	}
	else
	{
		CNumber*	pcResult;
		CNumber*	pcTwoPower;
		int			iWholeDigits;
		int			iFractionalDigits;
		CNumber		cExponent;
		CNumber		cTwo;
		int			iLeftMost;
		CChars		sz;
		int			iFractionalPart;
		BOOL		bNumeric;
		int			iSignificantDigits;
		int			iIndex;
		int			iLength;
		int			iMaxSignificantDigits;

		iMaxSignificantDigits = 9;
		iExponent = iExponent - 127;  //Convert by exponent bias.
		iValue = (iMantissa | 0x800000);  //Add implied [1.fraction].

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
		iWholeDigits+=2;
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

		int iDigit = 0x800000;
		for (int i = 0; i < 24; i++)
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

		sz.Init();
		bNumeric = pcResult->PrintFloating(&sz);
		if (bNumeric && bAppendF)
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
	}

	return szDest;
}

