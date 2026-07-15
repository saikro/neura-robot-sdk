# Robot Client SDK — gRPC Example

A minimal, self-contained example of a gRPC-based robot API: a C++ server approximating the robot backend, a Python client approximating consumption, and a shared `.proto` contract demonstrating request/response, one-way streaming, and bidirectional streaming.

## The assignment

> **Overall goal:** Create a minimal, self-contained example of a gRPC-based API.
>
> - Imagine some sort of robotics setting
> - 2–5 example functions of your choice that make up the API
> - A minimal server that approximates the API backend (robot)
> - A minimal client that approximates API consumption (client)
> - The API should demonstrate:
>   - how request/response-like communication could work
>   - how one-way and bi-directional streaming could work
>
> **Notes:** The task is deliberately open. The interest is in the approach, how decisions are made under ambiguity, and general development style. Assumptions and TODOs for a later production system are welcome. Keep it as simple as necessary — but apply rigorous development practices expected in a clean codebase.
>
> **Preferred tooling:** git · server in C++ · client in Python.

## Scope note

This gRPC contract is an **internal transport layer**, not the surface an application developer would use. In a real SDK a thin per-language facade sits on top of it, hiding channels, streams, status codes and retries behind a domain API. This example builds the transport and marks the facade boundary as a design decision (see [Design decisions](#design-decisions)) rather than implementing it.

---

## Repository layout

```
.
├── proto/robot/v1/robot.proto   # the API contract (see note below)
├── server/                      # C++ — approximates the robot backend
├── client/                      # Python — approximates a consumer
├── docker-compose.yml           # brings up server, runs client against it
└── README.md
```

**On `proto/` placement:** the contract lives at the repo root so it belongs to neither side. It is checked into the tree for a self-contained, reproducible example — the same rationale as vendoring a dependency into `third_party/` in large C++ projects. In a real system it would be owned by the robot team and consumed as a versioned artifact (a separate repo with its own CI/CD that clients pull, a pinned submodule, or a schema-registry package), with server and client generating from it independently.

## The API

Robotics setting: one robotic arm exposed over gRPC — identity, joint telemetry, and live motion control.

| RPC | Type | Purpose |
|-----|------|---------|
| `GetRobotInfo` | unary | identity + current state query |
| `StreamTelemetry` | server-streaming | one-way: robot pushes state at N Hz |
| `LiveControl` | bidirectional | client commands + robot state, independent streams |

## Running it

### With Docker Compose

```bash
docker compose up -d server   # build image, start server on :50051 (first build is slow — see note)
docker compose run --rm client info   # unary: robot identity + state
docker compose run --rm client telemetry --rate 10 --duration 5   # server-streaming at 10 Hz for 5 s
docker compose run --rm client control --duration 5   # bidi: stream joint targets, telemetry back
docker compose down   # stop server, remove containers + network
```

> First build compiles gRPC via vcpkg (~30–40 min, one-time; cached
> afterwards). See *Production TODOs* for the prebuilt-base-image plan.

### Building / running manually

Server (C++, from `server/`):

```bash
cmake --preset vcpkg
cmake --build build -j
./build/src/app/robot_server                       # listens on :50051
ctest --test-dir build --output-on-failure         # GoogleTest suite (model + in-process gRPC)
```

The `vcpkg` preset expects `$VCPKG_ROOT` to point at a vcpkg checkout; it
resolves gRPC/protobuf via the vcpkg toolchain file. See
`server/CMakePresets.json` for the exact contents.

Client (Python, from `client/`):

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -e '.[dev]'                # protos regenerate automatically
python -m robot_client info            # unary
python -m robot_client telemetry       # server-streaming
python -m robot_client control         # bidi
pytest tests/                          # unit tests
```

`control` drives joint 0 along a sine wave as a minimal teleop demo; the
other joints stay idle. It exercises the bidirectional path — continuously
streaming targets while telemetry streams back — without needing a full
multi-joint trajectory.

Override the target with `--target host:port` or `ROBOT_TARGET=host:port`.

## Design decisions

- **Contract as internal transport; the SDK facade is a named boundary, not built.** Application developers should get a per-language facade, not raw gRPC. `RobotClient` here is transport-shaped: it returns proto messages unchanged and only adds channel lifecycle, per-RPC deadlines, and human-readable error text (`UNAVAILABLE` → "robot not reachable", `DEADLINE_EXCEEDED` → "timed out"). A real SDK would map proto messages to SDK-owned types so the wire contract can evolve without breaking consumers — deferred here to keep the example minimal.

- **Code generation at build time, generated sources not committed.** `.gitignore` excludes generated `*.pb.*` / `*_pb2*.py`; both sides regenerate from the single contract — cmake on the server, a setuptools `build_py` hook on the client — so `cmake --build` and `pip install` are each the only step needed.

- **Error model: gRPC status codes end-to-end, not a custom error field.** The server returns `INVALID_ARGUMENT` for bad requests and `FAILED_PRECONDITION` for actions taken while in an error state; the client maps these status codes to readable messages. Using standard gRPC status means tooling like deadlines, retries, and tracing works out of the box.

- **`LiveControl` commands are last-write-wins, not queued.** They represent the latest desired motion, so a superseded command is dropped on both client and server — a stale motion target is worse than none. Discrete action sequences (pick → move → place) have the opposite requirement — steps must not be dropped — and would need a separate RPC with backpressure or per-step acknowledgement instead of a single-slot buffer.

- **`LiveControlSession` isolates the grpcio bidi generator/thread ceremony; the client stays synchronous** to match the synchronous server. An async variant (`grpc.aio`) is deferred (see Production TODOs).

## Production TODOs

Deliberately out of scope here; noted as what a real system would need:

**Production features (out of scope):**
- Client-streaming RPC for batch upload of an ordered waypoint sequence — executed in order with no dropped points, distinct from `LiveControl`, which streams latest-only real-time targets
- Per-language SDK facade over this transport, mapping proto to SDK-owned types
- Transport security: TLS / mTLS for fleet service-to-service auth
- Auth: per-call credentials (token in metadata)
- Reconnect strategy, health checks, observability (logging/metrics/tracing interceptors)
- Structured client error mapping: `grpc.StatusCode` → typed exceptions rather than one `RobotClientError`
- Retry / reconnect on `UNAVAILABLE` with backoff
- Async client variant (`grpc.aio`) alongside the synchronous one
- Client packaging & versioning of the SDK (currently editable install only)

**Build / repo hardening:**
- Substitute a base image with gRPC prebuilt to cut the first Docker build from ~30–40 min to a few minutes
- Pin vcpkg to a release tag/commit in `server/Dockerfile` (currently tracks master)

## Tech stack

- **Server:** C++ · gRPC · Protocol Buffers · CMake · vcpkg
- **Client:** Python · grpcio
- **Contract:** proto3, package `robot.v1`
