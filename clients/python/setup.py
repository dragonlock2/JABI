import multiprocessing
import platform
import subprocess
import sys
from glob import glob
from pathlib import Path
from setuptools import setup
from shutil import copytree, copyfile
from pybind11.setup_helpers import Pybind11Extension, build_ext

# workaround for using sources in parent directory
root_dir = Path(__file__).resolve().parent / "../.."
temp_dir = Path("./tmp")
if not temp_dir.exists(): # note won't copy updated files!
    for d in ["clients/cpp", "include"]:
        copytree(root_dir / d, temp_dir / d)
    for f in ["README.md"]:
        copyfile(root_dir / f, temp_dir / f)

def msvc_platform():
    bits = 64 if sys.maxsize > 2**32 else 32
    if platform.machine() in ["x86", "AMD64"]:
        plat = "Win32" if bits == 32 else "x64"
    elif platform.machine() in ["ARM", "ARM64"]:
        plat = "ARM" if bits == 32 else "ARM64"
    else:
        plat = ""
    return plat

class build_jabi(build_ext):
    def run(self):
        # build libusb from source
        msvc = sys.platform == "win32" and "GCC" not in sys.version
        plat = msvc_platform()
        if msvc and not list(Path(f"libusb/build").glob(f"**/{plat}")):
            msbuild = subprocess.check_output([
                "C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe",
                "-requires", "Microsoft.Component.MSBuild", "-find", "MSBuild/**/Bin/MSBuild.exe",
                "-latest", # TODO pick correct version
            ]).decode('utf-8').strip()
            if not msbuild:
                raise Exception("pls install msvc")
            cmds = [
                [msbuild, "/p:configuration=release", f"/p:platform={plat}", "/target:libusb_static", "msvc/libusb.sln"]
            ]
        elif sys.platform != "win32" and not Path("libusb/libusb/.libs").exists():
            cmds = [
                ["./bootstrap.sh"],
                ["./autogen.sh", "--disable-udev"],
                ["./configure", "--enable-static", "--disable-shared", "--disable-udev", "--with-pic"],
                ["make", f"-j{multiprocessing.cpu_count()}"],
            ]
        else:
            # MinGW subprocess doesn't run correctly, can manually run above
            cmds = []
        for c in cmds:
            subprocess.run(c, cwd="libusb").check_returncode()

        # link libusb
        if msvc:
            self.library_dirs.append(str(list(Path("libusb/build").glob(f"**/{plat}/**/libusb-1.0.lib"))[0].parent))
            self.libraries.append("libusb-1.0")
        else:
            self.library_dirs.append("libusb/libusb/.libs")
            self.libraries.append("usb-1.0")

        # continue build
        build_ext.run(self)

setup(ext_modules=[
    Pybind11Extension(
        name = "jabi",
        sources = [
            "jabi.cpp",
            *sorted(glob("tmp/clients/cpp/libjabi/interfaces/*.cpp")),
            *sorted(glob("tmp/clients/cpp/libjabi/peripherals/*.cpp")),
        ],
        include_dirs = [
            "tmp/clients/cpp",
            "tmp/include",
            "libusb/libusb",
        ],
    )],
    cmdclass={"build_ext": build_jabi},
)

# python -m build
# python -m twine upload dist/**/*
