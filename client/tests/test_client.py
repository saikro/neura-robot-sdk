"""Unit tests for RobotClient and LiveControlSession."""
from __future__ import annotations

import threading
from unittest.mock import MagicMock, patch

from robot_client import RobotClient
from robot_client.session import LiveControlSession
from robot.v1 import robot_pb2


def test_channel_closed_on_exit():
    with patch("robot_client.client.grpc.insecure_channel") as mock_ctor:
        mock_channel = MagicMock()
        mock_ctor.return_value = mock_channel
        with RobotClient("localhost:50051"):
            pass
        mock_channel.close.assert_called_once()


class _FakeResponseIter:
    """Iterator that blocks in __next__ until cancel() is called."""

    def __init__(self) -> None:
        self._stop = threading.Event()

    def __iter__(self) -> "_FakeResponseIter":
        return self

    def __next__(self) -> None:
        self._stop.wait()
        raise StopIteration

    def cancel(self) -> None:
        self._stop.set()


class _FakeStub:
    """Never iterates request_iter — the test needs to inspect the queue undrained."""

    def LiveControl(self, request_iter):
        return _FakeResponseIter()


def test_send_evicts_superseded_command():
    session = LiveControlSession(
        stub=_FakeStub(),
        on_telemetry=lambda t: None,
        translate_error=lambda e: e,
    )
    try:
        cmd1 = robot_pb2.MotionCommand(
            targets=[robot_pb2.JointTarget(joint_index=0, position=1.0)]
        )
        cmd2 = robot_pb2.MotionCommand(
            targets=[robot_pb2.JointTarget(joint_index=0, position=2.0)]
        )
        session.send(cmd1)
        session.send(cmd2)

        assert session._send_q.qsize() == 1
        remaining = session._send_q.get_nowait()
        assert remaining is cmd2
    finally:
        session.close()
