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
- Host `AAAA` record for IPv6.
- Goodbye responses with `TTL=0`.

The DNS responder API is exported from `sdk/src/main/cpp/protocol/mdns/include/mdns.hpp`. The implementation lives under `sdk/src/main/cpp/protocol/mdns/src`. `Responder` is the single responder implementation: it builds offline packets, handles query bytes, and can optionally run the POSIX UDP multicast loop. It uses `core/pattern/singleton` because the UDP 5353 listener is a process-wide service; callers access it through `Responder::instance()` and update runtime settings with `set_config`. Packet encoding and parsing stay in the mDNS protocol module, while generic IPv4/IPv6 parsing, UDP multicast socket setup, fd polling, and event dispatch live in `core/network/interface`, `core/socket`, `core/poll`, and `core/eventloop`. Project-owned native code uses POSIX APIs and the C++ STL so it remains portable to embedded toolchains with limited C++ support.

AirPlay and RAOP DNS-SD capability profiles belong to the mDNS protocol module, not to examples. `sdk/src/main/cpp/protocol/mdns/src/mdns_service.hpp` declares `AirPlayServiceProfile`, `RaopServiceProfile`, `make_airplay_service`, and `make_raop_service` for the responder and harness. Those helpers produce `protocol::mdns::Service` records for discovery; future AirPlay video and RAOP audio packet handling should remain in the owning streaming modules instead of moving runtime behavior into `example`.

## Reference Inputs

The behavior is validated against APlay-owned generated packets, live Linux harness captures, and reference captures that contain both IPv4 mDNS (`224.0.0.251`) and IPv6 mDNS (`ff02::fb`). Packet analysis notes may inform protocol understanding, but harness checks use APlay service profiles and do not depend on external project naming.

The Linux harness writes its live mDNS capture to `resources/pcap/mdns_announce.pcapng` by default. Replay scans that capture for AirPlay and RAOP DNS-SD names emitted by the announcer.

## DNS-SD Records

A family-specific AirPlay/RAOP response contains these records. IPv4 packets include the host
`A` record and IPv6 packets include the host `AAAA` record; cross-family host records are not
placed in the same response.

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
| `A` | `host.local` | IPv4 address, only in IPv4 responses | yes |
| `AAAA` | `host.local` | IPv6 address, only in IPv6 responses | yes |

Service records use `TTL=4500`. Host address records use `TTL=120`. Goodbye responses use `TTL=0`.

## Query Handling

The responder parses only the DNS header and question section. Query packets may contain known-answer records; those are valid mDNS traffic and are intentionally ignored for the first baseline instead of being treated as malformed packets.

Supported question matches:

- `_services._dns-sd._udp.local` `PTR` or `ANY`: include registered service type PTR records.
- `_airplay._tcp.local` `PTR` or `ANY`: include AirPlay service records and the matching-family host address record.
- `_raop._tcp.local` `PTR` or `ANY`: include RAOP service records and the matching-family host address record.
- AirPlay/RAOP instance `SRV`, `TXT`, or `ANY`: include the matching service records and the matching-family host address record.
- Host `A` on IPv4, host `AAAA` on IPv6, or `ANY`: include the matching-family host address record.

If the query class has the QU bit (`0x8000`) or the source port is not `5353`, the Linux POSIX responder sends a unicast copy in addition to the multicast response. IPv4 responses are sent on `224.0.0.251:5353` with IPv4 host records only; IPv6 responses are sent on `ff02::fb:5353` with IPv6 host records only and are scoped to the interface that received the query when available. Startup attempts both IPv4 and IPv6 listeners and only fails when both fail; live announcements are emitted on every successfully opened family.

## Packet Size Strategy

The encoder currently does not implement DNS name compression. To avoid multicast UDP fragmentation, it first tries a combined AirPlay + RAOP response and accepts it only when the payload is at most `1200` bytes. If the combined packet is larger, it falls back to separate service packets.

This keeps packet encoding simple first, with a conservative size threshold for multicast UDP.

## Public API

The SDK entry header is `sdk/src/main/cpp/protocol/mdns/include/mdns.hpp`.

Core types:

- `TxtRecord`: builds DNS-SD TXT payloads from key/value pairs.
- `Service`: describes one DNS-SD service instance.
- `ResponderConfig`: host name, IPv4 address, IPv6 address, AirPlay service, and RAOP service.
- `Responder`: singleton responder accessed with `Responder::instance()`; builds `AddressFamily`-specific announcements, goodbye packets, and responses to query bytes; `start`, `stop`, and `announce` run the optional POSIX UDP multicast responder.
- `PacketParser`: test/debug parser declared with packet helpers in `protocol/mdns/src/mdns_packet.hpp`.

The IPv4 value is stored in host byte order. `core::network::parse_ipv4_address("127.0.0.1", address)` fills the expected value for the Linux harness packet encoding path. The IPv6 value is stored as 16 network-order bytes and can be filled with `core::network::parse_ipv6_address("::1", address)`.

Internal service profile helpers:

- `protocol/mdns/src/mdns_service.hpp`: `AirPlayServiceProfile`, `RaopServiceProfile`, `make_airplay_service`, and `make_raop_service`.
- `streaming/connection/include/connection.hpp`: connection classification and dispatch boundary for HTTP/HLS/RTSP control connections.
- `streaming/airplay/include/airplay.hpp` and `streaming/raop/include/raop.hpp`: media-stream ownership boundaries for video mirror and RAOP audio processing.
- Harness utilities may set validation-specific profile fields, but they must not hand-build AirPlay or RAOP TXT records.

## Harness Coverage

`sdk/src/main/cpp/protocol/mdns/src/mdns_harness.cpp` validates the responder offline and analyzes pcap captures behind `APLAY_MDNS_HARNESS`:

- Builds a dual `_airplay` + `_raop` QU query.
- Verifies that one combined response is generated per address family.
- Parses generated IPv4 and IPv6 responses, checks expected PTR/SRV/TXT records, requires `A` only in IPv4 responses, and requires `AAAA` only in IPv6 responses.
- Verifies cache-flush behavior for unique records and no cache-flush on PTR records.
- Verifies AirPlay goodbye records use `TTL=0`.
- Scans a pcap file for IPv4 and IPv6 mDNS multicast addresses and DNS-encoded AirPlay/RAOP names used by the capture.
- Accepts optional receiver name, device id, and host name arguments so live harness captures and external reference captures can be checked against their actual identity.

Agent/CI validation:

```sh
./scripts/harness/verify_mdns.sh
```

mDNS harness announcer:

```sh
./scripts/app/linux_build.sh
build/linux/scripts/harness/mdns/aplay_harness_mdns_announce APlayHarness
```

By default the announcer starts the POSIX UDP 5353 multicast responder and keeps sending announcement packets until `SIGINT` or `SIGTERM`. Add `--once` to generate and parse announcement packets offline without opening UDP 5353.

`./scripts/harness/verify_mdns.sh` captures live UDP 5353 traffic to `resources/pcap/mdns_announce.pcapng` by default, then runs replay against that capture.

The project Linux build script also validates the SDK shared library path:

```sh
./scripts/app/linux_build.sh
```
