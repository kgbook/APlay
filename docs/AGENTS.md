# Agent Notes for Docs

## Documentation Authority

- `docs/code-style.md` defines project code style and must be followed for implementation work.
- `docs/architecture.md`, `docs/software-design.md`, `docs/rewrite-plan.md`, and related diagrams describe intended structure and behavior. Implementation changes should stay consistent with these documents or update the documents in the same change.
- `docs/airplay-spec` is the local AirPlay protocol reference. It is vendored documentation, not an executable submodule.

## Protocol Implementation Requirements

- Before changing AirPlay, RAOP, RTSP, HTTP, mDNS/DNS-SD, pairing, FairPlay, RTP, NTP, discovery TXT records, feature flags, status flags, codecs, encryption, or `/info` behavior, identify the matching section under `docs/airplay-spec`.
- BLE service discovery remains TODO and must not be included in the first baseline.

## Acceptance Requirements

- Protocol validation must compare changed behavior against the relevant `docs/airplay-spec` sections.
- At minimum, validate service names, TXT keys, feature values, status values, request paths, response status codes, and payload fields affected by the change.
- Build validation remains:
  - Linux build: `./scripts/linux_build.sh`
  - Android app: `./scripts/android_build.sh`
  - Harmony HAP: `./scripts/harmony_build.sh`

