# Says where the compiler is with the conformance cases (TestCppCompilerLib's TestCPPConformance), and can write
# that into the test as what is expected.
#
#   python lists.py              the lists of cases not done, for each stage and target, and how many are done
#   python lists.py -why         also why each case isn't done (the compiler's errors, or what the program returned)
#   python lists.py -write       also writes the lists into TestCPPConformance.cpp, after a stage has moved cases
#
# It runs the Release x64 TestCppCompilerLib.exe with TEST_CPP_CONFORMANCE_LISTS set (the lists are printed and not
# checked) and stops it once the conformance test is done.  Look at what changed in TestCPPConformance.cpp before
# committing it: a case that left a list should be one the stage meant to do.
import os, re, subprocess, sys, tempfile, time

HERE = os.path.dirname(os.path.abspath(__file__))
PROJECT = os.path.join(HERE, "..", "TestCppCompilerLib")
EXE = os.path.join(HERE, "..", "x64", "Release", "TestCppCompilerLib.exe")
TEST = os.path.join(PROJECT, "TestCPPConformance.cpp")
LOG = os.path.join(PROJECT, "Log.txt")

env = dict(os.environ, TEST_CPP_CONFORMANCE_LISTS="1")
if "-why" in sys.argv:
    env["TEST_CPP_CONFORMANCE_VERBOSE"] = "1"
if os.path.exists(LOG):
    os.remove(LOG)
with tempfile.TemporaryFile() as errors:
    process = subprocess.Popen([EXE], cwd=PROJECT, env=env, stdout=subprocess.DEVNULL, stderr=errors)
    while process.poll() is None:
        # The test after the conformance test has started when its name is in the log.
        if os.path.exists(LOG) and "TestIR.cpp" in open(LOG, errors="replace").read():
            process.kill()
            break
        time.sleep(0.5)
    process.wait()
    errors.seek(0)
    lines = errors.read().decode("utf-8", errors="replace").splitlines()

lists = {}
done = {}
total = {}
for line in lines:
    m = re.match(r'(\w+) (65C64|W65C816) (\d+) of (\d+): "(.*)"$', line)
    if m:
        lists[(m.group(1), m.group(2))] = m.group(5)
        done[m.group(2)] = done.get(m.group(2), 0) + int(m.group(3))
        total[m.group(2)] = total.get(m.group(2), 0) + int(m.group(4))
        print(line)
    elif "-why" in sys.argv and re.match(r"(65C64|W65C816) \w+\([CWA]\)", line):
        print("    " + line)
if not lists:
    sys.exit("The conformance test printed nothing: is TestCppCompilerLib built (Release x64)?")
for target in sorted(done):
    print("%s: %d of %d done" % (target, done[target], total[target]))

if "-write" in sys.argv:
    def group(m):
        a = lists[(m.group(1), "65C64")]
        b = lists[(m.group(1), "W65C816")]
        if a == "" and b == "":
            return 'CONFORMANCE_GROUP(%s, "", ""),' % m.group(1)
        return 'CONFORMANCE_GROUP(%s,\n\t\t"%s",\n\t\t"%s"),' % (m.group(1), a, b)

    text = open(TEST, "rb").read().decode("utf-8")
    text, count = re.subn(r'CONFORMANCE_GROUP\((\w+),\s*"[^"]*",\s*"[^"]*"\),', group, text)
    open(TEST, "wb").write(text.encode("utf-8"))
    print("%d lists written to TestCPPConformance.cpp" % (2 * count))
