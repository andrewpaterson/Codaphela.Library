#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include "BaseLib/NativeThread.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "ThreadLib/ProcessFork.h"
#include "ThreadLib/Process.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// The program TestProcess runs (this one, --test-process-output).  With
// "wait" it says so and sleeps until it is stopped.  With "bytes" it
// writes bytes a line splitter would change: a '\0', a carriage return
// and line feed, and a line feed, and an error.  Otherwise it writes
// a line to its output, one to its errors, its working directory and its
// arguments one a line, then "three" with no line end, and exits with
// its first argument.
//
//////////////////////////////////////////////////////////////////////////
int TestProcessOutputMain(int argc, char* argv[])
{
	CFileUtil	cFileUtil;
	CChars		szDirectory;
	int			i;

	if ((argc >= 1) && (strcmp(argv[0], "wait") == 0))
	{
		printf("waiting\n");
		fflush(stdout);
		ThreadSleep(30000);
		return 0;
	}
	if ((argc >= 1) && (strcmp(argv[0], "bytes") == 0))
	{
#ifdef _WIN32
		_setmode(_fileno(stdout), _O_BINARY);
#endif
		fwrite("a\0b\r\nc\n", 1, 7, stdout);
		fflush(stdout);
		fprintf(stderr, "bad");
		fflush(stderr);
		return 1;
	}

	printf("one\n");
	fflush(stdout);
	fprintf(stderr, "two\n");
	fflush(stderr);
	szDirectory.Init();
	cFileUtil.CurrentDirectory(&szDirectory);
	printf("directory %s\n", szDirectory.Text());
	szDirectory.Kill();
	for (i = 1; i < argc; i++)
	{
		printf("argument %s\n", argv[i]);
	}
	printf("three");
	fflush(stdout);
	return (argc >= 1) ? atoi(argv[0]) : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The output and errors are one stream of lines; the last line needn't
// end.  Arguments with spaces are one argument each.  The program runs in
// the directory it is given and its exit code is kept.
//
//////////////////////////////////////////////////////////////////////////
void TestProcessOutput(void)
{
	CProcess		cProcess;
	CArrayChars		aszArguments;
	CArrayChars		aszLines;
	CChars			szExecutable;
	CChars			szDirectory;
	CChars			szExpected;
	CFileUtil		cFileUtil;

	szExecutable.Init();
	AssertTrue(GetThisExecutable(&szExecutable));
	cFileUtil.MakeDir("Output");
	szDirectory.Init("Output");
	cFileUtil.FullPath(&szDirectory);

	aszArguments.Init();
	aszArguments.Add("--test-process-output");
	aszArguments.Add("3");
	aszArguments.Add("a b");
	aszLines.Init();
	cProcess.Init();
	AssertTrue(cProcess.Start(szExecutable.Text(), &aszArguments, szDirectory.Text()));
	AssertTrue(cProcess.IsRunning());
	AssertFalse(cProcess.Start(szExecutable.Text(), &aszArguments, szDirectory.Text()));
	cProcess.Wait(&aszLines);
	AssertFalse(cProcess.IsRunning());
	AssertFalse(cProcess.Poll(&aszLines));
	AssertInt(3, cProcess.GetExitCode());

	szExpected.Init("directory ");
	szExpected.Append(szDirectory);
	AssertSize(5, aszLines.NumElements());
	AssertString("one", aszLines.GetText(0));
	AssertString("two", aszLines.GetText(1));
	AssertString(szExpected.Text(), aszLines.GetText(2));
	AssertString("argument a b", aszLines.GetText(3));
	AssertString("three", aszLines.GetText(4));
	szExpected.Kill();
	cProcess.Kill();

	//Once it has exited another can be started.
	aszLines.ReInit();
	cProcess.Init();
	aszArguments.ReInit();
	aszArguments.Add("--test-process-output");
	aszArguments.Add("0");
	AssertTrue(cProcess.Start(szExecutable.Text(), &aszArguments));
	cProcess.Wait(&aszLines);
	AssertInt(0, cProcess.GetExitCode());
	AssertSize(4, aszLines.NumElements());
	AssertTrue(cProcess.Start(szExecutable.Text(), &aszArguments));
	cProcess.Wait(&aszLines);
	AssertSize(8, aszLines.NumElements());
	cProcess.Kill();

	aszLines.Kill();
	aszArguments.Kill();
	szDirectory.Kill();
	szExecutable.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Output is read while the program runs, without waiting for it to exit,
// and a program can be stopped.
//
//////////////////////////////////////////////////////////////////////////
void TestProcessTerminate(void)
{
	CProcess		cProcess;
	CArrayChars		aszArguments;
	CArrayChars		aszLines;
	CChars			szExecutable;
	int				i;
	bool			bRunning;

	szExecutable.Init();
	AssertTrue(GetThisExecutable(&szExecutable));
	aszArguments.Init();
	aszArguments.Add("--test-process-output");
	aszArguments.Add("wait");
	aszLines.Init();
	cProcess.Init();
	AssertTrue(cProcess.Start(szExecutable.Text(), &aszArguments));

	//Its line arrives while it sleeps.
	bRunning = true;
	for (i = 0; (i < 1000) && (aszLines.NumElements() == 0) && bRunning; i++)
	{
		bRunning = cProcess.Poll(&aszLines);
		ThreadSleep(10);
	}
	AssertTrue(bRunning);
	AssertSize(1, aszLines.NumElements());
	AssertString("waiting", aszLines.GetText(0));

	cProcess.Terminate();
	cProcess.Wait(&aszLines);
	AssertFalse(cProcess.IsRunning());
	AssertTrue(cProcess.GetExitCode() != 0);
	cProcess.Kill();

	//Killing a process stops its program.
	cProcess.Init();
	AssertTrue(cProcess.Start(szExecutable.Text(), &aszArguments));
	cProcess.Kill();
	Pass();

	aszLines.Kill();
	aszArguments.Kill();
	szExecutable.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Captured, the output is kept byte for byte and the errors apart from
// it.  A process can capture one program and read the lines of the next.
//
//////////////////////////////////////////////////////////////////////////
void TestProcessCapture(void)
{
	CProcess		cProcess;
	CArrayChars		aszArguments;
	CArrayChars		aszLines;
	CChars			szExecutable;
	CChars			szOutput;
	CChars			szErrors;

	szExecutable.Init();
	AssertTrue(GetThisExecutable(&szExecutable));
	aszArguments.Init();
	aszArguments.Add("--test-process-output");
	aszArguments.Add("bytes");
	szOutput.Init();
	szErrors.Init();
	cProcess.Init();
	AssertTrue(cProcess.StartCapture(szExecutable.Text(), &aszArguments));
	AssertTrue(cProcess.IsRunning());
	AssertFalse(cProcess.StartCapture(szExecutable.Text(), &aszArguments));
	cProcess.WaitCapture(&szOutput, &szErrors);
	AssertFalse(cProcess.IsRunning());
	AssertFalse(cProcess.PollCapture(&szOutput, &szErrors));
	AssertInt(1, cProcess.GetExitCode());
	AssertSize(7, szOutput.Length());
	AssertMemory("a\0b\r\nc\n", szOutput.Text(), 7);
	AssertString("bad", szErrors.Text());

	//The next program's output and errors are lines again.
	aszLines.Init();
	aszArguments.ReInit();
	aszArguments.Add("--test-process-output");
	aszArguments.Add("0");
	AssertTrue(cProcess.Start(szExecutable.Text(), &aszArguments));
	cProcess.Wait(&aszLines);
	AssertSize(4, aszLines.NumElements());
	AssertString("two", aszLines.GetText(1));

	//And captured again, its errors don't go into its output.
	szOutput.Clear();
	szErrors.Clear();
	AssertTrue(cProcess.StartCapture(szExecutable.Text(), &aszArguments));
	cProcess.WaitCapture(&szOutput, &szErrors);
	AssertInt(0, cProcess.GetExitCode());
	AssertTrue(szOutput.StartsWith("one"));
	AssertTrue(szOutput.EndsWith("three"));
	AssertTrue(szErrors.StartsWith("two"));
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, szOutput.Find("two"));
	cProcess.Kill();

	aszLines.Kill();
	szErrors.Kill();
	szOutput.Kill();
	aszArguments.Kill();
	szExecutable.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A program that isn't there can't be started on Windows; on Linux it is
// started and exits with 127, as a shell's does.
//
//////////////////////////////////////////////////////////////////////////
void TestProcessMissing(void)
{
	CProcess		cProcess;
	CArrayChars		aszLines;
	SLogConfig		sLogConfig;

	aszLines.Init();
	cProcess.Init();
#ifdef _WIN32
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cProcess.Start("no-such-program-for-test-process.exe", NULL));
	gcLogger.SetConfig(&sLogConfig);
	AssertFalse(cProcess.IsRunning());
#else
	AssertTrue(cProcess.Start("no-such-program-for-test-process", NULL));
	cProcess.Wait(&aszLines);
	AssertInt(127, cProcess.GetExitCode());
#endif
	cProcess.Kill();
	aszLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestProcess(void)
{
	BeginTests();

	TestProcessOutput();
	TestProcessTerminate();
	TestProcessCapture();
	TestProcessMissing();

	TestStatistics();
}

