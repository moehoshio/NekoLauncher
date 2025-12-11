# Minecraft Module

Helpers for Minecraft-specific auth, download sources, installation, and launch wiring.

## Files

- `authMinecraft.hpp` — auth/authlib helpers
- `downloadSource.hpp` — mirror/source descriptors
- `installMinecraft.hpp` — install/update routines
- `launcherMinecraft.hpp` — launch helpers
- `minecraftSubscribe.hpp` — event subscriptions for MC tasks

## Quick Use

```cpp
// Launch/install flows orchestrated via helpers and bus events
// minecraft::install::installMinecraft(...);
```

## Notes

- Works with core/bus for threading, events, and network IO.
- Errors are surfaced via exceptions; progress via events.
