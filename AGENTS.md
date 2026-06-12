# Agent Notes

## Build Boundaries

- `app` is a consumer entrypoint according to the target os.
- `sdk` is the shared SDK module, including C++, Java and ETS SDK.

## Platform Responsibilities

- `app/linux` owns Linux UI/business logic: CLI, config, process lifecycle, signal handling, and desktop integration.
- `app/android` owns Android UI/business logic: Activity, Service, Foreground Service, permissions, notifications, and Android lifecycle.
- `app/harmony` owns Harmony-facing app lifecycle and imports the ETS SDK HAR.
- `sdk` owns CPP/Java/ETS SDK APIs and their packaging.
- `core` owns common reusable C/C++ code.
- `osal` currently owns platform codec/render modules and native binding submodules.

## Implementation Rules

- Follow `docs/AGENTS.md` for documentation-driven implementation and acceptance requirements.
- Follow `cmake/CMakeHelper/AGENTS.md` for CMake code implementation.
- Follow `docs/airplay-spec/AGENTS.md`, and Implement all protocol, streaming, cryptography, and interaction logic strictly.
- After changing third-party submodule paths, run `git submodule sync --recursive` and `git submodule update --init --recursive`.

## Validation

Validate, review, and correct all code against `docs/airplay-spec` before submission. Any implementation that does not comply with the specification must be fixed.

### Build validation
- Linux build: `./scripts/linux_build.sh`.
- Android app: `./scripts/android_build.sh`.
- Harmony HAP: `./scripts/harmony_build.sh`.
