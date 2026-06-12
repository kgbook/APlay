# Harness Task Tracking

## Purpose

Harness task tracking is a module-agnostic contract for agent/CI validation.
Every module harness, such as mDNS, HTTP, RTSP, crypto, streaming, codec, render,
or platform lifecycle validation, must describe its own phases using the same
task fields and acceptance behavior.

## Acceptance Rules

- Each harness script must record a progress snapshot for every validation run.
- The progress snapshot must list each phase with `TODO`, `DOING`, or `DONE`,
  plus a numeric progress value.
- Each phase entry must state background, goal, current progress, validation,
  and changes so agent work follows Think Before Coding, Simplicity First,
  Surgical Changes, and Goal-Driven Execution.
- If acceptance fails, the current phase remains `DOING`, the result file is
  written as `fail`, and the task is corrected before the run is accepted.
- Harness validation code that needs module internals must live inside the
  owning module and be compiled only for harness builds. Harness executables may
  call a narrow private harness entrypoint, but must not include unrelated module
  implementation headers directly or expose new SDK/API surface only for tests.

## Required Task Fields

Every harness phase must define:

| Field | Requirement |
| --- | --- |
| phase | Stable lower-kebab-case phase name, unique inside one harness script. |
| status | `TODO`, `DOING`, or `DONE`. Failed phases remain `DOING`. |
| progress | Numeric progress value from `0` to `100`. |
| background | Why this phase exists and what risk it reduces. |
| goal | The concrete outcome this phase must reach. |
| current | What is happening now, or why the phase stopped. |
| validation | The exact command, artifact, return code, or observable condition that proves the goal. |
| changes | The implementation area affected by this phase. Use this to prove the change is surgical. |

## Common Phase Pattern

Module harnesses should keep their own phase list small. Use only phases that
produce meaningful validation evidence for that module.

| Phase | Background | Goal | Validation | Changes |
| --- | --- | --- | --- | --- |
| dependencies | The harness needs host tools before producing useful evidence. | Fail early with a clear dependency message. | Required commands are available. | Shell orchestration only. |
| inputs | The module needs fixtures, captures, vectors, device state, or generated resources. | Prove required inputs exist or can be produced. | Input artifact checks or generation command succeeds. | Harness resources only. |
| configure-build | Harness binaries must build from normal project entrypoints. | Configure and build the app plus module harness targets. | Configure and target build succeed with harness macros enabled. | Owning module CMake plus harness entrypoints. |
| execute | The module behavior must be exercised in a bounded run. | Run the module harness against live, fixture, or generated inputs. | Harness executable exits with status 0 and writes logs/artifacts. | Module-owned harness code only. |
| analyze | Raw execution evidence must become a pass/fail result. | Check protocol, state, output, or artifact semantics. | Analyzer exits with status 0 and writes result summary. | Module-owned analyzer code only. |
| cleanup | Validation must be repeatable. | Stop processes and leave bounded artifacts. | No harness-owned process remains; outputs are overwritten or versioned. | Shell orchestration only. |

Module-specific harnesses may replace or split these phases. For example, mDNS
uses a `capture-permission` phase because packet capture has a host permission
precondition, while a pure offline crypto vector harness would not need it.

## Progress Output

Each harness script should write:

- A result file with `status`, `phase`, `progress`, log/artifact paths, and a
  concise message.
- A progress file with one section per phase and all required task fields.
- Any module-specific logs, captures, vectors, or summaries needed for review.

Recommended default paths:

```text
resources/harness/<module>/<name>_result.txt
resources/harness/<module>/<name>_progress.md
resources/harness/<module>/<name>.log
```

Existing module scripts may keep older artifact paths when changing them would
break current validation.

## Current mDNS Instance

`scripts/harness/verify_mdns.sh` currently writes:

- `resources/pcap/mdns_verify_result.txt`
- `resources/pcap/mdns_verify_progress.md`
- `resources/pcap/mdns_replay.log`

Override knobs:

```sh
APLAY_MDNS_RESULT=/tmp/result.txt ./scripts/harness/verify_mdns.sh
APLAY_MDNS_PROGRESS=/tmp/progress.md ./scripts/harness/verify_mdns.sh
APLAY_MDNS_REPLAY_LOG=/tmp/replay.log ./scripts/harness/verify_mdns.sh
```
