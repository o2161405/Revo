#!/usr/bin/env python3

import glob
import os
import re
import io
import sys
from dataclasses import dataclass, field
from vendor.ninja_syntax import Writer

ON_WINDOWS = True if sys.platform.startswith("win32") else False

# Ninja variables and rules
out_buf = io.StringIO()
n = Writer(out_buf)

build_dir = "build"

n.variable("ninja_required_version", "1.3")
n.variable("builddir", build_dir)
n.variable("outdir", "out")
n.variable("compiler", "g++")
n.variable("ar", "ar")
n.variable("ldflags", "")
n.newline()
n.rule("cc", command="$compiler -MD -MT $out -MF $out.d $ccflags -c $in -o $out", depfile="$out.d", deps="gcc", description="CC $out")
n.rule("ar", command="$ar rcs $out $in", description="AR $out")
n.newline()
n.rule("ld", command="$compiler $in $ldflags -o $out", description="LD $out")
n.newline()
n.rule("ld_shared", command="$compiler -shared $in $ldflags -o $out", description="LD_SHARED $out")
n.newline()

# Flags
common_ccflags = [
    '-std=c++2c',
    '-freflection',
    '-pedantic-errors',
    '-fno-asynchronous-unwind-tables',
    '-I', 'source',
    '-include', 'source/util/Assert.hh',
    '-include', 'source/util/Types.hh',
    '-include', 'source/util/Console.hh',
    '-fno-rtti',
    '-Wall',
    '-Wextra',
    '-Werror',
    '-Wsuggest-override',
    '-Wold-style-cast',
    '-Wcast-qual',
    '-Wpessimizing-move',
    '-Wredundant-move',
    '-Wconversion',
    '-Wundef',
    '-Wfloat-equal',
    '-Wnull-dereference',
    '-Wformat=2',
]

# Build types
@dataclass
class Build:
    name: str
    suffix: str
    flags: list[str]
    obj_files: dict = field(default_factory=lambda: {
        "common": [],
        "main": []
    })

builds = [
    Build(
        name="target",
        suffix="",
        flags=["-O3", "-flto=auto", "-D_FORTIFY_SOURCE=3"]
    ),
    Build(
        name="debug",
        suffix="-debug",
        flags=[
            "-DBUILD_DEBUG", "-O0", "-ggdb", "-fsanitize=address,undefined",
            "-fno-omit-frame-pointer"
        ]
    )
]

# Compile files
def get_files():
    sep = re.escape(os.path.sep)
    main_regex = re.compile(rf".*{sep}host{sep}main\.cc$")

    main_files = []
    common_files = []

    for file in glob.glob(os.path.join("source", "**", "*.cc"), recursive=True):
        if main_regex.search(file):
            main_files.append(file)
            continue

        common_files.append(file)

    return common_files, main_files

def compile(files, builds, obj_key, flags):
    for in_file in files:
        base_name, _ = os.path.splitext(in_file)
        for build in builds:
            out_file = os.path.join("$builddir", f"{base_name}{build.suffix}.o")
            build.obj_files[obj_key].append(out_file)
            n.build(out_file, "cc", in_file, variables={"ccflags": " ".join([*flags, *build.flags])})

common_files, main_files = get_files()
compile(common_files, builds, "common", common_ccflags)
compile(main_files, builds, "main", common_ccflags)
n.newline()

# Link files
binary_extension = ".exe" if ON_WINDOWS else ""

for build in builds:
    # Common
    librevo_name = f"librevo{build.suffix}.a"
    librevo_path = os.path.join("$builddir", librevo_name)
    librevo_objs = build.obj_files["common"]
    n.build(librevo_path, "ar", librevo_objs)
    n.newline()

    # Main
    exe_name = f"Revo{build.suffix}{binary_extension}"
    exe_path = os.path.join("$outdir", exe_name)
    exe_obj = build.obj_files["main"] + [librevo_path]
    
    # Link ASan and UBSan runtimes
    current_ldflags = " ".join(build.flags)
    n.build(exe_path, "ld", exe_obj, variables={"ldflags": current_ldflags})

n.rule("configure", command=f"{sys.executable} configure.py", generator=True)
n.build("build.ninja", "configure", implicit=["configure.py", os.path.join("vendor", "ninja_syntax.py")])
n.newline()

with open("build.ninja", "w") as f:
    f.write(out_buf.getvalue())
n.close()
