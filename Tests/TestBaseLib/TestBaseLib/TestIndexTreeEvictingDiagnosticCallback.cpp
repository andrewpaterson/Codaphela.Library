#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/Logger.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/IndexTreeHelper.h"
#include "BaseLib/IndexTreeEvicting.h"
#include "BaseLib/IndexTreeEvictingAccess.h"
#include "BaseLib/IndexTreeEvictionStrategyDataOrderer.h"
#include "BaseLib/IndexTreeFileDiagnosticLoggingCallback.h"
#include "TestLib/Assert.h"
#include "TestIndexTreeObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileKeyDiagnosticCallbackStuff(void)
{
	CIndexTreeEvictingAccess 						cAccess;
	CIndexTreeEvicting								cIndexTree;
	CIndexTreeHelper								cHelper;
	CDurableFileController							cController;
	CIndexTreeEvictionStrategyDataOrderer			cEvictionStrategy;
	CAccessDataOrderer								cOrderer;
	CIndexTreeFileDiagnosticLoggingCallback			cCallback;
	char											szData[1024];
	int												i;
	CChars											sz;
	size											uiCacheSize;

	for (i = 0; i < 1023; i++)
	{
		szData[i] = (char)('A' + i % 26);
	}
	szData[1023] = '\0';

	cHelper.Init("Output" _FS_ "IndexTreeEvictingDiagnostic", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cOrderer.Init();
	cEvictionStrategy.Init(LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	cController.Begin();
	cCallback.Init();
	//Room for three keys.  The nodes are bigger in a 64 bit build.
#ifdef BUILD_64
	uiCacheSize = 9 KB;
#else
	uiCacheSize = 8 KB;
#endif // BUILD_64
	cIndexTree.Init(&cController, NULL, uiCacheSize, NULL, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy), NULL, IWT_No, IKR_No, LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	cIndexTree.SetDiagnosticCallback(&cCallback);
	cAccess.Init(&cIndexTree);

	cAccess.PutStringString("A0000001", szData);

	cAccess.PutStringString("B0000002", szData);

	cAccess.PutStringString("C0000003", szData);

	cAccess.PutStringString("D0000004", szData);

	cAccess.PutStringString("E0000005", szData);

	cAccess.PutStringString("F0000006", szData);

	cAccess.DeleteString("B0000002");

	cAccess.PutStringString("G0000007", szData);

	cAccess.Flush();

	cController.End();
	cAccess.Kill();
	cIndexTree.Kill();
	
	sz.Init();
	cCallback.Print(&sz);
	AssertString(
		"Put:    A0000001 [1024]\n" 
		"Put:    B0000002 [1024]\n" 
		"Put:    C0000003 [1024]\n" 
		"Put:    D0000004 [1024]\n" 
		"Flush:  A0000001 [1024]\n" 
		"Evict:  A0000001 [1024]\n" 
		"Put:    E0000005 [1024]\n" 
		"Flush:  B0000002 [1024]\n" 
		"Evict:  B0000002 [1024]\n" 
		"Put:    F0000006 [1024]\n" 
		"Flush:  C0000003 [1024]\n" 
		"Evict:  C0000003 [1024]\n" 
		"Remove: B0000002 [1024]\n" 
		"Flush:  D0000004 [1024]\n" 
		"Evict:  D0000004 [1024]\n" 
		"Put:    G0000007 [1024]\n" 
		"Flush:  E0000005 [1024]\n" 
		"Evict:  E0000005 [1024]\n" 
		"Flush:  B0000002 [0]\n" 
		"Flush:  F0000006 [1024]\n" 
		"Flush:  G0000007 [1024]\n" 
		"Evict:  F0000006 [1024]\n" 
		"Evict:  G0000007 [1024]\n", 
		sz.Text());

	sz.Kill();
	cCallback.Kill();
	cController.Kill();
	cEvictionStrategy.Kill();
	cOrderer.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileKeyDiagnosticCallback(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestIndexTreeFileKeyDiagnosticCallbackStuff();

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}


