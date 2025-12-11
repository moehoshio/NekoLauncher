# Bus Modules

Lightweight access layer for shared services: event bus, config bus, and thread pool.

## Surface

- `eventBus.hpp` — publish/subscribe helpers for UI/core events
- `configBus.hpp` — thread-safe config get/update/save
- `threadBus.hpp` — shared thread pool submit/wrap

## Usage

```cpp
bus::event::publish(neko::event::UpdateAvailableEvent{data});
auto cfg = bus::config::getClientConfig();
bus::thread::submit([]{ /* work */ });
```

## Notes

- Thin wrappers; complex logic lives in the underlying modules (event, config, thread pool).
- No standalone tests; covered by consumers.
