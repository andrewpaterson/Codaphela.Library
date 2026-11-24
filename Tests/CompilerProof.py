"""Proves that a change to the C++ compiler (CppCompilerLib) left what it writes and what it says as they were.

    python CompilerProof.py listings <directory>
        Compiles each of the three *65 suites' projects for each processor, a unit at a time and nothing run, and keeps
        every unit's listing in <directory>/<Suite>_<Processor>.  The suites must be built (Release x64 on Windows; on
        Linux the CMake builds in Codaphela/build/<Suite>, 32 bit Release).
    python CompilerProof.py compare <directory> <directory>
        Two runs of "listings", file by file: the listings that differ, and those only one has.
    python CompilerProof.py diagnostics <file>
        Runs TestCPPConformance alone with TEST_CPP_CONFORMANCE_DIAGNOSTICS: what the compiler says of every conformance
        case (its errors and its warnings), built for each processor and checked by the semantic pass, is kept in <file>.
        TestCppCompilerLib must be built, as the suites are.
    python CompilerProof.py said <file> <file>
        Two runs of "diagnostics", case by case: the cases only one has, and those of which something else is said.

Listings are compared from one folder: a program has its sources' paths in it (__FILE__), so two checkouts' listings
differ where nothing else does.  The machine these were written on has 7.9 GB: "listings" runs one compile at a time,
and is not run beside TestCppCompilerLib.  SemanticLayerPlan.txt in CppCompilerLib says what they are for.
"""
import filecmp
import io
import os
import shutil
import subprocess
import sys
import time

TESTS = os.path.dirname(os.path.abspath(__file__))
SUITES = ['TestBaseLib65', 'TestStandardLib65', 'TestSupportLib65']
PROCESSORS = [('65C64', []), ('W65C816', ['816'])]


def executable(project):
    if os.name == 'nt':
        return os.path.join(TESTS, project, 'x64', 'Release', project + '.exe')
    return os.path.join(TESTS, '..', '..', 'build', project, project)


def listings(directory):
    directory = os.path.abspath(directory)
    for suite in SUITES:
        for processor, arguments in PROCESSORS:
            out = os.path.join(directory, suite + '_' + processor)
            shutil.rmtree(out, ignore_errors=True)
            os.makedirs(out)
            env = dict(os.environ, TEST_PROJECT65_LISTINGS=out, TEST_PROJECT65_ONLY='NoSuchTest', TEST_PROJECT65_SERIAL='1',
                       TESTLIB_LOG=os.path.join(directory, suite + '_' + processor + '_Log.txt'))
            exe = executable(suite)
            start = time.time()
            with open(os.path.join(directory, suite + '_' + processor + '_out.txt'), 'w') as said:
                code = subprocess.run([exe] + arguments, cwd=os.path.join(TESTS, suite, suite), env=env, stdout=said, stderr=subprocess.STDOUT).returncode
            print('%s %s exit %d, %d listings, %d s' % (suite, processor, code, len(os.listdir(out)), time.time() - start), flush=True)


def compare(first, second):
    total = different = missing = 0
    folders = sorted(name for name in os.listdir(first) if os.path.isdir(os.path.join(first, name)))
    for folder in folders:
        names = set(os.listdir(os.path.join(first, folder)))
        if os.path.isdir(os.path.join(second, folder)):
            names |= set(os.listdir(os.path.join(second, folder)))
        for name in sorted(names):
            total += 1
            a, b = os.path.join(first, folder, name), os.path.join(second, folder, name)
            if not (os.path.exists(a) and os.path.exists(b)):
                missing += 1
                print('in one only: %s %s' % (folder, name))
            elif not filecmp.cmp(a, b, shallow=False):
                different += 1
                if different <= 40:
                    print('different: %s %s' % (folder, name))
    print('%d listings, %d different, %d in one only' % (total, different, missing))
    return 1 if (different or missing) else 0


def diagnostics(path):
    path = os.path.abspath(path)
    work = os.path.join(TESTS, 'TestCppCompilerLib', 'TestCppCompilerLib')
    exe = executable('TestCppCompilerLib')
    env = dict(os.environ, TEST_CPP_CONFORMANCE_DIAGNOSTICS=path, TESTLIB_LOG=path + '.Log.txt')
    with open(path + '.out.txt', 'w') as said:
        code = subprocess.run([exe, '-only', 'TestCPPConformance'], cwd=work, env=env, stdout=said, stderr=subprocess.STDOUT).returncode
    print('TestCPPConformance exit %d' % code)
    return code


def entries(path):
    """A diagnostics file as {"Group.Case by": what was said}, and the names in the order they were run."""
    with io.open(path, encoding='latin-1') as file:
        text = file.read()
    said = {}
    order = []
    for entry in text.split('== ')[1:]:
        head, _, body = entry.partition('\n')
        said[head] = body
        order.append(head)
    return said, order


def shown(text):
    return text.strip().replace('\n', '\n    ')[:600].encode('ascii', 'backslashreplace').decode('ascii')


def said(first, second):
    a, order_a = entries(first)
    b, order_b = entries(second)
    only_a = [name for name in order_a if name not in b]
    only_b = [name for name in order_b if name not in a]
    different = [name for name in order_a if name in b and a[name] != b[name]]
    print('%d cases and %d: %d only in the first, %d only in the second, %d of which something else is said' % (len(a), len(b), len(only_a), len(only_b), len(different)))
    for name in only_a:
        print('only in the first: %s' % name)
    for name in only_b:
        print('only in the second: %s\n    %s' % (name, shown(b[name])))
    for name in different:
        print('%s\n  was:\n    %s\n  is:\n    %s' % (name, shown(a[name]), shown(b[name])))
    return 1 if (only_a or only_b or different) else 0


def main():
    arguments = sys.argv[1:]
    if len(arguments) == 2 and arguments[0] == 'listings':
        listings(arguments[1])
        return 0
    if len(arguments) == 3 and arguments[0] == 'compare':
        return compare(arguments[1], arguments[2])
    if len(arguments) == 2 and arguments[0] == 'diagnostics':
        return diagnostics(arguments[1])
    if len(arguments) == 3 and arguments[0] == 'said':
        return said(arguments[1], arguments[2])
    print(__doc__)
    return 2


if __name__ == '__main__':
    sys.exit(main())
