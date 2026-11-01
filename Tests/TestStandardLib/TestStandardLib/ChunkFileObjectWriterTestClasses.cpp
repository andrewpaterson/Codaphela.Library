#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/ObjectReader.h"
#include "ChunkFileObjectWriterTestClasses.h"


void CTestWithArray::Init(const char* szString, int x)
{
	PreInit();
	mcArray = OMalloc<CArrayObject>();
	mszString.Init(szString);
	mx = x;
	PostInit();
}


void CTestWithArray::Class(void)
{
	M_Pointer(mcArray);
	U_String(mszString);
	U_Int32(mx);
}


void CTestWithArray::Free(void)
{
	mszString.Kill();
}


void CTestWithArray::Add(CPointer& pcObject)
{
	mcArray->Add(pcObject);
}


bool CTestWithArray::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(mszString.WriteString(pcFile));
	ReturnOnFalse(pcFile->WriteInt32(mx));
	return true;
}


bool CTestWithArray::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(mszString.ReadString(pcFile));
	ReturnOnFalse(pcFile->ReadInt32(&mx));
	return true;
}


void CTestInteger::Init(int32 x, int16 y, int8 z)
{
	PreInit();
	mx = x;
	my = y;
	mz = z;
	PostInit();
}


void CTestInteger::Class(void)
{
	U_Int32(mx);
	U_Int16(my);
	U_Int8(mz);
}


void CTestInteger::Free(void)
{
}


bool CTestInteger::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteInt32(mx));
	ReturnOnFalse(pcFile->WriteInt16(my));
	ReturnOnFalse(pcFile->WriteInt8(mz));
	return true;
}


bool CTestInteger::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadInt32(&mx));
	ReturnOnFalse(pcFile->ReadInt16(&my));
	ReturnOnFalse(pcFile->ReadInt8(&mz));
	return true;
}


void CTestNamedString::Init(void)
{
	PreInit();
	mszString = NULL;
	mpAnother = NULL;
	mszEmbedded._Init();
	PostInit();
}


void CTestNamedString::Init(Ptr<CString> szString, Ptr<CTestNamedString> pAnother, const char* szEmbedded)
{
	PreInit();
	Set(szString, pAnother, szEmbedded);
	PostInit();
}


void CTestNamedString::Set(Ptr<CString> szString, Ptr<CTestNamedString> pAnother, const char* szEmbedded)
{
	mszString = szString;
	mpAnother = pAnother;
	mszEmbedded.Init(szEmbedded);
}


void CTestNamedString::Class(void)
{
	M_Pointer(mszString);
	M_Pointer(mpAnother);
	U_String(mszEmbedded);
}


void CTestNamedString::Free(void)
{
	mszEmbedded.Kill();
}


bool CTestNamedString::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(mszEmbedded.WriteString(pcFile));
	return true;
}


bool CTestNamedString::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(mszEmbedded.ReadString(pcFile));
	return true;
}


void CTestDoubleNamedString::Init(void)
{
	PreInit();
	mszString = Null();
	mpSplit1 = Null();
	mpSplit2 = Null();
	PostInit();
}


void CTestDoubleNamedString::Init(Ptr<CString> szString, Ptr<CTestNamedString> pSplit2, Ptr<CTestNamedString> pSplit1)
{
	PreInit();
	mszString = szString;
	mpSplit1 = pSplit1;
	mpSplit2 = pSplit2;
	PostInit();
}


void CTestDoubleNamedString::Class(void)
{
	M_Pointer(mszString);
	M_Pointer(mpSplit1);
	M_Pointer(mpSplit2);
}


void CTestDoubleNamedString::Free(void)
{
}

