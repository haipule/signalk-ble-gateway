# Gateway library packaging spike (2026-10-25)

## Scope

This spike checks whether the repository can remain the source for the npm
consumer, standalone firmware, and a separately built PlatformIO library. It
does not move the production gateway implementation or alter either
engine-room application.

## Package layout

The repository-root `library.json` names the package
`signalk-remote-ble-gateway` and restricts its compiler inputs to:

```text
gateway-library/include
gateway-library/src
```

The standalone `firmware/` project and `consumer-plugin/` package therefore
remain outside the C++ library build. The initial public header exposes only a
development version constant; the transport API moves in Phase 2.

## Verification

PlatformIO Core 6.1.19 successfully:

1. validated and packed the repository with `pio pkg pack`;
2. installed commit `e07cc0348e98348dd04f6dbc514234d62c6c73ee` from a local
   Git URL declared in a separate project's `lib_deps`;
3. identified the installed package as
   `signalk-remote-ble-gateway@0.1.0-dev+sha.e07cc03`; and
4. compiled and linked a consumer that included
   `<signalk_remote_ble_gateway/version.h>`.

The test used `git+file://` so it did not require publishing or modifying a
remote repository. PlatformIO uses the same Git package flow for a pinned
HTTPS repository URL.

## Conclusion and remaining gate

The monorepo layout is viable for direct, immutable Git dependencies in
`lib_deps`; a separate library repository is not currently required.

This is deliberately only a packaging proof. Phase 2 must still move the real
transport implementation, declare its upstream dependencies, compile the
standalone ESP32 target, and establish behavior tests before an engine-room
application consumes it.

