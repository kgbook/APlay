# Agent Notes for AirPlay Spec Docs

## Scope

- This directory is a local, vendored AirPlay protocol reference for APlay.
- Keep the content focused on source documentation and protocol reference material.
- Do not restore upstream build output, mdBook/theme assets, JavaScript tooling, Nix files, CI files, or nested Git metadata here.
- Do not re-add this directory as a Git submodule.

## Layout

- Markdown files that came from the upstream `src` directory now live directly under `docs/airplay-spec` while preserving their subdirectory structure.
- Keep protocol data samples under `docs/airplay-spec/data`.
- Use `README.md` as the local attribution and navigation entrypoint.

## Maintenance

- If protocol references are updated, preserve attribution to the upstream project and keep local APlay implementation notes outside this directory unless they directly clarify how to use the reference.
- If links or references mention the old `src` prefix, update them to the current directory layout.

