#ifndef __CHAR_ENCODING_H__
#define __CHAR_ENCODING_H__


enum ECharEncoding
{
	CE_US_ASCII,
	CE_UTF_8,
	CE_UTF_16,  //UTF-16 byte ordering must be detected from the BOM.
	CE_ISO_8859_1,
	CE_Unkown,
	CE_NoSet,
};


class CCharEncoding
{
public:
	static ECharEncoding	ParseEncoding(char* szEncodingString);
};


#endif // __CHAR_ENCODING_H__

