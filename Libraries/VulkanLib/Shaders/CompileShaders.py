"""Compiles VulkanLib's shaders to SPIR-V with the Vulkan SDK's glslc, and writes them into VulkanShaders.h as arrays of words,
so that building VulkanLib doesn't need the SDK's tools.  Run it after changing a shader, and commit what it writes.

    python CompileShaders.py

VULKAN_SDK must be set, as the SDK's installer does.
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUTPUT = os.path.join(HERE, "..", "VulkanShaders.h")

#Each shader, and the name of its array.
SHADERS = [
    ("Mesh.vert", "gauiVulkanMeshVertexShader"),
    ("Mesh.frag", "gauiVulkanMeshFragmentShader"),
]


def compile_shader(glslc, source):
    result = subprocess.run([glslc, "-O", "--target-env=vulkan1.3", "-o", "-", os.path.join(HERE, source)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode != 0:
        sys.stderr.write(result.stderr.decode())
        sys.exit(1)
    return result.stdout


def main():
    sdk = os.environ.get("VULKAN_SDK")
    if not sdk:
        sys.stderr.write("VULKAN_SDK isn't set.\n")
        return 1
    glslc = os.path.join(sdk, "Bin", "glslc.exe" if os.name == "nt" else "glslc")
    if not os.path.exists(glslc):
        glslc = os.path.join(sdk, "bin", "glslc")

    lines = [
        "//Written by Shaders/CompileShaders.py from the shaders beside it.  Don't change it by hand.",
        "#ifndef __VULKAN_SHADERS_H__",
        "#define __VULKAN_SHADERS_H__",
        "#include \"BaseLib/PrimitiveTypes.h\"",
        "",
        "",
    ]
    for source, name in SHADERS:
        spirv = compile_shader(glslc, source)
        words = [int.from_bytes(spirv[i:i + 4], "little") for i in range(0, len(spirv), 4)]
        lines.append("//%s, %d bytes of SPIR-V." % (source, len(spirv)))
        lines.append("static const uint32 %s[] =" % name)
        lines.append("{")
        for i in range(0, len(words), 8):
            lines.append("\t" + ", ".join("0x%08X" % w for w in words[i:i + 8]) + ",")
        lines.append("};")
        lines.append("")
        lines.append("")
    lines.append("#endif // __VULKAN_SHADERS_H__")
    lines.append("")
    with open(OUTPUT, "w", newline="\n") as f:
        f.write("\n".join(lines))
    print("Wrote " + os.path.normpath(OUTPUT))
    return 0


if __name__ == "__main__":
    sys.exit(main())
