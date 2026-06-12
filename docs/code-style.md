# Code Style

## Naming

- Docs/markdown: `lower-kebab-case` (`mdns-responder.md`)
- C/C++ source: `lower_snake_case` (`mdns_packet.cpp`)
- Java/ETS: platform casing (`APlaySdk.java`, `Index.ets`)
- Directories: lowercase; hyphenate only third-party names
- Update all references when renaming

## File Layout

- One cohesive area per file; helpers near the class they serve.
- Public headers belong directly under the owning module's `include/`
  directory. Keep this set as small as possible, and give each public header a
  single responsibility that matches a real consumer-facing API surface.
- Types, helpers, and adapters that are not part of the consumer contract must
  live under `src/`.
- Prefer a small number of focused public headers over a broad aggregate header,
  but do not couple peer APIs just to share a type. For example, the socket
  module exposes `endpoint.hpp`, `tcp_socket.hpp`, and `udp_socket.hpp`
  directly instead of a catch-all `socket.hpp` or nested implementation headers.

## Module Design

- Single responsibility, open/closed boundaries
- Depend on abstractions at module boundaries; avoid inside modules
- Keep reusable helpers in `core/`
- SDK-facing API only in public headers; internal details stay private

## Logging

- Critical nodes and key state changes must emit necessary logs. This includes
  startup and shutdown, configuration load or validation, permission and
  lifecycle transitions, service discovery and publishing, connection/session
  open and close, request or stream start and finish, retry/fallback paths, and
  unrecoverable errors.
- Critical logs must carry enough context to debug the event without reproducing
  it: component name, action, stable request/session/device identifier when
  available, endpoint or service name when relevant, result/status, error code
  or errno, and elapsed time or byte/count metrics for bounded operations.
- Log at ownership boundaries where control transfers between modules or
  threads. Avoid duplicating the same event in both caller and callee unless the
  two logs add different context.
- Do not log secrets, credentials, raw media payloads, full tokens, or
  personally identifying data. Mask or omit values that could leak user or
  device privacy.
- Keep logs actionable and bounded. Avoid per-packet, per-frame, or tight-loop
  logs unless they are explicitly gated behind debug-only diagnostics.

## C/C++

- C++11 compatible
- Logs: `ALog.h` only
- POSIX APIs + STL; platform APIs only allowed in `osal` and `app` modules.
- Namespaces follow the owning module path for public APIs, such as
  `aplay::protocol::mdns` or `aplay::core::network`. Do not add implementation
  namespace layers such as `internal` or `impl`; private implementation helpers
  must live in `src/` files under the module namespace.
- Do not use anonymous namespaces.
- Define one primary C++ class per source file, third-party code is exempt.
  When a module needs multiple implementations of the same abstraction, keep
  each concrete implementation in its own `lower_snake_case` source file and
  share only the minimal private interface needed to connect them.
