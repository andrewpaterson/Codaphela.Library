# Sweep.py: the test sweep (Tests/TestSpeedPlan.txt step 5): every solution on the list built and its suites run, in
# Release|x64, Release|Win32 and Debug|x64, and Debug|Win32 for a solution that failed in a Release configuration.
#
#   python Sweep.py [-only <text>,...] [-configs Release|x64,...] [-skip <suite>,...] [-new] [-overlap]
#
# An item is a solution in one configuration: built (MSBuild -m:2), then its suites run, one item after another.  With
# -overlap a builder and a runner work side by side: while one item's suites run the next item builds, at most one
# ahead, and only with BUILD_FREE free, never beside the EXCLUSIVE suites (a build beside TestHCP65Lib ran this 7.9 GB
# machine out of memory, 2026-11-19).  Within a configuration the items whose suites took longest last time go first,
# so with -overlap the short ones' builds hide behind them.  Each item's result is a
# line in Output/SweepResults.txt; an item already there isn't done again, so a stopped sweep goes on where it was
# (-new starts again).  While Output/Sweep.pause exists the sweep waits before its next build and its next run, so
# another session can have the machine between items.  -only runs the items whose name has one of the texts in it; -skip leaves those suites out (their solutions are still
# built), saying so in the item's result.
import os, re, subprocess, sys, threading, time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
OUTPUT = os.path.join(HERE, "Output")
RESULTS = os.path.join(OUTPUT, "SweepResults.txt")
PAUSE = os.path.join(OUTPUT, "Sweep.pause")
MSBUILD = r"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
RUN_PARALLEL = os.path.join(HERE, "TestCppCompilerLib", "RunParallel.py")
BUILD_FREE = 3 << 30		#A build alongside a run needs this much memory free...
RUN_FREE = 2 << 30			#...and a run this much.
EXCLUSIVE = ["TestCppCompilerLib", "TestHCP65Lib"]	#Suites no build runs beside (RunParallel.py's four processes; the kernal's).
RELEASE = ["Release|x64", "Release|Win32"]
CONFIGS = RELEASE + ["Debug|x64"]
CONDITIONAL = "Debug|Win32"	#Only for a solution whose build or suite failed in a Release configuration (the user, 2026-11-18).
#Seconds the longest solutions' suites take (2026-11-19, Release|x64), for the order until a sweep has kept its own.
DEFAULT_SECONDS = {"HCP65Emulator.sln": 1580, "TestSupportLib65.sln": 1130, "TestBaseLib65.sln": 350, "TestStandardLib65.sln": 360,
				   "TestCppCompilerLib.sln": 165}

sys.path.insert(0, os.path.dirname(RUN_PARALLEL))
from RunParallel import FreeMemory


#The list of what is tested (the user's, memory: test-sweep-howto): (solution, suites, configurations).  A suite is
#(name, the directory it runs in, its exe's name, its arguments).  A configuration list of None is every one.
def Suite(szRepo, szTest, aszArgs=None):
	return (szTest, os.path.join(ROOT, szRepo, "Tests", szTest, szTest), szTest, aszArgs or [])


def Items():
	aItems = []
	for szTest in ["TestBaseLib", "TestStandardLib", "TestSupportLib", "TestMicroprocessorLib", "TestInputLib", "TestThreadLib",
				   "TestWindowLib", "TestGerberLib", "TestJavaParserLib", "TestCppCompilerLib", "TestVulkanLib", "TestSceneViewLib"]:
		aszArgs = ["--test", "param1", "--param:2"] if szTest == "TestThreadLib" else []
		aItems.append((os.path.join(ROOT, "CommonLibraries", "Tests", szTest, szTest + ".sln"), [Suite("CommonLibraries", szTest, aszArgs)], None))
	#The *65 suites: Release|x64 only (the user, 2026-11-08), on both processors (2026-11-19).
	for szTest in ["TestBaseLib65", "TestStandardLib65", "TestSupportLib65"]:
		sSuite = Suite("CommonLibraries", szTest)
		aItems.append((os.path.join(ROOT, "CommonLibraries", "Tests", szTest, szTest + ".sln"),
					   [sSuite, (szTest + " 816", sSuite[1], szTest, ["816"])], ["Release|x64"]))
	for szRepo, szTest in [("LogiCim", "TestLogicimLib"), ("LogiCim", "TestKicadImporterLib"), ("HCP65", "TestAssembler65Lib"), ("HCP65", "TestLibc65"),
						   ("HCP65-Sculpt", "TestSculptLib")]:
		aItems.append((os.path.join(ROOT, szRepo, "Tests", szTest, szTest + ".sln"), [Suite(szRepo, szTest)], None))
	aItems.append((os.path.join(ROOT, "HCP65-IDE", "HCP65IDE", "HCP65IDE.sln"),
				   [Suite("HCP65-IDE", szTest) for szTest in ["TestBuildLib", "TestGitLib", "TestClaudeLib", "TestVisualStudioLib", "TestTemplateLib"]], None))
	#TestHCP65Lib runs in Release|x64 only (the user, 2026-11-19); the other configurations only build the emulator.
	aItems.append((os.path.join(ROOT, "HCP65", "HCP65-Emulator", "HCP65Emulator.sln"),
				   [("TestHCP65Lib", os.path.join(ROOT, "HCP65", "HCP65-Emulator", "TestHCP65Lib", "TestHCP65Lib"), "TestHCP65Lib", [], ["Release|x64"])], None))
	for szSolution in [r"LogiCim\LogiCim\LogiCim.sln", r"HCP65\HCP65-Tester\HCP65Tester.sln", r"WindowInputTester\WindowInputTester.sln",
					   r"HCP65-Sculpt\HCP65Sculpt\HCP65Sculpt.sln"]:
		aItems.append((os.path.join(ROOT, szSolution), [], None))
	return aItems


def Key(szSolution, szConfig):
	return "%s|%s" % (os.path.basename(szSolution), szConfig)


def Done():
	#What SweepResults.txt has: each item's line, and each suite's time (for the order).
	asLines = {}
	if os.path.exists(RESULTS):
		for szLine in open(RESULTS, encoding="utf-8"):
			aszParts = szLine.rstrip("\n").split("\t")
			if len(aszParts) >= 3 and not szLine.startswith("#"):
				asLines[aszParts[0]] = aszParts[2]
	return asLines


def Seconds(szResult):
	#The time an item's suites took, from its result line (0 if it has none).
	return sum(int(x) for x in re.findall(r"failed \((\d+)s\)", szResult))


def Record(szKey, szBegan, szText):
	with open(RESULTS, "a", encoding="utf-8") as f:
		f.write("%s\t%s\t%s\n" % (szKey, szBegan, szText))
	print("%s\t%s" % (szKey, szText), flush=True)


def Say(szText):
	print("%s %s" % (time.strftime("%H:%M:%S"), szText), flush=True)


def WaitWhilePaused():
	if os.path.exists(PAUSE):
		Say("paused (%s)" % PAUSE)
		while os.path.exists(PAUSE):
			time.sleep(10)
		Say("going on")


def FindExe(szSolution, szConfig, szName):
	#Where MSBuild put a suite's exe: x64 builds under x64\, Win32 ones not.
	szBuild, szPlatform = szConfig.split("|")
	szDir = os.path.dirname(szSolution)
	for szPath in [os.path.join(szDir, "x64", szBuild, szName + ".exe"), os.path.join(szDir, szBuild, szName + ".exe"),
				   os.path.join(szDir, szName, "x64", szBuild, szName + ".exe"), os.path.join(szDir, szName, szBuild, szName + ".exe")]:
		if os.path.exists(szPath) and ((szPlatform == "x64") == ("\\x64\\" in szPath)):
			return szPath
	return None


class CSweep:
	def __init__(self, aOrder):
		self.aOrder = aOrder				#(key, solution, config, suites) in the order they are done.
		self.cLock = threading.Condition()
		self.aBuilt = []					#Built and waiting to run: (index, build result text, build began, ok).
		self.iNextBuild = 0
		self.iRunning = -1					#The index of the item whose suites run, -1 for none.
		self.bOverlap = False				#Builds may run beside suites (-overlap).
		self.aszSkip = []					#Suites left out (-skip).
		self.bCompilerRunning = False		#An EXCLUSIVE suite is running...
		self.bBuilding = False				#...which waits for a build going on to end first.
		self.asFailed = set()				#Solutions that failed in a Release configuration.
		self.fBegan = time.time()

	def Builder(self):
		while True:
			with self.cLock:
				#At most one item ahead of the runner (none at all without -overlap), and none built beside an EXCLUSIVE suite.
				while self.aBuilt or self.bCompilerRunning or ((not self.bOverlap) and (self.iRunning >= 0)):
					self.cLock.wait()
				if self.iNextBuild >= len(self.aOrder):
					self.aBuilt.append(None)
					self.cLock.notify_all()
					return
				iItem = self.iNextBuild
				self.iNextBuild += 1
				self.bBuilding = True
			WaitWhilePaused()
			while (self.iRunning >= 0) and (FreeMemory() < BUILD_FREE):
				time.sleep(5)
			szKey, szSolution, szConfig, aSuites = self.aOrder[iItem]
			szBuild, szPlatform = szConfig.split("|")
			szBegan = time.strftime("%Y-%m-%d %H:%M:%S")
			fStart = time.time()
			sResult = subprocess.run([MSBUILD, szSolution, "-m:2", "-nodeReuse:false", "-p:Configuration=" + szBuild, "-p:Platform=" + szPlatform,
									  "-v:m", "-nologo"], capture_output=True, text=True, errors="replace")
			szOutput = sResult.stdout + sResult.stderr
			open(os.path.join(OUTPUT, "SweepBuild.log"), "w", encoding="utf-8").write(szOutput)
			aszErrors = sorted(set(re.sub(r" \[.*", "", szLine).strip() for szLine in szOutput.splitlines() if " error " in szLine))
			szText = "build %s (%ds)%s" % ("ok" if sResult.returncode == 0 else "FAILED", time.time() - fStart,
										   (": " + " | ".join(szError[:160] for szError in aszErrors[:3])) if aszErrors else "")
			Say("%s: %s" % (szKey, szText.split(":")[0]))
			with self.cLock:
				self.aBuilt.append((iItem, szText, szBegan, sResult.returncode == 0))
				self.bBuilding = False
				self.cLock.notify_all()

	def RunSuite(self, szSolution, szConfig, sSuite):
		szName, szRunDir, szExe, aszArgs = sSuite[:4]
		szBuild, szPlatform = szConfig.split("|")
		if szName == "TestCppCompilerLib":
			aszCommand = [sys.executable, RUN_PARALLEL, "-config", szBuild, "-platform", szPlatform]
			szRunDir = os.path.dirname(RUN_PARALLEL)
			szLog = os.path.join(szRunDir, "TestCppCompilerLib", "Log.txt")
		else:
			szPath = FindExe(szSolution, szConfig, szExe)
			if not szPath:
				return "%s: no exe" % szName
			aszCommand = [szPath] + aszArgs
			szLog = os.path.join(szRunDir, "Log.txt")
		if os.path.exists(szLog):
			os.remove(szLog)
		while FreeMemory() < RUN_FREE:
			time.sleep(5)
		Say("start %s %s" % (szName, szConfig))
		fStart = time.time()
		sResult = subprocess.run(aszCommand, cwd=szRunDir, capture_output=True, text=True, errors="replace")
		szText = open(szLog, encoding="utf-8", errors="replace").read() if os.path.exists(szLog) else ""
		sRun = re.search(r"Total Tests Run: (\d+)", szText)
		sFailed = re.search(r"Total Failed: (\d+)", szText)
		sFirst = re.search(r"^Failed: ([A-Z]:\S+ line \d+)", szText, re.M)
		szResult = "%s: exit %d, %s run, %s failed (%ds)%s" % (szName, sResult.returncode, sRun.group(1) if sRun else "?", sFailed.group(1) if sFailed else "?",
															   time.time() - fStart, (" first: " + sFirst.group(1)) if sFirst else "")
		Say("end %s" % szResult)
		return szResult

	def Runner(self):
		while True:
			with self.cLock:
				while not self.aBuilt:
					self.cLock.wait()
				sBuilt = self.aBuilt.pop(0)
				if sBuilt is None:
					return
				iItem, szText, szBegan, bBuilt = sBuilt
				self.iRunning = iItem
				self.cLock.notify_all()
			szKey, szSolution, szConfig, aSuites = self.aOrder[iItem]
			aszParts = [szText]
			bFailed = not bBuilt
			if bBuilt:
				for sSuite in aSuites:
					if (len(sSuite) > 4) and (szConfig not in sSuite[4]):
						continue
					if sSuite[0] in self.aszSkip:
						aszParts.append("%s: skipped" % sSuite[0])
						continue
					WaitWhilePaused()
					with self.cLock:
						self.bCompilerRunning = sSuite[0] in EXCLUSIVE
						while self.bCompilerRunning and self.bBuilding:
							self.cLock.wait()
					szResult = self.RunSuite(szSolution, szConfig, sSuite)
					with self.cLock:
						self.bCompilerRunning = False
						self.cLock.notify_all()
					aszParts.append(szResult)
					bFailed |= not re.search(r"exit 0, \d+ run, 0 failed", szResult)
			if bFailed and (szConfig in RELEASE):
				self.asFailed.add(szSolution)
			Record(szKey, szBegan, "; ".join(aszParts))
			with self.cLock:
				self.iRunning = -1
				self.cLock.notify_all()

	def Run(self):
		cBuilder = threading.Thread(target=self.Builder)
		cBuilder.start()
		self.Runner()
		cBuilder.join()


def Order(aItems, aszConfigs, asDone, szOnly):
	#The items to do in each configuration, the longest suites (by their last time) first.
	aOrder = []
	for szConfig in aszConfigs:
		aConfig = []
		for i, (szSolution, aSuites, aszItemConfigs) in enumerate(aItems):
			szKey = Key(szSolution, szConfig)
			if (aszItemConfigs and (szConfig not in aszItemConfigs)) or (szKey in asDone) or (szOnly and not any(szText in szKey for szText in szOnly.split(","))):
				continue
			iLast = max([Seconds(szResult) for szItemKey, szResult in asDone.items() if szItemKey.startswith(os.path.basename(szSolution) + "|")] +
						[DEFAULT_SECONDS.get(os.path.basename(szSolution), 0)])
			aConfig.append((-iLast, i, (szKey, szSolution, szConfig, aSuites)))
		aOrder += [sItem for _, _, sItem in sorted(aConfig)]
	return aOrder


def main():
	aszArgs = sys.argv[1:]
	szOnly = aszArgs[aszArgs.index("-only") + 1] if "-only" in aszArgs else ""
	aszConfigs = aszArgs[aszArgs.index("-configs") + 1].split(",") if "-configs" in aszArgs else CONFIGS
	aszSkip = aszArgs[aszArgs.index("-skip") + 1].split(",") if "-skip" in aszArgs else []
	os.makedirs(OUTPUT, exist_ok=True)
	if ("-new" in aszArgs) and os.path.exists(RESULTS):
		os.rename(RESULTS, RESULTS + time.strftime(".%Y%m%d-%H%M%S"))
	aItems = Items()
	fBegan = time.time()
	cSweep = CSweep(Order(aItems, aszConfigs, Done(), szOnly))
	cSweep.bOverlap = "-overlap" in aszArgs
	cSweep.aszSkip = aszSkip
	Say("%d items%s" % (len(cSweep.aOrder), ", builds beside runs" if cSweep.bOverlap else ""))
	cSweep.Run()

	#Debug|Win32 for the solutions that failed in a Release configuration, this sweep or one it went on from.
	if ("-configs" not in aszArgs):
		asDone = Done()
		asFailed = set(s for s in cSweep.asFailed)
		for szSolution, aSuites, aszItemConfigs in aItems:
			for szConfig in RELEASE:
				szResult = asDone.get(Key(szSolution, szConfig), "")
				if szResult and (("build FAILED" in szResult) or re.search(r"exit (?!0,)|\? run| [1-9]\d* failed", szResult)):
					asFailed.add(szSolution)
		aFailed = [sItem for sItem in aItems if (sItem[0] in asFailed) and not (sItem[2] and (CONDITIONAL not in sItem[2]))]
		if aFailed:
			cSweep = CSweep(Order(aFailed, [CONDITIONAL], asDone, szOnly))
			cSweep.bOverlap = "-overlap" in aszArgs
			cSweep.aszSkip = aszSkip
			Say("%d items in %s" % (len(cSweep.aOrder), CONDITIONAL))
			cSweep.Run()

	#The summary.
	asDone = Done()
	aszFailures = [szKey + ": " + szResult for szKey, szResult in sorted(asDone.items())
				   if ("build FAILED" in szResult) or re.search(r"exit (?!0,)|\? run| [1-9]\d* failed|no exe", szResult)]
	with open(RESULTS, "a", encoding="utf-8") as f:
		f.write("# %s: this run %d s; %d items in all, %d failed\n" % (time.strftime("%Y-%m-%d %H:%M:%S"), time.time() - fBegan, len(asDone), len(aszFailures)))
	Say("done in %d s, %d items, %d failed" % (time.time() - fBegan, len(asDone), len(aszFailures)))
	for szFailure in aszFailures:
		print("  " + szFailure[:300])
	return 1 if aszFailures else 0


if __name__ == "__main__":
	sys.exit(main())
