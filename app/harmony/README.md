# APlay Harmony App

`app/harmony` is the HarmonyOS/DevEco Studio import entry. It builds the `entry` HAP and depends on the local `aplay_sdk` HAR module under `sdk/src/main/ets`.

The ETS SDK facade calls the NAPI native binding from `sdk/src/main/cpp/osal/harmony/napi`, which consumes the shared C++ SDK object target and the merged `aplay_sdk` shared library output.

Ensure `ohpm` and `hvigorw` or `hvigor` are available in `PATH`. `DEVECO_SDK_HOME` must point at the DevEco SDK root directory.

```sh
../../scripts/harmony_build.sh
```

Open this directory in DevEco Studio for interactive development.
