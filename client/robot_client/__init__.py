"""Python client for the robot.v1 gRPC service."""
from __future__ import annotations

import sys
from pathlib import Path

# grpc_tools emits `from robot.v1 import robot_pb2` — resolve via generated/.
sys.path.insert(0, str(Path(__file__).parent / "generated"))

from robot_client.client import RobotClient, RobotClientError  # noqa: E402

__all__ = ["RobotClient", "RobotClientError"]
