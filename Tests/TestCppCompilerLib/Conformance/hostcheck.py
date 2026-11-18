# Says what the conformance cases (TestCppCompilerLib's TestCPPConformance) should do, with the Microsoft compiler.
#
#   set TEST_CPP_CONFORMANCE_EXPORT=<directory>      then run TestCppCompilerLib.exe (it can be stopped once
#                                                    TestCPPConformance.cpp is in its Log.txt)
#   python hostcheck.py <directory> [name...]
#
# The test writes each case as <Group>.<Name>.cpp and lists them in Cases.txt with their values (-1 for a program
# that must be refused).  Each is compiled here as C++17: one that must be refused must not compile, and the others
# must return their value.  What is printed is what is wrong with the cases, not with the compiler they test.
# /Zc:lambda is MSVC's conforming lambda processor: without it a lambda can't use a constant it doesn't capture.
import os, shutil, subprocess, sys

VCVARS = r"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"

if len(sys.argv) < 2:
    sys.exit("hostcheck.py <directory of exported cases> [name...]")
directory = os.path.abspath(sys.argv[1])
only = sys.argv[2:]

out = subprocess.run('cmd /c ""%s" >nul && set"' % VCVARS, capture_output=True, text=True, shell=True).stdout
env = dict(line.split("=", 1) for line in out.splitlines() if "=" in line)
cl = shutil.which("cl", path=env.get("PATH") or env.get("Path"))
if not cl:
    sys.exit("The Microsoft compiler was not found (%s)." % VCVARS)

os.chdir(directory)
checked = 0
wrong = 0
for line in open("Cases.txt"):
    name, value = line.split()
    value = int(value)
    if only and name not in only and name.split(".")[1] not in only:
        continue
    checked += 1
    r = subprocess.run([cl, "/nologo", "/std:c++17", "/W3", "/EHsc", "/permissive-", "/Zc:lambda", name + ".cpp", "/Fe:" + name + ".exe", "/Fo:" + name + ".obj"], capture_output=True, text=True, env=env)
    if value == -1:
        if r.returncode == 0:
            print(name, "must be refused, and compiles")
            wrong += 1
        continue
    if r.returncode != 0:
        errors = [e[e.find("error"):][:160] for e in (r.stdout + r.stderr).splitlines() if "error" in e]
        print(name, "does not compile:", " | ".join(errors[:2]))
        wrong += 1
        continue
    returned = subprocess.run([os.path.join(directory, name + ".exe")], capture_output=True, timeout=20).returncode
    if returned != value:
        print(name, "returns", returned, "not", value)
        wrong += 1
print("%d cases checked, %d wrong" % (checked, wrong))
