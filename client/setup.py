"""Setuptools shim to run generate_proto.py as part of the build.

pyproject.toml carries all package metadata; this file exists solely for the
cmdclass hook (build_py subclass), which pyproject.toml alone cannot express.
"""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

from setuptools import setup
from setuptools.command.build_py import build_py as _build_py


HERE = Path(__file__).parent


class BuildPy(_build_py):
    def run(self) -> None:
        subprocess.check_call([sys.executable, str(HERE / "generate_proto.py")])
        super().run()


setup(cmdclass={"build_py": BuildPy})
