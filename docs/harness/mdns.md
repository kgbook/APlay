# mDNS Harness

## Linux Flow

Run:

```sh
./harness/verify_mdns.sh
```

The script performs these steps:

- Checks for `git`, `cmake`, `ninja`, `timeout`, and either `dumpcap` or `tcpdump` before building.
- Checks that the selected capture interface can be opened without sudo before building.
- Builds `APlayReceiver`, `aplay_sdk`, `aplay_harness_mdns_announce`, and `aplay_harness_mdns_replay`.
- Starts `dumpcap`, or `tcpdump` when `dumpcap` is unavailable or cannot be opened without sudo, for UDP 5353 traffic.
- Uses unprivileged capture only. The script does not request sudo or read sudo passwords.
- Starts `aplay_harness_mdns_announce` with receiver name `APlayHarness`.
- Stores the captured packets at `resources/pcap/mdns_announce.pcapng` by default.
- Runs `aplay_harness_mdns_replay resources/pcap/mdns_announce.pcapng APlayHarness 02:00:00:00:00:01 APlayHarness.local`.

Override knobs:

```sh
APLAY_CAPTURE_IFACE=wlan0 ./harness/verify_mdns.sh
APLAY_PCAP_CAPTURE=/tmp/aplay-mdns.pcap ./harness/verify_mdns.sh
```

## Capture Permissions

The harness only detects missing dependencies and capture permission problems; it does not install packages, change groups, set capabilities, request sudo, or read passwords. For passwordless validation, configure capture privileges once outside the harness.

Preferred Debian/Ubuntu setup:

```sh
sudo apt-get install git cmake ninja-build tcpdump wireshark-common
sudo dpkg-reconfigure wireshark-common
sudo usermod -aG wireshark "$USER"
```

Start a new login session after changing groups.

Capability-based alternative:

```sh
sudo setcap cap_net_raw,cap_net_admin=eip "$(command -v dumpcap)"
sudo setcap cap_net_raw,cap_net_admin=eip "$(command -v tcpdump)"
```

`APLAY_CAPTURE_IFACE=any` captures UDP 5353 packets visible on all Linux interfaces. `APLAY_CAPTURE_IFACE=lo` can keep validation local to loopback, but packet capture on `lo` still requires the same capture privileges.

## Unsupported Promiscuous Mode

Linux `any` is a pseudo-interface and does not support promiscuous mode. Without explicit non-promiscuous capture, `dumpcap` may print:

```text
dumpcap: Promiscuous mode not supported on the "any" device.
```

`verify_mdns.sh` starts both `dumpcap` and `tcpdump` with `-p` to disable promiscuous mode. This keeps the default `APLAY_CAPTURE_IFACE=any` path compatible with Linux while still capturing UDP 5353 multicast traffic visible to the host.

If this message still appears, check that the script being run is the current `harness/verify_mdns.sh` and that no local wrapper removes `-p` from the capture command.

## Announce

`build/linux/harness/mdns/aplay_harness_mdns_announce` defaults to live behavior:

```sh
build/linux/harness/mdns/aplay_harness_mdns_announce APlayHarness
```

It starts the POSIX mDNS responder on UDP 5353 and keeps broadcasting AirPlay/RAOP announcement packets until it receives `SIGINT` or `SIGTERM`. The responder attempts both IPv4 `224.0.0.251:5353` and IPv6 `ff02::fb:5353` listeners and keeps running if at least one opens; host records include both `A` and `AAAA` when configured.

Useful options:

- `--interval-ms <ms>` changes the announcement interval. The default is `1000`.
- `--once` only builds and parses announcement packets offline, then exits.

## Replay

`build/linux/harness/mdns/aplay_harness_mdns_replay` validates generated responder output and scans a pcap file for DNS-encoded mDNS service names:

```sh
build/linux/harness/mdns/aplay_harness_mdns_replay <pcap-file> [receiver-name] [device-id] [host-name]
```

When receiver name, device id, and host name are omitted, replay uses the APlay harness defaults: `APlayHarness`, `02:00:00:00:00:01`, and `APlayHarness.local`. The Linux harness passes those values explicitly so the live capture is checked against the packet names that `announce` actually emits. External reference captures can pass a different host name when the DNS-SD instance name and SRV target differ.

Replay checks generated packet semantics and pcap contents. Generated packets must include PTR/SRV/TXT plus host `A` and `AAAA` records. Pcap scans require both IPv4 mDNS multicast (`224.0.0.251`) and IPv6 mDNS multicast (`ff02::fb`) traffic, plus receiver AirPlay/RAOP DNS-SD names and host `A`/`AAAA` records.
