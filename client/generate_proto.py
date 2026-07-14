#!/usr/bin/env python3
"""Regenerate Python gRPC stubs from ../proto/robot/v1/robot.proto into robot_client/generated/."""
from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path


HERE = Path(__file__).parent
PROTO_ROOT = (HERE / ".." / "proto").resolve()
OUT = HERE / "robot_client" / "generated"


def main() -> int:
    if not PROTO_ROOT.is_dir():
        print(f"proto root not found: {PROTO_ROOT}", file=sys.stderr)
        return 1

    if OUT.exists():
        shutil.rmtree(OUT)
    OUT.mkdir(parents=True)

    cmd = [
        sys.executable,
        "-m", "grpc_tools.protoc",
        f"-I{PROTO_ROOT}",
        f"--python_out={OUT}",
        f"--grpc_python_out={OUT}",
        f"--pyi_out={OUT}",
        str(PROTO_ROOT / "robot" / "v1" / "robot.proto"),
    ]
    result = subprocess.run(cmd)
    if result.returncode != 0:
        return result.returncode

    # grpc_tools emits `from robot.v1 import robot_pb2` — needs package markers.
    (OUT / "__init__.py").touch()
    (OUT / "robot" / "__init__.py").touch()
    (OUT / "robot" / "v1" / "__init__.py").touch()

    print(f"generated -> {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
