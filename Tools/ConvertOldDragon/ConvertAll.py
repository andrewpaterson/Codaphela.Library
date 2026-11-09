"""Converts every old .DRG there is to a scene file.  This was meant to happen once: the new files are what's used from then on, and
the old ones were left where they are.  It's here to say where each new file came from.

    python ConvertAll.py [output directory]

With no directory each file goes where it belongs in the repositories.  With one they all go there, to try it.
Run from anywhere, after building ConvertOldDragon.sln Release|x64.
"""
import glob
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOOL = os.path.join(HERE, "x64", "Release", "ConvertOldDragon.exe")

GAME_ENGINE = "D:/gameengine"
COMMON = GAME_ENGINE + "/CommonLibraries"
INTO_OBLIVION = GAME_ENGINE + "/IntoOblivionX/Data/Meshes"
STAR_CONTROL = GAME_ENGINE + "/StarControlRedux/Data/Meshes"
OLD_STAR_CONTROL = "D:/Work/Star Control"
VIEWER = OLD_STAR_CONTROL + "/Builds/ViewerV7beta2009.9.20"

CONVERSIONS = [
    (VIEWER + "/Goober.DRG", COMMON + "/Tests/TestSceneFileLib/TestSceneFileLib/Input/Scene/Goober.DRG", ["--particles", VIEWER + "/Particles/Stars.csv"]),
    (GAME_ENGINE + "/StarControl1/StarControl1/Melee.DRG", STAR_CONTROL + "/Melee.DRG", ["--max", OLD_STAR_CONTROL + "/MaxFiles/Melee.max"]),
    (OLD_STAR_CONTROL + "/Builds/StarControl/Test.DRG", STAR_CONTROL + "/Test.DRG", []),
]
for old in sorted(glob.glob(GAME_ENGINE + "/IOX2021/data/meshes/*.DRG")):
    CONVERSIONS.append((old, INTO_OBLIVION + "/" + os.path.basename(old), []))


def main():
    only_directory = sys.argv[1] if len(sys.argv) > 1 else None
    failures = 0
    for old, new, options in CONVERSIONS:
        if only_directory:
            new = os.path.join(only_directory, os.path.basename(old))
        os.makedirs(os.path.dirname(new), exist_ok=True)
        result = subprocess.run([TOOL, old, new] + options, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        lines = result.stdout.splitlines()
        print(lines[-1] if lines else "(nothing printed)")
        if result.returncode != 0:
            print("\n".join(lines[:-1]))
            failures += 1
    print(f"{len(CONVERSIONS) - failures} of {len(CONVERSIONS)} converted.")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
