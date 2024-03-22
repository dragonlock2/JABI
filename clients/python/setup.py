import multiprocessing
import subprocess
from glob import glob
from pathlib import Path
from setuptools import setup
from shutil import rmtree, copytree, copyfile
from pybind11.setup_helpers import Pybind11Extension, build_ext

# workaround for using sources in parent directory
root_dir = Path(__file__).resolve().parent / "../.."
temp_dir = Path("./tmp")
if not temp_dir.exists(): # note won't copy updated files!
    for d in ["clients/cpp", "include"]:
        copytree(root_dir / d, temp_dir / d)
    for f in ["README.md"]:
        copyfile(root_dir / f, temp_dir / f)

class jabi_build(build_ext):
    def run(self):
        # build libusb from source
        if not Path("libusb/libusb/.libs").exists():
            cmds = [
                ["./bootstrap.sh"],
                ["./autogen.sh", "--disable-udev"],
                ["./configure", "--enable-static", "--disable-shared", "--disable-udev", "--with-pic"],
                ["make", f"-j{multiprocessing.cpu_count()}"],
            ]
            for c in cmds:
                subprocess.run(c, cwd="libusb").check_returncode()
            # TODO test on Windows

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
        library_dirs = ["libusb/libusb/.libs"],
        libraries = ["usb-1.0"],
    )],
    cmdclass={"build_ext": jabi_build},
)

# python -m build
# python -m twine upload dist/*
