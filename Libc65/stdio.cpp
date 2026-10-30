//Formatted output and files for the 65C64, in C++ for the 65C64 generator.  stdout and stderr write to the
//console at 00FF:0100 (each byte stored there is printed).  Files are the host's (host.h).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <host.h>


//The console: a byte stored here is printed.
#define CONSOLE_ADDRESS		0x00FF0100


struct FILE
{
	int		iKind;		//FILE_CONSOLE, FILE_HOST or FILE_CLOSED.
	int		iHandle;	//The host's handle.
	int		iError;
	int		iEnd;
};


#define FILE_CLOSED		0
#define FILE_CONSOLE	1
#define FILE_HOST		2


static FILE		gsStdin = { FILE_CLOSED, 0, 0, 1 };
static FILE		gsStdout = { FILE_CONSOLE, 0, 0, 0 };
static FILE		gsStderr = { FILE_CONSOLE, 0, 0, 0 };

FILE*	stdin = &gsStdin;
FILE*	stdout = &gsStdout;
FILE*	stderr = &gsStderr;


//Where formatted output goes: a file, or a buffer of uiSize bytes (the count goes on past it).
struct SFormatOutput
{
	FILE*	pcFile;
	char*	szDest;
	size_t	uiSize;
	size_t	uiCount;
};


static void ConsoleWrite(char c)
{
	*(volatile char*)CONSOLE_ADDRESS = c;
}


int fputc(int c, FILE* pcFile)
{
	char	cByte;

	cByte = (char)c;
	if (fwrite(&cByte, 1, 1, pcFile) != 1)
	{
		return EOF;
	}
	return (unsigned char)c;
}


int putchar(int c)
{
	return fputc(c, stdout);
}


int fputs(const char* sz, FILE* pcFile)
{
	while (*sz)
	{
		if (fputc(*sz, pcFile) == EOF)
		{
			return EOF;
		}
		sz++;
	}
	return 0;
}


int puts(const char* sz)
{
	if (fputs(sz, stdout) == EOF)
	{
		return EOF;
	}
	return putchar('\n');
}


//"r", "w", "a" then "+" for reading and writing; "b" and "t" make no difference.
FILE* fopen(const char* szName, const char* szMode)
{
	FILE*	pcFile;
	int		iMode;
	int		iHandle;

	if (szMode[0] == 'r')
	{
		iMode = HOST_READ;
	}
	else if (szMode[0] == 'w')
	{
		iMode = HOST_WRITE | HOST_CREATE | HOST_TRUNCATE;
	}
	else if (szMode[0] == 'a')
	{
		iMode = HOST_WRITE | HOST_CREATE | HOST_APPEND;
	}
	else
	{
		return NULL;
	}
	if (strchr(szMode, '+'))
	{
		iMode |= HOST_READ | HOST_WRITE;
	}

	iHandle = HostOpen(szName, iMode);
	if (iHandle == 0)
	{
		return NULL;
	}
	pcFile = (FILE*)malloc(sizeof(FILE));
	pcFile->iKind = FILE_HOST;
	pcFile->iHandle = iHandle;
	pcFile->iError = 0;
	pcFile->iEnd = 0;
	return pcFile;
}


int fclose(FILE* pcFile)
{
	int		iResult;

	if (!pcFile || (pcFile->iKind != FILE_HOST))
	{
		return EOF;
	}
	iResult = HostClose(pcFile->iHandle) ? 0 : EOF;
	pcFile->iKind = FILE_CLOSED;
	free(pcFile);
	return iResult;
}


size_t fread(void* pv, size_t uiSize, size_t uiCount, FILE* pcFile)
{
	size_t	uiBytes;
	size_t	uiRead;

	if (!pcFile || (pcFile->iKind != FILE_HOST) || (uiSize == 0) || (uiCount == 0))
	{
		return 0;
	}
	uiBytes = uiSize * uiCount;
	uiRead = HostRead(pcFile->iHandle, pv, uiBytes);
	if (uiRead < uiBytes)
	{
		pcFile->iEnd = 1;
	}
	return uiRead / uiSize;
}


size_t fwrite(const void* pv, size_t uiSize, size_t uiCount, FILE* pcFile)
{
	const char*	pc;
	size_t		uiBytes;
	size_t		i;

	if (!pcFile || (uiSize == 0) || (uiCount == 0))
	{
		return 0;
	}
	uiBytes = uiSize * uiCount;
	if (pcFile->iKind == FILE_CONSOLE)
	{
		pc = (const char*)pv;
		for (i = 0; i < uiBytes; i++)
		{
			ConsoleWrite(pc[i]);
		}
		return uiCount;
	}
	if (pcFile->iKind == FILE_HOST)
	{
		return HostWrite(pcFile->iHandle, pv, uiBytes) / uiSize;
	}
	return 0;
}


int fseek(FILE* pcFile, long iOffset, int iOrigin)
{
	if (!pcFile || (pcFile->iKind != FILE_HOST))
	{
		return -1;
	}
	pcFile->iEnd = 0;
	return HostSeek(pcFile->iHandle, iOffset, iOrigin) ? 0 : -1;
}


long ftell(FILE* pcFile)
{
	if (!pcFile || (pcFile->iKind != FILE_HOST))
	{
		return -1;
	}
	return (long)HostTell(pcFile->iHandle);
}


int fflush(FILE* pcFile)
{
	if (pcFile && (pcFile->iKind == FILE_HOST))
	{
		return HostFlush(pcFile->iHandle) ? 0 : EOF;
	}
	return 0;
}


int feof(FILE* pcFile)
{
	return pcFile ? pcFile->iEnd : 1;
}


int fgetc(FILE* pcFile)
{
	unsigned char	c;

	if (fread(&c, 1, 1, pcFile) != 1)
	{
		return EOF;
	}
	return c;
}


int remove(const char* szName)
{
	return HostDelete(szName) ? 0 : -1;
}


int rename(const char* szOld, const char* szNew)
{
	return HostRename(szOld, szNew) ? 0 : -1;
}


static void Emit(SFormatOutput* psOutput, char c)
{
	if (psOutput->pcFile)
	{
		fputc(c, psOutput->pcFile);
	}
	else if (psOutput->uiCount + 1 < psOutput->uiSize)
	{
		psOutput->szDest[psOutput->uiCount] = c;
	}
	psOutput->uiCount++;
}


static void EmitRepeated(SFormatOutput* psOutput, char c, int iCount)
{
	while (iCount > 0)
	{
		Emit(psOutput, c);
		iCount--;
	}
}


//The flags of a conversion.
#define FORMAT_LEFT		0x01
#define FORMAT_ZERO		0x02
#define FORMAT_PLUS		0x04
#define FORMAT_SPACE	0x08
#define FORMAT_ALTERNATE	0x10
#define FORMAT_UPPER	0x20


//The digits (and prefix and sign) of a conversion padded to its width.
static void EmitField(SFormatOutput* psOutput, const char* szPrefix, const char* szDigits, int iDigits, int iZeros, int iWidth, int iFlags)
{
	int		iPrefix;
	int		iPadding;

	iPrefix = (int)strlen(szPrefix);
	iPadding = iWidth - iPrefix - iZeros - iDigits;
	if ((iPadding > 0) && !(iFlags & FORMAT_LEFT) && !(iFlags & FORMAT_ZERO))
	{
		EmitRepeated(psOutput, ' ', iPadding);
		iPadding = 0;
	}
	while (*szPrefix)
	{
		Emit(psOutput, *szPrefix);
		szPrefix++;
	}
	if ((iPadding > 0) && !(iFlags & FORMAT_LEFT))
	{
		EmitRepeated(psOutput, '0', iPadding);
		iPadding = 0;
	}
	EmitRepeated(psOutput, '0', iZeros);
	while (iDigits > 0)
	{
		Emit(psOutput, *szDigits);
		szDigits++;
		iDigits--;
	}
	EmitRepeated(psOutput, ' ', iPadding);
}


static void FormatInteger(SFormatOutput* psOutput, unsigned long long ullValue, bool bNegative, int iBase, int iWidth, int iPrecision, int iFlags)
{
	char		acDigits[24];
	char		szPrefix[4];
	int			iDigits;
	int			iZeros;
	int			iDigit;
	const char*	szAlphabet;

	szAlphabet = (iFlags & FORMAT_UPPER) ? "0123456789ABCDEF" : "0123456789abcdef";
	iDigits = 0;
	while (ullValue != 0)
	{
		iDigit = (int)(ullValue % (unsigned)iBase);
		ullValue /= (unsigned)iBase;
		acDigits[sizeof(acDigits) - 1 - iDigits] = szAlphabet[iDigit];
		iDigits++;
	}
	if ((iDigits == 0) && (iPrecision != 0))
	{
		acDigits[sizeof(acDigits) - 1] = '0';
		iDigits = 1;
	}

	szPrefix[0] = 0;
	if (bNegative)
	{
		strcpy(szPrefix, "-");
	}
	else if (iFlags & FORMAT_PLUS)
	{
		strcpy(szPrefix, "+");
	}
	else if (iFlags & FORMAT_SPACE)
	{
		strcpy(szPrefix, " ");
	}
	if ((iFlags & FORMAT_ALTERNATE) && (iDigits > 0) && !((iDigits == 1) && (acDigits[sizeof(acDigits) - 1] == '0')))
	{
		if (iBase == 16)
		{
			strcat(szPrefix, (iFlags & FORMAT_UPPER) ? "0X" : "0x");
		}
		else if ((iBase == 8) && (iPrecision <= iDigits))
		{
			strcat(szPrefix, "0");
		}
	}

	iZeros = (iPrecision > iDigits) ? iPrecision - iDigits : 0;
	if (iPrecision >= 0)
	{
		iFlags &= ~FORMAT_ZERO;
	}
	EmitField(psOutput, szPrefix, &acDigits[sizeof(acDigits) - iDigits], iDigits, iZeros, iWidth, iFlags);
}


//The decimal digits of a positive finite double: *piExponent is the power of ten of the first digit.  Each digit
//is worked out from the remainder, so digits past the 17th are not exact.
static void DecimalDigits(double dValue, char* acDigits, int iCount, int* piExponent)
{
	int		iExponent;
	int		i;
	int		iDigit;

	iExponent = 0;
	if (dValue != 0.0)
	{
		while (dValue >= 1e16)
		{
			dValue /= 1e16;
			iExponent += 16;
		}
		while (dValue >= 10.0)
		{
			dValue /= 10.0;
			iExponent++;
		}
		while (dValue < 1e-16)
		{
			dValue *= 1e16;
			iExponent -= 16;
		}
		while (dValue < 1.0)
		{
			dValue *= 10.0;
			iExponent--;
		}
	}
	for (i = 0; i < iCount; i++)
	{
		iDigit = (int)dValue;
		if (iDigit > 9)
		{
			iDigit = 9;
		}
		acDigits[i] = (char)('0' + iDigit);
		dValue = (dValue - iDigit) * 10.0;
	}
	*piExponent = iExponent;
}


//Rounds the first iKeep digits half up.  Returns true if they carried into a new first digit (they are then
//1 followed by zeros).
static bool RoundDigits(char* acDigits, int iKeep, int iCount)
{
	int		i;

	if ((iKeep >= iCount) || (acDigits[iKeep] < '5'))
	{
		return false;
	}
	for (i = iKeep - 1; i >= 0; i--)
	{
		if (acDigits[i] != '9')
		{
			acDigits[i]++;
			return false;
		}
		acDigits[i] = '0';
	}
	if (iKeep > 0)
	{
		acDigits[0] = '1';
	}
	return true;
}


#define FORMAT_FLOAT_DIGITS	40


static void FormatFloat(SFormatOutput* psOutput, double dValue, char cConversion, int iWidth, int iPrecision, int iFlags)
{
	char		acDigits[FORMAT_FLOAT_DIGITS + 2];
	char		acText[FORMAT_FLOAT_DIGITS * 2 + 400];
	char		szPrefix[2];
	int			iExponent;
	int			iLength;
	int			iKeep;
	int			i;
	int			iDigit;
	bool		bNegative;
	bool		bExponent;
	bool		bTrim;
	char		cLower;

	cLower = (char)(cConversion | 0x20);
	if ((cConversion >= 'A') && (cConversion <= 'Z'))
	{
		iFlags |= FORMAT_UPPER;
	}
	if (iPrecision < 0)
	{
		iPrecision = 6;
	}

	bNegative = (dValue < 0.0) || ((dValue == 0.0) && (1.0 / dValue < 0.0));
	if (bNegative)
	{
		dValue = -dValue;
	}
	szPrefix[0] = bNegative ? '-' : (iFlags & FORMAT_PLUS) ? '+' : (iFlags & FORMAT_SPACE) ? ' ' : 0;
	szPrefix[1] = 0;

	if (dValue != dValue)
	{
		EmitField(psOutput, szPrefix, (iFlags & FORMAT_UPPER) ? "NAN" : "nan", 3, 0, iWidth, iFlags & ~FORMAT_ZERO);
		return;
	}
	if (dValue > 1.7976931348623157e308)
	{
		EmitField(psOutput, szPrefix, (iFlags & FORMAT_UPPER) ? "INF" : "inf", 3, 0, iWidth, iFlags & ~FORMAT_ZERO);
		return;
	}

	DecimalDigits(dValue, acDigits, FORMAT_FLOAT_DIGITS, &iExponent);
	if (dValue == 0.0)
	{
		iExponent = 0;
	}

	bTrim = false;
	bExponent = (cLower == 'e');
	if (cLower == 'g')
	{
		//%g: %e if the exponent is below -4 or at least the precision, else %f, without trailing zeros.
		if (iPrecision == 0)
		{
			iPrecision = 1;
		}
		if (RoundDigits(acDigits, iPrecision, FORMAT_FLOAT_DIGITS))
		{
			iExponent++;
		}
		bExponent = (iExponent < -4) || (iExponent >= iPrecision);
		iPrecision = bExponent ? iPrecision - 1 : iPrecision - 1 - iExponent;
		bTrim = !(iFlags & FORMAT_ALTERNATE);
	}

	iLength = 0;
	if (bExponent)
	{
		if ((cLower != 'g') && RoundDigits(acDigits, iPrecision + 1, FORMAT_FLOAT_DIGITS))
		{
			iExponent++;
		}
		acText[iLength++] = acDigits[0];
		if ((iPrecision > 0) || (iFlags & FORMAT_ALTERNATE))
		{
			acText[iLength++] = '.';
		}
		for (i = 1; i <= iPrecision; i++)
		{
			acText[iLength++] = (i < FORMAT_FLOAT_DIGITS) ? acDigits[i] : '0';
		}
	}
	else
	{
		//The digits before the point are the exponent + 1 of them.
		iKeep = iExponent + 1 + iPrecision;
		if (iKeep < 0)
		{
			for (i = 0; i < FORMAT_FLOAT_DIGITS; i++)
			{
				acDigits[i] = '0';
			}
		}
		else if ((cLower != 'g') && RoundDigits(acDigits, iKeep, FORMAT_FLOAT_DIGITS))
		{
			iExponent++;
		}
		else if ((iKeep == 0) && (acDigits[0] >= '5'))
		{
			acDigits[0] = '1';
			iExponent++;
		}
		else if (iKeep == 0)
		{
			acDigits[0] = '0';
		}

		if (iExponent < 0)
		{
			acText[iLength++] = '0';
		}
		for (i = 0; i <= iExponent; i++)
		{
			acText[iLength++] = (i < FORMAT_FLOAT_DIGITS) ? acDigits[i] : '0';
		}
		if ((iPrecision > 0) || (iFlags & FORMAT_ALTERNATE))
		{
			acText[iLength++] = '.';
		}
		for (i = 1; i <= iPrecision; i++)
		{
			iDigit = iExponent + i;
			acText[iLength++] = ((iDigit >= 0) && (iDigit < FORMAT_FLOAT_DIGITS)) ? acDigits[iDigit] : '0';
		}
	}

	if (bTrim)
	{
		for (i = 0; (i < iLength) && (acText[i] != '.'); i++)
		{
		}
		if (i < iLength)
		{
			while (acText[iLength - 1] == '0')
			{
				iLength--;
			}
			if (acText[iLength - 1] == '.')
			{
				iLength--;
			}
		}
	}

	if (bExponent)
	{
		acText[iLength++] = (iFlags & FORMAT_UPPER) ? 'E' : 'e';
		acText[iLength++] = (iExponent < 0) ? '-' : '+';
		if (iExponent < 0)
		{
			iExponent = -iExponent;
		}
		if (iExponent >= 100)
		{
			acText[iLength++] = (char)('0' + iExponent / 100);
		}
		acText[iLength++] = (char)('0' + (iExponent / 10) % 10);
		acText[iLength++] = (char)('0' + iExponent % 10);
	}
	EmitField(psOutput, szPrefix, acText, iLength, 0, iWidth, iFlags);
}


static int Format(SFormatOutput* psOutput, const char* szFormat, va_list vaArguments)
{
	int					iFlags;
	int					iWidth;
	int					iPrecision;
	int					iLength;
	char				c;
	long long			llValue;
	unsigned long long	ullValue;
	const char*			sz;
	int					iBytes;
	char				szChar[2];

	szChar[1] = 0;
	while (*szFormat)
	{
		c = *szFormat++;
		if (c != '%')
		{
			Emit(psOutput, c);
			continue;
		}

		iFlags = 0;
		for (;;)
		{
			c = *szFormat;
			if (c == '-')			iFlags |= FORMAT_LEFT;
			else if (c == '0')		iFlags |= FORMAT_ZERO;
			else if (c == '+')		iFlags |= FORMAT_PLUS;
			else if (c == ' ')		iFlags |= FORMAT_SPACE;
			else if (c == '#')		iFlags |= FORMAT_ALTERNATE;
			else					break;
			szFormat++;
		}

		iWidth = 0;
		if (*szFormat == '*')
		{
			iWidth = va_arg(vaArguments, int);
			if (iWidth < 0)
			{
				iFlags |= FORMAT_LEFT;
				iWidth = -iWidth;
			}
			szFormat++;
		}
		while ((*szFormat >= '0') && (*szFormat <= '9'))
		{
			iWidth = iWidth * 10 + (*szFormat - '0');
			szFormat++;
		}

		iPrecision = -1;
		if (*szFormat == '.')
		{
			szFormat++;
			iPrecision = 0;
			if (*szFormat == '*')
			{
				iPrecision = va_arg(vaArguments, int);
				szFormat++;
			}
			while ((*szFormat >= '0') && (*szFormat <= '9'))
			{
				iPrecision = iPrecision * 10 + (*szFormat - '0');
				szFormat++;
			}
		}

		//The argument's size: int unless l says long, l l (or j) long long and z or t size_t (an int is 16 bits on
		//the W65C816).  h and hh narrow it after it is read.
		iLength = 0;
		iBytes = (int)sizeof(int);
		for (;;)
		{
			c = *szFormat;
			if (c == 'h')
			{
				iLength = (iLength == 'h') ? 'H' : 'h';
			}
			else if (c == 'l')
			{
				iBytes = (iLength == 'l') ? (int)sizeof(long long) : (int)sizeof(long);
				iLength = 'l';
			}
			else if ((c == 'j') || (c == 'q'))	iBytes = 8;
			else if ((c == 'z') || (c == 't'))	iBytes = (int)sizeof(size_t);
			else if (c == 'L')					iLength = 'L';
			else if ((c == 'I') && (szFormat[1] == '6') && (szFormat[2] == '4'))
			{
				iBytes = 8;
				szFormat += 2;
			}
			else								break;
			szFormat++;
		}

		c = *szFormat;
		if (c == 0)
		{
			break;
		}
		szFormat++;
		switch (c)
		{
		case 'd':
		case 'i':
			if (iBytes == (int)sizeof(long long))
			{
				llValue = va_arg(vaArguments, long long);
			}
			else if (iBytes == (int)sizeof(int))
			{
				llValue = va_arg(vaArguments, int);
			}
			else
			{
				llValue = va_arg(vaArguments, long);
			}
			if (iLength == 'h')
			{
				llValue = (short)llValue;
			}
			else if (iLength == 'H')
			{
				llValue = (signed char)llValue;
			}
			ullValue = (llValue < 0) ? (unsigned long long)0 - (unsigned long long)llValue : (unsigned long long)llValue;
			FormatInteger(psOutput, ullValue, llValue < 0, 10, iWidth, iPrecision, iFlags);
			break;

		case 'u':
		case 'x':
		case 'X':
		case 'o':
		case 'b':
			if (iBytes == (int)sizeof(long long))
			{
				ullValue = va_arg(vaArguments, unsigned long long);
			}
			else if (iBytes == (int)sizeof(int))
			{
				ullValue = va_arg(vaArguments, unsigned int);
			}
			else
			{
				ullValue = va_arg(vaArguments, unsigned long);
			}
			if (iLength == 'h')
			{
				ullValue = (unsigned short)ullValue;
			}
			else if (iLength == 'H')
			{
				ullValue = (unsigned char)ullValue;
			}
			if (c == 'X')
			{
				iFlags |= FORMAT_UPPER;
			}
			FormatInteger(psOutput, ullValue, false, (c == 'u') ? 10 : (c == 'o') ? 8 : (c == 'b') ? 2 : 16, iWidth, iPrecision, iFlags & ~(FORMAT_PLUS | FORMAT_SPACE));
			break;

		case 'p':
			ullValue = (unsigned long long)(size_t)va_arg(vaArguments, void*);
			FormatInteger(psOutput, ullValue, false, 16, iWidth, 8, (iFlags & FORMAT_LEFT) | FORMAT_ALTERNATE);
			break;

		case 'c':
			szChar[0] = (char)va_arg(vaArguments, int);
			EmitField(psOutput, "", szChar, 1, 0, iWidth, iFlags & ~FORMAT_ZERO);
			break;

		case 's':
			sz = va_arg(vaArguments, const char*);
			if (!sz)
			{
				sz = "(null)";
			}
			iBytes = (int)strlen(sz);
			if ((iPrecision >= 0) && (iPrecision < iBytes))
			{
				iBytes = iPrecision;
			}
			EmitField(psOutput, "", sz, iBytes, 0, iWidth, iFlags & ~FORMAT_ZERO);
			break;

		case 'f':
		case 'F':
		case 'e':
		case 'E':
		case 'g':
		case 'G':
			FormatFloat(psOutput, va_arg(vaArguments, double), c, iWidth, iPrecision, iFlags);
			break;

		case 'n':
			*va_arg(vaArguments, int*) = (int)psOutput->uiCount;
			break;

		default:
			//%% and anything unknown is printed as it is.
			Emit(psOutput, c);
			break;
		}
	}

	if (!psOutput->pcFile && (psOutput->uiSize > 0))
	{
		psOutput->szDest[(psOutput->uiCount < psOutput->uiSize) ? psOutput->uiCount : psOutput->uiSize - 1] = 0;
	}
	return (int)psOutput->uiCount;
}


int vsnprintf(char* szDest, size_t uiSize, const char* szFormat, va_list vaArguments)
{
	SFormatOutput	sOutput;

	sOutput.pcFile = NULL;
	sOutput.szDest = szDest;
	sOutput.uiSize = uiSize;
	sOutput.uiCount = 0;
	return Format(&sOutput, szFormat, vaArguments);
}


int vsprintf(char* szDest, const char* szFormat, va_list vaArguments)
{
	return vsnprintf(szDest, 0x7FFFFFFF, szFormat, vaArguments);
}


int vfprintf(FILE* pcFile, const char* szFormat, va_list vaArguments)
{
	SFormatOutput	sOutput;

	sOutput.pcFile = pcFile;
	sOutput.szDest = NULL;
	sOutput.uiSize = 0;
	sOutput.uiCount = 0;
	return Format(&sOutput, szFormat, vaArguments);
}


int vprintf(const char* szFormat, va_list vaArguments)
{
	return vfprintf(stdout, szFormat, vaArguments);
}


int printf(const char* szFormat, ...)
{
	va_list	vaArguments;
	int		iResult;

	va_start(vaArguments, szFormat);
	iResult = vfprintf(stdout, szFormat, vaArguments);
	va_end(vaArguments);
	return iResult;
}


int fprintf(FILE* pcFile, const char* szFormat, ...)
{
	va_list	vaArguments;
	int		iResult;

	va_start(vaArguments, szFormat);
	iResult = vfprintf(pcFile, szFormat, vaArguments);
	va_end(vaArguments);
	return iResult;
}


int sprintf(char* szDest, const char* szFormat, ...)
{
	va_list	vaArguments;
	int		iResult;

	va_start(vaArguments, szFormat);
	iResult = vsnprintf(szDest, 0x7FFFFFFF, szFormat, vaArguments);
	va_end(vaArguments);
	return iResult;
}


int snprintf(char* szDest, size_t uiSize, const char* szFormat, ...)
{
	va_list	vaArguments;
	int		iResult;

	va_start(vaArguments, szFormat);
	iResult = vsnprintf(szDest, uiSize, szFormat, vaArguments);
	va_end(vaArguments);
	return iResult;
}

