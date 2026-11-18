# RunParallel.py: TestCppCompilerLib's files run as processes side by side (Tests/TestSpeedPlan.txt step 1), then their
# logs added up into one Log.txt with the same sections and totals one process writes.
#
#   python RunParallel.py [-exe <TestCppCompilerLib.exe>] [-jobs N] [-config Release|Debug] [-platform x64|Win32]
#
# The exe's -list says the files in the order they run and which can be run in parts; the long ones run in parts (-part
# p/n, or by the calls' times from the last run: -calls), the short ones together in one process, the longest first, at most -jobs at once (4 unless given), and none
# started while free memory is under 2 GB.  Each process writes its own log (TESTLIB_LOG, <Job>Log.txt, kept for
# looking at).  The exit code is the failed tests' count as the exe's is, or 1000 if a process didn't finish.  Each file's
# time, and each call's of the files run in parts (TEST_PART_TIMES), is kept in Output/RunParallel.times, for the next run.
import ctypes, json, os, re, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
WORK = os.path.join(HERE, "TestCppCompilerLib")
TIMES = os.path.join(WORK, "Output", "RunParallel.times")
MIN_FREE = 2 << 30
#Release|x64 seconds per file (2026-11-18), until a run has kept its own.
DEFAULT_TIMES = {"TestCPPW65C816": 309, "TestIRW65C816Backend": 83, "TestCPPConformance": 73, "TestCPP65Generator": 72,
                 "TestCPP65IR": 28, "TestCPP65ForwardingReferences": 15, "TestCPP65Spill": 15, "TestCPPOverloads": 6,
                 "TestCPP65HeaderCache": 6, "TestCPP65Jobs": 4, "TestCPPW65C816Assembly": 4, "TestCPPW65C816Interprocedural": 4,
                 "TestIR65Backend": 3}
SMALL = 10		#Files expected to take less (seconds) run together in one process.


def Option(aszArgs, szName, szDefault):
	if szName in aszArgs:
		i = aszArgs.index(szName)
		szValue = aszArgs[i + 1]
		del aszArgs[i:i + 2]
		return szValue
	return szDefault


def FreeMemory():
	#Bytes of physical memory free.
	if os.name == "nt":
		class MEMORYSTATUSEX(ctypes.Structure):
			_fields_ = [("dwLength", ctypes.c_ulong), ("dwMemoryLoad", ctypes.c_ulong), ("ullTotalPhys", ctypes.c_ulonglong),
						("ullAvailPhys", ctypes.c_ulonglong), ("ullTotalPageFile", ctypes.c_ulonglong), ("ullAvailPageFile", ctypes.c_ulonglong),
						("ullTotalVirtual", ctypes.c_ulonglong), ("ullAvailVirtual", ctypes.c_ulonglong), ("ullAvailExtendedVirtual", ctypes.c_ulonglong)]
		sStatus = MEMORYSTATUSEX()
		sStatus.dwLength = ctypes.sizeof(MEMORYSTATUSEX)
		ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(sStatus))
		return sStatus.ullAvailPhys
	for szLine in open("/proc/meminfo"):
		if szLine.startswith("MemAvailable:"):
			return int(szLine.split()[1]) * 1024
	return MIN_FREE


def ShareCalls(asCalls, iParts):
	#The calls (number: seconds) shared out into iParts lists of about the same time: the longest first, each to the part
	#with least so far.
	aaiParts = [[] for p in range(iParts)]
	afTimes = [0.0] * iParts
	for szCall, fTime in sorted(asCalls.items(), key=lambda c: -c[1]):
		p = afTimes.index(min(afTimes))
		aaiParts[p].append(int(szCall))
		afTimes[p] += fTime
	return [(sorted(a), f) for a, f in zip(aaiParts, afTimes)]


def Jobs(aFiles, asTimes, iJobs):
	#(label, arguments, expected seconds) for each process, longest first: a long file in parts, the short ones together.
	aJobs = []
	aszSmall = []
	fSmall = 0.0
	asFileTimes = asTimes.get("files", {})
	asCallTimes = asTimes.get("calls", {})
	for szName, bParts in aFiles:
		fTime = asFileTimes.get(szName, DEFAULT_TIMES.get(szName, 1))
		iParts = max(1, min(iJobs, int(fTime / 40 + 0.5))) if bParts else 1
		if (iParts > 1) and asCallTimes.get(szName):
			for p, (aiCalls, fPart) in enumerate(ShareCalls(asCallTimes[szName], iParts)):
				aJobs.append(("%sPart%d" % (szName, p), ["-only", szName, "-calls", ",".join(str(i) for i in aiCalls)], fPart))
		elif iParts > 1:
			for p in range(iParts):
				aJobs.append(("%sPart%d" % (szName, p), ["-only", szName, "-part", "%d/%d" % (p, iParts)], fTime / iParts))
		elif fTime >= SMALL:
			aJobs.append((szName, ["-only", szName], fTime))
		else:
			aszSmall.append(szName)
			fSmall += fTime
	if aszSmall:
		aJobs.append(("Small", ["-only", ",".join(aszSmall)], fSmall))
	aJobs.sort(key=lambda j: -j[2])
	return aJobs


def ReadLog(szPath):
	#Each file's counts and time ({name: [run, passed, failed, seconds]}), the failures' text, and whether it finished.
	asFiles = {}
	aszFailures = []
	if not os.path.exists(szPath):
		return asFiles, aszFailures, False
	szText = open(szPath, encoding="utf-8", errors="replace").read()
	for m in re.finditer(r"^-{15} (.*?) -{15}\n(.*?)(?=^-{15}|\Z)", szText, re.M | re.S):
		szName = os.path.splitext(os.path.basename(m.group(1).replace("\\", "/")))[0]
		szBody = m.group(2)
		aiCounts = [int(x) for x in [re.search(p, szBody, re.M).group(1) if re.search(p, szBody, re.M) else 0 for p in (r"^Tests Run: (\d+)", r"^Passed: (\d+)", r"^Failed: (\d+)$")]]
		mTime = re.search(r"^Time:\s*([0-9.]+)s", szBody, re.M)
		asFiles[szName] = aiCounts + [float(mTime.group(1)) if mTime else 0.0]
		aszFailures += [f.group(0) for f in re.finditer(r"^Failed: [A-Za-z]:.*?(?=^Failed: [A-Za-z]:|^Tests Run:|\Z)", szBody, re.M | re.S)]
	return asFiles, aszFailures, re.search(r"^Total Tests Run: \d+", szText, re.M) is not None


def main():
	aszArgs = sys.argv[1:]
	szConfig = Option(aszArgs, "-config", "Release")
	szPlatform = Option(aszArgs, "-platform", "x64")
	szExe = Option(aszArgs, "-exe", os.path.join(HERE, szPlatform if szPlatform == "x64" else "", szConfig, "TestCppCompilerLib.exe"))
	iJobs = int(Option(aszArgs, "-jobs", "4"))
	szExe = os.path.abspath(szExe)

	szList = subprocess.run([szExe, "-list"], cwd=WORK, capture_output=True, text=True).stdout
	aFiles = [(l.split()[0], l.split()[1] == "1") for l in szList.splitlines() if l.strip()]
	asTimes = json.load(open(TIMES)) if os.path.exists(TIMES) else {}
	if "files" not in asTimes:
		asTimes = {"files": asTimes, "calls": {}}
	aJobs = Jobs(aFiles, asTimes, iJobs)

	fStart = time.time()
	aRunning = []
	asDone = {}
	aWaiting = list(aJobs)
	while aWaiting or aRunning:
		for sRun in list(aRunning):
			if sRun[1].poll() is not None:
				aRunning.remove(sRun)
				sRun[3].close()
				asDone[sRun[0][0]] = (sRun[1].returncode, time.time() - sRun[2])
				print("%-34s exit %d, %5.0fs" % (sRun[0][0], sRun[1].returncode, time.time() - sRun[2]), flush=True)
		while aWaiting and (len(aRunning) < iJobs) and (not aRunning or FreeMemory() >= MIN_FREE):
			sJob = aWaiting.pop(0)
			szLog = sJob[0] + "Log.txt"
			if os.path.exists(os.path.join(WORK, szLog)):
				os.remove(os.path.join(WORK, szLog))
			asEnv = dict(os.environ)
			asEnv["TESTLIB_LOG"] = szLog
			asEnv["TEST_PART_TIMES"] = "1"
			pcOut = open(os.path.join(WORK, sJob[0] + "Out.txt"), "w")
			aRunning.append((sJob, subprocess.Popen([szExe] + sJob[1], cwd=WORK, env=asEnv, stdout=pcOut, stderr=subprocess.STDOUT), time.time(), pcOut))
		time.sleep(0.5)

	#The logs added up, in the files' order.
	asTotals = {}
	aszFailures = []
	aszUnfinished = []
	for sJob in aJobs:
		asFiles, aszJobFailures, bFinished = ReadLog(os.path.join(WORK, sJob[0] + "Log.txt"))
		if not bFinished:
			aszUnfinished.append("%s (exit %d)" % (sJob[0], asDone[sJob[0]][0]))
		aszFailures += aszJobFailures
		for szName, aCounts in asFiles.items():
			aTotal = asTotals.setdefault(szName, [0, 0, 0, 0.0])
			for i in range(4):
				aTotal[i] += aCounts[i]
	szLog = "-----------------------------------------------------------\n- Log File: [Log.txt]\n-----------------------------------------------------------\n\n"
	for szName, bParts in aFiles:
		if szName in asTotals:
			aTotal = asTotals[szName]
			szLog += "--------------- %s ---------------\n" % os.path.join(WORK, szName + ".cpp")
			szLog += "".join(f for f in aszFailures if os.path.basename(f.split(" line ")[0].split(": ", 1)[1]) == szName + ".cpp")
			szLog += "Tests Run: %d\nPassed: %d\nFailed: %d\nTime: %7.2fs\n\n" % tuple(aTotal)
	iRun = sum(a[0] for a in asTotals.values())
	iPassed = sum(a[1] for a in asTotals.values())
	iFailed = sum(a[2] for a in asTotals.values())
	szLog += "------------------------------ Total Results ------------------------------\nTotal Tests Run: %d\nTotal Passed: %d\nTotal Failed: %d\n" % (iRun, iPassed, iFailed)
	for szUnfinished in aszUnfinished:
		szLog += "Did not finish: %s\n" % szUnfinished
	open(os.path.join(WORK, "Log.txt"), "w", encoding="utf-8").write(szLog)

	#The times, kept for the next run: each file's, and each call's of those run in parts.
	asCalls = {}
	for sJob in aJobs:
		szOut = os.path.join(WORK, sJob[0] + "Out.txt")
		for m in re.finditer(r"^TESTPART (\S+) (\d+) ([0-9.]+) \d+$", open(szOut, errors="replace").read() if os.path.exists(szOut) else "", re.M):
			asCalls.setdefault(m.group(1), {})[m.group(2)] = float(m.group(3))
	if not aszUnfinished:
		os.makedirs(os.path.dirname(TIMES), exist_ok=True)
		json.dump({"files": {k: round(v[3], 1) for k, v in asTotals.items()}, "calls": asCalls}, open(TIMES, "w"), indent=0, sort_keys=True)
	print("%d run, %d failed, in %.0fs with %d jobs at once%s" % (iRun, iFailed, time.time() - fStart, iJobs, ("; did not finish: " + ", ".join(aszUnfinished)) if aszUnfinished else ""))
	sys.exit(1000 if aszUnfinished else iFailed)


if __name__ == "__main__":
	main()
