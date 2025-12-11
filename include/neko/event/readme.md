# Event Module

Shared event type declarations for the launcher.

## Files

- `eventTypes.hpp` — strongly-typed events for UI/core (loading status, updates, etc.)

## Notes

- Used by the bus layer (`eventBus.hpp`) for publish/subscribe.
- Pure definitions; logic lives in bus and handlers.
