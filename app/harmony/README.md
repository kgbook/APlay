# APlay Harmony App

`app/harmony` is the HarmonyOS/DevEco Studio import entry. It builds the `APlayReceiver` HAP target and depends on the local `APlaySdk` HAR target under `sdk`.

The ETS SDK facade imports `libaplay_napi.so` from `sdk/src/main/cpp/osal/harmony/napi`. Its ArkTS-visible API is declared by the native module type package at `sdk/src/main/ets/libaplay_napi`, which is referenced from `sdk/oh-package.json5`. The C++ SDK object target is linked into that NAPI library internally; Harmony packaging should only expose `libaplay_napi.so`.

Ensure `ohpm` and `hvigorw` or `hvigor` are available in `PATH`. `DEVECO_SDK_HOME` must point at the DevEco SDK root directory.

```sh
../../scripts/harmony_build.sh
```

The root Harmony project uses `appTasks`, so its aggregate tasks coordinate the app project and its modules. The SDK module uses `harTasks`, and the build script explicitly runs `assembleHar` before `assembleHap` to produce both the SDK HAR and the app HAP. Signing is intentionally not configured; Hvigor will leave the HAP as an unsigned output.

Open this directory in DevEco Studio for interactive development.
