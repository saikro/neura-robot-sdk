"""CLI for the robot client. Subcommands: info, telemetry, control."""
from __future__ import annotations

import argparse
import math
import os
import sys
import time

from robot_client import RobotClient, RobotClientError
from robot.v1 import robot_pb2


DEFAULT_TARGET = "localhost:50051"


def _target_default() -> str:
    return os.environ.get("ROBOT_TARGET", DEFAULT_TARGET)


def _print_info(info: robot_pb2.RobotInfo) -> None:
    name = info.name if info.HasField("name") else "(unset)"
    state = robot_pb2.RobotState.Name(info.state)
    print(f"serial_number: {info.serial_number}")
    print(f"model:         {info.model}")
    print(f"name:          {name}")
    print(f"state:         {state}")


def _cmd_info(args: argparse.Namespace) -> int:
    with RobotClient(args.target) as client:
        info = client.get_info()
    _print_info(info)
    return 0


def _print_telemetry(t: robot_pb2.Telemetry) -> None:
    p0 = t.joint_positions[0] if len(t.joint_positions) > 0 else float("nan")
    v0 = t.joint_velocities[0] if len(t.joint_velocities) > 0 else float("nan")
    state = robot_pb2.RobotState.Name(t.state)
    ts = t.timestamp.seconds + t.timestamp.nanos * 1e-9
    print(f"t={ts:.3f} joint0_pos={p0:+.4f} joint0_vel={v0:+.4f} state={state}")


def _cmd_telemetry(args: argparse.Namespace) -> int:
    with RobotClient(args.target) as client:
        for telem in client.stream_telemetry(rate_hz=args.rate, duration_s=args.duration):
            _print_telemetry(telem)
    return 0


def _cmd_control(args: argparse.Namespace) -> int:
    amplitude = args.amplitude
    freq_hz = args.freq_hz
    duration_s = args.duration
    period = 1.0 / args.rate
    omega = 2.0 * math.pi * freq_hz

    with RobotClient(args.target) as client:
        with client.live_control(on_telemetry=_print_telemetry) as session:
            start = time.monotonic()
            while True:
                t = time.monotonic() - start
                if t >= duration_s:
                    break
                pos = amplitude * math.sin(omega * t)
                vel = amplitude * omega * math.cos(omega * t)
                session.send(robot_pb2.MotionCommand(
                    targets=[robot_pb2.JointTarget(
                        joint_index=0, position=pos, velocity=vel,
                    )],
                ))
                time.sleep(period)
    return 0


def _build_parser() -> argparse.ArgumentParser:
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument(
        "--target",
        default=_target_default(),
        help=f"gRPC target (default: {DEFAULT_TARGET}; overridable via ROBOT_TARGET env var)",
    )

    parser = argparse.ArgumentParser(prog="robot-client")
    sub = parser.add_subparsers(dest="cmd", required=True)

    p_info = sub.add_parser("info", parents=[common], help="fetch identity and current state")
    p_info.set_defaults(func=_cmd_info)

    p_tel = sub.add_parser(
        "telemetry", parents=[common],
        help="subscribe to state telemetry for N seconds",
    )
    p_tel.add_argument("--rate", type=int, default=10, help="sampling rate in Hz (default: 10)")
    p_tel.add_argument("--duration", type=float, default=5.0, help="seconds to stream (default: 5)")
    p_tel.set_defaults(func=_cmd_telemetry)

    p_ctrl = sub.add_parser(
        "control", parents=[common],
        help="drive joint 0 on a sinusoidal trajectory via LiveControl",
    )
    p_ctrl.add_argument("--duration", type=float, default=5.0, help="seconds to run (default: 5)")
    p_ctrl.add_argument("--amplitude", type=float, default=0.5, help="rad (default: 0.5)")
    p_ctrl.add_argument("--freq-hz", dest="freq_hz", type=float, default=0.5, help="Hz (default: 0.5)")
    p_ctrl.add_argument("--rate", type=int, default=50, help="command send rate in Hz (default: 50)")
    p_ctrl.set_defaults(func=_cmd_control)

    return parser


def main() -> int:
    args = _build_parser().parse_args()
    try:
        return args.func(args)
    except RobotClientError as e:
        print(f"error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
