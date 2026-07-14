"""Thin facade over the robot.v1 gRPC stubs."""
from __future__ import annotations

from typing import Callable, Iterator, Optional

import grpc

from robot.v1 import robot_pb2, robot_pb2_grpc

from robot_client.session import LiveControlSession


class RobotClientError(Exception):
    """Human-readable error surfaced by RobotClient. Wraps grpc.RpcError."""


class RobotClient:
    def __init__(self, target: str, timeout: float = 5.0) -> None:
        self._target = target
        self._timeout = timeout
        self._channel: Optional[grpc.Channel] = None
        self._stub: Optional[robot_pb2_grpc.RobotServiceStub] = None

    def __enter__(self) -> "RobotClient":
        self._channel = grpc.insecure_channel(self._target)
        self._stub = robot_pb2_grpc.RobotServiceStub(self._channel)
        return self

    def __exit__(self, *exc) -> None:
        if self._channel is not None:
            self._channel.close()
            self._channel = None
            self._stub = None

    def get_info(self) -> robot_pb2.RobotInfo:
        # TODO: expose an SDK-owned type instead of the generated proto message —
        # the facade should hide the wire contract, not leak it to consumers.
        assert self._stub is not None, "RobotClient must be used as a context manager"
        try:
            response = self._stub.GetRobotInfo(
                robot_pb2.GetRobotInfoRequest(),
                timeout=self._timeout,
            )
        except grpc.RpcError as e:
            raise self._translate(e) from e
        return response.info

    def stream_telemetry(
        self, rate_hz: int, duration_s: float,
    ) -> Iterator[robot_pb2.Telemetry]:
        assert self._stub is not None, "RobotClient must be used as a context manager"
        request = robot_pb2.StreamTelemetryRequest(rate_hz=rate_hz)
        try:
            for telem in self._stub.StreamTelemetry(request, timeout=duration_s):
                yield telem
        except grpc.RpcError as e:
            # Deadline is how we stop the stream; not an error.
            if e.code() == grpc.StatusCode.DEADLINE_EXCEEDED:
                return
            raise self._translate(e) from e

    def live_control(
        self,
        on_telemetry: Callable[[robot_pb2.Telemetry], None],
    ) -> LiveControlSession:
        assert self._stub is not None, "RobotClient must be used as a context manager"
        return LiveControlSession(self._stub, on_telemetry, self._translate)

    def _translate(self, err: grpc.RpcError) -> RobotClientError:
        code = err.code() if hasattr(err, "code") else grpc.StatusCode.UNKNOWN
        details = err.details() if hasattr(err, "details") else str(err)
        if code == grpc.StatusCode.UNAVAILABLE:
            return RobotClientError(f"robot not reachable at {self._target}")
        if code == grpc.StatusCode.DEADLINE_EXCEEDED:
            return RobotClientError(f"timed out after {self._timeout}s")
        return RobotClientError(f"{code.name}: {details}")
