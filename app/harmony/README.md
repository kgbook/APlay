# APlay Harmony App

`app/harmony` is the HarmonyOS/DevEco Studio import entry. It builds the `entry` HAP and depends on the local `aplay_sdk` HAR module under `sdk/src/main/ets`.

The ETS SDK facade calls the NAPI native binding from `sdk/src/main/cpp/napi`, which links the shared C++ SDK target `aplay_cpp_sdk`.

Ensure `ohpm` and `hvigorw` or `hvigor` are available in `PATH`. `DEVECO_SDK_HOME` must point at the DevEco SDK root directory.

```sh
../../scripts/harmony_build.sh
```

Open this directory in DevEco Studio for interactive development.
