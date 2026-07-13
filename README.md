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

<!-- TODO: fill after proto is final -->
Robotics setting: _<one line>_

| RPC | Type | Purpose |
|-----|------|---------|
| `GetRobotInfo` | unary | identity + current state query |
| `StreamTelemetry` | server-streaming | one-way: robot pushes state at N Hz |
| `LiveControl` | bidirectional | client commands + robot state, independent streams |

## Running it

<!-- TODO: verify commands once docker-compose is in place -->
```bash
docker compose up --build
```
Brings up the server and runs the client against it. Expected output: _<...>_

### Building / running manually
<!-- TODO -->

## Design decisions

<!-- Keep each to 1–2 sentences. This section is where "reasoning under ambiguity" is graded. -->

- **Contract as internal transport, facade left as a boundary.** _<why>_
- **Code generation at build time, generated sources not committed.** Keeps the diff to the contract plus hand-written code; `.gitignore` excludes generated `*.pb.*` / `*_pb2*.py`.
- **Error model.** _<gRPC status codes vs status field — state the choice>_
- **Three RPCs cover three of the four gRPC modes.** Client-streaming is left as a TODO (see below).
- _<add as decisions are made>_

## Production TODOs

Deliberately out of scope here; noted as what a real system would need:

- Client-streaming RPC (e.g. trajectory upload)
- Per-language SDK facade over this transport
- Transport security: TLS / mTLS for fleet service-to-service auth
- Auth: per-call credentials (token in metadata)
- Backpressure / flow control on streams
- Reconnect strategy, health checks, observability (logging/metrics/tracing interceptors)
- _<add>_

## Tech stack

- **Server:** C++ · gRPC · Protocol Buffers · CMake · vcpkg
- **Client:** Python · grpcio
- **Contract:** proto3, package `robot.v1`
