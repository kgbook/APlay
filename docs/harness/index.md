# APlay Harness

## Scope

`scripts/harness` owns agent validation orchestration. It builds the relevant targets, runs validation utilities, manages shared resources, and makes pass/fail decisions.

## Modules

- [Design](design.md)
  Documents harness boundaries, ownership, validation types, current Phase 0 behavior, and BLE TODO scope.

- [mDNS](mdns.md)
  Validates the Linux mDNS responder by capturing live UDP 5353 announcement traffic and replaying the capture through the automated analyzer.

- [Tasks](tasks.md)
  Defines harness task status, progress, and acceptance records for agent/CI validation runs.

## Boundaries

- `scripts/harness/` owns validation scripts, target selection, fixture/resource paths, execution order, and pass/fail behavior.
- Module-specific harness source lives inside the owning module behind compile macros; mDNS harness entrypoints are under `sdk/src/main/cpp/protocol/mdns/harness`.
- `resources/pcap/` owns shared packet capture inputs produced or consumed by harness validation.
