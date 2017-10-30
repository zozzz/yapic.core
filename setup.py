import os
import sys
import re
from pathlib import Path
from glob import glob
from setuptools import setup, Command, Extension
from setuptools.command.test import test as TestCommand
from setuptools.command.build_ext import build_ext
from setuptools.command.egg_info import egg_info

# DEVELOP = os.environ.get("DEVELOP") == "1"
# DEVELOP = True
# extensions = []
package_data = ["include/**/*.hpp"]

# min version: 3.5


# if DEVELOP:
#     modules = [
#         "src/yapic/cpp/test/_module.cpp",
#         "src/yapic/cpp/test/_bad_module.cpp",
#         "src/yapic/cpp/test/_bad_module2.cpp",
#         "src/yapic/cpp/test/_import.cpp",
#         "src/yapic/cpp/test/_bad_import.cpp",
#         "src/yapic/cpp/test/_bad_import2.cpp",
#     ]
#     for m in modules:
#         extensions.append(
#             Extension(
#                 name="yapic.cpp.test." + os.path.splitext(os.path.basename(m))[0],
#                 sources=[m],
#                 include_dirs=["src/yapic/cpp/include"],
#                 undef_macros=["NDEBUG"],
#                 # extra_compile_args=["/P"],
#                 language="c++"
#             )
#         )


class PyTest(TestCommand):
    user_options = [
        ("pytest-args=", "a", "Arguments to pass to pytest"),
        ("file=", "f", "File to run"),
    ]

    ext_modules = [
        "src/yapic/cpp/test/_module.cpp",
        "src/yapic/cpp/test/_bad_module.cpp",
        "src/yapic/cpp/test/_bad_module2.cpp",
        "src/yapic/cpp/test/_import.cpp",
        "src/yapic/cpp/test/_bad_import.cpp",
        "src/yapic/cpp/test/_bad_import2.cpp",
        "src/yapic/cpp/test/_string_builder.cpp",
    ]

    def initialize_options(self):
        super().initialize_options()
        self.pytest_args = "-x -s"
        self.file = None

    def finalize_options(self):
        super().finalize_options()
        if self.file:
            self.pytest_args += " " + self.file.replace("\\", "/")

    def run(self):
        installed_dists = self.install_dists(self.distribution)
        for dp in map(lambda x: x.location, installed_dists):
            if dp not in sys.path:
                sys.path.insert(0, dp)

        self._init_exts()
        self.run_command('build_ext')

        ext = self.get_finalized_command("build_ext")
        ep = str(Path(ext.build_lib).absolute())

        if ep not in sys.path:
            sys.path.insert(0, ep)

        for e in ext.extensions:
            if e._needs_stub:
                ext.write_stub(ep, e, False)

        # p = str(Path(__file__).absolute().parent.joinpath("src"))
        # if p not in sys.path:
        #     sys.path.insert(0, p)

        self.run_tests()

    def run_tests(self):
        import shlex
        import pytest
        errno = pytest.main(shlex.split(self.pytest_args))
        sys.exit(errno)

    def _init_exts(self):
        if not self.distribution.ext_modules:
            self.distribution.ext_modules = []
        self.distribution.packages += ["yapic.cpp.test"]
        define_macros = {}

        if sys.platform == "win32":
            define_macros["UNICODE"] = 1
            undef_macros = []
            extra_compile_args = []
            # extra_compile_args = ["/FC", Path(__file__).absolute().path.joinpath("src", "yapic", "")]
            # extra_compile_args.append("/P")  # Preprocessor outpout

            # /FA Assembly code; .asm
            # /FAc Machine and assembly code; .cod
            # /FAs Source and assembly code; .asm
            # /FAcs Machine, source, and assembly code; .cod
            # extra_compile_args.append("/FAs")

            if sys.executable.endswith("python_d.exe"):
                define_macros["_DEBUG"] = 1
                undef_macros.append("NDEBUG")
                extra_compile_args.append("/MTd")
                extra_compile_args.append("/Zi")
                # extra_compile_args.append("/MDd")
            else:
                pass
                # extra_compile_args.append("/MT")
                # extra_compile_args.append("/MD")

        depends = glob("src/yapic/cpp/include/**/*.hpp")
        for m in self.ext_modules:
            ecp = list(extra_compile_args)
            # ecp.append("/FC")
            # ecp.append(str(Path(__file__).absolute().parent.joinpath(m)))
            self.distribution.ext_modules.append(
                Extension(
                    name="yapic.cpp.test." + os.path.splitext(os.path.basename(m))[0],
                    sources=[m],
                    include_dirs=["src/yapic/cpp/include"],
                    undef_macros=undef_macros,
                    define_macros=list(define_macros.items()),
                    extra_compile_args=ecp,
                    language="c++",
                    depends=depends
                )
            )


dist = setup(
    name="yapic.cpp",
    packages=["yapic.cpp"],
    package_dir={"yapic.cpp": "src/yapic/cpp"},
    package_data={"yapic.cpp": package_data},
    tests_require=["pytest", "pytest-benchmark"],
    python_requires=">=3.5",
    cmdclass={"test": PyTest}

)
