# Core Module

Launcher core services: update/maintenance checks, launcher process helpers, remote config, feedback/auth utilities, and poster download support.

## Files

- `update.hpp` — check/parse/apply updates
- `maintenance.hpp` — maintenance gate + info
- `launcher.hpp` / `launcherProcess.hpp` — launch/relay to external processes
- `remoteConfig.hpp` — fetch dynamic config
- `auth.hpp` / `feedback.hpp` — auth and feedback helpers
- `downloadPoster.hpp` — fetch update posters

## Quick Use

```cpp
// Check maintenance then auto-update
neko::core::update::autoUpdate();

// Launch the Minecraft process (helper wraps platform specifics)
// core::launcher::launch(...);
```

## Notes

- Uses network + event bus; errors propagate via exceptions.
- Update flow emits bus events for UI status/progress.
