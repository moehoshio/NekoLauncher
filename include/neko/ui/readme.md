# UI Module

Qt-facing UI layer: themes, windows, dialogs, widgets, and page routing helpers.

## Files

- `theme.hpp` — theme colors/metrics
- `uiEventDispatcher.hpp` — bridge from bus events to UI
- `uiMsg.hpp` — common UI messages/status helpers
- `page.hpp` / `pages/` — page definitions and navigation
- `windows/` — top-level windows (launcher, log viewer, dialogs)
- `widgets/` — shared custom widgets; `fonts.hpp` for font setup
- `uiSubscribe.hpp` — subscriptions to sync UI with core events

## Quick Use

```cpp
ui::UiEventDispatcher::setNekoWindow(&window);
bus::event::publish(event::LoadingStatusChangedEvent{.statusMessage = "Loading..."});
```

## Notes

- Depends on Qt; listens to bus events to update UI.
- Keep UI work on the main thread; heavy work goes through thread bus.
