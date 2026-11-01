#include "BaseLib/PointerRemapper.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "TestLib/Assert.h"
#include "JavaParserLib/JavaTokenParserEnvironment.h"
#include "JavaParserLib/JavaSyntaxParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserClassVariablesSimpleDeclaration(void)
{
	CTokenParserEnvironment		cTokenParser;
	CJavaSyntaxParser			cSyntaxParser;
	CJavaSyntaxMemory			cSyntaxMemory;
	CJavaSyntaxFile*			pcFile;
	CJavaSyntaxPackage*			pcPackage;
	CImportArray*				papcImports;
	CClassCommonArray*			papcClasses;
	CJavaSyntaxClass*			pcClass;
	CJavaSyntaxType*			pcType;
	CJavaSyntaxClassBlock*		pcBlock;
	CChars						sz;
	CStatementArray*			papcStatements;
	CJavaSyntaxStatement*		pcStatement;
	char						szFilename[] = "TestJavaSyntaxParserClassVariablesSimpleDeclaration.Java";
	char						szFileContents[] = "\
class Clazz\n\
{\n\
	private int y[];\n\
	public static final X x;\n\
}\n\
";

	cTokenParser.Init(szFilename, szFileContents, false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	AssertTrue(cSyntaxParser.Parse());

	pcFile = cSyntaxParser.GetSyntaxFile();
	pcPackage = pcFile->GetPackage();
	AssertNull(pcPackage);

	papcImports = pcFile->GetImports();
	AssertNotNull(papcImports);
	AssertSize(0, papcImports->NumElements());

	papcClasses = pcFile->GetClasses();
	AssertNotNull(papcClasses);
	AssertSize(1, papcClasses->NumElements());

	pcClass = (CJavaSyntaxClass*)papcClasses->GetPtr(0);
	AssertTrue(pcClass->IsClassCommon());
	AssertTrue(pcClass->IsClass());

	pcType = pcClass->GetSyntaxType();
	sz.Init();
	pcType->PrettyPrint(&sz);
	AssertString("Clazz", sz.Text());
	sz.Kill();
	pcBlock = pcClass->GetBlock();
	AssertNotNull(pcBlock);
	papcStatements = pcBlock->GetStatements();
	AssertNotNull(papcStatements);
	AssertSize(2, papcStatements->NumElements());
	pcStatement = papcStatements->GetPtr(0);
	AssertTrue(pcStatement->IsStatement());
	AssertTrue(pcStatement->IsVariableDeclaration());
	sz.Init();
	pcStatement->PrettyPrint(&sz, 0);
	AssertString("private int y[]", sz.Text());
	sz.Kill();

	pcStatement = papcStatements->GetPtr(1);
	AssertTrue(pcStatement->IsStatement());
	AssertTrue(pcStatement->IsVariableDeclaration());
	sz.Init();
	pcStatement->PrettyPrint(&sz, 0);
	AssertString("public static final X x", sz.Text());
	sz.Kill();

	sz.Init();
	pcFile->PrettyPrint(&sz, 0);
	AssertString("\
Class Clazz\n\
{\n\
	private int y[];\n\
	public static final X x;\n\
}\n\
", sz.Text());
	sz.Kill();

	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserClassVariablesGenericDeclaration(void)
{
	CTokenParserEnvironment				cTokenParser;
	CJavaSyntaxParser					cSyntaxParser;
	CJavaSyntaxMemory					cSyntaxMemory;
	CJavaSyntaxFile*					pcFile;
	CJavaSyntaxPackage*					pcPackage;
	CImportArray*						papcImports;
	CClassCommonArray*					papcClasses;
	CJavaSyntaxClass*					pcClass;
	CJavaSyntaxType*					pcType;
	CJavaSyntaxClassBlock*				pcBlock;
	CChars								sz;
	CStatementArray*					papcStatements;
	CJavaSyntaxStatement*				pcStatement;
	CJavaModifiers*						pcModifiers;
	CJavaSyntaxVariableDeclaration*		pcDeclaration;
	CIdentifierArray*					pcReferenceType;
	CJavaSyntaxGeneric*					pcGeneric;
	CTypeCommonArray*					apcGenerics;
	CJavaSyntaxExtent*					pcExtent;
	char								szFilename[] = "TestJavaSyntaxParserClassVariablesGenericDeclaration.Java";
	char								szFileContents[] = "\
class Clazz<Y>\n\
{\n\
	public final List<Map<? extends X, ? extends Integer>> map;\n\
	List<Pair<?, Y>> list;\n\
	List<> array3DList[][][];\n\
}\n\
";

	cTokenParser.Init(szFilename, szFileContents, false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	AssertTrue(cSyntaxParser.Parse());

	pcFile = cSyntaxParser.GetSyntaxFile();
	pcPackage = pcFile->GetPackage();
	AssertNull(pcPackage);

	papcImports = pcFile->GetImports();
	AssertNotNull(papcImports);
	AssertSize(0, papcImports->NumElements());

	papcClasses = pcFile->GetClasses();
	AssertNotNull(papcClasses);
	AssertSize(1, papcClasses->NumElements());

	pcClass = (CJavaSyntaxClass*)papcClasses->GetPtr(0);
	AssertTrue(pcClass->IsClassCommon());
	AssertTrue(pcClass->IsClass());

	pcType = pcClass->GetSyntaxType();
	sz.Init();
	pcType->PrettyPrint(&sz, 0);
	AssertString("Clazz<Y>", sz.Text());
	sz.Kill();
	pcBlock = pcClass->GetBlock();
	AssertNotNull(pcBlock);
	papcStatements = pcBlock->GetStatements();
	AssertNotNull(papcStatements);
	AssertSize(3, papcStatements->NumElements());

	pcStatement = papcStatements->GetPtr(0);
	AssertTrue(pcStatement->IsStatement());
	AssertTrue(pcStatement->IsVariableDeclaration());
	sz.Init();
	pcStatement->PrettyPrint(&sz, 0);
	AssertString("public final List<Map<? extends X, ? extends Integer>> map", sz.Text());
	sz.Kill();

	pcDeclaration = (CJavaSyntaxVariableDeclaration*)pcStatement;
	pcModifiers = pcDeclaration->GetModifiers();
	AssertFalse(pcModifiers->IsNone());
	AssertTrue(pcModifiers->IsPublic());
	AssertTrue(pcModifiers->IsFinal());
	AssertFalse(pcModifiers->IsProtected());
	AssertFalse(pcModifiers->IsPrivate());
	AssertFalse(pcModifiers->IsPackageModifier());
	AssertFalse(pcModifiers->IsStatic());
	AssertFalse(pcModifiers->IsStrictfp());
	
	AssertFalse(pcDeclaration->IsPrimitiveType());
	AssertTrue(pcDeclaration->IsReferenceType());
	AssertNull(pcDeclaration->GetPrimitiveType());
	pcReferenceType = pcDeclaration->GetReferenceType();
	AssertSize(1, pcReferenceType->NumElements());
	AssertString("List", pcReferenceType->GetPtr(0)->GetIdentifer());

	pcGeneric = pcDeclaration->GetGeneric();
	apcGenerics = pcGeneric->GetGenerics();
	AssertSize(1, apcGenerics->NumElements());
	pcType = (CJavaSyntaxType*)apcGenerics->GetPtr(0);
	AssertTrue(pcType->IsTypeCommon());
	AssertTrue(pcType->IsType());
	AssertFalse(pcType->IsExtent());
	AssertString("Map", pcType->GetName()->GetIdentifer());
	pcGeneric = pcType->GetGeneric();

	apcGenerics = pcGeneric->GetGenerics();
	AssertSize(2, apcGenerics->NumElements());
	pcExtent = (CJavaSyntaxExtent*)apcGenerics->GetPtr(0);
	AssertTrue(pcExtent->IsTypeCommon());
	AssertFalse(pcExtent->IsType());
	AssertTrue(pcExtent->IsExtent());
	AssertNull(pcExtent->GetName());
	AssertTrue(pcExtent->IsWildCard());

	pcStatement = papcStatements->GetPtr(1);
	AssertTrue(pcStatement->IsStatement());
	AssertTrue(pcStatement->IsVariableDeclaration());
	sz.Init();
	pcStatement->PrettyPrint(&sz, 0);
	AssertString("List<Pair<?, Y>> list", sz.Text());
	sz.Kill();

	pcStatement = papcStatements->GetPtr(2);
	AssertTrue(pcStatement->IsStatement());
	AssertTrue(pcStatement->IsVariableDeclaration());
	sz.Init();
	pcStatement->PrettyPrint(&sz, 0);
	AssertString("List<> array3DList[][][]", sz.Text());
	sz.Kill();

	sz.Init();
	pcFile->PrettyPrint(&sz, 0);
	AssertString("\
Class Clazz<Y>\n\
{\n\
	public final List<Map<? extends X, ? extends Integer>> map;\n\
	List<Pair<?, Y>> list;\n\
	List<> array3DList[][][];\n\
}\n\
", sz.Text());
	sz.Kill();

	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserClassVariablesSimpleInitialisation(void)
{
	CTokenParserEnvironment		cTokenParser;
	CJavaSyntaxParser			cSyntaxParser;
	CJavaSyntaxMemory			cSyntaxMemory;
	char						szFilename[] = "TestJavaSyntaxParserClassVariablesSimpleInitialisation.Java";
	char						szFileContents[] = "\
class Clazz\n\
{\n\
  public static final String s = \"\";\n\
  java.util.Date date = new Date();\n\
}\n\
";

	cTokenParser.Init(szFilename, szFileContents, false);
	AssertTrue(cTokenParser.Parse());

	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());

	AssertTrue(cSyntaxParser.Parse());
	cTokenParser.DumpLog();

	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserClassVariablesNewInitialisation(void)
{
	CTokenParserEnvironment		cTokenParser;
	CJavaSyntaxParser			cSyntaxParser;
	CJavaSyntaxMemory			cSyntaxMemory;
	CJavaSyntaxFile*			pcFile;
	CClassCommonArray*			papcClasses;
	CJavaSyntaxClass*			pcClass;
	CStatementArray*			papcStatements;
	char						szFilename[] = "TestJavaSyntaxParserClassVariablesNewInitialisation.Java";
	char						szFileContents[] = "\
class Clazz\n\
{\n\
  Pair pair = new Pair(1, \"x\");\n\
  Map<String, Integer> map = new HashMap<String, Integer>();\n\
  int i = 3;\n\
}\n\
";

	cTokenParser.Init(szFilename, szFileContents, false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	AssertTrue(cSyntaxParser.Parse());

	pcFile = cSyntaxParser.GetSyntaxFile();
	papcClasses = pcFile->GetClasses();
	AssertSize(1, papcClasses->NumElements());
	pcClass = (CJavaSyntaxClass*)papcClasses->GetPtr(0);
	papcStatements = pcClass->GetBlock()->GetStatements();
	AssertSize(3, papcStatements->NumElements());
	AssertTrue(papcStatements->GetPtr(0)->IsVariableDeclaration());
	AssertTrue(papcStatements->GetPtr(1)->IsVariableDeclaration());
	AssertTrue(papcStatements->GetPtr(2)->IsVariableDeclaration());

	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserClassVariablesNewInitialisationErrors(void)
{
	CTokenParserEnvironment		cTokenParser;
	CJavaSyntaxParser			cSyntaxParser;
	CJavaSyntaxMemory			cSyntaxMemory;
	char						szFilename[] = "TestJavaSyntaxParserClassVariablesNewInitialisationErrors.Java";
	char						szUnclosed[] = "\
class Clazz\n\
{\n\
  Date date = new Date(;\n\
}\n\
";
	char						szNoSemicolon[] = "\
class Clazz\n\
{\n\
  Date date = new Date()\n\
}\n\
";
	char						szBadGeneric[] = "\
class Clazz\n\
{\n\
  List list = new List<;\n\
}\n\
";

	cTokenParser.Init(szFilename, szUnclosed, false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	AssertFalse(cSyntaxParser.Parse());
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();

	cTokenParser.Init(szFilename, szNoSemicolon, false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	AssertFalse(cSyntaxParser.Parse());
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();

	cTokenParser.Init(szFilename, szBadGeneric, false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	AssertFalse(cSyntaxParser.Parse());
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserClassVariables(void)
{
	BeginTests();

	TestJavaSyntaxParserClassVariablesSimpleDeclaration();
	TestJavaSyntaxParserClassVariablesGenericDeclaration();
	TestJavaSyntaxParserClassVariablesSimpleInitialisation();
	TestJavaSyntaxParserClassVariablesNewInitialisation();
	TestJavaSyntaxParserClassVariablesNewInitialisationErrors();

	TestStatistics();
}

