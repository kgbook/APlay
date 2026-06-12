# Agent Notes

## Build Boundaries

- `app` is a consumer entrypoint according to the target os.
- `sdk` is the shared SDK module, including C++, Java and ETS SDK.
- `example` owns manually runnable example source code and binaries for humans to validate implemented behavior before confirming.

## Platform Responsibilities

- `app/linux` owns Linux UI/business logic: CLI, config, process lifecycle, signal handling, and desktop integration.
- `app/android` owns Android UI/business logic: Activity, Service, Foreground Service, permissions, notifications, and Android lifecycle.
- `app/harmony` owns Harmony-facing app lifecycle and imports the ETS SDK HAR.
- `sdk` owns CPP/Java/ETS SDK APIs and their packaging.
- `core` owns common reusable C/C++ code.
- `osal` currently owns platform codec/render modules and native binding submodules.
- 
## Workflow Constraints

- Do not push to remote unless explicitly asked. Only commit locally and wait for user verification before pushing.

## Implementation Rules

- Follow `docs/AGENTS.md` for documentation-driven implementation and acceptance requirements.
- Use `docs/airplay-spec` as the local AirPlay protocol reference. When implementing AirPlay, RAOP, RTSP, HTTP, mDNS/DNS-SD, pairing, FairPlay, RTP, NTP, or related capability fields, map behavior back to the corresponding spec section before coding.
- After changing third-party submodule paths, run `git submodule sync --recursive` and `git submodule update --init --recursive`.
- BLE service discovery is TODO and should not be implemented in the first baseline.

## Validation

- Protocol validation must include checks against `docs/airplay-spec` for any implemented or changed module behavior. At minimum, verify advertised service names, TXT keys, feature/status values, request paths, response status codes, and payload fields against the relevant spec section.

### Build validation
- Linux build: `./scripts/linux_build.sh`.
- Android app: `./scripts/android_build.sh`.
- Harmony HAP: `./scripts/harmony_build.sh`.
