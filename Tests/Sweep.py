# Sweep.py: the test sweep (Tests/TestSpeedPlan.txt step 5): every solution on the list built and its suites run, in
# Release|x64, Release|Win32 and Debug|x64, and Debug|Win32 for a solution that failed in a Release configuration.
#
#   python Sweep.py [-only <text>,...] [-configs Release|x64,...] [-skip <suite>,...] [-new] [-builds N] [-cores N]
#
# An item is a solution in one configuration: built (MSBuild -m:2), then its suites run.  Each configuration has a
# builder doing its items one after another, and up to -builds of them (3 unless given) build at once: two builds of
# one configuration would share the libraries' intermediate folders (BaseLib\Release\ and so on), two of different
# configurations don't.  A built item's suites run as soon as nothing else is running in their directories (a suite
# writes Log.txt and Output\ where it runs, so one suite's configurations take turns; a *65 suite's are its own and its
# non-65 suite's, which it copies) and the suites running weigh no
# more than -cores (12 unless given; TestCppCompilerLib and the *65 suites weigh 4: they run 4 processes or threads).
# Among the built items the one whose suites took longest last time runs first, and each configuration builds those
# first.  Builds wait for BUILD_FREE memory free and runs for RUN_FREE.  Each item's result is a line in
# Output/SweepResults.txt; an item already there isn't done again, so a stopped sweep goes on where it was (-new starts
# again).  While Output/Sweep.pause exists the sweep starts no build and no suite, so another session can have the
# machine.  -only runs the items whose name has one of the texts in it; -skip leaves those suites out (their solutions
# are still built), saying so in the item's result.  -builds 1 -cores 1 does one thing at a time.
import os, re, subprocess, sys, threading, time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
OUTPUT = os.path.join(HERE, "Output")
RESULTS = os.path.join(OUTPUT, "SweepResults.txt")
PAUSE = os.path.join(OUTPUT, "Sweep.pause")
MSBUILD = r"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
RUN_PARALLEL = os.path.join(HERE, "TestCppCompilerLib", "RunParallel.py")
BUILD_FREE = 3 << 30		#A build needs this much memory free to start...
RUN_FREE = 2 << 30			#...and a suite this much.
WEIGHTS = {"TestCppCompilerLib": 4, "TestBaseLib65": 4, "TestStandardLib65": 4, "TestSupportLib65": 4, "TestHCP65Lib": 2}	#By exe; 1 for the others.
#What a solution calls the projects' platforms, which MSBuild is given: a solution's configurations are names of its
#own (HCP65-IDE's ConfigurationsPlan.txt, C11-2).  The configurations here, and the folders the programs are built
#in, are the projects'.
SOLUTION_PLATFORMS = {"x64": "MSVC-x64", "Win32": "MSVC-x86"}
RELEASE = ["Release|x64", "Release|Win32"]
CONFIGS = RELEASE + ["Debug|x64"]
CONDITIONAL = "Debug|Win32"	#Only for a solution whose build or suite failed in a Release configuration (the user, 2026-11-18).
#Seconds the longest solutions' suites take (Release|x64), for the order until a sweep has kept its own.
DEFAULT_SECONDS = {"HCP65Emulator.sln": 2400, "TestSupportLib65.sln": 385, "TestBaseLib65.sln": 190, "TestStandardLib65.sln": 165,
				   "TestCppCompilerLib.sln": 210}

sys.path.insert(0, os.path.dirname(RUN_PARALLEL))
from RunParallel import FreeMemory


#The list of what is tested (the user's, memory: test-sweep-howto): (solution, suites, configurations).  A suite is
#(name, the directory it runs in, its exe's name, its arguments).  A configuration list of None is every one.
def Suite(szRepo, szTest, aszArgs=None):
	return (szTest, os.path.join(ROOT, szRepo, "Tests", szTest, szTest), szTest, aszArgs or [])


#The *65 suites copy their non-65 suite's directory (TestProject65CopyDirectory), so they don't run beside it.
COPIES = {"TestBaseLib65": "TestBaseLib", "TestStandardLib65": "TestStandardLib", "TestSupportLib65": "TestSupportLib"}	#By exe.


def SuiteDirs(sSuite):
	#The directories a suite works in: the one it runs in, and the one it copies.
	asDirs = set([sSuite[1]])
	if sSuite[2] in COPIES:
		asDirs.add(Suite("CommonLibraries", COPIES[sSuite[2]])[1])
	return asDirs


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
	def __init__(self, aOrder, iBuilds, iCores, aszSkip):
		self.aOrder = aOrder				#(key, solution, config, suites, seconds last time), each configuration's in the order they build.
		self.iBuilds = iBuilds				#Configurations building at once (-builds).
		self.iCores = iCores				#The weight of the suites running at once (-cores).
		self.aszSkip = aszSkip				#Suites left out (-skip).
		self.cLock = threading.Condition()
		self.iBuilding = 0					#Builds going on.
		self.aBuilt = []					#Built and waiting to run: (index, build result text, build began, ok).
		self.asRunDirs = set()				#The directories suites are running in.
		self.iWeight = 0					#The weight of the items running...
		self.iRunning = 0					#...and how many there are.
		self.iFinished = 0					#Items recorded.
		self.asFailed = set()				#Solutions that failed in a Release configuration.

	def Builder(self, aiItems):
		#One configuration's items, one after another.
		for iItem in aiItems:
			with self.cLock:
				while self.iBuilding >= self.iBuilds:
					self.cLock.wait()
				self.iBuilding += 1
			szKey, szSolution, szConfig, aSuites, _ = self.aOrder[iItem]
			szBegan = time.strftime("%Y-%m-%d %H:%M:%S")
			szText, bOk = "build FAILED (sweep error)", False
			try:
				WaitWhilePaused()
				while FreeMemory() < BUILD_FREE:
					time.sleep(5)
				szBuild, szPlatform = szConfig.split("|")
				szBegan = time.strftime("%Y-%m-%d %H:%M:%S")
				fStart = time.time()
				sResult = subprocess.run([MSBUILD, szSolution, "-m:2", "-nodeReuse:false", "-p:Configuration=" + szBuild, "-p:Platform=" + SOLUTION_PLATFORMS[szPlatform],
										  "-v:m", "-nologo"], capture_output=True, text=True, errors="replace")
				szOutput = sResult.stdout + sResult.stderr
				open(os.path.join(OUTPUT, "SweepBuild.%s.log" % szConfig.replace("|", "-")), "w", encoding="utf-8").write(szOutput)
				aszErrors = sorted(set(re.sub(r" \[.*", "", szLine).strip() for szLine in szOutput.splitlines() if " error " in szLine))
				bOk = sResult.returncode == 0
				szText = "build %s (%ds)%s" % ("ok" if bOk else "FAILED", time.time() - fStart,
											   (": " + " | ".join(szError[:160] for szError in aszErrors[:3])) if aszErrors else "")
				Say("%s: %s" % (szKey, szText.split(":")[0]))
			except Exception as e:
				szText = "build FAILED (sweep error: %s)" % e
			finally:
				with self.cLock:
					self.iBuilding -= 1
					self.aBuilt.append((iItem, szText, szBegan, bOk))
					self.cLock.notify_all()

	def Suites(self, iItem, bBuilt):
		#The suites an item runs in its configuration (none if its build failed), and those not skipped.
		szKey, szSolution, szConfig, aSuites, _ = self.aOrder[iItem]
		aRun = [sSuite for sSuite in aSuites if (len(sSuite) <= 4) or (szConfig in sSuite[4])] if bBuilt else []
		return aRun, [sSuite for sSuite in aRun if sSuite[0] not in self.aszSkip]

	def Pick(self):
		#The built item to run next, under the lock: the one whose suites took longest last time that can start now.
		if os.path.exists(PAUSE):
			return None
		bMemory = (self.iRunning == 0) or (FreeMemory() >= RUN_FREE)
		sPick = None
		for sBuilt in self.aBuilt:
			_, aRun = self.Suites(sBuilt[0], sBuilt[3])
			iWeight = max([WEIGHTS.get(sSuite[2], 1) for sSuite in aRun] + [0])
			if aRun and (not bMemory or (self.iRunning and (self.iWeight + iWeight > self.iCores)) or any(SuiteDirs(sSuite) & self.asRunDirs for sSuite in aRun)):
				continue
			if (sPick is None) or (self.aOrder[sBuilt[0]][4] > self.aOrder[sPick[0][0]][4]):
				sPick = (sBuilt, aRun, iWeight)
		return sPick

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
		Say("start %s %s" % (szName, szConfig))
		fStart = time.time()
		sResult = subprocess.run(aszCommand, cwd=szRunDir, capture_output=True, text=True, errors="replace")
		szText = open(szLog, encoding="utf-8", errors="replace").read() if os.path.exists(szLog) else ""
		sRun = re.search(r"Total Tests Run: (\d+)", szText)
		sFailed = re.search(r"Total Failed: (\d+)", szText)
		sFirst = re.search(r"^Failed: ([A-Z]:\S+ line \d+)", szText, re.M)
		szResult = "%s: exit %d, %s run, %s failed (%ds)%s" % (szName, sResult.returncode, sRun.group(1) if sRun else "?", sFailed.group(1) if sFailed else "?",
															   time.time() - fStart, (" first: " + sFirst.group(1)) if sFirst else "")
		Say("end %s %s" % (szConfig, szResult))
		return szResult

	def RunItem(self, sBuilt, aRun, iWeight):
		iItem, szText, szBegan, bBuilt = sBuilt
		szKey, szSolution, szConfig, aSuites, _ = self.aOrder[iItem]
		aszParts = [szText]
		bFailed = not bBuilt
		try:
			for sSuite in self.Suites(iItem, bBuilt)[0]:
				if sSuite[0] in self.aszSkip:
					aszParts.append("%s: skipped" % sSuite[0])
					continue
				WaitWhilePaused()
				szResult = self.RunSuite(szSolution, szConfig, sSuite)
				aszParts.append(szResult)
				bFailed |= not re.search(r"exit 0, \d+ run, 0 failed", szResult)
		except Exception as e:
			aszParts.append("sweep error: %s" % e)
			bFailed = True
		finally:
			with self.cLock:
				if bFailed and (szConfig in RELEASE):
					self.asFailed.add(szSolution)
				Record(szKey, szBegan, "; ".join(aszParts))
				for sSuite in aRun:
					self.asRunDirs -= SuiteDirs(sSuite)
				self.iWeight -= iWeight
				self.iRunning -= 1
				self.iFinished += 1
				self.cLock.notify_all()

	def Run(self):
		aszConfigs = []
		for sItem in self.aOrder:
			if sItem[2] not in aszConfigs:
				aszConfigs.append(sItem[2])
		aBuilders = [threading.Thread(target=self.Builder, args=([i for i, sItem in enumerate(self.aOrder) if sItem[2] == szConfig],)) for szConfig in aszConfigs]
		for cBuilder in aBuilders:
			cBuilder.start()
		aRunners = []
		with self.cLock:
			while self.iFinished < len(self.aOrder):
				sPick = self.Pick()
				if sPick is None:
					self.cLock.wait(5)	#Woken by a build or a run ending; the timeout looks at free memory and the pause again.
					continue
				sBuilt, aRun, iWeight = sPick
				self.aBuilt.remove(sBuilt)
				for sSuite in aRun:
					self.asRunDirs |= SuiteDirs(sSuite)
				self.iWeight += iWeight
				self.iRunning += 1
				cRunner = threading.Thread(target=self.RunItem, args=(sBuilt, aRun, iWeight))
				cRunner.start()
				aRunners.append(cRunner)
		for cThread in aBuilders + aRunners:
			cThread.join()


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
			aConfig.append((-iLast, i, (szKey, szSolution, szConfig, aSuites, iLast)))
		aOrder += [sItem for _, _, sItem in sorted(aConfig)]
	return aOrder


def Option(aszArgs, szName, szDefault):
	return aszArgs[aszArgs.index(szName) + 1] if szName in aszArgs else szDefault


def main():
	aszArgs = sys.argv[1:]
	szOnly = Option(aszArgs, "-only", "")
	aszConfigs = Option(aszArgs, "-configs", ",".join(CONFIGS)).split(",")
	aszSkip = [x for x in Option(aszArgs, "-skip", "").split(",") if x]
	iBuilds = int(Option(aszArgs, "-builds", "3"))
	iCores = int(Option(aszArgs, "-cores", "12"))
	os.makedirs(OUTPUT, exist_ok=True)
	if ("-new" in aszArgs) and os.path.exists(RESULTS):
		os.rename(RESULTS, RESULTS + time.strftime(".%Y%m%d-%H%M%S"))
	aItems = Items()
	fBegan = time.time()
	cSweep = CSweep(Order(aItems, aszConfigs, Done(), szOnly), iBuilds, iCores, aszSkip)
	Say("%d items, %d builds and %d cores at once" % (len(cSweep.aOrder), iBuilds, iCores))
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
			cSweep = CSweep(Order(aFailed, [CONDITIONAL], asDone, szOnly), iBuilds, iCores, aszSkip)
			Say("%d items in %s" % (len(cSweep.aOrder), CONDITIONAL))
			cSweep.Run()

	#The summary.
	asDone = Done()
	aszFailures = [szKey + ": " + szResult for szKey, szResult in sorted(asDone.items())
				   if ("build FAILED" in szResult) or re.search(r"exit (?!0,)|\? run| [1-9]\d* failed|no exe|sweep error", szResult)]
	with open(RESULTS, "a", encoding="utf-8") as f:
		f.write("# %s: this run %d s; %d items in all, %d failed\n" % (time.strftime("%Y-%m-%d %H:%M:%S"), time.time() - fBegan, len(asDone), len(aszFailures)))
	Say("done in %d s, %d items, %d failed" % (time.time() - fBegan, len(asDone), len(aszFailures)))
	for szFailure in aszFailures:
		print("  " + szFailure[:300])
	return 1 if aszFailures else 0


if __name__ == "__main__":
	sys.exit(main())
