# Code Style

## Naming

- Docs/markdown: `lower-kebab-case` (`mdns-responder.md`)
- C/C++ source: `lower_snake_case` (`mdns_packet.cpp`)
- Java/ETS: platform casing (`APlaySdk.java`, `Index.ets`)
- Directories: lowercase; hyphenate only third-party names
- Update all references when renaming

## File Layout

- One cohesive area per file; helpers near the class they serve
- Public headers under module's `include/`, private headers under `src/`

## Module Design

- Single responsibility, open/closed boundaries
- Depend on abstractions at module boundaries; avoid inside modules
- Keep reusable helpers in `core/`
- SDK-facing API only in public headers; internal details stay private

## CMake

- CMake code must use `cmake/CMakeHelper` target declarations instead of direct target creation helpers such as `add_library`, `add_executable`, `target_include_directories`, or `target_link_
         -libraries`.                                                                                                                                                                                                  
- Use the Android.mk-style `CMakeHelper` flow: `include(${CLEAR_VARS})`, set the relevant `LOCAL_*` variables, then include the matching `BUILD_*` file.                                                     
- Export public include directories from the owning target with `LOCAL_EXPORT_C_INCLUDES`.                                                                                                                   
- Keep private include directories on the compiling target with `LOCAL_C_INCLUDES`.                                                                                                                          
- Model module coupling with target dependencies. Do not compensate for missing dependencies by adding broad include directories to unrelated targets.                                                       
- CMake exceptions are limited to third-party submodules and the `cmake/CMakeHelper` implementation itself.

## C/C++

- C++11 compatible
- Logs: `ALog.h` only
- POSIX APIs + STL; no platform APIs outside platform modules