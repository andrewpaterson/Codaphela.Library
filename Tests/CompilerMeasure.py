"""Measures the C++ compiler (CppCompilerLib) with the Lib65 test suites, the same way each time.

    python CompilerMeasure.py [--compile] [--serial] [--size] [--configuration Release] [--only Suite,...] [--out file]
                              [--raw directory]

For each suite (TestBaseLib65, TestStandardLib65, TestSupportLib65) on each processor (65C64, W65C816) it runs the
suite's Release x64 executable and writes a table: whether it passed, its programs and asserts, the cycles its
programs took on the emulator and the bytes of their linked images (each added up), the wall time, the process's peak
memory, and the compiler's own times by phase and counts (TEST_PROJECT65_COMPILE_STATS).

--compile  only compiles the suite's project (no program is linked or run): for the compiler's own time and memory.
--serial   compiles the units one at a time, for CPU time that other work on the machine disturbs less.
--raw      keeps each run's own output there (Suite_Processor.txt): every program's cycles and size, to compare
           two compilers program by program.

Cycles, sizes and counts are exact: two runs of one compiler give the same.  Times are not: take them twice.

The suites must be built first (their solutions, Release x64).  ConsolidationPlan.txt in CppCompilerLib says what
the numbers are for.
"""
import argparse
import os
import re
import subprocess
import sys
import time

TESTS = os.path.dirname(os.path.abspath(__file__))
SUITES = ['TestBaseLib65', 'TestStandardLib65', 'TestSupportLib65']
PROCESSORS = [('65C64', []), ('W65C816', ['816'])]


def peak_memory(process):
    """The finished process's peak working set in megabytes, or 0 where it can't be asked for."""
    if os.name != 'nt':
        try:
            import resource
            return resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss // 1024
        except Exception:
            return 0
    import ctypes
    from ctypes import wintypes

    class Counters(ctypes.Structure):
        _fields_ = [('cb', wintypes.DWORD), ('PageFaultCount', wintypes.DWORD), ('PeakWorkingSetSize', ctypes.c_size_t),
                    ('WorkingSetSize', ctypes.c_size_t), ('QuotaPeakPagedPoolUsage', ctypes.c_size_t),
                    ('QuotaPagedPoolUsage', ctypes.c_size_t), ('QuotaPeakNonPagedPoolUsage', ctypes.c_size_t),
                    ('QuotaNonPagedPoolUsage', ctypes.c_size_t), ('PagefileUsage', ctypes.c_size_t),
                    ('PeakPagefileUsage', ctypes.c_size_t)]

    counters = Counters()
    counters.cb = ctypes.sizeof(Counters)
    get = ctypes.windll.psapi.GetProcessMemoryInfo
    get.argtypes = [wintypes.HANDLE, ctypes.POINTER(Counters), wintypes.DWORD]
    if not get(int(process._handle), ctypes.byref(counters), counters.cb):
        return 0
    return counters.PeakWorkingSetSize // (1024 * 1024)


def run(suite, processor, arguments, options):
    directory = os.path.join(TESTS, suite, suite)
    executable = os.path.join(TESTS, suite, 'x64', options.configuration, suite + ('.exe' if os.name == 'nt' else ''))
    if not os.path.isfile(executable):
        return {'suite': suite, 'processor': processor, 'error': 'not built: ' + executable}
    environment = dict(os.environ)
    environment['TEST_PROJECT65_COMPILE_STATS'] = '1'
    environment['TEST_PROJECT65_CYCLES'] = '1'
    if options.compile:
        environment['TEST_PROJECT65_ONLY'] = 'NoSuchTest'
    if options.serial:
        environment['TEST_PROJECT65_SERIAL'] = '1'
    if options.size:
        # Smaller code is slower code: a program gets more of the emulator's instructions before it is given up on.
        environment['TEST_PROJECT65_PREFER_SIZE'] = '1'
        environment.setdefault('TEST_PROJECT65_STEPS', '20000')
    started = time.time()
    process = subprocess.Popen([executable] + arguments, cwd=directory, env=environment, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output = process.communicate()[0].decode('latin-1')
    if options.raw:
        os.makedirs(options.raw, exist_ok=True)
        open(os.path.join(options.raw, '%s_%s.txt' % (suite, processor)), 'w', encoding='latin-1', newline='').write(output)
    result = {'suite': suite, 'processor': processor, 'seconds': time.time() - started, 'exit': process.returncode,
              'memory': peak_memory(process), 'cycles': 0, 'size': 0, 'programs': 0, 'asserts': 0, 'times': {}, 'counts': {}, 'passes': {}}
    for line in output.split('\n'):
        line = line.rstrip('\r')
        m = re.match(r'CYCLES (\d+)', line)
        if m:
            result['cycles'] += int(m.group(1))
        m = re.match(r'SIZE (\d+)', line)
        if m:
            result['size'] += int(m.group(1))
        m = re.match(r'\w+: (\d+) programs, (\d+) asserts', line)
        if m:
            result['programs'] = int(m.group(1))
            result['asserts'] = int(m.group(2))
        m = re.match(r'TIME (.+) (\d+)$', line)
        if m:
            result['times'][m.group(1)] = int(m.group(2))
        m = re.match(r'COUNT (.+) (\d+)$', line)
        if m:
            result['counts'][m.group(1)] = int(m.group(2))
        m = re.match(r'PASS (\w+) (\d+) (\d+) (\d+) (\d+) (\d+)$', line)
        if m:
            result['passes'][m.group(1)] = [int(m.group(i)) for i in range(2, 7)]
    log = os.path.join(directory, 'Log.txt')
    result['passed'] = False
    if process.returncode == 0 and os.path.isfile(log) and os.path.getmtime(log) >= started - 1:
        text = open(log, encoding='latin-1').read()
        result['passed'] = ('Total Failed: 0' in text) and not re.search(r'^Failed: [A-Z]', text, re.M)
    return result


def table(results, options):
    lines = []
    lines.append('%-18s %-8s %-6s %8s %8s %16s %12s %8s %8s' % ('Suite', 'CPU', 'Passed', 'Programs', 'Asserts', 'Cycles', 'Bytes', 'Seconds', 'Peak MB'))
    for r in results:
        if 'error' in r:
            lines.append('%-18s %-8s %s' % (r['suite'], r['processor'], r['error']))
            continue
        lines.append('%-18s %-8s %-6s %8d %8d %16d %12d %8.0f %8d' % (r['suite'], r['processor'], 'yes' if r['passed'] else 'NO', r['programs'], r['asserts'],
                                                                 r['cycles'], r['size'], r['seconds'], r['memory']))
    good = [r for r in results if 'error' not in r]
    for kind, title in (('times', 'Compile milliseconds by phase (added over the threads)'), ('counts', 'Counts')):
        names = []
        for r in good:
            for name in r[kind]:
                if name not in names:
                    names.append(name)
        if not names:
            continue
        lines.append('')
        lines.append(title)
        lines.append('%-24s' % '' + ''.join('%16s' % (r['suite'][4:-2] + ' ' + ('816' if r['processor'] == 'W65C816' else '65')) for r in good))
        for name in names:
            lines.append('%-24s' % name + ''.join('%16d' % r[kind].get(name, 0) for r in good))
    for r in good:
        if not r['passes']:
            continue
        lines.append('')
        lines.append("The optimiser's passes, %s %s: runs, the runs that changed their function, those of them in its first round," % (r['suite'], r['processor']))
        lines.append('those in its third round or later, and milliseconds (added over the threads)')
        for name, numbers in r['passes'].items():
            lines.append('%-28s %9d %9d %9d %9d %9d' % tuple([name] + numbers))
    return '\n'.join(lines) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--compile', action='store_true')
    parser.add_argument('--serial', action='store_true')
    parser.add_argument('--size', action='store_true', help='the compiler prefers size to speed')
    parser.add_argument('--configuration', default='Release')
    parser.add_argument('--only', default='')
    parser.add_argument('--processor', default='', help='65C64 or W65C816; both by default')
    parser.add_argument('--out', default='')
    parser.add_argument('--raw', default='', help="a directory for each run's own output")
    options = parser.parse_args()
    suites = [s for s in SUITES if not options.only or s in options.only.split(',')]
    results = []
    for suite in suites:
        for processor, arguments in PROCESSORS:
            if options.processor and options.processor != processor:
                continue
            results.append(run(suite, processor, arguments, options))
            sys.stderr.write('%s %s done\n' % (suite, processor))
    text = table(results, options)
    if options.out:
        open(options.out, 'w').write(text)
    sys.stdout.write(text)
    return 0 if all(r.get('passed') or options.compile for r in results if 'error' not in r) and not any('error' in r for r in results) else 1


if __name__ == '__main__':
    sys.exit(main())
