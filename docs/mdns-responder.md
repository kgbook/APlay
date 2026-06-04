# APlay mDNS Responder Design

Copyright (C) 2026 kgbook

## Scope

The mDNS implementation is a small AirPlay receiver responder, not a general DNS-SD framework. It covers the discovery records needed by AirPlay clients before the RTSP/AirPlay connection starts:

- `_services._dns-sd._udp.local` service enumeration.
- `_airplay._tcp.local` PTR lookup.
- `_raop._tcp.local` PTR lookup.
- AirPlay and RAOP instance `SRV` records.
- AirPlay and RAOP instance `TXT` capability records.
- Host `A` record for IPv4.
- Goodbye responses with `TTL=0`.

The DNS responder API is exported from `sdk/src/main/cpp/protocol/mdns/include/mdns.hpp`. The implementation lives under `sdk/src/main/cpp/protocol/mdns/src` so protocol, streaming, and crypto stay independent from platform app code. `MdnsResponder` is the single public responder design: it builds offline packets, handles query bytes, and can optionally run the POSIX UDP multicast loop. It uses `core/pattern/singleton` because the UDP 5353 listener is a process-wide service; callers access it through `MdnsResponder::instance()` and update runtime settings with `set_config`. Packet encoding and parsing stay in the mDNS protocol module, while generic IPv4 parsing, UDP multicast socket setup, fd polling, and event dispatch live in `core/network/interface`, `core/socket`, `core/poll`, and `core/eventloop`. Project-owned native code uses POSIX APIs and the C++ STL so it remains portable to embedded toolchains with limited C++ support.

AirPlay and RAOP DNS-SD capability profiles belong to their streaming modules, not to examples. `sdk/src/main/cpp/streaming/airplay/include/airplay.hpp` exports `make_airplay_service`, and `sdk/src/main/cpp/streaming/raop/include/raop.hpp` exports `make_raop_service`. Those helpers produce `protocol::mdns::Service` records for discovery; future AirPlay video and RAOP audio packet handling should remain in the same owning streaming modules instead of moving protocol capability details into `example`.

## Reference Inputs

The behavior is validated against APlay-owned generated packets and live Linux harness captures. Packet analysis notes may inform protocol understanding, but harness checks use APlay service profiles and do not depend on external project naming.

The Linux harness writes its live mDNS capture to `resources/pcap/mdns_announce.pcapng` by default. Replay scans that capture for AirPlay and RAOP DNS-SD names emitted by the announcer.

## DNS-SD Records

A full response for AirPlay and RAOP contains these records:

| Record | Name | Data | Cache flush |
| --- | --- | --- | --- |
| `PTR` | `_services._dns-sd._udp.local` | `_airplay._tcp.local` | no |
| `PTR` | `_services._dns-sd._udp.local` | `_raop._tcp.local` | no |
| `PTR` | `_airplay._tcp.local` | AirPlay instance name | no |
| `PTR` | `_raop._tcp.local` | RAOP instance name | no |
| `SRV` | AirPlay instance name | `host.local:port` | yes |
| `SRV` | RAOP instance name | `host.local:port` | yes |
| `TXT` | AirPlay instance name | AirPlay capability keys | yes |
| `TXT` | RAOP instance name | RAOP capability keys | yes |
| `A` | `host.local` | IPv4 address | yes |

Service records use `TTL=4500`. Host address records use `TTL=120`. Goodbye responses use `TTL=0`.

## Query Handling

The responder parses only the DNS header and question section. Query packets may contain known-answer records; those are valid mDNS traffic and are intentionally ignored for the first baseline instead of being treated as malformed packets.

Supported question matches:

- `_services._dns-sd._udp.local` `PTR` or `ANY`: include registered service type PTR records.
- `_airplay._tcp.local` `PTR` or `ANY`: include AirPlay service records and host `A`.
- `_raop._tcp.local` `PTR` or `ANY`: include RAOP service records and host `A`.
- AirPlay/RAOP instance `SRV`, `TXT`, or `ANY`: include the matching service records and host `A`.
- Host `A` or `ANY`: include host `A`.

If the query class has the QU bit (`0x8000`) or the source port is not `5353`, the Linux POSIX responder sends a unicast copy in addition to the multicast response.

## Packet Size Strategy

The encoder currently does not implement DNS name compression. To avoid multicast UDP fragmentation, it first tries a combined AirPlay + RAOP response and accepts it only when the payload is at most `1200` bytes. If the combined packet is larger, it falls back to separate service packets.

This keeps packet encoding simple first, with a conservative size threshold for multicast UDP.

## Public API

The main DNS responder header is `sdk/src/main/cpp/protocol/mdns/include/mdns.hpp`.

Core types:

- `TxtRecord`: builds DNS-SD TXT payloads from key/value pairs.
- `Service`: describes one DNS-SD service instance.
- `ResponderConfig`: host name, IPv4 address, AirPlay service, and RAOP service.
- `MdnsResponder`: singleton responder accessed with `MdnsResponder::instance()`; builds announcements, goodbye packets, and responses to query bytes; `start`, `stop`, and `announce` run the optional POSIX UDP multicast responder.
- `MdnsParser`: test/debug parser for generated DNS packets, with static `parse_packet`, `parse_question`, and `parse_record` methods using `bool` returns and output parameters for C++11 compatibility.

The IPv4 value is stored as a big-endian numeric address. `core::network::parse_ipv4_address("127.0.0.1", address)` fills the expected value for the Linux harness packet encoding path.

Streaming service profile helpers:

- `streaming/airplay/include/airplay.hpp`: `ServiceProfile` and `make_airplay_service`.
- `streaming/raop/include/raop.hpp`: `ServiceProfile` and `make_raop_service`.
- Harness utilities may set validation-specific profile fields, but they must not hand-build AirPlay or RAOP TXT records.

## Harness Coverage

`harness/mdns/mdns_replay.cpp` validates the responder offline and analyzes pcap captures:

- Builds a dual `_airplay` + `_raop` QU query.
- Verifies that one combined response is generated.
- Parses the generated response and checks all expected PTR/SRV/TXT/A records.
- Verifies cache-flush behavior for unique records and no cache-flush on PTR records.
- Verifies AirPlay goodbye records use `TTL=0`.
- Scans a pcap file for the DNS-encoded AirPlay/RAOP names used by the capture.
- Accepts optional receiver name and device id arguments so live harness captures can be checked against the current announcer identity.

Agent/CI validation:

```sh
./harness/verify_mdns.sh
```

mDNS harness announcer:

```sh
./scripts/linux_build.sh
build/linux/harness/mdns/aplay_harness_mdns_announce APlayHarness
```

By default the announcer starts the POSIX UDP 5353 multicast responder and keeps sending announcement packets until `SIGINT` or `SIGTERM`. Add `--once` to generate and parse announcement packets offline without opening UDP 5353.

`./harness/verify_mdns.sh` captures live UDP 5353 traffic to `resources/pcap/mdns_announce.pcapng` by default, then runs replay against that capture.

The project Linux build script also validates the SDK shared library path:

```sh
./scripts/linux_build.sh
```
