"""Bidirectional LiveControl session — hides grpcio's generator/thread ceremony."""
from __future__ import annotations

import queue
import threading
from typing import Callable, Optional

import grpc

from robot.v1 import robot_pb2, robot_pb2_grpc


_SENTINEL = object()


class LiveControlSession:
    def __init__(
        self,
        stub: robot_pb2_grpc.RobotServiceStub,
        on_telemetry: Callable[[robot_pb2.Telemetry], None],
        translate_error: Callable[[grpc.RpcError], Exception],
    ) -> None:
        self._on_telemetry = on_telemetry
        self._translate = translate_error
        self._send_q: queue.Queue = queue.Queue(maxsize=1)
        self._closed = False
        self._error: Optional[Exception] = None
        self._response_iter = stub.LiveControl(self._request_iter())
        self._reader = threading.Thread(target=self._read_loop, daemon=True)
        self._reader.start()

    def send(self, cmd: robot_pb2.MotionCommand) -> None:
        if self._error is not None:
            raise self._error
        if self._closed:
            raise RuntimeError("LiveControlSession is closed")
        # Client-side last-write-wins: drop any queued stale command first.
        try:
            self._send_q.get_nowait()
        except queue.Empty:
            pass
        self._send_q.put(cmd)

    def _request_iter(self):
        while True:
            item = self._send_q.get()
            if item is _SENTINEL:
                return
            yield item

    def _read_loop(self) -> None:
        try:
            for telem in self._response_iter:
                self._on_telemetry(telem)
        except grpc.RpcError as e:
            if not self._closed:
                self._error = self._translate(e)

    def close(self) -> None:
        if self._closed:
            return
        self._closed = True
        # Unblock the request generator, then cancel the response stream so
        # the reader thread doesn't wait forever on the server.
        try:
            self._send_q.get_nowait()
        except queue.Empty:
            pass
        self._send_q.put(_SENTINEL)
        self._response_iter.cancel()
        self._reader.join(timeout=2.0)

    def __enter__(self) -> "LiveControlSession":
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()
        # Surface a reader-thread error only when the with-block itself
        # exited cleanly — never mask a user exception.
        if exc is None and self._error is not None:
            raise self._error
